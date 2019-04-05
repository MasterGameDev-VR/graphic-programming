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

LightsDemoApp::LightsDemoApp(HINSTANCE instance,
	const application::WindowSettings& windowSettings,
	const application::DirectxSettings& directxSettings,
	uint32 fps /*=60*/)
	: application::DirectxApp(instance, windowSettings, directxSettings, fps)
	// , m_vertexBuffer(nullptr)
	// , m_indexBuffer(nullptr)
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

	InitMeshes();
	InitMatrices();
	InitShaders();
	InitBuffers();
	InitRasterizerState();

	service::Locator::GetMouse()->AddListener(this);
	service::Locator::GetKeyboard()->AddListener(this, { input::Key::F });

	m_d3dAnnotation->EndEvent();
}


void LightsDemoApp::InitMatrices()
{
	// view matrix
	// m_camera.SetRotation(0.1f, 0.1f);
	XMStoreFloat4x4(&m_viewMatrix, m_camera.GetViewMatrix());

	// projection matrix
	{
		XMMATRIX P = XMMatrixPerspectiveFovLH(math::ToRadians(45.f), AspectRatio(), 1.f, 1000.f);
		XMStoreFloat4x4(&m_projectionMatrix, P);
	}
	/*
	for (ModelsMap::iterator it = m_meshes.begin(); it != m_meshes.end(); it++) {
		// world matrix
		XMStoreFloat4x4(&it->second.WorldMatrix, XMMatrixIdentity() * XMMatrixTranslation(2.f, 0.f, 2.f));
	
	}
	*/
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
	//	{
	//		float3 posL : POSITION;
	//		float4 color : COLOR;
	//	};
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(VertexIn, color), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	XTEST_D3D_CHECK(m_d3dDevice->CreateInputLayout(vertexDesc, 2, vsByteCode.Data(), vsByteCode.ByteSize(), &m_inputLayout));
}


void LightsDemoApp::InitBuffers()
{
	for (ModelsMap::iterator it = m_meshes.begin();  it != m_meshes.end(); it++) {
		// Vertices
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;

		vertexBufferDesc.ByteWidth = sizeof(VertexIn) * it->second.VerticesCount;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexInitData;
		vertexInitData.pSysMem = &it->second.Vertices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexInitData, &it->second.VertexBuffer));

		// Indices
		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.ByteWidth = sizeof(uint32) * it->second.IndicesCount;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexInitdata;
		indexInitdata.pSysMem = &it->second.Indices[0];
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &indexInitdata, &it->second.IndexBuffer));

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
		XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vsConstantBufferDesc, nullptr, &it->second.ConstantBuffer));
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


void LightsDemoApp::InitMeshes() {
	
	// Sphere
	Model sphere;
	MeshData mesh1 = GenerateSphere(1.f, 12, 12);
	sphere.VerticesCount = mesh1.vertices.size();
	for (unsigned int i = 0; i < sphere.VerticesCount; i++) {
		sphere.Vertices.push_back({ XMFLOAT3(mesh1.vertices[i].position), XMFLOAT4(DirectX::Colors::DarkBlue) });
	}

	sphere.IndicesCount = mesh1.indices.size();
	sphere.Indices = mesh1.indices;
	XMStoreFloat4x4(&sphere.WorldMatrix, XMMatrixIdentity() * XMMatrixTranslation(-2.f, 0.f, -2.f));
	m_meshes["sphere"] = sphere;
	
	// Box 	
	Model box;
	MeshData mesh2 = GenerateBox(1.5f, 1.5f, 1.5f);
	box.VerticesCount = mesh2.vertices.size();
	for (unsigned int i = 0; i < box.VerticesCount; i++) {
		box.Vertices.push_back({ XMFLOAT3(mesh2.vertices[i].position), XMFLOAT4(DirectX::Colors::Blue) });
	}

	box.IndicesCount = mesh2.indices.size();
	box.Indices = mesh2.indices;
	XMStoreFloat4x4(&box.WorldMatrix, XMMatrixIdentity());
	m_meshes["box"] = box;

	// Plane
	Model plane;
	MeshData mesh3 = GeneratePlane(10.f, 10.f, 2, 2);
	plane.VerticesCount = mesh3.vertices.size();
	for (unsigned int i = 0; i < plane.VerticesCount; i++) {
		plane.Vertices.push_back({ XMFLOAT3(mesh3.vertices[i].position), XMFLOAT4(DirectX::Colors::LightBlue) });
	}

	plane.IndicesCount = mesh3.indices.size();
	plane.Indices = mesh3.indices;
	XMStoreFloat4x4(&plane.WorldMatrix, XMMatrixIdentity());
	m_meshes["plane"] = plane;
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

	XMStoreFloat4x4(&m_viewMatrix, m_camera.GetViewMatrix());
	{
		// load the constant buffer data in the gpu
		unsigned i = 1;
		for (ModelsMap::iterator it = m_meshes.begin(); it != m_meshes.end(); i *= 2, it++) {
			if (it->first != "plane") {
				XMMATRIX rotation = XMMatrixRotationY(math::ToRadians(30.0f * i) * deltaSeconds);
				// rotation *= XMMatrixRotationX(math::ToRadians(180.0f) * deltaSeconds);
				// rotation *= XMMatrixRotationZ(math::ToRadians(45.0f) * deltaSeconds);
				XMMATRIX worldMatrix = XMLoadFloat4x4(&it->second.WorldMatrix);
				worldMatrix *= rotation;
				XMStoreFloat4x4(&it->second.WorldMatrix, worldMatrix);
			}
		}
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

	XMMATRIX viewMatrix = XMLoadFloat4x4(&m_viewMatrix);
	XMMATRIX projectionMatrix = XMLoadFloat4x4(&m_projectionMatrix);
	for (ModelsMap::iterator it = m_meshes.begin(); it != m_meshes.end(); it++) {

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		// disable gpu access
		XTEST_D3D_CHECK(m_d3dContext->Map(it->second.ConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		PerObjectCB* constantBufferData = reinterpret_cast<PerObjectCB*>(mappedResource.pData);

		XMMATRIX worldMatrix = XMLoadFloat4x4(&it->second.WorldMatrix);
		XMMATRIX WVP = worldMatrix * viewMatrix * projectionMatrix;
			   
		//update the data
		XMStoreFloat4x4(&constantBufferData->WVP, XMMatrixTranspose(WVP));

		// enable gpu access
		m_d3dContext->Unmap(it->second.ConstantBuffer.Get(), 0);

		// bind the constant data to the vertex shader
		UINT bufferRegister = 0; // PerObjectCB was defined as register 0 inside the vertex shader file
		m_d3dContext->VSSetConstantBuffers(bufferRegister, 1, it->second.ConstantBuffer.GetAddressOf());
		// set what to draw
		UINT stride = sizeof(VertexIn);
		UINT offset = 0;
		m_d3dContext->IASetVertexBuffers(0, 1, it->second.VertexBuffer.GetAddressOf(), &stride, &offset);
		m_d3dContext->IASetIndexBuffer(it->second.IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// draw
		m_d3dContext->DrawIndexed(it->second.IndicesCount, 0, 0);
	}

	// present the frame
	XTEST_D3D_CHECK(m_swapChain->Present(0, 0));

	m_d3dAnnotation->EndEvent();
}