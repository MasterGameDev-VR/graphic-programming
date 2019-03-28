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

	// DONE
	// TODO:
	// Inizializza la World matrix con cui posizionare l'oggetto nel mondo e salvala nella variabile m_worldMatrix
	// Ricorda di utilizzare XMStoreFloat4x4 per scrivere una XMMATRIX in un XMFLOAT4x4
	//
	// hint: come prima prova potresti utilizzare la matrice identit� XMMatrixIdentity 
	//       in modo che l'origine del sistema locale del tuo oggetto coincida con l'origine nel mondo
	//
	// hint 2: una volta fatto funzionare, prova a cambiare la world matrix ad esempio, traslando
	//		   l'oggetto di qualche unit� nello spazio, prova a ruotarlo o a scalarlo. Ricorda che
	//         hai a disposizione funzioni come XMMatrixScaling, XMMatrixRotationY, XMMatrixTranslation
	//		   e simili per costruire le matrici di cui hai bisogno

	XMMATRIX worldMatr = XMMatrixIdentity();
	// optional operations
	XMStoreFloat4x4(&m_worldMatrix, worldMatr);


	// DONE
	// TODO:
	// Crea la view matrix utilizzando la funzione XMMatrixLookAtLH e salvala in m_viewMatrix
	// hint: come prima prova a posizionare la telecamera qualche unit� lungo -Z e puntala all'origine del mondo
	//       dove, ad esempio, hai posizionato l'oggetto come prima prova

	XMVECTOR eyePos = XMVectorSet(-4.0f, 2.5f, -4.0f, 0.0f);
	XMVECTOR focusPos = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR upDir = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX viewMatr = XMMatrixLookAtLH(eyePos, focusPos, upDir);

	XMStoreFloat4x4(&m_viewMatrix, viewMatr);


	// DONE
	// TODO:
	// Crea una matrice di proiezione prospettica utilizzando la funzione XMMatrixPerspectiveFovLH e salvala 
	// all'interno di m_projectionMatrix, ricorda che puoi usare la funzione AspectRatio() di WindowApp per conoscere
	// il rapporto tra larghezza e altezza della finestra, hai inoltre a disposizione funzioni come math::ToRadians di
	// questo framework per convertire facilmente angoli da gradi a radianti
	//
	// hint: una volta che sei riuscito a disegnare a schermo prova a costruire una proiezione ortografica XMMatrixOrthographicLH
	//       invece di una prospettica
	
	XMMATRIX projMatr = XMMatrixPerspectiveFovLH(math::ToRadians(45.0f), AspectRatio(), 1.0f, 100.0f);
	XMStoreFloat4x4(&m_projectionMatrix, projMatr);

}

void BoxDemoApp::InitShaders()
{
	// read pre-compiled shaders' bytecode
	std::future<file::BinaryFile> psByteCodeFuture = file::ReadBinaryFile(std::wstring(GetRootDir()).append(L"\\box_demo_PS.cso"));
	std::future<file::BinaryFile> vsByteCodeFuture = file::ReadBinaryFile(std::wstring(GetRootDir()).append(L"\\box_demo_VS.cso"));

	// future.get() can be called only once
	file::BinaryFile vsByteCode = vsByteCodeFuture.get();
	file::BinaryFile psByteCode = psByteCodeFuture.get();


	// DONE
	// TODO:
	// Crea il vertex shader e il pixel shader utilizzando le funzioni CreateVertexShader e CreatePixelShader di ID3D11Device
	// utilizzando le variabili vsByteCode e psByteCode create poco sopra, salva i due shader nei membri m_vertexShader
	// e m_pixelShader
	
	XTEST_D3D_CHECK(m_d3dDevice->CreateVertexShader(vsByteCode.Data(), vsByteCode.ByteSize(), nullptr, &m_vertexShader));
	XTEST_D3D_CHECK(m_d3dDevice->CreatePixelShader(psByteCode.Data(), psByteCode.ByteSize(), nullptr, &m_pixelShader));


	// DONE
	// TODO: Crea l'input layout per il vertex shader:
	// 1. Controlla nel file box_demo_VS.hlsl la struttura chiamata VertexIn, questa struttura � l'esatto match di quella lato
	//    c++ chiamata anch'essa VertexIn ma definita nell'header di questa classe (BoxDemoApp)
	// 2. Crae un array di due D3D11_INPUT_ELEMENT_DESC, nella prima dovrai descrivere come interpretare le posizioni, nella seconda 
	//	  i colori. I semantic name sono i nomi che appaiono dopo i ":" nella struttura dichiarata nel .hlsl. Per i formati utilizza 
	//    nel primo caso un formato RGB a 32 bit per canale con codifica float, per il secondo un formato RGBA a 32 bit per canale sempre
	//    con codifica float perch� nel nostro caso stiamo passando un float3 e un float4. 
	//    Come input slot utilizza sempre lo slot 0. Attenzione all'AlignedByteOffset che nel secondo caso sar� diverso da 0 e dovr� essere
	//    uguale al numero di byte che separano il float4 dei colori dal float3 delle posizioni nella struttura VertexIn definita nell'header 
	//    di BoxDemoApp, ricorda che puoi usare la macro "offsetof" a tale scopo. Come InputSlotClass utilizza sempre D3D11_INPUT_PER_VERTEX_DATA 
	//    mentre come InstanceDataStepRate utilizza sempre 0
	// 3. Crea l'input layout tramite la funzione CreateInputLayout di ID3D11Device e salvalo nel membro m_inputLayout, ricorda che hai a disposizione
	//    il bytecode del vertex shader nella variabile vsByteCode

	D3D11_INPUT_ELEMENT_DESC verticesDesc[2] = 
	{
		{
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA
		},
		{
			"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(VertexIn, color), D3D11_INPUT_PER_VERTEX_DATA
		}
	};

	XTEST_D3D_CHECK(m_d3dDevice->CreateInputLayout(verticesDesc, 2, vsByteCode.Data(), vsByteCode.ByteSize(), &m_inputLayout));

}


void BoxDemoApp::InitBuffers()
{
	// DONE
	// TODO: Creare un vertex buffer:
	// 1. Crea un array di VertexIn per definire i vertici del tuo oggetto, inizia in modo molto
	//    semplice, magari definendo un piano/quad/quadrato fatto da due soli triangoli, ricorda che puoi usare
	//    le initializer-list per rendere il codice pi� conciso. Nella nostra esercitazione
	//    useremo anche un index buffer, quindi nel tuo array di vertici, nel caso volessi fare
	//    un quadrato di due triangoli, dovrai specificare solo 4 vertici al posto di 6 (3 per triangolo) 
	//    visto che specificheremo come sono composti i triangoli tramite l'index buffer
	// 2. Riempi la struttura D3D11_BUFFER_DESC che descriver� il tuo vertex buffer, fai attenzione
	//    a specificare l'Usage corretto, ricorda, la risorsa dovr� essere inizializzata una sola volta, 
	//    non cambier� mai pi� e solo la GPU ne legger� il contenuto da quel momento in poi. Come ByteWith
	//    dovrai specificare la grandezza in byte dei dati del buffer in questo caso del tuo array di vertici 
	//    creato al punto 1. Nel bind flag dovrai specificare che si tratta di un bind di vertex buffer.
	//    Il nostro buffer non � structured quindi StructureByteStride sar� 0.
	// 3. Crea una istanza di D3D11_SUBRESOURCE_DATA e fai s� che il suo puntatore pSysMem punti all'array di
	//    vertici creato al punto 1, cosicch� durante la creazione del vertex buffer questo sia inizializzato
	//    con i dati da te definiti
	// 4. Crea il vertex buffer tramite la funzione CreateBuffer di ID3D11Device specificando la sub resource
	//    del punto 3 e salvando il buffer creato nel membro m_vertexBuffer

	VertexIn vertices[] = 
	{
		{ XMFLOAT3(-1.f, -1.f, -1.f), XMFLOAT4(DirectX::Colors::Red)},					// 0
		{ XMFLOAT3(+1.f, -1.f, -1.f), XMFLOAT4(DirectX::Colors::OrangeRed)},			// 1
		{ XMFLOAT3(-1.f, +1.f, -1.f), XMFLOAT4(DirectX::Colors::Blue)},					// 2
		{ XMFLOAT3(-1.f, -1.f, +1.f), XMFLOAT4(DirectX::Colors::Orange)},				// 3
		{ XMFLOAT3(+1.f, +1.f, -1.f), XMFLOAT4(DirectX::Colors::MediumBlue)},			// 4
		{ XMFLOAT3(-1.f, +1.f, +1.f), XMFLOAT4(DirectX::Colors::DarkBlue)},				// 5
		{ XMFLOAT3(+1.f, -1.f, +1.f), XMFLOAT4(DirectX::Colors::DarkOrange)},			// 6
		{ XMFLOAT3(+1.f, +1.f, +1.f), XMFLOAT4(DirectX::Colors::BlueViolet)}			// 7
	};

	D3D11_BUFFER_DESC vertexBuff;
	vertexBuff.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBuff.ByteWidth = sizeof(vertices);
	vertexBuff.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBuff.CPUAccessFlags = 0;
	vertexBuff.MiscFlags = 0;
	vertexBuff.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = vertices;

	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBuff, &vertexData, &m_vertexBuffer));
	

	// DONE
	// TODO: Craere un index buffer:
	// 1. Crea un array di indici di tipo uint32, ogni indice si riferir� ad un preciso vertice nel vertex buffer,
	//    e verr� utilizzato per specificare come � costruito un triangolo, nel nostro caso i due triangoli (se stai
	//    creando un quad di due triangoli). Pensa a questi indici come fossero usati per accedere all'array di vertici
	//    creato poco sopra, se vuoi disegnare un piano, avrai bisogno di 6 indici che utilizzeranno i 4 vertici 
	//    definiti prima per creare due triangoli. Ricorda, definisci un triangolo in modo che i suoi vertici siano definiti
	//    in senso anti-orario altrimenti la faccia del triangolo sar� rivolta verso l'interno
	// 2. Riempi la struttura D3D11_BUFFER_DESC che descriver� il tuo index buffer, l'usage sar� uguale a quello specificato
	//    nel vertex buffer visto che lo utilizzermo alla stessa modo. Come ByteWith dovrai specificare la grandezza in byte 
	//    dei dati del buffer in questo caso del tuo array di indici. Nel bind flag dovrai specificare che si tratta di un bind 
	//    di index buffer. Il nostro buffer non � structured quindi StructureByteStride sar� 0.
	// 3. Crea una istanza di D3D11_SUBRESOURCE_DATA e fai s� che il suo puntatore pSysMem punti all'array di
	//    indici creato al punto 1, cosicch� durante la creazione dell'index buffer questo sia inizializzato
	//    con i dati da te specificati
	// 4. Crea l'index buffer tramite la funzione CreateBuffer di ID3D11Device specificando la sub resource
	//    del punto 3 e salvando il buffer creato nel membro m_indexBuffer

	uint32 indices[] = {
	// front
	0, 2, 4,
	4, 1, 0,
	// back
	3, 6, 7,
	7, 5, 3,
	// left
	0, 3, 5,
	5, 2, 0,
	//right
	1, 4, 7,
	7, 6, 1,
	// up
	2, 5, 7,
	7, 4, 2,
	// down
	0, 3, 6,
	6, 1, 0
	};

	D3D11_BUFFER_DESC indexBuff;
	indexBuff.Usage = D3D11_USAGE_IMMUTABLE;
	indexBuff.ByteWidth = sizeof(indices);
	indexBuff.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBuff.CPUAccessFlags = 0;
	indexBuff.MiscFlags = 0;
	indexBuff.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = &indices;

	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBuff, &indexData, &m_indexBuffer));


	// DONE
	// TODO: Creare il CostantBuffer PerObjectCB che verr� utilizzato per fornire al vertex shader la matrice composta WVP
	// 1. controlla il cbuffer PerObjectCB definito nel file box_demo_VS.hlsl e confrontalo con la struttura (chiamata nello 
	//    stesso modo) definita lato c++ nell'header di questa classe.
	// 2. Riempi la struttura D3D11_BUFFER_DESC che descriver� il tuo constant buffer, l'usage dovr� garantire accesso in 
	//    lettura e scrittura sia alla GPU che alla CPU. Come ByteWith puoi specificare la grandezza della struttura PerObjectCB.
	//    Nel bind flag dovrai specificare che si tratta di un bind di un constant buffer. CPUAccessFlags dovrai specificare il flag
	//    corretto in modo che la CPU possa scrivere. Il nostro buffer non � structured quindi StructureByteStride sar� 0.
	// 3. Crea il costant buffer tramite la funzione CreateBuffer di ID3D11Device, non avendo dati con cui inizializzarlo e grazie 
	//    al fatto che sar� possibile modificarne il contenuto pi� tardi, puoi specificare nullptr

	D3D11_BUFFER_DESC vertexShaderBuff;
	vertexShaderBuff.Usage = D3D11_USAGE_DYNAMIC;
	vertexShaderBuff.ByteWidth = sizeof(PerObjectCB);
	vertexShaderBuff.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	vertexShaderBuff.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexShaderBuff.MiscFlags = 0;
	vertexShaderBuff.StructureByteStride = 0;

	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexShaderBuff, nullptr, &m_vsConstantBuffer));

}


void BoxDemoApp::InitRasterizerState()
{
	// DONE
	// TODO: Crea il rasterizer state con cui configureremo il RasterizerStage prima di disegnare a schermo
	// 1. riempi la struttura D3D11_RASTERIZER_DESC, 
	// 2. crae un rasterizer state tramite la funzione CreateRasterizerState di ID3D11Device e salvalo nella
	//    variabile membro m_rasterizerState

	D3D11_RASTERIZER_DESC rasterizer;
	rasterizer.FillMode = D3D11_FILL_SOLID;
	rasterizer.CullMode = D3D11_CULL_BACK;
	rasterizer.FrontCounterClockwise = false;
	rasterizer.DepthBias = 0;
	rasterizer.DepthBiasClamp = 0.f;
	rasterizer.SlopeScaledDepthBias = 0.f;
	rasterizer.DepthClipEnable = false;
	rasterizer.ScissorEnable = false;
	rasterizer.MultisampleEnable = false;
	rasterizer.AntialiasedLineEnable = false;

	XTEST_D3D_CHECK(m_d3dDevice->CreateRasterizerState(&rasterizer, &m_rasterizerState));

}


void BoxDemoApp::OnResized()
{
	application::DirectxApp::OnResized();

	// DONE
	// TODO:
	// quando la finestra viene ridimensionata dovresti aggiornare la matrice di proiezione, quindi ricostruiscine
	// una nuova utilizzando il nuovo aspect ratio e storicizzala sempre in m_projectionMatrix

	XMMATRIX projMatr = XMMatrixPerspectiveFovLH(math::ToRadians(45.0f), AspectRatio(), 1.0f, 100.0f);
	XMStoreFloat4x4(&m_projectionMatrix, projMatr);

}


void BoxDemoApp::UpdateScene(float deltaSeconds) 
{ 
	// DONE
	// TODO: creare la matrice WorldViewProjection da passare al vertex shader tramite il costant buffer PerObjectCB:
	// 1. questo metodo (UpdateScene) � chiamato ogni frame per aggiornare la logica dell'applicazione, qui ad esempio
	//    potrai modificare la world matrix ogni frame per far girare l'oggetto su se stesso ad sempio di 30 gradi
	//    al secondo sull'asse Y. 
	//
	// hint: inizialmente conviene mantenere la world, view e projection matrix costanti
	//
	// 2. Carica, grazie ai metodi XMLoadFloat4x4, le matrici m_worldMatrix, m_viewMatrix e m_projectionMatrix
	//    in tipi XMMATRIX e costruisci la matrice finale moltiplicandole tra loro nell'ordine corretto WVP

	XMMATRIX worldMatr = XMLoadFloat4x4(&m_worldMatrix);
	worldMatr = worldMatr * XMMatrixRotationY(math::ToRadians(30.0f) * deltaSeconds);
	XMStoreFloat4x4(&m_worldMatrix, worldMatr);
	
	XMMATRIX viewMatr = XMLoadFloat4x4(&m_viewMatrix);
	XMMATRIX projMatr = XMLoadFloat4x4(&m_projectionMatrix);

	XMMATRIX wvp = worldMatr * viewMatr * projMatr;


	// DONE
	// TODO: aggiorna il costant buffer in modo che al vertex shader arrivi la nuova versione di WVP
	// 1. Crea una D3D11_MAPPED_SUBRESOURCE e inizializzala utilizzando ZeroMemory
	// 2. Utilizza il metodo Map di ID3D11DeviceContext per permettere alla CPU di accedere al costant 
	//    buffer m_vsConstantBuffer passando tra i paramentri la subresource createa al punto 1
	// 3. Casta il puntatore pData della subresource a PerObjectCB (la struttura che identifica il constant
	//    buffer lato c++)
	// 4. setta tramite XMStoreFloat4x4 il membro WVP del PerObjectCB ottenuto al punto 3. con la TRASPOSTA 
	//    della matrice WVP calcolata precedentemente (la trasposta � obbligatoria dato che hlsl ha un layout
	//    column major)
	// 5. Utilizza il metodo Unmap di ID3D11DeviceContext per avvertire che la modifica da parte della CPU 
	//    sul buffer m_vsConstantBuffer � completa

	D3D11_MAPPED_SUBRESOURCE mappedSubResource;
	ZeroMemory(&mappedSubResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	XTEST_D3D_CHECK(m_d3dContext->Map(m_vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubResource));
	PerObjectCB* constBuff = static_cast<PerObjectCB*>(mappedSubResource.pData);
	XMMATRIX wvp_T = XMMatrixTranspose(wvp);
	XMStoreFloat4x4(&(constBuff->WVP), wvp_T);
	m_d3dContext->Unmap(m_vsConstantBuffer.Get(), 0);

}


void BoxDemoApp::RenderScene()
{
	// DONE
	// TODO: pulisci il depth/stencil buffer e il back buffer 
	// 1. utilizza ClearDepthStencilView di ID3D11DeviceContext, specifica nei ClearFlags sia quello per pulire
	//    il depth buffer che quello per pulire lo stencil buffer mettendoli in or "|", come valore di pulizia per
	//    il depth utilizza 1.0 mentre per lo stencil 0.0 nonostante al momento non useremo lo stencil
	// 2. utilizza ClearRenderTargetView di ID3D11DeviceContext per riempire il back buffer col colore da te specificato
	//
	// hint: prima di provare a disegnare qualsiasi geometria prova a utilizzare solo il comando per il clear del render target
	//       in modo da controllare che il colore da te specificato sia presente come sfondo della finestra renderizzata, in quel 
	//       caso puoi essere sicuro della corretta inizializzazione di DirectX, naturalmente dovrai almeno invocare il metodo 
	//       present descritto pi� sotto altrimenti non vedrai nessun cambiamento a schermo

	m_d3dContext->ClearDepthStencilView(m_depthBufferView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);
	m_d3dContext->ClearRenderTargetView(m_backBufferView.Get(), DirectX::Colors::Black);


	// DONE
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
	m_d3dContext->VSSetConstantBuffers(0, 1, m_vsConstantBuffer.GetAddressOf());

	UINT strides[] = { sizeof(VertexIn) };
	UINT offsets[] = { 0 };
	m_d3dContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), strides, offsets);
	m_d3dContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	

	// DONE
	// TODO: disgna a schermo
	// 1. visto che stiamo utilizzando sia un vertex buffer che un index buffer devi utilizzare la chiamata
	//    DrawIndexed di ID3D11DeviceContext
	// 2. chiama la funzione present per mostrare a schermo il contenuto del back buffer tramite questa chiamata:
	//	  XTEST_D3D_CHECK(m_swapChain->Present(0, 0));

	m_d3dContext->DrawIndexed(36, 0, 0);
	XTEST_D3D_CHECK(m_swapChain->Present(0, 0));

}

