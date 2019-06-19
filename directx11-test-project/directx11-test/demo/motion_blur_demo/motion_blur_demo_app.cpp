#include "stdafx.h"
#include "motion_blur_demo_app.h"
#include <file/file_utils.h>
#include <math/math_utils.h>
#include <service/locator.h>
#include <render/shading/vertex_input_types.h>
#include <render/shading/rasterizer_state_types.h>
#include <render/shading/sampler_types.h>
#include <external_libs/directxtk/WICTextureLoader.h>

using namespace DirectX;
using namespace xtest;
using namespace xtest::demo;
using namespace xtest::render::shading;
using namespace xtest::file;

MotionBlurDemoApp::MotionBlurDemoApp(HINSTANCE instance,
	const application::WindowSettings& windowSettings,
	const application::DirectxSettings& directxSettings,
	uint32 fps /*=60*/)
	: application::DirectxApp(instance, windowSettings, directxSettings, fps)
	, m_dirKeyLight()
	, m_dirFillLight()
	, m_rarelyChangedData()
	, m_isRarelyChangedDataDirty(true)
	, m_camera(math::ToRadians(60.f), math::ToRadians(115.f), 30.f, { 5.f, 4.f, -5.f }, { 0.f, 1.f, 0.f }, { math::ToRadians(4.f), math::ToRadians(175.f) }, { 3.f, 50.f })
	, m_objects()
	, m_shadowPass()
	, m_renderPass()
	, m_shadowMap(2048)
	, m_motionBlurMap()
	, m_colorRenderMap()
	, m_sceneBoundingSphere({ 0.f, 0.f, 0.f }, 21.f)
	, targetFPS(fps)
{}

MotionBlurDemoApp::~MotionBlurDemoApp()
{
}

void MotionBlurDemoApp::Init()
{
	application::DirectxApp::Init();

	m_camera.SetPerspectiveProjection(math::ToRadians(45.f), AspectRatio(), 1.f, 1000.f);
	
	InitLights();
	InitRenderTechnique();
	//CreateNewGPFMeshes();
	InitRenderables();
	
	service::Locator::GetMouse()->AddListener(this);
	service::Locator::GetKeyboard()->AddListener(this, { input::Key::F, input::Key::F1 });
}

void MotionBlurDemoApp::InitRenderables()
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

	/*render::Renderable Aircraft{ *(service::Locator::GetResourceLoader()->LoadGPFMesh(GetRootDir().append(LR"(\3d-objects\E-45-Aircraft\E 45 Aircraft.gpf)"))) };
	Aircraft.SetTransform(XMMatrixRotationY(math::ToRadians(45.f)) * XMMatrixTranslation(15.f, 10.f, -15.f));
	Aircraft.Init();
	previous_Transforms.push_back(Aircraft.GetTransform());
	m_objects.push_back(std::move(Aircraft));*/
}


void MotionBlurDemoApp::CreateNewGPFMeshes() {
	file::WriteGPFOnDiskFromObj(GetRootDir().append(LR"(\3d-objects\E-45-Aircraft\E 45 Aircraft_obj.obj)"), GetRootDir().append(LR"(\3d-objects\E-45-Aircraft\E 45 Aircraft.gpf)"), true);

}


void MotionBlurDemoApp::InitRenderTechnique()
{

	file::ResourceLoader* loader = service::Locator::GetResourceLoader();


	// shadow pass
	{
		m_shadowMap.SetTargetBoundingSphere(m_sceneBoundingSphere);
		m_shadowMap.SetLight(m_dirKeyLight.dirW);
		m_shadowMap.Init();

		m_rarelyChangedData.shadowMapResolution = float(m_shadowMap.Resolution());

		std::shared_ptr<VertexShader> vertexShader = std::make_shared<VertexShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\shadow_VS.cso")));
		vertexShader->SetVertexInput(std::make_shared<PosOnlyVertexInput>());
		vertexShader->AddConstantBuffer(CBufferFrequency::per_object, std::make_unique<CBuffer<PerObjectShadowMapData>>());

		m_shadowPass.SetState(std::make_shared<RenderPassState>(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_shadowMap.Viewport(), std::make_shared<SolidCullBackDepthBiasRS>(), nullptr, m_shadowMap.AsDepthStencilView()));
		m_shadowPass.SetVertexShader(vertexShader);
		m_shadowPass.Init();
	}

	// motion blur map pass
	{
		m_rarelyChangedData.useMotionBlurMap = true;

	// inizializzo la motion blur map
		m_motionBlurMap.SetViewAndProjectionMatrices(m_camera);
		m_motionBlurMap.Init(GetCurrentWidth(), GetCurrentHeight());
		//m_motionBlurMap.SetWidthHeight(GetCurrentWidth(), GetCurrentHeight());


		std::shared_ptr<VertexShader> vertexShader = std::make_shared<VertexShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\motion_blur_demo_motionblurmap_VS.cso")));
		vertexShader->SetVertexInput(std::make_shared<PosOnlyVertexInput>());
		vertexShader->AddConstantBuffer(CBufferFrequency::per_object, std::make_unique<CBuffer<MotionBlurMap::PerObjectMotionBlurMapData>>());

		std::shared_ptr<PixelShader> pixelShader = std::make_shared<PixelShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\motion_blur_demo_motionblurmap_PS.cso")));

		m_motionBlurPass.SetState(std::make_shared<RenderPassState>(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_motionBlurMap.Viewport(), std::make_shared<SolidCullBackRS>(), m_motionBlurMap.AsMotionBlurView(), m_depthBufferView.Get()));
		m_motionBlurPass.SetVertexShader(vertexShader);
		m_motionBlurPass.SetPixelShader(pixelShader);
		m_motionBlurPass.Init();
	}


	// render pass
	{
		m_colorRenderMap.Init(GetCurrentWidth(), GetCurrentHeight());
		//m_colorRenderMap.SetWidthHeight(GetCurrentWidth(), GetCurrentHeight());


		std::shared_ptr<VertexShader> vertexShader = std::make_shared<VertexShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\motion_blur_demo_VS.cso")));
		vertexShader->SetVertexInput(std::make_shared<MeshDataVertexInput>());
		vertexShader->AddConstantBuffer(CBufferFrequency::per_object, std::make_unique<CBuffer<PerObjectData>>());

		std::shared_ptr<PixelShader> pixelShader = std::make_shared<PixelShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\motion_blur_demo_PS.cso")));
		pixelShader->AddConstantBuffer(CBufferFrequency::per_object, std::make_unique<CBuffer<PerObjectData>>());
		pixelShader->AddConstantBuffer(CBufferFrequency::per_frame, std::make_unique<CBuffer<PerFrameData>>());
		pixelShader->AddConstantBuffer(CBufferFrequency::rarely_changed, std::make_unique<CBuffer<RarelyChangedData>>());
		pixelShader->AddSampler(SamplerUsage::common_textures, std::make_shared<AnisotropicSampler>());
		pixelShader->AddSampler(SamplerUsage::shadow_map, std::make_shared<PCFSampler>());

		m_renderPass.SetState(std::make_shared<RenderPassState>(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_colorRenderMap.Viewport(), std::make_shared<SolidCullBackRS>(), m_colorRenderMap.AsColorRenderTargetView(), m_depthBufferView.Get()));
		m_renderPass.SetVertexShader(vertexShader);
		m_renderPass.SetPixelShader(pixelShader);
		m_renderPass.Init();
	}

	// combine motion blur pass
	{
		std::shared_ptr<VertexShader> vertexShader = std::make_shared<VertexShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\combine_VS.cso")));
		vertexShader->SetVertexInput(std::make_shared<PosTexVertexInput>());

		std::shared_ptr<PixelShader> pixelShader = std::make_shared<PixelShader>(loader->LoadBinaryFile(GetRootDir().append(L"\\combine_PS.cso")));
		pixelShader->AddSampler(SamplerUsage::common_textures, std::make_shared<MotionBlurSampler>());
		pixelShader->AddConstantBuffer(CBufferFrequency::per_frame, std::make_unique<CBuffer<PerFrameData>>());
		pixelShader->AddConstantBuffer(CBufferFrequency::rarely_changed, std::make_unique<CBuffer<RarelyChangedData>>());

		m_combinePass.SetState(std::make_shared<RenderPassState>(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_viewport, std::make_shared<SolidCullBackRS>(), m_backBufferView.Get(), m_depthBufferView.Get()));
		m_combinePass.SetVertexShader(vertexShader);
		m_combinePass.SetPixelShader(pixelShader);
		m_combinePass.Init();
	}
	
}
void MotionBlurDemoApp::InitLights()
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

void MotionBlurDemoApp::OnResized()
{
	//questo metodo si trova in directx_app.cpp e chiama il metodo DirectxApp::ResizeBuffers()
	application::DirectxApp::OnResized();
	//nel metodo ResizeBuffers() si resetta m_backBufferView con il suo metodo Reset()
	//si chiama un metodo ResizeBuffers() sulla m_swapChain
	//si crea un nuovo ComPTR wrappato su ID3D11Texture2D
	//si crea una nuova RenderTargetView ( il puntatore è m_backBufferView )
	//si chiama CreateDepthStencilBuffer()
	//si chiama il metodo SetViewport(__args_)

	//.... nel caso semplice bisogna aggiornare solo il render pass.... qui ci sono altri pass
	
	//m_shadowPass: riga 107 non serve far niente, perchè ha una sua viewport un suo depthstencilbuffer, e nessun render target view

	//m_motionBlurPass: riga 127 ha una viewport e una render target view proprie, e usa lo stesso depth buffer "comune"(quello del render pass)
	//il metodo 'SetWidthHeight va a modificare width e height della motion blur map:
	//queste vengono usate per aggiornare la viewport
	//nella classe MotionBlurMap questo metodo va riscritto in modo tale da fare l'update anche di render target view e shader resource view
	m_motionBlurMap.SetWidthHeight(GetCurrentWidth(), GetCurrentHeight());

	m_motionBlurPass.GetState()->ChangeRenderTargetView(m_motionBlurMap.AsMotionBlurView());
	m_motionBlurPass.GetState()->ChangeViewPort(m_motionBlurMap.Viewport());
	m_motionBlurPass.GetState()->ChangeDepthStencilView(m_depthBufferView.Get());


	//update the render pass state with the resized render target and depth buffer
	//queste righe vanno modificate perchè il render pass non usa più m_backBufferView, m_depthBufferView, m_viewPort
	m_colorRenderMap.SetWidthHeight(GetCurrentWidth(), GetCurrentHeight());
	m_renderPass.GetState()->ChangeRenderTargetView(m_colorRenderMap.AsColorRenderTargetView());
	m_renderPass.GetState()->ChangeDepthStencilView(m_depthBufferView.Get());
	m_renderPass.GetState()->ChangeViewPort(m_colorRenderMap.Viewport());

	//m_combinePass
	m_combinePass.GetState()->ChangeRenderTargetView(m_backBufferView.Get());
	m_combinePass.GetState()->ChangeDepthStencilView(m_depthBufferView.Get());
	m_combinePass.GetState()->ChangeViewPort(m_viewport);


	//update the projection matrix with the new aspect ratio
	m_camera.SetPerspectiveProjection(math::ToRadians(45.f), AspectRatio(), 1.f, 1000.f);
	m_motionBlurMap.SetViewAndProjectionMatrices(m_camera);

}


void MotionBlurDemoApp::OnWheelScroll(input::ScrollStatus scroll)
{
	// zoom in or out when the scroll wheel is used
	if (service::Locator::GetMouse()->IsInClientArea())
	{
		m_camera.IncreaseRadiusBy(scroll.isScrollingUp ? -0.5f : 0.5f);
		m_motionBlurMap.SetViewAndProjectionMatrices(m_camera);

	}
}
void MotionBlurDemoApp::OnMouseMove(const DirectX::XMINT2& movement, const DirectX::XMINT2& currentPos)
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
}
void MotionBlurDemoApp::OnKeyStatusChange(input::Key key, const input::KeyStatus& status)
{
	if (key == input::Key::F && status.isDown)
	{
		m_camera.SetPivot({ 5.f, 4.f, -5.f });
		m_motionBlurMap.SetViewAndProjectionMatrices(m_camera);

	}
	else if (key == input::Key::F1 && status.isDown)
	{
		m_rarelyChangedData.useMotionBlurMap = !m_rarelyChangedData.useMotionBlurMap;
		m_isRarelyChangedDataDirty = true;
	}
}

float totalTime = 0.0f;

void MotionBlurDemoApp::UpdateScene(float deltaSeconds)
{
	XTEST_UNUSED_VAR(deltaSeconds);

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

	// PerFrameCB
	{
		PerFrameData data;
		data.dirLights[0] = m_dirKeyLight;
		data.dirLights[1] = m_dirFillLight;
		data.eyePosW = m_camera.GetPosition();
		data.blurMultiplier = (1.0f / deltaSeconds) / targetFPS;

		m_renderPass.GetPixelShader()->GetConstantBuffer(CBufferFrequency::per_frame)->UpdateBuffer(data);
		m_combinePass.GetPixelShader()->GetConstantBuffer(CBufferFrequency::per_frame)->UpdateBuffer(data);
	}


	// RarelyChangedCB
	if (m_isRarelyChangedDataDirty)
	{
		m_renderPass.GetPixelShader()->GetConstantBuffer(CBufferFrequency::rarely_changed)->UpdateBuffer(m_rarelyChangedData);
		m_combinePass.GetPixelShader()->GetConstantBuffer(CBufferFrequency::rarely_changed)->UpdateBuffer(m_rarelyChangedData);
		m_isRarelyChangedDataDirty = false;
	}
	
}


void MotionBlurDemoApp::RenderScene()
{
	m_d3dAnnotation->BeginEvent(L"shadow-map");
	m_shadowPass.Bind();
	m_shadowPass.GetState()->ClearDepthOnly();

	// draw SHADOW
	for (render::Renderable& renderable : m_objects) {
		for (const std::string& meshName : renderable.GetMeshNames()) {
			PerObjectShadowMapData data = ToPerObjectShadowMapData(renderable, meshName);
			m_shadowPass.GetVertexShader()->GetConstantBuffer(CBufferFrequency::per_object)->UpdateBuffer(data);
			renderable.Draw(meshName);
		}
	}
	m_d3dAnnotation->EndEvent();


	m_d3dAnnotation->BeginEvent(L"render-scene");
	m_renderPass.Bind();
	m_renderPass.GetState()->ClearDepthOnly();
	m_renderPass.GetState()->ClearRenderTarget(DirectX::Colors::DarkGray);
	m_renderPass.GetPixelShader()->BindTexture(TextureUsage::shadow_map, m_shadowMap.AsShaderView());

	// draw OBJECTS
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

	m_d3dAnnotation->BeginEvent(L"combine");
	m_combinePass.Bind();
	m_combinePass.GetState()->ClearRenderTarget(DirectX::Colors::White);
	m_combinePass.GetState()->ClearDepthOnly();
	m_combinePass.GetPixelShader()->BindTexture(TextureUsage::color, m_colorRenderMap.AsColorShaderView());
	m_combinePass.GetPixelShader()->BindTexture(TextureUsage::motionblur, m_motionBlurMap.AsShaderView());

	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.ByteWidth = sizeof(m_quad.vertices);
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexInitData;
	vertexInitData.pSysMem = m_quad.vertices;
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexInitData, &m_quadVertexBuffer));

	UINT stride = sizeof(Quad::VertexIn);
	UINT offset = 0;
	m_d3dContext->IASetVertexBuffers(0, 1, m_quadVertexBuffer.GetAddressOf(), &stride, &offset);
	m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_d3dContext->Draw(6, 0);

	m_d3dAnnotation->EndEvent();

	XTEST_D3D_CHECK(m_swapChain->Present(0, 0));
}


MotionBlurDemoApp::PerObjectData MotionBlurDemoApp::ToPerObjectData(const render::Renderable& renderable, const std::string& meshName)
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

MotionBlurDemoApp::PerObjectShadowMapData MotionBlurDemoApp::ToPerObjectShadowMapData(const render::Renderable& renderable, const std::string& meshName)
{
	XTEST_UNUSED_VAR(meshName);
	PerObjectShadowMapData data;

	XMMATRIX W = XMLoadFloat4x4(&renderable.GetTransform());
	XMMATRIX WVP = W * m_shadowMap.LightViewMatrix() * m_shadowMap.LightProjMatrix();

	XMStoreFloat4x4(&data.WVP_lightSpace, XMMatrixTranspose(WVP));
	return data;
}

MotionBlurDemoApp::PerObjectCombineData MotionBlurDemoApp::ToPerObjectCombineData(const render::Renderable& renderable, const std::string& meshName) {
	XTEST_UNUSED_VAR(meshName);
	PerObjectCombineData data;

	XMMATRIX W = XMLoadFloat4x4(&renderable.GetTransform());
	XMMATRIX V = m_camera.GetViewMatrix();
	XMMATRIX P = m_camera.GetProjectionMatrix();
	XMMATRIX WVP = W * V*P;

	XMStoreFloat4x4(&data.WVP, XMMatrixTranspose(WVP));
	return data;
}