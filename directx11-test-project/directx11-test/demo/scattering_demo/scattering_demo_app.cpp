#include "stdafx.h"
#include "scattering_demo_app.h"
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
using namespace xtest::demo;
using Microsoft::WRL::ComPtr;


LightScatteringDemoApp::LightScatteringDemoApp(HINSTANCE instance,
	const application::WindowSettings& windowSettings,
	const application::DirectxSettings& directxSettings,
	uint32 fps /*=60*/)
	: application::DirectxApp(instance, windowSettings, directxSettings, fps)
	, m_dirKeyLight()
	, m_dirFillLight()
	, m_rarelyChangedData()
	, m_isRarelyChangedDataDirty(true)
	, m_camera(math::ToRadians(90.f), math::ToRadians(160.f), 15.f, { 5.4f, 4.5f, -4.8f }, { 0.f, 1.f, 0.f }, { math::ToRadians(4.f), math::ToRadians(175.f) }, { 3.f, 50.f })
	, m_objects()
	, m_shadowPass()
	, m_scatteringPass()
	, m_renderPass()
	, m_shadowMap(2048)
	, m_lightOcclusionMap(2048)
	, m_sceneBoundingSphere({ 0.f, 0.f, 0.f }, 21.f)
	, m_worldTime(0.f)
	, m_stopCube(false)
{}


LightScatteringDemoApp::~LightScatteringDemoApp()
{}


void LightScatteringDemoApp::Init()
{
	application::DirectxApp::Init();

	m_camera.SetPerspectiveProjection(math::ToRadians(45.f), AspectRatio(), 1.f, 1000.f);

	InitLights();
	InitRenderTechnique();
	InitRenderables();

	service::Locator::GetMouse()->AddListener(this);
	service::Locator::GetKeyboard()->AddListener(this, { input::Key::F, input::Key::F1, input::Key::space_bar });
}


void LightScatteringDemoApp::InitLights()
{
	m_dirKeyLight.ambient = { 0.16f, 0.18f, 0.18f, 1.f };
	m_dirKeyLight.diffuse = { 0.8f ,  0.8f,  0.7f, 1.f };
	m_dirKeyLight.specular = { 0.8f ,  0.8f,  0.7f, 1.f };
	XMStoreFloat3(&m_dirKeyLight.dirW, XMVector3Normalize(-XMVectorSet(0.f, 0.4f, 0.9f, 0.f)));


	m_dirFillLight.ambient = { 0.01f * 0.16f , 0.01f * 0.18f, 0.005f * 0.18f, 1.f };
	m_dirFillLight.diffuse = { 0.02f * 1.f   , 0.01f * 0.91f, 0.005f * 0.85f, 1.f };
	m_dirFillLight.specular = { 0.01f * 0.087f, 0.01f * 0.09f, 0.005f * 0.094f, 1.f };
	XMStoreFloat3(&m_dirFillLight.dirW, XMVector3Transform(XMLoadFloat3(&m_dirKeyLight.dirW), XMMatrixRotationY(math::ToRadians(45.f))));

	m_rarelyChangedData.useShadowMap = true;
	m_rarelyChangedData.useLightScattering = true;
}


void LightScatteringDemoApp::InitRenderTechnique()
{
	file::ResourceLoader* loader = service::Locator::GetResourceLoader();


	// shadow pass
	{
		m_shadowMap.SetTargetBoundingSphere(m_sceneBoundingSphere);
		m_shadowMap.SetLight(m_dirKeyLight.dirW);
		m_shadowMap.Init();

		m_rarelyChangedData.shadowMapResolution = float(m_shadowMap.Resolution());

		std::shared_ptr<VertexShader> vertexShader = std::make_shared<VertexShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\scattering_demo_shadowmap_VS.cso")));
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
		std::shared_ptr<VertexShader> vertexShader = std::make_shared<VertexShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\scattering_demo_VS.cso")));
		vertexShader->SetVertexInput(std::make_shared<MeshDataVertexInput>());
		vertexShader->AddConstantBuffer(CBufferFrequency::per_object, std::make_unique<CBuffer<PerObjectData>>());

		std::shared_ptr<PixelShader> pixelShader = std::make_shared<PixelShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\scattering_demo_PS.cso")));
		pixelShader->AddConstantBuffer(CBufferFrequency::per_object, std::make_unique<CBuffer<PerObjectData>>());
		pixelShader->AddConstantBuffer(CBufferFrequency::per_frame, std::make_unique<CBuffer<PerFrameData>>());
		pixelShader->AddConstantBuffer(CBufferFrequency::rarely_changed, std::make_unique<CBuffer<RarelyChangedData>>());
		pixelShader->AddSampler(SamplerUsage::common_textures, std::make_shared<AnisotropicSampler>());
		pixelShader->AddSampler(SamplerUsage::shadow_map, std::make_shared<PCFSampler>());
		pixelShader->AddSampler(SamplerUsage::light_occlusion_map, std::make_shared<NoFilterSampler>());

		m_renderPass.SetState(std::make_shared<RenderPassState>(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_viewport, std::make_shared<SolidCullBackRS>(), m_backBufferView.Get(), m_depthBufferView.Get()));
		m_renderPass.SetVertexShader(vertexShader);
		m_renderPass.SetPixelShader(pixelShader);
		m_renderPass.Init();
	}
}


void LightScatteringDemoApp::InitRenderables()
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

	//Cube
	mesh::MeshMaterial mat;
	mat.ambient = { 0.15f, 0.15f, 0.15f, 1.f };
	mat.diffuse = { 0.52f, 0.52f, 0.52f, 1.f };
	mat.specular = { 0.5f, 0.5f, 0.5f, 1.f };
	mat.diffuseMap = GetRootDir().append(std::wstring(LR"(\3d-objects\ground\ground_)").append(L"color.png"));
	mat.normalMap = GetRootDir().append(std::wstring(LR"(\3d-objects\ground\ground_)").append(L"norm.png"));
	mat.glossMap = GetRootDir().append(std::wstring(LR"(\3d-objects\ground\ground_)").append(L"gloss.png"));
	render::Renderable cube(mesh::GenerateBox(2.f, 2.f, 2.f), mat);
	cube.SetTransform(XMMatrixTranslation(5.f, 7.f, 5.f));
	cube.SetTexcoordTransform(XMMatrixScaling(0.5f, 0.5f, 0.5f));
	cube.Init();
	m_objects.push_back(cube);
}


void LightScatteringDemoApp::OnResized()
{
	application::DirectxApp::OnResized();

	//update the render pass state with the resized render target and depth buffer
	m_renderPass.GetState()->ChangeRenderTargetView(m_backBufferView.Get());
	m_renderPass.GetState()->ChangeDepthStencilView(m_depthBufferView.Get());
	m_renderPass.GetState()->ChangeViewPort(m_viewport);

	//update the projection matrix with the new aspect ratio
	m_camera.SetPerspectiveProjection(math::ToRadians(45.f), AspectRatio(), 1.f, 1000.f);
}


void LightScatteringDemoApp::OnWheelScroll(input::ScrollStatus scroll)
{
	// zoom in or out when the scroll wheel is used
	if (service::Locator::GetMouse()->IsInClientArea())
	{
		m_camera.IncreaseRadiusBy(scroll.isScrollingUp ? -0.5f : 0.5f);
	}
}


void LightScatteringDemoApp::OnMouseMove(const DirectX::XMINT2& movement, const DirectX::XMINT2& currentPos)
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


void LightScatteringDemoApp::OnKeyStatusChange(input::Key key, const input::KeyStatus& status)
{
	if (key == input::Key::F && status.isDown)
	{
		m_camera.SetRotation(math::ToRadians(90.f), math::ToRadians(160.f));
		m_camera.SetRadius(15.f);
		m_camera.SetPivot({ 5.4f, 4.5f, -4.8f });
	}
	else if (key == input::Key::F1 && status.isDown)
	{
		m_rarelyChangedData.useLightScattering = !m_rarelyChangedData.useLightScattering;
		m_isRarelyChangedDataDirty = true;
	}
	else if (key == input::Key::space_bar && status.isDown)
	{
		m_stopCube = !m_stopCube;
	}
}


void LightScatteringDemoApp::UpdateScene(float deltaSeconds)
{
	XTEST_UNUSED_VAR(deltaSeconds);

	//Cube movement
	if(!m_stopCube)
	{
		m_worldTime += deltaSeconds;
		XMMATRIX newCubeTransform = XMMatrixTranslation(5.f, (sin(m_worldTime*5.f)*2.f + 7.f), 5.f);
		m_objects[3].SetTransform(newCubeTransform);
	}

	// PerFrameCB
	{
		PerFrameData data;
		data.dirLights[0] = m_dirKeyLight;
		data.dirLights[1] = m_dirFillLight;
		data.eyePosW = m_camera.GetPosition();

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
	}


	// RarelyChangedCB
	if (m_isRarelyChangedDataDirty)
	{
		m_renderPass.GetPixelShader()->GetConstantBuffer(CBufferFrequency::rarely_changed)->UpdateBuffer(m_rarelyChangedData);
		m_isRarelyChangedDataDirty = false;
	}

}


void LightScatteringDemoApp::RenderScene()
{

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


	m_d3dAnnotation->BeginEvent(L"render-scene");
	m_renderPass.Bind();
	m_renderPass.GetState()->ClearDepthOnly();
	m_renderPass.GetState()->ClearRenderTarget(DirectX::Colors::LightCyan);
	m_renderPass.GetPixelShader()->BindTexture(TextureUsage::shadow_map, m_shadowMap.AsShaderView());
	m_renderPass.GetPixelShader()->BindTexture(TextureUsage::light_occlusion_map, m_lightOcclusionMap.AsShaderView());

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
	m_renderPass.GetPixelShader()->BindTexture(TextureUsage::shadow_map, nullptr); // explicit unbind the shadow map to suppress warning
	m_renderPass.GetPixelShader()->BindTexture(TextureUsage::light_occlusion_map, nullptr); // explicit unbind the shadow map to suppress warning
	m_d3dAnnotation->EndEvent();

	XTEST_D3D_CHECK(m_swapChain->Present(0, 0));
}


LightScatteringDemoApp::PerObjectData LightScatteringDemoApp::ToPerObjectData(const render::Renderable& renderable, const std::string& meshName)
{
	PerObjectData data;

	XMMATRIX W = XMLoadFloat4x4(&renderable.GetTransform());
	XMMATRIX T = XMLoadFloat4x4(&renderable.GetTexcoordTransform(meshName));
	XMMATRIX V = m_camera.GetViewMatrix();
	XMMATRIX P = m_camera.GetProjectionMatrix();
	XMMATRIX WVP = W * V * P;
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


LightScatteringDemoApp::PerObjectShadowMapData LightScatteringDemoApp::ToPerObjectShadowMapData(const render::Renderable& renderable, const std::string& meshName)
{
	XTEST_UNUSED_VAR(meshName);
	PerObjectShadowMapData data;

	XMMATRIX W = XMLoadFloat4x4(&renderable.GetTransform());
	XMMATRIX WVP = W * m_shadowMap.LightViewMatrix() * m_shadowMap.LightProjMatrix();

	XMStoreFloat4x4(&data.WVP_lightSpace, XMMatrixTranspose(WVP));
	return data;
}


LightScatteringDemoApp::PerObjectLightScatteringData LightScatteringDemoApp::ToPerObjectLightScatteringData(const render::Renderable& renderable, const std::string& meshName, boolean isLight)
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

