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

using xtest::demo::ShawdowsDemoApp;
using Microsoft::WRL::ComPtr;

ShawdowsDemoApp::ShawdowsDemoApp(HINSTANCE instance,
	const application::WindowSettings& windowSettings,
	const application::DirectxSettings& directxSettings,
	uint32 fps /*=60*/)
	: application::DirectxApp(instance, windowSettings, directxSettings, fps)
	, m_dirKeyLight()
	, m_dirFillLight()
	, m_lightingControls()
	, m_isLightingControlsDirty(true)
	, m_camera(math::ToRadians(60.f), math::ToRadians(125.f), 5.f, { 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, { math::ToRadians(4.f), math::ToRadians(175.f) }, { 3.f, 25.f })
	, m_objects()
	, m_renderPass()
{}


ShawdowsDemoApp::~ShawdowsDemoApp()
{}


void ShawdowsDemoApp::Init()
{
	application::DirectxApp::Init();

	m_camera.SetPerspectiveProjection(math::ToRadians(45.f), AspectRatio(), 1.f, 1000.f);

	InitRenderTechnique();
	InitRenderables();
	InitLights();

	service::Locator::GetMouse()->AddListener(this);
	service::Locator::GetKeyboard()->AddListener(this, { input::Key::F, input::Key::F1, input::Key::F2, input::Key::F3, input::Key::space_bar });
}


void ShawdowsDemoApp::InitRenderTechnique()
{
	file::ResourceLoader* loader = service::Locator::GetResourceLoader();
	
	std::shared_ptr<VertexShader> vertexShader = std::make_shared<VertexShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\shawdow_demo_VS.cso")));
	vertexShader->SetVertexInput(std::make_shared<MeshDataVertexInput>());
	vertexShader->AddConstantBuffer(CBufferFrequency::per_object, std::make_unique<CBuffer<PerObjectData>>());

	std::shared_ptr<PixelShader> pixelShader = std::make_shared<PixelShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\shawdow_demo_PS.cso")));
	pixelShader->AddConstantBuffer(CBufferFrequency::per_object, std::make_unique<CBuffer<PerObjectData>>());
	pixelShader->AddConstantBuffer(CBufferFrequency::per_frame, std::make_unique<CBuffer<PerFrameData>>());
	pixelShader->AddConstantBuffer(CBufferFrequency::rarely_changed, std::make_unique<CBuffer<RarelyChangedData>>());
	pixelShader->AddSampler(SamplerUsage::common_textures, std::make_shared<AnisotropicSampler>());

	m_renderPass.SetState(std::make_shared<RenderPassState>(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_viewport, std::make_shared<SolidCullBackRS>(), m_backBufferView.Get(), m_depthBufferView.Get()));
	m_renderPass.SetVertexShader(vertexShader);
	m_renderPass.SetPixelShader(pixelShader);
	m_renderPass.Init();
}


void ShawdowsDemoApp::InitRenderables()
{	
	render::Renderable ground{ *(service::Locator::GetResourceLoader()->LoadGPFMesh(GetRootDir().append(LR"(\3d-objects\rocks_dorama\rocks_composition.gpf)"))) };
	ground.SetTransform(XMMatrixScaling(0.2f, 0.2f, 0.2f));
	ground.Init();
	m_objects.push_back(std::move(ground));

	render::Renderable female1{ *(service::Locator::GetResourceLoader()->LoadGPFMesh(GetRootDir().append(LR"(\3d-objects\gdc_female\gdc_female_posed.gpf)"))) };
	female1.SetTransform(XMMatrixTranslation(2.f, -0.2f, 12.f) * XMMatrixScaling(0.2f, 0.2f, 0.2f) * XMMatrixRotationY(math::ToRadians(150.f)));
	female1.Init();
	m_objects.push_back(std::move(female1));

	render::Renderable female2{ *(service::Locator::GetResourceLoader()->LoadGPFMesh(GetRootDir().append(LR"(\3d-objects\gdc_female\gdc_female_posed_2.gpf)"))) };
	female2.SetTransform(XMMatrixTranslation(-2.f, 0.3f, -1.f) * XMMatrixScaling(0.2f, 0.2f, 0.2f) );
	female2.Init();
	m_objects.push_back(std::move(female2));
}


void ShawdowsDemoApp::InitLights()
{
	m_dirKeyLight.ambient = { 0.16f, 0.18f, 0.18f, 1.f };
	m_dirKeyLight.diffuse = { 2.f* 0.78f, 2.f* 0.83f, 2.f* 1.f, 1.f };
	m_dirKeyLight.specular = {  0.87f,  0.90f,  0.94f, 1.f };
	XMVECTOR dirLightDirection = XMVector3Normalize(-XMVectorSet(5.f, 3.f, 5.f, 0.f));
	XMStoreFloat3(&m_dirKeyLight.dirW, dirLightDirection);

	m_dirFillLight.ambient = { 0.16f, 0.18f, 0.18f, 1.f };
	m_dirFillLight.diffuse  = { 0.4f * 1.f, 0.4f * 0.91f, 0.4f * 0.85f, 1.f };
	m_dirFillLight.specular = { 0.087f, 0.09f, 0.094f, 1.f };
	XMStoreFloat3(&m_dirFillLight.dirW, XMVectorScale(dirLightDirection, -1.f));

	m_lightingControls.useDirLight = true;
	m_lightingControls.useBumpMap = true;
}


void ShawdowsDemoApp::OnResized()
{
	application::DirectxApp::OnResized();

	//update the render pass state with the resized render target and depth buffer
	m_renderPass.GetState()->ChangeRenderTargetView(m_backBufferView.Get());
	m_renderPass.GetState()->ChangeDepthStencilView(m_depthBufferView.Get());
	m_renderPass.GetState()->ChangeViewPort(m_viewport);

	//update the projection matrix with the new aspect ratio
	m_camera.SetPerspectiveProjection(math::ToRadians(45.f), AspectRatio(), 1.f, 1000.f);
}


void ShawdowsDemoApp::OnWheelScroll(input::ScrollStatus scroll)
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


void ShawdowsDemoApp::OnMouseMove(const DirectX::XMINT2& movement, const DirectX::XMINT2& currentPos)
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


void ShawdowsDemoApp::OnKeyStatusChange(input::Key key, const input::KeyStatus& status)
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
	else if (key == input::Key::F3 && status.isDown)
	{
		m_lightingControls.useBumpMap = !m_lightingControls.useBumpMap;
		m_isLightingControlsDirty = true;
	}
}


void ShawdowsDemoApp::UpdateScene(float deltaSeconds)
{

	// PerFrameCB
	{

		PerFrameData data;
		data.dirLights[0] = m_dirKeyLight;
		data.dirLights[1] = m_dirFillLight;
		data.eyePosW = m_camera.GetPosition();

		m_renderPass.GetPixelShader()->GetConstantBuffer(CBufferFrequency::per_frame)->UpdateBuffer(data);
	}


	// RarelyChangedCB
	if (m_isLightingControlsDirty)
	{
		m_renderPass.GetPixelShader()->GetConstantBuffer(CBufferFrequency::rarely_changed)->UpdateBuffer(m_lightingControls);
		m_isLightingControlsDirty = false;
	}
	
}


void ShawdowsDemoApp::RenderScene()
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


ShawdowsDemoApp::PerObjectData ShawdowsDemoApp::ToPerObjectData(const render::Renderable& renderable, const std::string& meshName) const
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

