#include "FreeCamera.h"

#include <pxr/base/gf/frustum.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/range1d.h>
#include <pxr/base/gf/range2d.h>
#include <pxr/base/gf/rotation.h>
#include <pxr/base/gf/vec2d.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/vec3f.h>
#include <qeasingcurve.h>
#include <qobject.h>
#include <qpropertyanimation.h>
#include <qtmetamacros.h>
#include <qvariant.h>
#include <qvectornd.h>

#include <iostream>

QVariant cameraViewInterpolator(const CameraView& start, const CameraView& end,
                                qreal progress) {
    auto pos = start.position + ((end.position - start.position) * progress);
    auto dis = start.viewDistance +
               ((end.viewDistance - start.viewDistance) * progress);

    CameraView cameraView{pos, dis};
    return QVariant::fromValue(cameraView);
}

FreeCamera::FreeCamera(QObject* parent) : QObject(parent) {
    m_frustum = pxr::GfFrustum(
        pxr::GfVec3d(0, -10, 5), pxr::GfRotation(pxr::GfVec3d(1, 0, 0), 60),
        pxr::GfRange2d(pxr::GfVec2d(-1, -1), pxr::GfVec2d(1, 1)),
        pxr::GfRange1d(0.1, 1000.0), pxr::GfFrustum::Perspective, 10.0);
    m_frustum.SetPerspective(45.0, 16.0 / 9.0, 0.1, 1000.0);

    m_fit_animation = new QPropertyAnimation(this, "cameraView", this);
    m_fit_animation->setEasingCurve(QEasingCurve::InOutQuad);
}

FreeCamera::~FreeCamera() = default;

pxr::GfMatrix4d FreeCamera::getViewMatrix() const {
    return m_frustum.ComputeViewMatrix();
}

pxr::GfMatrix4d FreeCamera::getProjectionMatrix() const {
    return m_frustum.ComputeProjectionMatrix();
}

const pxr::GfFrustum& FreeCamera::getFrustum() const { return m_frustum; }

CameraView FreeCamera::cameraView() const {
    return CameraView{m_frustum.GetPosition(), m_frustum.GetViewDistance()};
}

void FreeCamera::setCameraView(CameraView value) {
    m_frustum.SetPosition(value.position);
    m_frustum.SetViewDistance(value.viewDistance);
    Q_EMIT viewUpdated();
}

FreeCamera& FreeCamera::orbit(const double deltaX, const double deltaY) {
    pxr::GfVec3d lookAtPoint = m_frustum.ComputeLookAtPoint();
    pxr::GfMatrix4d translate = pxr::GfMatrix4d().SetTranslate(lookAtPoint);
    double factor = 0.5;

    m_frustum.Transform(translate.GetInverse() *
                        pxr::GfMatrix4d().SetRotate(pxr::GfRotation(
                            pxr::GfVec3f(0, 0, 1), deltaX * factor)) *
                        translate);

    pxr::GfVec3d side;
    pxr::GfVec3d up;
    pxr::GfVec3d view;
    m_frustum.ComputeViewFrame(&side, &up, &view);

    m_frustum.Transform(
        translate.GetInverse() *
        pxr::GfMatrix4d().SetRotate(pxr::GfRotation(side, deltaY * factor)) *
        translate);

    return *this;
}

FreeCamera& FreeCamera::pan(const double deltaX, const double deltaY) {
    pxr::GfVec3d side;
    pxr::GfVec3d up;
    pxr::GfVec3d view;
    m_frustum.ComputeViewFrame(&side, &up, &view);

    double factor = m_frustum.GetViewDistance() * 0.002;
    auto moveX = pxr::GfMatrix4d().SetTranslate(side * deltaX * factor);
    auto moveY = pxr::GfMatrix4d().SetTranslate(up * deltaY * factor);
    m_frustum.Transform(moveX * moveY);

    return *this;
}

FreeCamera& FreeCamera::zoom(const double deltaDistance) {
    auto viewDirection = m_frustum.ComputeViewDirection();
    double distance = m_frustum.GetViewDistance();
    double factor = distance * 0.01;

    if (distance - deltaDistance > 0.1) {
        m_frustum.Transform(pxr::GfMatrix4d().SetTranslate(
            viewDirection * deltaDistance * factor));
        m_frustum.SetViewDistance(distance - deltaDistance * factor);
    }

    return *this;
}

FreeCamera& FreeCamera::fit(const pxr::GfBBox3d bbox) {
    auto box = bbox.GetBox();
    auto size = box.GetSize().GetArray();

    auto center = bbox.ComputeCentroid();
    auto radius = pxr::GfMax(size[0], size[1], size[2]);
    auto origNearFar = m_frustum.GetNearFar();

    pxr::GfFrustum f_frustum(m_frustum);
    f_frustum.FitToSphere(center, radius, 1);

    m_fit_animation->setDuration(200);
    m_fit_animation->setStartValue(QVariant::fromValue(
        CameraView{m_frustum.GetPosition(), m_frustum.GetViewDistance()}));
    m_fit_animation->setEndValue(QVariant::fromValue(
        CameraView{f_frustum.GetPosition(), f_frustum.GetViewDistance()}));
    m_fit_animation->start();

    return *this;
}