#include "stdafx.h"
#include "camera.h"

using namespace xtest::camera;

Camera::Camera()
	: m_projMatrix()
{}


DirectX::XMMATRIX Camera::GetProjectionMatrix() const
{
	return XMLoadFloat4x4(&m_projMatrix);
}

DirectX::XMFLOAT3 Camera::GetXAxis() const
{
	DirectX::XMMATRIX Vt = XMMatrixTranspose(GetViewMatrix());

	DirectX::XMFLOAT3 xAxis;
	XMStoreFloat3(&xAxis, Vt.r[0]);
	return xAxis;
}

DirectX::XMFLOAT3 Camera::GetYAxis() const
{
	DirectX::XMMATRIX Vt = XMMatrixTranspose(GetViewMatrix());

	DirectX::XMFLOAT3 yAxis;
	XMStoreFloat3(&yAxis, Vt.r[1]);
	return yAxis;
}

DirectX::XMFLOAT3 Camera::GetZAxis() const
{
	DirectX::XMMATRIX Vt = XMMatrixTranspose(GetViewMatrix());

	DirectX::XMFLOAT3 zAxis;
	XMStoreFloat3(&zAxis, Vt.r[2]);
	return zAxis;
}

float Camera::GetYFov() const {
	return m_fov_y;
}

float Camera::GetZFarPlane() const {
	return m_far_z;
}

float xtest::camera::Camera::GetAspectRatio() const
{
	return m_aspect_ratio;
}

void Camera::SetPerspectiveProjection(float viewYAngleRadians, float aspectRatio, float nearPlaneZ, float farPlaneZ)
{
	m_fov_y = viewYAngleRadians;
	m_far_z = farPlaneZ;
	m_near_z = nearPlaneZ;
	m_aspect_ratio = aspectRatio;
	XMStoreFloat4x4(&m_projMatrix, DirectX::XMMatrixPerspectiveFovLH(m_fov_y, aspectRatio, nearPlaneZ, farPlaneZ));
}

void Camera::SetOrthographicProjection(float width, float height, float nearPlaneZ, float farPlaneZ)
{
	XMStoreFloat4x4(&m_projMatrix, DirectX::XMMatrixOrthographicLH(width, height, nearPlaneZ, farPlaneZ));
}

