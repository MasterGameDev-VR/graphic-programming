#include "stdafx.h"
#include "alpha_demo_app.h"
#include <file/file_utils.h>
#include <math/math_utils.h>
#include <service/locator.h>
#include <render/shading/vertex_input_types.h>
#include <render/shading/rasterizer_state_types.h>
#include <render/shading/sampler_types.h>
#include <external_libs/directxtk/WICTextureLoader.h>
#include <alpha/alpha_vertex_input_types.h>
#include <external_libs/directxtk/WICTextureLoader.h>


using namespace DirectX;
using namespace xtest;
using namespace xtest::render::shading;
using namespace xtest::demo;
using Microsoft::WRL::ComPtr;


AlphaDemoApp::AlphaDemoApp(HINSTANCE instance,
	const application::WindowSettings& windowSettings,
	const application::DirectxSettings& directxSettings,
	uint32 fps /*=60*/)
	: application::DirectxApp(instance, windowSettings, directxSettings, fps)
	, m_dirKeyLight()
	, m_dirFillLight()
	, m_rarelyChangedData()
	, m_isRarelyChangedDataDirty(true)
	, m_camera(math::ToRadians(80.f), math::ToRadians(170.f), 15.f, { 5.f, 4.f, -5.f }, { 0.f, 1.f, 0.f }, { math::ToRadians(4.f), math::ToRadians(175.f) }, { 3.f, 50.f })
	, m_objects()
	, m_shadowPass()
	, m_renderPass()
	, m_shadowMap(2048)
	, m_sceneBoundingSphere({ 0.f, 0.f, 0.f }, 21.f)
	, m_textureRenderable()
	, m_glowObjectsMap()
{}


AlphaDemoApp::~AlphaDemoApp()
{}


void AlphaDemoApp::Init()
{
	application::DirectxApp::Init();

	m_camera.SetPerspectiveProjection(math::ToRadians(45.f), AspectRatio(), 1.f, 1000.f);

	InitLights();
	InitRenderTechnique();
	InitRenderables();
	InitRenderToTexture();
	InitGlowMap();

	service::Locator::GetMouse()->AddListener(this);
	service::Locator::GetKeyboard()->AddListener(this, { input::Key::F, input::Key::F1 });
}


void AlphaDemoApp::InitLights()
{
	m_dirKeyLight.ambient = { 0.16f, 0.18f, 0.18f, 1.f };
	m_dirKeyLight.diffuse = { 0.8f ,  0.8f,  0.7f, 1.f };
	m_dirKeyLight.specular = { 0.8f ,  0.8f,  0.7f, 1.f };
	XMStoreFloat3(&m_dirKeyLight.dirW, XMVector3Normalize(-XMVectorSet(0.917053342f, 0.390566736f, 0.0802310705f, 0.f)));

	m_dirFillLight.ambient = { 0.01f * 0.16f , 0.01f * 0.18f, 0.005f * 0.18f, 1.f };
	m_dirFillLight.diffuse = { 0.02f * 1.f   , 0.01f * 0.91f, 0.005f * 0.85f, 1.f };
	m_dirFillLight.specular = { 0.01f * 0.087f, 0.01f * 0.09f, 0.005f * 0.094f, 1.f };
	XMStoreFloat3(&m_dirFillLight.dirW, XMVector3Transform(XMLoadFloat3(&m_dirKeyLight.dirW), XMMatrixRotationY(math::ToRadians(45.f))));

	m_rarelyChangedData.useShadowMap = true;
}


void AlphaDemoApp::InitRenderTechnique()
{
	file::ResourceLoader* loader = service::Locator::GetResourceLoader();


	// shadow pass
	{
		m_shadowMap.SetTargetBoundingSphere(m_sceneBoundingSphere);
		m_shadowMap.SetLight(m_dirKeyLight.dirW);
		m_shadowMap.Init();

		m_rarelyChangedData.shadowMapResolution = float(m_shadowMap.Resolution());

		std::shared_ptr<VertexShader> vertexShader = std::make_shared<VertexShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\alpha_demo_shadowmap_VS.cso")));
		vertexShader->SetVertexInput(std::make_shared<PosOnlyVertexInput>());
		vertexShader->AddConstantBuffer(CBufferFrequency::per_object, std::make_unique<CBuffer<PerObjectShadowMapData>>());

		m_shadowPass.SetState(std::make_shared<RenderPassState>(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_shadowMap.Viewport(), std::make_shared<SolidCullBackDepthBiasRS>(), nullptr, m_shadowMap.AsDepthStencilView()));
		m_shadowPass.SetVertexShader(vertexShader);
		m_shadowPass.Init();
	}


	// render pass
	{
		m_sceneTexture.Init(GetCurrentWidth(), GetCurrentHeight());

		std::shared_ptr<VertexShader> vertexShader = std::make_shared<VertexShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\alpha_demo_VS.cso")));
		vertexShader->SetVertexInput(std::make_shared<MeshDataVertexInput>());
		vertexShader->AddConstantBuffer(CBufferFrequency::per_object, std::make_unique<CBuffer<PerObjectData>>());

		std::shared_ptr<PixelShader> pixelShader = std::make_shared<PixelShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\alpha_demo_PS.cso")));
		pixelShader->AddConstantBuffer(CBufferFrequency::per_object, std::make_unique<CBuffer<PerObjectData>>());
		pixelShader->AddConstantBuffer(CBufferFrequency::per_frame, std::make_unique<CBuffer<PerFrameData>>());
		pixelShader->AddConstantBuffer(CBufferFrequency::rarely_changed, std::make_unique<CBuffer<RarelyChangedData>>());
		pixelShader->AddSampler(SamplerUsage::common_textures, std::make_shared<AnisotropicSampler>());
		pixelShader->AddSampler(SamplerUsage::shadow_map, std::make_shared<PCFSampler>());

		m_renderPass.SetState(std::make_shared<RenderPassState>(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_viewport, std::make_shared<SolidCullBackRS>(), m_sceneTexture.AsRenderTargetView(), m_depthBufferView.Get()));
		m_renderPass.SetVertexShader(vertexShader);
		m_renderPass.SetPixelShader(pixelShader);
		m_renderPass.Init();
	}

	// glow pass
	{
		m_glowmap.Init(GetCurrentWidth(), GetCurrentHeight());

		std::shared_ptr<VertexShader> vertexShader = std::make_shared<VertexShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\alpha_demo_glow_VS.cso")));
		vertexShader->SetVertexInput(std::make_shared<MeshDataVertexInput>());
		vertexShader->AddConstantBuffer(CBufferFrequency::per_object, std::make_unique<CBuffer<PerObjectGlowData>>());

		std::shared_ptr<PixelShader> pixelShader = std::make_shared<PixelShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\alpha_demo_glow_PS.cso")));
		pixelShader->AddConstantBuffer(CBufferFrequency::per_object, std::make_unique<CBuffer<PerObjectGlowData>>());
		pixelShader->AddSampler(SamplerUsage::common_textures, std::make_shared<AnisotropicSampler>());

		m_glowPass.SetState(std::make_shared<RenderPassState>(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_viewport, std::make_shared<SolidCullBackRS>(), m_glowmap.AsRenderTargetView(), m_depthBufferView.Get()));
		m_glowPass.SetVertexShader(vertexShader);
		m_glowPass.SetPixelShader(pixelShader);(std::make_shared<RenderPassState>(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_viewport, std::make_shared<SolidCullBackRS>(), m_sceneTexture.AsRenderTargetView(), m_depthBufferView.Get()));
		m_glowPass.Init();
	}

	// Horizontal blur pass
	{
		m_horizontalBlurTexture.Init(GetCurrentWidth(), GetCurrentHeight());

		std::shared_ptr<VertexShader> vertexShader = std::make_shared<VertexShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\alpha_demo_postprocessing_VS.cso")));
		vertexShader->SetVertexInput(std::make_shared<alpha::TextureDataVertexInput>());
		
		std::shared_ptr<PixelShader> pixelShader = std::make_shared<PixelShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\alpha_demo_horizontal_blur_PS.cso")));
		pixelShader->AddConstantBuffer(CBufferFrequency::per_frame, std::make_unique<CBuffer<PerFrameBlurData>>());

		m_horizontalBlurPass.SetState(std::make_shared<RenderPassState>(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_viewport, std::make_shared<SolidCullBackRS>(), m_horizontalBlurTexture.AsRenderTargetView(), m_depthBufferView.Get()));
		m_horizontalBlurPass.SetVertexShader(vertexShader);
		m_horizontalBlurPass.SetPixelShader(pixelShader);
		m_horizontalBlurPass.Init();
	}

	// postprocessing pass
	{
		std::shared_ptr<VertexShader> vertexShader = std::make_shared<VertexShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\alpha_demo_postprocessing_VS.cso")));
		vertexShader->SetVertexInput(std::make_shared<alpha::TextureDataVertexInput>());

		std::shared_ptr<PixelShader> pixelShader = std::make_shared<PixelShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\alpha_demo_postprocessing_PS.cso")));

		m_PostPass.SetState(std::make_shared<RenderPassState>(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_viewport, std::make_shared<SolidCullBackRS>(), m_backBufferView.Get(), m_depthBufferView.Get()));
		m_PostPass.SetVertexShader(vertexShader);
		m_PostPass.SetPixelShader(pixelShader);
		m_PostPass.Init();
	}
}


void AlphaDemoApp::InitRenderables()
{
	render::Renderable ground{ *(service::Locator::GetResourceLoader()->LoadGPFMesh(GetRootDir().append(LR"(\3d-objects\rocks_dorama\rocks_composition.gpf)"))) };
	ground.SetTransform(XMMatrixScaling(1.5f, 1.5f, 1.5f) * XMMatrixTranslation(3.f, 0.f, 2.5f));
	ground.Init();
	m_objects.push_back(ground);

	render::Renderable soldier1{ *(service::Locator::GetResourceLoader()->LoadGPFMesh(GetRootDir().append(LR"(\3d-objects\gdc_female\gdc_female_posed_2.gpf)"))) };
	soldier1.SetTransform(XMMatrixRotationY(math::ToRadians(-12.f)) * XMMatrixTranslation(0.f, 0.4f, 0.f));
	soldier1.Init();
	m_objects.push_back(std::move(soldier1));

	render::Renderable soldier2{ *(service::Locator::GetResourceLoader()->LoadGPFMesh(GetRootDir().append(LR"(\3d-objects\gdc_female\gdc_female_posed.gpf)"))) };
	soldier2.SetTransform(XMMatrixRotationY(math::ToRadians(135.f)) * XMMatrixTranslation(10.f, 0.35f, -10.f));
	soldier2.Init();
	m_objects.push_back(std::move(soldier2));
}

void AlphaDemoApp::InitRenderToTexture()
{
	m_textureRenderable.Init();
}		

void AlphaDemoApp::InitGlowMap()
{
	//init of the glowmap for the head of the character
	Microsoft::WRL::ComPtr<ID3D11Resource> headGlowTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> headGlowTextureView;
	XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(xtest::service::Locator::GetD3DDevice(), xtest::service::Locator::GetD3DContext(), 
		GetRootDir().append(LR"(\3d-objects\gdc_female\textures\head_glow.png)").c_str(), headGlowTexture.GetAddressOf(), headGlowTextureView.GetAddressOf()));

	//INIT OF GLOWOBJECTS
	{
		std::pair<render::Renderable*, std::string> glowObjectKey;
		alpha::GlowObject glowObject;
		glowObjectKey.first = &m_objects[1];
		glowObjectKey.second = "head";
		glowObject.glowTexture = headGlowTexture;
		glowObject.glowTextureView = headGlowTextureView;

		m_glowObjectsMap.emplace(glowObjectKey, glowObject);
	}

	{
		std::pair<render::Renderable*, std::string> glowObjectKey;
		alpha::GlowObject glowObject;
		glowObjectKey.first = &m_objects[2];
		glowObjectKey.second = "head";
		glowObject.glowTexture = headGlowTexture;
		glowObject.glowTextureView = headGlowTextureView;

		m_glowObjectsMap.emplace(glowObjectKey, glowObject);
	}
}

void AlphaDemoApp::OnResized()
{
	application::DirectxApp::OnResized();

	m_sceneTexture.Resize(GetCurrentWidth(), GetCurrentHeight());
	//update the render pass state with the resized render target and depth buffer
	m_renderPass.GetState()->ChangeRenderTargetView(m_sceneTexture.AsRenderTargetView());
	m_renderPass.GetState()->ChangeDepthStencilView(m_depthBufferView.Get());
	m_renderPass.GetState()->ChangeViewPort(m_viewport);

	m_glowmap.Resize(GetCurrentWidth(), GetCurrentHeight());
	//update the glow pass state with the resized render target and depth buffer
	m_glowPass.GetState()->ChangeRenderTargetView(m_glowmap.AsRenderTargetView());
	m_glowPass.GetState()->ChangeDepthStencilView(m_depthBufferView.Get());
	m_glowPass.GetState()->ChangeViewPort(m_viewport);

	//update the post pass state with the resized render target and depth buffer
	m_PostPass.GetState()->ChangeRenderTargetView(m_backBufferView.Get());
	m_PostPass.GetState()->ChangeDepthStencilView(m_depthBufferView.Get());
	m_PostPass.GetState()->ChangeViewPort(m_viewport);

	//update the projection matrix with the new aspect ratio
	m_camera.SetPerspectiveProjection(math::ToRadians(45.f), AspectRatio(), 1.f, 1000.f);
}


void AlphaDemoApp::OnWheelScroll(input::ScrollStatus scroll)
{
	// zoom in or out when the scroll wheel is used
	if (service::Locator::GetMouse()->IsInClientArea())
	{
		m_camera.IncreaseRadiusBy(scroll.isScrollingUp ? -0.5f : 0.5f);
	}
}


void AlphaDemoApp::OnMouseMove(const DirectX::XMINT2& movement, const DirectX::XMINT2& currentPos)
{
	XTEST_UNUSED_VAR(currentPos);

	input::Mouse* mouse = service::Locator::GetMouse();

	// rotate the camera position around 
	if (mouse->GetButtonStatus(input::MouseButton::left_button).isDown && mouse->IsInClientArea())
	{
		m_camera.RotateBy(math::ToRadians(movement.y * -0.25f), math::ToRadians(movement.x * 0.25f));
	}

	// pan the camera 
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

	// rotate the directional light
	if (mouse->GetButtonStatus(input::MouseButton::middle_button).isDown && mouse->IsInClientArea())
	{
		XMStoreFloat3(&m_dirKeyLight.dirW, XMVector3Rotate(XMLoadFloat3(&m_dirKeyLight.dirW), XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), math::ToRadians(movement.x * -0.25f))));
		XMStoreFloat3(&m_dirFillLight.dirW, XMVector3Rotate(XMLoadFloat3(&m_dirFillLight.dirW), XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), math::ToRadians(movement.x * -0.25f))));

		m_shadowMap.SetLight(m_dirKeyLight.dirW);
	}

}


void AlphaDemoApp::OnKeyStatusChange(input::Key key, const input::KeyStatus& status)
{
	if (key == input::Key::F && status.isDown)
	{
		m_camera.SetPivot({ 5.f, 4.f, -5.f });
	}
	else if (key == input::Key::F1 && status.isDown)
	{
		m_rarelyChangedData.useShadowMap = !m_rarelyChangedData.useShadowMap;
		m_isRarelyChangedDataDirty = true;
	}
}


void AlphaDemoApp::UpdateScene(float deltaSeconds)
{
	XTEST_UNUSED_VAR(deltaSeconds);

	// PerFrameCB
	{
		PerFrameData data;
		data.dirLights[0] = m_dirKeyLight;
		data.dirLights[1] = m_dirFillLight;
		data.eyePosW = m_camera.GetPosition();

		m_renderPass.GetPixelShader()->GetConstantBuffer(CBufferFrequency::per_frame)->UpdateBuffer(data);
	}


	// RarelyChangedCB
	if (m_isRarelyChangedDataDirty)
	{
		m_renderPass.GetPixelShader()->GetConstantBuffer(CBufferFrequency::rarely_changed)->UpdateBuffer(m_rarelyChangedData);
		m_isRarelyChangedDataDirty = false;
	}

}


void AlphaDemoApp::RenderScene()
{
	// draw shawdowmap
	m_d3dAnnotation->BeginEvent(L"shadow-map");
	m_shadowPass.Bind();
	m_shadowPass.GetState()->ClearDepthOnly();

	
	for (render::Renderable& renderable : m_objects)
	{
		for (const std::string& meshName : renderable.GetMeshNames())
		{
			PerObjectShadowMapData data = ToPerObjectShadowMapData(renderable, meshName);
			m_shadowPass.GetVertexShader()->GetConstantBuffer(CBufferFrequency::per_object)->UpdateBuffer(data);
			renderable.Draw(meshName);
		}
	}
	m_d3dAnnotation->EndEvent();


	// draw objects to texture
	m_d3dAnnotation->BeginEvent(L"render-scene");
	m_renderPass.Bind();
	m_renderPass.GetState()->ClearDepthOnly();
	m_renderPass.GetState()->ClearRenderTarget(DirectX::Colors::SkyBlue);
	m_renderPass.GetPixelShader()->BindTexture(TextureUsage::shadow_map, m_shadowMap.AsShaderView());

	
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
	m_renderPass.GetPixelShader()->BindTexture(TextureUsage::shadow_map, nullptr); // explicit unbind the shadow map to suppress warning
	m_d3dAnnotation->EndEvent();


	//Drawing the glowing objects on texture
	m_d3dAnnotation->BeginEvent(L"render-glowmap");
	m_glowPass.Bind();
	m_glowPass.GetState()->ClearDepthOnly();
	m_glowPass.GetState()->ClearRenderTarget(DirectX::Colors::Transparent);

	for (render::Renderable& renderable : m_objects)
	{
		for (const std::string& meshName : renderable.GetMeshNames())
		{
			PerObjectGlowData data = ToPerObjectGlowData(renderable, meshName);
			m_glowPass.GetVertexShader()->GetConstantBuffer(CBufferFrequency::per_object)->UpdateBuffer(data);
			m_glowPass.GetPixelShader()->GetConstantBuffer(CBufferFrequency::per_object)->UpdateBuffer(data);

			if (data.useGlow)
			{
				GlowObjectKey glowObjectKey (&renderable, meshName);
				const alpha::GlowObject glowObject = m_glowObjectsMap.at(glowObjectKey);

				m_glowPass.GetPixelShader()->BindTexture(TextureUsage::color, renderable.GetTextureView(TextureUsage::color, meshName));
				m_glowPass.GetPixelShader()->BindTexture(TextureUsage::glow_map, glowObject.glowTextureView.Get());
			}

			renderable.Draw(meshName);
		}
	}
	m_d3dAnnotation->EndEvent();
	
	// Drawing horizontal blur
	m_d3dAnnotation->BeginEvent(L"horizontal-blur");
	m_horizontalBlurPass.Bind();
	m_horizontalBlurPass.GetState()->ClearDepthOnly();
	m_horizontalBlurPass.GetState()->ClearRenderTarget(DirectX::Colors::SkyBlue);
	m_horizontalBlurPass.GetPixelShader()->BindTexture(TextureUsage::blur, m_glowmap.AsShaderView());

	PerFrameBlurData frameData;
	frameData.resolution = GetCurrentWidth();
	m_horizontalBlurPass.GetPixelShader()->GetConstantBuffer(CBufferFrequency::per_frame)->UpdateBuffer(frameData);

	m_textureRenderable.Draw();

	m_PostPass.GetPixelShader()->BindTexture(TextureUsage::blur, nullptr);
	m_d3dAnnotation->EndEvent();
	   
	//Drawing all the textures together
	m_d3dAnnotation->BeginEvent(L"render-from-texture");
	m_PostPass.Bind();
	m_PostPass.GetState()->ClearDepthOnly();
	m_PostPass.GetState()->ClearRenderTarget(DirectX::Colors::SkyBlue);
	m_PostPass.GetPixelShader()->BindTexture(TextureUsage::texture_map, m_sceneTexture.AsShaderView());
	m_PostPass.GetPixelShader()->BindTexture(TextureUsage::bloom, m_horizontalBlurTexture.AsShaderView());
	   
	m_textureRenderable.Draw();

	m_PostPass.GetPixelShader()->BindTexture(TextureUsage::bloom, nullptr); // explicit unbind bloom to suppress warning
	m_PostPass.GetPixelShader()->BindTexture(TextureUsage::texture_map, nullptr);
	m_d3dAnnotation->EndEvent();

	XTEST_D3D_CHECK(m_swapChain->Present(0, 0));
}


AlphaDemoApp::PerObjectData AlphaDemoApp::ToPerObjectData(const render::Renderable& renderable, const std::string& meshName)
{
	PerObjectData data;

	XMMATRIX W = XMLoadFloat4x4(&renderable.GetTransform());
	XMMATRIX T = XMLoadFloat4x4(&renderable.GetTexcoordTransform(meshName));
	XMMATRIX V = m_camera.GetViewMatrix();
	XMMATRIX P = m_camera.GetProjectionMatrix();
	XMMATRIX WVP = W * V*P;
	XMMATRIX WVPT_shadowMap = W * m_shadowMap.VPTMatrix();

	XMStoreFloat4x4(&data.W, XMMatrixTranspose(W));
	XMStoreFloat4x4(&data.WVP, XMMatrixTranspose(WVP));
	XMStoreFloat4x4(&data.W_inverseTraspose, XMMatrixInverse(nullptr, W));
	XMStoreFloat4x4(&data.TexcoordMatrix, XMMatrixTranspose(T));
	XMStoreFloat4x4(&data.WVPT_shadowMap, XMMatrixTranspose(WVPT_shadowMap));
	data.material.ambient = renderable.GetMaterial(meshName).ambient;
	data.material.diffuse = renderable.GetMaterial(meshName).diffuse;
	data.material.specular = renderable.GetMaterial(meshName).specular;

	return data;
}


AlphaDemoApp::PerObjectShadowMapData AlphaDemoApp::ToPerObjectShadowMapData(const render::Renderable& renderable, const std::string& meshName)
{
	XTEST_UNUSED_VAR(meshName);
	PerObjectShadowMapData data;

	XMMATRIX W = XMLoadFloat4x4(&renderable.GetTransform());
	XMMATRIX WVP = W * m_shadowMap.LightViewMatrix() * m_shadowMap.LightProjMatrix();

	XMStoreFloat4x4(&data.WVP_lightSpace, XMMatrixTranspose(WVP));
	return data;
}

//Returns the data for the glow, and the useglow flag as true if the object is inside the glowobject map
AlphaDemoApp::PerObjectGlowData AlphaDemoApp::ToPerObjectGlowData(const render::Renderable& renderable, const std::string& meshName)
{
	PerObjectGlowData data;

	XMMATRIX W = XMLoadFloat4x4(&renderable.GetTransform());
	XMMATRIX T = XMLoadFloat4x4(&renderable.GetTexcoordTransform(meshName));
	XMMATRIX V = m_camera.GetViewMatrix();
	XMMATRIX P = m_camera.GetProjectionMatrix();
	XMMATRIX WVP = W * V*P;

	XMStoreFloat4x4(&data.WVP, XMMatrixTranspose(WVP));
	XMStoreFloat4x4(&data.TexcoordMatrix, XMMatrixTranspose(T));

	GlowObjectKey glowObjectKey(&renderable, meshName);

	data.useGlow = m_glowObjectsMap.count(glowObjectKey) > 0;


	return data;
}

