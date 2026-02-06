#pragma once

#include <pxr/base/gf/bbox3d.h>
#include <pxr/usd/usd/common.h>
#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include <pxr/usdImaging/usdImagingGL/renderParams.h>
#include <qevent.h>
#include <qopengldebug.h>
#include <qopenglfunctions.h>
#include <qopenglwindow.h>
#include <qwindow.h>

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPointF>
#include <QWheelEvent>
#include <memory>

#include "FreeCamera.h"

class StageViewWindow : public QOpenGLWindow, protected QOpenGLFunctions {
    Q_OBJECT

   public:
    StageViewWindow();
    ~StageViewWindow() override;

   private:
    enum NavigateType {
        Orbiting,
        Panning,
        Zooming,
    };

   protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void closeEvent(QCloseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

   private:
    pxr::UsdImagingGLEngine *m_engine;
    pxr::UsdImagingGLRenderParams m_renderParams;
    pxr::UsdStageRefPtr m_stage;

    pxr::UsdGeomBBoxCache m_bboxCache;
    std::unique_ptr<pxr::GfBBox3d> m_bboxToDraw = nullptr;

    FreeCamera *m_camera;
    QPointF m_startPos;
    NavigateType m_navigateType;
    bool m_isMoving = false;

    QOpenGLDebugLogger *m_debugLogger;
};