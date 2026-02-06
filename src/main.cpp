#include <qapplication.h>
#include <qsurfaceformat.h>

#include <QVariantAnimation>

#include "FreeCamera.h"
#include "StageViewWindow.h"

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

    StageViewWindow window;
    window.resize(800, 600);
    window.show();

    return app.exec();
}