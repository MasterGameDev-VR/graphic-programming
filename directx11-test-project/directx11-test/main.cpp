#include "stdafx.h"
#include <demo/box_demo/box_demo_app.h>


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
	windowSettings.width = 1280;
	windowSettings.height = 720;
	windowSettings.fullScreen = false;

	DirectxSettings directxSettings;
	directxSettings.showFrameStats = true;
	

	typedef xtest::demo::BoxDemoApp TestApp;

	TestApp app(hInstance, windowSettings, directxSettings);
	app.Init();
	app.Run();
}
