#include "stdafx.h"
#include "box_demo_app.h"
#include <file/file_utils.h>
#include <math/math_utils.h>


using namespace DirectX;
using namespace xtest;

using xtest::demo::BoxDemoApp;
using Microsoft::WRL::ComPtr;

BoxDemoApp::BoxDemoApp(HINSTANCE instance,
	const application::WindowSettings& windowSettings,
	const application::DirectxSettings& directxSettings,
	uint32 fps /*=60*/)
	: application::DirectxApp(instance, windowSettings, directxSettings, fps)
	, m_vertexBuffer(nullptr)
	, m_indexBuffer(nullptr)
	, m_vertexShader(nullptr)
	, m_pixelShader(nullptr)
	, m_inputLayout(nullptr)
{}


BoxDemoApp::~BoxDemoApp()
{}


void BoxDemoApp::Init()
{
	application::DirectxApp::Init();

	InitMatrices();
	InitShaders();
	InitBuffers();
	InitRasterizerState();
}


void BoxDemoApp::InitMatrices()
{

	//TODO:
	// Inizializza la World matrix con cui posizionare l'oggetto nel mondo e salvala nella variabile m_worldMatrix
	// Ricorda di utilizzare XMStoreFloat4x4 per scrivere una XMMATRIX in un XMFLOAT4x4
	//
	// hint: come prima prova potresti utilizzare la matrice identità XMMatrixIdentity 
	//       in modo che l'origine del sistema locale del tuo oggetto coincida con l'origine nel mondo
	//
	// hint 2: una volta fatto funzionare, prova a cambiare la world matrix ad esempio, traslando
	//		   l'oggetto di qualche unità nello spazio, prova a ruotarlo o a scalarlo. Ricorda che
	//         hai a disposizione funzioni come XMMatrixScaling, XMMatrixRotationY, XMMatrixTranslation
	//		   e simili per costruire le matrici di cui hai bisogno

	XMStoreFloat4x4(&m_worldMatrix, XMMatrixIdentity());

	// TODO:
	// Crea la view matrix utilizzando la funzione XMMatrixLookAtLH e salvala in m_viewMatrix
	// hint: come prima prova a posizionare la telecamera qualche unità lungo -Z e puntala all'origine del mondo
	//       dove, ad esempio, hai posizionato l'oggetto come prima prova

	XMVECTOR eyePosition = XMVectorSet(0, 0, -10, 1);
	XMVECTOR focusPoint = XMVectorSet(0, 0, 0, 1);
	XMVECTOR upDirection = XMVectorSet(0, 1, 0, 0);
	XMMATRIX viewMatrix = XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);
	XMStoreFloat4x4(&m_viewMatrix, viewMatrix);


	// TODO:
	// Crea una matrice di proiezione prospettica utilizzando la funzione XMMatrixPerspectiveFovLH e salvala 
	// all'interno di m_projectionMatrix, ricorda che puoi usare la funzione AspectRatio() di WindowApp per conoscere
	// il rapporto tra larghezza e altezza della finestra, hai inoltre a disposizione funzioni come math::ToRadians di
	// questo framework per convertire facilmente angoli da gradi a radianti
	//
	// hint: una volta che sei riuscito a disegnare a schermo prova a costruire una proiezione ortografica XMMatrixOrthographicLH
	//       invece di una prospettica

	XMMATRIX projMatrix = XMMatrixPerspectiveFovLH(math::ToRadians(45.f), AspectRatio(), 1.0f, 100.0f);
	XMStoreFloat4x4(&m_projectionMatrix, projMatrix);

}

void BoxDemoApp::InitShaders()
{
	// read pre-compiled shaders' bytecode
	std::future<file::BinaryFile> psByteCodeFuture = file::ReadBinaryFile(std::wstring(GetRootDir()).append(L"\\box_demo_PS.cso"));
	std::future<file::BinaryFile> vsByteCodeFuture = file::ReadBinaryFile(std::wstring(GetRootDir()).append(L"\\box_demo_VS.cso"));

	// future.get() can be called only once
	file::BinaryFile vsByteCode = vsByteCodeFuture.get();
	file::BinaryFile psByteCode = psByteCodeFuture.get();

	// TODO:
	// Crea il vertex shader e il pixel shader utilizzando le funzioni CreateVertexShader e CreatePixelShader di ID3D11Device
	// utilizzando le variabili vsByteCode e psByteCode create poco sopra, salva i due shader nei membri m_vertexShader
	// e m_pixelShader

	XTEST_D3D_CHECK(m_d3dDevice->CreateVertexShader(vsByteCode.Data(), vsByteCode.ByteSize(), nullptr, m_vertexShader.GetAddressOf()));
	XTEST_D3D_CHECK(m_d3dDevice->CreatePixelShader(psByteCode.Data(), psByteCode.ByteSize(), nullptr, m_pixelShader.GetAddressOf()));


	// TODO: Crea l'input layout per il vertex shader:
	// 1. Controlla nel file box_demo_VS.hlsl la struttura chiamata VertexIn, questa struttura è l'esatto match di quella lato
	//    c++ chiamata anch'essa VertexIn ma definita nell'header di questa classe (BoxDemoApp)
	// 2. Crae un array di due D3D11_INPUT_ELEMENT_DESC, nella prima dovrai descrivere come interpretare le posizioni, nella seconda 
	//	  i colori. I semantic name sono i nomi che appaiono dopo i ":" nella struttura dichiarata nel .hlsl. Per i formati utilizza 
	//    nel primo caso un formato RGB a 32 bit per canale con codifica float, per il secondo un formato RGBA a 32 bit per canale sempre
	//    con codifica float perché nel nostro caso stiamo passando un float3 e un float4. 
	//    Come input slot utilizza sempre lo slot 0. Attenzione all'AlignedByteOffset che nel secondo caso sarà diverso da 0 e dovrà essere
	//    uguale al numero di byte che separano il float4 dei colori dal float3 delle posizioni nella struttura VertexIn definita nell'header 
	//    di BoxDemoApp, ricorda che puoi usare la macro "offsetof" a tale scopo. Come InputSlotClass utilizza sempre D3D11_INPUT_PER_VERTEX_DATA 
	//    mentre come InstanceDataStepRate utilizza sempre 0
	// 3. Crea l'input layout tramite la funzione CreateInputLayout di ID3D11Device e salvalo nel membro m_inputLayout, ricorda che hai a disposizione
	//    il bytecode del vertex shader nella variabile vsByteCode

	D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexIn, color), D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	XTEST_D3D_CHECK(m_d3dDevice->CreateInputLayout(vertexLayoutDesc,
		_countof(vertexLayoutDesc),
		vsByteCode.Data(),
		vsByteCode.ByteSize(),
		m_inputLayout.GetAddressOf()));

}


void BoxDemoApp::InitBuffers()
{

	// TODO: Creare un vertex buffer:
	// 1. Crea un array di VertexIn per definire i vertici del tuo oggetto, inizia in modo molto
	//    semplice, magari definendo un piano/quad/quadrato fatto da due soli triangoli, ricorda che puoi usare
	//    le initializer-list per rendere il codice più conciso. Nella nostra esercitazione
	//    useremo anche un index buffer, quindi nel tuo array di vertici, nel caso volessi fare
	//    un quadrato di due triangoli, dovrai specificare solo 4 vertici al posto di 6 (3 per triangolo) 
	//    visto che specificheremo come sono composti i triangoli tramite l'index buffer
	// 2. Riempi la struttura D3D11_BUFFER_DESC che descriverà il tuo vertex buffer, fai attenzione
	//    a specificare l'Usage corretto, ricorda, la risorsa dovrà essere inizializzata una sola volta, 
	//    non cambierà mai più e solo la GPU ne leggerà il contenuto da quel momento in poi. Come ByteWith
	//    dovrai specificare la grandezza in byte dei dati del buffer in questo caso del tuo array di vertici 
	//    creato al punto 1. Nel bind flag dovrai specificare che si tratta di un bind di vertex buffer.
	//    Il nostro buffer non è structured quindi StructureByteStride sarà 0.
	// 3. Crea una istanza di D3D11_SUBRESOURCE_DATA e fai sì che il suo puntatore pSysMem punti all'array di
	//    vertici creato al punto 1, cosicchè durante la creazione del vertex buffer questo sia inizializzato
	//    con i dati da te definiti
	// 4. Crea il vertex buffer tramite la funzione CreateBuffer di ID3D11Device specificando la sub resource
	//    del punto 3 e salvando il buffer creato nel membro m_vertexBuffer

	VertexIn vertices[] =
	{
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(DirectX::Colors::Red) }, // 0
		{ XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT4(DirectX::Colors::Red) }, // 1
		{ XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT4(DirectX::Colors::Green) }, // 2
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(DirectX::Colors::Green) }, // 3
		{ XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT4(DirectX::Colors::Yellow) }, // 4
		{ XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT4(DirectX::Colors::Yellow) }, // 5
		{ XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT4(DirectX::Colors::Blue) }, // 6
		{ XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT4(DirectX::Colors::Blue) }  // 7
	};

	D3D11_BUFFER_DESC vertexBufferDesc;

	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.ByteWidth = sizeof(vertices);
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA subresourceData;
	ZeroMemory(&subresourceData, sizeof(D3D11_SUBRESOURCE_DATA));
	subresourceData.pSysMem = vertices;
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &subresourceData, &m_vertexBuffer));


	// TODO: Craere un index buffer:
	// 1. Crea un array di indici di tipo uint32, ogni indice si riferirà ad un preciso vertice nel vertex buffer,
	//    e verrà utilizzato per specificare come è costruito un triangolo, nel nostro caso i due triangoli (se stai
	//    creando un quad di due triangoli). Pensa a questi indici come fossero usati per accedere all'array di vertici
	//    creato poco sopra, se vuoi disegnare un piano, avrai bisogno di 6 indici che utilizzeranno i 4 vertici 
	//    definiti prima per creare due triangoli. Ricorda, definisci un triangolo in modo che i suoi vertici siano definiti
	//    in senso anti-orario altrimenti la faccia del triangolo sarà rivolta verso l'interno
	// 2. Riempi la struttura D3D11_BUFFER_DESC che descriverà il tuo index buffer, l'usage sarà uguale a quello specificato
	//    nel vertex buffer visto che lo utilizzermo alla stessa modo. Come ByteWith dovrai specificare la grandezza in byte 
	//    dei dati del buffer in questo caso del tuo array di indici. Nel bind flag dovrai specificare che si tratta di un bind 
	//    di index buffer. Il nostro buffer non è structured quindi StructureByteStride sarà 0.
	// 3. Crea una istanza di D3D11_SUBRESOURCE_DATA e fai sì che il suo puntatore pSysMem punti all'array di
	//    indici creato al punto 1, cosicchè durante la creazione dell'index buffer questo sia inizializzato
	//    con i dati da te specificati
	// 4. Crea l'index buffer tramite la funzione CreateBuffer di ID3D11Device specificando la sub resource
	//    del punto 3 e salvando il buffer creato nel membro m_indexBuffer

	uint32 indicies[36] =
	{
		0, 1, 2, 0, 2, 3,
		4, 6, 5, 4, 7, 6,
		4, 5, 1, 4, 1, 0,
		3, 2, 6, 3, 6, 7,
		1, 5, 6, 1, 6, 2,
		4, 0, 3, 4, 3, 7
	};

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.ByteWidth = sizeof(indicies);
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;

	ZeroMemory(&subresourceData, sizeof(D3D11_SUBRESOURCE_DATA));
	subresourceData.pSysMem = indicies;
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &subresourceData, &m_indexBuffer));


	// TODO: Creare il CostantBuffer PerObjectCB che verrà utilizzato per fornire al vertex shader la matrice composta WVP
	// 1. controlla il cbuffer PerObjectCB definito nel file box_demo_VS.hlsl e confrontalo con la struttura (chiamata nello 
	//    stesso modo) definita lato c++ nell'header di questa classe.
	// 2. Riempi la struttura D3D11_BUFFER_DESC che descriverà il tuo constant buffer, l'usage dovrà garantire accesso in 
	//    lettura e scrittura sia alla GPU che alla CPU. Come ByteWith puoi specificare la grandezza della struttura PerObjectCB.
	//    Nel bind flag dovrai specificare che si tratta di un bind di un constant buffer. CPUAccessFlags dovrai specificare il flag
	//    corretto in modo che la CPU possa scrivere. Il nostro buffer non è structured quindi StructureByteStride sarà 0.
	// 3. Crea il costant buffer tramite la funzione CreateBuffer di ID3D11Device, non avendo dati con cui inizializzarlo e grazie 
	//    al fatto che sarà possibile modificarne il contenuto più tardi, puoi specificare nullptr
	D3D11_BUFFER_DESC vsConstantBufferDesc;
	vsConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vsConstantBufferDesc.ByteWidth = sizeof(PerObjectCB);
	vsConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	vsConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vsConstantBufferDesc.MiscFlags = 0;
	vsConstantBufferDesc.StructureByteStride = 0;
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vsConstantBufferDesc, nullptr, &m_vsConstantBuffer));
}


void BoxDemoApp::InitRasterizerState()
{

	// TODO: Crea il rasterizer state con cui configureremo il RasterizerStage prima di disegnare a schermo
	// 1. riempi la struttura D3D11_RASTERIZER_DESC, 
	// 2. crae un rasterizer state tramite la funzione CreateRasterizerState di ID3D11Device e salvalo nella
	//    variabile membro m_rasterizerState

	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));

	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.ScissorEnable = FALSE;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state object.
	XTEST_D3D_CHECK(m_d3dDevice->CreateRasterizerState(&rasterizerDesc, &m_rasterizerState));
}


void BoxDemoApp::OnResized()
{
	application::DirectxApp::OnResized();

	// TODO:
	// quando la finestra viene ridimensionata dovresti aggiornare la matrice di proiezione, quindi ricostruiscine
	// una nuova utilizzando il nuovo aspect ratio e storicizzala sempre in m_projectionMatrix

	XMMATRIX projMatrix = XMMatrixPerspectiveFovLH(math::ToRadians(45.f), AspectRatio(), 1.0f, 100.0f);
	XMStoreFloat4x4(&m_projectionMatrix, projMatrix);
}


void BoxDemoApp::UpdateScene(float deltaSeconds)
{

	// TODO: creare la matrice WorldViewProjection da passare al vertex shader tramite il costant buffer PerObjectCB:
	// 1. questo metodo (UpdateScene) è chiamato ogni frame per aggiornare la logica dell'applicazione, qui ad esempio
	//    potrai modificare la world matrix ogni frame per far girare l'oggetto su se stesso ad sempio di 30 gradi
	//    al secondo sull'asse Y. 
	//
	// hint: inizialmente conviene mantenere la world, view e projection matrix costanti
	//
	// 2. Carica, grazie ai metodi XMLoadFloat4x4, le matrici m_worldMatrix, m_viewMatrix e m_projectionMatrix
	//    in tipi XMMATRIX e costruisci la matrice finale moltiplicandole tra loro nell'ordine corretto WVP

	XMVECTOR eyePosition = XMVectorSet(0, 0, -10, 1);
	XMVECTOR focusPoint = XMVectorSet(0, 0, 0, 1);
	XMVECTOR upDirection = XMVectorSet(0, 1, 0, 0);
	XMMATRIX viewMatrix = XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);
	XMStoreFloat4x4(&m_viewMatrix, viewMatrix);

	static float angle = 0.0f;
	angle += 90.0f * deltaSeconds;
	XMVECTOR rotationAxis = XMVectorSet(0, 1, 1, 0);

	XMMATRIX worldMatrix = XMMatrixRotationAxis(rotationAxis, XMConvertToRadians(angle));
	XMStoreFloat4x4(&m_worldMatrix, worldMatrix);


	XMMATRIX projMatrix = XMLoadFloat4x4(&m_projectionMatrix);
	XMStoreFloat4x4(&m_projectionMatrix, projMatrix);

	XMMATRIX WVP = worldMatrix * viewMatrix * projMatrix;

	// TODO: aggiorna il costant buffer in modo che al vertex shader arrivi la nuova versione di WVP
	// 1. Crea una D3D11_MAPPED_SUBRESOURCE e inizializzala utilizzando ZeroMemory
	// 2. Utilizza il metodo Map di ID3D11DeviceContext per permettere alla CPU di accedere al costant 
	//    buffer m_vsConstantBuffer passando tra i paramentri la subresource createa al punto 1
	// 3. Casta il puntatore pData della subresource a PerObjectCB (la struttura che identifica il constant
	//    buffer lato c++)
	// 4. setta tramite XMStoreFloat4x4 il membro WVP del PerObjectCB ottenuto al punto 3. con la TRASPOSTA 
	//    della matrice WVP calcolata precedentemente (la trasposta è obbligatoria dato che hlsl ha un layout
	//    column major)
	// 5. Utilizza il metodo Unmap di ID3D11DeviceContext per avvertire che la modifica da parte della CPU 
	//    sul buffer m_vsConstantBuffer è completa

	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	ZeroMemory(&mappedSubresource, sizeof(mappedSubresource));

	XTEST_D3D_CHECK(m_d3dContext.Get()->Map(m_vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource));

	PerObjectCB* pDestination = reinterpret_cast<PerObjectCB*>(mappedSubresource.pData);

	XMStoreFloat4x4(&pDestination->WVP, XMMatrixTranspose(WVP));

	m_d3dContext->Unmap(m_vsConstantBuffer.Get(), 0);
}


void BoxDemoApp::RenderScene()
{

	// TODO: pulisci il depth/stencil buffer e il back buffer 
	// 1. utilizza ClearDepthStencilView di ID3D11DeviceContext, specifica nei ClearFlags sia quello per pulire
	//    il depth buffer che quello per pulire lo stencil buffer mettendoli in or "|", come valore di pulizia per
	//    il depth utilizza 1.0 mentre per lo stencil 0.0 nonostante al momento non useremo lo stencil
	// 2. utilizza ClearRenderTargetView di ID3D11DeviceContext per riempire il back buffer col colore da te specificato
	//
	// hint: prima di provare a disegnare qualsiasi geometria prova a utilizzare solo il comando per il clear del render target
	//       in modo da controllare che il colore da te specificato sia presente come sfondo della finestra renderizzata, in quel 
	//       caso puoi essere sicuro della corretta inizializzazione di DirectX, naturalmente dovrai almeno invocare il metodo 
	//       present descritto più sotto altrimenti non vedrai nessun cambiamento a schermo
	m_d3dContext->ClearDepthStencilView(m_depthBufferView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_d3dContext->ClearRenderTargetView(m_backBufferView.Get(), DirectX::Colors::Gray);

	// TODO: prepara tutti gli stati per disegnare:
	// 1. setta il rasterizer state creato in precedenza e salvato all'intero di m_rasterizerState
	//    tramite il metodo RSSetState di ID3D11DeviceContext
	// 2. setta setta l'input layout creato in precedenza e salvato all'intero di m_inputLayout
	//    tramite il metodo IASetInputLayout di ID3D11DeviceContext
	// 3. setta il vertex shader creato in precedenza e salvato all'intero di m_vertexShader
	//    tramite il metodo VSSetShader di ID3D11DeviceContext
	// 4. setta il pixel shader creato in precedenza e salvato all'intero di m_pixelShader
	//    tramite il metodo PSSetShader di ID3D11DeviceContext
	// 5. setta il costant buffer salvato all'interno di m_vsConstantBuffer tramite il metodo 
	//    VSSetConstantBuffers, come StartSlot devi usare lo stesso registro definito nel file hlsl
	//    nel nostro caso PerObjectCB sta utilizzando il registro 0
	// 6. setta il vertex buffer salvato in precedenza in m_vertexBuffer utilizzando il metodo
	//    IASetVertexBuffers di ID3D11DeviceContext, come slot utilizza sempre lo slot 0, attenzione a 
	//    settare correttamente lo stride
	// 7. setta l'index buffer salvato in precedenza in m_indexBuffer utilizzando il metodo
	//    IASetIndexBuffer di ID3D11DeviceContext, come formato, se avevi utilizzato uint32 per il
	//    tuo array di indici dovrai specificare un formato R a 32 bit con codifica uint
	// 8. configura il tipo di primitiva che vogliamo disegnare come triangle list tramite la funzione
	//    IASetPrimitiveTopology di ID3D11DeviceContext
	m_d3dContext->RSSetState(m_rasterizerState.Get());
	m_d3dContext->IASetInputLayout(m_inputLayout.Get());
	m_d3dContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	m_d3dContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	UINT bufferNumber = 0;
	m_d3dContext->VSSetConstantBuffers(bufferNumber, 1, m_vsConstantBuffer.GetAddressOf());

	UINT stride = sizeof(VertexIn);
	UINT offset = 0;
	m_d3dContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	m_d3dContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// TODO: disgna a schermo
	// 1. visto che stiamo utilizzando sia un vertex buffer che un index buffer devi utilizzare la chiamata
	//    DrawIndexed di ID3D11DeviceContext
	// 2. chiama la funzione present per mostrare a schermo il contenuto del back buffer tramite questa chiamata:
	//	  XTEST_D3D_CHECK(m_swapChain->Present(0, 0));
	m_d3dContext->DrawIndexed(36, 0, 0);
	XTEST_D3D_CHECK(m_swapChain->Present(0, 0));
}

