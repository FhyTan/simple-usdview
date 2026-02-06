#include "StageViewWindow.h"

#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/gf/frustum.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/vec2d.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/vec4d.h>
#include <pxr/base/tf/token.h>
#include <pxr/imaging/hd/tokens.h>
#include <pxr/imaging/hdx/tokens.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/timeCode.h>
#include <pxr/usd/usdGeom/tokens.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include <pxr/usdImaging/usdImagingGL/renderParams.h>
#include <qevent.h>
#include <qnamespace.h>

#include <iostream>
#include <memory>
#include <vector>

#include "FreeCamera.h"

StageViewWindow::StageViewWindow()
    : QOpenGLWindow(QOpenGLWindow::NoPartialUpdate, nullptr),
      m_engine(nullptr),
      m_debugLogger(nullptr),
      m_bboxCache(pxr::UsdTimeCode::Default(),
                  pxr::TfTokenVector{pxr::UsdGeomTokens->default_,
                                     pxr::UsdGeomTokens->render,
                                     pxr::UsdGeomTokens->proxy}) {}

StageViewWindow::~StageViewWindow() = default;

void StageViewWindow::initializeGL() {
    initializeOpenGLFunctions();

    m_stage = pxr::UsdStage::Open("display.usda");

    m_engine = new pxr::UsdImagingGLEngine();
    m_engine->SetRendererAov(pxr::HdAovTokens->color);
    m_engine->SetSelectionColor(pxr::GfVec4f(0.5, 1.0, 0.5, 0.5));

    m_renderParams = pxr::UsdImagingGLRenderParams();
    m_renderParams.drawMode = pxr::UsdImagingGLDrawMode::DRAW_SHADED_SMOOTH;
    m_renderParams.clearColor = pxr::GfVec4f(0.1f, 0.1f, 0.1f, 1.0f);
    m_renderParams.colorCorrectionMode = pxr::HdxColorCorrectionTokens->sRGB;
    m_renderParams.highlight = true;
    m_renderParams.bboxLineColor = pxr::GfVec4f(1.0, 1.0, 1.0, 0.5);
    m_renderParams.bboxLineDashSize = 3;
    m_renderParams.showGuides = true;
}

void StageViewWindow::resizeGL(int w, int h) {
    m_engine->SetRenderViewport(pxr::GfVec4d(0, 0, w, h));
    m_engine->SetRenderBufferSize(pxr::GfVec2i(w, h));
}

void StageViewWindow::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_bboxToDraw) {
        m_renderParams.bboxes = std::vector<pxr::GfBBox3d>{*m_bboxToDraw};
    } else {
        m_renderParams.bboxes = std::vector<pxr::GfBBox3d>{};
    }

    m_engine->SetCameraState(m_camera.GetViewMatrix(),
                             m_camera.GetProjectionMatrix());
    m_engine->Render(m_stage->GetPseudoRoot(), m_renderParams);
}

void StageViewWindow::closeEvent(QCloseEvent *event) {
    // Explicitly release resources before the OpenGL context is destroyed
    delete m_engine;
    delete m_debugLogger;
    QOpenGLWindow::closeEvent(event);
}

void StageViewWindow::wheelEvent(QWheelEvent *event) {
    int delta = event->angleDelta().y();
    m_camera.Zoom(static_cast<double>(delta) * 0.01);
    update();
}

void StageViewWindow::mousePressEvent(QMouseEvent *event) {
    if (event->modifiers() & Qt::AltModifier) {
        // Navigating
        m_startPos = event->position();
        m_isMoving = true;

        switch (event->button()) {
            case Qt::LeftButton:
                m_navigateType = Orbiting;
                break;
            case Qt::MiddleButton:
                m_navigateType = Panning;
                break;
            case Qt::RightButton:
                m_navigateType = Zooming;
                break;
            default:
                break;
        }
    } else if (event->button() == Qt::LeftButton) {
        // Picking
        auto clickPoint = event->position();
        auto windowSize = size();
        // qDebug() << "click pos:" << clickPoint;

        auto x = clickPoint.x() / windowSize.width() * 2 - 1;
        auto y =
            (windowSize.height() - clickPoint.y()) / windowSize.height() * 2 -
            1;
        auto w = 1.0 / windowSize.width();
        auto h = 1.0 / windowSize.height();
        // qDebug() << "window pos:" << x << y;
        // qDebug() << "pick size:" << w << h;

        auto pickFrustum = m_camera.GetFrustum().ComputeNarrowedFrustum(
            pxr::GfVec2d(x, y), pxr::GfVec2d(w, h));

        pxr::UsdImagingGLEngine::PickParams pickParams{
            pxr::TfToken("resolveNearestToCenter")};
        pxr::UsdImagingGLEngine::IntersectionResultVector results;

        makeCurrent();
        m_engine->TestIntersection(pickParams, pickFrustum.ComputeViewMatrix(),
                                   pickFrustum.ComputeProjectionMatrix(),
                                   m_stage->GetPseudoRoot(), m_renderParams,
                                   &results);

        if (results.empty()) {
            // qDebug() << "Select no prim, clear selection";
            m_engine->SetSelected(pxr::SdfPathVector{});
            m_bboxToDraw = nullptr;
        } else {
            auto hitPrimPath = results[0].hitPrimPath;
            // qDebug() << "Select prim:" << hitPrimPath.GetString();
            m_engine->SetSelected(pxr::SdfPathVector{hitPrimPath});

            auto prim = m_stage->GetPrimAtPath(hitPrimPath);
            m_bboxToDraw = std::make_unique<pxr::GfBBox3d>(
                m_bboxCache.ComputeWorldBound(prim));
        }
        update();
    }
}

void StageViewWindow::mouseReleaseEvent(QMouseEvent *event) {
    m_isMoving = false;
}

void StageViewWindow::mouseMoveEvent(QMouseEvent *event) {
    if (m_isMoving) {
        auto delta = event->position() - m_startPos;

        switch (m_navigateType) {
            case Orbiting:
                m_camera.Orbit(-delta.x() * 0.5, -delta.y() * 0.5);
                break;
            case Panning:
                m_camera.Pan(-delta.x() * 0.01, delta.y() * 0.01);
                break;
            case Zooming:
                m_camera.Zoom(-delta.y() * 0.1);
                break;
        }

        m_startPos = event->position();
        update();
    }
}

void StageViewWindow::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
        case Qt::Key_F: {
            // Focus to prim
            if (m_bboxToDraw) {
                m_camera.Fit(*m_bboxToDraw);
                update();
            }
            break;
        }

        case Qt::Key_A: {
            // Focus to all
            auto bbox = m_bboxCache.ComputeWorldBound(m_stage->GetPseudoRoot());
            m_camera.Fit(bbox);
            update();
            break;
        }
    }
}