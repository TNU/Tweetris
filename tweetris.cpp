#include "precompiled.h"
#include "tweetris.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
		
	// defines the main window class
	LPCTSTR MAIN_CLASS_NAME = TEXT("MainWindowClass");
	
	WNDCLASSEX mainClass;
	mainClass.cbSize			= sizeof(WNDCLASSEX);
	mainClass.style				= CS_HREDRAW | CS_VREDRAW;
	mainClass.lpfnWndProc		= DefWindowProc;
	mainClass.cbClsExtra		= 0;
	mainClass.cbWndExtra		= sizeof(LONG_PTR);
	mainClass.hInstance			= hInstance;
	mainClass.hIcon				= LoadIcon(NULL, IDI_APPLICATION);
	mainClass.hCursor			= LoadCursor(NULL, IDC_ARROW);
	mainClass.hbrBackground		= (HBRUSH)(COLOR_WINDOW + 1);
	mainClass.lpszMenuName		= NULL;
	mainClass.lpszClassName		= MAIN_CLASS_NAME;
	mainClass.hIconSm			= LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&mainClass)) {
		return false;
	}
	
	// creates the main window
	LPCTSTR MAIN_WINDOW_TITLE = TEXT("Tweetris");
	HWND mainWindow = CreateWindowEx(WS_EX_STATICEDGE, MAIN_CLASS_NAME, 
									 MAIN_WINDOW_TITLE, WS_OVERLAPPEDWINDOW, 
									 CW_USEDEFAULT, CW_USEDEFAULT, 656, 518, 
									 NULL, NULL, hInstance, NULL);

	if (mainWindow == NULL) {
		return false;
	}

	HRESULT result = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	// creates and binds the main window to tweetris video output
	Tweetris tweetris(mainWindow);

	// displays the main window
	ShowWindow(mainWindow, nCmdShow);
	UpdateWindow(mainWindow);
	
	// starts the debug dialog
	tweetris.initDebugDialog(hInstance, nCmdShow);

	// handles any incoming messages
	MSG message;
	while(GetMessage(&message, NULL, 0, 0)) {
		if(tweetris.debugDialog !=NULL &&
			IsDialogMessage(tweetris.debugDialog, &message)) {
			continue;
		}

		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	CoUninitialize();

	return message.wParam;
}

Tweetris::Tweetris(HWND outputWindow) 
		: outputWindow(outputWindow),
		  videoSize(D2D1::SizeU()),
		  depthSize(D2D1::SizeU()),
		  outputArea(D2D1::RectF()),
		  snapshotScaleRatio(0.5),
		  TWITPIC_UPLOAD_AND_POST_API_URL(TEXT("/api/uploadAndPost")),
		  USERNAME("TweetrisTester"),
		  PASSWORD("TestMyPatience"),

		  allowedPlayTime(30 * 1000),
		  grid(3, 4, 0.025f, 0.025f, 0.01f, 0),
		  ignoredColor(D2D1::ColorF(D2D1::ColorF::White, 0.25)),
		  borderColor(D2D1::ColorF(D2D1::ColorF::Black, 0.75)),
		  backgroundColor(D2D1::ColorF::Beige),
		  player1(D2D1::ColorF(D2D1::ColorF::Green, 0.75),
				  D2D1::ColorF(D2D1::ColorF::Blue, 0.5),
				  D2D1::ColorF(D2D1::ColorF::Aqua, 0.5)),
		  player2(D2D1::ColorF(D2D1::ColorF::Orange, 0.75),
				  D2D1::ColorF(D2D1::ColorF::Red, 0.5),
				  D2D1::ColorF(D2D1::ColorF::Yellow, 0.5)),
		  matchLimit(0.5), outLimit(0.1) {

	srand(GetTickCount());

	toolsLoaded = false;
	canvasMaker	= NULL;
	canvas = NULL;

	snapshotMaker = NULL;
	snapshotImage = NULL;
	snapshot = NULL;
	snapshotBitmap = NULL;

	debugging = false;
	debugDialog	= NULL;

	frameCount = 0;
	frameCountingThread = NULL;
	
	videoStream = NULL;
	depthStream = NULL;

	videoBitmap = NULL;
	depthBitmap = NULL;

	ignoredBrush = NULL;
	borderBrush = NULL;
	shapeBrush = NULL;

	shape = NULL;
	playStartTime = 0;
	
	playerTally = new int [grid.numBoxes];
	boxTally = new int * [grid.numBoxes];
	for (int i = 0; i < grid.numBoxes; i++) {
		boxTally[i] = new int [MAX_PLAYERS];
	}

	// initialize Kinect
	newVideoEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	newDepthEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	paintEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	resizeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	HRESULT result = NuiInitialize(
		NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX |
		NUI_INITIALIZE_FLAG_USES_SKELETON |
		NUI_INITIALIZE_FLAG_USES_COLOR);
	
	if (FAILED(result)) {
		return;
	}

	videoSize.width = 640;
	videoSize.height = 480;
	result = NuiImageStreamOpen(
		NUI_IMAGE_TYPE_COLOR,
		NUI_IMAGE_RESOLUTION_640x480,
		0, 2, newVideoEvent, &videoStream);
	
	if (FAILED(result)) {
		return;
	}
	
	depthSize.width = 320;
	depthSize.height = 240;
	result = NuiImageStreamOpen(
		NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX,
		NUI_IMAGE_RESOLUTION_320x240,
		0, 2, newDepthEvent, &depthStream);
	
	if (FAILED(result)) {
		return;
	}

	// initialize Twitpic uploading functionality
	InitializeCriticalSectionAndSpinCount(&uploadQueueAccess, 4000);  
	uploadQueueEvent = CreateEvent(NULL, FALSE, FALSE, NULL); 
	uploadThread = CreateThread(NULL, 0, uploadProc, this, 0, NULL);

	SetWindowLongPtr(outputWindow, GWLP_USERDATA, (LONG_PTR) (this));
	SetWindowLongPtr(outputWindow, GWLP_WNDPROC, (LONG_PTR) (Tweetris::mainProc));
	
	startPainter();
}

LRESULT CALLBACK Tweetris::mainProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	Tweetris * pThis = NULL;
	switch (message) {
		// tweetris specific messages and events
		case WM_DISPLAYCHANGE:
		case WM_PAINT: 
		case WM_SIZE: 
		case WM_CHAR:
		case WM_CLOSE:
		case WM_DESTROY:
			pThis = (Tweetris *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

			switch (message) {
			case WM_CHAR:
				pThis->selectShape(wParam);

				switch (wParam) {
				case 8:
					pThis->console.str(TEXT(""));
					break;
				default:
					pThis->console << TEXT("[INPUT]: ") 
									  << (TCHAR) wParam << TEXT("\r\n");
				}
				pThis->updateConsole();
				break;

			case WM_SIZE:
				SetEvent(pThis->paintEvent);
				pThis->console << TEXT("[RESIZE]") << TEXT("\r\n");
				pThis->updateConsole();
				break;

			case WM_DISPLAYCHANGE:
				InvalidateRect(hwnd, NULL, FALSE);
				pThis->console << TEXT("[DCHANGE]") << TEXT("\r\n");
				pThis->updateConsole();
				break;

			case WM_PAINT:
				SetEvent(pThis->resizeEvent);
				ValidateRect(hwnd, NULL);
				pThis->console << TEXT("[DRAW]") << TEXT("\r\n");
				pThis->updateConsole();
				break;
			case WM_CLOSE:
				DestroyWindow(hwnd);
				break;
			case WM_DESTROY:
				pThis->stopPainter();
				PostQuitMessage(0);
				break;
			}
			break;

		// all other messages
		default:
			return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}

Tweetris::~Tweetris() {
	
	EnterCriticalSection(&uploadQueueAccess);
	uploadQueue.push(NULL);
	LeaveCriticalSection(&uploadQueueAccess);
	
	SetEvent(uploadQueueEvent);

	stopPainter();

	uninitDebugDialog();

	NuiShutdown();
	
	// stop the upload thread
	WaitForSingleObject(uploadThread, INFINITE);
	CloseHandle(uploadThread);
	CloseHandle(uploadQueueEvent);
	DeleteCriticalSection(&uploadQueueAccess);
	
	delete [] playerTally;
	
	for (int i = 0; i < grid.numBoxes; i++) {
		delete [] boxTally[i];
	}
	delete [] boxTally;
}