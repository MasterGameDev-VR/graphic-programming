#include "stdafx.h"
#include "threeDC.h"
#include <file/file_utils.h>
#include <service/locator.h>
#include <math/math_utils.h>

using namespace DirectX;


threeDC::threeDC(HINSTANCE instance,
	const application::WindowSettings& windowSettings,
	const application::DirectxSettings& directxSettings,
	uint32 fps /*=60*/)
	: application::DirectxApp(instance, windowSettings, directxSettings, fps)
	, m_vertexBufferPlane(nullptr)
	, m_indexBufferPlane(nullptr)
	, m_vertexBufferBox(nullptr)
	, m_indexBufferBox(nullptr)
	, m_vertexBufferSphere(nullptr)
	, m_indexBufferSphere(nullptr)
	, m_vertexShader(nullptr)
	, m_pixelShader(nullptr)
	, m_inputLayout(nullptr)
	, m_camera(math::ToRadians(90.f), math::ToRadians(30.f), 10.f, { 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, { math::ToRadians(5.f), math::ToRadians(175.f) }, { 3.f, 25.f })
{}


threeDC::~threeDC()
{
}

void threeDC::Init()
{
	application::DirectxApp::Init();

	m_d3dAnnotation->BeginEvent(L"init-demo");

	InitMatrices();
	InitShaders();
	InitBuffers();
	InitRasterizerState();

	//service::Locator::GetMouse()->AddListener(this);
	//service::Locator::GetKeyboard()->AddListener(this, { input::Key::F });

	m_d3dAnnotation->EndEvent();
}


void threeDC::InitMatrices()
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


void threeDC::InitShaders() {
	// read pre-compiled shaders' bytecode
	std::future<file::BinaryFile> psByteCodeFuture = file::ReadBinaryFile(std::wstring(GetRootDir()).append(L"\\threeDC_PS.cso"));
	std::future<file::BinaryFile> vsByteCodeFuture = file::ReadBinaryFile(std::wstring(GetRootDir()).append(L"\\threeDC_VS.cso"));

	// future.get() can be called only once
	file::BinaryFile vsByteCode = vsByteCodeFuture.get();
	file::BinaryFile psByteCode = psByteCodeFuture.get();
	XTEST_D3D_CHECK(m_d3dDevice->CreateVertexShader(vsByteCode.Data(), vsByteCode.ByteSize(), nullptr, &m_vertexShader));
	XTEST_D3D_CHECK(m_d3dDevice->CreatePixelShader(psByteCode.Data(), psByteCode.ByteSize(), nullptr, &m_pixelShader));
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(VertexIn,color ), D3D11_INPUT_PER_VERTEX_DATA, 0 }

	};

	XTEST_D3D_CHECK(m_d3dDevice->CreateInputLayout(vertexDesc, 2, vsByteCode.Data(), vsByteCode.ByteSize(), &m_inputLayout));
}
/*

void threeDC::BuildVerticesArray(MeshData& mesh, DirectX::XMFLOAT4 color, VertexIn* planeMeshVertices)
{
	//VertexIn planeMeshVertices[] = { { XMFLOAT3(+1.f, +1.f, +1.f), color } };
	for (int i = 0; i < mesh.vertices.size(); i++) {
		planeMeshVertices[i] = { mesh.vertices[i].position,  color};
	}

	//XMFLOAT4(DirectX::Colors::Red)
}
void threeDC::BuildIndicesArray(MeshData& mesh, uint32* planeMeshIndices)
{
	//uint32 planeMeshIndices[] = {0};

	for (int i = 0; i < mesh.indices.size(); i++) {
		planeMeshIndices[i] = mesh.indices[i];
	}
}
*/
void threeDC::InitRasterizerState()
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

void threeDC::InitBuffers() {
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	//vertexBufferDesc.ByteWidth = sizeof(planeMesh.vertices);
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	//indexBufferDesc.ByteWidth = sizeof(indicies);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	//VERTICES SHOULD COME FROM THE MESH GENERATOR METHODS
	MeshData planeMesh = GeneratePlane( 100.0f, 100.0f, 500, 500 );

	/*
	threeDC::VertexIn planeVertices[] = { { XMFLOAT3(+1.f, +1.f, +1.f), XMFLOAT4(DirectX::Colors::BlueViolet) } };
	BuildVerticesArray(planeMesh, XMFLOAT4(DirectX::Colors::BlueViolet), planeVertices);
	uint32 planeIndices[] = { 0 }; BuildIndicesArray(planeMesh, planeIndices);
	*/
	//------------
	MeshData sphereMesh = GenerateSphere(21.0f, 60, 100);
	/*
	threeDC::VertexIn sphereVertices[] = { { XMFLOAT3(+1.f, +1.f, +1.f), XMFLOAT4(DirectX::Colors::Red) } };
	BuildVerticesArray(sphereMesh, XMFLOAT4(DirectX::Colors::Red), sphereVertices);
	uint32 sphereIndices[] = { 0 }; BuildIndicesArray(sphereMesh, sphereIndices);
	*/
	//--------------
	MeshData boxMesh = GenerateBox(20.0f, 12.0f, 31.0f);
	/*
	threeDC::VertexIn boxVertices[] = { { XMFLOAT3(+1.f, +1.f, +1.f), XMFLOAT4(DirectX::Colors::Green) } };
	BuildVerticesArray(boxMesh, XMFLOAT4(DirectX::Colors::Green), boxVertices);
	uint32 boxIndices[] = {0}; BuildIndicesArray(boxMesh, boxIndices);
	*/

	//------------------- Plane
	D3D11_SUBRESOURCE_DATA planeVerticesInitData;
	planeVerticesInitData.pSysMem = &planeMesh.vertices[0];	
	vertexBufferDesc.ByteWidth = UINT(sizeof(MeshData::Vertex)*planeMesh.vertices.size());
	indexBufferDesc.ByteWidth = UINT(sizeof(uint32)*planeMesh.indices.size());
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &planeVerticesInitData, &m_vertexBufferPlane));

	D3D11_SUBRESOURCE_DATA planeIndicesInitdata;
	planeIndicesInitdata.pSysMem = &planeMesh.indices[0];
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &planeIndicesInitdata, &m_indexBufferPlane));

	// ----------------- Sphere

	D3D11_SUBRESOURCE_DATA sphereVerticesInitData;
	sphereVerticesInitData.pSysMem = &sphereMesh.vertices[0];
	vertexBufferDesc.ByteWidth = UINT(sizeof(MeshData::Vertex)*sphereMesh.vertices.size());
	indexBufferDesc.ByteWidth = UINT(sizeof(uint32)*sphereMesh.indices.size());
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &planeVerticesInitData, &m_vertexBufferPlane));

	D3D11_SUBRESOURCE_DATA sphereIndicesInitdata;
	sphereIndicesInitdata.pSysMem = &sphereMesh.indices[0];
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &sphereIndicesInitdata, &m_indexBufferSphere));

	//------------------- Box
	D3D11_SUBRESOURCE_DATA boxVerticesInitData;
	boxVerticesInitData.pSysMem = &boxMesh.vertices[0];
	vertexBufferDesc.ByteWidth = UINT(sizeof(MeshData::Vertex)*boxMesh.vertices.size());
	indexBufferDesc.ByteWidth = UINT(sizeof(uint32)*boxMesh.indices.size());
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &boxVerticesInitData, &m_vertexBufferBox));

	D3D11_SUBRESOURCE_DATA boxIndicesInitdata;
	boxIndicesInitdata.pSysMem = &boxMesh.indices[0];
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &boxIndicesInitdata, &m_indexBufferBox));
	//-------------------------------------

	
	
}
void threeDC::RenderScene() 
{
	

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

	m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	D3D11_BUFFER_DESC tempIndexBufferDesc;
	UINT stride = sizeof(VertexIn);
	UINT offset = 0;
	m_d3dContext->IASetVertexBuffers(0, 1, m_vertexBufferPlane.GetAddressOf(), &stride, &offset);
	m_d3dContext->IASetIndexBuffer(m_indexBufferPlane.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_indexBufferPlane->GetDesc(&tempIndexBufferDesc);
	m_d3dContext->DrawIndexed(tempIndexBufferDesc.ByteWidth/sizeof(uint32),0,0);
	
	
	
	m_d3dContext->IASetVertexBuffers(0, 1, m_vertexBufferSphere.GetAddressOf(), &stride, &offset);
	m_d3dContext->IASetIndexBuffer(m_indexBufferSphere.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_indexBufferPlane->GetDesc(&tempIndexBufferDesc);
	m_d3dContext->DrawIndexed(tempIndexBufferDesc.ByteWidth / sizeof(uint32), 0, 0);

	m_d3dContext->IASetVertexBuffers(0, 1, m_vertexBufferBox.GetAddressOf(), &stride, &offset);
	m_d3dContext->IASetIndexBuffer(m_indexBufferBox.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_indexBufferPlane->GetDesc(&tempIndexBufferDesc);
	m_d3dContext->DrawIndexed(tempIndexBufferDesc.ByteWidth / sizeof(uint32), 0, 0);


	XTEST_D3D_CHECK(m_swapChain->Present(0, 0));
}
void threeDC::UpdateScene(float deltaSeconds) 
{
	XTEST_UNUSED_VAR(deltaSeconds);

	XMMATRIX W = XMLoadFloat4x4(&m_worldMatrix);
	XMStoreFloat4x4(&m_worldMatrix, W);

	// create the model-view-projection matrix
	XMMATRIX V = m_camera.GetViewMatrix();
	XMStoreFloat4x4(&m_viewMatrix, V);

	// create projection matrix
	XMMATRIX P = XMLoadFloat4x4(&m_projectionMatrix);
	XMMATRIX WVP = W * V*P;

	// matrices must be transposed since HLSL use column-major ordering.
	WVP = XMMatrixTranspose(WVP);
}
