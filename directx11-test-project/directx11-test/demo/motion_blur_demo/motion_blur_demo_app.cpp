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

MotionBlurDemoApp::MotionBlurDemoApp(HINSTANCE instance, const application::WindowSettings& windowSettings, const application::DirectxSettings& directxSettings, uint32 fps)
	: application::DirectxApp(instance, windowSettings, directxSettings, fps),
	m_dirKeyLight()
	, m_dirFillLight()
	, m_rarelyChangedData()
	, m_camera(math::ToRadians(80.f), math::ToRadians(170.f), 15.f, { 5.f, 4.f, -5.f }, { 0.f, 1.f, 0.f }, { math::ToRadians(4.f), math::ToRadians(175.f) }, { 3.f, 50.f })
	, m_motionBlurPass()
	, m_renderPass()
{
}



MotionBlurDemoApp::~MotionBlurDemoApp()
{
}

void MotionBlurDemoApp::Init()
{
	application::DirectxApp::Init();

	m_camera.SetPerspectiveProjection(math::ToRadians(45.f), AspectRatio(), 1.f, 1000.f);
}

void MotionBlurDemoApp::InitRenderables()
{
}

void MotionBlurDemoApp::InitRenderTechnique()
{
	file::ResourceLoader* loader = service::Locator::GetResourceLoader();

	m_rarelyChangedData.useMotionBlurMap = true;

	// motion blur pass
	{
	}


	// render pass
	{
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

}
MotionBlurDemoApp::PerObjectData MotionBlurDemoApp::ToPerObjectData(const render::Renderable& renderable, const std::string& meshName)
{

}

