#include "precompiled.h"
#include "tweetris.h"

bool Tweetris::startPainter() {
	running = false;
	
	HRESULT result  = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &canvasMaker);
	
	if (SUCCEEDED(result)) {
		result = CoCreateInstance(CLSID_WICImagingFactory, NULL,
								  CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, 
								  (LPVOID*) &snapshotMaker);
	}
	
	if (SUCCEEDED(result)) {
		result = snapshotMaker->CreateBitmap(videoSize.width, videoSize.height,
											GUID_WICPixelFormat32bppPBGRA, 
											WICBitmapCacheOnLoad, &snapshotImage);
	}

	if (SUCCEEDED(result) && loadTools()) {
		running = true;
		painterThread = CreateThread(NULL, 0, painterProc, this, 0, NULL);
	}	

	return running;
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
	canvasProps.type = D2D1_RENDER_TARGET_TYPE_SOFTWARE;
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

	if (SUCCEEDED(result)) {
		result = canvas->CreateSolidColorBrush(D2D1::ColorF(0,0), &shapeBrush);
	}
	
	D2D1_BITMAP_PROPERTIES bitmapProps = D2D1::BitmapProperties();
	bitmapProps.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	bitmapProps.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;

	if (SUCCEEDED(result)) {
		result = canvas->CreateBitmap(videoSize, bitmapProps, &videoBitmap);
	}

	bitmapProps.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
	if (SUCCEEDED(result)) {
		result = canvas->CreateBitmap(depthSize, bitmapProps, &depthBitmap);
	}

	canvasProps.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
	
	if (SUCCEEDED(result)) {
		result = canvasMaker->CreateWicBitmapRenderTarget(snapshotImage, canvasProps, &snapshot);
	}
	
	if (SUCCEEDED(result)) {
		result = snapshot->CreateSharedBitmap(__uuidof(ID2D1Bitmap *), (LPVOID) videoBitmap, 
											  NULL, &snapshotBitmap);
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
			pThis->updateVideoBitmap();
			pThis->draw();
			ResetEvent(pThis->newVideoEvent);
			break;
		case 1:
			pThis->checkPlayers();
			ResetEvent(pThis->newDepthEvent);
			break;
		case 2:
			pThis->draw();
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
	
	outputArea.left = (newSize.width - newWidth) / 2;
	outputArea.top = (newSize.height - newHeight) / 2;
	outputArea.right = outputArea.left + newWidth;
	outputArea.bottom = outputArea.top + newHeight;

	grid.resizeTo(outputArea);
}

void Tweetris::unloadTools() {
	
	player1.cleanBrushes();
	player2.cleanBrushes();

	if (ignoredBrush != NULL) {
		ignoredBrush->Release();
		ignoredBrush = NULL;
	}

	if (borderBrush != NULL) {
		borderBrush->Release();
		borderBrush = NULL;
	}

	if (shapeBrush != NULL) {
		shapeBrush->Release();
		shapeBrush = NULL;
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

	if (snapshot != NULL) {
		snapshot->Release();
		snapshot = NULL;
	}

	if (snapshotImage != NULL) {
		snapshotImage->Release();
		snapshotImage = NULL;
	}
	
	if (snapshotBitmap != NULL) {
		snapshotBitmap->Release();
		snapshotBitmap = NULL;
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

	if (snapshotMaker != NULL) {
		snapshotMaker->Release();
		snapshotMaker = NULL;
	}

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