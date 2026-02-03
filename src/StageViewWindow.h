#pragma once

#include <pxr/usd/usd/common.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include <pxr/usdImaging/usdImagingGL/renderParams.h>
#include <qopengldebug.h>
#include <qopenglfunctions.h>
#include <qopenglwindow.h>
#include <qwindow.h>

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPointF>
#include <QWheelEvent>

#include "FreeCamera.h"

class StageViewWindow : public QOpenGLWindow, protected QOpenGLFunctions {
    Q_OBJECT

   public:
    StageViewWindow();
    ~StageViewWindow() override;

   protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

   private:
    pxr::UsdImagingGLEngine *m_engine;
    pxr::UsdImagingGLRenderParams m_renderParams;
    pxr::UsdStageRefPtr m_stage;
    FreeCamera m_camera;

   protected:
    void closeEvent(QCloseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

   private:
    enum NavigateType {
        Orbiting,
        Panning,
        Zooming,
    };

    QPointF m_startPos;
    NavigateType m_navigateType;
    bool m_isMoving = false;

   private:
    QOpenGLDebugLogger *m_debugLogger;
};