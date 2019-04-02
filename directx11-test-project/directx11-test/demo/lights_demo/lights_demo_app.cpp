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
	objects = std::vector<ObjectToDraw>(m_objectsNumber);

	//LIGHTS DEFINITION
	dirLight.ambient = DirectX::XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	dirLight.diffuse = DirectX::XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	dirLight.specular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 0.0f);
	dirLight.dirW = DirectX::XMFLOAT3(-1.f, -1.f, 0.f);
	{
		XMVECTOR direction = XMLoadFloat3(&dirLight.dirW);
		direction = XMVector3Normalize(direction);
		XMStoreFloat3(&dirLight.dirW, direction);
	}
	

	pLight.ambient = DirectX::XMFLOAT4(0.1f, 0.4f, 0.1f, 0.0f);
	pLight.diffuse = DirectX::XMFLOAT4(0.1f, 0.8f, 0.1f, 0.0f);
	pLight.specular = DirectX::XMFLOAT4(0.2f, 0.8f, 0.3f, 0.0f);
	pLight.posW = DirectX::XMFLOAT3(0.f, 3.f, 3.f);
	pLight.range = 10.f;
	pLight.attenuation = DirectX::XMFLOAT3(0.1f, 0.8f, 0.2f);

	sLight.ambient = DirectX::XMFLOAT4(5.f, 0.3f, 0.3f, 0.0f);
	sLight.diffuse = DirectX::XMFLOAT4(1.f, 0.4f, 0.4f, 0.0f);
	sLight.specular = DirectX::XMFLOAT4(1.f, 0.5f, 0.5f, 0.0f);
	sLight.posW = DirectX::XMFLOAT3(0.f, 3.f, -8.f);
	sLight.range = 20.f;
	sLight.dirW = DirectX::XMFLOAT3(0.f, -1.f, 2.f);
	sLight.spot = 50.f;
	sLight.attenuation = DirectX::XMFLOAT3(0.0f, 0.1f, 0.2f);
	{
		XMVECTOR direction = XMLoadFloat3(&sLight.dirW);
		direction = XMVector3Normalize(direction);
		XMStoreFloat3(&sLight.dirW, direction);
	}

	//OBJECTS DEFINITION

	//PLANE
	{
		Material material;
		material.ambient = DirectX::XMFLOAT4(0.3f, 0.3f, 0.5f, 0.0f);
		material.diffuse = DirectX::XMFLOAT4(0.5f, 0.5f, 0.6f, 0.0f);
		material.specular = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 10.f);

		objects[0].material = material;
	}

	//SPHERE
	{
		Material material;
		material.ambient = DirectX::XMFLOAT4(0.2f, 0.1f, 0.6f, 0.0f);
		material.diffuse = DirectX::XMFLOAT4(0.3f, 0.3f, 1.0f, 0.0f);
		material.specular = DirectX::XMFLOAT4(0.4f, 0.1f, 0.2f, 60.f);

		objects[1].material = material;
	}

	//TORUS
	{
		Material material;
		material.ambient = DirectX::XMFLOAT4(0.7f, 0.6f, 0.1f, 0.0f);
		material.diffuse = DirectX::XMFLOAT4(0.8f, 0.6f, 0.1f, 0.0f);
		material.specular = DirectX::XMFLOAT4(0.7f, 0.7f, 0.4f, 60.f);

		objects[2].material = material;
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

		XMStoreFloat4x4(&(objects[0].worldMatrix), matrix);
	}

	//SPHERE
	{
		XMMATRIX matrix = XMMatrixIdentity() * XMMatrixTranslation(0.0f, 1.0f, 0.0f);

		XMStoreFloat4x4(&(objects[1].worldMatrix), matrix);
	}

	//TORUS
	{
		XMMATRIX matrix = XMMatrixIdentity() * XMMatrixTranslation(0.0f, 0.5f, 0.0f);

		XMStoreFloat4x4(&(objects[2].worldMatrix), matrix);
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


	// create the input layout, it must match the Vertex Shader HLSL input format:
	//	struct VertexIn
	//	{
	//		float3 posL : POSITION;
	//		float4 color : COLOR;
	//	};
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(xtest::mesh::MeshData::Vertex, normal), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	XTEST_D3D_CHECK(m_d3dDevice->CreateInputLayout(vertexDesc, 2, vsByteCode.Data(), vsByteCode.ByteSize(), &m_inputLayout));
}


void LightDemoApp::InitBuffers()
{
	std::vector< xtest::mesh::MeshData> dataVector;
	dataVector.push_back(xtest::mesh::GeneratePlane(20.0f, 20.0f, 10, 10));
	dataVector.push_back(xtest::mesh::GenerateSphere(1.0f, 30, 30));
	dataVector.push_back(xtest::mesh::GenerateTorus(1.5f, 0.4f, 30, 30));

	//mesh::GPFMesh boxMesh = xtest::file::ReadGPF(L"\\crate.gpf");

	for (size_t i = 0; i < m_objectsNumber; i++) {
		objects[i].indeces = dataVector[i].indices.size();

		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.ByteWidth = sizeof(xtest::mesh::MeshData::Vertex) * dataVector[i].vertices.size();
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0; 
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexInitData;
		vertexInitData.pSysMem = dataVector[i].vertices.data();
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexInitData, &(objects[i].m_vertexBuffer)));

		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.ByteWidth = sizeof(uint32) * dataVector[i].indices.size();
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexInitdata;
		indexInitdata.pSysMem = dataVector[i].indices.data();
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &indexInitdata, &(objects[i].m_indexBuffer)));


		// constant buffer to update the Vertex Shader "PerObjectCB" constant buffer:
		//	 cbuffer PerObjectCB
		//	 {
		//		 float4x4 WVP;
		//	 };
		D3D11_BUFFER_DESC vsConstantBufferDesc;
		vsConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC; // this buffer needs to be updated every frame
		vsConstantBufferDesc.ByteWidth = sizeof(PerObjectCB);
		vsConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		vsConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		vsConstantBufferDesc.MiscFlags = 0;
		vsConstantBufferDesc.StructureByteStride = 0;
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vsConstantBufferDesc, nullptr, &(objects[i].m_constantBuffer)));
	}

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

	for (size_t i = 0; i < m_objectsNumber; i++) {

		XMMATRIX W = XMLoadFloat4x4(&(objects[i].worldMatrix));
		XMStoreFloat4x4(&(objects[i].worldMatrix), W);

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
			XTEST_D3D_CHECK(m_d3dContext->Map(objects[i].m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
			PerObjectCB* constantBufferData = (PerObjectCB*)mappedResource.pData;

			//update the data
			XMStoreFloat4x4(&constantBufferData->WVP, WVP);
			XMStoreFloat4x4(&constantBufferData->W, W);
			constantBufferData->material = objects[i].material;


			// enable gpu access
			m_d3dContext->Unmap(objects[i].m_constantBuffer.Get(), 0);
		}
	}

	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		// disable gpu access
		XTEST_D3D_CHECK(m_d3dContext->Map(m_vsLightBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		PerFrameCB* constantBufferData = (PerFrameCB*)mappedResource.pData;

		constantBufferData->dirLigth = dirLight;
		constantBufferData->pLight = pLight;
		constantBufferData->sLight = sLight;
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

	for (size_t i = 0; i < m_objectsNumber; i++) {
		// bind the constant data to the vertex shader
		UINT bufferRegister = 0; // PerObjectCB was defined as register 0 inside the vertex shader file
		m_d3dContext->VSSetConstantBuffers(bufferRegister, 1, objects[i].m_constantBuffer.GetAddressOf());
		m_d3dContext->PSSetConstantBuffers(bufferRegister, 1, objects[i].m_constantBuffer.GetAddressOf());

		bufferRegister = 1;
		m_d3dContext->PSSetConstantBuffers(bufferRegister, 1, m_vsLightBuffer.GetAddressOf());

		// set what to draw
		UINT stride = sizeof(xtest::mesh::MeshData::Vertex);
		UINT offset = 0;
		m_d3dContext->IASetVertexBuffers(0, 1, objects[i].m_vertexBuffer.GetAddressOf(), &stride, &offset);
		m_d3dContext->IASetIndexBuffer(objects[i].m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// draw and present the frame
		m_d3dContext->DrawIndexed(objects[i].indeces, 0, 0);
	}
	XTEST_D3D_CHECK(m_swapChain->Present(0, 0));

	m_d3dAnnotation->EndEvent();
}

