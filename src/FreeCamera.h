#pragma once

#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/gf/frustum.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/usd/usd/common.h>
#include <pxr/usd/usd/prim.h>

#include <QObject>

class FreeCamera : public QObject {
    Q_OBJECT

   public:
    FreeCamera();
    ~FreeCamera() override;

    pxr::GfMatrix4d GetViewMatrix() const;
    pxr::GfMatrix4d GetProjectionMatrix() const;
    const pxr::GfFrustum& GetFrustum() const;

    FreeCamera& Orbit(const double deltaX, const double deltaY);
    FreeCamera& Pan(const double deltaX, const double deltaY);
    FreeCamera& Zoom(const double deltaDistance);

    FreeCamera& Fit(const pxr::GfBBox3d bbox);

   private:
    pxr::GfFrustum m_frustum;
};