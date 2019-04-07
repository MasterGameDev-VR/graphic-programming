#pragma once
#include<mesh/mesh_format.h>
#include <application/directx_app.h>
#include <camera/spherical_camera.h>
#include <mesh/mesh_generator.h>


using namespace xtest;
using namespace xtest::mesh;

class threeDC : public application::DirectxApp
{
public:
	struct VertexIn {
		MeshData::Vertex vertex;
		DirectX::XMFLOAT4 color;
	};

	struct PerObjectCB
	{
		DirectX::XMFLOAT4X4 WVP;
	};


	threeDC(HINSTANCE instance, const application::WindowSettings& windowSettings, const application::DirectxSettings& directxSettings, uint32 fps = 60);
	~threeDC();

	
	threeDC(threeDC&&) = delete;
	threeDC(const threeDC&) = delete;
	threeDC& operator=(threeDC&&) = delete;
	threeDC& operator=(const threeDC&) = delete;
	

	//era una prova, per prendere confidenza
	//MeshData ciao;

	void Init();
	void UpdateScene(float ) override;
	void RenderScene() override;

private:
	
	void InitMatrices();

	void InitShaders();
	void InitBuffers();
	void InitRasterizerState();

	void BuildIndicesArray(MeshData&, uint32*);
	void BuildVerticesArray(MeshData&, DirectX::XMFLOAT4, VertexIn*);

	DirectX::XMFLOAT4X4 m_viewMatrix;
	DirectX::XMFLOAT4X4 m_worldMatrix;
	DirectX::XMFLOAT4X4 m_projectionMatrix;

	camera::SphericalCamera m_camera;


	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBufferPlane;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBufferPlane;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBufferBox;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBufferBox;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBufferSphere;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBufferSphere;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vsConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
};

