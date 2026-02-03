#include "StageViewWindow.h"

#include <pxr/base/gf/frustum.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/tf/token.h>
#include <pxr/imaging/hd/tokens.h>
#include <pxr/imaging/hdx/tokens.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include <pxr/usdImaging/usdImagingGL/renderParams.h>
#include <qevent.h>
#include <qnamespace.h>

#include <iostream>

#include "FreeCamera.h"

StageViewWindow::StageViewWindow()
    : QOpenGLWindow(QOpenGLWindow::NoPartialUpdate, nullptr),
      m_engine(nullptr),
      m_debugLogger(nullptr) {}

StageViewWindow::~StageViewWindow() = default;

void StageViewWindow::initializeGL() {
    initializeOpenGLFunctions();

    m_stage = pxr::UsdStage::Open("display.usda");

    m_engine = new pxr::UsdImagingGLEngine();
    // m_engine->SetCameraPath(pxr::SdfPath("/root/Camera/Camera"));

    m_renderParams = pxr::UsdImagingGLRenderParams();
    m_renderParams.drawMode = pxr::UsdImagingGLDrawMode::DRAW_SHADED_SMOOTH;
    m_renderParams.clearColor = pxr::GfVec4f(0.1f, 0.1f, 0.1f, 1.0f);
    m_renderParams.colorCorrectionMode = pxr::HdxColorCorrectionTokens->sRGB;
}

void StageViewWindow::resizeGL(int w, int h) {
    std::cout << "Viewport size: " << w << "x" << h << std::endl;
    m_engine->SetRenderViewport(pxr::GfVec4d(0, 0, w, h));
    m_engine->SetRenderBufferSize(pxr::GfVec2i(w, h));
    m_engine->SetRendererAov(pxr::HdAovTokens->color);
}

void StageViewWindow::paintGL() {
    std::cout << "Painting GL" << std::endl;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
    }
}

void StageViewWindow::mouseReleaseEvent(QMouseEvent *event) {
    m_isMoving = false;
}

void StageViewWindow::mouseMoveEvent(QMouseEvent *event) {
    if (m_isMoving) {
        auto delta = event->position() - m_startPos;

        std::cout << "delta: " << delta.x() << "  " << delta.y() << std::endl;

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