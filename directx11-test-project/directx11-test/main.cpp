#include "stdafx.h"
#include <demo/box_demo/box_demo_app.h>
#include <demo/DrawCalls/threeDC.h>
#include <demo/lights_demo/lights_demo_app.h>


using namespace xtest::application;


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
					 _In_opt_ HINSTANCE hPrevInstance,
					 _In_ LPWSTR	lpCmdLine,
					 _In_ int	   nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	WindowSettings windowSettings;
	windowSettings.width = 800;
	windowSettings.height = 600;
	windowSettings.fullScreen = false;

	DirectxSettings directxSettings;
	directxSettings.showFrameStats = true;

	/*
	typedef xtest::demo::BoxDemoApp TestApp;
=======

	typedef xtest::demo::LightsDemoApp TestApp;

>>>>>>> master
	TestApp app(hInstance, windowSettings, directxSettings);
	app.Init();
	app.Run();
	*/
	//typedef threeDC TestApp;
	//TestApp myApp (hInstance, windowSettings, directxSettings);

	
	typedef xtest::demo::LightsDemoApp MyTestApp;
	MyTestApp myApp(hInstance, windowSettings, directxSettings);

	myApp.Init();
	myApp.Run();
	
	
}
