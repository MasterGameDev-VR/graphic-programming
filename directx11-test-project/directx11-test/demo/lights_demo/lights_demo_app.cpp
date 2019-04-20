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
	, m_viewMatrix()
	, m_projectionMatrix()
	, m_camera(math::ToRadians(68.f), math::ToRadians(135.f), 7.f, { 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, { math::ToRadians(4.f), math::ToRadians(175.f) }, { 3.f, 25.f })
	, m_dirLight()
	, m_spotLight()
	, m_pointLight()
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


LightsDemoApp::~LightsDemoApp()
{}


void LightsDemoApp::Init()
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
	// 	{
	// 		float3 posL : POSITION;
	// 		float3 normalL : NORMAL;
	// 	};
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(mesh::MeshData::Vertex, normal), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	XTEST_D3D_CHECK(m_d3dDevice->CreateInputLayout(vertexDesc, 2, vsByteCode.Data(), vsByteCode.ByteSize(), &m_inputLayout));


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


void xtest::demo::LightsDemoApp::InitRenderable()
{

	// plane
	{
		// geo
		m_plane.mesh = mesh::GeneratePlane(50.f, 50.f, 50, 50);


		// W
		XMStoreFloat4x4(&m_plane.W, XMMatrixIdentity());


		// material
		m_plane.material.ambient  = { 0.15f, 0.15f, 0.15f, 1.f };
		m_plane.material.diffuse  = { 0.52f, 0.52f, 0.52f, 1.f };
		m_plane.material.specular = { 0.8f, 0.8f, 0.8f, 190.0f };


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


	// sphere
	{

		//geo
		m_sphere.mesh = mesh::GenerateSphere(1.f, 40, 40); // mesh::GenerateBox(2, 2, 2);


		// W
		XMStoreFloat4x4(&m_sphere.W,XMMatrixTranslation(-4.f, 1.f, 0.f));

		// material
		m_sphere.material.ambient = { 0.7f, 0.1f, 0.1f, 1.0f };
		m_sphere.material.diffuse = { 0.81f, 0.15f, 0.15f, 1.0f };
		m_sphere.material.specular = { 0.7f, 0.7f, 0.7f, 40.0f };


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
		XMStoreFloat4x4(&m_crate.W, XMMatrixMultiply(XMMatrixScaling(0.01f, 0.01f, 0.01f), XMMatrixTranslation(0.f, 0.f, 0.f)));


		//bottom material
		Material& bottomMat = m_crate.shapeAttributeMapByName["bottom_1"].material;
		bottomMat.ambient = { 0.8f, 0.3f, 0.1f, 1.0f };
		bottomMat.diffuse = { 0.94f, 0.40f, 0.14f, 1.0f };
		bottomMat.specular = { 0.94f, 0.40f, 0.14f, 30.0f };

		//top material
		Material& topMat = m_crate.shapeAttributeMapByName["top_2"].material;
		topMat.ambient = { 0.8f, 0.8f, 0.8f, 1.0f };
		topMat.diffuse = { 0.9f, 0.9f, 0.9f, 1.0f };
		topMat.specular = { 0.9f, 0.9f, 0.9f, 550.0f };

		//top handles material
		Material& topHandleMat = m_crate.shapeAttributeMapByName["top_handles_4"].material;
		topHandleMat.ambient = { 0.3f, 0.3f, 0.3f, 1.0f };
		topHandleMat.diffuse = { 0.4f, 0.4f, 0.4f, 1.0f };
		topHandleMat.specular = { 0.9f, 0.9f, 0.9f, 120.0f };

		//handle material
		Material& handleMat = m_crate.shapeAttributeMapByName["handles_8"].material;
		handleMat.ambient = { 0.5f, 0.5f, 0.1f, 1.0f };
		handleMat.diffuse = { 0.67f, 0.61f, 0.1f, 1.0f };
		handleMat.specular = { 0.67f, 0.61f, 0.1f, 200.0f };

		//metal material
		Material& metalPiecesMat = m_crate.shapeAttributeMapByName["metal_pieces_3"].material;
		metalPiecesMat.ambient = { 0.3f, 0.3f, 0.3f, 1.0f };
		metalPiecesMat.diffuse = { 0.4f, 0.4f, 0.4f, 1.0f };
		metalPiecesMat.specular = { 0.4f, 0.4f, 0.4f, 520.0f };



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


void LightsDemoApp::InitLights()
{
	m_dirLight.ambient  = { 0.16f, 0.18f, 0.18f, 1.f };
	m_dirLight.diffuse  = {0.4f* 0.87f,0.4f* 0.90f,0.4f* 0.94f, 1.f };
	m_dirLight.specular = { 0.87f, 0.90f, 0.94f, 1.f };
	XMVECTOR dirLightDirection = XMVector3Normalize(-XMVectorSet(5.f, 3.f, 5.f, 0.f));
	XMStoreFloat3(&m_dirLight.dirW, dirLightDirection);


	m_pointLight.ambient = { 0.18f, 0.04f, 0.16f, 1.0f };
	m_pointLight.diffuse = { 0.94f, 0.23f, 0.87f, 1.0f };
	m_pointLight.specular = { 0.94f, 0.23f, 0.87f, 1.0f };
	m_pointLight.posW = { -5.f, 2.f, 5.f };
	m_pointLight.range = 15.f;
	m_pointLight.attenuation = { 0.0f, 0.2f, 0.f };


	m_spotLight.ambient  = { 0.018f, 0.018f, 0.18f, 1.0f };
	m_spotLight.diffuse  = { 0.1f, 0.1f, 0.9f, 1.0f };
	m_spotLight.specular = { 0.1f, 0.1f, 0.9f, 1.0f };
	XMVECTOR posW = XMVectorSet(5.f, 5.f, -5.f, 1.f);
	XMStoreFloat3(&m_spotLight.posW, posW);
	m_spotLight.range = 50.f;
	XMVECTOR dirW = XMVector3Normalize( XMVectorSet(-4.f, 1.f, 0.f, 1.f) -  posW);
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

}


void LightsDemoApp::OnKeyStatusChange(input::Key key, const input::KeyStatus& status)
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


void LightsDemoApp::UpdateScene(float deltaSeconds)
{
	XTEST_UNUSED_VAR(deltaSeconds);

	
	// create the model-view-projection matrix
	XMMATRIX V = m_camera.GetViewMatrix();
	XMStoreFloat4x4(&m_viewMatrix, V);

	// create projection matrix
	XMMATRIX P = XMLoadFloat4x4(&m_projectionMatrix);
	
	

	m_d3dAnnotation->BeginEvent(L"update-constant-buffer");


	// plane PerObjectCB
	{
		XMMATRIX W = XMLoadFloat4x4(&m_plane.W);
		XMMATRIX WVP = W*V*P;

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


	// sphere PerObjectCB
	{
		XMMATRIX W = XMLoadFloat4x4(&m_sphere.W);
		XMMATRIX WVP = W*V*P;

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


	// crate PerObjectCB
	{
		XMMATRIX W = XMLoadFloat4x4(&m_crate.W);
		XMMATRIX WVP = W*V*P;

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
			XMStoreFloat3(&m_pointLight.posW, XMVector3Transform(XMLoadFloat3(&m_pointLight.posW), R));

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
		perFrameCB->pointLight = m_pointLight;
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


void LightsDemoApp::RenderScene()
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

	m_d3dContext->PSSetConstantBuffers(1, 1, m_d3dPerFrameCB.GetAddressOf());
	m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// draw plane
	{
		// bind the constant data to the vertex shader
		m_d3dContext->VSSetConstantBuffers(0, 1, m_plane.d3dPerObjectCB.GetAddressOf());
		m_d3dContext->PSSetConstantBuffers(0, 1, m_plane.d3dPerObjectCB.GetAddressOf());

		// set what to draw
		UINT stride = sizeof(mesh::MeshData::Vertex);
		UINT offset = 0;
		m_d3dContext->IASetVertexBuffers(0, 1, m_plane.d3dVertexBuffer.GetAddressOf(), &stride, &offset);
		m_d3dContext->IASetIndexBuffer(m_plane.d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		m_d3dContext->DrawIndexed(UINT(m_plane.mesh.indices.size()), 0, 0);
	}

	
	// draw sphere
	{
		// bind the constant data to the vertex shader
		m_d3dContext->VSSetConstantBuffers(0, 1, m_sphere.d3dPerObjectCB.GetAddressOf());
		m_d3dContext->PSSetConstantBuffers(0, 1, m_sphere.d3dPerObjectCB.GetAddressOf());

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

