#include "textures_demo_app.h"
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
	, m_pointLights()
	, m_lightsControl()
	, m_isLightControlDirty(true)
	, m_stopLights(false)
	, m_d3dPerFrameCB(nullptr)
	, m_d3dRarelyChangedCB(nullptr)
	, m_vertexShader(nullptr)
	, m_pixelShader(nullptr)
	, m_inputLayout(nullptr)
	, m_rasterizerState(nullptr)
	, m_objectsToDrawByGroup()
	, m_materialMap()
	, m_objectsNumber(3)
{

}


TexturesDemoApp::~TexturesDemoApp()
{}


void TexturesDemoApp::Init()
{
	application::DirectxApp::Init();

	m_d3dAnnotation->BeginEvent(L"init-demo");

	InitMatrices();
	InitShaders();
	InitRenderable();
	InitLights();
	InitRasterizerState();
	InitTextures();
	InitMaterials();
	InitObjects();

	service::Locator::GetMouse()->AddListener(this);
	service::Locator::GetKeyboard()->AddListener(this, { input::Key::F, input::Key::F1, input::Key::F2, input::Key::F3, input::Key::space_bar });

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


	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(mesh::MeshData::Vertex, normal), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(mesh::MeshData::Vertex, tangentU), D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(mesh::MeshData::Vertex, uv), D3D11_INPUT_PER_VERTEX_DATA, 0}
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
		ObjectGroup planeGroup;

		Mesh mesh;
		mesh.meshData = mesh::GeneratePlane(35.f, 35.f, 100, 100);

		// vertex buffer
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.ByteWidth = UINT(sizeof(mesh::MeshData::Vertex) * mesh.meshData.vertices.size());
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexInitData;
		vertexInitData.pSysMem = &mesh.meshData.vertices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexInitData, &mesh.d3dVertexBuffer));


		// index buffer
		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.ByteWidth = UINT(sizeof(uint32) * mesh.meshData.indices.size());
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexInitdata;
		indexInitdata.pSysMem = &mesh.meshData.indices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &indexInitdata, &mesh.d3dIndexBuffer));

		planeGroup.mesh = mesh;

		Renderable plane;

		XMStoreFloat4x4(&plane.W, XMMatrixIdentity());
		XMStoreFloat4x4(&plane.textureMatrix, XMMatrixIdentity());
		plane.materialKey = "material_default";

		// perObjectCB
		D3D11_BUFFER_DESC perObjectCBDesc;
		perObjectCBDesc.Usage = D3D11_USAGE_DYNAMIC;
		perObjectCBDesc.ByteWidth = sizeof(PerObjectCB);
		perObjectCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		perObjectCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		perObjectCBDesc.MiscFlags = 0;
		perObjectCBDesc.StructureByteStride = 0;
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&perObjectCBDesc, nullptr, &plane.d3dPerObjectCB));

		plane.textureViewKey = "ground";

		planeGroup.objects.push_back(plane);

		m_objectsToDrawByGroup.push_back(planeGroup);
	}



	// SPHERES

	{
		ObjectGroup sphereGroup;

		Mesh mesh;
		mesh.meshData = mesh::GenerateSphere(1.f, 40, 40); // mesh::GenerateBox(2, 2, 2)


		// vertex buffer
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.ByteWidth = UINT(sizeof(mesh::MeshData::Vertex) * mesh.meshData.vertices.size());
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexInitData;
		vertexInitData.pSysMem = &mesh.meshData.vertices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexInitData, &mesh.d3dVertexBuffer));


		// index buffer
		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.ByteWidth = UINT(sizeof(uint32) * mesh.meshData.indices.size());
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexInitdata;
		indexInitdata.pSysMem = &mesh.meshData.indices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &indexInitdata, &mesh.d3dIndexBuffer));

		sphereGroup.mesh = mesh;


		for (size_t i = 0; i < m_objectsNumber; i++) {

			Renderable sphere;

			sphere.materialKey = "material_default";

			XMStoreFloat4x4(&sphere.W, XMMatrixTranslation(0.f, 2.5f, 0.f));
			XMStoreFloat4x4(&sphere.textureMatrix, XMMatrixIdentity());

			D3D11_BUFFER_DESC perObjectCBDesc;
			perObjectCBDesc.Usage = D3D11_USAGE_DYNAMIC;
			perObjectCBDesc.ByteWidth = sizeof(PerObjectCB);
			perObjectCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			perObjectCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			perObjectCBDesc.MiscFlags = 0;
			perObjectCBDesc.StructureByteStride = 0;
			XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&perObjectCBDesc, nullptr, &sphere.d3dPerObjectCB));

			sphere.textureViewKey = "ground";

			sphereGroup.objects.push_back(sphere);
		}

		m_objectsToDrawByGroup.push_back(sphereGroup);
	}

	// SQUARES
	{
		ObjectGroup squareGroup;

		Mesh mesh;
		mesh.meshData = mesh::GenerateBox(2.f, 2.f, 2.f); // mesh::GenerateBox(2, 2, 2);


		// vertex buffer
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.ByteWidth = UINT(sizeof(mesh::MeshData::Vertex) * mesh.meshData.vertices.size());
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexInitData;
		vertexInitData.pSysMem = &mesh.meshData.vertices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexInitData, &mesh.d3dVertexBuffer));


		// index buffer
		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.ByteWidth = UINT(sizeof(uint32) * mesh.meshData.indices.size());
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexInitdata;
		indexInitdata.pSysMem = &mesh.meshData.indices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &indexInitdata, &mesh.d3dIndexBuffer));

		squareGroup.mesh = mesh;

		for (size_t i = 0; i < m_objectsNumber; i++) {

			Renderable square;

			XMStoreFloat4x4(&square.W, XMMatrixTranslation(0.f, 1.f, 0.f));
			XMStoreFloat4x4(&square.textureMatrix, XMMatrixIdentity());
			square.materialKey = "material_default";

			// perObjectCB
			D3D11_BUFFER_DESC perObjectCBDesc;
			perObjectCBDesc.Usage = D3D11_USAGE_DYNAMIC;
			perObjectCBDesc.ByteWidth = sizeof(PerObjectCB);
			perObjectCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			perObjectCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			perObjectCBDesc.MiscFlags = 0;
			perObjectCBDesc.StructureByteStride = 0;
			XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&perObjectCBDesc, nullptr, &square.d3dPerObjectCB));

			square.textureViewKey = "ground";

			squareGroup.objects.push_back(square);
		}

		m_objectsToDrawByGroup.push_back(squareGroup);
	}

}


void TexturesDemoApp::InitLights()
{
	m_dirLight.ambient = { 0.3f, 0.3f, 0.3f, 1.f };
	m_dirLight.diffuse = { 0.6f, 0.6 , 0.6 , 1.f };
	m_dirLight.specular = { 0.87f, 0.90f, 0.94f, 1.f };
	XMVECTOR dirLightDirection = XMVector3Normalize(-XMVectorSet(5.f, 3.f, 5.f, 0.f));
	XMStoreFloat3(&m_dirLight.dirW, dirLightDirection);


	m_pointLights[0].ambient = { 0.2f, 0.2f, 0.2f, 1.0f };
	m_pointLights[0].diffuse = { 0.8f, 0.8f, 0.8f, 1.0f };
	m_pointLights[0].specular = { 0.9f, 0.9f, 0.9f, 1.0f };
	m_pointLights[0].posW = { 0.f, 2.f, 0.f };
	m_pointLights[0].range = 20.f;
	m_pointLights[0].attenuation = { 0.0f, 0.2f, 0.f };

	for (int i = 1; i < m_pointLights.size(); i++) {
		m_pointLights[i] = m_pointLights[0];
	}

	m_pointLights[0].posW = { 0.f, 3.f, -7.f };
	m_pointLights[1].posW = { -7.f, 3.f, -2.f };
	m_pointLights[2].posW = { -4.f, 3.f, 6.f };
	m_pointLights[3].posW = { 4.f, 3.f, 6.f };
	m_pointLights[4].posW = { 7.f, 3.f, -2.f };

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

void TexturesDemoApp::InitTextures()
{
	//WOOD
	{
		TexturePack texturePack;
		TexturePackView texturePackView;
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), GetRootDir().append(LR"(\3d-objects\wood\wood_color.png)").c_str(), texturePack.texture.GetAddressOf(), texturePackView.textureView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), GetRootDir().append(LR"(\3d-objects\wood\wood_norm.png)").c_str(), texturePack.normalMap.GetAddressOf(), texturePackView.normalMapView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), GetRootDir().append(LR"(\3d-objects\wood\wood_gloss.png)").c_str(), texturePack.glossMap.GetAddressOf(), texturePackView.glossMapView.GetAddressOf()));

		m_texturePacks.push_back(texturePack);
		m_texturePackViewMap.emplace("wood", texturePackView);
	}

	//GROUND
	{
		TexturePack texturePack;
		TexturePackView texturePackView;
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), GetRootDir().append(LR"(\3d-objects\ground\ground_color.png)").c_str(), texturePack.texture.GetAddressOf(), texturePackView.textureView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), GetRootDir().append(LR"(\3d-objects\ground\ground_norm.png)").c_str(), texturePack.normalMap.GetAddressOf(), texturePackView.normalMapView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), GetRootDir().append(LR"(\3d-objects\ground\ground_gloss.png)").c_str(), texturePack.glossMap.GetAddressOf(), texturePackView.glossMapView.GetAddressOf()));

		m_texturePacks.push_back(texturePack);
		m_texturePackViewMap.emplace("ground", texturePackView);
	}

	//TWINE
	{
		TexturePack texturePack;
		TexturePackView texturePackView;
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), GetRootDir().append(LR"(\3d-objects\twine\twine_color.png)").c_str(), texturePack.texture.GetAddressOf(), texturePackView.textureView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), GetRootDir().append(LR"(\3d-objects\twine\twine_norm.png)").c_str(), texturePack.normalMap.GetAddressOf(), texturePackView.normalMapView.GetAddressOf()));
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(), GetRootDir().append(LR"(\3d-objects\twine\twine_gloss.png)").c_str(), texturePack.glossMap.GetAddressOf(), texturePackView.glossMapView.GetAddressOf()));

		m_texturePacks.push_back(texturePack);
		m_texturePackViewMap.emplace("twine", texturePackView);
	}

	//SAMPLER
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

void TexturesDemoApp::InitMaterials()
{
	{
		Material material;
		material.ambient = { 0.2f, 0.2f, 0.2f, 1.f };
		material.diffuse = { 0.5f, 0.5f, 0.5f, 1.f };
		material.specular = { 0.5f, 0.5f, 0.5f, 50.0f };

		m_materialMap.emplace("material_default", material);
	}

	{
		// material
		Material material;
		material.ambient = { 0.3f, 0.3f, 0.3f, 1.0f };
		material.diffuse = { 0.5f, 0.5f, 0.5f, 1.0f };
		material.specular = { 0.3f, 0.3f, 0.3f, 40.0f };
		m_materialMap.emplace("material_0", material);
	}
}

void TexturesDemoApp::InitObjects()
{
	const int PLANE = 0, SPHERE = 1, SQUARE = 2;

	//Plane
	{
		m_objectsToDrawByGroup[PLANE].objects[0].textureViewKey = "ground";
	}

	//Object0
	{
		size_t INDEX = 0;
		m_objectsToDrawByGroup[SPHERE].objects[INDEX].textureViewKey = "twine";
		m_objectsToDrawByGroup[SPHERE].objects[INDEX].materialKey = "material_0";

		m_objectsToDrawByGroup[SQUARE].objects[INDEX].textureViewKey = "twine";
		m_objectsToDrawByGroup[SQUARE].objects[INDEX].materialKey = "material_0";
	}

	//Object1
	{
		size_t INDEX = 1;
		XMMATRIX W;
		W = XMLoadFloat4x4(&m_objectsToDrawByGroup[SPHERE].objects[INDEX].W);
		W *= XMMatrixTranslation(-10.f, 0.f, 0.f);
		XMStoreFloat4x4(&m_objectsToDrawByGroup[SPHERE].objects[INDEX].W, W);

		W = XMLoadFloat4x4(&m_objectsToDrawByGroup[SQUARE].objects[INDEX].W);
		W *= XMMatrixTranslation(-10.f, 0.f, 0.f);
		XMStoreFloat4x4(&m_objectsToDrawByGroup[SQUARE].objects[INDEX].W, W);

		m_objectsToDrawByGroup[SPHERE].objects[INDEX].textureViewKey = "wood";
		m_objectsToDrawByGroup[SPHERE].objects[INDEX].materialKey = "material_0";

		m_objectsToDrawByGroup[SQUARE].objects[INDEX].textureViewKey = "wood";
		m_objectsToDrawByGroup[SQUARE].objects[INDEX].materialKey = "material_0";
	}

	//Object2
	{
		size_t INDEX = 2;
		XMMATRIX W;
		W = XMLoadFloat4x4(&m_objectsToDrawByGroup[SPHERE].objects[INDEX].W);
		W *= XMMatrixTranslation(10.f, 0.f, 0.f);
		XMStoreFloat4x4(&m_objectsToDrawByGroup[SPHERE].objects[INDEX].W, W);

		W = XMLoadFloat4x4(&m_objectsToDrawByGroup[SQUARE].objects[INDEX].W);
		W *= XMMatrixTranslation(10.f, 0.f, 0.f);
		XMStoreFloat4x4(&m_objectsToDrawByGroup[SQUARE].objects[INDEX].W, W);

		m_objectsToDrawByGroup[SPHERE].objects[INDEX].textureViewKey = "wood";
		m_objectsToDrawByGroup[SPHERE].objects[INDEX].materialKey = "material_0";

		m_objectsToDrawByGroup[SQUARE].objects[INDEX].textureViewKey = "wood";
		m_objectsToDrawByGroup[SQUARE].objects[INDEX].materialKey = "material_0";
	}


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


void TexturesDemoApp::UpdateScene(float deltaSeconds)
{
	XTEST_UNUSED_VAR(deltaSeconds);


	// create the model-view-projection matrix
	XMMATRIX V = m_camera.GetViewMatrix();
	XMStoreFloat4x4(&m_viewMatrix, V);

	// create projection matrix
	XMMATRIX P = XMLoadFloat4x4(&m_projectionMatrix);



	m_d3dAnnotation->BeginEvent(L"update-constant-buffer");


	// plane PerObjectCB
	for (size_t i = 0; i < m_objectsToDrawByGroup.size(); i++)
	{
		for (size_t j = 0; j < m_objectsToDrawByGroup[i].objects.size(); j++)
		{
			XMMATRIX W = XMLoadFloat4x4(&m_objectsToDrawByGroup[i].objects[j].W);
			XMMATRIX textureMatrix = XMLoadFloat4x4(&m_objectsToDrawByGroup[i].objects[j].textureMatrix);
			XMMATRIX WVP = W * V*P;

			D3D11_MAPPED_SUBRESOURCE mappedResource;
			ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

			// disable gpu access
			XTEST_D3D_CHECK(m_d3dContext->Map(m_objectsToDrawByGroup[i].objects[j].d3dPerObjectCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
			PerObjectCB* perObjectCB = (PerObjectCB*)mappedResource.pData;

			//update the data
			XMStoreFloat4x4(&perObjectCB->W, XMMatrixTranspose(W));
			XMStoreFloat4x4(&perObjectCB->WVP, XMMatrixTranspose(WVP));
			XMStoreFloat4x4(&perObjectCB->W_inverseTraspose, XMMatrixInverse(nullptr, W));
			XMStoreFloat4x4(&perObjectCB->textureMatrix, XMMatrixTranspose(textureMatrix));
			perObjectCB->material = m_materialMap.at(m_objectsToDrawByGroup[i].objects[j].materialKey);

			// enable gpu access
			m_d3dContext->Unmap(m_objectsToDrawByGroup[i].objects[j].d3dPerObjectCB.Get(), 0);
		}
	}


	// PerFrameCB
	{

		if (!m_stopLights)
		{
			XMMATRIX R = XMMatrixRotationY(math::ToRadians(30.f) * deltaSeconds);
			for (size_t i = 0; i < m_pointLights.size(); i++)
			{
				XMStoreFloat3(&m_pointLights[i].posW, XMVector3Transform(XMLoadFloat3(&m_pointLights[i].posW), R));
			}


			//R = XMMatrixRotationAxis(XMVectorSet(-1.f, 0.f, 1.f, 1.f), math::ToRadians(10.f) * deltaSeconds);
			//XMStoreFloat3(&m_dirLight.dirW, XMVector3Transform(XMLoadFloat3(&m_dirLight.dirW), R));
		}

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		// disable gpu access
		XTEST_D3D_CHECK(m_d3dContext->Map(m_d3dPerFrameCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		PerFrameCB* perFrameCB = (PerFrameCB*)mappedResource.pData;

		//update the data
		perFrameCB->dirLight = m_dirLight;
		memcpy(&perFrameCB->pointLights, m_pointLights.data(), sizeof(m_pointLights));
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


void TexturesDemoApp::RenderScene()
{
	m_d3dAnnotation->BeginEvent(L"render-scene");

	// clear the frame
	m_d3dContext->ClearDepthStencilView(m_depthBufferView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
	m_d3dContext->ClearRenderTargetView(m_backBufferView.Get(), DirectX::Colors::LightSkyBlue);

	// set the shaders and the input layout
	m_d3dContext->RSSetState(m_rasterizerState.Get());
	m_d3dContext->IASetInputLayout(m_inputLayout.Get());
	m_d3dContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	m_d3dContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	m_d3dContext->PSSetConstantBuffers(1, 1, m_d3dPerFrameCB.GetAddressOf());
	m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_d3dContext->PSSetSamplers(0, 1, m_textureSampler.GetAddressOf());

	for (size_t i = 0; i < m_objectsToDrawByGroup.size(); i++)
	{

		// set what to draw
		UINT stride = sizeof(mesh::MeshData::Vertex);
		UINT offset = 0;
		const Mesh& mesh = m_objectsToDrawByGroup[i].mesh;
		m_d3dContext->IASetVertexBuffers(0, 1, mesh.d3dVertexBuffer.GetAddressOf(), &stride, &offset);
		m_d3dContext->IASetIndexBuffer(mesh.d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		for (size_t j = 0; j < m_objectsToDrawByGroup[i].objects.size(); j++)
		{
			// bind the constant data to the vertex shader
			m_d3dContext->VSSetConstantBuffers(0, 1, m_objectsToDrawByGroup[i].objects[j].d3dPerObjectCB.GetAddressOf());
			m_d3dContext->PSSetConstantBuffers(0, 1, m_objectsToDrawByGroup[i].objects[j].d3dPerObjectCB.GetAddressOf());

			m_d3dContext->PSSetShaderResources(0, 1, m_texturePackViewMap.at(m_objectsToDrawByGroup[i].objects[j].textureViewKey).textureView.GetAddressOf());
			m_d3dContext->PSSetShaderResources(1, 1, m_texturePackViewMap.at(m_objectsToDrawByGroup[i].objects[j].textureViewKey).normalMapView.GetAddressOf());
			m_d3dContext->PSSetShaderResources(2, 1, m_texturePackViewMap.at(m_objectsToDrawByGroup[i].objects[j].textureViewKey).glossMapView.GetAddressOf());

			m_d3dContext->DrawIndexed(UINT(mesh.meshData.indices.size()), 0, 0);
		}
	}


	XTEST_D3D_CHECK(m_swapChain->Present(0, 0));

	m_d3dAnnotation->EndEvent();
}





