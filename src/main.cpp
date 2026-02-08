#include <pxr/usd/usd/stage.h>
#include <qapplication.h>
#include <qsurfaceformat.h>

#include <QVariantAnimation>

#include "FreeCamera.h"
#include "MainWindow.h"
#include "Outliner.h"
#include "StageViewWidget.h"


int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    qRegisterAnimationInterpolator<CameraView>(cameraViewInterpolator);

    QSurfaceFormat fmt;
    fmt.setVersion(4, 5);
    fmt.setProfile(QSurfaceFormat::CompatibilityProfile);
    fmt.setDepthBufferSize(24);
    fmt.setSamples(8);
    fmt.setOption(QSurfaceFormat::DebugContext);
    QSurfaceFormat::setDefaultFormat(fmt);

    // StageViewWidget stageViewWidget;
    // stageViewWidget.resize(800, 600);
    // stageViewWidget.show();

    // Outliner outliner;
    // outliner.setUsdStage(pxr::UsdStage::Open("display.usda"));
    // outliner.show();

    MainWindow window;
    window.resize(800, 600);
    window.show();

    return app.exec();
}