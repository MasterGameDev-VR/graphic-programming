#pragma once

#include <math/math_utils.h>

namespace xtest {
namespace camera {

	class Camera
	{
	public:

		Camera();
		virtual ~Camera() {}

		virtual DirectX::XMFLOAT3 GetPosition() const = 0;
		virtual DirectX::XMMATRIX GetViewMatrix() const = 0;
		
		DirectX::XMMATRIX GetProjectionMatrix() const;
		DirectX::XMFLOAT3 GetXAxis() const;
		DirectX::XMFLOAT3 GetYAxis() const;
		DirectX::XMFLOAT3 GetZAxis() const;

		void SetPerspectiveProjection(float viewYAngleRadians, float aspectRatio, float nearPlaneZ, float farPlaneZ);
		void SetOrthographicProjection(float width, float height, float nearPlaneZ, float farPlaneZ);

	private:
		
		DirectX::XMFLOAT4X4 m_projMatrix;

	};


} // camera
} // xtest