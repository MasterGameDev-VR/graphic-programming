#include "stdafx.h"
#include "spherical_camera.h"


using namespace xtest;
using namespace xtest::camera;
using namespace DirectX;

SphericalCamera::SphericalCamera(
	float polarAngleRadians, 
	float azimuthAngleRadians, 
	float radius, 
	const XMFLOAT3& pivot,
	const XMFLOAT3& alignUpDir /*= DirectX::XMFLOAT3(0.f, 1.f, 0.f)*/,
	const math::ClampValues<float>& polarAngleLimits /*= math::ClampValues<float>(math::ToRadians(1.f), math::ToRadians(179.f))*/,
	const math::ClampValues<float>& radiusLimits /*= math::ClampValues<float>(1.f, 100.f) */)
{
	XTEST_ASSERT(polarAngleRadians > 0.f);
	XTEST_ASSERT(azimuthAngleRadians > 0.f);
	XTEST_ASSERT(radius > 0.f);

	m_polarAngle = polarAngleRadians;
	m_azimuthAngle = azimuthAngleRadians;
	m_radius = radius;
	m_polarAngleLimits = polarAngleLimits;
	m_radiusLimits = radiusLimits;

	m_pivot = pivot;
	m_upDir = alignUpDir;

}


void SphericalCamera::RotateBy(float deltaPolarAngle, float deltaAzimuthAngle)
{
	m_polarAngle = math::Clamp(m_polarAngle + deltaPolarAngle, m_polarAngleLimits);
	m_azimuthAngle = std::fmod(m_azimuthAngle + deltaAzimuthAngle, DirectX::XM_2PI);
}

void SphericalCamera::IncreaseRadiusBy(float deltaRadiusLength)
{
	m_radius = math::Clamp(m_radius + deltaRadiusLength, m_radiusLimits);
}

void SphericalCamera::TranslatePivotBy(const XMFLOAT3& translation)
{
	XMVECTOR pivot = XMVectorAdd(XMLoadFloat3(&m_pivot), XMLoadFloat3(&translation));
	XMStoreFloat3(&m_pivot, pivot);
}

XMFLOAT3 SphericalCamera::GetPosition() const
{
	XMVECTOR posL = XMVectorSet(
		std::sinf(m_polarAngle) * std::sinf(m_azimuthAngle) * m_radius,
		std::cosf(m_polarAngle) * m_radius,
		std::sinf(m_polarAngle) * std::cos(m_azimuthAngle) * m_radius,
		1
	);

	XMFLOAT3 posW ;
    XMStoreFloat3(&posW, XMVectorAdd(posL, XMLoadFloat3(&m_pivot)));
	return posW;
}

XMMATRIX SphericalCamera::GetViewMatrix() const
{
	XMFLOAT3 position = GetPosition();

	return XMMatrixLookAtLH(
		XMVectorSet(position.x, position.y, position.z, 1.f), 
		XMVectorSet(m_pivot.x, m_pivot.y, m_pivot.z, 1.f), 
		XMVectorSet(m_upDir.x, m_upDir.y, m_upDir.z, 0.f)
	);
}

XMFLOAT3 SphericalCamera::GetXAxis() const
{
	XMMATRIX Vt = XMMatrixTranspose(GetViewMatrix());

	XMFLOAT3 xAxis;
	XMStoreFloat3(&xAxis, Vt.r[0]);
	return xAxis;
}

XMFLOAT3 SphericalCamera::GetYAxis() const
{
	XMMATRIX Vt = XMMatrixTranspose(GetViewMatrix());

	XMFLOAT3 yAxis;
	XMStoreFloat3(&yAxis, Vt.r[1]);
	return yAxis;
}

XMFLOAT3 SphericalCamera::GetZAxis() const
{
	XMFLOAT3 position_float3 = GetPosition();
	
	XMVECTOR zAxis = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&m_pivot), XMLoadFloat3(&position_float3)));
	XMFLOAT3 zAxis_float3;
	XMStoreFloat3(&zAxis_float3, zAxis);

	return zAxis_float3;
}

float SphericalCamera::GetPolarAngle() const
{
	return m_polarAngle;
}

float SphericalCamera::GetAzimuthAngle() const
{
	return m_azimuthAngle;
}

float SphericalCamera::GetRadius() const
{
	return m_radius;
}

void SphericalCamera::SetRotation(float polarAngle, float azimuthAngle)
{
	m_polarAngle = math::Clamp(polarAngle, m_polarAngleLimits);
	m_azimuthAngle = std::fmod(azimuthAngle, DirectX::XM_2PI);
}

void SphericalCamera::SetRadius(float radiusLength)
{
	m_radius = math::Clamp(radiusLength, m_radiusLimits);
}

void SphericalCamera::SetPivot(const XMFLOAT3& pivotW)
{
	m_pivot = pivotW;
}
