
#include "stdafx.h"
#include "lights_demo_app.h"
#include <file/file_utils.h>
#include <math/math_utils.h>
#include <service/locator.h>
#include <external_libs/directxtk/WICTextureLoader.h>

using namespace DirectX;
using namespace xtest;

using xtest::demo::LightsDemoApp;
using Microsoft::WRL::ComPtr;

LightsDemoApp::LightsDemoApp(HINSTANCE instance,
	const application::WindowSettings& windowSettings,
	const application::DirectxSettings& directxSettings,
	uint32 fps /*=60*/)
	: application::DirectxApp(instance, windowSettings, directxSettings, fps)
	, m_viewMatrix()
	, m_projectionMatrix()
	, m_camera(math::ToRadians(68.f), math::ToRadians(135.f), 7.f, { 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, { math::ToRadians(4.f), math::ToRadians(175.f) }, { 3.f, 25.f })
	, m_dirLight()
	, m_spotLights()
	, m_pointLights()
	, m_sphere()
	, m_torus()
	, m_box()
	, m_plane()
	, m_crate()
	, m_lightsControl()
	, m_isLightControlDirty(true)
	, m_stopLights(false)
	, m_texturesControl()
	, m_isTexturesControlDirty(true)
	, m_d3dPerFrameCB(nullptr)
	//, m_d3dPerFrameTextureCB(nullptr)
	, m_d3dRarelyChangedCB(nullptr)
	, m_d3dRarelyChangedTextureCB(nullptr)
	, m_vertexShader(nullptr)
	, m_pixelShader(nullptr)
	, m_inputLayout(nullptr)
	, m_rasterizerState(nullptr)
{}


LightsDemoApp::~LightsDemoApp()
{}


void LightsDemoApp::Init()
{
	application::DirectxApp::Init();

	m_d3dAnnotation->BeginEvent(L"init-demo");

	InitMatrices();
	InitShaders();
	InitRenderable();
	InitSamplerState();
	InitLights();
	InitTexturesControls();
	InitRasterizerState();

	service::Locator::GetMouse()->AddListener(this);
	service::Locator::GetKeyboard()->AddListener(this, { input::Key::F, input::Key::F1, input::Key::F2, input::Key::F3,input::Key::F4, input::Key::F5, input::Key::F6, input::Key::space_bar });

	m_d3dAnnotation->EndEvent();
}


void LightsDemoApp::InitMatrices()
{
	// view matrix
	XMStoreFloat4x4(&m_viewMatrix, m_camera.GetViewMatrix());

	// projection matrix
	{
		XMMATRIX P = XMMatrixPerspectiveFovLH(math::ToRadians(45.f), AspectRatio(), 1.f, 1000.f);
		XMStoreFloat4x4(&m_projectionMatrix, P);
	}
}


void LightsDemoApp::InitShaders()
{
	// read pre-compiled shaders' bytecode
	std::future<file::BinaryFile> psByteCodeFuture = file::ReadBinaryFile(std::wstring(GetRootDir()).append(L"\\lights_demo_PS.cso"));
	std::future<file::BinaryFile> vsByteCodeFuture = file::ReadBinaryFile(std::wstring(GetRootDir()).append(L"\\lights_demo_VS.cso"));

	// future.get() can be called only once
	file::BinaryFile vsByteCode = vsByteCodeFuture.get();
	file::BinaryFile psByteCode = psByteCodeFuture.get();
	XTEST_D3D_CHECK(m_d3dDevice->CreateVertexShader(vsByteCode.Data(), vsByteCode.ByteSize(), nullptr, &m_vertexShader));
	XTEST_D3D_CHECK(m_d3dDevice->CreatePixelShader(psByteCode.Data(), psByteCode.ByteSize(), nullptr, &m_pixelShader));


	// create the input layout, it must match the Vertex Shader HLSL input format:
	//	struct VertexIn
	// 	{
	// 		float3 posL : POSITION;
	// 		float3 normalL : NORMAL;
	// 	};
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(mesh::MeshData::Vertex, normal), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(mesh::MeshData::Vertex, tangentU), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(mesh::MeshData::Vertex, uv), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	XTEST_D3D_CHECK(m_d3dDevice->CreateInputLayout(vertexDesc, 4, vsByteCode.Data(), vsByteCode.ByteSize(), &m_inputLayout));


	// perFrameCB
	D3D11_BUFFER_DESC perFrameCBDesc;
	perFrameCBDesc.Usage = D3D11_USAGE_DYNAMIC;
	perFrameCBDesc.ByteWidth = sizeof(PerFrameCB);
	perFrameCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	perFrameCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	perFrameCBDesc.MiscFlags = 0;
	perFrameCBDesc.StructureByteStride = 0;
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&perFrameCBDesc, nullptr, &m_d3dPerFrameCB));
	//perFrameCBDesc.ByteWidth = sizeof(PerFrameTextureCB);
	//XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&perFrameCBDesc, nullptr, &m_d3dPerFrameTextureCB));



}

void xtest::demo::LightsDemoApp::InitSamplerState() 
{
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc,sizeof(D3D11_SAMPLER_DESC));
	//properties.... to define
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	//samplerDesc.MinLOD = 0;
	XTEST_D3D_CHECK(m_d3dDevice->CreateSamplerState(&samplerDesc, &m_textureSamplerState));

}

void xtest::demo::LightsDemoApp::InitRenderable()
{

	// plane
	{
		// geo
		m_plane.mesh = mesh::GeneratePlane(50.f, 50.f, 50, 50);


		// W
		XMStoreFloat4x4(&m_plane.W, XMMatrixIdentity());


		// material
		m_plane.material.ambient = { 0.15f, 0.15f, 0.15f, 1.f };
		m_plane.material.diffuse = { 0.52f, 0.52f, 0.52f, 1.f };
		m_plane.material.specular = { 0.8f, 0.8f, 0.8f, 190.0f };


		// perObjectCB
		D3D11_BUFFER_DESC perObjectCBDesc;
		perObjectCBDesc.Usage = D3D11_USAGE_DYNAMIC;
		perObjectCBDesc.ByteWidth = sizeof(PerObjectCB);
		perObjectCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		perObjectCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		perObjectCBDesc.MiscFlags = 0;
		perObjectCBDesc.StructureByteStride = 0;
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&perObjectCBDesc, nullptr, &m_plane.d3dPerObjectCB));
		//perObjectCBDesc.ByteWidth = sizeof(PerObjectTextureCB);
		//XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&perObjectCBDesc, nullptr, &m_plane.d3dPerObjectTextureCB));



		// vertex buffer
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.ByteWidth = UINT(sizeof(mesh::MeshData::Vertex) * m_plane.mesh.vertices.size());
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexInitData;
		vertexInitData.pSysMem = &m_plane.mesh.vertices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexInitData, &m_plane.d3dVertexBuffer));


		// index buffer
		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.ByteWidth = UINT(sizeof(uint32) * m_plane.mesh.indices.size());
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexInitdata;
		indexInitdata.pSysMem = &m_plane.mesh.indices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &indexInitdata, &m_plane.d3dIndexBuffer));

		//diffuseTexture
		wchar_t* diffTextFileName = L"D:\\graphic_programming_Colombo\\graphic-programming\\directx11-test-project\\directx11-test\\application\\resources\\data\\3d-objects\\ground\\ground_color.png";
		//discard explicit creation
		//XTEST_D3D_CHECK(CreateWICTextureFromFileEx(m_d3dDevice.Get(),m_d3dContext.Get(), diffTextFileName,0,D3D11_USAGE_DYNAMIC,D3D11_BIND_SHADER_RESOURCE,D3D11_CPU_ACCESS_WRITE,D3D11_RESOURCE_MISC_TILED,0,&m_plane.d3dResourceDiffText, &m_plane.d3dShaderResourceViewDiffText));
		XTEST_D3D_CHECK(CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), diffTextFileName,&m_plane.d3dResourceDiffText, &m_plane.d3dShaderResourceViewDiffText,0Ui64));
		
		//normal map Texture
		wchar_t* normalMapTextFileName = L"D:\\graphic_programming_Colombo\\graphic-programming\\directx11-test-project\\directx11-test\\application\\resources\\data\\3d-objects\\ground\\ground_norm.png";
		XTEST_D3D_CHECK(CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), normalMapTextFileName, &m_plane.d3dResourceNormalMapText, &m_plane.d3dShaderResourceViewNormalMapText, 0Ui64));

		//gloss map Texture
		wchar_t* glossMapTextFileName = L"D:\\graphic_programming_Colombo\\graphic-programming\\directx11-test-project\\directx11-test\\application\\resources\\data\\3d-objects\\ground\\ground_gloss.png";
		XTEST_D3D_CHECK(CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), glossMapTextFileName, &m_plane.d3dResourceGlossMapText, &m_plane.d3dShaderResourceViewGlossMapText, 0Ui64));


	}


	// sphere
	{

		//geo
		m_sphere.mesh = mesh::GenerateSphere(1.f, 40, 40); // mesh::GenerateBox(2, 2, 2);


		// W
		XMStoreFloat4x4(&m_sphere.W, XMMatrixTranslation(-4.f, 1.f, 0.f));

		// material
		m_sphere.material.ambient = { 0.7f, 0.1f, 0.1f, 1.0f };
		m_sphere.material.diffuse = { 0.81f, 0.15f, 0.15f, 1.0f };
		m_sphere.material.specular = { 0.7f, 0.7f, 0.7f, 40.0f };


		// perObjectCB
		D3D11_BUFFER_DESC perObjectCBDesc;
		perObjectCBDesc.Usage = D3D11_USAGE_DYNAMIC;
		perObjectCBDesc.ByteWidth = sizeof(PerObjectCB);
		perObjectCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		perObjectCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		perObjectCBDesc.MiscFlags = 0;
		perObjectCBDesc.StructureByteStride = 0;
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&perObjectCBDesc, nullptr, &m_sphere.d3dPerObjectCB));
		//perObjectCBDesc.ByteWidth = sizeof(PerObjectTextureCB);
		//XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&perObjectCBDesc, nullptr, &m_sphere.d3dPerObjectTextureCB));

		// vertex buffer
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.ByteWidth = UINT(sizeof(mesh::MeshData::Vertex) * m_sphere.mesh.vertices.size());
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexInitData;
		vertexInitData.pSysMem = &m_sphere.mesh.vertices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexInitData, &m_sphere.d3dVertexBuffer));


		// index buffer
		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.ByteWidth = UINT(sizeof(uint32) * m_sphere.mesh.indices.size());
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexInitdata;
		indexInitdata.pSysMem = &m_sphere.mesh.indices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &indexInitdata, &m_sphere.d3dIndexBuffer));


		//diffuseTexture
		wchar_t* diffTextFileName = L"D:\\graphic_programming_Colombo\\graphic-programming\\directx11-test-project\\directx11-test\\application\\resources\\data\\3d-objects\\sci-fi\\sci_fi_color.png";
		//discard explicit creation
		//XTEST_D3D_CHECK(CreateWICTextureFromFileEx(m_d3dDevice.Get(),m_d3dContext.Get(), diffTextFileName,0,D3D11_USAGE_DYNAMIC,D3D11_BIND_SHADER_RESOURCE,D3D11_CPU_ACCESS_WRITE,D3D11_RESOURCE_MISC_TILED,0,&m_plane.d3dResourceDiffText, &m_plane.d3dShaderResourceViewDiffText));
		XTEST_D3D_CHECK(CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), diffTextFileName, &m_sphere.d3dResourceDiffText, &m_sphere.d3dShaderResourceViewDiffText, 0Ui64));


		//normal map Texture
		wchar_t* normalMapTextFileName = L"D:\\graphic_programming_Colombo\\graphic-programming\\directx11-test-project\\directx11-test\\application\\resources\\data\\3d-objects\\sci-fi\\sci_fi_norm.png";
		XTEST_D3D_CHECK(CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), normalMapTextFileName, &m_sphere.d3dResourceNormalMapText, &m_sphere.d3dShaderResourceViewNormalMapText, 0Ui64));
		//gloss map Texture
		wchar_t* glossMapTextFileName = L"D:\\graphic_programming_Colombo\\graphic-programming\\directx11-test-project\\directx11-test\\application\\resources\\data\\3d-objects\\sci-fi\\sci_fi_gloss.png";
		XTEST_D3D_CHECK(CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), glossMapTextFileName, &m_sphere.d3dResourceGlossMapText, &m_sphere.d3dShaderResourceViewGlossMapText, 0Ui64));
		//motion color Texture - diffuse
		//wchar_t* motionDiffTextFileName = L"D:\\graphic_programming_Colombo\\graphic-programming\\directx11-test-project\\directx11-test\\application\\resources\\data\\3d-objects\\twine\\twine_color.png";
		//XTEST_D3D_CHECK(CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), motionDiffTextFileName, &m_sphere.d3dResourceMotionDiffText, &m_sphere.d3dShaderResourceViewMotionDiffText, 0Ui64));

	}
	// torus
	{
		// geo
		m_torus.mesh = mesh::GenerateTorus(1.5f, 5.f, 360, 360);


		// W
		XMStoreFloat4x4(&m_torus.W, XMMatrixTranslation(-9.f,2.f,-8.f));


		// material
		m_torus.material.ambient = { 0.15f, 0.15f, 0.15f, 1.f };
		m_torus.material.diffuse = { 0.52f, 0.52f, 0.52f, 1.f };
		m_torus.material.specular = { 0.8f, 0.8f, 0.8f, 190.0f };

		// perObjectCB
		D3D11_BUFFER_DESC perObjectCBDesc;
		perObjectCBDesc.Usage = D3D11_USAGE_DYNAMIC;
		perObjectCBDesc.ByteWidth = sizeof(PerObjectCB);
		perObjectCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		perObjectCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		perObjectCBDesc.MiscFlags = 0;
		perObjectCBDesc.StructureByteStride = 0;
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&perObjectCBDesc, nullptr, &m_torus.d3dPerObjectCB));
		//perObjectCBDesc.ByteWidth = sizeof(PerObjectTextureCB);
		//XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&perObjectCBDesc, nullptr, &m_torus.d3dPerObjectTextureCB));

		// vertex buffer
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.ByteWidth = UINT(sizeof(mesh::MeshData::Vertex) * m_torus.mesh.vertices.size());
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexInitData;
		vertexInitData.pSysMem = &m_torus.mesh.vertices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexInitData, &m_torus.d3dVertexBuffer));


		// index buffer
		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.ByteWidth = UINT(sizeof(uint32) * m_torus.mesh.indices.size());
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexInitdata;
		indexInitdata.pSysMem = &m_torus.mesh.indices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &indexInitdata, &m_torus.d3dIndexBuffer));


		//diffuseTexture
		wchar_t* diffTextFileName = L"D:\\graphic_programming_Colombo\\graphic-programming\\directx11-test-project\\directx11-test\\application\\resources\\data\\3d-objects\\jeans\\jeans_color.png";
		//discard explicit creation
		//XTEST_D3D_CHECK(CreateWICTextureFromFileEx(m_d3dDevice.Get(),m_d3dContext.Get(), diffTextFileName,0,D3D11_USAGE_DYNAMIC,D3D11_BIND_SHADER_RESOURCE,D3D11_CPU_ACCESS_WRITE,D3D11_RESOURCE_MISC_TILED,0,&m_plane.d3dResourceDiffText, &m_plane.d3dShaderResourceViewDiffText));
		XTEST_D3D_CHECK(CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), diffTextFileName, &m_torus.d3dResourceDiffText, &m_torus.d3dShaderResourceViewDiffText, 0Ui64));

		//normal map Texture
		wchar_t* normalMapTextFileName = L"D:\\graphic_programming_Colombo\\graphic-programming\\directx11-test-project\\directx11-test\\application\\resources\\data\\3d-objects\\jeans\\jeans_norm.png";
		XTEST_D3D_CHECK(CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), normalMapTextFileName, &m_torus.d3dResourceNormalMapText, &m_torus.d3dShaderResourceViewNormalMapText, 0Ui64));
		//gloss map Texture
		wchar_t* glossMapTextFileName = L"D:\\graphic_programming_Colombo\\graphic-programming\\directx11-test-project\\directx11-test\\application\\resources\\data\\3d-objects\\jeans\\jeans_gloss.png";
		XTEST_D3D_CHECK(CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), glossMapTextFileName, &m_torus.d3dResourceGlossMapText, &m_torus.d3dShaderResourceViewGlossMapText, 0Ui64));
		//motion color Texture - diffuse
		//wchar_t* motionDiffTextFileName = L"D:\\graphic_programming_Colombo\\graphic-programming\\directx11-test-project\\directx11-test\\application\\resources\\data\\3d-objects\\twine\\twine_color.png";
		//XTEST_D3D_CHECK(CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), motionDiffTextFileName, &m_torus.d3dResourceMotionDiffText, &m_torus.d3dShaderResourceViewMotionDiffText, 0Ui64));

		
	}
	// box
	{
		// geo
		m_box.mesh = mesh::GenerateBox(4.f, 5.f, 6.f);
		// W
		XMStoreFloat4x4(&m_box.W, XMMatrixTranslation(9.f, 6.f, 8.f));
		// material
		m_box.material.ambient = { 0.15f, 0.15f, 0.15f, 1.f };
		m_box.material.diffuse = { 0.52f, 0.52f, 0.52f, 1.f };
		m_box.material.specular = { 0.8f, 0.8f, 0.8f, 190.0f };
		// perObjectCB
		D3D11_BUFFER_DESC perObjectCBDesc;
		perObjectCBDesc.Usage = D3D11_USAGE_DYNAMIC;
		perObjectCBDesc.ByteWidth = sizeof(PerObjectCB);
		perObjectCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		perObjectCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		perObjectCBDesc.MiscFlags = 0;
		perObjectCBDesc.StructureByteStride = 0;
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&perObjectCBDesc, nullptr, &m_box.d3dPerObjectCB));


		// vertex buffer
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.ByteWidth = UINT(sizeof(mesh::MeshData::Vertex) * m_box.mesh.vertices.size());
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexInitData;
		vertexInitData.pSysMem = &m_box.mesh.vertices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexInitData, &m_box.d3dVertexBuffer));
		//perObjectCBDesc.ByteWidth = sizeof(PerObjectTextureCB);
		//XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&perObjectCBDesc, nullptr, &m_box.d3dPerObjectTextureCB));

		// index buffer
		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.ByteWidth = UINT(sizeof(uint32) * m_box.mesh.indices.size());
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexInitdata;
		indexInitdata.pSysMem = &m_box.mesh.indices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &indexInitdata, &m_box.d3dIndexBuffer));


		//diffuseTexture
		wchar_t* diffTextFileName = L"D:\\graphic_programming_Colombo\\graphic-programming\\directx11-test-project\\directx11-test\\application\\resources\\data\\3d-objects\\wet-stone\\wet_stone_color.png";
		//discard explicit creation
		//XTEST_D3D_CHECK(CreateWICTextureFromFileEx(m_d3dDevice.Get(),m_d3dContext.Get(), diffTextFileName,0,D3D11_USAGE_DYNAMIC,D3D11_BIND_SHADER_RESOURCE,D3D11_CPU_ACCESS_WRITE,D3D11_RESOURCE_MISC_TILED,0,&m_plane.d3dResourceDiffText, &m_plane.d3dShaderResourceViewDiffText));
		XTEST_D3D_CHECK(CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), diffTextFileName, &m_box.d3dResourceDiffText, &m_box.d3dShaderResourceViewDiffText, 0Ui64));

		//normal map Texture
		wchar_t* normalMapTextFileName = L"D:\\graphic_programming_Colombo\\graphic-programming\\directx11-test-project\\directx11-test\\application\\resources\\data\\3d-objects\\wet-stone\\wet_stone_norm.png";
		XTEST_D3D_CHECK(CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), normalMapTextFileName, &m_box.d3dResourceNormalMapText, &m_box.d3dShaderResourceViewNormalMapText, 0Ui64));

		//gloss map Texture
		wchar_t* glossMapTextFileName = L"D:\\graphic_programming_Colombo\\graphic-programming\\directx11-test-project\\directx11-test\\application\\resources\\data\\3d-objects\\wet-stone\\wet_stone_gloss.png";
		XTEST_D3D_CHECK(CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), glossMapTextFileName, &m_box.d3dResourceGlossMapText, &m_box.d3dShaderResourceViewGlossMapText, 0Ui64));

	}


	// crate
	{

		//geo
		std::wstring targetFile = GetRootDir().append(LR"(\3d-objects\crate.gpf)");
		
		{
			mesh::GPFMesh gpfMesh = file::ReadGPF(targetFile);
			m_crate.mesh = std::move(gpfMesh);
		}

		// W
		XMStoreFloat4x4(&m_crate.W, XMMatrixMultiply(XMMatrixScaling(0.01f, 0.01f, 0.01f), XMMatrixTranslation(0.f, 0.f, 0.f)));

		//the following rows populate the shapeAttributeMapByName member of the struct object m_crate
		//-----------------------------
        //bottom material
		Material& bottomMat = m_crate.shapeAttributeMapByName["bottom_1"].material;
		bottomMat.ambient = { 0.8f, 0.3f, 0.1f, 1.0f };
		bottomMat.diffuse = { 0.94f, 0.40f, 0.14f, 1.0f };
		bottomMat.specular = { 0.94f, 0.40f, 0.14f, 30.0f };
		
		//diffuseTexture
		wchar_t* diffTextFileName_bottom = L"D:\\graphic_programming_Colombo\\graphic-programming\\directx11-test-project\\directx11-test\\application\\resources\\data\\3d-objects\\wood\\wood_color.png";
		XTEST_D3D_CHECK(CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), diffTextFileName_bottom, &m_crate.shapeAttributeMapByName["bottom_1"].d3dResourceDiffText , &m_crate.shapeAttributeMapByName["bottom_1"].d3dShaderResourceViewDiffText, 0Ui64));
     	//normal map Texture
		wchar_t* normalMapTextFileName_bottom = L"D:\\graphic_programming_Colombo\\graphic-programming\\directx11-test-project\\directx11-test\\application\\resources\\data\\3d-objects\\wood\\wood_norm.png";
		XTEST_D3D_CHECK(CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), normalMapTextFileName_bottom, &m_crate.shapeAttributeMapByName["bottom_1"].d3dResourceNormalMapText, &m_crate.shapeAttributeMapByName["bottom_1"].d3dShaderResourceViewNormalMapText, 0Ui64));
		//gloss map Texture
		wchar_t* glossMapTextFileName_bottom = L"D:\\graphic_programming_Colombo\\graphic-programming\\directx11-test-project\\directx11-test\\application\\resources\\data\\3d-objects\\wood\\wood_gloss.png";
		XTEST_D3D_CHECK(CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), glossMapTextFileName_bottom, &m_crate.shapeAttributeMapByName["bottom_1"].d3dResourceGlossMapText, &m_crate.shapeAttributeMapByName["bottom_1"].d3dShaderResourceViewGlossMapText, 0Ui64));


		//top material
		Material& topMat = m_crate.shapeAttributeMapByName["top_2"].material;
		topMat.ambient = { 0.8f, 0.8f, 0.8f, 1.0f };
		topMat.diffuse = { 0.9f, 0.9f, 0.9f, 1.0f };
		topMat.specular = { 0.9f, 0.9f, 0.9f, 550.0f };
		
		//diffuseTexture
		wchar_t* diffTextFileName_top = L"D:\\graphic_programming_Colombo\\graphic-programming\\directx11-test-project\\directx11-test\\application\\resources\\data\\3d-objects\\fabric\\fabric_color.png";
		XTEST_D3D_CHECK(CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), diffTextFileName_top, &m_crate.shapeAttributeMapByName["top_2"].d3dResourceDiffText, &m_crate.shapeAttributeMapByName["top_2"].d3dShaderResourceViewDiffText, 0Ui64));
		//normal map Texture
		wchar_t* normalMapTextFileName_top = L"D:\\graphic_programming_Colombo\\graphic-programming\\directx11-test-project\\directx11-test\\application\\resources\\data\\3d-objects\\fabric\\fabric_norm.png";
		XTEST_D3D_CHECK(CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), normalMapTextFileName_top, &m_crate.shapeAttributeMapByName["top_2"].d3dResourceNormalMapText, &m_crate.shapeAttributeMapByName["top_2"].d3dShaderResourceViewNormalMapText, 0Ui64));
		//gloss map Texture
		wchar_t* glossMapTextFileName_top = L"D:\\graphic_programming_Colombo\\graphic-programming\\directx11-test-project\\directx11-test\\application\\resources\\data\\3d-objects\\fabric\\fabric_gloss.png";
		XTEST_D3D_CHECK(CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), glossMapTextFileName_top, &m_crate.shapeAttributeMapByName["top_2"].d3dResourceGlossMapText, &m_crate.shapeAttributeMapByName["top_2"].d3dShaderResourceViewGlossMapText, 0Ui64));

		//top handles material
		Material& topHandleMat = m_crate.shapeAttributeMapByName["top_handles_4"].material;
		topHandleMat.ambient = { 0.3f, 0.3f, 0.3f, 1.0f };
		topHandleMat.diffuse = { 0.4f, 0.4f, 0.4f, 1.0f };
		topHandleMat.specular = { 0.9f, 0.9f, 0.9f, 120.0f };
		//diffuseTexture
		wchar_t* diffTextFileName_top_handles = L"D:\\graphic_programming_Colombo\\graphic-programming\\directx11-test-project\\directx11-test\\application\\resources\\data\\3d-objects\\plastic-cover\\plastic_cover_color.png";
		XTEST_D3D_CHECK(CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), diffTextFileName_top_handles, &m_crate.shapeAttributeMapByName["top_handles_4"].d3dResourceDiffText, &m_crate.shapeAttributeMapByName["top_handles_4"].d3dShaderResourceViewDiffText, 0Ui64));
		//normal map Texture
		wchar_t* normalMapTextFileName_top_handles = L"D:\\graphic_programming_Colombo\\graphic-programming\\directx11-test-project\\directx11-test\\application\\resources\\data\\3d-objects\\plastic-cover\\plastic_cover_norm.png";
		XTEST_D3D_CHECK(CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), normalMapTextFileName_top_handles, &m_crate.shapeAttributeMapByName["top_handles_4"].d3dResourceNormalMapText, &m_crate.shapeAttributeMapByName["top_handles_4"].d3dShaderResourceViewNormalMapText, 0Ui64));
		//gloss map Texture
		wchar_t* glossMapTextFileName_top_handles = L"D:\\graphic_programming_Colombo\\graphic-programming\\directx11-test-project\\directx11-test\\application\\resources\\data\\3d-objects\\plastic-cover\\plastic_cover_gloss.png";
		XTEST_D3D_CHECK(CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), glossMapTextFileName_top_handles, &m_crate.shapeAttributeMapByName["top_handles_4"].d3dResourceGlossMapText, &m_crate.shapeAttributeMapByName["top_handles_4"].d3dShaderResourceViewGlossMapText, 0Ui64));


		//handle material
		Material& handleMat = m_crate.shapeAttributeMapByName["handles_8"].material;
		handleMat.ambient = { 0.5f, 0.5f, 0.1f, 1.0f };
		handleMat.diffuse = { 0.67f, 0.61f, 0.1f, 1.0f };
		handleMat.specular = { 0.67f, 0.61f, 0.1f, 200.0f };

		wchar_t* diffTextFileName_handles = L"D:\\graphic_programming_Colombo\\graphic-programming\\directx11-test-project\\directx11-test\\application\\resources\\data\\3d-objects\\coat\\coat_color.png";
		XTEST_D3D_CHECK(CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), diffTextFileName_handles, &m_crate.shapeAttributeMapByName["handles_8"].d3dResourceDiffText, &m_crate.shapeAttributeMapByName["handles_8"].d3dShaderResourceViewDiffText, 0Ui64));
		//normal map Texture
		wchar_t* normalMapTextFileName_handles = L"D:\\graphic_programming_Colombo\\graphic-programming\\directx11-test-project\\directx11-test\\application\\resources\\data\\3d-objects\\coat\\coat_norm.png";
		XTEST_D3D_CHECK(CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), normalMapTextFileName_handles, &m_crate.shapeAttributeMapByName["handles_8"].d3dResourceNormalMapText, &m_crate.shapeAttributeMapByName["handles_8"].d3dShaderResourceViewNormalMapText, 0Ui64));
		//gloss map Texture
		wchar_t* glossMapTextFileName_handles = L"D:\\graphic_programming_Colombo\\graphic-programming\\directx11-test-project\\directx11-test\\application\\resources\\data\\3d-objects\\coat\\coat_gloss.png";
		XTEST_D3D_CHECK(CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), glossMapTextFileName_handles, &m_crate.shapeAttributeMapByName["handles_8"].d3dResourceGlossMapText, &m_crate.shapeAttributeMapByName["handles_8"].d3dShaderResourceViewGlossMapText, 0Ui64));


		//metal material
		Material& metalPiecesMat = m_crate.shapeAttributeMapByName["metal_pieces_3"].material;
		metalPiecesMat.ambient = { 0.3f, 0.3f, 0.3f, 1.0f };
		metalPiecesMat.diffuse = { 0.4f, 0.4f, 0.4f, 1.0f };
		metalPiecesMat.specular = { 0.4f, 0.4f, 0.4f, 520.0f };


		wchar_t* diffTextFileName_metal_p = L"D:\\graphic_programming_Colombo\\graphic-programming\\directx11-test-project\\directx11-test\\application\\resources\\data\\3d-objects\\twine\\twine_color.png";
		XTEST_D3D_CHECK(CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), diffTextFileName_metal_p, &m_crate.shapeAttributeMapByName["metal_pieces_3"].d3dResourceDiffText, &m_crate.shapeAttributeMapByName["metal_pieces_3"].d3dShaderResourceViewDiffText, 0Ui64));
		//normal map Texture
		wchar_t* normalMapTextFileName_metal_p = L"D:\\graphic_programming_Colombo\\graphic-programming\\directx11-test-project\\directx11-test\\application\\resources\\data\\3d-objects\\twine\\twine_norm.png";
		XTEST_D3D_CHECK(CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), normalMapTextFileName_metal_p, &m_crate.shapeAttributeMapByName["metal_pieces_3"].d3dResourceNormalMapText, &m_crate.shapeAttributeMapByName["metal_pieces_3"].d3dShaderResourceViewNormalMapText, 0Ui64));
		//gloss map Texture
		wchar_t* glossMapTextFileName_metal_p = L"D:\\graphic_programming_Colombo\\graphic-programming\\directx11-test-project\\directx11-test\\application\\resources\\data\\3d-objects\\twine\\twine_gloss.png";
		XTEST_D3D_CHECK(CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), glossMapTextFileName_metal_p, &m_crate.shapeAttributeMapByName["metal_pieces_3"].d3dResourceGlossMapText, &m_crate.shapeAttributeMapByName["metal_pieces_3"].d3dShaderResourceViewGlossMapText, 0Ui64));


		//then, we populate the shapeAttributeMapByName with many constant buffers, one for each of the crate's parts
		//---------------------
		for (const auto& namePairWithDesc : m_crate.mesh.meshDescriptorMapByName)
		{
			ComPtr<ID3D11Buffer> d3dPerObjectCB;
			//ComPtr<ID3D11Buffer> d3dPerObjectTextureCB;
			// perObjectCBs
			D3D11_BUFFER_DESC perObjectCBDesc;
			perObjectCBDesc.Usage = D3D11_USAGE_DYNAMIC;
			perObjectCBDesc.ByteWidth = sizeof(PerObjectCB);
			perObjectCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			perObjectCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			perObjectCBDesc.MiscFlags = 0;
			perObjectCBDesc.StructureByteStride = 0;
			XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&perObjectCBDesc, nullptr, &d3dPerObjectCB));
			//perObjectCBDesc.ByteWidth = sizeof(PerObjectTextureCB);
			//XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&perObjectCBDesc, nullptr, &d3dPerObjectTextureCB));

			m_crate.shapeAttributeMapByName[namePairWithDesc.first].d3dPerObjectCB = d3dPerObjectCB;
			//m_crate.shapeAttributeMapByName[namePairWithDesc.first].d3dPerObjectTextureCB = d3dPerObjectTextureCB;

		}
	

		// vertex buffer
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.ByteWidth = UINT(sizeof(mesh::MeshData::Vertex) * m_crate.mesh.meshData.vertices.size());
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexInitData;
		vertexInitData.pSysMem = &m_crate.mesh.meshData.vertices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexInitData, &m_crate.d3dVertexBuffer));


		// index buffer
		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.ByteWidth = UINT(sizeof(uint32) * m_crate.mesh.meshData.indices.size());
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexInitdata;
		indexInitdata.pSysMem = &m_crate.mesh.meshData.indices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &indexInitdata, &m_crate.d3dIndexBuffer));
	}

}


void LightsDemoApp::InitLights()
{
	m_dirLight.ambient = { 0.16f, 0.18f, 0.18f, 1.f };
	m_dirLight.diffuse = { 0.4f* 0.87f,0.4f* 0.90f,0.4f* 0.94f, 1.f };
	m_dirLight.specular = { 0.87f, 0.90f, 0.94f, 1.f };
	XMVECTOR dirLightDirection = XMVector3Normalize(-XMVectorSet(5.f, 3.f, 5.f, 0.f));
	XMStoreFloat3(&m_dirLight.dirW, dirLightDirection);

	
	for (int i = 0; i < 5;i++) {
		PointLight pointLight;

		pointLight.ambient = { 0.18f, 0.04f, 0.16f, 1.0f };
		pointLight.diffuse = { 0.94f, 0.23f, 0.87f, 1.0f };
		pointLight.specular = { 0.94f, 0.23f, 0.87f, 1.0f };
		pointLight.posW = XMFLOAT3(10.f*cosf(XM_2PI/5 *(i+1)),2.f, 10.f*sinf(XM_2PI / 5 * (i + 1)));
		//XMMATRIX R = XMMatrixRotationY(math::ToRadians(30.f)*(i + 1));
		//XMStoreFloat3(&(*pointLightsRefs[i]).posW, XMVector3Transform(XMLoadFloat3(&(*pointLightsRefs[i]).posW), R));
		pointLight.range = 150.f;
		pointLight.attenuation = { 0.0f, 0.2f, 0.f };
		m_pointLights.push_back(pointLight);
	}

	for (int i = 0; i < 3; i++) {
		SpotLight spotLight;
		spotLight.ambient = { 0.018f, 0.018f, 0.18f, 1.0f };
		spotLight.diffuse = { 0.1f, 0.1f, 0.9f, 1.0f };
		spotLight.specular = { 0.1f, 0.1f, 0.9f, 1.0f };
		//XMVECTOR posW = XMVectorSet(5.f*cosf(XM_2PI / 3 * (i + 1)), 5.f, 5.f*sinf(XM_2PI / 3 * (i + 1)), 1.f);
		//XMStoreFloat3(&spotLight.posW, posW);
		spotLight.posW = XMFLOAT3(20.f*cosf(XM_2PI / 3 * (i + 1)), 35.f, 20.f*sinf(XM_2PI / 3 * (i + 1)));
		spotLight.range = 70.f;
		XMVECTOR dirW = XMVector3Normalize(XMVectorSet(0.f, 0.5f, 0.f, 1.f) - XMVectorSet(spotLight.posW.x, spotLight.posW.y, spotLight.posW.z,1.0f));
		XMStoreFloat3(&spotLight.dirW, dirW);
		spotLight.spot = 20.f;
		spotLight.attenuation = { 0.0f, 0.125f, 0.f };
		m_spotLights.push_back(spotLight);
	}
	
	m_lightsControl.useDirLight = true;
	m_lightsControl.usePointLight = true;
	m_lightsControl.useSpotLight = true;


	// RarelyChangedCB
	{
		D3D11_BUFFER_DESC rarelyChangedCB;
		rarelyChangedCB.Usage = D3D11_USAGE_DYNAMIC;
		rarelyChangedCB.ByteWidth = sizeof(RarelyChangedCB);
		rarelyChangedCB.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		rarelyChangedCB.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		rarelyChangedCB.MiscFlags = 0;
		rarelyChangedCB.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA lightControlData;
		lightControlData.pSysMem = &m_lightsControl;
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&rarelyChangedCB, &lightControlData, &m_d3dRarelyChangedCB));
	}
}
void LightsDemoApp::InitTexturesControls() 
{

	m_texturesControl.useColorTextureMap = true;
	m_texturesControl.useNormalTextureMap = true;
	m_texturesControl.useGlossTextureMap = true;
	// RarelyChangedTextureCB
	{
		D3D11_BUFFER_DESC rarelyChangedTextureCB;
		rarelyChangedTextureCB.Usage = D3D11_USAGE_DYNAMIC;
		rarelyChangedTextureCB.ByteWidth = sizeof(RarelyChangedTextureCB);
		rarelyChangedTextureCB.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		rarelyChangedTextureCB.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		rarelyChangedTextureCB.MiscFlags = 0;
		rarelyChangedTextureCB.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA textureControlData;
		textureControlData.pSysMem = &m_texturesControl;
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&rarelyChangedTextureCB, &textureControlData, &m_d3dRarelyChangedTextureCB));


	}


}



void LightsDemoApp::InitRasterizerState()
{
	// rasterizer state
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthClipEnable = true;

	m_d3dDevice->CreateRasterizerState(&rasterizerDesc, &m_rasterizerState);
}


void LightsDemoApp::OnResized()
{
	application::DirectxApp::OnResized();

	//update the projection matrix with the new aspect ratio
	XMMATRIX P = XMMatrixPerspectiveFovLH(math::ToRadians(45.f), AspectRatio(), 1.f, 1000.f);
	XMStoreFloat4x4(&m_projectionMatrix, P);
}


void LightsDemoApp::OnWheelScroll(input::ScrollStatus scroll)
{
	// zoom in or out when the scroll wheel is used
	if (service::Locator::GetMouse()->IsInClientArea())
	{
		m_camera.IncreaseRadiusBy(scroll.isScrollingUp ? -0.5f : 0.5f);
	}
}


void LightsDemoApp::OnMouseMove(const DirectX::XMINT2& movement, const DirectX::XMINT2& currentPos)
{
	XTEST_UNUSED_VAR(currentPos);

	input::Mouse* mouse = service::Locator::GetMouse();

	// rotate the camera position around the cube when the left button is pressed
	if (mouse->GetButtonStatus(input::MouseButton::left_button).isDown && mouse->IsInClientArea())
	{
		m_camera.RotateBy(math::ToRadians(movement.y * -0.25f), math::ToRadians(movement.x * 0.25f));
	}

	// pan the camera position when the right button is pressed
	if (mouse->GetButtonStatus(input::MouseButton::right_button).isDown && mouse->IsInClientArea())
	{
		XMFLOAT3 cameraX = m_camera.GetXAxis();
		XMFLOAT3 cameraY = m_camera.GetYAxis();

		// we should calculate the right amount of pan in screen space but for now this is good enough
		XMVECTOR xPanTranslation = XMVectorScale(XMLoadFloat3(&cameraX), float(-movement.x) * 0.01f);
		XMVECTOR yPanTranslation = XMVectorScale(XMLoadFloat3(&cameraY), float(movement.y) * 0.01f);

		XMFLOAT3 panTranslation;
		XMStoreFloat3(&panTranslation, XMVectorAdd(xPanTranslation, yPanTranslation));
		m_camera.TranslatePivotBy(panTranslation);
	}

}


void LightsDemoApp::OnKeyStatusChange(input::Key key, const input::KeyStatus& status)
{

	// re-frame F key is pressed
	if (key == input::Key::F && status.isDown)
	{
		m_camera.SetPivot({ 0.f, 0.f, 0.f });
	}
	else if (key == input::Key::F1 && status.isDown)
	{
		m_lightsControl.useDirLight = !m_lightsControl.useDirLight;
		m_isLightControlDirty = true;
	}
	else if (key == input::Key::F2 && status.isDown)
	{
		m_lightsControl.usePointLight = !m_lightsControl.usePointLight;
		m_isLightControlDirty = true;
	}
	else if (key == input::Key::F3 && status.isDown)
	{
		m_lightsControl.useSpotLight = !m_lightsControl.useSpotLight;
		m_isLightControlDirty = true;
	}
	else if (key == input::Key::space_bar && status.isDown)
	{
		m_stopLights = !m_stopLights;
	}
	else if (key == input::Key::F4 && status.isDown)
	{
		m_texturesControl.useColorTextureMap = !m_texturesControl.useColorTextureMap;
		m_isTexturesControlDirty = true;
	}
	else if (key == input::Key::F5 && status.isDown)
	{
		m_texturesControl.useNormalTextureMap = !m_texturesControl.useNormalTextureMap;
		m_isTexturesControlDirty = true;
	}
	else if (key == input::Key::F6 && status.isDown)
	{
		m_texturesControl.useGlossTextureMap = !m_texturesControl.useGlossTextureMap;
		m_isTexturesControlDirty = true;
	}
}


void LightsDemoApp::UpdateScene(float deltaSeconds)
{
	XTEST_UNUSED_VAR(deltaSeconds);


	// create the model-view-projection matrix
	XMMATRIX V = m_camera.GetViewMatrix();
	XMStoreFloat4x4(&m_viewMatrix, V);

	// create projection matrix
	XMMATRIX P = XMLoadFloat4x4(&m_projectionMatrix);



	m_d3dAnnotation->BeginEvent(L"update-constant-buffer");


	// plane PerObjectCB
	{
		XMMATRIX W = XMLoadFloat4x4(&m_plane.W);
		XMMATRIX WVP = W * V*P;

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		// disable gpu access
		XTEST_D3D_CHECK(m_d3dContext->Map(m_plane.d3dPerObjectCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		PerObjectCB* perObjectCB = (PerObjectCB*)mappedResource.pData;

		//update the data
		XMStoreFloat4x4(&perObjectCB->W, XMMatrixTranspose(W));
		XMStoreFloat4x4(&perObjectCB->WVP, XMMatrixTranspose(WVP));
		XMStoreFloat4x4(&perObjectCB->W_inverseTraspose, XMMatrixInverse(nullptr, W));
		perObjectCB->material = m_plane.material;

		// enable gpu access
		m_d3dContext->Unmap(m_plane.d3dPerObjectCB.Get(), 0);
		/*
		D3D11_MAPPED_SUBRESOURCE texturePropsMappedResource;
		ZeroMemory(&texturePropsMappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		// disable gpu access
		XTEST_D3D_CHECK(m_d3dContext->Map(m_plane.d3dPerObjectTextureCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &texturePropsMappedResource));
		PerObjectTextureCB* perObjectTextureCB = (PerObjectTextureCB*)texturePropsMappedResource.pData;
		perObjectTextureCB->usesNormalMapTexture = true;
		perObjectTextureCB->usesTwoColorMapTextures = false;
		m_d3dContext->Unmap(m_plane.d3dPerObjectTextureCB.Get(), 0);
		*/
	}


	// sphere PerObjectCB
	{
		XMMATRIX W = XMLoadFloat4x4(&m_sphere.W);
		XMMATRIX WVP = W * V*P;

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		// disable gpu access
		XTEST_D3D_CHECK(m_d3dContext->Map(m_sphere.d3dPerObjectCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		PerObjectCB* perObjectCB = (PerObjectCB*)mappedResource.pData;

		//update the data
		XMStoreFloat4x4(&perObjectCB->W, XMMatrixTranspose(W));
		XMStoreFloat4x4(&perObjectCB->WVP, XMMatrixTranspose(WVP));
		XMStoreFloat4x4(&perObjectCB->W_inverseTraspose, XMMatrixInverse(nullptr, W));
		perObjectCB->material = m_sphere.material;

		// enable gpu access
		m_d3dContext->Unmap(m_sphere.d3dPerObjectCB.Get(), 0);

		/*
		D3D11_MAPPED_SUBRESOURCE texturePropsMappedResource;
		ZeroMemory(&texturePropsMappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		// disable gpu access
		XTEST_D3D_CHECK(m_d3dContext->Map(m_sphere.d3dPerObjectTextureCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &texturePropsMappedResource));
		PerObjectTextureCB* perObjectTextureCB = (PerObjectTextureCB*)texturePropsMappedResource.pData;
		perObjectTextureCB->usesNormalMapTexture = true;
		perObjectTextureCB->usesTwoColorMapTextures = true;
		m_d3dContext->Unmap(m_sphere.d3dPerObjectTextureCB.Get(), 0);
		*/
	}

	// torus PerObjectCB
	{
		XMMATRIX W = XMLoadFloat4x4(&m_torus.W);
		XMMATRIX WVP = W * V*P;

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		// disable gpu access
		XTEST_D3D_CHECK(m_d3dContext->Map(m_torus.d3dPerObjectCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		PerObjectCB* perObjectCB = (PerObjectCB*)mappedResource.pData;

		//update the data
		XMStoreFloat4x4(&perObjectCB->W, XMMatrixTranspose(W));
		XMStoreFloat4x4(&perObjectCB->WVP, XMMatrixTranspose(WVP));
		XMStoreFloat4x4(&perObjectCB->W_inverseTraspose, XMMatrixInverse(nullptr, W));
		perObjectCB->material = m_torus.material;

		// enable gpu access
		m_d3dContext->Unmap(m_torus.d3dPerObjectCB.Get(), 0);
		/*
		D3D11_MAPPED_SUBRESOURCE texturePropsMappedResource;
		ZeroMemory(&texturePropsMappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		// disable gpu access
		XTEST_D3D_CHECK(m_d3dContext->Map(m_torus.d3dPerObjectTextureCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &texturePropsMappedResource));
		PerObjectTextureCB* perObjectTextureCB = (PerObjectTextureCB*)texturePropsMappedResource.pData;
		perObjectTextureCB->usesNormalMapTexture = true;
		perObjectTextureCB->usesTwoColorMapTextures = true;
		m_d3dContext->Unmap(m_torus.d3dPerObjectTextureCB.Get(), 0);
		*/
	}

	// box PerObjectCB
	{
		XMMATRIX W = XMLoadFloat4x4(&m_box.W);
		XMMATRIX WVP = W * V*P;

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		// disable gpu access
		XTEST_D3D_CHECK(m_d3dContext->Map(m_box.d3dPerObjectCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		PerObjectCB* perObjectCB = (PerObjectCB*)mappedResource.pData;

		//update the data
		XMStoreFloat4x4(&perObjectCB->W, XMMatrixTranspose(W));
		XMStoreFloat4x4(&perObjectCB->WVP, XMMatrixTranspose(WVP));
		XMStoreFloat4x4(&perObjectCB->W_inverseTraspose, XMMatrixInverse(nullptr, W));
		perObjectCB->material = m_box.material;

		// enable gpu access
		m_d3dContext->Unmap(m_box.d3dPerObjectCB.Get(), 0);
		/*
		D3D11_MAPPED_SUBRESOURCE texturePropsMappedResource;
		ZeroMemory(&texturePropsMappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		// disable gpu access
		XTEST_D3D_CHECK(m_d3dContext->Map(m_box.d3dPerObjectTextureCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &texturePropsMappedResource));
		PerObjectTextureCB* perObjectTextureCB = (PerObjectTextureCB*)texturePropsMappedResource.pData;
		perObjectTextureCB->usesNormalMapTexture = true;
		perObjectTextureCB->usesTwoColorMapTextures = false;
		m_d3dContext->Unmap(m_box.d3dPerObjectTextureCB.Get(), 0);
		*/
	}


	// crate PerObjectCB
	{
		XMMATRIX W = XMLoadFloat4x4(&m_crate.W);
		XMMATRIX WVP = W * V*P;

		for (const auto& namePairWithDesc : m_crate.mesh.meshDescriptorMapByName)
		{
			const std::string& shapeName = namePairWithDesc.first;


			D3D11_MAPPED_SUBRESOURCE mappedResource;
			ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

			// disable gpu access
			XTEST_D3D_CHECK(m_d3dContext->Map(m_crate.shapeAttributeMapByName[shapeName].d3dPerObjectCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
			PerObjectCB* perObjectCB = (PerObjectCB*)mappedResource.pData;

			//update the data
			XMStoreFloat4x4(&perObjectCB->W, XMMatrixTranspose(W));
			XMStoreFloat4x4(&perObjectCB->WVP, XMMatrixTranspose(WVP));
			XMStoreFloat4x4(&perObjectCB->W_inverseTraspose, XMMatrixInverse(nullptr, W));
			perObjectCB->material = m_crate.shapeAttributeMapByName[shapeName].material;

			// enable gpu access
			m_d3dContext->Unmap(m_crate.shapeAttributeMapByName[shapeName].d3dPerObjectCB.Get(), 0);
			/*
			D3D11_MAPPED_SUBRESOURCE texturePropsMappedResource;
			ZeroMemory(&texturePropsMappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

			// disable gpu access
			XTEST_D3D_CHECK(m_d3dContext->Map(m_crate.shapeAttributeMapByName[shapeName].d3dPerObjectTextureCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &texturePropsMappedResource));
			PerObjectTextureCB* perObjectTextureCB = (PerObjectTextureCB*)texturePropsMappedResource.pData;
			perObjectTextureCB->usesNormalMapTexture = false;
			perObjectTextureCB->usesTwoColorMapTextures = false;
			m_d3dContext->Unmap(m_crate.shapeAttributeMapByName[shapeName].d3dPerObjectTextureCB.Get(), 0);
			*/

		}
	}


	// PerFrameCB
	{

		if (!m_stopLights)
		{
			XMMATRIX R = XMMatrixRotationY(math::ToRadians(30.f) * deltaSeconds);
			for (int i = 0; i < m_pointLights.size(); i++) {
				XMStoreFloat3(&m_pointLights[i].posW, XMVector3Transform(XMLoadFloat3(&m_pointLights[i].posW), R));
			}
			R = XMMatrixRotationY(math::ToRadians(60.f) * deltaSeconds);
			for (int i = 0; i < m_spotLights.size(); i++) {
				XMStoreFloat3(&m_spotLights[i].posW, XMVector3Transform(XMLoadFloat3(&m_spotLights[i].posW), R));
				XMStoreFloat3(&m_spotLights[i].dirW, XMVector3Transform(XMLoadFloat3(&m_spotLights[i].dirW), R));
			}

			R = XMMatrixRotationAxis(XMVectorSet(-1.f, 0.f, 1.f, 1.f), math::ToRadians(10.f) * deltaSeconds);
			XMStoreFloat3(&m_dirLight.dirW, XMVector3Transform(XMLoadFloat3(&m_dirLight.dirW), R));
		}

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		// disable gpu access
		XTEST_D3D_CHECK(m_d3dContext->Map(m_d3dPerFrameCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		PerFrameCB* perFrameCB = (PerFrameCB*)mappedResource.pData;

		//update the data
		perFrameCB->dirLight = m_dirLight;
		perFrameCB->pointLight0 = m_pointLights[0];
		perFrameCB->pointLight1 = m_pointLights[1];
		perFrameCB->pointLight2 = m_pointLights[2];
		perFrameCB->pointLight3 = m_pointLights[3];
		perFrameCB->pointLight4 = m_pointLights[4];
		perFrameCB->spotLight0 = m_spotLights[0];
		perFrameCB->spotLight0 = m_spotLights[1];
		perFrameCB->spotLight0 = m_spotLights[2];

		perFrameCB->eyePosW = m_camera.GetPosition();

		// enable gpu access
		m_d3dContext->Unmap(m_d3dPerFrameCB.Get(), 0);


	}

	//PerFrameTextureCB
	/*
	{

		XMMATRIX translateUVcoords = XMMatrixTranslation(0.0f, deltaSeconds, 0.0f);

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		XTEST_D3D_CHECK(m_d3dContext->Map(m_d3dPerFrameCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		PerFrameTextureCB* perFrameCB = (PerFrameTextureCB*)mappedResource.pData;
		XMStoreFloat4x4(&(perFrameCB->texCoordMatrix),translateUVcoords);
		m_d3dContext->Unmap(m_d3dPerFrameTextureCB.Get(), 0);

	}
	*/
		// RarelyChangedCB
		
	{
			if (m_isLightControlDirty)
			{
				D3D11_MAPPED_SUBRESOURCE mappedResource;
				ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

				// disable gpu access
				XTEST_D3D_CHECK(m_d3dContext->Map(m_d3dRarelyChangedCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
				RarelyChangedCB* rarelyChangedCB = (RarelyChangedCB*)mappedResource.pData;

				//update the data
				rarelyChangedCB->useDirLight = m_lightsControl.useDirLight;
				rarelyChangedCB->usePointLight = m_lightsControl.usePointLight;
				rarelyChangedCB->useSpotLight = m_lightsControl.useSpotLight;

				// enable gpu access
				m_d3dContext->Unmap(m_d3dRarelyChangedCB.Get(), 0);

				m_d3dContext->PSSetConstantBuffers(2, 1, m_d3dRarelyChangedCB.GetAddressOf());
				m_isLightControlDirty = false;

			}
		
	}
		
		
	// RarelyChangedTextureCB
		
	{
			if (m_isTexturesControlDirty) 
			{
				D3D11_MAPPED_SUBRESOURCE mappedResource;
				ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

				// disable gpu access
				XTEST_D3D_CHECK(m_d3dContext->Map(m_d3dRarelyChangedTextureCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
				RarelyChangedTextureCB* rarelyChangedTextureCB = (RarelyChangedTextureCB*)mappedResource.pData;

				//update the data
				rarelyChangedTextureCB->useColorTextureMap = m_texturesControl.useColorTextureMap;
				rarelyChangedTextureCB->useNormalTextureMap = m_texturesControl.useNormalTextureMap;
				rarelyChangedTextureCB->useGlossTextureMap = m_texturesControl.useGlossTextureMap;

				// enable gpu access
				m_d3dContext->Unmap(m_d3dRarelyChangedTextureCB.Get(), 0);
				m_d3dContext->PSSetConstantBuffers(3, 1, m_d3dRarelyChangedTextureCB.GetAddressOf());


				m_isTexturesControlDirty = false;

			}
		
	}

		
	m_d3dAnnotation->EndEvent();
	
}


void LightsDemoApp::RenderScene()
{
	m_d3dAnnotation->BeginEvent(L"render-scene");

	// clear the frame
	m_d3dContext->ClearDepthStencilView(m_depthBufferView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
	m_d3dContext->ClearRenderTargetView(m_backBufferView.Get(), DirectX::Colors::DarkGray);

	// set the shaders and the input layout
	m_d3dContext->RSSetState(m_rasterizerState.Get());
	m_d3dContext->IASetInputLayout(m_inputLayout.Get());
	m_d3dContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	m_d3dContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	m_d3dContext->PSSetConstantBuffers(1, 1, m_d3dPerFrameCB.GetAddressOf());

	//m_d3dContext->VSSetConstantBuffers(4, 1, m_d3dPerFrameTextureCB.GetAddressOf());

	m_d3dContext->PSSetSamplers(0,1,m_textureSamplerState.GetAddressOf());

	m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// draw plane
	{
		// bind the constant data to the vertex shader
		m_d3dContext->VSSetConstantBuffers(0, 1, m_plane.d3dPerObjectCB.GetAddressOf());
		m_d3dContext->PSSetConstantBuffers(0, 1, m_plane.d3dPerObjectCB.GetAddressOf());

		//m_d3dContext->VSSetConstantBuffers(5, 1, m_plane.d3dPerObjectTextureCB.GetAddressOf());
		//m_d3dContext->PSSetConstantBuffers(5, 1, m_plane.d3dPerObjectTextureCB.GetAddressOf());

		m_d3dContext->PSSetShaderResources(0, 1, m_plane.d3dShaderResourceViewDiffText.GetAddressOf());
		m_d3dContext->PSSetShaderResources(1, 1, m_plane.d3dShaderResourceViewNormalMapText.GetAddressOf());
		m_d3dContext->PSSetShaderResources(2, 1, m_plane.d3dShaderResourceViewGlossMapText.GetAddressOf());


		// set what to draw
		UINT stride = sizeof(mesh::MeshData::Vertex);
		UINT offset = 0;
		m_d3dContext->IASetVertexBuffers(0, 1, m_plane.d3dVertexBuffer.GetAddressOf(), &stride, &offset);
		m_d3dContext->IASetIndexBuffer(m_plane.d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		m_d3dContext->DrawIndexed(UINT(m_plane.mesh.indices.size()), 0, 0);
	}


	// draw sphere
	{
		// bind the constant data to the vertex shader
		m_d3dContext->VSSetConstantBuffers(0, 1, m_sphere.d3dPerObjectCB.GetAddressOf());
		m_d3dContext->PSSetConstantBuffers(0, 1, m_sphere.d3dPerObjectCB.GetAddressOf());

		//m_d3dContext->VSSetConstantBuffers(5, 1, m_sphere.d3dPerObjectTextureCB.GetAddressOf());
		//m_d3dContext->PSSetConstantBuffers(5, 1, m_sphere.d3dPerObjectTextureCB.GetAddressOf());

		m_d3dContext->PSSetShaderResources(0, 1, m_sphere.d3dShaderResourceViewDiffText.GetAddressOf());
		m_d3dContext->PSSetShaderResources(1, 1, m_sphere.d3dShaderResourceViewNormalMapText.GetAddressOf());
		m_d3dContext->PSSetShaderResources(2, 1, m_sphere.d3dShaderResourceViewGlossMapText.GetAddressOf());
		//m_d3dContext->PSSetShaderResources(3, 1, m_sphere.d3dShaderResourceViewMotionDiffText.GetAddressOf());


		// set what to draw
		UINT stride = sizeof(mesh::MeshData::Vertex);
		UINT offset = 0;
		m_d3dContext->IASetVertexBuffers(0, 1, m_sphere.d3dVertexBuffer.GetAddressOf(), &stride, &offset);
		m_d3dContext->IASetIndexBuffer(m_sphere.d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		m_d3dContext->DrawIndexed(UINT(m_sphere.mesh.indices.size()), 0, 0);
	}

	// draw torus
	{
		// bind the constant data to the vertex shader
		m_d3dContext->VSSetConstantBuffers(0, 1, m_torus.d3dPerObjectCB.GetAddressOf());
		m_d3dContext->PSSetConstantBuffers(0, 1, m_torus.d3dPerObjectCB.GetAddressOf());

		//m_d3dContext->VSSetConstantBuffers(5, 1, m_torus.d3dPerObjectTextureCB.GetAddressOf());
		//m_d3dContext->PSSetConstantBuffers(5, 1, m_torus.d3dPerObjectTextureCB.GetAddressOf());

		m_d3dContext->PSSetShaderResources(0, 1, m_torus.d3dShaderResourceViewDiffText.GetAddressOf());
		m_d3dContext->PSSetShaderResources(1, 1, m_torus.d3dShaderResourceViewNormalMapText.GetAddressOf());
		m_d3dContext->PSSetShaderResources(2, 1, m_torus.d3dShaderResourceViewGlossMapText.GetAddressOf());
		//m_d3dContext->PSSetShaderResources(3, 1, m_torus.d3dShaderResourceViewMotionDiffText.GetAddressOf());



		// set what to draw
		UINT stride = sizeof(mesh::MeshData::Vertex);
		UINT offset = 0;
		m_d3dContext->IASetVertexBuffers(0, 1, m_torus.d3dVertexBuffer.GetAddressOf(), &stride, &offset);
		m_d3dContext->IASetIndexBuffer(m_torus.d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		m_d3dContext->DrawIndexed(UINT(m_torus.mesh.indices.size()), 0, 0);
	}
	// draw box
	{
		// bind the constant data to the vertex shader
		m_d3dContext->VSSetConstantBuffers(0, 1, m_box.d3dPerObjectCB.GetAddressOf());
		m_d3dContext->PSSetConstantBuffers(0, 1, m_box.d3dPerObjectCB.GetAddressOf());

		//m_d3dContext->VSSetConstantBuffers(5, 1, m_box.d3dPerObjectTextureCB.GetAddressOf());
		//m_d3dContext->PSSetConstantBuffers(5, 1, m_box.d3dPerObjectTextureCB.GetAddressOf());

		m_d3dContext->PSSetShaderResources(0, 1, m_box.d3dShaderResourceViewDiffText.GetAddressOf());
		m_d3dContext->PSSetShaderResources(1, 1, m_box.d3dShaderResourceViewNormalMapText.GetAddressOf());
		m_d3dContext->PSSetShaderResources(2, 1, m_box.d3dShaderResourceViewGlossMapText.GetAddressOf());



		// set what to draw
		UINT stride = sizeof(mesh::MeshData::Vertex);
		UINT offset = 0;
		m_d3dContext->IASetVertexBuffers(0, 1, m_box.d3dVertexBuffer.GetAddressOf(), &stride, &offset);
		m_d3dContext->IASetIndexBuffer(m_box.d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		m_d3dContext->DrawIndexed(UINT(m_box.mesh.indices.size()), 0, 0);
	}


	// draw crate
	{
		// set what to draw
		UINT stride = sizeof(mesh::MeshData::Vertex);
		UINT offset = 0;
		m_d3dContext->IASetVertexBuffers(0, 1, m_crate.d3dVertexBuffer.GetAddressOf(), &stride, &offset);
		m_d3dContext->IASetIndexBuffer(m_crate.d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		for (const auto& namePairWithDesc : m_crate.mesh.meshDescriptorMapByName)
		{
			const std::string& shapeName = namePairWithDesc.first;
			const mesh::GPFMesh::MeshDescriptor& meshDesc = m_crate.mesh.meshDescriptorMapByName[shapeName];

			// bind the constant data to the vertex shader
			m_d3dContext->VSSetConstantBuffers(0, 1, m_crate.shapeAttributeMapByName[shapeName].d3dPerObjectCB.GetAddressOf());
			m_d3dContext->PSSetConstantBuffers(0, 1, m_crate.shapeAttributeMapByName[shapeName].d3dPerObjectCB.GetAddressOf());

			//m_d3dContext->VSSetConstantBuffers(5, 1, m_crate.shapeAttributeMapByName[shapeName].d3dPerObjectTextureCB.GetAddressOf());
			//m_d3dContext->PSSetConstantBuffers(5, 1, m_crate.shapeAttributeMapByName[shapeName].d3dPerObjectTextureCB.GetAddressOf());

			m_d3dContext->PSSetShaderResources(0, 1, m_crate.shapeAttributeMapByName[shapeName].d3dShaderResourceViewDiffText.GetAddressOf());
			m_d3dContext->PSSetShaderResources(1, 1, m_crate.shapeAttributeMapByName[shapeName].d3dShaderResourceViewNormalMapText.GetAddressOf());
			m_d3dContext->PSSetShaderResources(2, 1, m_crate.shapeAttributeMapByName[shapeName].d3dShaderResourceViewGlossMapText.GetAddressOf());

			// draw
			m_d3dContext->DrawIndexed(meshDesc.indexCount, meshDesc.indexOffset, meshDesc.vertexOffset);
		}

	}


	XTEST_D3D_CHECK(m_swapChain->Present(0, 0));

	m_d3dAnnotation->EndEvent();
}
