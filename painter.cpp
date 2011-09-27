#include "precompiled.h"
#include "tweetris.h"

bool Tweetris::startPainter() {

	HRESULT result;
	newVideoEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	newDepthEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	paintEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	resizeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	result = NuiInitialize(
		NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX |
		NUI_INITIALIZE_FLAG_USES_SKELETON |
		NUI_INITIALIZE_FLAG_USES_COLOR);
	
	if (FAILED(result)) {
		return false;
	}

	videoSize.width = 640;
	videoSize.height = 480;
	result = NuiImageStreamOpen(
		NUI_IMAGE_TYPE_COLOR,
		NUI_IMAGE_RESOLUTION_640x480,
		0, 2, newVideoEvent, &videoStream);
	
	if (FAILED(result)) {
		return false;
	}
	
	depthSize.width = 320;
	depthSize.height = 240;
	result = NuiImageStreamOpen(
		NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX,
		NUI_IMAGE_RESOLUTION_320x240,
		0, 2, newDepthEvent, &depthStream);
	
	if (FAILED(result)) {
		return false;
	}
	
	result = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &canvasMaker);
	if (FAILED(result)) {
		return false;
	}
	
	if (! loadTools()) {
		return false;
	}	
	
	running = true;

	painterThread = CreateThread(NULL, 0, painterProc, this, 0, NULL);
	return true;
}

bool Tweetris::loadTools() {
	
	if (toolsLoaded) {
		return true;
	}

	RECT rect;
	GetClientRect(outputWindow, &rect);
		
	D2D1_SIZE_U size = D2D1::SizeU(
		rect.right - rect.left,
		rect.bottom - rect.top
	);

	D2D1_RENDER_TARGET_PROPERTIES canvasProps = D2D1::RenderTargetProperties();
	canvasProps.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	canvasProps.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;

	HRESULT result = canvasMaker->CreateHwndRenderTarget(canvasProps,
			D2D1::HwndRenderTargetProperties(outputWindow, size), &canvas);
	
	if (SUCCEEDED(result)) {
		result = canvas->CreateSolidColorBrush(ignoredColor, &ignoredBrush);
	}
	
	if (SUCCEEDED(result)) {
		result = canvas->CreateSolidColorBrush(borderColor, &borderBrush);
	}
	
	D2D1_BITMAP_PROPERTIES bitmapProps = D2D1::BitmapProperties();
	bitmapProps.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	bitmapProps.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;

	if (SUCCEEDED(result)) {
		result = canvas->CreateBitmap(videoSize, bitmapProps, &videoBitmap);
	}

	bitmapProps.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED   ;
	if (SUCCEEDED(result)) {
		result = canvas->CreateBitmap(depthSize, bitmapProps, &depthBitmap);
	}

	if (debugging && !loadDebugTools()) {
		result = E_FAIL;
	}

	if (SUCCEEDED(result)) {
		toolsLoaded = player1.loadBrushes(canvas) 
							  && player2.loadBrushes(canvas);
	}

	if (!toolsLoaded) {
		unloadTools();
	}

	return toolsLoaded;
}

DWORD CALLBACK Tweetris::painterProc(LPVOID tweetris) {
	Tweetris * pThis = (Tweetris *) tweetris;

	HANDLE events[] = {
		pThis->newVideoEvent,
		pThis->newDepthEvent,
		pThis->paintEvent,
		pThis->resizeEvent
	};
	
	DWORD index;
	while (pThis->running) {
		index = WaitForMultipleObjects(ARRAYSIZE(events), events, FALSE, 1000);

		switch (index) {
		case 0:
			pThis->draw(true);
			ResetEvent(pThis->newVideoEvent);
			break;
		case 1:
			pThis->checkPlayers();
			ResetEvent(pThis->newDepthEvent);
			break;
		case 2:
			pThis->draw(false);
			break;
		case 3:
			pThis->resize();
			break;
		case WAIT_FAILED:
			return 1;
		case WAIT_TIMEOUT:
		default:
			break;
		}
	}

	return 0;
}

void Tweetris::resize() {
	
	D2D1_SIZE_U newSize;
	RECT rect;
	GetClientRect(outputWindow, &rect);
		
	newSize = D2D1::SizeU(
		rect.right - rect.left,
		rect.bottom - rect.top
	);
			
	canvas->Resize(newSize);
	
	D2D1_SIZE_F videoSize = videoBitmap->GetSize();

	FLOAT widthRatio = newSize.width / videoSize.width;
	FLOAT heightRatio = newSize.height / videoSize.height;
	
	FLOAT fitRatio = (widthRatio < heightRatio) ? widthRatio : heightRatio;
	FLOAT newWidth = videoSize.width * fitRatio;
	FLOAT newHeight = videoSize.height * fitRatio;
	
	outputRect.left = (newSize.width - newWidth) / 2;
	outputRect.top = (newSize.height - newHeight) / 2;
	outputRect.right = outputRect.left + newWidth;
	outputRect.bottom = outputRect.top + newHeight;

	grid.resizeTo(outputRect);
}

void Tweetris::unloadTools() {
	
	player1.cleanBrushes();
	player2.cleanBrushes();
	
	if (debugging) {
		unloadDebugTools();
	}

	if (ignoredBrush != NULL) {
		ignoredBrush->Release();
		ignoredBrush = NULL;
	}

	if (borderBrush != NULL) {
		borderBrush->Release();
		borderBrush = NULL;
	}

	if (videoBitmap != NULL) {
		videoBitmap->Release();
		videoBitmap = NULL;
	}

	if (depthBitmap != NULL) {
		depthBitmap->Release();
		depthBitmap = NULL;
	}
	
	if (canvas != NULL) {
		canvas->Release();
		canvas = NULL;
	}
}

void Tweetris::stopPainter() {
	running = false;
	
	if (painterThread != NULL) {
		WaitForSingleObject(painterThread, INFINITE);
	}

	unloadTools();
	if (canvasMaker != NULL) {
		canvasMaker->Release();
		canvasMaker = NULL;
	}
	
	NuiShutdown();
	
	if (newVideoEvent != NULL) {
		CloseHandle(newVideoEvent);
		newVideoEvent = NULL;
	}

	if (newDepthEvent != NULL) {
		CloseHandle(newDepthEvent);
		newDepthEvent = NULL;
	}

	if (paintEvent != NULL) {
		CloseHandle(paintEvent);
		paintEvent = NULL;
	}

	if (resizeEvent != NULL) {
		CloseHandle(resizeEvent);
		resizeEvent = NULL;
	}
}