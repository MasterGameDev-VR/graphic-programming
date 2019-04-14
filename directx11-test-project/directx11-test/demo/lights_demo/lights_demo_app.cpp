#include "stdafx.h"
#include "lights_demo_app.h"
#include <file/file_utils.h>
#include <math/math_utils.h>
#include <service/locator.h>

<<<<<<< HEAD
=======

>>>>>>> master
using namespace DirectX;
using namespace xtest;

using xtest::demo::LightsDemoApp;
using Microsoft::WRL::ComPtr;

<<<<<<< HEAD

=======
>>>>>>> master
LightsDemoApp::LightsDemoApp(HINSTANCE instance,
	const application::WindowSettings& windowSettings,
	const application::DirectxSettings& directxSettings,
	uint32 fps /*=60*/)
	: application::DirectxApp(instance, windowSettings, directxSettings, fps)
<<<<<<< HEAD
	, m_vertexBufferPlane(nullptr)
	, m_indexBufferPlane(nullptr)
	, m_vertexBufferTorus(nullptr)
	, m_indexBufferTorus(nullptr)
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
=======
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
>>>>>>> master


LightsDemoApp::~LightsDemoApp()
{}
<<<<<<< HEAD
=======


>>>>>>> master
void LightsDemoApp::Init()
{
	application::DirectxApp::Init();

	m_d3dAnnotation->BeginEvent(L"init-demo");

	InitMatrices();
<<<<<<< HEAD
	InitLights();
	InitMaterials();
	InitShaders();
	InitBuffers();
	InitRasterizerState();

	service::Locator::GetMouse()->AddListener(this);
	service::Locator::GetKeyboard()->AddListener(this, { input::Key::F });
=======
	InitShaders();
	InitRenderable();
	InitLights();
	InitRasterizerState();

	service::Locator::GetMouse()->AddListener(this);
	service::Locator::GetKeyboard()->AddListener(this, { input::Key::F, input::Key::F1, input::Key::F2, input::Key::F3, input::Key::space_bar });
>>>>>>> master

	m_d3dAnnotation->EndEvent();
}

<<<<<<< HEAD
void LightsDemoApp::InitLights()
{
	color_dirLight_ambient = { 0.16f, 0.18f, 0.18f,1.0f };
	color_dirLight_diffuse = { 0.4f* 0.87f, 0.4f*0.9f, 0.4f*0.94f,1.0f };
	color_dirLight_specular = { 0.87f,0.9f,0.94f,1.0f };
	color_pointLight_ambient = { 0.18f,  0.04f,  0.16f,1.0f };
	color_pointLigh_diffuse = { 0.94f, 0.23f, 0.87f,1.0f };
	color_pointLight_specular = { 0.94f, 0.23f, 0.87f,1.0f };
	color_spotLight_ambient = { 0.018f,  0.18f,  0.18f,1.0f };
	color_spotLight_diffuse = { 0.1f, 0.1f, 0.9f,1.0f };
	color_spotLight_specular = { 0.1f, 0.1f, 0.9f,4.0f };

	dirSpotLight = DirectX::XMFLOAT3{ -2.0f,-2.0f,7.0f };
	float dirLength = sqrtf((dirSpotLight.x)*(dirSpotLight.x) + (dirSpotLight.y)*(dirSpotLight.y) + (dirSpotLight.z)*(dirSpotLight.z));
	dirSpotLight = DirectX::XMFLOAT3{ dirSpotLight.x / dirLength,dirSpotLight.y / dirLength,dirSpotLight.z / dirLength };

	myDirectionalLight = { color_dirLight_ambient ,color_dirLight_diffuse ,color_dirLight_specular,DirectX::XMFLOAT3{1 / sqrtf(2.0f),0.0f,1 / sqrtf(2.0f)},0.0f };
	// colors ---- direction -------- padding float
	myPointLight = { color_pointLight_ambient ,color_pointLigh_diffuse ,color_pointLight_specular,DirectX::XMFLOAT3{35.0f,35.0f,35.0f},10.0f, DirectX::XMFLOAT3{0.0f,0.2f,0.0f},0.0f };
	// colors --------position ------- range ------- attenuation ---- padding float

	mySpotLight = { color_spotLight_ambient, color_spotLight_diffuse, color_spotLight_specular , DirectX::XMFLOAT3{30.0f,90.0f,+30.0f}, 100.0f,DirectX::XMFLOAT3{-1 / sqrtf(2.0f),0.0f,-1 / sqrtf(2.0f)} , 0.6f, DirectX::XMFLOAT3{1.0f,0.0f,0.0f},0.0f };
	// colors --------position ------- range -------direction ----------- spot dimension (cosine of the angle)---------  attenuation ---- padding float


	eyePosW = DirectX::XMFLOAT3{ 0.f, 0.f, 0.f };   //possibile errore che causa il malfunzionamento di PointLightContrib e SpotLightContrib

}
void LightsDemoApp::InitMaterials()
{
	mat_plane_rgba_ambient = { 0.15f,0.15f,0.15f,1.0f };
	mat_plane_rgba_diffuse = { 0.52f,0.52f,0.52f,1.0f };
	mat_plane_rgba_specular = { 0.8f,0.8f,0.8f,190.0f };

	mat_torus_rgba_ambient = { 0.7f,0.1f,0.1f,1.0f };
	mat_torus_rgba_diffuse = { 0.81f,0.15f,0.15f,1.0f };
	mat_torus_rgba_specular = { 0.7f,0.7f,0.7f,40.0f };

	mat_sphere_rgba_ambient = { 0.7f,0.1f,0.1f,1.0f };
	mat_sphere_rgba_diffuse = { 0.81f,0.15f,0.15f,1.0f };
	mat_sphere_rgba_specular = { 0.7f,0.7f,0.7f,40.0f };

	mat_box_rgba_ambient = { 0.0f,0.0f,1.0f,100.0f };
	mat_box_rgba_diffuse = { 0.0f,0.0f,1.0f,100.0f };
	mat_box_rgba_specular = { 0.0f,0.0f,1.0f,100.0f };

	mat_plane = { mat_plane_rgba_ambient,  mat_plane_rgba_diffuse , mat_plane_rgba_specular };
	mat_torus = { mat_torus_rgba_ambient,  mat_torus_rgba_diffuse , mat_torus_rgba_specular };
	mat_sphere = { mat_sphere_rgba_ambient,   mat_sphere_rgba_diffuse ,  mat_sphere_rgba_specular };  
	mat_box = { mat_box_rgba_ambient,  mat_box_rgba_diffuse , mat_box_rgba_specular }; 

}

void LightsDemoApp::InitMatrices()
{
	//XMMATRIX translateSphere = XMMatrixIdentity();
	//XMMATRIX translatePlane = XMMatrixIdentity();
	

	// world matrices
	XMStoreFloat4x4(&m_worldMatrixPlane, XMMatrixIdentity());
	XMStoreFloat4x4(&m_worldMatrixTorus, XMMatrixIdentity());

	XMStoreFloat4x4(&m_worldMatrixBox, XMMatrixIdentity());
	XMStoreFloat4x4(&m_worldMatrixSphere, XMMatrixIdentity());
	
=======

void LightsDemoApp::InitMatrices()
{
>>>>>>> master
	// view matrix
	XMStoreFloat4x4(&m_viewMatrix, m_camera.GetViewMatrix());

	// projection matrix
	{
		XMMATRIX P = XMMatrixPerspectiveFovLH(math::ToRadians(45.f), AspectRatio(), 1.f, 1000.f);
		XMStoreFloat4x4(&m_projectionMatrix, P);
	}
}

<<<<<<< HEAD
void LightsDemoApp::OnResized()
{
	application::DirectxApp::OnResized();

	//update the projection matrix with the new aspect ratio
	XMMATRIX P = XMMatrixPerspectiveFovLH(math::ToRadians(45.f), AspectRatio(), 1.f, 1000.f);
	XMStoreFloat4x4(&m_projectionMatrix, P);
}

void LightsDemoApp::InitShaders() {
=======

void LightsDemoApp::InitShaders()
{
>>>>>>> master
	// read pre-compiled shaders' bytecode
	std::future<file::BinaryFile> psByteCodeFuture = file::ReadBinaryFile(std::wstring(GetRootDir()).append(L"\\lights_demo_PS.cso"));
	std::future<file::BinaryFile> vsByteCodeFuture = file::ReadBinaryFile(std::wstring(GetRootDir()).append(L"\\lights_demo_VS.cso"));

	// future.get() can be called only once
	file::BinaryFile vsByteCode = vsByteCodeFuture.get();
	file::BinaryFile psByteCode = psByteCodeFuture.get();
	XTEST_D3D_CHECK(m_d3dDevice->CreateVertexShader(vsByteCode.Data(), vsByteCode.ByteSize(), nullptr, &m_vertexShader));
	XTEST_D3D_CHECK(m_d3dDevice->CreatePixelShader(psByteCode.Data(), psByteCode.ByteSize(), nullptr, &m_pixelShader));
<<<<<<< HEAD
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
	MeshData torusMesh = GenerateTorus(20.0f, 70.0f, 360, 360);

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

	//------------------- Torus
	vertexBufferDesc.ByteWidth = UINT(sizeof(MeshData::Vertex)*torusMesh.vertices.size());
	indexBufferDesc.ByteWidth = UINT(sizeof(uint32)*torusMesh.indices.size());

	D3D11_SUBRESOURCE_DATA torusVerticesInitData;
	torusVerticesInitData.pSysMem = &torusMesh.vertices[0];
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &torusVerticesInitData, &m_vertexBufferTorus));
	D3D11_SUBRESOURCE_DATA torusIndicesInitdata;
	torusIndicesInitdata.pSysMem = &torusMesh.indices[0];
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &torusIndicesInitdata, &m_indexBufferTorus));

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
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vsConstantBufferDesc, nullptr, &m_vsConstantBufferTorus));
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

=======


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


>>>>>>> master
void LightsDemoApp::InitRasterizerState()
{
	// rasterizer state
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
<<<<<<< HEAD
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
=======
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
>>>>>>> master
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthClipEnable = true;

	m_d3dDevice->CreateRasterizerState(&rasterizerDesc, &m_rasterizerState);
}

<<<<<<< HEAD
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

	XMMATRIX W_torus = XMLoadFloat4x4(&m_worldMatrixTorus);
	W_torus.r[3] = { -100.0f,40.0f,-100.0f,1.0f };
	//W_plane.r[3].m128_f32[0] = -1.0f;
	XMStoreFloat4x4(&m_worldMatrixTorus, W_torus);

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
	XMMATRIX WVP_torus = W_torus * V*P;
	XMMATRIX WVP_sphere = W_sphere * V*P;
	XMMATRIX WVP_box = W_box * V*P;

	XMVECTOR det_W_plane = XMMatrixDeterminant(W_plane);
	XMVECTOR det_W_torus = XMMatrixDeterminant(W_torus);
	XMVECTOR det_W_sphere = XMMatrixDeterminant(W_sphere);
	XMVECTOR det_W_box = XMMatrixDeterminant(W_box);
	XMMATRIX W_Inv_Transp_plane = XMMatrixInverse(&det_W_plane,W_plane);
	XMMATRIX W_Inv_Transp_torus = XMMatrixInverse(&det_W_torus, W_torus);
	XMMATRIX W_Inv_Transp_sphere = XMMatrixInverse(&det_W_sphere, W_sphere);
	XMMATRIX W_Inv_Transp_box = XMMatrixInverse(&det_W_box, W_box);

	// matrices must be transposed since HLSL use column-major ordering.
	WVP_plane = XMMatrixTranspose(WVP_plane);
	WVP_torus = XMMatrixTranspose(WVP_torus);
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
		D3D11_MAPPED_SUBRESOURCE mappedResourceTorus;
		ZeroMemory(&mappedResourceTorus, sizeof(D3D11_MAPPED_SUBRESOURCE));

		// disable gpu access
		XTEST_D3D_CHECK(m_d3dContext->Map(m_vsConstantBufferTorus.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResourceTorus));
		PerObjectCB* constantBufferDataTorus = (PerObjectCB*)mappedResourceTorus.pData;

		//update the data
		XMStoreFloat4x4(&constantBufferDataTorus->W, W_torus);
		XMStoreFloat4x4(&constantBufferDataTorus->WVP, WVP_torus);
		XMStoreFloat4x4(&constantBufferDataTorus->W_Inv_Transp, W_Inv_Transp_torus);
		XMStoreFloat4(&constantBufferDataTorus->material.ambient, XMLoadFloat4(&mat_torus.ambient));
		XMStoreFloat4(&constantBufferDataTorus->material.diffuse, XMLoadFloat4(&mat_torus.diffuse));
		XMStoreFloat4(&constantBufferDataTorus->material.specular, XMLoadFloat4(&mat_torus.specular));




		// enable gpu access
		m_d3dContext->Unmap(m_vsConstantBufferTorus.Get(), 0);
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

=======

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


>>>>>>> master
void LightsDemoApp::RenderScene()
{
	m_d3dAnnotation->BeginEvent(L"render-scene");

	// clear the frame
	m_d3dContext->ClearDepthStencilView(m_depthBufferView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
<<<<<<< HEAD
	m_d3dContext->ClearRenderTargetView(m_backBufferView.Get(), DirectX::Colors::Transparent);
=======
	m_d3dContext->ClearRenderTargetView(m_backBufferView.Get(), DirectX::Colors::DarkGray);
>>>>>>> master

	// set the shaders and the input layout
	m_d3dContext->RSSetState(m_rasterizerState.Get());
	m_d3dContext->IASetInputLayout(m_inputLayout.Get());
	m_d3dContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	m_d3dContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

<<<<<<< HEAD
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

	// set what to draw - SPHERE!!!

	m_d3dContext->VSSetConstantBuffers(bufferRegisterMatrices, 1, m_vsConstantBufferSphere.GetAddressOf());
	m_d3dContext->PSSetConstantBuffers(bufferRegisterMatrices, 1, m_vsConstantBufferSphere.GetAddressOf());

	m_d3dContext->IASetVertexBuffers(0, 1, m_vertexBufferSphere.GetAddressOf(), &stride, &offset);
	m_d3dContext->IASetIndexBuffer(m_indexBufferSphere.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_indexBufferSphere->GetDesc(&tempIndexBufferDesc);
	// draw and present the frame
	m_d3dContext->DrawIndexed(tempIndexBufferDesc.ByteWidth / sizeof(uint32), 0, 0);
	
	// set what to draw - TORUS
	
	
	m_d3dContext->VSSetConstantBuffers(bufferRegisterMatrices, 1, m_vsConstantBufferTorus.GetAddressOf());
	m_d3dContext->PSSetConstantBuffers(bufferRegisterMatrices, 1, m_vsConstantBufferTorus.GetAddressOf());

	m_d3dContext->IASetVertexBuffers(0, 1, m_vertexBufferTorus.GetAddressOf(), &stride, &offset);
	m_d3dContext->IASetIndexBuffer(m_indexBufferTorus.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_indexBufferTorus->GetDesc(&tempIndexBufferDesc);
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


=======
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

>>>>>>> master
