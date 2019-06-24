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
#include <alpha/alpha_sampler_types.h>


using namespace DirectX;
using namespace xtest;
using namespace xtest::render::shading;
using namespace xtest::demo;
using namespace xtest::render::shading;
using namespace xtest::file;
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
	, m_scatteringPass()
	, m_renderPass()
	, m_shadowMap(2048)
	, m_lightOcclusionMap(2048)
	, m_motionBlurMap()
	, m_colorRenderMap()
	, m_sceneBoundingSphere({ 0.f, 0.f, 0.f }, 21.f)
	, targetFPS(fps)
	, totalTime(0.0f)
	, m_textureRenderable()
	, m_glowObjectsMap()
	, m_stop(false)
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
	service::Locator::GetKeyboard()->AddListener(this, { 
		input::Key::F, input::Key::F1, input::Key::F2, input::Key::F3, input::Key::F4, input::Key::space_bar 
	});
}

void AlphaDemoApp::InitRenderables()
{
	render::Renderable ground{ *(service::Locator::GetResourceLoader()->LoadGPFMesh(GetRootDir().append(LR"(\3d-objects\rocks_dorama\rocks_composition.gpf)"))) };
	ground.SetTransform(XMMatrixScaling(1.5f, 1.5f, 1.5f) * XMMatrixTranslation(3.f, 0.f, 2.5f));
	ground.Init();
	previous_Transforms.push_back(ground.GetTransform());
	m_objects.push_back(std::move(ground));

	render::Renderable soldier1{ *(service::Locator::GetResourceLoader()->LoadGPFMesh(GetRootDir().append(LR"(\3d-objects\gdc_female\gdc_female_posed_2.gpf)"))) };
	soldier1.SetTransform(XMMatrixRotationY(math::ToRadians(-12.f)) * XMMatrixTranslation(0.f, 0.4f, 0.f));
	soldier1.Init();
	previous_Transforms.push_back(soldier1.GetTransform());
	m_objects.push_back(std::move(soldier1));

	render::Renderable soldier2{ *(service::Locator::GetResourceLoader()->LoadGPFMesh(GetRootDir().append(LR"(\3d-objects\gdc_female\gdc_female_posed.gpf)"))) };
	soldier2.SetTransform(XMMatrixRotationY(math::ToRadians(135.f)) * XMMatrixTranslation(10.f, 0.35f, -10.f));
	soldier2.Init();
	previous_Transforms.push_back(soldier2.GetTransform());
	m_objects.push_back(std::move(soldier2));

	render::Renderable crate{ *(service::Locator::GetResourceLoader()->LoadGPFMesh(GetRootDir().append(LR"(\3d-objects\crate\crate.gpf)"))) };
	crate.SetTransform(XMMatrixTranslation(-3.0f, 0.5f, -5.0f));
	crate.Init();
	previous_Transforms.push_back(crate.GetTransform());
	backupWCrate = XMLoadFloat4x4(&crate.GetTransform());
	m_objects.push_back(std::move(crate));


	//SPHERE
	{
		mesh::MeshMaterial mat;
		mat.ambient = { 0.8f, 0.8f, 0.8f, 1.f };
		mat.diffuse = { 0.52f, 0.52f, 0.48f, 1.f };
		mat.specular = { 0.5f, 0.5f, 0.5f, 1.f };
		mat.diffuseMap = GetRootDir().append(LR"(\3d-objects\plastic-cover\plastic_cover_color.png)");
		mat.normalMap = GetRootDir().append(LR"(\3d-objects\plastic-cover\plastic_cover_norm.png)");
		mat.glossMap = GetRootDir().append(LR"(\3d-objects\plastic-cover\plastic_cover_gloss.png)");

		render::Renderable sphere(mesh::GenerateSphere(3.f, 30, 30), mat);
		sphere.SetTransform(XMMatrixScaling(0.2f, 0.2f, 0.2f) * XMMatrixTranslation(3.75f, 7.5f, -3.f));
		sphere.SetTexcoordTransform(XMMatrixIdentity());
		sphere.Init();
		previous_Transforms.push_back(sphere.GetTransform());
		m_objects.push_back(sphere);
	}

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
	m_rarelyChangedData.useLightScattering = true;
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

	// light scattering - occlusion pre-pass
	{
		m_lightOcclusionMap.SetTargetBoundingSphere(m_sceneBoundingSphere);
		m_lightOcclusionMap.SetLight(m_dirKeyLight.dirW);
		m_lightOcclusionMap.Init();

		m_rarelyChangedData.lightOcclusionMapResolution = float(m_lightOcclusionMap.Resolution());

		std::shared_ptr<VertexShader> vertexShader = std::make_shared<VertexShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\scattering_demo_lightocclusionmap_VS.cso")));
		vertexShader->SetVertexInput(std::make_shared<MeshDataVertexInput>());
		vertexShader->AddConstantBuffer(CBufferFrequency::per_object, std::make_unique<CBuffer<PerObjectLightScatteringData>>());

		std::shared_ptr<PixelShader> pixelShader = std::make_shared<PixelShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\scattering_demo_lightocclusionmap_PS.cso")));
		pixelShader->AddConstantBuffer(CBufferFrequency::per_object, std::make_unique<CBuffer<PerObjectLightScatteringData>>());

		m_scatteringPass.SetState(std::make_shared<RenderPassState>(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_lightOcclusionMap.Viewport(), std::make_shared<SolidCullBackRS>(), m_lightOcclusionMap.AsRenderTargetView(), m_lightOcclusionMap.DepthBufferView()));
		m_scatteringPass.SetVertexShader(vertexShader);
		m_scatteringPass.SetPixelShader(pixelShader);
		m_scatteringPass.Init();
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
		pixelShader->AddSampler(SamplerUsage::light_occlusion_map, std::make_shared<NoFilterSampler>());

		m_renderPass.SetState(std::make_shared<RenderPassState>(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_viewport, std::make_shared<SolidCullBackRS>(), m_sceneTexture.AsRenderTargetView(), m_depthBufferView.Get()));
		m_renderPass.SetVertexShader(vertexShader);
		m_renderPass.SetPixelShader(pixelShader);
		m_renderPass.Init();
	}

	{
		m_colorRenderMap.Init(GetCurrentWidth(), GetCurrentHeight());
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
		m_glowPass.SetPixelShader(pixelShader);
		m_glowPass.Init();

		m_rarelyChangedData.useGlowMap = true;
	}

	// downsample pass
	{
		m_downsampledGlowTexture.Init(GetCurrentWidth() / 2, GetCurrentHeight() / 2);
		m_downViewport.TopLeftX = 0.f;
		m_downViewport.TopLeftY = 0.f;
		m_downViewport.Width = static_cast<float>(GetCurrentWidth() / 2);
		m_downViewport.Height = static_cast<float>(GetCurrentHeight() / 2);
		m_downViewport.MinDepth = 0.f;
		m_downViewport.MaxDepth = 1.f;

		CreateDownDepthStencilBuffer();

		std::shared_ptr<VertexShader> vertexShader = std::make_shared<VertexShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\alpha_demo_postprocessing_VS.cso")));
		vertexShader->SetVertexInput(std::make_shared<alpha::TextureDataVertexInput>());

		std::shared_ptr<PixelShader> pixelShader = std::make_shared<PixelShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\alpha_demo_sample_PS.cso")));

		m_downPass.SetState(std::make_shared<RenderPassState>(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_downViewport, std::make_shared<SolidCullBackRS>(), m_downsampledGlowTexture.AsRenderTargetView(), m_depthStencilViewDownsample.Get()));
		m_downPass.SetVertexShader(vertexShader);
		m_downPass.SetPixelShader(pixelShader);
		m_downPass.Init();
	}

	// upsample pass
	{
		m_upsampledGlowTexture.Init(GetCurrentWidth(), GetCurrentHeight());

		std::shared_ptr<VertexShader> vertexShader = std::make_shared<VertexShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\alpha_demo_postprocessing_VS.cso")));
		vertexShader->SetVertexInput(std::make_shared<alpha::TextureDataVertexInput>());

		std::shared_ptr<PixelShader> pixelShader = std::make_shared<PixelShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\alpha_demo_sample_PS.cso")));

		m_upPass.SetState(std::make_shared<RenderPassState>(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_viewport, std::make_shared<SolidCullBackRS>(), m_upsampledGlowTexture.AsRenderTargetView(), m_depthBufferView.Get()));
		m_upPass.SetVertexShader(vertexShader);
		m_upPass.SetPixelShader(pixelShader);
		m_upPass.Init();
	}

	// Horizontal blur pass
	{
		m_horizontalBlurTexture.Init(GetCurrentWidth(), GetCurrentHeight());

		std::shared_ptr<VertexShader> vertexShader = std::make_shared<VertexShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\alpha_demo_postprocessing_VS.cso")));
		vertexShader->SetVertexInput(std::make_shared<alpha::TextureDataVertexInput>());
		
		std::shared_ptr<PixelShader> pixelShader = std::make_shared<PixelShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\alpha_demo_horizontal_blur_PS.cso")));
		pixelShader->AddConstantBuffer(CBufferFrequency::per_frame, std::make_unique<CBuffer<PerFrameBlurData>>());
		pixelShader->AddSampler(SamplerUsage::blur, std::make_shared<alpha::BlurSampler>());

		m_horizontalBlurPass.SetState(std::make_shared<RenderPassState>(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_viewport, std::make_shared<SolidCullBackRS>(), m_horizontalBlurTexture.AsRenderTargetView(), m_depthBufferView.Get()));
		m_horizontalBlurPass.SetVertexShader(vertexShader);
		m_horizontalBlurPass.SetPixelShader(pixelShader);
		m_horizontalBlurPass.Init();
	}

	// Vertical blur pass
	{
		m_verticalBlurTexture.Init(GetCurrentWidth(), GetCurrentHeight());

		std::shared_ptr<VertexShader> vertexShader = std::make_shared<VertexShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\alpha_demo_postprocessing_VS.cso")));
		vertexShader->SetVertexInput(std::make_shared<alpha::TextureDataVertexInput>());

		std::shared_ptr<PixelShader> pixelShader = std::make_shared<PixelShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\alpha_demo_vertical_blur_PS.cso")));
		pixelShader->AddConstantBuffer(CBufferFrequency::per_frame, std::make_unique<CBuffer<PerFrameBlurData>>());
		pixelShader->AddSampler(SamplerUsage::blur, std::make_shared<alpha::BlurSampler>());

		m_verticalBlurPass.SetState(std::make_shared<RenderPassState>(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_viewport, std::make_shared<SolidCullBackRS>(), m_verticalBlurTexture.AsRenderTargetView(), m_depthBufferView.Get()));
		m_verticalBlurPass.SetVertexShader(vertexShader);
		m_verticalBlurPass.SetPixelShader(pixelShader);
		m_verticalBlurPass.Init();
	}
{
		m_rarelyChangedData.useMotionBlurMap = true;

		// inizializzo la motion blur map
		m_motionBlurMap.SetViewAndProjectionMatrices(m_camera);
		m_motionBlurMap.Init(GetCurrentWidth(), GetCurrentHeight());

		std::shared_ptr<VertexShader> vertexShader = std::make_shared<VertexShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\motion_blur_demo_motionblurmap_VS.cso")));
		vertexShader->SetVertexInput(std::make_shared<PosOnlyVertexInput>());
		vertexShader->AddConstantBuffer(CBufferFrequency::per_object, std::make_unique<CBuffer<MotionBlurMap::PerObjectMotionBlurMapData>>());
		std::shared_ptr<PixelShader> pixelShader = std::make_shared<PixelShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\motion_blur_demo_motionblurmap_PS.cso")));

		m_motionBlurPass.SetState(std::make_shared<RenderPassState>(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_motionBlurMap.Viewport(), std::make_shared<SolidCullBackRS>(), m_motionBlurMap.AsMotionBlurView(), m_depthBufferView.Get()));
		m_motionBlurPass.SetVertexShader(vertexShader);
		m_motionBlurPass.SetPixelShader(pixelShader);
		m_motionBlurPass.Init();
	}

	// postprocessing pass
	{
		std::shared_ptr<VertexShader> vertexShader = std::make_shared<VertexShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\alpha_demo_postprocessing_VS.cso")));
		vertexShader->SetVertexInput(std::make_shared<alpha::TextureDataVertexInput>());

		std::shared_ptr<PixelShader> pixelShader = std::make_shared<PixelShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\alpha_demo_postprocessing_PS.cso")));
		pixelShader->AddSampler(SamplerUsage::common_textures, std::make_shared<MotionBlurSampler>());
		pixelShader->AddConstantBuffer(CBufferFrequency::per_frame, std::make_unique<CBuffer<PerFrameData>>());
		pixelShader->AddConstantBuffer(CBufferFrequency::rarely_changed, std::make_unique<CBuffer<RarelyChangedData>>());

		m_PostPass.SetState(std::make_shared<RenderPassState>(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_viewport, std::make_shared<SolidCullBackRS>(), m_backBufferView.Get(), m_depthBufferView.Get()));
		m_PostPass.SetVertexShader(vertexShader);
		m_PostPass.SetPixelShader(pixelShader);
		m_PostPass.Init();
	}
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

	//init of the glowmap for the sphere
	Microsoft::WRL::ComPtr<ID3D11Resource> sphereGlowTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sphereGlowTextureView;
	XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(xtest::service::Locator::GetD3DDevice(), xtest::service::Locator::GetD3DContext(),
		GetRootDir().append(LR"(\3d-objects\plastic-cover\plastic_cover_glow.png)").c_str(), sphereGlowTexture.GetAddressOf(), sphereGlowTextureView.GetAddressOf()));

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

	{
		std::pair<render::Renderable*, std::string> glowObjectKey;
		alpha::GlowObject glowObject;
		glowObjectKey.first = &m_objects[4];
		glowObjectKey.second = "";
		glowObject.glowTexture = sphereGlowTexture;
		glowObject.glowTextureView = sphereGlowTextureView;

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

	m_motionBlurPass.GetState()->ChangeRenderTargetView(m_motionBlurMap.AsMotionBlurView());
	m_motionBlurPass.GetState()->ChangeViewPort(m_motionBlurMap.Viewport());
	m_motionBlurPass.GetState()->ChangeDepthStencilView(m_depthBufferView.Get());
	//m_combinePass
	m_combinePass.GetState()->ChangeRenderTargetView(m_backBufferView.Get());
	m_combinePass.GetState()->ChangeDepthStencilView(m_depthBufferView.Get());
	m_combinePass.GetState()->ChangeViewPort(m_viewport);
	m_glowmap.Resize(GetCurrentWidth(), GetCurrentHeight());
	//update the glow pass state with the resized render target and depth buffer
	m_glowPass.GetState()->ChangeRenderTargetView(m_glowmap.AsRenderTargetView());
	m_glowPass.GetState()->ChangeDepthStencilView(m_depthBufferView.Get());
	m_glowPass.GetState()->ChangeViewPort(m_viewport);

	m_downsampledGlowTexture.Resize(GetCurrentWidth() / 2, GetCurrentHeight() / 2);
	m_downViewport.Width = static_cast<float>(GetCurrentWidth() / 2);
	m_downViewport.Height = static_cast<float>(GetCurrentHeight() / 2);
	m_downPass.GetState()->ChangeRenderTargetView(m_downsampledGlowTexture.AsRenderTargetView());
	m_downPass.GetState()->ChangeDepthStencilView(m_depthBufferView.Get());
	m_downPass.GetState()->ChangeViewPort(m_downViewport);

	m_horizontalBlurTexture.Resize(GetCurrentWidth(), GetCurrentHeight());
	//update the glow pass state with the resized render target and depth buffer
	m_horizontalBlurPass.GetState()->ChangeRenderTargetView(m_horizontalBlurTexture.AsRenderTargetView());
	m_horizontalBlurPass.GetState()->ChangeDepthStencilView(m_depthBufferView.Get());
	m_horizontalBlurPass.GetState()->ChangeViewPort(m_viewport);

	m_verticalBlurTexture.Resize(GetCurrentWidth(), GetCurrentHeight());
	//update the glow pass state with the resized render target and depth buffer
	m_verticalBlurPass.GetState()->ChangeRenderTargetView(m_verticalBlurTexture.AsRenderTargetView());
	m_verticalBlurPass.GetState()->ChangeDepthStencilView(m_depthBufferView.Get());
	m_verticalBlurPass.GetState()->ChangeViewPort(m_viewport);

	m_upsampledGlowTexture.Resize(GetCurrentWidth(), GetCurrentHeight());
	m_upPass.GetState()->ChangeRenderTargetView(m_upsampledGlowTexture.AsRenderTargetView());
	m_upPass.GetState()->ChangeDepthStencilView(m_depthBufferView.Get());
	m_upPass.GetState()->ChangeViewPort(m_viewport);

	//update the post pass state with the resized render target and depth buffer
	m_PostPass.GetState()->ChangeRenderTargetView(m_backBufferView.Get());
	m_PostPass.GetState()->ChangeDepthStencilView(m_depthBufferView.Get());
	m_PostPass.GetState()->ChangeViewPort(m_viewport);

	//update the projection matrix with the new aspect ratio
	m_camera.SetPerspectiveProjection(math::ToRadians(45.f), AspectRatio(), 1.f, 1000.f);
	m_motionBlurMap.SetViewAndProjectionMatrices(m_camera);

	CreateDownDepthStencilBuffer();
}


void AlphaDemoApp::OnWheelScroll(input::ScrollStatus scroll)
{
	// zoom in or out when the scroll wheel is used
	if (service::Locator::GetMouse()->IsInClientArea())
	{
		m_camera.IncreaseRadiusBy(scroll.isScrollingUp ? -0.5f : 0.5f);
		m_motionBlurMap.SetViewAndProjectionMatrices(m_camera);
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
		m_motionBlurMap.SetViewAndProjectionMatrices(m_camera);
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
		m_motionBlurMap.SetViewAndProjectionMatrices(m_camera);
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
		m_motionBlurMap.SetViewAndProjectionMatrices(m_camera);
	}
	else if (key == input::Key::F1 && status.isDown)
	{
		m_rarelyChangedData.useShadowMap = !m_rarelyChangedData.useShadowMap;
		m_isRarelyChangedDataDirty = true;
	}
	else if (key == input::Key::F2 && status.isDown)
	{
		m_rarelyChangedData.useGlowMap = !m_rarelyChangedData.useGlowMap;
		m_isRarelyChangedDataDirty = true;
	}
	else if (key == input::Key::F3 && status.isDown)
	{
		m_rarelyChangedData.useMotionBlurMap = !m_rarelyChangedData.useMotionBlurMap;
		m_isRarelyChangedDataDirty = true;
	}
	else if (key == input::Key::F4 && status.isDown)
	{
		m_rarelyChangedData.useLightScattering = !m_rarelyChangedData.useLightScattering;
		m_isRarelyChangedDataDirty = true;
	}
	else if (key == input::Key::space_bar && status.isDown)
	{
		m_stop = !m_stop;
	}
}


void AlphaDemoApp::UpdateScene(float deltaSeconds)
{
	XTEST_UNUSED_VAR(deltaSeconds);

	if (!m_stop)
	{
		//Update previous transform
		for (int k = 0; k < m_objects.size(); k++) {
			previous_Transforms[k] = m_objects[k].GetTransform();
		}

		XMMATRIX R = XMMatrixRotationY(math::ToRadians(120.f) * deltaSeconds);
		XMMATRIX W = XMLoadFloat4x4(&m_objects[1].GetTransform());
		W *= R;
		m_objects[1].SetTransform(W);
		W = XMLoadFloat4x4(&m_objects[2].GetTransform());
		W *= R;
		m_objects[2].SetTransform(W);

		totalTime += deltaSeconds;
		XMMATRIX newW = backupWCrate * XMMatrixTranslation(0.0f, (sin(totalTime*10.0f) + 1.0f) * 4.f, 0.0f);
		m_objects[3].SetTransform(newW);

		m_motionBlurMap.SetViewAndProjectionMatrices(m_camera);
	}

	// PerFrameCB
	{
		PerFrameData data;
		data.dirLights[0] = m_dirKeyLight;
		data.dirLights[1] = m_dirFillLight;
		data.eyePosW = m_camera.GetPosition();
		data.blurMultiplier = (1.0f / deltaSeconds) / (float) targetFPS;

		// calculate where the light placeholder is in screen space
		{
			XMMATRIX W = XMLoadFloat4x4(&m_lightOcclusionMap.LightPlaceHolder().GetTransform());
			XMMATRIX V = m_camera.GetViewMatrix();
			XMMATRIX P = m_camera.GetProjectionMatrix();
			XMMATRIX T = m_lightOcclusionMap.TMatrix();
			XMMATRIX WVPT = W * V * P * T;

			XMVECTOR dirLightPosL = XMVectorSet(0.f, 0.f, 0.f, 1.f);
			XMVECTOR dirLightPosH = XMVector4Transform(dirLightPosL, WVPT);
			XMStoreFloat4(&data.dirLightPosH, dirLightPosH);
		}

		m_renderPass.GetPixelShader()->GetConstantBuffer(CBufferFrequency::per_frame)->UpdateBuffer(data);
		m_PostPass.GetPixelShader()->GetConstantBuffer(CBufferFrequency::per_frame)->UpdateBuffer(data);
	}

	// RarelyChangedCB
	if (m_isRarelyChangedDataDirty)
	{
		m_renderPass.GetPixelShader()->GetConstantBuffer(CBufferFrequency::rarely_changed)->UpdateBuffer(m_rarelyChangedData);
		m_PostPass.GetPixelShader()->GetConstantBuffer(CBufferFrequency::rarely_changed)->UpdateBuffer(m_rarelyChangedData);
		m_isRarelyChangedDataDirty = false;
	}
}


void AlphaDemoApp::RenderScene()
{
	// draw shawdowmap
	m_d3dAnnotation->BeginEvent(L"shadow-map");
	m_shadowPass.Bind();
	m_shadowPass.GetState()->ClearDepthOnly();

	// draw objects
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

	m_d3dAnnotation->BeginEvent(L"scattering-occlusion");
	m_scatteringPass.Bind();
	m_scatteringPass.GetState()->ClearDepthOnly();
	m_scatteringPass.GetState()->ClearRenderTarget(DirectX::Colors::Black);

	// draw a white placeholder to simulate the light source
	{
		const std::string meshName = "";
		render::Renderable placeholder = m_lightOcclusionMap.LightPlaceHolder();

		PerObjectLightScatteringData data = ToPerObjectLightScatteringData(placeholder, meshName, true);
		m_scatteringPass.GetVertexShader()->GetConstantBuffer(CBufferFrequency::per_object)->UpdateBuffer(data);
		m_scatteringPass.GetPixelShader()->GetConstantBuffer(CBufferFrequency::per_object)->UpdateBuffer(data);
		placeholder.Draw(meshName);
	}

	// draw all objects with a black material
	for (render::Renderable& renderable : m_objects)
	{
		for (const std::string& meshName : renderable.GetMeshNames())
		{
			PerObjectLightScatteringData data = ToPerObjectLightScatteringData(renderable, meshName, false);
			m_scatteringPass.GetVertexShader()->GetConstantBuffer(CBufferFrequency::per_object)->UpdateBuffer(data);
			m_scatteringPass.GetPixelShader()->GetConstantBuffer(CBufferFrequency::per_object)->UpdateBuffer(data);
			renderable.Draw(meshName);
		}
	}
	m_d3dAnnotation->EndEvent();

	// draw objects to texture
	m_d3dAnnotation->BeginEvent(L"render-scene");
	m_renderPass.Bind();
	m_renderPass.GetState()->ClearDepthOnly();
	m_renderPass.GetState()->ClearRenderTarget(DirectX::Colors::DarkSlateGray);
	m_renderPass.GetPixelShader()->BindTexture(TextureUsage::shadow_map, m_shadowMap.AsShaderView());
	m_renderPass.GetPixelShader()->BindTexture(TextureUsage::light_occlusion_map, m_lightOcclusionMap.AsShaderView());

	
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
	m_renderPass.GetPixelShader()->BindTexture(TextureUsage::light_occlusion_map, nullptr); // explicit unbind the shadow map to suppress warning
	m_d3dAnnotation->EndEvent();

	if (m_rarelyChangedData.useGlowMap)
	{
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
					GlowObjectKey glowObjectKey(&renderable, meshName);
					const alpha::GlowObject glowObject = m_glowObjectsMap.at(glowObjectKey);

					m_glowPass.GetPixelShader()->BindTexture(TextureUsage::color, renderable.GetTextureView(TextureUsage::color, meshName));
					m_glowPass.GetPixelShader()->BindTexture(TextureUsage::glow_map, glowObject.glowTextureView.Get());
				}

				renderable.Draw(meshName);
			}
		}
		m_d3dAnnotation->EndEvent();


		//Drawing the downsample texture
		m_d3dAnnotation->BeginEvent(L"down-sample");
		m_downPass.Bind();
		m_downPass.GetState()->ClearDepthOnly();
		m_downPass.GetState()->ClearRenderTarget(DirectX::Colors::Transparent);
		m_downPass.GetPixelShader()->BindTexture(TextureUsage::scaleSample, m_glowmap.AsShaderView());

		m_textureRenderable.Draw();

		m_downPass.GetPixelShader()->BindTexture(TextureUsage::scaleSample, nullptr);
		m_d3dAnnotation->EndEvent();


		// Drawing horizontal blur
		m_d3dAnnotation->BeginEvent(L"horizontal-blur");
		m_horizontalBlurPass.Bind();
		m_horizontalBlurPass.GetState()->ClearDepthOnly();
		m_horizontalBlurPass.GetState()->ClearRenderTarget(DirectX::Colors::Transparent);
		m_horizontalBlurPass.GetPixelShader()->BindTexture(TextureUsage::blur, m_downsampledGlowTexture.AsShaderView());

		PerFrameBlurData horizontalData;
		horizontalData.resolution = static_cast<float>(GetCurrentWidth());
		m_horizontalBlurPass.GetPixelShader()->GetConstantBuffer(CBufferFrequency::per_frame)->UpdateBuffer(horizontalData);

		m_textureRenderable.Draw();

		m_horizontalBlurPass.GetPixelShader()->BindTexture(TextureUsage::blur, nullptr);
		m_d3dAnnotation->EndEvent();


		// Drawing vertical blur
		m_d3dAnnotation->BeginEvent(L"vertical-blur");
		m_verticalBlurPass.Bind();
		m_verticalBlurPass.GetState()->ClearDepthOnly();
		m_verticalBlurPass.GetState()->ClearRenderTarget(DirectX::Colors::Transparent);
		m_verticalBlurPass.GetPixelShader()->BindTexture(TextureUsage::blur, m_horizontalBlurTexture.AsShaderView());

		PerFrameBlurData verticalCData;
		verticalCData.resolution = static_cast<float>(GetCurrentHeight());
		m_verticalBlurPass.GetPixelShader()->GetConstantBuffer(CBufferFrequency::per_frame)->UpdateBuffer(verticalCData);

		m_textureRenderable.Draw();

		m_verticalBlurPass.GetPixelShader()->BindTexture(TextureUsage::blur, nullptr);
		m_d3dAnnotation->EndEvent();

		// Drawing horizontal blur second pass
		m_d3dAnnotation->BeginEvent(L"horizontal-blur-2");
		m_horizontalBlurPass.Bind();
		m_horizontalBlurPass.GetState()->ClearDepthOnly();
		m_horizontalBlurPass.GetState()->ClearRenderTarget(DirectX::Colors::Transparent);
		m_horizontalBlurPass.GetPixelShader()->BindTexture(TextureUsage::blur, m_verticalBlurTexture.AsShaderView());

		m_horizontalBlurPass.GetPixelShader()->GetConstantBuffer(CBufferFrequency::per_frame)->UpdateBuffer(horizontalData);

		m_textureRenderable.Draw();

		m_horizontalBlurPass.GetPixelShader()->BindTexture(TextureUsage::blur, nullptr);
		m_d3dAnnotation->EndEvent();


		// Drawing vertical blur second pass
		m_d3dAnnotation->BeginEvent(L"vertical-blur-2");
		m_verticalBlurPass.Bind();
		m_verticalBlurPass.GetState()->ClearDepthOnly();
		m_verticalBlurPass.GetState()->ClearRenderTarget(DirectX::Colors::Transparent);
		m_verticalBlurPass.GetPixelShader()->BindTexture(TextureUsage::blur, m_horizontalBlurTexture.AsShaderView());

		m_verticalBlurPass.GetPixelShader()->GetConstantBuffer(CBufferFrequency::per_frame)->UpdateBuffer(verticalCData);

		m_textureRenderable.Draw();

		m_verticalBlurPass.GetPixelShader()->BindTexture(TextureUsage::blur, nullptr);
		m_d3dAnnotation->EndEvent();


		// Drawing the upsample texture
		m_d3dAnnotation->BeginEvent(L"up-sample");
		m_upPass.Bind();
		m_upPass.GetState()->ClearDepthOnly();
		m_upPass.GetState()->ClearRenderTarget(DirectX::Colors::Transparent);
		m_upPass.GetPixelShader()->BindTexture(TextureUsage::scaleSample, m_verticalBlurTexture.AsShaderView());

		m_textureRenderable.Draw();

		m_upPass.GetPixelShader()->BindTexture(TextureUsage::scaleSample, nullptr);
		m_d3dAnnotation->EndEvent();
	}

	// motion blur pass
	m_d3dAnnotation->BeginEvent(L"motion-blur-map");
	m_motionBlurPass.Bind();
	m_motionBlurPass.GetState()->ClearDepthOnly();
	m_motionBlurPass.GetState()->ClearRenderTarget(DirectX::Colors::Black);

	int i = 0;
	// draw MOTIONBLURMAP
	for (render::Renderable& renderable : m_objects) {
		for (const std::string& meshName : renderable.GetMeshNames()) {
			MotionBlurMap::PerObjectMotionBlurMapData data = m_motionBlurMap.ToPerObjectMotionBlurMapData(renderable, meshName, m_camera, previous_Transforms[i]);
			m_motionBlurPass.GetVertexShader()->GetConstantBuffer(CBufferFrequency::per_object)->UpdateBuffer(data);
			renderable.Draw(meshName);
		}
		i++;
	}
	m_d3dAnnotation->EndEvent();


	//Drawing all the textures together
	m_d3dAnnotation->BeginEvent(L"render-from-texture");
	m_PostPass.Bind();
	m_PostPass.GetState()->ClearDepthOnly();
	m_PostPass.GetState()->ClearRenderTarget(DirectX::Colors::Black);
	m_PostPass.GetPixelShader()->BindTexture(TextureUsage::texture_map, m_sceneTexture.AsShaderView());
	m_PostPass.GetPixelShader()->BindTexture(TextureUsage::bloom, m_upsampledGlowTexture.AsShaderView());
	m_PostPass.GetPixelShader()->BindTexture(TextureUsage::motionblur, m_motionBlurMap.AsShaderView());

	   
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
	XMMATRIX WVPT_occlusionMap = WVP * m_lightOcclusionMap.TMatrix();

	XMStoreFloat4x4(&data.W, XMMatrixTranspose(W));
	XMStoreFloat4x4(&data.WVP, XMMatrixTranspose(WVP));
	XMStoreFloat4x4(&data.W_inverseTraspose, XMMatrixInverse(nullptr, W));
	XMStoreFloat4x4(&data.TexcoordMatrix, XMMatrixTranspose(T));
	XMStoreFloat4x4(&data.WVPT_shadowMap, XMMatrixTranspose(WVPT_shadowMap));
	XMStoreFloat4x4(&data.WVPT_occlusionMap, XMMatrixTranspose(WVPT_occlusionMap));
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

AlphaDemoApp::PerObjectLightScatteringData AlphaDemoApp::ToPerObjectLightScatteringData(const render::Renderable& renderable, const std::string& meshName, boolean isLight)
{
	PerObjectLightScatteringData data;

	XMMATRIX W = XMLoadFloat4x4(&renderable.GetTransform());
	XMMATRIX V = m_camera.GetViewMatrix();
	XMMATRIX P = m_camera.GetProjectionMatrix();
	XMMATRIX WVP = W * V * P;

	XMStoreFloat4x4(&data.WVP, XMMatrixTranspose(WVP));
	if (isLight)
	{
		data.material.ambient = { 1.f, 1.f, 1.f, 1.f };
		data.material.diffuse = { 1.f, 1.f, 1.f, 1.f };
		data.material.specular = { 1.f, 1.f, 1.f, 1.f };
	}
	else
	{
		data.material.ambient = { 0.f, 0.f, 0.f, 1.f };
		data.material.diffuse = { 0.f, 0.f, 0.f, 1.f };
		data.material.specular = { 0.f, 0.f, 0.f, 1.f };
	}

	return data;
}

AlphaDemoApp::PerObjectCombineData AlphaDemoApp::ToPerObjectCombineData(const render::Renderable& renderable, const std::string& meshName) {
	XTEST_UNUSED_VAR(meshName);
	PerObjectCombineData data;

	XMMATRIX W = XMLoadFloat4x4(&renderable.GetTransform());
	XMMATRIX V = m_camera.GetViewMatrix();
	XMMATRIX P = m_camera.GetProjectionMatrix();
	XMMATRIX WVP = W * V*P;

	XMStoreFloat4x4(&data.WVP, XMMatrixTranspose(WVP));
	return data;
}

void xtest::demo::AlphaDemoApp::CreateDownDepthStencilBuffer()
{
	m_depthBufferDownsample.Reset();
	m_depthStencilViewDownsample.Reset();

	D3D11_TEXTURE2D_DESC depthDesc;
	depthDesc.Width = GetCurrentWidth() / 2;
	depthDesc.Height = GetCurrentHeight() / 2;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;		// no MSAA
	depthDesc.SampleDesc.Quality = 0;	// no MSAA
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;

	// create the depth buffer and its view
	XTEST_D3D_CHECK(m_d3dDevice->CreateTexture2D(&depthDesc, nullptr, &m_depthBufferDownsample));
	XTEST_D3D_CHECK(m_d3dDevice->CreateDepthStencilView(m_depthBufferDownsample.Get(), nullptr, &m_depthStencilViewDownsample));
}