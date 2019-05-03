#include "stdafx.h"
#include "textures_demo_app.h"
#include <file/file_utils.h>
#include <math/math_utils.h>
#include <service/locator.h>
#include <external_libs/directxtk/WICTextureLoader.h>

using namespace DirectX;
using namespace xtest;

using xtest::demo::TexturesDemoApp;
using Microsoft::WRL::ComPtr;

TexturesDemoApp::TexturesDemoApp(HINSTANCE instance,
	const application::WindowSettings& windowSettings,
	const application::DirectxSettings& directxSettings,
	uint32 fps /*=60*/)
	: application::DirectxApp(instance, windowSettings, directxSettings, fps)
	, m_viewMatrix()
	, m_projectionMatrix()
	, m_camera(math::ToRadians(68.f), math::ToRadians(135.f), 7.f, { 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, { math::ToRadians(4.f), math::ToRadians(175.f) }, { 3.f, 25.f })
	, m_dirLight()
	, m_spotLight()
	, m_pointLights()
	, m_renderables()
	, m_controls()
	, m_isControlsDirty(true)
	, m_stopLights(false)
	, m_d3dPerFrameCB(nullptr)
	, m_d3dRarelyChangedCB(nullptr)
	, m_vertexShader(nullptr)
	, m_pixelShader(nullptr)
	, m_inputLayout(nullptr)
	, m_rasterizerState(nullptr)
{}


TexturesDemoApp::~TexturesDemoApp()
{}


void TexturesDemoApp::Init()
{
	application::DirectxApp::Init();

	m_d3dAnnotation->BeginEvent(L"init-demo");

	InitMatrices();
	InitShaders();
	InitRenderable();
	InitTextures();
	InitLights();
	InitRasterizerState();

	service::Locator::GetMouse()->AddListener(this);
	service::Locator::GetKeyboard()->AddListener(this, {input::Key::F, input::Key::F1, input::Key::F2, input::Key::F3,
														input::Key::F4, input::Key::F5, input::Key::F6, input::Key::space_bar });

	m_d3dAnnotation->EndEvent();
}


void TexturesDemoApp::InitMatrices()
{
	// view matrix
	XMStoreFloat4x4(&m_viewMatrix, m_camera.GetViewMatrix());

	// projection matrix
	{
		XMMATRIX P = XMMatrixPerspectiveFovLH(math::ToRadians(45.f), AspectRatio(), 1.f, 1000.f);
		XMStoreFloat4x4(&m_projectionMatrix, P);
	}
}


void TexturesDemoApp::InitTextures() 
{
	// Texture plane
	{
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\tiles\\tiles_color.png").c_str(), m_renderables[0].texture.GetAddressOf(), m_renderables[0].textureView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\tiles\\tiles_norm.png").c_str(), m_renderables[0].normalMap.GetAddressOf(), m_renderables[0].normalMapView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\tiles\\tiles_gloss.png").c_str(), m_renderables[0].glossMap.GetAddressOf(), m_renderables[0].glossMapView.GetAddressOf()));
	}
	// Texture column 1
	{
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\fabric\\fabric_color.png").c_str(), m_renderables[1].texture.GetAddressOf(), m_renderables[1].textureView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\jeans\\jeans_color.png").c_str(), m_renderables[1].movingTexture.GetAddressOf(), m_renderables[1].movingTextureView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\fabric\\fabric_norm.png").c_str(), m_renderables[1].normalMap.GetAddressOf(), m_renderables[1].normalMapView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\fabric\\fabric_gloss.png").c_str(), m_renderables[1].glossMap.GetAddressOf(), m_renderables[1].glossMapView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\fabric\\fabric_color.png").c_str(), m_renderables[2].texture.GetAddressOf(), m_renderables[2].textureView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\jeans\\jeans_color.png").c_str(), m_renderables[2].movingTexture.GetAddressOf(), m_renderables[2].movingTextureView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\fabric\\fabric_norm.png").c_str(), m_renderables[2].normalMap.GetAddressOf(), m_renderables[2].normalMapView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\fabric\\fabric_gloss.png").c_str(), m_renderables[2].glossMap.GetAddressOf(), m_renderables[2].glossMapView.GetAddressOf()));
	}
	// Texture column 2
	{
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\lizard\\lizard_color.png").c_str(), m_renderables[3].texture.GetAddressOf(), m_renderables[3].textureView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\lizard\\lizard_norm.png").c_str(), m_renderables[3].normalMap.GetAddressOf(), m_renderables[3].normalMapView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\lizard\\lizard_gloss.png").c_str(), m_renderables[3].glossMap.GetAddressOf(), m_renderables[3].glossMapView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\lizard\\lizard_color.png").c_str(), m_renderables[4].texture.GetAddressOf(), m_renderables[4].textureView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\lizard\\lizard_norm.png").c_str(), m_renderables[4].normalMap.GetAddressOf(), m_renderables[4].normalMapView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\lizard\\lizard_gloss.png").c_str(), m_renderables[4].glossMap.GetAddressOf(), m_renderables[4].glossMapView.GetAddressOf()));
	}
	// Texture column 3
	{
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\plastic-cover\\plastic_cover_color.png").c_str(), m_renderables[5].texture.GetAddressOf(), m_renderables[5].textureView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\plastic-cover\\plastic_cover_norm.png").c_str(), m_renderables[5].normalMap.GetAddressOf(), m_renderables[5].normalMapView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\plastic-cover\\plastic_cover_gloss.png").c_str(), m_renderables[5].glossMap.GetAddressOf(), m_renderables[5].glossMapView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\plastic-cover\\plastic_cover_color.png").c_str(), m_renderables[6].texture.GetAddressOf(), m_renderables[6].textureView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\plastic-cover\\plastic_cover_norm.png").c_str(), m_renderables[6].normalMap.GetAddressOf(), m_renderables[6].normalMapView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\plastic-cover\\plastic_cover_gloss.png").c_str(), m_renderables[6].glossMap.GetAddressOf(), m_renderables[6].glossMapView.GetAddressOf()));
	}
	// Texture column 4
	{
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\twine\\twine_color.png").c_str(), m_renderables[7].texture.GetAddressOf(), m_renderables[7].textureView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\twine\\twine_norm.png").c_str(), m_renderables[7].normalMap.GetAddressOf(), m_renderables[7].normalMapView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\twine\\twine_gloss.png").c_str(), m_renderables[7].glossMap.GetAddressOf(), m_renderables[7].glossMapView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\twine\\twine_color.png").c_str(), m_renderables[8].texture.GetAddressOf(), m_renderables[8].textureView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\twine\\twine_norm.png").c_str(), m_renderables[8].normalMap.GetAddressOf(), m_renderables[8].normalMapView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\twine\\twine_gloss.png").c_str(), m_renderables[8].glossMap.GetAddressOf(), m_renderables[8].glossMapView.GetAddressOf()));
	}
	// Texture column 5
	{
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\wet-stone\\wet_stone_color.png").c_str(), m_renderables[9].texture.GetAddressOf(), m_renderables[9].textureView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\wet-stone\\wet_stone_norm.png").c_str(), m_renderables[9].normalMap.GetAddressOf(), m_renderables[9].normalMapView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\wet-stone\\wet_stone_gloss.png").c_str(), m_renderables[9].glossMap.GetAddressOf(), m_renderables[9].glossMapView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\wet-stone\\wet_stone_color.png").c_str(), m_renderables[10].texture.GetAddressOf(), m_renderables[10].textureView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\wet-stone\\wet_stone_norm.png").c_str(), m_renderables[10].normalMap.GetAddressOf(), m_renderables[10].normalMapView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\wet-stone\\wet_stone_gloss.png").c_str(), m_renderables[10].glossMap.GetAddressOf(), m_renderables[10].glossMapView.GetAddressOf()));
	}
	// Texture column 6
	{
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\wood\\wood_color.png").c_str(), m_renderables[11].texture.GetAddressOf(), m_renderables[11].textureView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\wood\\wood_norm.png").c_str(), m_renderables[11].normalMap.GetAddressOf(), m_renderables[11].normalMapView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\wood\\wood_gloss.png").c_str(), m_renderables[11].glossMap.GetAddressOf(), m_renderables[11].glossMapView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\wood\\wood_color.png").c_str(), m_renderables[12].texture.GetAddressOf(), m_renderables[12].textureView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\wood\\wood_norm.png").c_str(), m_renderables[12].normalMap.GetAddressOf(), m_renderables[12].normalMapView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\wood\\wood_gloss.png").c_str(), m_renderables[12].glossMap.GetAddressOf(), m_renderables[12].glossMapView.GetAddressOf()));
	}
	// Texture carpet
	{
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\coat\\coat_color.png").c_str(), m_renderables[13].texture.GetAddressOf(), m_renderables[13].textureView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\coat\\coat_norm.png").c_str(), m_renderables[13].normalMap.GetAddressOf(), m_renderables[13].normalMapView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\coat\\coat_gloss.png").c_str(), m_renderables[13].glossMap.GetAddressOf(), m_renderables[13].glossMapView.GetAddressOf()));
	}

	// Texture torus
	{
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\ground\\ground_color.png").c_str(), m_renderables[14].texture.GetAddressOf(), m_renderables[14].textureView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\ground\\ground_norm.png").c_str(), m_renderables[14].normalMap.GetAddressOf(), m_renderables[14].normalMapView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), std::wstring(GetRootDir()).append(L"\\3d-objects\\ground\\ground_gloss.png").c_str(), m_renderables[14].glossMap.GetAddressOf(), m_renderables[14].glossMapView.GetAddressOf()));
	}

	// Sampler
	{
		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
		samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MaxAnisotropy = 16;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		XTEST_D3D_CHECK(m_d3dDevice->CreateSamplerState(&samplerDesc, &m_textureSampler));
	}
}


void TexturesDemoApp::InitShaders()
{
	// read pre-compiled shaders' bytecode
	std::future<file::BinaryFile> psByteCodeFuture = file::ReadBinaryFile(std::wstring(GetRootDir()).append(L"\\textures_demo_PS.cso"));
	std::future<file::BinaryFile> vsByteCodeFuture = file::ReadBinaryFile(std::wstring(GetRootDir()).append(L"\\textures_demo_VS.cso"));

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
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(mesh::MeshData::Vertex, uv), D3D11_INPUT_PER_VERTEX_DATA, 0 }
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
}


void xtest::demo::TexturesDemoApp::InitRenderable()
{

	// plane
	{
		// geo
		Renderable plane;
		plane.mesh = mesh::GeneratePlane(15.f, 15.f, 30, 30);

		// W
		XMStoreFloat4x4(&plane.W, XMMatrixIdentity());
		XMStoreFloat4x4(&plane.textureMatrix, XMMatrixIdentity());

		// material
		plane.material.ambient = { 0.15f, 0.15f, 0.15f, 1.f };
		plane.material.diffuse = { 0.52f, 0.52f, 0.52f, 1.f };
		plane.material.specular = { 0.5f, 0.5f, 0.5f, 190.0f };

		// perObjectCB
		D3D11_BUFFER_DESC perObjectCBDesc;
		perObjectCBDesc.Usage = D3D11_USAGE_DYNAMIC;
		perObjectCBDesc.ByteWidth = sizeof(PerObjectCB);
		perObjectCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		perObjectCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		perObjectCBDesc.MiscFlags = 0;
		perObjectCBDesc.StructureByteStride = 0;
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&perObjectCBDesc, nullptr, &plane.d3dPerObjectCB));

		// vertex buffer
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.ByteWidth = UINT(sizeof(mesh::MeshData::Vertex) * plane.mesh.vertices.size());
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexInitData;
		vertexInitData.pSysMem = &plane.mesh.vertices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexInitData, &plane.d3dVertexBuffer));
		
		// index buffer
		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.ByteWidth = UINT(sizeof(uint32) * plane.mesh.indices.size());
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexInitdata;
		indexInitdata.pSysMem = &plane.mesh.indices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &indexInitdata, &plane.d3dIndexBuffer));

		m_renderables.push_back(plane);
	}

	// columns
	for (unsigned int i = 0; i < 6; i++) {
		{
			// geo
			Renderable column;
			column.mesh = mesh::GenerateBox(1.0f, 2.0f, 1.0f);

			// W
			if (i == 0) 
			{
				XMStoreFloat4x4(&column.W, XMMatrixTranslation(0.0f, 1.0f, 0.0f));				
				XMStoreFloat4x4(&column.movingTextureMatrix, XMMatrixIdentity());
				column.hasMovingTexture = true;
			}
			else 
			{
				float x = cos((DirectX::XM_PI * 2 / 5) * i) * 5.0f;
				float y = 1.0f;
				float z = sin((DirectX::XM_PI * 2 / 5) * i) * 5.0f;
				XMStoreFloat4x4(&column.W, XMMatrixTranslation(x, y, z));
			}
			XMStoreFloat4x4(&column.textureMatrix, XMMatrixIdentity());

			// material
			column.material.ambient = { 0.7f, 0.7f, 0.7f, 1.0f };
			column.material.diffuse = { 0.81f, 0.8f, 0.8f, 1.0f };
			column.material.specular = { 0.7f, 0.7f, 0.7f, 60.0f };

			// perObjectCB
			D3D11_BUFFER_DESC perObjectCBDesc;
			perObjectCBDesc.Usage = D3D11_USAGE_DYNAMIC;
			perObjectCBDesc.ByteWidth = sizeof(PerObjectCB);
			perObjectCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			perObjectCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			perObjectCBDesc.MiscFlags = 0;
			perObjectCBDesc.StructureByteStride = 0;
			XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&perObjectCBDesc, nullptr, &column.d3dPerObjectCB));

			// vertex buffer
			D3D11_BUFFER_DESC vertexBufferDesc;
			vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
			vertexBufferDesc.ByteWidth = UINT(sizeof(mesh::MeshData::Vertex) * column.mesh.vertices.size());
			vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			vertexBufferDesc.CPUAccessFlags = 0;
			vertexBufferDesc.MiscFlags = 0;
			vertexBufferDesc.StructureByteStride = 0;

			D3D11_SUBRESOURCE_DATA vertexInitData;
			vertexInitData.pSysMem = &column.mesh.vertices[0];
			XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexInitData, &column.d3dVertexBuffer));

			// index buffer
			D3D11_BUFFER_DESC indexBufferDesc;
			indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
			indexBufferDesc.ByteWidth = UINT(sizeof(uint32) * column.mesh.indices.size());
			indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			indexBufferDesc.CPUAccessFlags = 0;
			indexBufferDesc.MiscFlags = 0;
			indexBufferDesc.StructureByteStride = 0;

			D3D11_SUBRESOURCE_DATA indexInitdata;
			indexInitdata.pSysMem = &column.mesh.indices[0];
			XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &indexInitdata, &column.d3dIndexBuffer));

			m_renderables.push_back(column);
		}

		// sphere
		{
			//geo
			Renderable sphere;
			sphere.mesh = mesh::GenerateSphere(0.5f, 40, 40);

			// W
			if (i == 0) 
			{
				XMStoreFloat4x4(&sphere.W, XMMatrixTranslation(0.0f, 2.5f, 0.0f));
				XMStoreFloat4x4(&sphere.movingTextureMatrix, XMMatrixIdentity());
				sphere.hasMovingTexture = true;
			}
			else 
			{
				float x = cos((DirectX::XM_PI * 2 / 5) * i) * 5.0f;
				float y = 2.5f;
				float z = sin((DirectX::XM_PI * 2 / 5) * i) * 5.0f;
				XMStoreFloat4x4(&sphere.W, XMMatrixTranslation(x, y, z));
			}
			XMStoreFloat4x4(&sphere.textureMatrix, XMMatrixIdentity());

			// material
			sphere.material.ambient = { 0.7f, 0.7f, 0.7f, 1.0f };
			sphere.material.diffuse = { 0.81f, 0.8f, 0.8f, 1.0f };
			sphere.material.specular = { 0.7f, 0.7f, 0.7f, 60.0f };

			// perObjectCB
			D3D11_BUFFER_DESC perObjectCBDesc;
			perObjectCBDesc.Usage = D3D11_USAGE_DYNAMIC;
			perObjectCBDesc.ByteWidth = sizeof(PerObjectCB);
			perObjectCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			perObjectCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			perObjectCBDesc.MiscFlags = 0;
			perObjectCBDesc.StructureByteStride = 0;
			XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&perObjectCBDesc, nullptr, &sphere.d3dPerObjectCB));

			// vertex buffer
			D3D11_BUFFER_DESC vertexBufferDesc;
			vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
			vertexBufferDesc.ByteWidth = UINT(sizeof(mesh::MeshData::Vertex) * sphere.mesh.vertices.size());
			vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			vertexBufferDesc.CPUAccessFlags = 0;
			vertexBufferDesc.MiscFlags = 0;
			vertexBufferDesc.StructureByteStride = 0;

			D3D11_SUBRESOURCE_DATA vertexInitData;
			vertexInitData.pSysMem = &sphere.mesh.vertices[0];
			XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexInitData, &sphere.d3dVertexBuffer));

			// index buffer
			D3D11_BUFFER_DESC indexBufferDesc;
			indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
			indexBufferDesc.ByteWidth = UINT(sizeof(uint32) * sphere.mesh.indices.size());
			indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			indexBufferDesc.CPUAccessFlags = 0;
			indexBufferDesc.MiscFlags = 0;
			indexBufferDesc.StructureByteStride = 0;

			D3D11_SUBRESOURCE_DATA indexInitdata;
			indexInitdata.pSysMem = &sphere.mesh.indices[0];
			XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &indexInitdata, &sphere.d3dIndexBuffer));

			m_renderables.push_back(sphere);
		}
	}

	// carpet
	{
		// geo
		Renderable carpet;
		carpet.mesh = mesh::GeneratePlane(12.0f, 12.0f, 30, 30);

		// W
		XMStoreFloat4x4(&carpet.W, XMMatrixTranslation(0.0f, 0.01f, 0.0f));
		XMStoreFloat4x4(&carpet.textureMatrix, XMMatrixIdentity());

		// material
		carpet.material.ambient = { 0.5f, 0.15f, 0.15f, 1.f };
		carpet.material.diffuse = { 0.92f, 0.22f, 0.22f, 1.f };
		carpet.material.specular = { 0.01f, 0.01f, 0.01f, 190.0f };

		// perObjectCB
		D3D11_BUFFER_DESC perObjectCBDesc;
		perObjectCBDesc.Usage = D3D11_USAGE_DYNAMIC;
		perObjectCBDesc.ByteWidth = sizeof(PerObjectCB);
		perObjectCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		perObjectCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		perObjectCBDesc.MiscFlags = 0;
		perObjectCBDesc.StructureByteStride = 0;
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&perObjectCBDesc, nullptr, &carpet.d3dPerObjectCB));

		// vertex buffer
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.ByteWidth = UINT(sizeof(mesh::MeshData::Vertex) * carpet.mesh.vertices.size());
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexInitData;
		vertexInitData.pSysMem = &carpet.mesh.vertices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexInitData, &carpet.d3dVertexBuffer));

		// index buffer
		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.ByteWidth = UINT(sizeof(uint32) * carpet.mesh.indices.size());
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexInitdata;
		indexInitdata.pSysMem = &carpet.mesh.indices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &indexInitdata, &carpet.d3dIndexBuffer));

		m_renderables.push_back(carpet);
	}

	// torus
	{
		// geo
		Renderable torus;
		torus.mesh = mesh::GenerateTorus(1.5f, 0.5f, 30, 30);

		// W
		XMStoreFloat4x4(&torus.W, XMMatrixTranslation(0.0f, 4.0f, 0.0f));
		XMStoreFloat4x4(&torus.textureMatrix, XMMatrixIdentity());

		// material
		torus.material.ambient = { 0.4f, 1.0f, 0.4f, 1.f };
		torus.material.diffuse = { 0.22f, 0.9f, 0.22f, 1.f };
		torus.material.specular = { 0.6f, 1.0f, 0.6f, 190.0f };

		// perObjectCB
		D3D11_BUFFER_DESC perObjectCBDesc;
		perObjectCBDesc.Usage = D3D11_USAGE_DYNAMIC;
		perObjectCBDesc.ByteWidth = sizeof(PerObjectCB);
		perObjectCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		perObjectCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		perObjectCBDesc.MiscFlags = 0;
		perObjectCBDesc.StructureByteStride = 0;
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&perObjectCBDesc, nullptr, &torus.d3dPerObjectCB));

		// vertex buffer
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.ByteWidth = UINT(sizeof(mesh::MeshData::Vertex) * torus.mesh.vertices.size());
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexInitData;
		vertexInitData.pSysMem = &torus.mesh.vertices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexInitData, &torus.d3dVertexBuffer));

		// index buffer
		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.ByteWidth = UINT(sizeof(uint32) * torus.mesh.indices.size());
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexInitdata;
		indexInitdata.pSysMem = &torus.mesh.indices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &indexInitdata, &torus.d3dIndexBuffer));

		m_renderables.push_back(torus);
	}
}


void TexturesDemoApp::InitLights()
{
	m_dirLight.ambient = { 0.2f, 0.2f, 0.2f, 1.f };
	m_dirLight.diffuse = { 0.2f, 0.2f, 0.2f, 1.f };
	m_dirLight.specular = { 0.2f, 0.2f, 0.2f, 1.f };
	XMVECTOR dirLightDirection = XMVector3Normalize(-XMVectorSet(5.f, 3.f, 5.f, 0.f));
	XMStoreFloat3(&m_dirLight.dirW, dirLightDirection);

	for (int i = 0; i < 5; i++) {
		float x = cos((DirectX::XM_PI * 2 / 5) * i) * 3.0f;
		float y = 1.0f;
		float z = sin((DirectX::XM_PI * 2 / 5) * i) * 3.0f;
		m_pointLights[i].ambient = { 0.2f, 0.1f, 0.1f, 1.0f };
		m_pointLights[i].diffuse = { 0.4f, 0.10f, 0.10f, 1.0f };
		m_pointLights[i].specular = { 0.4f, 0.10f, 0.10f, 1.0f };
		m_pointLights[i].posW = { x, y, z };
		m_pointLights[i].range = 8.f;
		m_pointLights[i].attenuation = { 0.0f, 0.3f, 0.f };
	}

	m_spotLight.ambient = { 0.018f, 0.018f, 1.8f, 1.0f };
	m_spotLight.diffuse = { 0.1f, 0.1f, 1.0f, 1.0f };
	m_spotLight.specular = { 0.1f, 0.1f, 1.0f, 1.0f };
	XMVECTOR posW = XMVectorSet(5.f, 5.f, -5.f, 1.f);
	XMStoreFloat3(&m_spotLight.posW, posW);
	m_spotLight.range = 50.f;
	XMVECTOR dirW = XMVector3Normalize(XMVectorSet(-4.f, 1.f, 0.f, 1.f) - posW);
	XMStoreFloat3(&m_spotLight.dirW, dirW);
	m_spotLight.spot = 30.f;
	m_spotLight.attenuation = { 0.0f, 0.1f, 0.f };
	
	m_controls.useDirLight = true;
	m_controls.usePointLight = true;
	m_controls.useSpotLight = true;
	m_controls.useDiffuseTexture = true;
	m_controls.useNormalTexture = true;
	m_controls.useGlossTexture = true;
	   
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
		lightControlData.pSysMem = &m_controls;
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&rarelyChangedCB, &lightControlData, &m_d3dRarelyChangedCB));
	}
}


void TexturesDemoApp::InitRasterizerState()
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


void TexturesDemoApp::OnResized()
{
	application::DirectxApp::OnResized();

	//update the projection matrix with the new aspect ratio
	XMMATRIX P = XMMatrixPerspectiveFovLH(math::ToRadians(45.f), AspectRatio(), 1.f, 1000.f);
	XMStoreFloat4x4(&m_projectionMatrix, P);
}


void TexturesDemoApp::OnWheelScroll(input::ScrollStatus scroll)
{
	// zoom in or out when the scroll wheel is used
	if (service::Locator::GetMouse()->IsInClientArea())
	{
		m_camera.IncreaseRadiusBy(scroll.isScrollingUp ? -0.5f : 0.5f);
	}
}


void TexturesDemoApp::OnMouseMove(const DirectX::XMINT2& movement, const DirectX::XMINT2& currentPos)
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


void TexturesDemoApp::OnKeyStatusChange(input::Key key, const input::KeyStatus& status)
{

	// re-frame F key is pressed
	if (key == input::Key::F && status.isDown)
	{
		m_camera.SetPivot({ 0.f, 0.f, 0.f });
	}
	else if (key == input::Key::F1 && status.isDown)
	{
		m_controls.useDirLight = !m_controls.useDirLight;
		m_isControlsDirty = true;
	}
	else if (key == input::Key::F2 && status.isDown)
	{
		m_controls.usePointLight = !m_controls.usePointLight;
		m_isControlsDirty = true;
	}
	else if (key == input::Key::F3 && status.isDown)
	{
		m_controls.useSpotLight = !m_controls.useSpotLight;
		m_isControlsDirty = true;
	}
	else if (key == input::Key::F4 && status.isDown)
	{
		m_controls.useDiffuseTexture = !m_controls.useDiffuseTexture;
		m_isControlsDirty = true;
	}
	else if (key == input::Key::F5 && status.isDown)
	{
		m_controls.useNormalTexture = !m_controls.useNormalTexture;
		m_isControlsDirty = true;
	}
	else if (key == input::Key::F6 && status.isDown)
	{
		m_controls.useGlossTexture = !m_controls.useGlossTexture;
		m_isControlsDirty = true;
	}
	else if (key == input::Key::space_bar && status.isDown)
	{
		m_stopLights = !m_stopLights;
	}
}


void TexturesDemoApp::UpdateScene(float deltaSeconds)
{
	XTEST_UNUSED_VAR(deltaSeconds);


	// create the model-view-projection matrix
	XMMATRIX V = m_camera.GetViewMatrix();
	XMStoreFloat4x4(&m_viewMatrix, V);

	// create projection matrix
	XMMATRIX P = XMLoadFloat4x4(&m_projectionMatrix);
	   
	m_d3dAnnotation->BeginEvent(L"update-constant-buffer");

	// PerObjectCB
	for(unsigned int i = 0; i < m_renderables.size(); i++) 
	{
		XMMATRIX W = XMLoadFloat4x4(&m_renderables[i].W);
		if (i == m_renderables.size() - 1) {
			XMMATRIX R = XMMatrixRotationY(math::ToRadians(120.0f) * deltaSeconds);
			W = W * R;
			XMStoreFloat4x4(&m_renderables[i].W, W);
		}

		XMMATRIX WVP = W * V * P;
		XMMATRIX TextureMatrix = XMLoadFloat4x4(&m_renderables[i].textureMatrix);

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		// disable gpu access
		XTEST_D3D_CHECK(m_d3dContext->Map(m_renderables[i].d3dPerObjectCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		PerObjectCB* perObjectCB = (PerObjectCB*)mappedResource.pData;

		// update the data
		XMStoreFloat4x4(&perObjectCB->W, XMMatrixTranspose(W));
		XMStoreFloat4x4(&perObjectCB->WVP, XMMatrixTranspose(WVP));
		XMStoreFloat4x4(&perObjectCB->W_inverseTraspose, XMMatrixInverse(nullptr, W));
		XMStoreFloat4x4(&perObjectCB->TexcoordMatrix, XMMatrixTranspose(TextureMatrix));
		
		// update the moving texture, if there is one
		perObjectCB->hasMovingTexture = false;
		if (m_renderables[i].hasMovingTexture) {
			perObjectCB->hasMovingTexture = true;
			XMMATRIX MovingTextureMatrix = XMLoadFloat4x4(&m_renderables[i].movingTextureMatrix);
			MovingTextureMatrix *= XMMatrixTranslation(0.0f, - 0.8f * deltaSeconds, 0.0f);
			XMStoreFloat4x4(&m_renderables[i].movingTextureMatrix, MovingTextureMatrix);
			XMStoreFloat4x4(&perObjectCB->MovingTexcoordMatrix, XMMatrixTranspose(MovingTextureMatrix));
		}

		perObjectCB->material = m_renderables[i].material;

		// enable gpu access
		m_d3dContext->Unmap(m_renderables[i].d3dPerObjectCB.Get(), 0);
	}

	// PerFrameCB
	{

		if (!m_stopLights)
		{
			XMMATRIX R = XMMatrixRotationY(math::ToRadians(30.f) * deltaSeconds);
			for (int i = 0; i < 5; i++) {
				XMStoreFloat3(&m_pointLights[i].posW, XMVector3Transform(XMLoadFloat3(&m_pointLights[i].posW), R));
			}
		}

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		// disable gpu access
		XTEST_D3D_CHECK(m_d3dContext->Map(m_d3dPerFrameCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		PerFrameCB* perFrameCB = (PerFrameCB*)mappedResource.pData;

		//update the data
		perFrameCB->dirLight = m_dirLight;
		perFrameCB->spotLight = m_spotLight;
		perFrameCB->pointLights = m_pointLights;
		perFrameCB->eyePosW = m_camera.GetPosition();

		// enable gpu access
		m_d3dContext->Unmap(m_d3dPerFrameCB.Get(), 0);
	}

	// RarelyChangedCB
	{
		if (m_isControlsDirty)
		{
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

			// disable gpu access
			XTEST_D3D_CHECK(m_d3dContext->Map(m_d3dRarelyChangedCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
			RarelyChangedCB* rarelyChangedCB = (RarelyChangedCB*)mappedResource.pData;

			//update the data
			rarelyChangedCB->useDirLight = m_controls.useDirLight;
			rarelyChangedCB->usePointLight = m_controls.usePointLight;
			rarelyChangedCB->useSpotLight = m_controls.useSpotLight;
			rarelyChangedCB->useDiffuseTexture = m_controls.useDiffuseTexture;
			rarelyChangedCB->useNormalTexture = m_controls.useNormalTexture;
			rarelyChangedCB->useGlossTexture = m_controls.useGlossTexture;

			// enable gpu access
			m_d3dContext->Unmap(m_d3dRarelyChangedCB.Get(), 0);

			m_d3dContext->PSSetConstantBuffers(2, 1, m_d3dRarelyChangedCB.GetAddressOf());
			m_isControlsDirty = false;

		}
	}

	m_d3dAnnotation->EndEvent();
}


void TexturesDemoApp::RenderScene()
{
	m_d3dAnnotation->BeginEvent(L"render-scene");

	// clear the frame
	m_d3dContext->ClearDepthStencilView(m_depthBufferView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
	m_d3dContext->ClearRenderTargetView(m_backBufferView.Get(), DirectX::Colors::Black);

	// set the shaders and the input layout
	m_d3dContext->RSSetState(m_rasterizerState.Get());
	m_d3dContext->IASetInputLayout(m_inputLayout.Get());
	m_d3dContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	m_d3dContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	m_d3dContext->PSSetConstantBuffers(1, 1, m_d3dPerFrameCB.GetAddressOf());
	m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_d3dContext->PSSetSamplers(0, 1, m_textureSampler.GetAddressOf());

	// draw
	for (unsigned int i = 0; i < m_renderables.size(); i++) {
		// bind the constant data to the vertex shader
		m_d3dContext->VSSetConstantBuffers(0, 1, m_renderables[i].d3dPerObjectCB.GetAddressOf());
		m_d3dContext->PSSetConstantBuffers(0, 1, m_renderables[i].d3dPerObjectCB.GetAddressOf());

		// bind the textures
		m_d3dContext->PSSetShaderResources(0, 1, m_renderables[i].textureView.GetAddressOf());
		m_d3dContext->PSSetShaderResources(1, 1, m_renderables[i].normalMapView.GetAddressOf());
		m_d3dContext->PSSetShaderResources(2, 1, m_renderables[i].glossMapView.GetAddressOf());
		
		if (m_renderables[i].hasMovingTexture) {
			m_d3dContext->PSSetShaderResources(3, 1, m_renderables[i].movingTextureView.GetAddressOf());
		}

		// set what to draw
		UINT stride = sizeof(mesh::MeshData::Vertex);
		UINT offset = 0;
		m_d3dContext->IASetVertexBuffers(0, 1, m_renderables[i].d3dVertexBuffer.GetAddressOf(), &stride, &offset);
		m_d3dContext->IASetIndexBuffer(m_renderables[i].d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		m_d3dContext->DrawIndexed(UINT(m_renderables[i].mesh.indices.size()), 0, 0);
	}
		
	XTEST_D3D_CHECK(m_swapChain->Present(0, 0));

	m_d3dAnnotation->EndEvent();
}

