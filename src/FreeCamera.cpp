#include "FreeCamera.h"

#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/vec3f.h>

#include <algorithm>

FreeCamera::FreeCamera() {
    auto frustum = pxr::GfFrustum();
    frustum.SetPerspective(m_fovHeight, m_aspectRatio, m_nearClip, m_farClip);
    m_projectionMatrix = frustum.ComputeProjectionMatrix();
    m_viewMatrix.SetLookAt(m_eyePoint, m_centerPoint, m_upDirection);
}

FreeCamera::~FreeCamera() = default;

pxr::GfMatrix4d FreeCamera::GetViewMatrix() const { return m_viewMatrix; }

void FreeCamera::SetViewMatrix(const pxr::GfMatrix4d& viewMatrix) {
    m_viewMatrix = viewMatrix;
}

void FreeCamera::SetViewMatrix(const pxr::GfVec3d& eyePoint,
                               const pxr::GfVec3d& centerPoint,
                               const pxr::GfVec3d& upDirection) {
    m_eyePoint = eyePoint;
    m_centerPoint = centerPoint;
    m_upDirection = upDirection;
    m_viewMatrix.SetLookAt(m_eyePoint, m_centerPoint, m_upDirection);
}

pxr::GfMatrix4d FreeCamera::GetProjectionMatrix() const {
    return m_projectionMatrix;
}

void FreeCamera::SetProjectionMatrix(const pxr::GfMatrix4d& projectionMatrix) {
    m_projectionMatrix = projectionMatrix;
}

void FreeCamera::SetPerspective(double fovHeight, double aspectRatio,
                                double nearClip, double farClip) {
    m_fovHeight = fovHeight;
    m_aspectRatio = aspectRatio;
    m_nearClip = nearClip;
    m_farClip = farClip;

    auto frustum = pxr::GfFrustum();
    frustum.SetPerspective(m_fovHeight, m_aspectRatio, m_nearClip, m_farClip);
    m_projectionMatrix = frustum.ComputeProjectionMatrix();
}

FreeCamera& FreeCamera::Pan(const double deltaX, const double deltaY) {
    // Calculate right and up vectors
    pxr::GfVec3d forward = m_centerPoint - m_eyePoint;
    pxr::GfVec3d up = m_upDirection.GetNormalized();
    pxr::GfVec3d right = pxr::GfCross(forward, up).GetNormalized();

    // Update eye and center points
    m_eyePoint += right * deltaX + up * deltaY;
    m_centerPoint += right * deltaX + up * deltaY;

    m_viewMatrix.SetLookAt(m_eyePoint, m_centerPoint, m_upDirection);
    return *this;
}

FreeCamera& FreeCamera::Orbit(const double deltaX, const double deltaY) {
    pxr::GfMatrix4d rotX = pxr::GfMatrix4d().SetRotate(
        pxr::GfRotation(pxr::GfVec3f(0, 0, 1), deltaX));

    m_eyePoint = (pxr::GfMatrix4d().SetTranslate(-m_centerPoint) * rotX *
                  pxr::GfMatrix4d().SetTranslate(m_centerPoint))
                     .Transform(m_eyePoint);
    m_upDirection = rotX.Transform(m_upDirection);

    pxr::GfVec3d forward = m_centerPoint - m_eyePoint;
    pxr::GfVec3d up = m_upDirection.GetNormalized();
    pxr::GfVec3d right = pxr::GfCross(forward, up).GetNormalized();
    pxr::GfMatrix4d rotY =
        pxr::GfMatrix4d().SetRotate(pxr::GfRotation(right, deltaY));

    m_eyePoint = (pxr::GfMatrix4d().SetTranslate(-m_centerPoint) * rotY *
                  pxr::GfMatrix4d().SetTranslate(m_centerPoint))
                     .Transform(m_eyePoint);
    m_upDirection = rotY.Transform(m_upDirection);

    m_viewMatrix.SetLookAt(m_eyePoint, m_centerPoint, m_upDirection);
    return *this;
}

FreeCamera& FreeCamera::Zoom(const double deltaDistance) {
    pxr::GfVec3d direction = (m_centerPoint - m_eyePoint).GetNormalized();
    double max_movement = (m_centerPoint - m_eyePoint).GetLength() * 0.1;
    m_eyePoint += direction * std::min(deltaDistance, max_movement);

    m_viewMatrix.SetLookAt(m_eyePoint, m_centerPoint, m_upDirection);
    return *this;
}