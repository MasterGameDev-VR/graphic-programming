#include "stdafx.h"
#include "shadow_demo_app.h"
#include <file/file_utils.h>
#include <math/math_utils.h>
#include <service/locator.h>
#include <render/shading/vertex_input_types.h>
#include <render/shading/rasterizer_state_types.h>
#include <render/shading/sampler_types.h>
#include <external_libs/directxtk/WICTextureLoader.h>


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
	, m_bSphere(XMFLOAT3(0.0f, 0.0f, 0.0f), 20.0f)
	, m_shadowMap()
	, m_shadowMapDB()
	, m_resolution(2048)
	, m_spotLight()
	, m_stopLights(false)
	, m_camera(math::ToRadians(60.f), math::ToRadians(125.f), 5.f, { 12.f, 10.f, -12.f }, { 0.f, 1.f, 0.f }, { math::ToRadians(4.f), math::ToRadians(175.f) }, { 3.f, 25.f })
	, m_objects()
	, m_renderPass()
{
}


ShadowDemoApp::~ShadowDemoApp()
{}


void ShadowDemoApp::Init()
{
	application::DirectxApp::Init();

	m_camera.SetPerspectiveProjection(math::ToRadians(45.f), AspectRatio(), 1.f, 1000.f);

	InitShadowMap();
	InitRenderTechnique();
	InitRenderables();
	InitLights();

	service::Locator::GetMouse()->AddListener(this);
	service::Locator::GetKeyboard()->AddListener(this, { input::Key::F, input::Key::F1, input::Key::F2, input::Key::F3, input::Key::F4, input::Key::space_bar });
}

void ShadowDemoApp::InitShadowMap() 
{
	{
		D3D11_TEXTURE2D_DESC textureDesc;
		textureDesc.Width = m_resolution;
		textureDesc.Height = m_resolution;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		textureDesc.SampleDesc.Count = 1;	// no MSAA
		textureDesc.SampleDesc.Quality = 0;	// no MSAA
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;
		XTEST_D3D_CHECK(m_d3dDevice->CreateTexture2D(&textureDesc, nullptr, &m_shadowMap));

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		depthStencilViewDesc.Flags = 0;
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
		XTEST_D3D_CHECK(m_d3dDevice->CreateDepthStencilView(m_shadowMap.Get(), &depthStencilViewDesc, &m_shadowMapDB));

		D3D11_SHADER_RESOURCE_VIEW_DESC shaderViewDesc;
		shaderViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS; // 24bit red channel (depth), 8 bit unused (stencil)
		shaderViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderViewDesc.Texture2D.MipLevels = 1;
		shaderViewDesc.Texture2D.MostDetailedMip = 0;
		XTEST_D3D_CHECK(m_d3dDevice->CreateShaderResourceView(m_shadowMap.Get(), &shaderViewDesc, &m_shadowMapPS));

		m_shadowMapViewport.TopLeftX = 0.0f;
		m_shadowMapViewport.TopLeftY = 0.0f;
		m_shadowMapViewport.Width = static_cast<float>(m_resolution);
		m_shadowMapViewport.Height = static_cast<float>(m_resolution);
		m_shadowMapViewport.MinDepth = 0.0f;
		m_shadowMapViewport.MaxDepth = 1.0f;
	}

	{
		// Projector
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(service::Locator::GetD3DDevice(), service::Locator::GetD3DContext(), std::wstring(GetRootDir()).append(L"\\grumpy.png").c_str(), m_projectorTexture.GetAddressOf(), m_projectorTexturePS.GetAddressOf()));
		
		D3D11_TEXTURE2D_DESC textureDesc;
		textureDesc.Width = m_resolution;
		textureDesc.Height = m_resolution;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		textureDesc.SampleDesc.Count = 1;	// no MSAA
		textureDesc.SampleDesc.Quality = 0;	// no MSAA
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;
		XTEST_D3D_CHECK(m_d3dDevice->CreateTexture2D(&textureDesc, nullptr, &m_projector));

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		depthStencilViewDesc.Flags = 0;
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
		XTEST_D3D_CHECK(m_d3dDevice->CreateDepthStencilView(m_projector.Get(), &depthStencilViewDesc, &m_projectorDB));

		D3D11_SHADER_RESOURCE_VIEW_DESC shaderViewDesc;
		shaderViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS; // 24bit red channel (depth), 8 bit unused (stencil)
		shaderViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderViewDesc.Texture2D.MipLevels = 1;
		shaderViewDesc.Texture2D.MostDetailedMip = 0;
		XTEST_D3D_CHECK(m_d3dDevice->CreateShaderResourceView(m_projector.Get(), &shaderViewDesc, &m_projectorPS));

		m_projectorViewport.TopLeftX = 0.0f;
		m_projectorViewport.TopLeftY = 0.0f;
		m_projectorViewport.Width = static_cast<float>(m_resolution);
		m_projectorViewport.Height = static_cast<float>(m_resolution);
		m_projectorViewport.MinDepth = 0.0f;
		m_projectorViewport.MaxDepth = 1.0f;
	}
}


void ShadowDemoApp::InitRenderTechnique()
{
	file::ResourceLoader* loader = service::Locator::GetResourceLoader();

	std::shared_ptr<VertexShader> vertexShader = std::make_shared<VertexShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\shadow_demo_VS.cso")));
	vertexShader->SetVertexInput(std::make_shared<MeshDataVertexInput>());
	vertexShader->AddConstantBuffer(CBufferFrequency::per_object, std::make_unique<CBuffer<PerObjectData>>());

	std::shared_ptr<VertexShader> shadowMapVertexShader = std::make_shared<VertexShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\firstpass_shadow_demo_VS.cso")));
	shadowMapVertexShader->SetVertexInput(std::make_shared<MeshDataVertexInput>());
	shadowMapVertexShader->AddConstantBuffer(CBufferFrequency::per_object, std::make_unique<CBuffer<PerObjectData>>());

	std::shared_ptr<VertexShader> projectorVertexShader = std::make_shared<VertexShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\firstpass_shadow_demo_VS.cso")));
	projectorVertexShader->SetVertexInput(std::make_shared<MeshDataVertexInput>());
	projectorVertexShader->AddConstantBuffer(CBufferFrequency::per_object, std::make_unique<CBuffer<PerObjectData>>());

	std::shared_ptr<PixelShader> pixelShader = std::make_shared<PixelShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\shadow_demo_PS.cso")));
	pixelShader->AddConstantBuffer(CBufferFrequency::per_object, std::make_unique<CBuffer<PerObjectData>>());
	pixelShader->AddConstantBuffer(CBufferFrequency::per_frame, std::make_unique<CBuffer<PerFrameData>>());
	pixelShader->AddConstantBuffer(CBufferFrequency::rarely_changed, std::make_unique<CBuffer<RarelyChangedData>>());
	pixelShader->AddSampler(SamplerUsage::common_textures, std::make_shared<AnisotropicSampler>());
	pixelShader->AddSampler(SamplerUsage::shadow_map, std::make_shared<MinMagMipPointSampler>());
	pixelShader->AddSampler(SamplerUsage::projector, std::make_shared<ProjectorSampler>());

	m_renderPass.SetState(std::make_shared<RenderPassState>(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_viewport, std::make_shared<SolidCullBackRS>(), m_backBufferView.Get(), m_depthBufferView.Get()));
	m_renderPass.SetVertexShader(vertexShader);
	m_renderPass.SetPixelShader(pixelShader);
	m_renderPass.Init();

	m_firstRenderPass.SetState(std::make_shared<RenderPassState>(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_shadowMapViewport, std::make_shared<DepthBiasRS>(), nullptr, m_shadowMapDB.Get()));
	m_firstRenderPass.SetVertexShader(projectorVertexShader);
	m_firstRenderPass.Init();

	m_firstRenderPassProjector.SetState(std::make_shared<RenderPassState>(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_projectorViewport, std::make_shared<DepthBiasRS>(), nullptr, m_projectorDB.Get()));
	m_firstRenderPassProjector.SetVertexShader(shadowMapVertexShader);
	m_firstRenderPassProjector.Init();
}


void ShadowDemoApp::InitRenderables()
{
	{
		render::Renderable rocks_composition{ *(service::Locator::GetResourceLoader()->LoadGPFMesh(GetRootDir().append(LR"(\3d-objects\rocks_dorama\rocks_composition.gpf)"))) };
		rocks_composition.SetTransform(XMMatrixTranslation(0.f, 0.f, 0.f));
		rocks_composition.Init();
		m_objects.push_back(std::move(rocks_composition));
	}

	{
		render::Renderable gdc_female{ *(service::Locator::GetResourceLoader()->LoadGPFMesh(GetRootDir().append(LR"(\3d-objects\gdc_female\gdc_female_posed_2.gpf)"))) };
		XMMATRIX S = XMMatrixScaling(0.72f, 0.72f, 0.72f);
		XMMATRIX R = XMMatrixRotationY(math::ToRadians(-15.f));
		XMMATRIX T = XMMatrixTranslation(-3.0f, 0.4f, -3.0f);
		XMMATRIX W = T * R * S;
		gdc_female.SetTransform(W);
		gdc_female.Init();
		m_objects.push_back(std::move(gdc_female));
	}

	{
		render::Renderable gdc_female{ *(service::Locator::GetResourceLoader()->LoadGPFMesh(GetRootDir().append(LR"(\3d-objects\gdc_female\gdc_female_posed.gpf)"))) };
		XMMATRIX S = XMMatrixScaling(0.7f, 0.7f, 0.7f);
		XMMATRIX R = XMMatrixRotationY(math::ToRadians(135.f));
		XMMATRIX T = XMMatrixTranslation(4.0f, 0.2f, 13.f);
		XMMATRIX W =  T * R * S;
		gdc_female.SetTransform(W);
		gdc_female.Init();
		m_objects.push_back(std::move(gdc_female));
	}
}


void ShadowDemoApp::InitLights()
{
	m_dirKeyLight.ambient = { 0.16f, 0.18f, 0.18f, 1.f };
	m_dirKeyLight.diffuse = { 1.f* 0.78f, 1.f* 0.83f, 1.f* 1.f, 1.f };
	m_dirKeyLight.specular = { 0.87f,  0.90f,  0.94f, 1.f };
	XMVECTOR dirLightDirection = XMVector3Normalize(XMVectorSet(-0.6f, -0.3f, -0.4f, 0.f));
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

	m_spotLight.ambient = { 0.018f, 0.018f, 1.8f, 1.0f };
	m_spotLight.diffuse = { 0.1f, 0.1f, 1.0f, 1.0f };
	m_spotLight.specular = { 0.1f, 0.1f, 1.0f, 1.0f };
	XMVECTOR posW = XMVectorSet(5.f, 5.f, -5.f, 1.f);
	XMStoreFloat3(&m_spotLight.posW, posW);
	m_spotLight.range = 50.f;
	XMVECTOR dirW = XMVector3Normalize(XMVectorSet(-4.f, 1.f, 0.f, 1.f) - posW);
	XMStoreFloat3(&m_spotLight.dirW, dirW);
	m_spotLight.spot = 30.f;
	m_spotLight.attenuation = { 0.0f, 0.1f, 0.f };

	m_lightingControls.useDirLight = true;
	m_lightingControls.usePointLight = false;
	m_lightingControls.useSpotLight = true;
	m_lightingControls.useBumpMap = true;

	// Shadow Map
	m_LightViewMatrix = XMMatrixLookAtLH(
		XMVectorSet(-m_dirKeyLight.dirW.x, -m_dirKeyLight.dirW.y, -m_dirKeyLight.dirW.z, 1.0f),
		XMVectorSet(m_bSphere.Center.x, m_bSphere.Center.y, m_bSphere.Center.z, 1.f),
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
	);

	m_LightProjectionMatrix = XMMatrixOrthographicOffCenterLH(
		m_bSphere.Center.x - m_bSphere.Radius,
		m_bSphere.Center.x + m_bSphere.Radius,
		m_bSphere.Center.y - m_bSphere.Radius,
		m_bSphere.Center.y + m_bSphere.Radius,
		m_bSphere.Center.z - m_bSphere.Radius,
		m_bSphere.Center.z + m_bSphere.Radius
	);

	m_T = {
		XMVectorSet(0.5f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, -0.5f, 0.0f, 0.0f),
		XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
		XMVectorSet(0.5f, 0.5f, 0.0f, 1.0f)
	};

	// Projector
	m_SpotLightViewMatrix = XMMatrixLookAtLH(
		XMVectorSet(m_spotLight.posW.x, m_spotLight.posW.x, m_spotLight.posW.z, 1.0f),
		XMVectorSet(m_bSphere.Center.x, m_bSphere.Center.y, m_bSphere.Center.z, 1.f),
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
	);

	m_SpotLightProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(math::ToRadians(45.f), AspectRatio(), 1.f, 1000.f);
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

	else if (key == input::Key::F4 && status.isDown)
	{
		m_lightingControls.useSpotLight = !m_lightingControls.useSpotLight;
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

		PerFrameData pfdata;
		pfdata.dirLights[0] = m_dirKeyLight;
		pfdata.dirLights[1] = m_dirFillLight;
		
		pfdata.pointLights[0] = pfdata.pointLights[1] = pfdata.pointLights[2] = pfdata.pointLights[3] = m_pointLight;
		pfdata.pointLights[0].posW = tmp[0];
		pfdata.pointLights[1].posW = tmp[1];
		pfdata.pointLights[2].posW = tmp[2];
		pfdata.pointLights[3].posW = tmp[3];
		
		pfdata.eyePosW = m_camera.GetPosition();

		m_renderPass.GetPixelShader()->GetConstantBuffer(CBufferFrequency::per_frame)->UpdateBuffer(pfdata);
	}

	// RarelyChangedCB
	if (m_isLightingControlsDirty)
	{
		m_renderPass.GetPixelShader()->GetConstantBuffer(CBufferFrequency::rarely_changed)->UpdateBuffer(m_lightingControls);
		m_isLightingControlsDirty = false;
	}

}


void ShadowDemoApp::RenderScene()
{
	m_d3dAnnotation->BeginEvent(L"render-scene");

	// First pass
	m_firstRenderPass.Bind();
	m_firstRenderPass.GetState()->ClearDepthOnly();

	// draw objects
	for (render::Renderable& renderable : m_objects)
	{
		for (const std::string& meshName : renderable.GetMeshNames())
		{
			PerObjectData data = ToPerObjectData_ShadowMap(renderable, meshName);
			m_firstRenderPass.GetVertexShader()->GetConstantBuffer(CBufferFrequency::per_object)->UpdateBuffer(data);
			renderable.Draw(meshName);
		}
	}
	
	// First pass projector
	m_firstRenderPassProjector.Bind();
	m_firstRenderPassProjector.GetState()->ClearDepthOnly();

	// draw objects
	for (render::Renderable& renderable : m_objects)
	{
		for (const std::string& meshName : renderable.GetMeshNames())
		{
			PerObjectData data = ToPerObjectData_Projector(renderable, meshName);
			m_firstRenderPassProjector.GetVertexShader()->GetConstantBuffer(CBufferFrequency::per_object)->UpdateBuffer(data);
			renderable.Draw(meshName);
		}
	}
	
	// Second Pass
	m_renderPass.Bind();
	m_renderPass.GetState()->ClearDepthOnly();
	m_renderPass.GetState()->ClearRenderTarget(DirectX::Colors::DarkGray);

	// draw objects
	m_renderPass.GetPixelShader()->BindTexture(TextureUsage::shadow_map, m_shadowMapPS.Get());
	m_renderPass.GetPixelShader()->BindTexture(TextureUsage::projector, m_projectorPS.Get());
	m_renderPass.GetPixelShader()->BindTexture(TextureUsage::projectorTexture, m_projectorTexturePS.Get());
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
	XMMATRIX WVP = W * V * P;
	XMMATRIX WVPT = W * m_LightViewMatrix * m_LightProjectionMatrix * m_T;
	XMMATRIX WVPT_projector = W * m_SpotLightViewMatrix * m_SpotLightProjectionMatrix * m_T;

	XMStoreFloat4x4(&data.W, XMMatrixTranspose(W));
	XMStoreFloat4x4(&data.WVP, XMMatrixTranspose(WVP));
	XMStoreFloat4x4(&data.W_inverseTraspose, XMMatrixInverse(nullptr, W));
	XMStoreFloat4x4(&data.TexcoordMatrix, XMMatrixTranspose(T));
	XMStoreFloat4x4(&data.WVPT_shadowMap, XMMatrixTranspose(WVPT));
	XMStoreFloat4x4(&data.WVPT_projector, XMMatrixTranspose(WVPT_projector));
	data.material.ambient = renderable.GetMaterial(meshName).ambient;
	data.material.diffuse = renderable.GetMaterial(meshName).diffuse;
	data.material.specular = renderable.GetMaterial(meshName).specular;

	return data;
}

ShadowDemoApp::PerObjectData ShadowDemoApp::ToPerObjectData_ShadowMap(const render::Renderable& renderable, const std::string& meshName) const
{
	PerObjectData data;

	XMMATRIX W = XMLoadFloat4x4(&renderable.GetTransform());
	XMMATRIX V = m_LightViewMatrix;
	XMMATRIX P = m_LightProjectionMatrix;
	XMMATRIX WVP = W * V * P;

	XMStoreFloat4x4(&data.W, XMMatrixTranspose(W));
	XMStoreFloat4x4(&data.WVP, XMMatrixTranspose(WVP));
	XMStoreFloat4x4(&data.W_inverseTraspose, XMMatrixInverse(nullptr, W));

	return data;
}

ShadowDemoApp::PerObjectData ShadowDemoApp::ToPerObjectData_Projector(const render::Renderable& renderable, const std::string& meshName) const
{
	PerObjectData data;

	XMMATRIX W = XMLoadFloat4x4(&renderable.GetTransform());
	XMMATRIX V = m_SpotLightViewMatrix;
	XMMATRIX P = m_SpotLightProjectionMatrix;
	XMMATRIX WVP = W * V * P;

	XMStoreFloat4x4(&data.W, XMMatrixTranspose(W));
	XMStoreFloat4x4(&data.WVP, XMMatrixTranspose(WVP));
	XMStoreFloat4x4(&data.W_inverseTraspose, XMMatrixInverse(nullptr, W));

	return data;
}
