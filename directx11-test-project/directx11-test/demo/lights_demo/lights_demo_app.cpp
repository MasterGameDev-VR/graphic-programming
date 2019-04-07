#include "stdafx.h"
#include "lights_demo_app.h"
#include <file/file_utils.h>
#include <math/math_utils.h>
#include <service/locator.h>

using namespace DirectX;
using namespace xtest;

using xtest::demo::LightsDemoApp;
using Microsoft::WRL::ComPtr;


LightsDemoApp::LightsDemoApp(HINSTANCE instance,
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
	, m_camera(math::ToRadians(45.f), math::ToRadians(60.f), 250.f, { 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, { math::ToRadians(5.f), math::ToRadians(175.f) })
{
	eyePosW = m_camera.GetPosition();
}


LightsDemoApp::~LightsDemoApp()
{}
void LightsDemoApp::Init()
{
	application::DirectxApp::Init();

	m_d3dAnnotation->BeginEvent(L"init-demo");

	InitMatrices();
	InitLights();
	InitMaterials();
	InitShaders();
	InitBuffers();
	InitRasterizerState();

	service::Locator::GetMouse()->AddListener(this);
	service::Locator::GetKeyboard()->AddListener(this, { input::Key::F });

	m_d3dAnnotation->EndEvent();
}

void LightsDemoApp::InitLights()
{
	color_dirLight_ambient = { 0.2f, 0.2f, 0.2f,1.0f };
	color_dirLight_diffuse = { 1.0f,1.0f,1.0f,1.0f };	 
	color_dirLight_specular = { 1.0f,1.0f,1.0f,0.05f };
	color_pointLight_ambient = { 0.0f,  0.0f,  0.0f,1.0f };
	color_pointLigh_diffuse = { 1.0f, 1.0f, 1.0f,1.0f };
	color_pointLight_specular = { 1.0f, 1.0f, 1.0f,4.0f };
	color_spotLight_ambient = { 0.0f,  0.0f,  0.0f,1.0f };
	color_spotLight_diffuse = { 1.0f, 1.0f, 1.0f,1.0f };
	color_spotLight_specular = { 1.0f, 1.0f, 1.0f,4.0f };

	dirSpotLight = DirectX::XMFLOAT3{ -2.0f,-2.0f,7.0f };
	float dirLength = sqrtf((dirSpotLight.x)*(dirSpotLight.x) + (dirSpotLight.y)*(dirSpotLight.y) + (dirSpotLight.z)*(dirSpotLight.z));
	dirSpotLight = DirectX::XMFLOAT3{ dirSpotLight.x / dirLength,dirSpotLight.y / dirLength,dirSpotLight.z / dirLength };

	myDirectionalLight = { color_dirLight_ambient ,color_dirLight_diffuse ,color_dirLight_specular,DirectX::XMFLOAT3{-1 / sqrtf(2.0f),0.0f,1 / sqrtf(2.0f)},0.0f };
	// colors ---- direction -------- padding float
	myPointLight = { color_pointLight_ambient ,color_pointLigh_diffuse ,color_pointLight_specular,DirectX::XMFLOAT3{35.0f,35.0f,35.0f},200.0f, DirectX::XMFLOAT3{0.0f,1.0f,0.0f},0.0f };
	// colors --------position ------- range ------- attenuation ---- padding float

	mySpotLight = { color_spotLight_ambient, color_spotLight_diffuse, color_spotLight_specular , DirectX::XMFLOAT3{30.0f,40.0f,+30.0f}, 100.0f,DirectX::XMFLOAT3{-1 / sqrtf(2.0f),0.0f,-1 / sqrtf(2.0f)} , 0.6f, DirectX::XMFLOAT3{1.0f,0.0f,0.0f},0.0f };
	// colors --------position ------- range -------direction ----------- spot dimension (cosine of the angle)---------  attenuation ---- padding float


	eyePosW = DirectX::XMFLOAT3{ 0.f, 0.f, 0.f };   //possibile errore che causa il malfunzionamento di PointLightContrib e SpotLightContrib

}
void LightsDemoApp::InitMaterials()
{
	mat_plane_rgba = { 1.0f,0.0f,0.0f,1.0f };
	mat_sphere_rgba = { 0.0f,1.0f,0.0f,1.0f };
	mat_box_rgba = { 0.0f,0.0f,1.0f,1.0f };
	mat_plane = { mat_plane_rgba,  mat_plane_rgba , mat_plane_rgba };
	mat_sphere = { mat_sphere_rgba,   mat_sphere_rgba ,  mat_sphere_rgba };  //orange-red
	mat_box = { mat_box_rgba,  mat_box_rgba , mat_box_rgba }; //tomato

}

void LightsDemoApp::InitMatrices()
{
	//XMMATRIX translateSphere = XMMatrixIdentity();
	//XMMATRIX translatePlane = XMMatrixIdentity();
	

	// world matrices
	XMStoreFloat4x4(&m_worldMatrixPlane, XMMatrixIdentity());
	XMStoreFloat4x4(&m_worldMatrixBox, XMMatrixIdentity());
	XMStoreFloat4x4(&m_worldMatrixSphere, XMMatrixIdentity());
	
	// view matrix
	XMStoreFloat4x4(&m_viewMatrix, m_camera.GetViewMatrix());

	// projection matrix
	{
		XMMATRIX P = XMMatrixPerspectiveFovLH(math::ToRadians(45.f), AspectRatio(), 1.f, 1000.f);
		XMStoreFloat4x4(&m_projectionMatrix, P);
	}
}

void LightsDemoApp::OnResized()
{
	application::DirectxApp::OnResized();

	//update the projection matrix with the new aspect ratio
	XMMATRIX P = XMMatrixPerspectiveFovLH(math::ToRadians(45.f), AspectRatio(), 1.f, 1000.f);
	XMStoreFloat4x4(&m_projectionMatrix, P);
}

void LightsDemoApp::InitShaders() {
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
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(MeshData::Vertex,normal), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(MeshData::Vertex,tangentU), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(MeshData::Vertex,uv), D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	XTEST_D3D_CHECK(m_d3dDevice->CreateInputLayout(vertexDesc, 4, vsByteCode.Data(), vsByteCode.ByteSize(), &m_inputLayout));
}
void LightsDemoApp::InitBuffers() {
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	//VERTICES SHOULD COME FROM THE MESH GENERATOR METHODS
	MeshData planeMesh = GeneratePlane(500.0f, 500.0f, 1500, 1500);
	MeshData sphereMesh = GenerateSphere(25.0f, 360, 360);
	MeshData boxMesh = GenerateBox(40.0f, 30.0f, 70.0f);

	//------------------- Plane
	vertexBufferDesc.ByteWidth = UINT(sizeof(MeshData::Vertex)*planeMesh.vertices.size());
	indexBufferDesc.ByteWidth = UINT(sizeof(uint32)*planeMesh.indices.size());

	D3D11_SUBRESOURCE_DATA planeVerticesInitData;
	planeVerticesInitData.pSysMem = &planeMesh.vertices[0];
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &planeVerticesInitData, &m_vertexBufferPlane));
	D3D11_SUBRESOURCE_DATA planeIndicesInitdata;
	planeIndicesInitdata.pSysMem = &planeMesh.indices[0];
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &planeIndicesInitdata, &m_indexBufferPlane));

	// ----------------- Sphere
	vertexBufferDesc.ByteWidth = UINT(sizeof(MeshData::Vertex)*sphereMesh.vertices.size());
	indexBufferDesc.ByteWidth = UINT(sizeof(uint32)*sphereMesh.indices.size());

	D3D11_SUBRESOURCE_DATA sphereVerticesInitData;
	sphereVerticesInitData.pSysMem = &sphereMesh.vertices[0];
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &sphereVerticesInitData, &m_vertexBufferSphere));
	D3D11_SUBRESOURCE_DATA sphereIndicesInitdata;
	sphereIndicesInitdata.pSysMem = &sphereMesh.indices[0];
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &sphereIndicesInitdata, &m_indexBufferSphere));

	//------------------- Box
	vertexBufferDesc.ByteWidth = UINT(sizeof(MeshData::Vertex)*boxMesh.vertices.size());
	indexBufferDesc.ByteWidth = UINT(sizeof(uint32)*boxMesh.indices.size());

	D3D11_SUBRESOURCE_DATA boxVerticesInitData;
	boxVerticesInitData.pSysMem = &boxMesh.vertices[0];
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &boxVerticesInitData, &m_vertexBufferBox));
	D3D11_SUBRESOURCE_DATA boxIndicesInitdata;
	boxIndicesInitdata.pSysMem = &boxMesh.indices[0];
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &boxIndicesInitdata, &m_indexBufferBox));

	//CONSTANT BUFFERS --------------
	D3D11_BUFFER_DESC vsConstantBufferDesc;
	vsConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC; // this buffer needs to be updated every frame
	vsConstantBufferDesc.ByteWidth = sizeof(PerObjectCB);
	vsConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	vsConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vsConstantBufferDesc.MiscFlags = 0;
	vsConstantBufferDesc.StructureByteStride = 0;
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vsConstantBufferDesc, nullptr, &m_vsConstantBufferPlane));
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vsConstantBufferDesc, nullptr, &m_vsConstantBufferSphere));
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vsConstantBufferDesc, nullptr, &m_vsConstantBufferBox));


	D3D11_BUFFER_DESC psConstantBufferDesc;
	psConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC; // this buffer needs to be updated every frame
	psConstantBufferDesc.ByteWidth = sizeof(PerFrameCB);
	psConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	psConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	psConstantBufferDesc.MiscFlags = 0;
	psConstantBufferDesc.StructureByteStride = 0;
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&psConstantBufferDesc, nullptr, &m_psConstantBufferLights));
}

void LightsDemoApp::InitRasterizerState()
{
	// rasterizer state
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthClipEnable = true;

	m_d3dDevice->CreateRasterizerState(&rasterizerDesc, &m_rasterizerState);
}

void LightsDemoApp::UpdateScene(float deltaSeconds)
{
	XTEST_UNUSED_VAR(deltaSeconds);

	XMMATRIX rotatePlane_X = XMMatrixRotationAxis(XMVECTOR{ 1.0f,0.0f,0.0f }, -80.0f);
	XMMATRIX rotateBox_X = XMMatrixRotationAxis(XMVECTOR{ 1.0f,0.0f,0.0f }, -20.0f);
	XMMATRIX rotateBox_Z = XMMatrixRotationAxis(XMVECTOR{ 0.0f,0.0f,1.0f }, 20.0f);

	XMMATRIX W_plane = XMLoadFloat4x4(&m_worldMatrixPlane);
	W_plane.r[3] = { 2.0f,-24.0f,0.0f,1.0f };
	//W_plane.r[3].m128_f32[0] = -1.0f;
	XMStoreFloat4x4(&m_worldMatrixPlane, W_plane);

	XMMATRIX W_sphere = XMLoadFloat4x4(&m_worldMatrixSphere);
	W_sphere.r[3] = { 20.0f,0.0f,80.0f,1.0f };
	XMStoreFloat4x4(&m_worldMatrixSphere, W_sphere);

	XMMATRIX W_box = XMLoadFloat4x4(&m_worldMatrixBox);
	W_box.r[3] = { 30.0f,0.0f,20.0f,1.0f };
	XMStoreFloat4x4(&m_worldMatrixBox, W_box);

	// create the model-view-projection matrix
	XMMATRIX V = m_camera.GetViewMatrix();
	XMStoreFloat4x4(&m_viewMatrix, V);

	// create projection matrix
	XMMATRIX P = XMLoadFloat4x4(&m_projectionMatrix);
	XMMATRIX WVP_plane = W_plane * V*P;
	XMMATRIX WVP_sphere = W_sphere * V*P;
	XMMATRIX WVP_box = W_box * V*P;

	XMVECTOR det_W_plane = XMMatrixDeterminant(W_plane);
	XMVECTOR det_W_sphere = XMMatrixDeterminant(W_sphere);
	XMVECTOR det_W_box = XMMatrixDeterminant(W_box);
	XMMATRIX W_Inv_Transp_plane = XMMatrixInverse(&det_W_plane,W_plane);
	XMMATRIX W_Inv_Transp_sphere = XMMatrixInverse(&det_W_sphere, W_sphere);
	XMMATRIX W_Inv_Transp_box = XMMatrixInverse(&det_W_box, W_box);

	// matrices must be transposed since HLSL use column-major ordering.
	WVP_plane = XMMatrixTranspose(WVP_plane);
	WVP_sphere = XMMatrixTranspose(WVP_sphere);
	WVP_box = XMMatrixTranspose(WVP_box);



	m_d3dAnnotation->BeginEvent(L"update-constant-buffer");

	// load the constant buffer data in the gpu
	{
		D3D11_MAPPED_SUBRESOURCE mappedResourcePlane;
		ZeroMemory(&mappedResourcePlane, sizeof(D3D11_MAPPED_SUBRESOURCE));

		// disable gpu access
		XTEST_D3D_CHECK(m_d3dContext->Map(m_vsConstantBufferPlane.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResourcePlane));
		PerObjectCB* constantBufferDataPlane = (PerObjectCB*)mappedResourcePlane.pData;
	
		//update the data
		XMStoreFloat4x4(&constantBufferDataPlane->W, W_plane);
		XMStoreFloat4x4(&constantBufferDataPlane->WVP, WVP_plane);
		XMStoreFloat4x4(&constantBufferDataPlane->W_Inv_Transp, W_Inv_Transp_plane);
		XMStoreFloat4(&constantBufferDataPlane->material.ambient, XMLoadFloat4(&mat_plane.ambient));
		XMStoreFloat4(&constantBufferDataPlane->material.diffuse, XMLoadFloat4(&mat_plane.diffuse));
		XMStoreFloat4(&constantBufferDataPlane->material.specular, XMLoadFloat4(&mat_plane.specular));


		

		// enable gpu access
		m_d3dContext->Unmap(m_vsConstantBufferPlane.Get(), 0);
	}
	m_d3dAnnotation->EndEvent();

	m_d3dAnnotation->BeginEvent(L"update-constant-buffer");

	// load the constant buffer data in the gpu
	{
		D3D11_MAPPED_SUBRESOURCE mappedResourceSphere;
		ZeroMemory(&mappedResourceSphere, sizeof(D3D11_MAPPED_SUBRESOURCE));

		// disable gpu access
		XTEST_D3D_CHECK(m_d3dContext->Map(m_vsConstantBufferSphere.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResourceSphere));
		PerObjectCB* constantBufferDataSphere = (PerObjectCB*)mappedResourceSphere.pData;

		//update the data
		XMStoreFloat4x4(&constantBufferDataSphere->W, W_sphere);
		XMStoreFloat4x4(&constantBufferDataSphere->WVP, WVP_sphere);
		XMStoreFloat4x4(&constantBufferDataSphere->W_Inv_Transp, W_Inv_Transp_sphere);
		XMStoreFloat4(&constantBufferDataSphere->material.ambient, XMLoadFloat4(&mat_sphere.ambient));
		XMStoreFloat4(&constantBufferDataSphere->material.diffuse, XMLoadFloat4(&mat_sphere.diffuse));
		XMStoreFloat4(&constantBufferDataSphere->material.specular, XMLoadFloat4(&mat_sphere.specular));




		// enable gpu access
		m_d3dContext->Unmap(m_vsConstantBufferSphere.Get(), 0);
	}
	m_d3dAnnotation->EndEvent();
	m_d3dAnnotation->BeginEvent(L"update-constant-buffer");

	// load the constant buffer data in the gpu  ------------- BOX
	{
		D3D11_MAPPED_SUBRESOURCE mappedResourceBox;
		ZeroMemory(&mappedResourceBox, sizeof(D3D11_MAPPED_SUBRESOURCE));

		// disable gpu access
		XTEST_D3D_CHECK(m_d3dContext->Map(m_vsConstantBufferBox.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResourceBox));
		PerObjectCB* constantBufferDataBox = (PerObjectCB*)mappedResourceBox.pData;

		//update the data
		XMStoreFloat4x4(&constantBufferDataBox->W, W_box);
		XMStoreFloat4x4(&constantBufferDataBox->WVP, WVP_box);
		XMStoreFloat4x4(&constantBufferDataBox->W_Inv_Transp, W_Inv_Transp_box);
		XMStoreFloat4(&constantBufferDataBox->material.ambient, XMLoadFloat4(&mat_box.ambient));
		XMStoreFloat4(&constantBufferDataBox->material.diffuse, XMLoadFloat4(&mat_box.diffuse));
		XMStoreFloat4(&constantBufferDataBox->material.specular, XMLoadFloat4(&mat_box.specular));



		// enable gpu access
		m_d3dContext->Unmap(m_vsConstantBufferBox.Get(), 0);
	}
	m_d3dAnnotation->EndEvent();
	//LIGHTS RESOURCES LOADING --------------------------- !!!!!

	m_d3dAnnotation->BeginEvent(L"update-constant-buffer");
	{
		D3D11_MAPPED_SUBRESOURCE mappedResourceLights;
		ZeroMemory(&mappedResourceLights, sizeof(D3D11_MAPPED_SUBRESOURCE));

		// disable gpu access
		XTEST_D3D_CHECK(m_d3dContext->Map(m_psConstantBufferLights.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResourceLights));
		PerFrameCB* constantBufferDataLights = (PerFrameCB*)mappedResourceLights.pData;

		//update the data
	
		XMStoreFloat4(&constantBufferDataLights->dirLight.ambient, XMLoadFloat4(&myDirectionalLight.ambient));
		XMStoreFloat4(&constantBufferDataLights->dirLight.diffuse, XMLoadFloat4(&myDirectionalLight.diffuse));
		XMStoreFloat4(&constantBufferDataLights->dirLight.specular, XMLoadFloat4(&myDirectionalLight.specular));
		XMStoreFloat3(&constantBufferDataLights->dirLight.dirW, XMLoadFloat3(&myDirectionalLight.dirW));
		XMStoreFloat(&constantBufferDataLights->dirLight._explicit_pad_, XMLoadFloat(&myDirectionalLight._explicit_pad_));

		XMStoreFloat4(&constantBufferDataLights->pointLight.ambient, XMLoadFloat4(&myPointLight.ambient));
		XMStoreFloat4(&constantBufferDataLights->pointLight.diffuse, XMLoadFloat4(&myPointLight.diffuse));
		XMStoreFloat4(&constantBufferDataLights->pointLight.specular, XMLoadFloat4(&myPointLight.specular));
		XMStoreFloat3(&constantBufferDataLights->pointLight.posW, XMLoadFloat3(&myPointLight.posW));
		XMStoreFloat(&constantBufferDataLights->pointLight.range, XMLoadFloat(&myPointLight.range));
		XMStoreFloat3(&constantBufferDataLights->pointLight.attenuation, XMLoadFloat3(&myPointLight.attenuation));
		XMStoreFloat(&constantBufferDataLights->pointLight._explicit_pad_, XMLoadFloat(&myPointLight._explicit_pad_));


		XMStoreFloat4(&constantBufferDataLights->spotLight.ambient, XMLoadFloat4(&mySpotLight.ambient));
		XMStoreFloat4(&constantBufferDataLights->spotLight.diffuse, XMLoadFloat4(&mySpotLight.diffuse));
		XMStoreFloat4(&constantBufferDataLights->spotLight.specular, XMLoadFloat4(&mySpotLight.specular));
		XMStoreFloat3(&constantBufferDataLights->spotLight.posW, XMLoadFloat3(&mySpotLight.posW));
		XMStoreFloat(&constantBufferDataLights->spotLight.range, XMLoadFloat(&mySpotLight.range));
		XMStoreFloat3(&constantBufferDataLights->spotLight.dirW, XMLoadFloat3(&mySpotLight.dirW));
		XMStoreFloat(&constantBufferDataLights->spotLight.spot, XMLoadFloat(&mySpotLight.spot));
		XMStoreFloat3(&constantBufferDataLights->spotLight.attenuation, XMLoadFloat3(&mySpotLight.attenuation));
		XMStoreFloat(&constantBufferDataLights->spotLight._explicit_pad_, XMLoadFloat(&mySpotLight._explicit_pad_));

		XMStoreFloat3(&constantBufferDataLights->eyePosW, XMLoadFloat3(&eyePosW));
		XMStoreFloat(&constantBufferDataLights->_explicit_pad_, XMLoadFloat(&_explicit_pad_));

		// enable gpu access
		m_d3dContext->Unmap(m_psConstantBufferLights.Get(), 0);
	}
	m_d3dAnnotation->EndEvent();
}

void LightsDemoApp::RenderScene()
{
	m_d3dAnnotation->BeginEvent(L"render-scene");

	// clear the frame
	m_d3dContext->ClearDepthStencilView(m_depthBufferView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
	m_d3dContext->ClearRenderTargetView(m_backBufferView.Get(), DirectX::Colors::Transparent);

	// set the shaders and the input layout
	m_d3dContext->RSSetState(m_rasterizerState.Get());
	m_d3dContext->IASetInputLayout(m_inputLayout.Get());
	m_d3dContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	m_d3dContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	// bind the constant data to the vertex shader
	// PerObjectCB was defined as register 0 inside the vertex shader file
	UINT bufferRegisterMatrices = 0; 
	UINT bufferRegisterLights = 1;
	
	m_d3dContext->PSSetConstantBuffers(bufferRegisterLights, 1, m_psConstantBufferLights.GetAddressOf());

	D3D11_BUFFER_DESC tempIndexBufferDesc;

	// set what to draw - plane
	UINT stride = sizeof(MeshData::Vertex);
	UINT offset = 0;

	
	m_d3dContext->VSSetConstantBuffers(bufferRegisterMatrices, 1, m_vsConstantBufferPlane.GetAddressOf());
	m_d3dContext->PSSetConstantBuffers(bufferRegisterMatrices, 1, m_vsConstantBufferPlane.GetAddressOf());

	m_d3dContext->IASetVertexBuffers(0, 1, m_vertexBufferPlane.GetAddressOf(), &stride, &offset);
	m_d3dContext->IASetIndexBuffer(m_indexBufferPlane.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_indexBufferPlane->GetDesc(&tempIndexBufferDesc);
	// draw and present the frame
	m_d3dContext->DrawIndexed(tempIndexBufferDesc.ByteWidth / sizeof(uint32), 0, 0);
	
	// set what to draw - sphere
	
	
	m_d3dContext->VSSetConstantBuffers(bufferRegisterMatrices, 1, m_vsConstantBufferSphere.GetAddressOf());
	m_d3dContext->PSSetConstantBuffers(bufferRegisterMatrices, 1, m_vsConstantBufferSphere.GetAddressOf());

	m_d3dContext->IASetVertexBuffers(0, 1, m_vertexBufferSphere.GetAddressOf(), &stride, &offset);
	m_d3dContext->IASetIndexBuffer(m_indexBufferSphere.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_indexBufferSphere->GetDesc(&tempIndexBufferDesc);
	// draw and present the frame
	m_d3dContext->DrawIndexed(tempIndexBufferDesc.ByteWidth / sizeof(uint32), 0, 0);
	

	// set what to draw - box
	
	m_d3dContext->VSSetConstantBuffers(bufferRegisterMatrices, 1, m_vsConstantBufferBox.GetAddressOf());
	m_d3dContext->PSSetConstantBuffers(bufferRegisterMatrices, 1, m_vsConstantBufferBox.GetAddressOf());

	m_d3dContext->IASetVertexBuffers(0, 1, m_vertexBufferBox.GetAddressOf(), &stride, &offset);
	m_d3dContext->IASetIndexBuffer(m_indexBufferBox.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_indexBufferBox->GetDesc(&tempIndexBufferDesc);
	// draw and present the frame
	m_d3dContext->DrawIndexed(tempIndexBufferDesc.ByteWidth / sizeof(uint32), 0, 0);
	

	XTEST_D3D_CHECK(m_swapChain->Present(0, 0));

	m_d3dAnnotation->EndEvent();
}

void LightsDemoApp::OnWheelScroll(input::ScrollStatus scroll)
{
	// zoom in or out when the scroll wheel is used
	if (service::Locator::GetMouse()->IsInClientArea())
	{
		m_camera.IncreaseRadiusBy(scroll.isScrollingUp ? -0.5f : 0.5f);
	}
	eyePosW = m_camera.GetPosition();
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
	eyePosW = m_camera.GetPosition();

}

void LightsDemoApp::OnKeyStatusChange(input::Key key, const input::KeyStatus& status)
{
	XTEST_ASSERT(key == input::Key::F); // the only key registered for this listener

	// re-frame the cube when F key is pressed
	if (status.isDown)
	{
		m_camera.SetPivot({ 0.f, 0.f, 0.f });
	}
	eyePosW = m_camera.GetPosition();
}


