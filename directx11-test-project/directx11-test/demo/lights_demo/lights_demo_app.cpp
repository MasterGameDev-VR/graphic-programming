#include "stdafx.h"
#include "lights_demo_app.h"
#include <file/file_utils.h>
#include <service/locator.h>
#include <mesh/mesh_generator.h>

using namespace DirectX;
using namespace xtest;
using namespace mesh;
using xtest::demo::LightsDemoApp;
using Microsoft::WRL::ComPtr;

typedef std::map<std::string, Microsoft::WRL::ComPtr<ID3D11Buffer>> CostantBuffersMap;
typedef std::map<std::string, xtest::mesh::GPFMesh::MeshDescriptor> MeshDescriptorMap;

LightsDemoApp::LightsDemoApp(HINSTANCE instance,
	const application::WindowSettings& windowSettings,
	const application::DirectxSettings& directxSettings,
	uint32 fps /*=60*/)
	: application::DirectxApp(instance, windowSettings, directxSettings, fps)
	, m_vertexShader(nullptr)
	, m_pixelShader(nullptr)
	, m_inputLayout(nullptr)
	, m_camera(math::ToRadians(90.f), math::ToRadians(30.f), 10.f, { 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, { math::ToRadians(5.f), math::ToRadians(175.f) }, { 3.f, 25.f })
{}


LightsDemoApp::~LightsDemoApp()
{}


void LightsDemoApp::Init()
{
	application::DirectxApp::Init();

	m_d3dAnnotation->BeginEvent(L"init-demo");

	InitLights();
	InitMeshes();
	InitMatrices();
	InitShaders();
	InitBuffers();
	InitRasterizerState();

	service::Locator::GetMouse()->AddListener(this);
	service::Locator::GetKeyboard()->AddListener(this, { input::Key::F });
	// TODO altri pulsanti per accendere e spegnere le luci

	m_d3dAnnotation->EndEvent();
}

// Initializing matrices
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

// Initializing meshes
void LightsDemoApp::InitMeshes()
{
	// Sphere
	Model sphere;
	sphere.Data = GenerateSphere(1.f, 12, 12);
	sphere.VerticesCount = sphere.Data.vertices.size();
	sphere.IndicesCount = sphere.Data.indices.size();
	XMStoreFloat4x4(&sphere.WorldMatrix, XMMatrixIdentity() * XMMatrixTranslation(-2.0f, 0.f, -2.0f));
	sphere.rotationSpeed = math::ToRadians(180.0f);
	sphere.material.ambient = DirectX::XMFLOAT4(0.5f, 0.1f, 0.1f, 0.0f);
	sphere.material.diffuse = DirectX::XMFLOAT4(1.0f, 0.3f, 0.3f, 0.0f);
	sphere.material.specular = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	m_models["sphere"] = sphere;

	// Box 	
	Model box;
	box.Data = GenerateBox(2.0f, 2.0f, 2.0f);
	box.VerticesCount = box.Data.vertices.size();
	box.IndicesCount = box.Data.indices.size();
	XMStoreFloat4x4(&box.WorldMatrix, XMMatrixIdentity());
	box.rotationSpeed = math::ToRadians(90.0f);
	box.material.ambient = DirectX::XMFLOAT4(1.5f, 1.5f, 0.5f, 0.0f);
	box.material.diffuse = DirectX::XMFLOAT4(0.8f, 0.8f, 0.1f, 0.0f);
	box.material.specular = DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 0.1f);
	m_models["box"] = box;

	// Plane
	Model plane;
	plane.Data = GeneratePlane(15.f, 15.f, 2, 2);
	plane.VerticesCount = plane.Data.vertices.size();
	plane.IndicesCount = plane.Data.indices.size();
	XMStoreFloat4x4(&plane.WorldMatrix, XMMatrixIdentity() * XMMatrixTranslation(0.f, -1.0f, 0.f));
	plane.rotationSpeed = math::ToRadians(0.f);
	plane.material.ambient = DirectX::XMFLOAT4(0.5f, 0.5f, 1.f, 0.0f);
	plane.material.diffuse = DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f);
	plane.material.specular = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 10.f);
	m_models["plane"] = plane;

	// Torus
	Model torus;
	torus.Data = GenerateTorus(2.0f, 1.2f, 14, 14);
	torus.VerticesCount = torus.Data.vertices.size();
	torus.IndicesCount = torus.Data.indices.size();
	XMStoreFloat4x4(&torus.WorldMatrix, XMMatrixIdentity() * XMMatrixTranslation(-4.25f, -0.5f, 4.25f));
	torus.rotationSpeed = math::ToRadians(0.0f);
	torus.material.ambient = DirectX::XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	torus.material.diffuse = DirectX::XMFLOAT4(0.5f, 1.5f, 0.5f, 0.0f);
	torus.material.specular = DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 0.1f);
	m_models["torus"] = torus;

	// Crate
	m_crate.Data = file::ReadGPF(std::wstring(GetRootDir()).append(L"\\3d-objects\\crate.gpf"));
	m_crate.VerticesCount = m_crate.Data.meshData.vertices.size();
	m_crate.IndicesCount = m_crate.Data.meshData.indices.size();
	XMStoreFloat4x4(&m_crate.ScaleMatrix, XMMatrixIdentity() * XMMatrixScaling(0.5f, 0.5f, 0.5f));
	XMStoreFloat4x4(&m_crate.WorldMatrix, XMMatrixIdentity() * XMMatrixRotationY(math::ToRadians(45.f)) * XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixTranslation(4.25f, -1.0f, -4.25f));
	m_crate.rotationSpeed = math::ToRadians(0.0f);
	// Crate materials
	{
		// bottom_1
		Material bottom_1;
		bottom_1.ambient = DirectX::XMFLOAT4(0.3f, 0.2f, 0.1f, 0.0f);
		bottom_1.diffuse = DirectX::XMFLOAT4(0.1f, 0.2f, 0.3f, 0.0f);
		bottom_1.specular = DirectX::XMFLOAT4(10.5f, 0.5f, 0.5f, 10.f);
		m_crate.materials["bottom_1"] = bottom_1;

		// top_2
		Material top_2;
		top_2.ambient = DirectX::XMFLOAT4(0.3f, 0.2f, 0.1f, 0.0f);
		top_2.diffuse = DirectX::XMFLOAT4(0.1f, 0.2f, 0.3f, 0.0f);
		top_2.specular = DirectX::XMFLOAT4(0.5f, 0.5f, 10.5f, 10.5f);
		m_crate.materials["top_2"] = top_2;

		// metal_pieces_3
		Material metal_pieces_3;
		metal_pieces_3.ambient = DirectX::XMFLOAT4(0.6f, 0.4f, 0.5f, 0.0f);
		metal_pieces_3.diffuse = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 0.0f);
		metal_pieces_3.specular = DirectX::XMFLOAT4(10.0f, 10.0f, 10.0f, 10.0f);
		m_crate.materials["metal_pieces_3"] = metal_pieces_3;
		
		// top_handles_4
		Material top_handles_4;
		top_handles_4.ambient = DirectX::XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
		top_handles_4.diffuse = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
		top_handles_4.specular = DirectX::XMFLOAT4(0.5f, 10.5f, 0.5f, 10.f);
		m_crate.materials["top_handles_4"] = top_handles_4;

		// handles_8
		Material handles_8;
		handles_8.ambient = DirectX::XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
		handles_8.diffuse = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
		handles_8.specular = DirectX::XMFLOAT4(0.5f, 10.5f, 0.5f, 10.0f);
		m_crate.materials["handles_8"] = handles_8;
	}
}


// Initializing lights
void LightsDemoApp::InitLights()
{
	// Directional light
	m_directionalLight.ambient = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 0.0f);
	m_directionalLight.diffuse = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 0.0f);
	m_directionalLight.specular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 0.0f);
	m_directionalLight.dirW = DirectX::XMFLOAT3(-1.f, -0.5f, 0.f);
	XMVECTOR direction = XMLoadFloat3(&m_directionalLight.dirW);
	direction = XMVector3Normalize(direction);
	XMStoreFloat3(&m_directionalLight.dirW, direction);

	// Point light
	m_pointLight.ambient = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 0.0f);
	m_pointLight.diffuse = DirectX::XMFLOAT4(1.0f, 0.3f, 0.3f, 0.0f);
	m_pointLight.specular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 0.0f);
	m_pointLight.posW = DirectX::XMFLOAT3(-4.25f, -0.5f, 4.25f);
	m_pointLight.range = 1.5f;
	m_pointLight.attenuation = DirectX::XMFLOAT3(0.0f, 0.25f, 0.2f);

	// Spot light
	m_spotLight.ambient = DirectX::XMFLOAT4(0.4f, 0.4f, 0.4f, 0.0f);
	m_spotLight.diffuse = DirectX::XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_spotLight.specular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 0.0f);
	m_spotLight.posW = DirectX::XMFLOAT3(1.0f, 2.5f, -1.25f);
	m_spotLight.range = 4.5f;
	m_spotLight.dirW = DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f);
	direction = XMLoadFloat3(&m_spotLight.dirW);
	direction = XMVector3Normalize(direction);
	XMStoreFloat3(&m_spotLight.dirW, direction);
	m_spotLight.spot = 2.0f;
	m_spotLight.attenuation = DirectX::XMFLOAT3(0.0f, 0.1f, 0.0f);
}

// Initializing shaders
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

	// create the input layout, it must match the Vertex Shader HLSL input format
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(MeshData::Vertex, normal), D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	XTEST_D3D_CHECK(m_d3dDevice->CreateInputLayout(vertexDesc, 2, vsByteCode.Data(), vsByteCode.ByteSize(), &m_inputLayout));
}


void LightsDemoApp::InitBuffers()
{
	// Shapes
	for (ModelsMap::iterator it = m_models.begin();  it != m_models.end(); it++) {
		// Vertices
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.ByteWidth = UINT(sizeof(MeshData::Vertex) * it->second.VerticesCount);
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexInitData;
		vertexInitData.pSysMem = &it->second.Data.vertices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexInitData, &it->second.VertexBuffer));

		// Indices
		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.ByteWidth = UINT(sizeof(uint32) * it->second.IndicesCount);
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexInitdata;
		indexInitdata.pSysMem = &it->second.Data.indices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &indexInitdata, &it->second.IndexBuffer));

		// constant buffer to update the Vertex Shader "PerObjectCB" constant buffer
		D3D11_BUFFER_DESC vsConstantBufferDesc;
		vsConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		vsConstantBufferDesc.ByteWidth = sizeof(PerObjectCB);
		vsConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		vsConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		vsConstantBufferDesc.MiscFlags = 0;
		vsConstantBufferDesc.StructureByteStride = 0;
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vsConstantBufferDesc, nullptr, &it->second.ConstantBuffer));
	}

	// Crate
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.ByteWidth = sizeof(xtest::mesh::MeshData::Vertex) * m_crate.Data.meshData.vertices.size();
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexInitData;
	vertexInitData.pSysMem = m_crate.Data.meshData.vertices.data();
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexInitData, &m_crate.VertexBuffer));

	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.ByteWidth = sizeof(uint32) * m_crate.Data.meshData.indices.size();
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexInitdata;
	indexInitdata.pSysMem = m_crate.Data.meshData.indices.data();
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &indexInitdata, &m_crate.IndexBuffer));

	for (MeshDescriptorMap::iterator it = m_crate.Data.meshDescriptorMapByName.begin(); it != m_crate.Data.meshDescriptorMapByName.end(); it++) {
		Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer;

		D3D11_BUFFER_DESC vsConstantBufferDesc;
		vsConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		vsConstantBufferDesc.ByteWidth = sizeof(PerObjectCB);
		vsConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		vsConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		vsConstantBufferDesc.MiscFlags = 0;
		vsConstantBufferDesc.StructureByteStride = 0;
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vsConstantBufferDesc, nullptr, &vsConstantBuffer));

		m_crate.ConstantBuffers[it->first] = vsConstantBuffer;
	}

	// Lights
	D3D11_BUFFER_DESC lightsBufferDesc;
	lightsBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightsBufferDesc.ByteWidth = sizeof(PerFrameCB);
	lightsBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightsBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightsBufferDesc.MiscFlags = 0;
	lightsBufferDesc.StructureByteStride = 0;
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&lightsBufferDesc, nullptr, &m_lightsBuffer));
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


void xtest::demo::LightsDemoApp::OnMouseMove(const DirectX::XMINT2& movement, const DirectX::XMINT2& currentPos)
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
	XTEST_ASSERT(key == input::Key::F); // the only key registered for this listener

	// re-frame the cube when F key is pressed
	if (status.isDown)
	{
		m_camera.SetPivot({ 0.f, 0.f, 0.f });
	}
}


void LightsDemoApp::UpdateScene(float deltaSeconds)
{
	XTEST_UNUSED_VAR(deltaSeconds);

	m_d3dAnnotation->BeginEvent(L"update-constant-buffer");

	XMMATRIX viewMatrix = m_camera.GetViewMatrix();
	XMStoreFloat4x4(&m_viewMatrix, m_camera.GetViewMatrix());
	XMMATRIX projectionMatrix = XMLoadFloat4x4(&m_projectionMatrix);
	{
		// load the constant buffer data in the gpu
		for (ModelsMap::iterator it = m_models.begin(); it != m_models.end(); it++) {
			XMMATRIX worldMatrix = XMLoadFloat4x4(&it->second.WorldMatrix);
			XMMATRIX rotation = XMMatrixRotationY(it->second.rotationSpeed * deltaSeconds);
			worldMatrix *= rotation;
			XMStoreFloat4x4(&it->second.WorldMatrix, worldMatrix);
			XMMATRIX W_inverseTranspose = XMMatrixInverse(nullptr, worldMatrix);
			XMMATRIX WVP = worldMatrix * viewMatrix * projectionMatrix;

			D3D11_MAPPED_SUBRESOURCE mappedResource;
			ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
				
			// disable gpu access
			XTEST_D3D_CHECK(m_d3dContext->Map(it->second.ConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
			PerObjectCB* constantBufferData = reinterpret_cast<PerObjectCB*>(mappedResource.pData);

			// update the data
			XMStoreFloat4x4(&constantBufferData->W, XMMatrixTranspose(worldMatrix));
			XMStoreFloat4x4(&constantBufferData->W_inverseTranspose, W_inverseTranspose);
			XMStoreFloat4x4(&constantBufferData->WVP, XMMatrixTranspose(WVP));
			constantBufferData->material = it->second.material;

			// enable gpu access
			m_d3dContext->Unmap(it->second.ConstantBuffer.Get(), 0);
		}
	}
	{
		// Crate
		XMMATRIX worldMatrix = XMLoadFloat4x4(&m_crate.WorldMatrix);
		// XMMATRIX scale = XMLoadFloat4x4(&m_crate.ScaleMatrix);
		// XMMATRIX rotation = XMMatrixRotationY(m_crate.rotationSpeed * deltaSeconds);
		// worldMatrix *= scale;
		// worldMatrix *= rotation;
		XMMATRIX W_inverseTranspose = XMMatrixInverse(nullptr, worldMatrix);
		XMStoreFloat4x4(&m_crate.WorldMatrix, worldMatrix);
		XMMATRIX WVP = worldMatrix * viewMatrix * projectionMatrix;
		for (CostantBuffersMap::iterator it = m_crate.ConstantBuffers.begin(); it != m_crate.ConstantBuffers.end(); it++) {
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

			// disable gpu access
			XTEST_D3D_CHECK(m_d3dContext->Map(it->second.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
			PerObjectCB* constantBufferData = reinterpret_cast<PerObjectCB*>(mappedResource.pData);

			//update the data
			XMStoreFloat4x4(&constantBufferData->W, XMMatrixTranspose(worldMatrix));
			XMStoreFloat4x4(&constantBufferData->W_inverseTranspose, W_inverseTranspose);
			XMStoreFloat4x4(&constantBufferData->WVP, XMMatrixTranspose(WVP));
			constantBufferData->material = m_crate.materials[it->first];

			// enable gpu access
			m_d3dContext->Unmap(it->second.Get(), 0);
		}
	}

	{
		// Lights
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		XTEST_D3D_CHECK(m_d3dContext->Map(m_lightsBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		PerFrameCB* constantBufferData = (PerFrameCB*)mappedResource.pData;
		constantBufferData->directionalLight = m_directionalLight;
		constantBufferData->pointLight = m_pointLight;
		constantBufferData->spotLight = m_spotLight;
		constantBufferData->eyePosW = m_camera.GetPosition();
		m_d3dContext->Unmap(m_lightsBuffer.Get(), 0);
	}
	
	m_d3dAnnotation->EndEvent();
}


void LightsDemoApp::RenderScene()
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

	UINT bufferRegister = 1;
	m_d3dContext->PSSetConstantBuffers(bufferRegister, 1, m_lightsBuffer.GetAddressOf());

	// Shapes
	for (ModelsMap::iterator it = m_models.begin(); it != m_models.end(); it++) {

		// bind the constant data to the vertex shader
		bufferRegister = 0;
		m_d3dContext->VSSetConstantBuffers(bufferRegister, 1, it->second.ConstantBuffer.GetAddressOf());
		m_d3dContext->PSSetConstantBuffers(bufferRegister, 1, it->second.ConstantBuffer.GetAddressOf());

		// set what to draw
		UINT stride = sizeof(MeshData::Vertex);
		UINT offset = 0;
		m_d3dContext->IASetVertexBuffers(0, 1, it->second.VertexBuffer.GetAddressOf(), &stride, &offset);
		m_d3dContext->IASetIndexBuffer(it->second.IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// draw
		m_d3dContext->DrawIndexed(it->second.IndicesCount, 0, 0);
	}

	// Crate
	// set what to draw
	UINT stride = sizeof(MeshData::Vertex);
	UINT offset = 0;
	m_d3dContext->IASetVertexBuffers(0, 1, m_crate.VertexBuffer.GetAddressOf(), &stride, &offset);
	m_d3dContext->IASetIndexBuffer(m_crate.IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (CostantBuffersMap::iterator it = m_crate.ConstantBuffers.begin(); it != m_crate.ConstantBuffers.end(); it++) {
		// bind the constant data to the vertex shader
		bufferRegister = 0;
		m_d3dContext->VSSetConstantBuffers(bufferRegister, 1, it->second.GetAddressOf());
		m_d3dContext->PSSetConstantBuffers(bufferRegister, 1, it->second.GetAddressOf());
		// draw with offsets
		m_d3dContext->DrawIndexed(m_crate.Data.meshDescriptorMapByName[it->first].indexCount, 
								m_crate.Data.meshDescriptorMapByName[it->first].indexOffset, 
								m_crate.Data.meshDescriptorMapByName[it->first].vertexOffset);
	}

	// present the frame
	XTEST_D3D_CHECK(m_swapChain->Present(0, 0));

	m_d3dAnnotation->EndEvent();
}