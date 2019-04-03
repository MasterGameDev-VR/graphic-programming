#include "stdafx.h"
#include "lights_demo_app.h"
#include <file/file_utils.h>
#include <math/math_utils.h>
#include <service/locator.h>

namespace mesh {
	class MeshData;
}

using namespace DirectX;
using namespace xtest;

using xtest::demo::LightsDemoApp;
using Microsoft::WRL::ComPtr;

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
	// world matrix
	XMStoreFloat4x4(&m_worldMatrix, XMMatrixIdentity());

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
	//	{
	//		float3 posL : POSITION;
	//		float4 color : COLOR;
	//	};
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  offsetof(mesh::MeshData::Vertex, normal), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(mesh::MeshData::Vertex, tangentU), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(mesh::MeshData::Vertex, uv), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	XTEST_D3D_CHECK(m_d3dDevice->CreateInputLayout(vertexDesc, 4, vsByteCode.Data(), vsByteCode.ByteSize(), &m_inputLayout));
}


void LightsDemoApp::InitBuffers()
{
	// vertex buffer composed by the cube's vertices
	m_plane.mesh = mesh::GeneratePlane(500, 500, 2, 2);
	m_sphere.mesh = mesh::GenerateSphere(5,2,2);
	m_box.mesh = mesh::GenerateBox(5,5,5);

	//plane buffers
	D3D11_BUFFER_DESC vertexBufferDescPlane;
	vertexBufferDescPlane.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDescPlane.ByteWidth = UINT(sizeof(mesh::MeshData::Vertex) * m_plane.mesh.vertices.size());
	vertexBufferDescPlane.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDescPlane.CPUAccessFlags = 0;
	vertexBufferDescPlane.MiscFlags = 0;
	vertexBufferDescPlane.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexInitData;
	vertexInitData.pSysMem = m_plane.mesh.vertices.data();
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDescPlane, &vertexInitData, m_plane.m_vertexBuffer.GetAddressOf()));

	D3D11_BUFFER_DESC indexBufferDescPlane;
	indexBufferDescPlane.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDescPlane.ByteWidth = UINT(sizeof(uint32) * m_plane.mesh.indices.size());
	indexBufferDescPlane.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDescPlane.CPUAccessFlags = 0;
	indexBufferDescPlane.MiscFlags = 0;
	indexBufferDescPlane.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexInitdata;
	indexInitdata.pSysMem = m_plane.mesh.indices.data();
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDescPlane, &indexInitdata, m_plane.m_indexBuffer.GetAddressOf()));


	//sphere buffers

	D3D11_BUFFER_DESC vertexBufferDescSphere;
	vertexBufferDescSphere.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDescSphere.ByteWidth = UINT(sizeof(mesh::MeshData::Vertex) * m_sphere.mesh.vertices.size());
	vertexBufferDescSphere.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDescSphere.CPUAccessFlags = 0;
	vertexBufferDescSphere.MiscFlags = 0;
	vertexBufferDescSphere.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexInitDataSphere;
	vertexInitData.pSysMem = m_sphere.mesh.vertices.data();
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDescSphere, &vertexInitData, m_sphere.m_vertexBuffer.GetAddressOf()));

	D3D11_BUFFER_DESC indexBufferDescSphere;
	indexBufferDescSphere.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDescSphere.ByteWidth = UINT(sizeof(uint32) * m_sphere.mesh.indices.size());
	indexBufferDescSphere.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDescSphere.CPUAccessFlags = 0;
	indexBufferDescSphere.MiscFlags = 0;
	indexBufferDescSphere.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexInitdataSphere;
	indexInitdata.pSysMem = m_sphere.mesh.indices.data();
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDescSphere, &indexInitdata, m_sphere.m_indexBuffer.GetAddressOf()));

	//box buffers
	D3D11_BUFFER_DESC vertexBufferDescBox;
	vertexBufferDescBox.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDescBox.ByteWidth = UINT(sizeof(mesh::MeshData::Vertex) * m_box.mesh.vertices.size());
	vertexBufferDescBox.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDescBox.CPUAccessFlags = 0;
	vertexBufferDescBox.MiscFlags = 0;
	vertexBufferDescBox.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexInitDataBox;
	vertexInitData.pSysMem = m_box.mesh.vertices.data();
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDescBox, &vertexInitData, m_box.m_vertexBuffer.GetAddressOf()));

	D3D11_BUFFER_DESC indexBufferDescBox;
	indexBufferDescBox.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDescBox.ByteWidth = UINT(sizeof(uint32) * m_box.mesh.indices.size());
	indexBufferDescBox.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDescBox.CPUAccessFlags = 0;
	indexBufferDescBox.MiscFlags = 0;
	indexBufferDescBox.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexInitdataBox;
	indexInitdata.pSysMem = m_box.mesh.indices.data();
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDescBox, &indexInitdata, m_box.m_indexBuffer.GetAddressOf()));

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
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vsConstantBufferDesc, nullptr, &m_vsConstantBuffer));
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


void xtest::demo::LightsDemoApp::OnMouseMove(const DirectX::XMINT2 & movement, const DirectX::XMINT2 & currentPos)
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

void LightsDemoApp::OnKeyStatusChange(input::Key key, const input::KeyStatus & status)
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

	XMMATRIX W = XMLoadFloat4x4(&m_worldMatrix);
	XMStoreFloat4x4(&m_worldMatrix, W);

	// create the model-view-projection matrix
	XMMATRIX V = m_camera.GetViewMatrix();
	XMStoreFloat4x4(&m_viewMatrix, V);

	// create projection matrix
	XMMATRIX P = XMLoadFloat4x4(&m_projectionMatrix);
	XMMATRIX WVP = W * V * P;

	// matrices must be transposed since HLSL use column-major ordering.
	WVP = XMMatrixTranspose(WVP);


	m_d3dAnnotation->BeginEvent(L"update-constant-buffer");

	// load the constant buffer data in the gpu
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		// disable gpu access
		XTEST_D3D_CHECK(m_d3dContext->Map(m_vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		PerObjectCB* constantBufferData = (PerObjectCB*)mappedResource.pData;

		//update the data
		XMStoreFloat4x4(&constantBufferData->WVP, WVP);

		// enable gpu access
		m_d3dContext->Unmap(m_vsConstantBuffer.Get(), 0);
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

	// bind the constant data to the vertex shader
	UINT bufferRegister = 0; // PerObjectCB was defined as register 0 inside the vertex shader file
	m_d3dContext->VSSetConstantBuffers(bufferRegister, 1, m_vsConstantBuffer.GetAddressOf());

	// set what to draw
	UINT stride = sizeof(mesh::MeshData::Vertex);
	UINT offset = 0;
	//m_d3dContext->IASetVertexBuffers(0, 1, m_plane.m_vertexBuffer.GetAddressOf(), &stride, &offset);
	//m_d3dContext->IASetIndexBuffer(m_plane.m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	//m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// draw and present the frame
	//m_d3dContext->Draw(m_plane.mesh.vertices.size(), 0);

	m_d3dContext->IASetVertexBuffers(0, 1, m_sphere.m_vertexBuffer.GetAddressOf(), &stride, &offset);
	m_d3dContext->IASetIndexBuffer(m_sphere.m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//draw and present the frame
	m_d3dContext->Draw(m_sphere.mesh.vertices.size(), 0);
	m_d3dContext->DrawIndexed(m_sphere.mesh.indices.size(), 0, 0);

	//m_d3dContext->IASetVertexBuffers(0, 1, m_box.m_vertexBuffer.GetAddressOf(), &stride, &offset);
	//m_d3dContext->IASetIndexBuffer(m_box.m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	//m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// draw and present the frame
	//m_d3dContext->Draw(m_box.mesh.vertices.size(), 0);

	XTEST_D3D_CHECK(m_swapChain->Present(0, 0));

	m_d3dAnnotation->EndEvent();
}