#pragma once

#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/gf/frustum.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/rotation.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/usd/usd/common.h>
#include <pxr/usd/usd/prim.h>
#include <qobject.h>
#include <qtmetamacros.h>

#include <QObject>
#include <QPropertyAnimation>
#include <QVector3D>

struct CameraView {
    pxr::GfVec3d position;
    double viewDistance;
};

QVariant cameraViewInterpolator(const CameraView& start, const CameraView& end,
                                qreal progress);

class FreeCamera : public QObject {
    Q_OBJECT
    Q_PROPERTY(CameraView cameraView READ cameraView WRITE setCameraView)

   public:
    FreeCamera(QObject* parent = nullptr);
    ~FreeCamera() override;

    pxr::GfMatrix4d getViewMatrix() const;
    pxr::GfMatrix4d getProjectionMatrix() const;
    const pxr::GfFrustum& getFrustum() const;

    CameraView cameraView() const;
    void setCameraView(CameraView value);

    FreeCamera& orbit(const double deltaX, const double deltaY);
    FreeCamera& pan(const double deltaX, const double deltaY);
    FreeCamera& zoom(const double deltaDistance);

    FreeCamera& fit(const pxr::GfBBox3d bbox);

   Q_SIGNALS:
    void viewUpdated();

   private:
    pxr::GfFrustum m_frustum;
    QPropertyAnimation* m_fit_animation;
};