#include "stdafx.h"
#include "motion_blur_map.h"
#include <service/locator.h>

using namespace xtest::render::shading;
using namespace DirectX;
using namespace xtest;
using namespace xtest::camera;


MotionBlurMap::MotionBlurMap(uint32 width, uint32 height):
	m_width(width),
	m_height(height),
	m_motionBlurView(nullptr)
{
	//è necessario settare questi due dati con la matrice identità
	XMStoreFloat4x4(&m_data.WVP_previousFrame, XMMatrixIdentity());
	XMStoreFloat4x4(&m_data.WVP_currentFrame, XMMatrixIdentity());
}


void MotionBlurMap::Init()
{
	// already initialized
	if (m_motionBlurView)
	{
		return;
	}

	ID3D11Device* d3dDevice = service::Locator::GetD3DDevice();


	// create the shadow map texture
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = m_width;
	textureDesc.Height = m_height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
}

//qui è necessario risolvere un problema riguardante il dato da inviare al buffer:
//per il fatto che esso deve contenere due matrici W, ognuna relativa a uno dei due frame consecutivi
//è necessario mettere l'oggetto fuori dal blocco del metodo ToPerObjectMotionBlurData

//le matrici V e P le prendo da m_camera? in questo caso non ho nessun membro m_camera
//XMMATRIX V = m_camera.GetViewMatrix();
//XMMATRIX P = m_camera.GetProjectionMatrix();
//XMMATRIX WVP = W * m_shadowMap.LightViewMatrix() * m_shadowMap.LightProjMatrix();


//in questo caso non c'è da calcolare delle matrici in base alla direzione della luce
//inoltre non vanno passate nuovamente all'esterno della classe con metodi simili a LightViewMatrix
//e a LightProjMatrix
//quindi non ho usato un valore di flag simile a m_isDirty
void MotionBlurMap::SetViewAndProjectionMatrices(const SphericalCamera& camera)
{
	XMStoreFloat4x4(&m_V, camera.GetViewMatrix());
	XMStoreFloat4x4(&m_P, camera.GetProjectionMatrix());

}

MotionBlurMap::PerObjectMotionBlurMapData MotionBlurMap::ToPerObjectMotionBlurMapData(const render::Renderable& renderable, const std::string& meshName)
{
	XTEST_UNUSED_VAR(meshName);
	m_data.dataSetFilled = false;

	//in teoria basta passare una sola matrice W per volta
	//questa, moltiplicata per V e P verrà salvata in WVP_currentFrame e poi passerà a previousFrame
	XMMATRIX W = XMLoadFloat4x4(&renderable.GetTransform());
	XMMATRIX WVP = W * XMLoadFloat4x4(&m_V)*  XMLoadFloat4x4(&m_P);

	m_data.WVP_previousFrame = m_data.WVP_currentFrame;
	XMStoreFloat4x4(&m_data.WVP_currentFrame, XMMatrixTranspose(WVP));
	XMFLOAT4X4 identityMatrix;
	XMStoreFloat4x4(&identityMatrix, XMMatrixIdentity());
	//questa 'if' serve per dire al vertex shader che sono pronte le matrici relative ai due frame consecutivi
	if (m_data.WVP_previousFrame.m != identityMatrix.m) {
		m_data.dataSetFilled = true;
	}
	return m_data;
}
