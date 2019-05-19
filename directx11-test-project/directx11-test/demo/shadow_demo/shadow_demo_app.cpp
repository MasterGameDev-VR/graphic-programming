#include "stdafx.h"
#include "shadow_demo_app.h"
#include <file/file_utils.h>
#include <math/math_utils.h>
#include <service/locator.h>
//all the classes defined in vertex_input_types.h inherit from the class in vertex_input.h with the following parenting scheme
//MeshDataVertexInput <- VertexInput <- RenderResource
//define a new class in VertexInputTypes to load a new input layout and also a new .cso vertex shader
//I need to create a new input layout and a new vertex shader
#include <render/shading/vertex_input_types.h>

//not fully understood... see later
#include <render/shading/rasterizer_state_types.h>
#include <render/shading/sampler_types.h>
#include <external_libs/directxtk/WICTextureLoader.h>


//RasterizerState <- RenderResource   override of the Bind() method
// RasterizerStateTypes <- RasterizerState override of the Init() method

//-------------

//RenderPass <- RenderResource
/*
RenderPass members are:
**************
m_state (sharedPtr)    RenderPassState
m_vertexShader (sharedPtr) VertexShader
m_pixelShader (sharedPtr)  PixelShader
*********
*/

//RenderPassState<-RenderResource
/*
RenderPassState members are:
*******
m_primitiveTopology
m_viewPort
m_renderTargetView
m_depthStencilView
m_rasterizerState (sharedPtr)
*********
*/

//Renderable <- RenderResource

using namespace DirectX;
using namespace xtest;
using namespace xtest::render::shading;

using xtest::demo::ShadowDemoApp;
using Microsoft::WRL::ComPtr;


ShadowDemoApp::ShadowDemoApp(HINSTANCE instance,
	const application::WindowSettings& windowSettings,
	const application::DirectxSettings& directxSettings,
	uint32 fps /*=60*/)
	: application::DirectxApp(instance, windowSettings, directxSettings, fps)
	, m_dirKeyLight()
	, m_dirFillLight()
	, m_pointLight()
	, m_lightingControls()
	, m_isLightingControlsDirty(true)
	, m_stopLights(false)
	, m_camera(math::ToRadians(60.f), math::ToRadians(125.f), 5.f, { 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, { math::ToRadians(4.f), math::ToRadians(175.f) }, { 3.f, 25.f })
	, m_objects()
	, m_renderPass()
	, m_shadowRenderPass()
	,m_boundingSphere()
{}


ShadowDemoApp::~ShadowDemoApp()
{}


void ShadowDemoApp::Init()
{
	application::DirectxApp::Init();
	ShadowUtilities::Init();
	ShadowUtilities::SetViewPort();

	m_camera.SetPerspectiveProjection(math::ToRadians(45.f), AspectRatio(), 1.f, 1000.f);

	m_boundingSphere.radius=20.0f;
	m_boundingSphere.bSpherePositionLS= XMFLOAT4( 0.0f ,0.0f ,0.0f ,1.0f );
	m_boundingSphere.view_Left = m_boundingSphere.bSpherePositionLS.x - m_boundingSphere.radius;
	m_boundingSphere.view_Right = m_boundingSphere.bSpherePositionLS.x + m_boundingSphere.radius;	
	m_boundingSphere.view_Bottom= m_boundingSphere.bSpherePositionLS.y - m_boundingSphere.radius;
	m_boundingSphere.view_Top=m_boundingSphere.bSpherePositionLS.y + m_boundingSphere.radius;	
	m_boundingSphere.view_NearZ = m_boundingSphere.bSpherePositionLS.z - m_boundingSphere.radius;
	m_boundingSphere.view_FarZ = m_boundingSphere.bSpherePositionLS.z + m_boundingSphere.radius;
		
	InitRenderTechnique();
	InitShadowRenderTechnique();

	InitRenderables();
	InitLights();

	service::Locator::GetMouse()->AddListener(this);
	service::Locator::GetKeyboard()->AddListener(this, { input::Key::F, input::Key::F1, input::Key::F2, input::Key::F3, input::Key::space_bar });

	
}


void ShadowDemoApp::InitRenderTechnique()
{
	file::ResourceLoader* loader = service::Locator::GetResourceLoader();

	std::shared_ptr<VertexShader> vertexShader = std::make_shared<VertexShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\shadow_demo_VS.cso")));
	vertexShader->SetVertexInput(std::make_shared<MeshDataVertexInput>());
	vertexShader->AddConstantBuffer(CBufferFrequency::per_object, std::make_unique<CBuffer<PerObjectData>>());

	std::shared_ptr<PixelShader> pixelShader = std::make_shared<PixelShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\shadow_demo_PS.cso")));
	pixelShader->AddConstantBuffer(CBufferFrequency::per_object, std::make_unique<CBuffer<PerObjectData>>());
	pixelShader->AddConstantBuffer(CBufferFrequency::per_frame, std::make_unique<CBuffer<PerFrameData>>());
	pixelShader->AddConstantBuffer(CBufferFrequency::rarely_changed, std::make_unique<CBuffer<RarelyChangedData>>());
	pixelShader->AddSampler(SamplerUsage::common_textures, std::make_shared<AnisotropicSampler>());

	m_renderPass.SetState(std::make_shared<RenderPassState>(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_viewport, std::make_shared<SolidCullBackRS>(), m_backBufferView.Get(), m_depthBufferView.Get()));
	m_renderPass.SetVertexShader(vertexShader);
	m_renderPass.SetPixelShader(pixelShader);
	m_renderPass.Init();
}

void ShadowDemoApp::InitShadowRenderTechnique() 
{
	file::ResourceLoader* loader = service::Locator::GetResourceLoader();

	//I would like to create a shared pointer of a vertex shader....
	std::shared_ptr<VertexShader> vertexShader = std::make_shared<VertexShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\shadow_demo_shadowsVS.cso")));
	//..and add at least two constant buffer with a different frequency
	vertexShader->AddConstantBuffer(CBufferFrequency::per_object, std::make_unique<CBuffer<PerObjectData>>());
	vertexShader->AddConstantBuffer(CBufferFrequency::per_frame, std::make_unique<CBuffer<PerFrameData>>());
	std::shared_ptr<PixelShader> pixelShader = nullptr;
	
	


	m_shadowRenderPass.SetState(std::make_shared<RenderPassState>(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, ShadowUtilities::m_shadowsViewPort, std::make_shared<SolidCullBackRS>(), nullptr, ShadowUtilities::m_shadowDepthBufferView.Get()));
	m_shadowRenderPass.SetVertexShader(vertexShader);
	m_shadowRenderPass.SetPixelShader(pixelShader);
	m_shadowRenderPass.Init();
}

void ShadowDemoApp::InitLights()
{
	m_dirKeyLight.ambient = { 0.16f, 0.18f, 0.18f, 1.f };
	m_dirKeyLight.diffuse = { 2.f* 0.78f, 2.f* 0.83f, 2.f* 1.f, 1.f };
	m_dirKeyLight.specular = { 0.87f,  0.90f,  0.94f, 1.f };
	XMVECTOR dirLightDirection = XMVector3Normalize(-XMVectorSet(5.f, 3.f, 5.f, 0.f));
	XMStoreFloat3(&m_dirKeyLight.dirW, dirLightDirection);

	m_dirFillLight.ambient = { 0.16f, 0.18f, 0.18f, 1.f };
	m_dirFillLight.diffuse = { 0.4f * 1.f, 0.4f * 0.91f, 0.4f * 0.85f, 1.f };
	m_dirFillLight.specular = { 0.087f, 0.09f, 0.094f, 1.f };
	XMStoreFloat3(&m_dirFillLight.dirW, XMVectorScale(dirLightDirection, -1.f));

	m_pointLight.ambient = { 0.18f, 0.04f, 0.16f, 1.0f };
	m_pointLight.diffuse = { 0.4f* 0.87f,  0.4f*0.90f,   0.4f*0.94f, 1.f };
	m_pointLight.specular = { 0.4f*0.87f,   0.4f*0.90f,   0.4f*0.94f, 1.f };
	m_pointLight.posW = { -3.75f, 1.f, 3.75f };
	m_pointLight.range = 20.f;
	m_pointLight.attenuation = { 0.0f, 0.2f, 0.f };


	m_lightingControls.useDirLight = true;
	m_lightingControls.usePointLight = true;
	m_lightingControls.useBumpMap = true;
}
void ShadowDemoApp::InitRenderables()
{
	//ground
	{
		mesh::MeshMaterial mat;
		mat.ambient = { 0.15f, 0.15f, 0.15f, 1.f };
		mat.diffuse = { 0.52f, 0.52f, 0.48f, 1.f };
		mat.specular = { 0.5f, 0.5f, 0.5f, 1.f };
		mat.diffuseMap = GetRootDir().append(LR"(\3d-objects\ground\ground_color.png)");
		mat.normalMap = GetRootDir().append(LR"(\3d-objects\ground\ground_norm.png)");
		mat.glossMap = GetRootDir().append(LR"(\3d-objects\ground\ground_gloss.png)");

		render::Renderable plane(mesh::GeneratePlane(30.f, 30.f, 30, 30), mat);
		plane.SetTransform(XMMatrixIdentity());
		plane.SetTexcoordTransform(XMMatrixScaling(3.f, 3.f, 3.f));
		plane.Init();
		m_objects.push_back(plane);
	}
	render::Renderable crate{ *(service::Locator::GetResourceLoader()->LoadGPFMesh(GetRootDir().append(LR"(\3d-objects\crate\crate.gpf)"))) };
	crate.SetTransform(XMMatrixTranslation(0.f, 0.f, 0.f));
	crate.Init();
	m_objects.push_back(std::move(crate));

	render::Renderable female{ *(service::Locator::GetResourceLoader()->LoadGPFMesh(GetRootDir().append(LR"(\3d-objects\gdc_female\gdc_female.gpf)"))) };
	female.SetTransform(XMMatrixTranslation(-8.0f, 0.0f, -8.0f));
	female.Init();
	m_objects.push_back(female);

	render::Renderable femalePosed{ *(service::Locator::GetResourceLoader()->LoadGPFMesh(GetRootDir().append(LR"(\3d-objects\gdc_female\gdc_female_posed.gpf)"))) };
	femalePosed.SetTransform(XMMatrixTranslation(10.0f, 0.0f, 10.0f));
	femalePosed.Init();
	m_objects.push_back(femalePosed);

}





void ShadowDemoApp::OnResized()
{
	application::DirectxApp::OnResized();

	//update the render pass state with the resized render target and depth buffer
	m_renderPass.GetState()->ChangeRenderTargetView(m_backBufferView.Get());
	m_renderPass.GetState()->ChangeDepthStencilView(m_depthBufferView.Get());
	m_renderPass.GetState()->ChangeViewPort(m_viewport);

	//update the projection matrix with the new aspect ratio
	m_camera.SetPerspectiveProjection(math::ToRadians(45.f), AspectRatio(), 1.f, 1000.f);
}


void ShadowDemoApp::OnWheelScroll(input::ScrollStatus scroll)
{
	// move forward/backward when the wheel is used
	if (service::Locator::GetMouse()->IsInClientArea())
	{
		XMFLOAT3 cameraZ = m_camera.GetZAxis();
		XMFLOAT3 forwardMovement;
		XMStoreFloat3(&forwardMovement, XMVectorScale(XMLoadFloat3(&cameraZ), scroll.isScrollingUp ? 0.5f : -0.5f));
		m_camera.TranslatePivotBy(forwardMovement);
	}
}

void ShadowDemoApp::OnMouseMove(const DirectX::XMINT2& movement, const DirectX::XMINT2& currentPos)
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


void ShadowDemoApp::OnKeyStatusChange(input::Key key, const input::KeyStatus& status)
{

	// re-frame F key is pressed
	if (key == input::Key::F && status.isDown)
	{
		m_camera.SetPivot({ 0.f, 0.f, 0.f });
	}
	else if (key == input::Key::F1 && status.isDown)
	{
		m_lightingControls.useDirLight = !m_lightingControls.useDirLight;
		m_isLightingControlsDirty = true;
	}
	else if (key == input::Key::F2 && status.isDown)
	{
		m_lightingControls.usePointLight = !m_lightingControls.usePointLight;
		m_isLightingControlsDirty = true;
	}
	else if (key == input::Key::F3 && status.isDown)
	{
		m_lightingControls.useBumpMap = !m_lightingControls.useBumpMap;
		m_isLightingControlsDirty = true;
	}
	else if (key == input::Key::space_bar && status.isDown)
	{
		m_stopLights = !m_stopLights;
	}
}

void ShadowDemoApp::UpdateScene(float deltaSeconds)
{

	// PerFrameCB
	{
		if (!m_stopLights)
		{
			XMMATRIX R = XMMatrixRotationY(math::ToRadians(30.f) * deltaSeconds);
			XMStoreFloat3(&m_pointLight.posW, XMVector3Transform(XMLoadFloat3(&m_pointLight.posW), R));
		}

		XMFLOAT3 tmp[4];
		tmp[0] = m_pointLight.posW;
		XMStoreFloat3(&tmp[1], XMVector3Transform(XMLoadFloat3(&tmp[0]), XMMatrixRotationY(math::ToRadians(90.f))));
		XMStoreFloat3(&tmp[2], XMVector3Transform(XMLoadFloat3(&tmp[1]), XMMatrixRotationY(math::ToRadians(90.f))));
		XMStoreFloat3(&tmp[3], XMVector3Transform(XMLoadFloat3(&tmp[2]), XMMatrixRotationY(math::ToRadians(90.f))));


		PerFrameData data;
		data.dirLights[0] = m_dirKeyLight;
		data.dirLights[1] = m_dirFillLight;
		data.pointLights[0] = data.pointLights[1] = data.pointLights[2] = data.pointLights[3] = m_pointLight;
		data.pointLights[0].posW = tmp[0];
		data.pointLights[1].posW = tmp[1];
		data.pointLights[2].posW = tmp[2];
		data.pointLights[3].posW = tmp[3];
		data.eyePosW = m_camera.GetPosition();

		
		XMVECTOR bSpherePosLS=XMLoadFloat4(&m_boundingSphere.bSpherePositionLS);
		float radius = m_boundingSphere.radius;
		XMVECTOR lightCameraDir{ m_dirKeyLight.dirW.x * radius,m_dirKeyLight.dirW.y * radius,m_dirKeyLight.dirW.z * radius ,0.0f };
		XMVECTOR eyePosition = lightCameraDir + bSpherePosLS;
		XMStoreFloat4x4(&LVMtemp[0], XMMatrixLookAtLH(eyePosition, bSpherePosLS,XMVectorSet(0.0f,1.0f,0.0f,0.0f)));
		data.LightViewMatrices[0] = LVMtemp[0];
		
		XMStoreFloat4x4(&PrMatrtemp[0], XMMatrixOrthographicOffCenterLH(
			m_boundingSphere.view_Left,
			m_boundingSphere.view_Right,
			m_boundingSphere.view_Bottom,
			m_boundingSphere.view_Top,
			m_boundingSphere.view_NearZ,
			m_boundingSphere.view_FarZ
		));
		data.ProjectionMatrices[0] = PrMatrtemp[0];

		m_renderPass.GetPixelShader()->GetConstantBuffer(CBufferFrequency::per_frame)->UpdateBuffer(data);

		m_shadowRenderPass.GetVertexShader()->GetConstantBuffer(CBufferFrequency::per_frame)->UpdateBuffer(data);
		//m_shadowRenderPass.GetState()->ChangeViewPort(m_viewport);
	}
	//PerFrameCBShadows
	//{m_shadowRenderPass.GetVertexShader()->GetConstantBuffer(CBufferFrequency::per_frame)->UpdateBuffer();}


	// RarelyChangedCB
	if (m_isLightingControlsDirty)
	{
		m_renderPass.GetPixelShader()->GetConstantBuffer(CBufferFrequency::rarely_changed)->UpdateBuffer(m_lightingControls);
		m_isLightingControlsDirty = false;
	}

}
/*
void ShadowDemoApp::UpdateShadowRenderPass()
{
	m_shadowRenderPass.GetState()->ChangeRenderTargetView(m_backBufferView.Get());
	m_shadowRenderPass.GetState()->ChangeDepthStencilView(m_depthBufferView.Get());
	m_shadowRenderPass.GetState()->ChangeViewPort(m_viewport);
}
*/

void ShadowDemoApp::RenderShadows()
{
	m_d3dAnnotation->BeginEvent(L"render-scene");

	//the Bind method is the RenderPass class version and deactivates the pixel shader - see render_pass.cpp row 46 - 56
	//the following lines calls also the Bind method of the RenderPassState instance (so OMSetRenderTargets is called)
	m_shadowRenderPass.Bind();
	m_shadowRenderPass.GetState()->ClearDepthOnly();
	//unuseful: the RenderTarget is set to null
	//m_shadowRenderPass.GetState()->ClearRenderTarget(DirectX::Colors::DarkGray);

	//here we should pass another TEXTURE, the shadowMap - is it necessary for each object?
	//m_renderPass.GetVertexShader()->BindTexture(TextureUsage::shadow_map, );
	// draw objects
	for (render::Renderable& renderable : m_objects)
	{
		for (const std::string& meshName : renderable.GetMeshNames())
		{
			PerObjectData data = ToPerObjectData(renderable, meshName);
			XMStoreFloat4x4(&data.WVP, XMMatrixMultiply(  XMMatrixMultiply(XMLoadFloat4x4(&data.W) , XMLoadFloat4x4(&LVMtemp[0]) ) ,  XMLoadFloat4x4(&PrMatrtemp[0]) ) );
			//data.WVP = data.W * LVMtemp[0] *  PrMatrtemp[0];
			m_shadowRenderPass.GetVertexShader()->GetConstantBuffer(CBufferFrequency::per_object)->UpdateBuffer(data);
			//m_shadowRenderPass.GetVertexShader()->BindTexture(TextureUsage::shadow_map, );

			renderable.Draw(meshName);
		}
	}


	m_d3dAnnotation->EndEvent();
}

void ShadowDemoApp::RenderScene()
{
	m_d3dAnnotation->BeginEvent(L"render-scene");


	m_renderPass.Bind();
	m_renderPass.GetState()->ClearDepthOnly();
	m_renderPass.GetState()->ClearRenderTarget(DirectX::Colors::DarkGray);

	

	// draw objects
	for (render::Renderable& renderable : m_objects)
	{
		for (const std::string& meshName : renderable.GetMeshNames())
		{
			PerObjectData data = ToPerObjectData(renderable, meshName);
			m_renderPass.GetVertexShader()->GetConstantBuffer(CBufferFrequency::per_object)->UpdateBuffer(data);
			m_renderPass.GetPixelShader()->GetConstantBuffer(CBufferFrequency::per_object)->UpdateBuffer(data);
			m_renderPass.GetPixelShader()->BindTexture(TextureUsage::color, renderable.GetTextureView(TextureUsage::color, meshName));
			m_renderPass.GetPixelShader()->BindTexture(TextureUsage::normal, renderable.GetTextureView(TextureUsage::normal, meshName));
			m_renderPass.GetPixelShader()->BindTexture(TextureUsage::glossiness, renderable.GetTextureView(TextureUsage::glossiness, meshName));
			


			renderable.Draw(meshName);
		}
	}


	XTEST_D3D_CHECK(m_swapChain->Present(0, 0));

	m_d3dAnnotation->EndEvent();
}


ShadowDemoApp::PerObjectData ShadowDemoApp::ToPerObjectData(const render::Renderable& renderable, const std::string& meshName) const
{
	PerObjectData data;

	XMMATRIX W = XMLoadFloat4x4(&renderable.GetTransform());
	XMMATRIX T = XMLoadFloat4x4(&renderable.GetTexcoordTransform(meshName));
	XMMATRIX V = m_camera.GetViewMatrix();
	XMMATRIX P = m_camera.GetProjectionMatrix();
	XMMATRIX WVP = W * V*P;

	XMStoreFloat4x4(&data.W, XMMatrixTranspose(W));
	XMStoreFloat4x4(&data.WVP, XMMatrixTranspose(WVP));
	XMStoreFloat4x4(&data.W_inverseTraspose, XMMatrixInverse(nullptr, W));
	XMStoreFloat4x4(&data.TexcoordMatrix, XMMatrixTranspose(T));
	data.material.ambient = renderable.GetMaterial(meshName).ambient;
	data.material.diffuse = renderable.GetMaterial(meshName).diffuse;
	data.material.specular = renderable.GetMaterial(meshName).specular;

	return data;
}

