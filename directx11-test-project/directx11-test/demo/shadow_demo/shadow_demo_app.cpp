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
{}


ShadowDemoApp::~ShadowDemoApp()
{}


void ShadowDemoApp::Init()
{
	application::DirectxApp::Init();

	m_camera.SetPerspectiveProjection(math::ToRadians(45.f), AspectRatio(), 1.f, 1000.f);

	InitRenderTechnique();
	InitRenderables();
	InitLights();

	service::Locator::GetMouse()->AddListener(this);
	service::Locator::GetKeyboard()->AddListener(this, { input::Key::F, input::Key::F1, input::Key::F2, input::Key::F3, input::Key::space_bar });
}


void ShadowDemoApp::InitRenderTechnique()
{
	file::ResourceLoader* loader = service::Locator::GetResourceLoader();

	std::shared_ptr<VertexShader> vertexShader = std::make_shared<VertexShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\textures_demo_VS.cso")));
	vertexShader->SetVertexInput(std::make_shared<MeshDataVertexInput>());
	vertexShader->AddConstantBuffer(CBufferFrequency::per_object, std::make_unique<CBuffer<PerObjectData>>());

	std::shared_ptr<PixelShader> pixelShader = std::make_shared<PixelShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\textures_demo_PS.cso")));
	pixelShader->AddConstantBuffer(CBufferFrequency::per_object, std::make_unique<CBuffer<PerObjectData>>());
	pixelShader->AddConstantBuffer(CBufferFrequency::per_frame, std::make_unique<CBuffer<PerFrameData>>());
	pixelShader->AddConstantBuffer(CBufferFrequency::rarely_changed, std::make_unique<CBuffer<RarelyChangedData>>());
	pixelShader->AddSampler(SamplerUsage::common_textures, std::make_shared<AnisotropicSampler>());

	m_renderPass.SetState(std::make_shared<RenderPassState>(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_viewport, std::make_shared<SolidCullBackRS>(), m_backBufferView.Get(), m_depthBufferView.Get()));
	m_renderPass.SetVertexShader(vertexShader);
	m_renderPass.SetPixelShader(pixelShader);
	m_renderPass.Init();
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
}
