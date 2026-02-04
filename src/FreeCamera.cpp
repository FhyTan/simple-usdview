#include "FreeCamera.h"

#include <pxr/base/gf/frustum.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/range1d.h>
#include <pxr/base/gf/range2d.h>
#include <pxr/base/gf/rotation.h>
#include <pxr/base/gf/vec2d.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/vec3f.h>

FreeCamera::FreeCamera() {
    m_frustum = pxr::GfFrustum(
        pxr::GfVec3d(0, -10, 5), pxr::GfRotation(pxr::GfVec3d(1, 0, 0), 60),
        pxr::GfRange2d(pxr::GfVec2d(-1, -1), pxr::GfVec2d(1, 1)),
        pxr::GfRange1d(0.1, 1000.0), pxr::GfFrustum::Perspective, 10.0);
    m_frustum.SetPerspective(45.0, 16.0 / 9.0, 0.1, 1000.0);
}

FreeCamera::~FreeCamera() = default;

pxr::GfMatrix4d FreeCamera::GetViewMatrix() const {
    return m_frustum.ComputeViewMatrix();
}

pxr::GfMatrix4d FreeCamera::GetProjectionMatrix() const {
    return m_frustum.ComputeProjectionMatrix();
}

const pxr::GfFrustum& FreeCamera::GetFrustum() const { return m_frustum; }

FreeCamera& FreeCamera::Orbit(const double deltaX, const double deltaY) {
    pxr::GfVec3d lookAtPoint = m_frustum.ComputeLookAtPoint();
    pxr::GfMatrix4d translate = pxr::GfMatrix4d().SetTranslate(lookAtPoint);

    m_frustum.Transform(translate.GetInverse() *
                        pxr::GfMatrix4d().SetRotate(
                            pxr::GfRotation(pxr::GfVec3f(0, 0, 1), deltaX)) *
                        translate);

    pxr::GfVec3d side;
    pxr::GfVec3d up;
    pxr::GfVec3d view;
    m_frustum.ComputeViewFrame(&side, &up, &view);

    m_frustum.Transform(
        translate.GetInverse() *
        pxr::GfMatrix4d().SetRotate(pxr::GfRotation(side, deltaY)) * translate);

    return *this;
}

FreeCamera& FreeCamera::Pan(const double deltaX, const double deltaY) {
    pxr::GfVec3d side;
    pxr::GfVec3d up;
    pxr::GfVec3d view;
    m_frustum.ComputeViewFrame(&side, &up, &view);

    auto moveX = pxr::GfMatrix4d().SetTranslate(side * deltaX);
    auto moveY = pxr::GfMatrix4d().SetTranslate(up * deltaY);
    m_frustum.Transform(moveX * moveY);

    return *this;
}

FreeCamera& FreeCamera::Zoom(const double deltaDistance) {
    auto viewDirection = m_frustum.ComputeViewDirection();
    double distance = m_frustum.GetViewDistance();

    if (distance - deltaDistance > 0.1) {
        m_frustum.Transform(
            pxr::GfMatrix4d().SetTranslate(viewDirection * deltaDistance));
    }

    return *this;
}

FreeCamera& FreeCamera::FitToPrim(const pxr::UsdPrim& prim) { return *this; }
FreeCamera& FreeCamera::FitToAll(const pxr::UsdStagePtr& stage) {
    return *this;
}