#include "stdafx.h"

#include "stdafx.h"
#include "lights_demo_app.h"
#include <file/file_utils.h>
#include <math/math_utils.h>
#include <service/locator.h>
#include <mesh/mesh_generator.h>


using namespace DirectX;
using namespace xtest;

using xtest::demo::LightDemoApp;
using Microsoft::WRL::ComPtr;

LightDemoApp::LightDemoApp(HINSTANCE instance,
	const application::WindowSettings& windowSettings,
	const application::DirectxSettings& directxSettings,
	uint32 fps /*=60*/)
	: application::DirectxApp(instance, windowSettings, directxSettings, fps)
	, m_vertexShader(nullptr)
	, m_pixelShader(nullptr)
	, m_inputLayout(nullptr)
	, m_camera(math::ToRadians(90.f), math::ToRadians(30.f), 10.f, { 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, { math::ToRadians(5.f), math::ToRadians(175.f) }, { 3.f, 25.f })
{
	m_objectsNumber = 3;
	m_objects = std::vector<ObjectToDraw>(m_objectsNumber);

	//LIGHTS DEFINITION
	m_dirLight.ambient = DirectX::XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_dirLight.diffuse = DirectX::XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_dirLight.specular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 0.0f);
	m_dirLight.dirW = DirectX::XMFLOAT3(-1.f, -1.f, 0.f);
	{
		XMVECTOR direction = XMLoadFloat3(&m_dirLight.dirW);
		direction = XMVector3Normalize(direction);
		XMStoreFloat3(&m_dirLight.dirW, direction);
	}

	m_pLight.ambient = DirectX::XMFLOAT4(0.1f, 0.4f, 0.1f, 0.0f);
	m_pLight.diffuse = DirectX::XMFLOAT4(0.2f, 2.5f, 0.2f, 0.0f);
	m_pLight.specular = DirectX::XMFLOAT4(0.2f, 1.f, 0.3f, 0.0f);
	m_pLight.posW = DirectX::XMFLOAT3(0.f, 4.f, 4.f);
	m_pLight.range = 18.f;
	m_pLight.attenuation = DirectX::XMFLOAT3(0.1f, 0.2f, 1.f);

	m_sLight.ambient = DirectX::XMFLOAT4(1.f, 0.3f, 0.3f, 0.0f);
	m_sLight.diffuse = DirectX::XMFLOAT4(5.f, 0.4f, 0.4f, 0.0f);
	m_sLight.specular = DirectX::XMFLOAT4(3.f, 0.5f, 0.5f, 0.0f);
	m_sLight.posW = DirectX::XMFLOAT3(-8.f, 3.f, -2.f);
	m_sLight.range = 20.f;
	m_sLight.dirW = DirectX::XMFLOAT3(5.f, -2.5f, 1.f);
	m_sLight.spot = 50.f;
	m_sLight.attenuation = DirectX::XMFLOAT3(0.0f, 0.1f, 0.3f);
	{
		XMVECTOR direction = XMLoadFloat3(&m_sLight.dirW);
		direction = XMVector3Normalize(direction);
		XMStoreFloat3(&m_sLight.dirW, direction);
	}

	//OBJECTS DEFINITION

	//PLANE
	{
		Material material;
		material.ambient = DirectX::XMFLOAT4(0.3f, 0.3f, 0.5f, 0.0f);
		material.diffuse = DirectX::XMFLOAT4(0.5f, 0.5f, 0.6f, 0.0f);
		material.specular = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 10.f);

		m_objects[0].material = material;
	}

	//SPHERE
	{
		Material material;
		material.ambient = DirectX::XMFLOAT4(0.2f, 0.1f, 0.6f, 0.0f);
		material.diffuse = DirectX::XMFLOAT4(0.3f, 0.3f, 1.0f, 0.0f);
		material.specular = DirectX::XMFLOAT4(0.4f, 0.1f, 0.2f, 60.f);

		m_objects[1].material = material;
	}

	//TORUS
	{
		Material material;
		material.ambient = DirectX::XMFLOAT4(0.7f, 0.6f, 0.1f, 0.0f);
		material.diffuse = DirectX::XMFLOAT4(0.8f, 0.6f, 0.1f, 0.0f);
		material.specular = DirectX::XMFLOAT4(0.7f, 0.7f, 0.4f, 20.f);

		m_objects[2].material = material;
	}

	//bottom_1
	{
		Material material;

		material.ambient = DirectX::XMFLOAT4(0.8f, 0.5f, 0.1f, 0.0f);
		material.diffuse = DirectX::XMFLOAT4(0.9f, 0.5f, 0.2f, 0.0f);
		material.specular = DirectX::XMFLOAT4(0.9f, 0.7f, 0.3f, 10.f);

		m_crateMaterials.emplace("bottom_1", material);
	}
	//handles_8
	{
		Material material;

		material.ambient = DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f);
		material.diffuse = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 0.0f);
		material.specular = DirectX::XMFLOAT4(0.7f, 0.7f, 0.7f, 100.f);

		m_crateMaterials.emplace("handles_8", material);
	}
	//metal_pieces_3
	{
		Material material;

		material.ambient = DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f);
		material.diffuse = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 0.0f);
		material.specular = DirectX::XMFLOAT4(0.7f, 0.7f, 0.7f, 100.f);

		m_crateMaterials.emplace("metal_pieces_3", material);
	}
	//top_2
	{
		Material material;

		material.ambient = DirectX::XMFLOAT4(0.7f, 0.7f, 0.7f, 0.0f);
		material.diffuse = DirectX::XMFLOAT4(0.9f, 0.9f, 0.9f, 0.0f);
		material.specular = DirectX::XMFLOAT4(0.3f, 0.3f, 0.3f, 20.f);

		m_crateMaterials.emplace("top_2", material);
	}
	//top_handles_4
	{
		Material material;

		material.ambient = DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f);
		material.diffuse = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 0.0f);
		material.specular = DirectX::XMFLOAT4(0.7f, 0.7f, 0.7f, 100.f);

		m_crateMaterials.emplace("top_handles_4", material);
	}
}


LightDemoApp::~LightDemoApp()
{}


void LightDemoApp::Init()
{
	application::DirectxApp::Init();

	m_d3dAnnotation->BeginEvent(L"init-demo");

	InitMatrices();
	InitShaders();
	InitBuffers();
	InitRasterizerState();

	service::Locator::GetMouse()->AddListener(this);
	service::Locator::GetKeyboard()->AddListener(this, { input::Key::F });

	m_d3dAnnotation->EndEvent();
}

void LightDemoApp::InitMatrices()
{

	//PLANE
	{
		XMMATRIX matrix = XMMatrixIdentity();

		XMStoreFloat4x4(&(m_objects[0].worldMatrix), matrix);
	}

	//SPHERE
	{
		XMMATRIX matrix = XMMatrixIdentity() * XMMatrixTranslation(1.0f, 1.0f, 5.0f);

		XMStoreFloat4x4(&(m_objects[1].worldMatrix), matrix);
	}

	//TORUS
	{
		XMMATRIX matrix = XMMatrixIdentity() * XMMatrixTranslation(1.0f, 0.5f, 5.0f);

		XMStoreFloat4x4(&(m_objects[2].worldMatrix), matrix);
	}

	//CRATE
	{
		XMMATRIX matrix = XMMatrixIdentity();;
		matrix *= XMMatrixScaling(0.01f, 0.01f, 0.01f);
		matrix *= XMMatrixRotationY(math::ToRadians(90.f));

		XMStoreFloat4x4(&(m_crateWorldMatrix), matrix);
	}

	// view matrix
	XMStoreFloat4x4(&m_viewMatrix, m_camera.GetViewMatrix());

	// projection matrix
	{
		XMMATRIX P = XMMatrixPerspectiveFovLH(math::ToRadians(45.f), AspectRatio(), 1.f, 1000.f);
		XMStoreFloat4x4(&m_projectionMatrix, P);
	}
}

void LightDemoApp::InitShaders()
{
	// read pre-compiled shaders' bytecode
	std::future<file::BinaryFile> psByteCodeFuture = file::ReadBinaryFile(std::wstring(GetRootDir()).append(L"\\lights_demo_PS.cso"));
	std::future<file::BinaryFile> vsByteCodeFuture = file::ReadBinaryFile(std::wstring(GetRootDir()).append(L"\\lights_demo_VS.cso"));

	// future.get() can be called only once
	file::BinaryFile vsByteCode = vsByteCodeFuture.get();
	file::BinaryFile psByteCode = psByteCodeFuture.get();
	XTEST_D3D_CHECK(m_d3dDevice->CreateVertexShader(vsByteCode.Data(), vsByteCode.ByteSize(), nullptr, &m_vertexShader));
	XTEST_D3D_CHECK(m_d3dDevice->CreatePixelShader(psByteCode.Data(), psByteCode.ByteSize(), nullptr, &m_pixelShader));


	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(xtest::mesh::MeshData::Vertex, normal), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	XTEST_D3D_CHECK(m_d3dDevice->CreateInputLayout(vertexDesc, 2, vsByteCode.Data(), vsByteCode.ByteSize(), &m_inputLayout));
}


void LightDemoApp::InitBuffers()
{
	//PRIMITIVE OBJECTS BUFFERS
	std::vector< xtest::mesh::MeshData> dataVector;
	dataVector.push_back(xtest::mesh::GeneratePlane(20.0f, 20.0f, 10, 10));
	dataVector.push_back(xtest::mesh::GenerateSphere(1.0f, 30, 30));
	dataVector.push_back(xtest::mesh::GenerateTorus(1.5f, 0.4f, 30, 30));

	for (size_t i = 0; i < m_objectsNumber; i++) {
		m_objects[i].indeces = dataVector[i].indices.size();

		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.ByteWidth = sizeof(xtest::mesh::MeshData::Vertex) * dataVector[i].vertices.size();
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0; 
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexInitData;
		vertexInitData.pSysMem = dataVector[i].vertices.data();
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexInitData, &(m_objects[i].vertexBuffer)));

		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.ByteWidth = sizeof(uint32) * dataVector[i].indices.size();
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexInitdata;
		indexInitdata.pSysMem = dataVector[i].indices.data();
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &indexInitdata, &(m_objects[i].indexBuffer)));

		D3D11_BUFFER_DESC vsConstantBufferDesc;
		vsConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC; // this buffer needs to be updated every frame
		vsConstantBufferDesc.ByteWidth = sizeof(PerObjectCB);
		vsConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		vsConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		vsConstantBufferDesc.MiscFlags = 0;
		vsConstantBufferDesc.StructureByteStride = 0;
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vsConstantBufferDesc, nullptr, &(m_objects[i].constantBuffer)));
	}

	//CRATE BUFFERS
	mesh::GPFMesh crateMesh = xtest::file::ReadGPF(std::wstring(GetRootDir()).append(L"\\3d-objects\\crate.gpf"));
	m_crateMeshDescriptorMap = crateMesh.meshDescriptorMapByName;

	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.ByteWidth = sizeof(xtest::mesh::MeshData::Vertex) * crateMesh.meshData.vertices.size();
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexInitData;
	vertexInitData.pSysMem = crateMesh.meshData.vertices.data();
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexInitData, &(m_crateVertexBuffer)));

	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.ByteWidth = sizeof(uint32) * crateMesh.meshData.indices.size();
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexInitdata;
	indexInitdata.pSysMem = crateMesh.meshData.indices.data();
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &indexInitdata, &(m_crateIndexBuffer)));

	for (auto it = crateMesh.meshDescriptorMapByName.begin(); it != crateMesh.meshDescriptorMapByName.end(); it++) {

		Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

		D3D11_BUFFER_DESC vsConstantBufferDesc;
		vsConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC; // this buffer needs to be updated every frame
		vsConstantBufferDesc.ByteWidth = sizeof(PerObjectCB);
		vsConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		vsConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		vsConstantBufferDesc.MiscFlags = 0;
		vsConstantBufferDesc.StructureByteStride = 0;
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vsConstantBufferDesc, nullptr, &(constantBuffer)));

		m_crateConstantBufferMap.emplace(it->first, constantBuffer);
	}

	//LIGHTS BUFFER
	D3D11_BUFFER_DESC lightBufferDesc;
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(PerFrameCB);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&lightBufferDesc, nullptr, &m_vsLightBuffer));
}


void LightDemoApp::InitRasterizerState()
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


void LightDemoApp::OnResized()
{
	application::DirectxApp::OnResized();

	//update the projection matrix with the new aspect ratio
	XMMATRIX P = XMMatrixPerspectiveFovLH(math::ToRadians(45.f), AspectRatio(), 1.f, 1000.f);
	XMStoreFloat4x4(&m_projectionMatrix, P);
}


void LightDemoApp::OnWheelScroll(input::ScrollStatus scroll)
{
	// zoom in or out when the scroll wheel is used
	if (service::Locator::GetMouse()->IsInClientArea())
	{
		m_camera.IncreaseRadiusBy(scroll.isScrollingUp ? -0.5f : 0.5f);
	}
}


void xtest::demo::LightDemoApp::OnMouseMove(const DirectX::XMINT2& movement, const DirectX::XMINT2& currentPos)
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

void LightDemoApp::OnKeyStatusChange(input::Key key, const input::KeyStatus& status)
{
	XTEST_ASSERT(key == input::Key::F); // the only key registered for this listener

	// re-frame the cube when F key is pressed
	if (status.isDown)
	{
		m_camera.SetPivot({ 0.f, 0.f, 0.f });
	}
}


void LightDemoApp::UpdateScene(float deltaSeconds)
{
	XTEST_UNUSED_VAR(deltaSeconds);

	


	m_d3dAnnotation->BeginEvent(L"update-constant-buffer");

	//OBJECTS CONSTATNT BUFFER UPDATE
	for (size_t i = 0; i < m_objectsNumber; i++) {

		XMMATRIX W = XMLoadFloat4x4(&(m_objects[i].worldMatrix));
		XMStoreFloat4x4(&(m_objects[i].worldMatrix), W);

		XMMATRIX W_inverseTranspose = XMMatrixInverse(nullptr, W);

		// create the model-view-projection matrix
		XMMATRIX V = m_camera.GetViewMatrix();
		XMStoreFloat4x4(&m_viewMatrix, V);

		// create projection matrix
		XMMATRIX P = XMLoadFloat4x4(&m_projectionMatrix);
		XMMATRIX WVP = W * V*P;

		// matrices must be transposed since HLSL use column-major ordering.
		WVP = XMMatrixTranspose(WVP);
		W = XMMatrixTranspose(W);

		// load the constant buffer data in the gpu
		{
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

			// disable gpu access
			XTEST_D3D_CHECK(m_d3dContext->Map(m_objects[i].constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
			PerObjectCB* constantBufferData = (PerObjectCB*)mappedResource.pData;

			//update the data
			XMStoreFloat4x4(&constantBufferData->WVP, WVP);
			XMStoreFloat4x4(&constantBufferData->W, W);
			XMStoreFloat4x4(&constantBufferData->W_inverseTranspose, W_inverseTranspose);
			constantBufferData->material = m_objects[i].material;


			// enable gpu access
			m_d3dContext->Unmap(m_objects[i].constantBuffer.Get(), 0);
		}
	}

	//CRATE CONSTANT BUFFER UPDATE
	for (auto it = m_crateMeshDescriptorMap.begin(); it != m_crateMeshDescriptorMap.end(); it++) {

		XMMATRIX W = XMLoadFloat4x4(&(m_crateWorldMatrix));
		XMStoreFloat4x4(&(m_crateWorldMatrix), W);

		XMMATRIX W_inverseTranspose = XMMatrixInverse(nullptr, W);

		// create the model-view-projection matrix
		XMMATRIX V = m_camera.GetViewMatrix();
		XMStoreFloat4x4(&m_viewMatrix, V);

		// create projection matrix
		XMMATRIX P = XMLoadFloat4x4(&m_projectionMatrix);
		XMMATRIX WVP = W * V*P;

		// matrices must be transposed since HLSL use column-major ordering.
		WVP = XMMatrixTranspose(WVP);
		W = XMMatrixTranspose(W);

		// load the constant buffer data in the gpu
		{
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

			// disable gpu access
			XTEST_D3D_CHECK(m_d3dContext->Map(m_crateConstantBufferMap.at(it->first).Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
			PerObjectCB* constantBufferData = (PerObjectCB*)mappedResource.pData;

			//update the data
			XMStoreFloat4x4(&constantBufferData->WVP, WVP);
			XMStoreFloat4x4(&constantBufferData->W, W);
			XMStoreFloat4x4(&constantBufferData->W_inverseTranspose, W_inverseTranspose);
			constantBufferData->material = m_crateMaterials.at(it->first);

			// enable gpu access
			m_d3dContext->Unmap(m_crateConstantBufferMap.at(it->first).Get(), 0);
		}
	}

	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		// disable gpu access
		XTEST_D3D_CHECK(m_d3dContext->Map(m_vsLightBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		PerFrameCB* constantBufferData = (PerFrameCB*)mappedResource.pData;

		constantBufferData->dirLigth = m_dirLight;
		constantBufferData->pLight = m_pLight;
		constantBufferData->sLight = m_sLight;
		constantBufferData->eyePos = m_camera.GetPosition();

		// enable gpu access
		m_d3dContext->Unmap(m_vsLightBuffer.Get(), 0);
	}

	m_d3dAnnotation->EndEvent();
}


void LightDemoApp::RenderScene()
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

	//Sets the light constant buffer for all objects
	int bufferRegister = 1;
	m_d3dContext->PSSetConstantBuffers(bufferRegister, 1, m_vsLightBuffer.GetAddressOf());

	//OBJECTS DRAW
	for (size_t i = 0; i < m_objectsNumber; i++) {
		
		bufferRegister = 0; 
		m_d3dContext->VSSetConstantBuffers(bufferRegister, 1, m_objects[i].constantBuffer.GetAddressOf());
		m_d3dContext->PSSetConstantBuffers(bufferRegister, 1, m_objects[i].constantBuffer.GetAddressOf());

		// set what to draw
		UINT stride = sizeof(xtest::mesh::MeshData::Vertex);
		UINT offset = 0;
		m_d3dContext->IASetVertexBuffers(0, 1, m_objects[i].vertexBuffer.GetAddressOf(), &stride, &offset);
		m_d3dContext->IASetIndexBuffer(m_objects[i].indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// draw and present the frame
		m_d3dContext->DrawIndexed(m_objects[i].indeces, 0, 0);
	}

	//CRATE DRAW

	//Sets the vertex buffer and the index buffer for all the components of the crate 
	UINT stride = sizeof(xtest::mesh::MeshData::Vertex);
	UINT offset = 0;
	m_d3dContext->IASetVertexBuffers(0, 2, m_crateVertexBuffer.GetAddressOf(), &stride, &offset);
	m_d3dContext->IASetIndexBuffer(m_crateIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Draws all the componenets of the crate
	for (auto it = m_crateMeshDescriptorMap.begin(); it != m_crateMeshDescriptorMap.end(); it++) {
			
			bufferRegister = 0;
			m_d3dContext->VSSetConstantBuffers(bufferRegister, 1, m_crateConstantBufferMap.at(it->first).GetAddressOf());
			m_d3dContext->PSSetConstantBuffers(bufferRegister, 1, m_crateConstantBufferMap.at(it->first).GetAddressOf());

			// draw and present the frame
			m_d3dContext->DrawIndexed(it->second.indexCount, it->second.indexOffset, it->second.vertexOffset);
	}

	XTEST_D3D_CHECK(m_swapChain->Present(0, 0));

	m_d3dAnnotation->EndEvent();
}

