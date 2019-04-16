#include "stdafx.h"
#include "lights_demo_app.h"
#include <file/file_utils.h>
#include <math/math_utils.h>
#include <service/locator.h>
#include <mesh/mesh_generator.h>



using namespace DirectX;
using namespace xtest;

using xtest::demo::LightsDemoApp;
using Microsoft::WRL::ComPtr;

LightsDemoApp::LightsDemoApp(HINSTANCE instance,
	const application::WindowSettings& windowSettings,
	const application::DirectxSettings& directxSettings,
	uint32 fps /*=60*/)
	: application::DirectxApp(instance, windowSettings, directxSettings, fps)
	, m_vertexBuffers{}//(nullptr)
	, m_indexBuffers{}//(nullptr)
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
	service::Locator::GetKeyboard()->AddListener(this, { input::Key::A });
	service::Locator::GetKeyboard()->AddListener(this, { input::Key::S });
	service::Locator::GetKeyboard()->AddListener(this, { input::Key::D });
	m_d3dAnnotation->EndEvent();
}


void LightsDemoApp::InitMatrices()
{
	// world matrix
	DirectX::XMStoreFloat4x4(&m_worldMatrix, XMMatrixIdentity());

	// view matrix
	DirectX::XMStoreFloat4x4(&m_viewMatrix, m_camera.GetViewMatrix());

	// projection matrix
	{
		XMMATRIX P = XMMatrixPerspectiveFovLH(math::ToRadians(45.f), AspectRatio(), 1.f, 1000.f);
		DirectX::XMStoreFloat4x4(&m_projectionMatrix, P);
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
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexIn, normal), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexIn, tangent), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexIn, uv), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	XTEST_D3D_CHECK(m_d3dDevice->CreateInputLayout(vertexDesc, 4, vsByteCode.Data(), vsByteCode.ByteSize(), &m_inputLayout));
}

void LightsDemoApp::TransformMesh(XMMATRIX& transformMatrix, std::vector<xtest::mesh::MeshData::Vertex>& vertices)
{
	XMVECTOR vertexVector;
	for (auto& v : vertices)
	{
		vertexVector = XMLoadFloat3(&v.position);
		vertexVector = XMVector3Transform(vertexVector, transformMatrix);
		XMStoreFloat3(&v.position, vertexVector);
	}
}

void LightsDemoApp::InitBuffers()
{
	// how many meshes to draw
	m_vertexBuffers = std::vector<Microsoft::WRL::ComPtr<ID3D11Buffer>>(meshesNumber);
	m_indexBuffers = std::vector<Microsoft::WRL::ComPtr<ID3D11Buffer>>(meshesNumber);
	m_vsConstantBufferObjects = std::vector<Microsoft::WRL::ComPtr<ID3D11Buffer>>(meshesNumber);

	vertexBufferDescs = std::vector<D3D11_BUFFER_DESC>(meshesNumber);
	indexBufferDescs = std::vector<D3D11_BUFFER_DESC>(meshesNumber);
	vsConstantBufferDescs = std::vector<D3D11_BUFFER_DESC>(meshesNumber);
	vertexInitData = std::vector<D3D11_SUBRESOURCE_DATA>(meshesNumber);
	indexInitData = std::vector<D3D11_SUBRESOURCE_DATA>(meshesNumber);

	// first mesh - plane
	m_meshesToDraw.push_back(xtest::mesh::GeneratePlane(20.0f, 20.0f, 10, 10));

	// traslation of plane
	XMMATRIX translationMatrix = XMMatrixTranslation(0.0f, -1.0f, 0.0f);
	TransformMesh(translationMatrix, m_meshesToDraw[0].vertices);
	
	m_indexCountPerMesh.push_back((uint32)m_meshesToDraw[0].indices.size());

	m_meshesToDraw.push_back(xtest::mesh::GenerateBox(2.0f, 2.0f, 2.0f));
	m_indexCountPerMesh.push_back((uint32)m_meshesToDraw[1].indices.size());

	m_meshesToDraw.push_back(xtest::mesh::GenerateSphere(1.0f, 100, 100));
	m_indexCountPerMesh.push_back((uint32)m_meshesToDraw[2].indices.size());

	translationMatrix = XMMatrixTranslation(-3.0f, 0.0f, 2.0f);
	TransformMesh(translationMatrix, m_meshesToDraw[2].vertices);

	m_meshesToDraw.push_back(xtest::mesh::GenerateTorus(0.5f, 1.0f, 100, 100));
	m_indexCountPerMesh.push_back((uint32)m_meshesToDraw[3].indices.size());

	translationMatrix = XMMatrixTranslation(3.0f, 1.0f, 0.0f);
	TransformMesh(translationMatrix, m_meshesToDraw[3].vertices);

	std::wstring filePath = L"application\\resources\\data\\3d-objects\\crate.gpf";
	xtest::mesh::GPFMesh crate = xtest::file::ReadGPF(filePath);

	uint32 vertexCount1 = crate.meshDescriptorMapByName["metal_pieces_3"].vertexCount;
	uint32 vertexOffset1 = crate.meshDescriptorMapByName["metal_pieces_3"].vertexOffset;
	uint32 indexCount1 = crate.meshDescriptorMapByName["metal_pieces_3"].indexCount;
	uint32 indexOffset1 = crate.meshDescriptorMapByName["metal_pieces_3"].indexOffset;

	GenerateCrate(4, vertexCount1, indexCount1, vertexOffset1, indexOffset1, crate);

	vertexCount1 = crate.meshDescriptorMapByName["bottom_1"].vertexCount;
	vertexOffset1 = crate.meshDescriptorMapByName["bottom_1"].vertexOffset;
	indexCount1 = crate.meshDescriptorMapByName["bottom_1"].indexCount;
	indexOffset1 = crate.meshDescriptorMapByName["bottom_1"].indexOffset;

	GenerateCrate(5, vertexCount1, indexCount1, vertexOffset1, indexOffset1, crate);

	vertexCount1 = crate.meshDescriptorMapByName["handles_8"].vertexCount;
	vertexOffset1 = crate.meshDescriptorMapByName["handles_8"].vertexOffset;
	indexCount1 = crate.meshDescriptorMapByName["handles_8"].indexCount;
	indexOffset1 = crate.meshDescriptorMapByName["handles_8"].indexOffset;

	GenerateCrate(6, vertexCount1, indexCount1, vertexOffset1, indexOffset1, crate);

	vertexCount1 = crate.meshDescriptorMapByName["top_2"].vertexCount;
	vertexOffset1 = crate.meshDescriptorMapByName["top_2"].vertexOffset;
	indexCount1 = crate.meshDescriptorMapByName["top_2"].indexCount;
	indexOffset1 = crate.meshDescriptorMapByName["top_2"].indexOffset;

	GenerateCrate(7, vertexCount1, indexCount1, vertexOffset1, indexOffset1, crate);

	vertexCount1 = crate.meshDescriptorMapByName["top_handles_4"].vertexCount;
	vertexOffset1 = crate.meshDescriptorMapByName["top_handles_4"].vertexOffset;
	indexCount1 = crate.meshDescriptorMapByName["top_handles_4"].indexCount;
	indexOffset1 = crate.meshDescriptorMapByName["top_handles_4"].indexOffset;

	GenerateCrate(8, vertexCount1, indexCount1, vertexOffset1, indexOffset1, crate);

	for (int i = 0; i < meshesNumber; i++)
	{
		CreateBuffers(i);
	}

	D3D11_BUFFER_DESC vsConstantBufferDesc1;
	vsConstantBufferDesc1.Usage = D3D11_USAGE_DYNAMIC; // this buffer needs to be updated every frame
	vsConstantBufferDesc1.ByteWidth = sizeof(PerFrameCB);
	vsConstantBufferDesc1.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	vsConstantBufferDesc1.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vsConstantBufferDesc1.MiscFlags = 0;
	vsConstantBufferDesc1.StructureByteStride = 0;
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vsConstantBufferDesc1, nullptr, &m_vsConstantBufferFrame));
}

void LightsDemoApp::GenerateCrate(uint32 pos, uint32 vertexCount, uint32 indexCount, uint32 vertexOffset, uint32 indexOffset, xtest::mesh::GPFMesh& crate)
{
	xtest::mesh::MeshData crate0;
	std::vector<xtest::mesh::MeshData::Vertex> vertex;
	std::vector<uint32> indices;
	for (uint32 i = vertexOffset; i < vertexOffset+vertexCount; i++)
	{
		vertex.push_back(crate.meshData.vertices[i]);
	}
	for (uint32 i = indexOffset; i < indexOffset+indexCount; i++)
	{
		indices.push_back(crate.meshData.indices[i]);
	}
	crate0.vertices = vertex;
	crate0.indices = indices;

	m_meshesToDraw.push_back(crate0);
	m_indexCountPerMesh.push_back((uint32)m_meshesToDraw[pos].indices.size());

	XMMATRIX scaleMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f);
	XMMATRIX translationMatrix = XMMatrixTranslation(6.0f, -1.0f, 0.0f);
	XMMATRIX transformMatrix = scaleMatrix * translationMatrix;
	TransformMesh(transformMatrix, m_meshesToDraw[pos].vertices);
}

void LightsDemoApp::CreateBuffers(int i)
{
	vertexBufferDescs[i].Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDescs[i].ByteWidth = (UINT)(sizeof(xtest::mesh::MeshData::Vertex) * m_meshesToDraw[i].vertices.size());
	vertexBufferDescs[i].BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDescs[i].CPUAccessFlags = 0;
	vertexBufferDescs[i].MiscFlags = 0;
	vertexBufferDescs[i].StructureByteStride = 0;

	vertexInitData[i].pSysMem = m_meshesToDraw[i].vertices.data();
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDescs[i], &vertexInitData[i], &m_vertexBuffers[i]));

	indexBufferDescs[i].Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDescs[i].ByteWidth = (UINT)(sizeof(uint32) * m_meshesToDraw[i].indices.size());
	indexBufferDescs[i].BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDescs[i].CPUAccessFlags = 0;
	indexBufferDescs[i].MiscFlags = 0;
	indexBufferDescs[i].StructureByteStride = 0;

	indexInitData[i].pSysMem = m_meshesToDraw[i].indices.data();
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDescs[i], &indexInitData[i], &m_indexBuffers[i]));

	vsConstantBufferDescs[i].Usage = D3D11_USAGE_DYNAMIC; // this buffer needs to be updated every frame
	vsConstantBufferDescs[i].ByteWidth = (UINT)(sizeof(PerObjectCB));
	vsConstantBufferDescs[i].BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	vsConstantBufferDescs[i].CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vsConstantBufferDescs[i].MiscFlags = 0;
	vsConstantBufferDescs[i].StructureByteStride = 0;
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vsConstantBufferDescs[i], nullptr, &m_vsConstantBufferObjects[i]));
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
	DirectX::XMStoreFloat4x4(&m_projectionMatrix, P);
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
	//XTEST_ASSERT(key == input::Key::F); // the only key registered for this listener
	if (key == input::Key::A) // directional light on/off
	{
		directional = !directional;
	}
	if (key == input::Key::S) // pointlight on/off
	{
		point = !point;
	}
	if (key == input::Key::D) // spotlight on/off
	{
		spot = !spot;
	}
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
	DirectX::XMStoreFloat4x4(&m_worldMatrix, W);

	// create the model-view-projection matrix
	XMMATRIX V = m_camera.GetViewMatrix();
	DirectX::XMStoreFloat4x4(&m_viewMatrix, V);

	// create projection matrix
	XMMATRIX P = XMLoadFloat4x4(&m_projectionMatrix);
	XMMATRIX WVP = W * V*P;

	// matrices must be transposed since HLSL use column-major ordering.
	WVP = XMMatrixTranspose(WVP);

	DirectionalLight dirLight =
	{
		XMFLOAT4(0.6f, 0.3f, 0.3f, 1.0f),
		XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f),
		XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f),
		XMFLOAT3(1.0f, -1.0f, 0.0f),
		1.0f, // padding
	};

	DirectionalLight offDirLight =
	{
		XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
		XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
		XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		1.0f, // padding
	};

	PointLight pointLight =
	{
		XMFLOAT4(0.0f, 0.6f, 0.2f, 1.0f),
		XMFLOAT4(0.0f, 0.6f, 0.2f, 1.0f),
		XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f),
		XMFLOAT3(0.0f, 2.0f, 0.0f),
		5.0f,
		XMFLOAT3(0.05f, 0.05f, 0.05f),
	};
	PointLight offPointLight =
	{
		XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
		XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
		XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		0.0f,
		XMFLOAT3(0.0f, 0.0f, 0.0f),
	};
	SpotLight spotLight =
	{
		XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f),
		XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT3(4.0f, 0.0f, 0.0f),
		10.0f,
		XMFLOAT3(-1.0f, -0.8f, 0.0f),
		20.0f,
		XMFLOAT3(0.05f, 0.05f, 0.05f),
	};
	SpotLight offSpotLight =
	{
		XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
		XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
		XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		0.0f,
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		0.0f,
		XMFLOAT3(0.0f, 0.0f, 0.0f),
	};
	// plane
	Material material0 =
	{
		XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),
		XMFLOAT4(0.2f, 0.6f, 0.3f, 1.0f),
		XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f),
	};
	// box
	Material material1 =
	{
		XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f),
		XMFLOAT4(0.2f, 0.6f, 0.3f, 1.0f),
		XMFLOAT4(0.8f, 0.8f, 0.8f, 100.0f),
	};
	// sphere
	Material material2 =
	{
		XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f),
		XMFLOAT4(0.8f, 0.8f, 0.8f, 100.0f),
	};
	// torus
	Material material3 =
	{
		XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f),
		XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f),
		XMFLOAT4(0.8f, 0.8f, 0.8f, 100.0f),
	};
	// crate
	Material material4 =
	{
		XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f),
		XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f),
		XMFLOAT4(0.8f, 0.8f, 0.8f, 100.0f),
	};
	Material material5 =
	{
		XMFLOAT4(0.3f, 0.7f, 0.0f, 1.0f),
		XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f),
		XMFLOAT4(0.8f, 0.8f, 0.8f, 100.0f),
	};
	Material material6 =
	{
		XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f),
		XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f),
		XMFLOAT4(0.8f, 0.8f, 0.8f, 100.0f),
	};
	Material material7 =
	{
		XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f),
		XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f),
		XMFLOAT4(0.8f, 0.8f, 0.8f, 100.0f),
	};
	Material material8 =
	{
		XMFLOAT4(0.0f, 0.0f, 0.6f, 1.0f),
		XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f),
		XMFLOAT4(0.8f, 0.8f, 0.8f, 100.0f),
	};

	materialPerMesh.push_back(material0);
	materialPerMesh.push_back(material1);
	materialPerMesh.push_back(material2);
	materialPerMesh.push_back(material3);
	materialPerMesh.push_back(material4);
	materialPerMesh.push_back(material5);
	materialPerMesh.push_back(material6);
	materialPerMesh.push_back(material7);
	materialPerMesh.push_back(material8);

	m_d3dAnnotation->BeginEvent(L"update-constant-buffer");

	// load the constant buffer data in the gpu
	{
		//  ----- perObjectCB
		for (int i = 0; i < meshesNumber; i++)
		{
			MapPerMesh(i, WVP, W);
		}
		//  ----- perFrameCB
		D3D11_MAPPED_SUBRESOURCE mappedResource1;
		ZeroMemory(&mappedResource1, sizeof(D3D11_MAPPED_SUBRESOURCE));

		// disable gpu access
		XTEST_D3D_CHECK(m_d3dContext->Map(m_vsConstantBufferFrame.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource1));
		PerFrameCB* constantBufferData1 = (PerFrameCB*)mappedResource1.pData;

		//update the data
		if (directional)
			constantBufferData1->dirlight = dirLight;
		else
			constantBufferData1->dirlight = offDirLight;
		if(spot)
			constantBufferData1->spotLight = spotLight;
		else
			constantBufferData1->spotLight = offSpotLight;
		if(point)
			constantBufferData1->pointLight = pointLight;
		else
			constantBufferData1->pointLight = offPointLight;
		constantBufferData1->eyePosW = m_camera.GetPosition();
		// enable gpu access
		m_d3dContext->Unmap(m_vsConstantBufferFrame.Get(), 0);
	}
	m_d3dAnnotation->EndEvent();
}

void LightsDemoApp::MapPerMesh(int i, XMMATRIX& WVP, XMMATRIX& W)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	// disable gpu access
	XTEST_D3D_CHECK(m_d3dContext->Map(m_vsConstantBufferObjects[i].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	PerObjectCB* constantBufferData = (PerObjectCB*)mappedResource.pData;

	//update the data
	XMVECTOR determinant = XMMatrixDeterminant(W);
	XMVECTOR* ptrToDeterminant = &determinant;
	DirectX::XMStoreFloat4x4(&constantBufferData->WVP, WVP);
	DirectX::XMStoreFloat4x4(&constantBufferData->W, XMMatrixTranspose(W));
	DirectX::XMStoreFloat4x4(&constantBufferData->W_INV_T, (XMMatrixInverse(ptrToDeterminant, W)));
	constantBufferData->material = materialPerMesh[i];
	// enable gpu access
	m_d3dContext->Unmap(m_vsConstantBufferObjects[i].Get(), 0);
}

void LightsDemoApp::BindPerMesh(int i, UINT bufferRegister, UINT bufferRegister1, UINT stride, UINT offset)
{
	m_d3dContext->VSSetConstantBuffers(bufferRegister, 1, m_vsConstantBufferObjects[i].GetAddressOf());

	// PerObjectCB was defined as register 0 inside the vertex shader file
	m_d3dContext->PSSetConstantBuffers(bufferRegister, 1, m_vsConstantBufferObjects[i].GetAddressOf());
	m_d3dContext->PSSetConstantBuffers(bufferRegister1, 1, m_vsConstantBufferFrame.GetAddressOf());

	// set what to draw

	m_d3dContext->IASetVertexBuffers(0, 1, m_vertexBuffers[i].GetAddressOf(), &stride, &offset);
	m_d3dContext->IASetIndexBuffer(m_indexBuffers[i].Get(), DXGI_FORMAT_R32_UINT, 0);
	m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// draw and present the frame
	m_d3dContext->DrawIndexed(m_indexCountPerMesh[i], 0, 0);
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

	UINT bufferRegister = 0;
	UINT bufferRegister1 = 1;
	UINT stride = sizeof(VertexIn);
	UINT offset = 0;
	// bind the constant data to the vertex shader
	for (int i = 0; i < meshesNumber; i++)
	{
		BindPerMesh(i, bufferRegister, bufferRegister1, stride, offset);
	}

	XTEST_D3D_CHECK(m_swapChain->Present(0, 0));

	m_d3dAnnotation->EndEvent();
}

