#include "stdafx.h"
#include "textures_demo_app.h"
#include <file/file_utils.h>
#include <math/math_utils.h>
#include <service/locator.h>
#include <external_libs\directxtk\WICTextureLoader.h>

using namespace DirectX;
using namespace xtest;

using xtest::demo::TextureDemoApp;
using Microsoft::WRL::ComPtr;

TextureDemoApp::TextureDemoApp(HINSTANCE instance,
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
	, m_sphere()
	, m_plane()
	, m_crate()
	, m_lightsControl()
	, m_isLightControlDirty(true)
	, m_stopLights(false)
	, m_d3dPerFrameCB(nullptr)
	, m_d3dRarelyChangedCB(nullptr)
	, m_vertexShader(nullptr)
	, m_pixelShader(nullptr)
	, m_inputLayout(nullptr)
	, m_rasterizerState(nullptr)
{}


TextureDemoApp::~TextureDemoApp()
{}


void TextureDemoApp::Init()
{
	application::DirectxApp::Init();

	m_d3dAnnotation->BeginEvent(L"init-demo");

	InitMatrices();
	InitShaders();
	InitRenderable();
	InitLights();
	InitRasterizerState();

	service::Locator::GetMouse()->AddListener(this);
	service::Locator::GetKeyboard()->AddListener(this, { input::Key::F, input::Key::F1, input::Key::F2, input::Key::F3, input::Key::space_bar });

	m_d3dAnnotation->EndEvent();
}


void TextureDemoApp::InitMatrices()
{
	// view matrix
	XMStoreFloat4x4(&m_viewMatrix, m_camera.GetViewMatrix());

	// projection matrix
	{
		XMMATRIX P = XMMatrixPerspectiveFovLH(math::ToRadians(45.f), AspectRatio(), 1.f, 50.f);
		XMStoreFloat4x4(&m_projectionMatrix, P);
	}
}


void TextureDemoApp::InitShaders()
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
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT,	0, offsetof(xtest::mesh::MeshData::Vertex, normal),		D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT,	0, offsetof(xtest::mesh::MeshData::Vertex, tangentU),	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,		0, offsetof(xtest::mesh::MeshData::Vertex, uv),			D3D11_INPUT_PER_VERTEX_DATA, 0 }
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


	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc,sizeof(D3D11_SAMPLER_DESC));

	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	XTEST_D3D_CHECK(m_d3dDevice->CreateSamplerState(&samplerDesc,&m_textureSampler));
}


void xtest::demo::TextureDemoApp::InitRenderable()
{

	//TORUS
	{
		m_torus.mesh = mesh::GenerateTorus(4, 0.2f,25);
		CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), GetRootDir().append(LR"(\3d-objects\wood\wood_color.png)").c_str(), NULL, &m_torus.diffuse_texture_view, NULL);
		CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), GetRootDir().append(LR"(\3d-objects\wood\wood_norm.png)").c_str(), NULL, &m_torus.normal_texture_view, NULL);
		CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), GetRootDir().append(LR"(\3d-objects\wood\wood_gloss.png)").c_str(), NULL, &m_torus.gloss_texture_view, NULL);

		// W
		XMStoreFloat4x4(&m_torus.W, XMMatrixMultiply(XMMatrixScaling(1.0f,1.0f, 1.0f), XMMatrixTranslation(0.f, 1.f, 0.f)));

		// material
		m_torus.material.ambient  = { 0.15f, 0.15f, 0.15f, 1.f };
		m_torus.material.diffuse  = { 0.77f, 0.77f, 0.77f, 1.f };
		m_torus.material.specular = { 0.8f, 0.8f, 0.8f, 190.0f };
		m_torus.material.options  = { 1,0.8f,0,0 };

		// perObjectCB
		D3D11_BUFFER_DESC perObjectCBDesc;
		perObjectCBDesc.Usage = D3D11_USAGE_DYNAMIC;
		perObjectCBDesc.ByteWidth = sizeof(PerObjectCB);
		perObjectCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		perObjectCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		perObjectCBDesc.MiscFlags = 0;
		perObjectCBDesc.StructureByteStride = 0;
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&perObjectCBDesc, nullptr, &m_torus.d3dPerObjectCB));


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
	}

	//TORUS KNOT
	{
		m_torus_knot.mesh = mesh::GenerateTorusKnot(2.0f, 10, 0.05, 1000, 20, 1);
		// W
		XMStoreFloat4x4(&m_torus_knot.W, XMMatrixMultiply(XMMatrixRotationRollPitchYaw(55.f, 0, 0.0f), XMMatrixMultiply(XMMatrixScaling(1.0f, 1.0f, 1.0f), XMMatrixTranslation(0.f, 8.f, 0.f))));

		// material
		m_torus_knot.material.ambient = { 0.15f, 0.15f, 0.15f, 1.f };
		m_torus_knot.material.diffuse = { 0.77f, 0.77f, 0.77f, 1.f };
		m_torus_knot.material.specular = { 0.8f, 0.8f, 0.8f, 190.0f };
		m_torus_knot.material.options = { 0 ,0 ,0 ,0 };

		// perObjectCB
		D3D11_BUFFER_DESC perObjectCBDesc;
		perObjectCBDesc.Usage = D3D11_USAGE_DYNAMIC;
		perObjectCBDesc.ByteWidth = sizeof(PerObjectCB);
		perObjectCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		perObjectCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		perObjectCBDesc.MiscFlags = 0;
		perObjectCBDesc.StructureByteStride = 0;
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&perObjectCBDesc, nullptr, &m_torus_knot.d3dPerObjectCB));


		// vertex buffer
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.ByteWidth = UINT(sizeof(mesh::MeshData::Vertex) * m_torus_knot.mesh.vertices.size());
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexInitData;
		vertexInitData.pSysMem = &m_torus_knot.mesh.vertices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexInitData, &m_torus_knot.d3dVertexBuffer));


		// index buffer
		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.ByteWidth = UINT(sizeof(uint32) * m_torus_knot.mesh.indices.size());
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexInitdata;
		indexInitdata.pSysMem = &m_torus_knot.mesh.indices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &indexInitdata, &m_torus_knot.d3dIndexBuffer));
	}

	//TORUS KNOT 2
	{
		m_torus_knot2.mesh = mesh::GenerateTorusKnot(1.0f, 4.0f, 0.5f, 500, 11, 3);
		
		// W
		XMStoreFloat4x4(&m_torus_knot2.W, XMMatrixMultiply(XMMatrixRotationRollPitchYaw(55.f, 0, 0.0f), XMMatrixMultiply(XMMatrixScaling(1.0f, 1.0f, 1.0f), XMMatrixTranslation(0.f, 8.f, 0.f))));

		// material
		m_torus_knot2.material.ambient = { 0.15f, 0.15f, 0.15f, 1.f };
		m_torus_knot2.material.diffuse = { 0.77f, 0.77f, 0.77f, 1.f };
		m_torus_knot2.material.specular = { 0.8f, 0.8f, 0.8f, 190.0f };
		m_torus_knot2.material.options = { 0 , 0 , 0, 0 };

		// perObjectCB
		D3D11_BUFFER_DESC perObjectCBDesc;
		perObjectCBDesc.Usage = D3D11_USAGE_DYNAMIC;
		perObjectCBDesc.ByteWidth = sizeof(PerObjectCB);
		perObjectCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		perObjectCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		perObjectCBDesc.MiscFlags = 0;
		perObjectCBDesc.StructureByteStride = 0;
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&perObjectCBDesc, nullptr, &m_torus_knot2.d3dPerObjectCB));


		// vertex buffer
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.ByteWidth = UINT(sizeof(mesh::MeshData::Vertex) * m_torus_knot2.mesh.vertices.size());
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexInitData;
		vertexInitData.pSysMem = &m_torus_knot2.mesh.vertices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexInitData, &m_torus_knot2.d3dVertexBuffer));


		// index buffer
		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.ByteWidth = UINT(sizeof(uint32) * m_torus_knot2.mesh.indices.size());
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexInitdata;
		indexInitdata.pSysMem = &m_torus_knot2.mesh.indices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &indexInitdata, &m_torus_knot2.d3dIndexBuffer));
	}



	// plane
	{
		// geo
		m_plane.mesh = mesh::GeneratePlane(50.f, 50.f, 50, 50);
		//CreateWICTextureFromFile(m_d3dDevice.Get(), GetRootDir().append(LR"(\3d-objects\tiles\tiles_color.png)").c_str(), NULL, &m_plane.diffuse_texture_view, NULL);
		//CreateWICTextureFromFile(m_d3dDevice.Get(), GetRootDir().append(LR"(\3d-objects\tiles\tiles_norm.png)").c_str(), NULL, &m_plane.normal_texture_view, NULL);
		//CreateWICTextureFromFile(m_d3dDevice.Get(), GetRootDir().append(LR"(\3d-objects\tiles\tiles_gloss.png)").c_str(), NULL, &m_plane.gloss_texture_view, NULL);
		CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), GetRootDir().append(LR"(\3d-objects\tiles\tiles_color.png)").c_str(), NULL, &m_plane.diffuse_texture_view, NULL);
		CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), GetRootDir().append(LR"(\3d-objects\tiles\tiles_norm.png)").c_str(), NULL, &m_plane.normal_texture_view, NULL);
		CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), GetRootDir().append(LR"(\3d-objects\tiles\tiles_gloss.png)").c_str(), NULL, &m_plane.gloss_texture_view, NULL);


		// W
		XMStoreFloat4x4(&m_plane.W, XMMatrixIdentity());


		// material
		m_plane.material.ambient = { 0.15f, 0.15f, 0.15f, 1.f };
		m_plane.material.diffuse = { 0.77f, 0.77f, 0.77f, 1.f };
		m_plane.material.specular = { 0.8f, 0.8f, 0.8f, 190.0f };
		m_plane.material.options = { 1,20,0,0 };

		// perObjectCB
		D3D11_BUFFER_DESC perObjectCBDesc;
		perObjectCBDesc.Usage = D3D11_USAGE_DYNAMIC;
		perObjectCBDesc.ByteWidth = sizeof(PerObjectCB);
		perObjectCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		perObjectCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		perObjectCBDesc.MiscFlags = 0;
		perObjectCBDesc.StructureByteStride = 0;
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&perObjectCBDesc, nullptr, &m_plane.d3dPerObjectCB));


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
	}


	// cube 1
	{
		// geo
		m_box1.mesh = mesh::GenerateBox(1.0f, 10.f, 1.f);
		CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), GetRootDir().append(LR"(\3d-objects\lizard\lizard_color.png)").c_str(), NULL, &m_box1.diffuse_texture_view, NULL);
		CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), GetRootDir().append(LR"(\3d-objects\lizard\lizard_norm.png)").c_str(), NULL, &m_box1.normal_texture_view, NULL);
		CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), GetRootDir().append(LR"(\3d-objects\lizard\lizard_gloss.png)").c_str(), NULL, &m_box1.gloss_texture_view, NULL);

		CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), GetRootDir().append(LR"(\3d-objects\plasma.jpg)").c_str(), NULL, &m_box1.animated_texture_view, NULL);

		// W
		XMStoreFloat4x4(&m_box1.W, XMMatrixMultiply(XMMatrixScaling(1.0f, 1.0f, 1.0f), XMMatrixTranslation(8.f, 5.f, 8.f)));

		// material
		m_box1.material.ambient = { 0.15f , 0.15f, 0.15f, 1.f };
		m_box1.material.diffuse = { 0.77f , 0.77f, 0.77f, 1.f };
		m_box1.material.specular = { 0.8f  , 0.8f , 0.8f , 190.0f };
		m_box1.material.options = { 3, 1.2f, 0    , 0 };

		// perObjectCB
		D3D11_BUFFER_DESC perObjectCBDesc;
		perObjectCBDesc.Usage = D3D11_USAGE_DYNAMIC;
		perObjectCBDesc.ByteWidth = sizeof(PerObjectCB);
		perObjectCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		perObjectCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		perObjectCBDesc.MiscFlags = 0;
		perObjectCBDesc.StructureByteStride = 0;
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&perObjectCBDesc, nullptr, &m_box1.d3dPerObjectCB));


		// vertex buffer
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.ByteWidth = UINT(sizeof(mesh::MeshData::Vertex) * m_box1.mesh.vertices.size());
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexInitData;
		vertexInitData.pSysMem = &m_box1.mesh.vertices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexInitData, &m_box1.d3dVertexBuffer));


		// index buffer
		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.ByteWidth = UINT(sizeof(uint32) * m_box1.mesh.indices.size());
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexInitdata;
		indexInitdata.pSysMem = &m_box1.mesh.indices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &indexInitdata, &m_box1.d3dIndexBuffer));
	}

	// cube 2
	{
		// geo
		m_box2.mesh = mesh::GenerateBox(1.0f, 10.f, 1.f);
		CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), GetRootDir().append(LR"(\3d-objects\lizard\lizard_color.png)").c_str(), NULL, &m_box2.diffuse_texture_view, NULL);
		CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), GetRootDir().append(LR"(\3d-objects\lizard\lizard_norm.png)").c_str(), NULL, &m_box2.normal_texture_view, NULL);
		CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), GetRootDir().append(LR"(\3d-objects\lizard\lizard_gloss.png)").c_str(), NULL, &m_box2.gloss_texture_view, NULL);

		CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), GetRootDir().append(LR"(\3d-objects\waves.jpg)").c_str(), NULL, &m_box2.animated_texture_view, NULL);

		// W
		XMStoreFloat4x4(&m_box2.W, XMMatrixMultiply( XMMatrixScaling(1.0f, 1.0f, 1.0f), XMMatrixTranslation(-8.f, 5.f, 8.f)));

		// material
		m_box2.material.ambient = { 0.15f, 0.15f, 0.15f, 1.f };
		m_box2.material.diffuse = { 0.77f, 0.77f, 0.77f, 1.f };
		m_box2.material.specular = { 0.8f, 0.8f, 0.8f, 190.0f };
		m_box2.material.options = { 3, 1.2f, 0, 0 };

		// perObjectCB
		D3D11_BUFFER_DESC perObjectCBDesc;
		perObjectCBDesc.Usage = D3D11_USAGE_DYNAMIC;
		perObjectCBDesc.ByteWidth = sizeof(PerObjectCB);
		perObjectCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		perObjectCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		perObjectCBDesc.MiscFlags = 0;
		perObjectCBDesc.StructureByteStride = 0;
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&perObjectCBDesc, nullptr, &m_box2.d3dPerObjectCB));


		// vertex buffer
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.ByteWidth = UINT(sizeof(mesh::MeshData::Vertex) * m_box2.mesh.vertices.size());
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexInitData;
		vertexInitData.pSysMem = &m_box2.mesh.vertices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexInitData, &m_box2.d3dVertexBuffer));


		// index buffer
		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.ByteWidth = UINT(sizeof(uint32) * m_box2.mesh.indices.size());
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexInitdata;
		indexInitdata.pSysMem = &m_box2.mesh.indices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &indexInitdata, &m_box2.d3dIndexBuffer));
	}

	// cube 3
	{
		// geo
		m_box3.mesh = mesh::GenerateBox(1.0f, 10.f, 1.f);
		CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), GetRootDir().append(LR"(\3d-objects\lizard\lizard_color.png)").c_str(), NULL, &m_box3.diffuse_texture_view, NULL);
		CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), GetRootDir().append(LR"(\3d-objects\lizard\lizard_norm.png)").c_str(), NULL, &m_box3.normal_texture_view, NULL);
		CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), GetRootDir().append(LR"(\3d-objects\lizard\lizard_gloss.png)").c_str(), NULL, &m_box3.gloss_texture_view, NULL);

		CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), GetRootDir().append(LR"(\3d-objects\waves.jpg)").c_str(), NULL, &m_box3.animated_texture_view, NULL);

		// W
		XMStoreFloat4x4(&m_box3.W, XMMatrixMultiply(XMMatrixScaling(1.0f, 1.0f, 1.0f), XMMatrixTranslation(-8.f, 5.f, -8.f)));

		// material
		m_box3.material.ambient = { 0.15f , 0.15f, 0.15f, 1.f };
		m_box3.material.diffuse = { 0.77f , 0.77f, 0.77f, 1.f };
		m_box3.material.specular = { 0.8f  , 0.8f , 0.8f , 190.0f };
		m_box3.material.options = { 3, 1.2f, 0    , 0 };

		// perObjectCB
		D3D11_BUFFER_DESC perObjectCBDesc;
		perObjectCBDesc.Usage = D3D11_USAGE_DYNAMIC;
		perObjectCBDesc.ByteWidth = sizeof(PerObjectCB);
		perObjectCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		perObjectCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		perObjectCBDesc.MiscFlags = 0;
		perObjectCBDesc.StructureByteStride = 0;
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&perObjectCBDesc, nullptr, &m_box3.d3dPerObjectCB));


		// vertex buffer
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.ByteWidth = UINT(sizeof(mesh::MeshData::Vertex) * m_box3.mesh.vertices.size());
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexInitData;
		vertexInitData.pSysMem = &m_box3.mesh.vertices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexInitData, &m_box3.d3dVertexBuffer));


		// index buffer
		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.ByteWidth = UINT(sizeof(uint32) * m_box3.mesh.indices.size());
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexInitdata;
		indexInitdata.pSysMem = &m_box3.mesh.indices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &indexInitdata, &m_box3.d3dIndexBuffer));
	}

	// cube 4
	{
		// geo
		m_box4.mesh = mesh::GenerateBox(1.0f, 10.f, 1.f);
		CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), GetRootDir().append(LR"(\3d-objects\lizard\lizard_color.png)").c_str(), NULL, &m_box4.diffuse_texture_view, NULL);
		CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), GetRootDir().append(LR"(\3d-objects\lizard\lizard_norm.png)").c_str(), NULL, &m_box4.normal_texture_view, NULL);
		CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), GetRootDir().append(LR"(\3d-objects\lizard\lizard_gloss.png)").c_str(), NULL, &m_box4.gloss_texture_view, NULL);

		CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), GetRootDir().append(LR"(\3d-objects\plasma.jpg)").c_str(), NULL, &m_box4.animated_texture_view, NULL);

		// W
		XMStoreFloat4x4(&m_box4.W, XMMatrixMultiply(XMMatrixScaling(1.0f, 1.0f, 1.0f), XMMatrixTranslation(8.f, 5.f, -8.f)));

		// material
		m_box4.material.ambient = { 0.15f, 0.15f, 0.15f, 1.f };
		m_box4.material.diffuse = { 0.77f, 0.77f, 0.77f, 1.f };
		m_box4.material.specular = { 0.8f, 0.8f, 0.8f, 190.0f };
		m_box4.material.options = { 3, 1.2f, 0, 0 };

		// perObjectCB
		D3D11_BUFFER_DESC perObjectCBDesc;
		perObjectCBDesc.Usage = D3D11_USAGE_DYNAMIC;
		perObjectCBDesc.ByteWidth = sizeof(PerObjectCB);
		perObjectCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		perObjectCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		perObjectCBDesc.MiscFlags = 0;
		perObjectCBDesc.StructureByteStride = 0;
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&perObjectCBDesc, nullptr, &m_box4.d3dPerObjectCB));


		// vertex buffer
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.ByteWidth = UINT(sizeof(mesh::MeshData::Vertex) * m_box4.mesh.vertices.size());
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexInitData;
		vertexInitData.pSysMem = &m_box4.mesh.vertices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexInitData, &m_box4.d3dVertexBuffer));


		// index buffer
		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.ByteWidth = UINT(sizeof(uint32) * m_box4.mesh.indices.size());
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexInitdata;
		indexInitdata.pSysMem = &m_box4.mesh.indices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &indexInitdata, &m_box4.d3dIndexBuffer));
	}






	// sphere
	{

		//geo
		m_sphere.mesh = mesh::GenerateSphere(1.f, 40, 40); // mesh::GenerateBox(2, 2, 2);
		CreateWICTextureFromFile(m_d3dDevice.Get(), GetRootDir().append(LR"(\3d-objects\lava\lava_color.png)").c_str(), NULL, &m_sphere.diffuse_texture_view, NULL);
		CreateWICTextureFromFile(m_d3dDevice.Get(), GetRootDir().append(LR"(\3d-objects\lava\lava_norm.png)").c_str(), NULL, &m_sphere.normal_texture_view, NULL);



		// W
		XMStoreFloat4x4(&m_sphere.W, XMMatrixTranslation(0.f, 4.f, 0.f));

		// material
		m_sphere.material.ambient = { 0.7f, 0.1f, 0.1f, 1.0f };
		m_sphere.material.diffuse = { 1.00f, 1.00f, 1.00f, 1.0f };
		m_sphere.material.specular = { 0.7f, 0.7f, 0.7f, 40.0f };
		m_sphere.material.options = { 1, 2, 0, 0 };


		// perObjectCB
		D3D11_BUFFER_DESC perObjectCBDesc;
		perObjectCBDesc.Usage = D3D11_USAGE_DYNAMIC;
		perObjectCBDesc.ByteWidth = sizeof(PerObjectCB);
		perObjectCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		perObjectCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		perObjectCBDesc.MiscFlags = 0;
		perObjectCBDesc.StructureByteStride = 0;
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&perObjectCBDesc, nullptr, &m_sphere.d3dPerObjectCB));


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
		XMStoreFloat4x4(&m_crate.W, XMMatrixMultiply( XMMatrixRotationRollPitchYaw(0.0f, 15.0f, 0.0f), XMMatrixMultiply(XMMatrixScaling(0.01f, 0.01f, 0.01f), XMMatrixTranslation(6.f, 0.f, 6.f))));


		//bottom material
		Material& bottomMat = m_crate.shapeAttributeMapByName["bottom_1"].material;
		bottomMat.ambient = { 0.8f, 0.3f, 0.1f, 1.0f };
		bottomMat.diffuse = { 0.94f, 0.40f, 0.14f, 1.0f };
		bottomMat.specular = { 0.94f, 0.40f, 0.14f, 30.0f };
		bottomMat.options = { 0, 0, 0, 0 };

		//top material
		Material& topMat = m_crate.shapeAttributeMapByName["top_2"].material;
		topMat.ambient = { 0.8f, 0.8f, 0.8f, 1.0f };
		topMat.diffuse = { 0.9f, 0.9f, 0.9f, 1.0f };
		topMat.specular = { 0.9f, 0.9f, 0.9f, 550.0f };
		topMat.options = { 0, 0, 0, 0 };

		//top handles material
		Material& topHandleMat = m_crate.shapeAttributeMapByName["top_handles_4"].material;
		topHandleMat.ambient = { 0.3f, 0.3f, 0.3f, 1.0f };
		topHandleMat.diffuse = { 0.4f, 0.4f, 0.4f, 1.0f };
		topHandleMat.specular = { 0.9f, 0.9f, 0.9f, 120.0f };
		topHandleMat.options = { 0, 0, 0, 0 };

		//handle material
		Material& handleMat = m_crate.shapeAttributeMapByName["handles_8"].material;
		handleMat.ambient = { 0.5f, 0.5f, 0.1f, 1.0f };
		handleMat.diffuse = { 0.67f, 0.61f, 0.1f, 1.0f };
		handleMat.specular = { 0.67f, 0.61f, 0.1f, 200.0f };
		handleMat.options = { 0, 0, 0, 0 };

		//metal material
		Material& metalPiecesMat = m_crate.shapeAttributeMapByName["metal_pieces_3"].material;
		metalPiecesMat.ambient = { 0.3f, 0.3f, 0.3f, 1.0f };
		metalPiecesMat.diffuse = { 0.4f, 0.4f, 0.4f, 1.0f };
		metalPiecesMat.specular = { 0.4f, 0.4f, 0.4f, 520.0f };
		metalPiecesMat.options = { 0, 0, 0, 0 };



		for (const auto& namePairWithDesc : m_crate.mesh.meshDescriptorMapByName)
		{
			ComPtr<ID3D11Buffer> d3dPerObjectCB;

			// perObjectCBs
			D3D11_BUFFER_DESC perObjectCBDesc;
			perObjectCBDesc.Usage = D3D11_USAGE_DYNAMIC;
			perObjectCBDesc.ByteWidth = sizeof(PerObjectCB);
			perObjectCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			perObjectCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			perObjectCBDesc.MiscFlags = 0;
			perObjectCBDesc.StructureByteStride = 0;
			XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&perObjectCBDesc, nullptr, &d3dPerObjectCB));

			m_crate.shapeAttributeMapByName[namePairWithDesc.first].d3dPerObjectCB = d3dPerObjectCB;
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


void TextureDemoApp::InitLights()
{
	m_dirLight.ambient = { 0.16f, 0.18f, 0.18f, 1.f };
	m_dirLight.diffuse = { 0.4f* 0.87f,0.4f* 0.90f,0.4f* 0.94f, 1.f };
	m_dirLight.specular = { 0.87f, 0.90f, 0.94f, 1.f };
	XMVECTOR dirLightDirection = XMVector3Normalize(-XMVectorSet(5.f, 3.f, 5.f, 0.f));
	XMStoreFloat3(&m_dirLight.dirW, dirLightDirection);

	PointLight pointLight;



	pointLight.ambient = { 0.18f, 0.18f, 0.18f, 1.0f };
	pointLight.diffuse = { 0.94f, 0.94f, 0.94f, 1.0f };
	pointLight.specular = { 0.94f, 0.94f, 0.94f, 1.0f };

	pointLight.range = 15.f;
	pointLight.attenuation = { 0.2f, 0.2f, 0.2f };

	pointLight.posW = { 0, 2.f, 0 };
	m_pointLights[0] = pointLight;

	pointLight.posW = { -8.f, 2.f, 8.f };
	m_pointLights[1] = pointLight;

	pointLight.posW = { 5.f, 2.f, 5.f };
	m_pointLights[2] = pointLight;


	pointLight.posW = { 5.f, 2.f, -5.f };
	m_pointLights[3] = pointLight;

	pointLight.posW = { -8.f, 2.f, -8.f };
	m_pointLights[4] = pointLight;


	m_spotLight.ambient = { 0.018f, 0.018f, 0.18f, 1.0f };
	m_spotLight.diffuse = { 0.1f, 0.1f, 0.9f, 1.0f };
	m_spotLight.specular = { 0.1f, 0.1f, 0.9f, 1.0f };
	XMVECTOR posW = XMVectorSet(5.f, 5.f, -5.f, 1.f);
	XMStoreFloat3(&m_spotLight.posW, posW);
	m_spotLight.range = 50.f;
	XMVECTOR dirW = XMVector3Normalize(XMVectorSet(-4.f, 1.f, 0.f, 1.f) - posW);
	XMStoreFloat3(&m_spotLight.dirW, dirW);
	m_spotLight.spot = 40.f;
	m_spotLight.attenuation = { 0.0f, 0.125f, 0.f };


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


void TextureDemoApp::InitRasterizerState()
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


void TextureDemoApp::OnResized()
{
	application::DirectxApp::OnResized();

	//update the projection matrix with the new aspect ratio
	XMMATRIX P = XMMatrixPerspectiveFovLH(math::ToRadians(45.f), AspectRatio(), 1.f, 1000.f);
	XMStoreFloat4x4(&m_projectionMatrix, P);
}


void TextureDemoApp::OnWheelScroll(input::ScrollStatus scroll)
{
	// zoom in or out when the scroll wheel is used
	if (service::Locator::GetMouse()->IsInClientArea())
	{
		m_camera.IncreaseRadiusBy(scroll.isScrollingUp ? -0.5f : 0.5f);
	}
}


void TextureDemoApp::OnMouseMove(const DirectX::XMINT2& movement, const DirectX::XMINT2& currentPos)
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


void TextureDemoApp::OnKeyStatusChange(input::Key key, const input::KeyStatus& status)
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
}

float pos=0;
float textureSpeed=0.2f;
void TextureDemoApp::UpdateScene(float deltaSeconds)
{
	XTEST_UNUSED_VAR(deltaSeconds);
	pos += textureSpeed * deltaSeconds;

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
	}

	//BOX 1
	{
		XMMATRIX W = XMLoadFloat4x4(&m_box1.W);
		XMMATRIX WVP = W * V * P;

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		// disable gpu access
		XTEST_D3D_CHECK(m_d3dContext->Map(m_box1.d3dPerObjectCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		PerObjectCB* perObjectCB = (PerObjectCB*)mappedResource.pData;

		//update the data
		XMStoreFloat4x4(&perObjectCB->W, XMMatrixTranspose(W));
		XMStoreFloat4x4(&perObjectCB->WVP, XMMatrixTranspose(WVP));
		XMStoreFloat4x4(&perObjectCB->W_inverseTraspose, XMMatrixInverse(nullptr, W));

		XMStoreFloat4x4(&perObjectCB->TexcoordMatrix, XMMatrixTranspose(XMMatrixMultiply(XMMatrixScaling(1.f, 1.f, 1.f), XMMatrixTranslation(pos, pos, 0.0f))));

		perObjectCB->material = m_box1.material;

		// enable gpu access
		m_d3dContext->Unmap(m_box1.d3dPerObjectCB.Get(), 0);
	}
	//BOX 2
	{
		XMMATRIX W = XMLoadFloat4x4(&m_box2.W);
		XMMATRIX WVP = W * V * P;

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		// disable gpu access
		XTEST_D3D_CHECK(m_d3dContext->Map(m_box2.d3dPerObjectCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		PerObjectCB* perObjectCB = (PerObjectCB*)mappedResource.pData;

		//update the data
		XMStoreFloat4x4(&perObjectCB->W, XMMatrixTranspose(W));
		XMStoreFloat4x4(&perObjectCB->WVP, XMMatrixTranspose(WVP));
		XMStoreFloat4x4(&perObjectCB->W_inverseTraspose, XMMatrixInverse(nullptr, W));
		XMStoreFloat4x4(&perObjectCB->TexcoordMatrix, XMMatrixTranspose(XMMatrixMultiply(XMMatrixScaling(1.f, 1.f, 1.f), XMMatrixTranslation(pos, pos, 0.0f))));

		perObjectCB->material = m_box2.material;

		// enable gpu access
		m_d3dContext->Unmap(m_box2.d3dPerObjectCB.Get(), 0);
	}


	//BOX 3
	{
		XMMATRIX W = XMLoadFloat4x4(&m_box3.W);
		XMMATRIX WVP = W * V * P;

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		// disable gpu access
		XTEST_D3D_CHECK(m_d3dContext->Map(m_box3.d3dPerObjectCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		PerObjectCB* perObjectCB = (PerObjectCB*)mappedResource.pData;

		//update the data
		XMStoreFloat4x4(&perObjectCB->W, XMMatrixTranspose(W));
		XMStoreFloat4x4(&perObjectCB->WVP, XMMatrixTranspose(WVP));
		XMStoreFloat4x4(&perObjectCB->W_inverseTraspose, XMMatrixInverse(nullptr, W));

		XMStoreFloat4x4(&perObjectCB->TexcoordMatrix, XMMatrixTranspose(XMMatrixMultiply(XMMatrixScaling(1.f, 1.f, 1.f), XMMatrixTranslation(pos, pos, 0.0f))));

		perObjectCB->material = m_box3.material;

		// enable gpu access
		m_d3dContext->Unmap(m_box3.d3dPerObjectCB.Get(), 0);
	}
	//BOX 4
	{
		XMMATRIX W = XMLoadFloat4x4(&m_box4.W);
		XMMATRIX WVP = W * V * P;

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		// disable gpu access
		XTEST_D3D_CHECK(m_d3dContext->Map(m_box4.d3dPerObjectCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		PerObjectCB* perObjectCB = (PerObjectCB*)mappedResource.pData;

		//update the data
		XMStoreFloat4x4(&perObjectCB->W, XMMatrixTranspose(W));
		XMStoreFloat4x4(&perObjectCB->WVP, XMMatrixTranspose(WVP));
		XMStoreFloat4x4(&perObjectCB->W_inverseTraspose, XMMatrixInverse(nullptr, W));
		XMStoreFloat4x4(&perObjectCB->TexcoordMatrix, XMMatrixTranspose(XMMatrixMultiply(XMMatrixScaling(1.f, 1.f, 1.f), XMMatrixTranslation(pos, pos, 0.0f))));

		perObjectCB->material = m_box4.material;

		// enable gpu access
		m_d3dContext->Unmap(m_box4.d3dPerObjectCB.Get(), 0);
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
	}


	// torus knot PerObjectCB
	{
		XMMATRIX W = XMLoadFloat4x4(&m_torus_knot.W);
		XMMATRIX WVP = W * V * P;

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		// disable gpu access
		XTEST_D3D_CHECK(m_d3dContext->Map(m_torus_knot.d3dPerObjectCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		PerObjectCB* perObjectCB = (PerObjectCB*)mappedResource.pData;

		//update the data
		XMStoreFloat4x4(&perObjectCB->W, XMMatrixTranspose(W));
		XMStoreFloat4x4(&perObjectCB->WVP, XMMatrixTranspose(WVP));
		XMStoreFloat4x4(&perObjectCB->W_inverseTraspose, XMMatrixInverse(nullptr, W));
		perObjectCB->material = m_torus_knot.material;

		// enable gpu access
		m_d3dContext->Unmap(m_torus_knot.d3dPerObjectCB.Get(), 0);
	}


	// torus knot 2 PerObjectCB
	{
		XMMATRIX W = XMLoadFloat4x4(&m_torus_knot2.W);
		XMMATRIX WVP = W * V * P;

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		// disable gpu access
		XTEST_D3D_CHECK(m_d3dContext->Map(m_torus_knot2.d3dPerObjectCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		PerObjectCB* perObjectCB = (PerObjectCB*)mappedResource.pData;

		//update the data
		XMStoreFloat4x4(&perObjectCB->W, XMMatrixTranspose(W));
		XMStoreFloat4x4(&perObjectCB->WVP, XMMatrixTranspose(WVP));
		XMStoreFloat4x4(&perObjectCB->W_inverseTraspose, XMMatrixInverse(nullptr, W));
		perObjectCB->material = m_torus_knot2.material;

		// enable gpu access
		m_d3dContext->Unmap(m_torus_knot2.d3dPerObjectCB.Get(), 0);
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

		}
	}


	// PerFrameCB
	{

		if (!m_stopLights)
		{
			XMMATRIX R = XMMatrixRotationY(math::ToRadians(30.f) * deltaSeconds);

			for (int i = 0; i < 5; i++) {
				XMStoreFloat3(&m_pointLights[i].posW, XMVector3Transform(XMLoadFloat3(&m_pointLights[i].posW), R));
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
		perFrameCB->spotLight = m_spotLight;
		perFrameCB->pointLights = m_pointLights;
		perFrameCB->eyePosW = m_camera.GetPosition();

		// enable gpu access
		m_d3dContext->Unmap(m_d3dPerFrameCB.Get(), 0);
	}


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

	m_d3dAnnotation->EndEvent();
}


void TextureDemoApp::RenderScene()
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
	m_d3dContext->PSSetSamplers(0, 1, m_textureSampler.GetAddressOf());

	m_d3dContext->PSSetConstantBuffers(1, 1, m_d3dPerFrameCB.GetAddressOf());
	m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	// draw torus knot
	{
		// bind the constant data to the vertex shader
		m_d3dContext->VSSetConstantBuffers(0, 1, m_torus_knot.d3dPerObjectCB.GetAddressOf());
		m_d3dContext->PSSetConstantBuffers(0, 1, m_torus_knot.d3dPerObjectCB.GetAddressOf());

		// set what to draw
		UINT stride = sizeof(mesh::MeshData::Vertex);
		UINT offset = 0;
		m_d3dContext->IASetVertexBuffers(0, 1, m_torus_knot.d3dVertexBuffer.GetAddressOf(), &stride, &offset);
		m_d3dContext->IASetIndexBuffer(m_torus_knot.d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		m_d3dContext->DrawIndexed(UINT(m_torus_knot.mesh.indices.size()), 0, 0);
	}

	// draw torus knot 2
	{
		// bind the constant data to the vertex shader
		m_d3dContext->VSSetConstantBuffers(0, 1, m_torus_knot2.d3dPerObjectCB.GetAddressOf());
		m_d3dContext->PSSetConstantBuffers(0, 1, m_torus_knot2.d3dPerObjectCB.GetAddressOf());

	
		// set what to draw
		UINT stride = sizeof(mesh::MeshData::Vertex);
		UINT offset = 0;
		m_d3dContext->IASetVertexBuffers(0, 1, m_torus_knot2.d3dVertexBuffer.GetAddressOf(), &stride, &offset);
		m_d3dContext->IASetIndexBuffer(m_torus_knot2.d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		m_d3dContext->DrawIndexed(UINT(m_torus_knot2.mesh.indices.size()), 0, 0);
	}
	// draw torus
	{
		// bind the constant data to the vertex shader
		m_d3dContext->VSSetConstantBuffers(0, 1, m_torus.d3dPerObjectCB.GetAddressOf());
		m_d3dContext->PSSetConstantBuffers(0, 1, m_torus.d3dPerObjectCB.GetAddressOf());

		m_d3dContext->PSSetShaderResources(0, 1, m_torus.diffuse_texture_view.GetAddressOf());
		m_d3dContext->PSSetShaderResources(1, 1, m_torus.normal_texture_view.GetAddressOf());
		m_d3dContext->PSSetShaderResources(2, 1, m_torus.gloss_texture_view.GetAddressOf());


		// set what to draw
		UINT stride = sizeof(mesh::MeshData::Vertex);
		UINT offset = 0;
		m_d3dContext->IASetVertexBuffers(0, 1, m_torus.d3dVertexBuffer.GetAddressOf(), &stride, &offset);
		m_d3dContext->IASetIndexBuffer(m_torus.d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		m_d3dContext->DrawIndexed(UINT(m_torus.mesh.indices.size()), 0, 0);
	}

	// draw plane
	{
		// bind the constant data to the vertex shader
		m_d3dContext->VSSetConstantBuffers(0, 1, m_plane.d3dPerObjectCB.GetAddressOf());
		m_d3dContext->PSSetConstantBuffers(0, 1, m_plane.d3dPerObjectCB.GetAddressOf());

		m_d3dContext->PSSetShaderResources(0, 1, m_plane.diffuse_texture_view.GetAddressOf());
		m_d3dContext->PSSetShaderResources(1, 1, m_plane.normal_texture_view.GetAddressOf());
		m_d3dContext->PSSetShaderResources(2, 1, m_plane.gloss_texture_view.GetAddressOf());

		// set what to draw
		UINT stride = sizeof(mesh::MeshData::Vertex);
		UINT offset = 0;
		m_d3dContext->IASetVertexBuffers(0, 1, m_plane.d3dVertexBuffer.GetAddressOf(), &stride, &offset);
		m_d3dContext->IASetIndexBuffer(m_plane.d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		m_d3dContext->DrawIndexed(UINT(m_plane.mesh.indices.size()), 0, 0);
	}


	// draw box1
	{
		// bind the constant data to the vertex shader
		m_d3dContext->VSSetConstantBuffers(0, 1, m_box1.d3dPerObjectCB.GetAddressOf());
		m_d3dContext->PSSetConstantBuffers(0, 1, m_box1.d3dPerObjectCB.GetAddressOf());

		m_d3dContext->PSSetShaderResources(0, 1, m_box1.diffuse_texture_view.GetAddressOf());
		m_d3dContext->PSSetShaderResources(1, 1, m_box1.normal_texture_view.GetAddressOf());
		m_d3dContext->PSSetShaderResources(2, 1, m_box1.gloss_texture_view.GetAddressOf());
		m_d3dContext->PSSetShaderResources(3, 1, m_box1.animated_texture_view.GetAddressOf());

		// set what to draw
		UINT stride = sizeof(mesh::MeshData::Vertex);
		UINT offset = 0;
		m_d3dContext->IASetVertexBuffers(0, 1, m_box1.d3dVertexBuffer.GetAddressOf(), &stride, &offset);
		m_d3dContext->IASetIndexBuffer(m_box1.d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		m_d3dContext->DrawIndexed(UINT(m_box1.mesh.indices.size()), 0, 0);
	}



	// draw box2
	{
		// bind the constant data to the vertex shader
		m_d3dContext->VSSetConstantBuffers(0, 1, m_box2.d3dPerObjectCB.GetAddressOf());
		m_d3dContext->PSSetConstantBuffers(0, 1, m_box2.d3dPerObjectCB.GetAddressOf());

		m_d3dContext->PSSetShaderResources(0, 1, m_box2.diffuse_texture_view.GetAddressOf());
		m_d3dContext->PSSetShaderResources(1, 1, m_box2.normal_texture_view.GetAddressOf());
		m_d3dContext->PSSetShaderResources(2, 1, m_box2.gloss_texture_view.GetAddressOf());
		m_d3dContext->PSSetShaderResources(3, 1, m_box2.animated_texture_view.GetAddressOf());

		// set what to draw
		UINT stride = sizeof(mesh::MeshData::Vertex);
		UINT offset = 0;
		m_d3dContext->IASetVertexBuffers(0, 1, m_box2.d3dVertexBuffer.GetAddressOf(), &stride, &offset);
		m_d3dContext->IASetIndexBuffer(m_box2.d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		m_d3dContext->DrawIndexed(UINT(m_box2.mesh.indices.size()), 0, 0);
	}



	// draw box3
	{
		// bind the constant data to the vertex shader
		m_d3dContext->VSSetConstantBuffers(0, 1, m_box3.d3dPerObjectCB.GetAddressOf());
		m_d3dContext->PSSetConstantBuffers(0, 1, m_box3.d3dPerObjectCB.GetAddressOf());

		m_d3dContext->PSSetShaderResources(0, 1, m_box3.diffuse_texture_view.GetAddressOf());
		m_d3dContext->PSSetShaderResources(1, 1, m_box3.normal_texture_view.GetAddressOf());
		m_d3dContext->PSSetShaderResources(2, 1, m_box3.gloss_texture_view.GetAddressOf());
		m_d3dContext->PSSetShaderResources(3, 1, m_box3.animated_texture_view.GetAddressOf());

		// set what to draw
		UINT stride = sizeof(mesh::MeshData::Vertex);
		UINT offset = 0;
		m_d3dContext->IASetVertexBuffers(0, 1, m_box3.d3dVertexBuffer.GetAddressOf(), &stride, &offset);
		m_d3dContext->IASetIndexBuffer(m_box3.d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		m_d3dContext->DrawIndexed(UINT(m_box3.mesh.indices.size()), 0, 0);
	}



	// draw box4
	{
		// bind the constant data to the vertex shader
		m_d3dContext->VSSetConstantBuffers(0, 1, m_box4.d3dPerObjectCB.GetAddressOf());
		m_d3dContext->PSSetConstantBuffers(0, 1, m_box4.d3dPerObjectCB.GetAddressOf());

		m_d3dContext->PSSetShaderResources(0, 1, m_box4.diffuse_texture_view.GetAddressOf());
		m_d3dContext->PSSetShaderResources(1, 1, m_box4.normal_texture_view.GetAddressOf());
		m_d3dContext->PSSetShaderResources(2, 1, m_box4.gloss_texture_view.GetAddressOf());
		m_d3dContext->PSSetShaderResources(3, 1, m_box4.animated_texture_view.GetAddressOf());

		// set what to draw
		UINT stride = sizeof(mesh::MeshData::Vertex);
		UINT offset = 0;
		m_d3dContext->IASetVertexBuffers(0, 1, m_box4.d3dVertexBuffer.GetAddressOf(), &stride, &offset);
		m_d3dContext->IASetIndexBuffer(m_box4.d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		m_d3dContext->DrawIndexed(UINT(m_box4.mesh.indices.size()), 0, 0);
	}




	// draw sphere
	{
		// bind the constant data to the vertex shader
		m_d3dContext->VSSetConstantBuffers(0, 1, m_sphere.d3dPerObjectCB.GetAddressOf());
		m_d3dContext->PSSetConstantBuffers(0, 1, m_sphere.d3dPerObjectCB.GetAddressOf());

		m_d3dContext->PSSetShaderResources(0, 1, m_sphere.diffuse_texture_view.GetAddressOf());
		m_d3dContext->PSSetShaderResources(1, 1, m_sphere.normal_texture_view.GetAddressOf());
		m_d3dContext->PSSetShaderResources(2, 1, m_sphere.gloss_texture_view.GetAddressOf());


		// set what to draw
		UINT stride = sizeof(mesh::MeshData::Vertex);
		UINT offset = 0;
		m_d3dContext->IASetVertexBuffers(0, 1, m_sphere.d3dVertexBuffer.GetAddressOf(), &stride, &offset);
		m_d3dContext->IASetIndexBuffer(m_sphere.d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		m_d3dContext->DrawIndexed(UINT(m_sphere.mesh.indices.size()), 0, 0);
	}





	// draw crate
	{
		// set what to draw
		UINT stride = sizeof(mesh::MeshData::Vertex);
		UINT offset = 0;
		m_d3dContext->IASetVertexBuffers(0, 1, m_crate.d3dVertexBuffer.GetAddressOf(), &stride, &offset);
		m_d3dContext->IASetIndexBuffer(m_crate.d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		m_d3dContext->PSSetShaderResources(0, 1, m_crate.diffuse_texture_view.GetAddressOf());
		m_d3dContext->PSSetShaderResources(1, 1, m_crate.normal_texture_view.GetAddressOf());
		m_d3dContext->PSSetShaderResources(2, 1, m_crate.gloss_texture_view.GetAddressOf());

		for (const auto& namePairWithDesc : m_crate.mesh.meshDescriptorMapByName)
		{
			const std::string& shapeName = namePairWithDesc.first;
			const mesh::GPFMesh::MeshDescriptor& meshDesc = m_crate.mesh.meshDescriptorMapByName[shapeName];

			// bind the constant data to the vertex shader
			m_d3dContext->VSSetConstantBuffers(0, 1, m_crate.shapeAttributeMapByName[shapeName].d3dPerObjectCB.GetAddressOf());
			m_d3dContext->PSSetConstantBuffers(0, 1, m_crate.shapeAttributeMapByName[shapeName].d3dPerObjectCB.GetAddressOf());

			// draw
			m_d3dContext->DrawIndexed(meshDesc.indexCount, meshDesc.indexOffset, meshDesc.vertexOffset);
		}

	}



	XTEST_D3D_CHECK(m_swapChain->Present(0, 0));

	m_d3dAnnotation->EndEvent();
}

