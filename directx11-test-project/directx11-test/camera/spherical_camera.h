#pragma once

#include <math/math_utils.h>
#include <camera/camera.h>

namespace xtest {
namespace camera {

	// basic spherical camera implementation
	class SphericalCamera : public Camera
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


		void RotateBy(float deltaPolarAngle, float deltaAzimuthAngle);
		void IncreaseRadiusBy(float deltaRadiusLength);
		void TranslatePivotBy(const DirectX::XMFLOAT3& translation);
		void SetRotation(float polarAngle, float azimuthAngle);
		void SetRadius(float radiusLength);
		void SetPivot(const DirectX::XMFLOAT3& pivot);

		virtual DirectX::XMFLOAT3 GetPosition() const override;
		virtual DirectX::XMMATRIX GetViewMatrix() const override;
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