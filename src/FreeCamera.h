#pragma once

#include <pxr/base/gf/frustum.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/vec3d.h>

#include <QObject>

class FreeCamera : public QObject {
    Q_OBJECT

   public:
    FreeCamera();
    ~FreeCamera() override;

    pxr::GfMatrix4d GetViewMatrix() const;
    void SetViewMatrix(const pxr::GfMatrix4d& viewMatrix);
    void SetViewMatrix(const pxr::GfVec3d& eyePoint,
                       const pxr::GfVec3d& centerPoint,
                       const pxr::GfVec3d& upDirection);

    pxr::GfMatrix4d GetProjectionMatrix() const;
    void SetProjectionMatrix(const pxr::GfMatrix4d& projectionMatrix);
    void SetPerspective(double fovHeight, double aspectRatio, double nearClip,
                        double farClip);

    FreeCamera& Pan(const double deltaX, const double deltaY);
    FreeCamera& Orbit(const double deltaX, const double deltaY);
    FreeCamera& Zoom(const double deltaDistance);

   private:
    double m_fovHeight = 45.0f;
    double m_aspectRatio = 1.77778f;
    double m_nearClip = 0.1f;
    double m_farClip = 1000.0f;
    pxr::GfMatrix4d m_projectionMatrix;

    pxr::GfVec3d m_eyePoint = pxr::GfVec3d(0, -1, 0);
    pxr::GfVec3d m_centerPoint = pxr::GfVec3d(0, 0, 0);
    pxr::GfVec3d m_upDirection = pxr::GfVec3d(0, 0, 1);
    pxr::GfMatrix4d m_viewMatrix;
};