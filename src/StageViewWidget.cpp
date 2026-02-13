#include "StageViewWidget.h"

#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/gf/frustum.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/rotation.h>
#include <pxr/base/gf/vec2d.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/vec4d.h>
#include <pxr/base/gf/vec4f.h>
#include <pxr/base/tf/token.h>
#include <pxr/base/vt/value.h>
#include <pxr/imaging/cameraUtil/conformWindow.h>
#include <pxr/imaging/glf/simpleLight.h>
#include <pxr/imaging/glf/simpleMaterial.h>
#include <pxr/imaging/hd/tokens.h>
#include <pxr/imaging/hdx/tokens.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/common.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/timeCode.h>
#include <pxr/usd/usdGeom/tokens.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include <pxr/usdImaging/usdImagingGL/renderParams.h>
#include <pxr/usdImaging/usdImagingGL/rendererSettings.h>
#include <qboxlayout.h>
#include <qcoreevent.h>
#include <qevent.h>
#include <qnamespace.h>
#include <qopenglwindow.h>
#include <qoverload.h>
#include <qtmetamacros.h>
#include <qvariant.h>
#include <qwidget.h>
#include <winsock.h>

#include <QMimeData>
#include <QVBoxLayout>
#include <iostream>
#include <memory>
#include <optional>
#include <vector>

#include "FreeCamera.h"
#include "Settings.h"

StageViewWindow::StageViewWindow()
    : QOpenGLWindow(QOpenGLWindow::NoPartialUpdate, nullptr),
      m_engine(nullptr),
      m_camera(new FreeCamera(this)),
      m_bboxCache(pxr::UsdTimeCode::Default(),
                  pxr::TfTokenVector{pxr::UsdGeomTokens->default_,
                                     pxr::UsdGeomTokens->render,
                                     pxr::UsdGeomTokens->proxy}),
      m_debugLogger(nullptr) {
    connect(m_camera, &FreeCamera::viewUpdated, this,
            qOverload<>(&StageViewWindow::update));
    connect(this, &StageViewWindow::primSelected, this,
            &StageViewWindow::onPrimSelected);
}

StageViewWindow::~StageViewWindow() = default;

void StageViewWindow::initializeRenderEngine() {
    makeCurrent();

    if (m_engine) {
        delete m_engine;
    }

    m_engine = new pxr::UsdImagingGLEngine();
    m_engine->SetRendererAov(pxr::HdAovTokens->color);
    m_engine->SetSelectionColor(pxr::GfVec4f(0.5, 1.0, 0.5, 0.5));

    int w = width();
    int h = height();
    m_engine->SetRenderViewport(pxr::GfVec4d(0, 0, w, h));
    m_engine->SetRenderBufferSize(pxr::GfVec2i(w, h));

    auto light = pxr::GlfSimpleLight();
    auto material = pxr::GlfSimpleMaterial();
    auto ambient = pxr::GfVec4f(0.01, 0.01, 0.01, 1.0);

    light.SetIsDomeLight(true);
    light.SetTransform(pxr::GfMatrix4d().SetRotate(
        pxr::GfRotation(pxr::GfVec3d::XAxis(), 90)));

    m_engine->SetLightingState(pxr::GlfSimpleLightVector{light}, material,
                               ambient);
    m_engine->SetRendererSetting(
        pxr::HdRenderSettingsTokens->domeLightCameraVisibility,
        pxr::VtValue(false));
}

void StageViewWindow::initializeGL() {
    initializeOpenGLFunctions();
    initializeRenderEngine();

    m_stage = pxr::UsdStage::CreateInMemory();

    m_renderParams = pxr::UsdImagingGLRenderParams();
    m_renderParams.drawMode = pxr::UsdImagingGLDrawMode::DRAW_SHADED_SMOOTH;
    m_renderParams.clearColor = pxr::GfVec4f(0.1f, 0.1f, 0.1f, 1.0f);
    m_renderParams.colorCorrectionMode = pxr::HdxColorCorrectionTokens->sRGB;
    m_renderParams.highlight = true;
    m_renderParams.bboxLineColor = pxr::GfVec4f(1.0, 1.0, 1.0, 0.5);
    // m_renderParams.bboxLineDashSize = 3;
    m_renderParams.enableSceneLights = false;
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

    m_engine->SetCameraState(m_camera->getViewMatrix(),
                             m_camera->getProjectionMatrix());
    m_engine->Render(m_stage->GetPseudoRoot(), m_renderParams);
}

bool StageViewWindow::event(QEvent *event) {
    if (event->type() == QEvent::Drop) {
        dropEvent(static_cast<QDropEvent *>(event));
        return true;
    }

    return QOpenGLWindow::event(event);
}

void StageViewWindow::closeEvent(QCloseEvent *event) {
    // Explicitly release resources before the OpenGL context is destroyed
    delete m_engine;
    delete m_debugLogger;
    QOpenGLWindow::closeEvent(event);
}

void StageViewWindow::wheelEvent(QWheelEvent *event) {
    int delta = event->angleDelta().y();
    m_camera->zoom(static_cast<double>(delta) * 1 / 3);
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

        double x = clickPoint.x() / windowSize.width() * 2 - 1;
        double y =
            (windowSize.height() - clickPoint.y()) / windowSize.height() * 2 -
            1;
        double w = 1.0 / windowSize.width();
        double h = 1.0 / windowSize.height();
        double aspect = windowSize.width() * 1.0 / windowSize.height();
        // qDebug() << "window pos:" << x << y;
        // qDebug() << "pick size:" << w << h;

        // Copy frustum and modify it to fit the window size
        pxr::GfFrustum viewFrustum{m_camera->getFrustum()};
        pxr::CameraUtilConformWindow(
            &viewFrustum, pxr::CameraUtilConformWindowPolicy::CameraUtilFit,
            aspect);

        auto pickFrustum = viewFrustum.ComputeNarrowedFrustum(
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
            Q_EMIT primSelected(std::nullopt);
        } else {
            auto hitPrimPath = results[0].hitPrimPath;
            auto prim = m_stage->GetPrimAtPath(hitPrimPath);
            Q_EMIT primSelected(std::make_optional(prim));
        }
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
                m_camera->orbit(-delta.x() * 0.5, -delta.y() * 0.5);
                break;
            case Panning:
                m_camera->pan(-delta.x(), delta.y());
                break;
            case Zooming:
                m_camera->zoom(-delta.y());
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
                m_camera->fit(*m_bboxToDraw);
                update();
            }
            break;
        }

        case Qt::Key_A: {
            // Focus to all
            auto bbox = m_bboxCache.ComputeWorldBound(m_stage->GetPseudoRoot());
            m_camera->fit(bbox);
            update();
            break;
        }
    }
}

void StageViewWindow::dropEvent(QDropEvent *event) {
    if (!event->mimeData()->hasUrls()) {
        return;
    }

    auto filePath = event->mimeData()->urls()[0].toLocalFile();
    for (auto s : Settings::usdFileExts) {
        if (filePath.endsWith(s)) {
            m_stage = pxr::UsdStage::Open(filePath.toStdString());
            initializeRenderEngine();

            m_bboxCache.Clear();
            auto bbox = m_bboxCache.ComputeWorldBound(m_stage->GetPseudoRoot());
            m_camera->fit(bbox);

            Q_EMIT stageOpened(m_stage);
            update();
            event->accept();
            return;
        }
    }
}

void StageViewWindow::onPrimSelected(const std::optional<pxr::UsdPrim> &prim) {
    if (prim) {
        auto path = prim->GetPath();

        m_engine->SetSelected(pxr::SdfPathVector{path});
        m_bboxToDraw = std::make_unique<pxr::GfBBox3d>(
            m_bboxCache.ComputeWorldBound(*prim));
    } else {
        m_engine->ClearSelected();
        m_bboxToDraw = nullptr;
    }
    update();
}

StageViewWidget::StageViewWidget(QWidget *parent) : QWidget(parent) {
    setAcceptDrops(true);

    m_stageViewWindow = new StageViewWindow();
    m_container = createWindowContainer(m_stageViewWindow, this);

    setLayout(new QVBoxLayout());
    layout()->addWidget(m_container);
    layout()->setContentsMargins(0, 0, 0, 0);

    connect(m_stageViewWindow, &StageViewWindow::stageOpened, this,
            &StageViewWidget::stageOpened);
    connect(m_stageViewWindow, &StageViewWindow::primSelected, this,
            &StageViewWidget::primSelected);
}

void StageViewWidget::onPrimSelected(const std::optional<pxr::UsdPrim> &prim) {
    m_stageViewWindow->onPrimSelected(prim);
}