#pragma once

#include <math/math_utils.h>

namespace xtest {
namespace camera {

	class SphericalCamera
	{
	public:

		SphericalCamera(
			float polarAngleRadians,
			float azimuthAngleRadians,
			float radius,
			const DirectX::XMFLOAT3& pivot,
			const DirectX::XMFLOAT3& alignUpDir = DirectX::XMFLOAT3(0.f, 1.f, 0.f),
			const math::ClampValues<float>& polarAngleLimits = math::ClampValues<float>(math::ToRadians(1.f), math::ToRadians(179.f)),
			const math::ClampValues<float>& radiusLimits = math::ClampValues<float>(1.f, 1000.f)
		);

		SphericalCamera(SphericalCamera&&) = default;
		SphericalCamera(const SphericalCamera&) = default;
		SphericalCamera& operator=(SphericalCamera&&) = default;
		SphericalCamera& operator=(const SphericalCamera&) = default;


		void RotateBy(float deltaPolarAngle, float deltaAzimuthAngle);
		void IncreaseRadiusBy(float deltaRadiusLength);
		void TranslatePivotBy(const DirectX::XMFLOAT3& translation);
		void SetRotation(float polarAngle, float azimuthAngle);
		void SetRadius(float radiusLength);
		void SetPivot(const DirectX::XMFLOAT3& pivot);

		DirectX::XMFLOAT3 GetPosition() const;
		DirectX::XMMATRIX GetViewMatrix() const;
		DirectX::XMFLOAT3 GetXAxis() const;
 		DirectX::XMFLOAT3 GetYAxis() const;
 		DirectX::XMFLOAT3 GetZAxis() const;
		float GetPolarAngle() const;
		float GetAzimuthAngle() const;
		float GetRadius() const;

	private:
		
		float m_polarAngle;
		float m_azimuthAngle;
		float m_radius;
		math::ClampValues<float> m_polarAngleLimits;
		math::ClampValues<float> m_radiusLimits;
		DirectX::XMFLOAT3 m_pivot;
		DirectX::XMFLOAT3 m_upDir;
	};


} // camera
} // xtest