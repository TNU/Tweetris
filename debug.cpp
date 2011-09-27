#include "precompiled.h"
#include "tweetris.h"

bool Tweetris::initDebugDialog(HINSTANCE hInstance, int nCmdShow) {	
	
	if (debugging) {
		return true;
	}

	// creates the debugging dialog
	debugDialog = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_DEBUG), 
									NULL, (DLGPROC) debugProc, (LONG_PTR) this);

	if (debugDialog == NULL) {
		return false;
	}

	if (! loadDebugTools()) {
		return false;
	}

	// displays the debugging dialog
	ShowWindow(debugDialog,nCmdShow);
	UpdateWindow(debugDialog);

	if (! loadDebugTools()) {
		return false;
	}

	frameCountingThread = CreateThread(NULL, 0, frameCountingProc, this, 0, NULL);
	if (frameCountingThread == NULL) {
		return false;
	}

	debugging = true;

	return true;
}

bool Tweetris::loadDebugTools() {
	D2D1_RENDER_TARGET_PROPERTIES canvasProps = D2D1::RenderTargetProperties();
	canvasProps.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	canvasProps.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;

	HRESULT result;
	HWND videoView = GetDlgItem(debugDialog, IDC_VIDEO);
	
	RECT rect;
	GetClientRect(videoView, &rect);
		
	D2D1_SIZE_U size = D2D1::SizeU(
		rect.right - rect.left,
		rect.bottom - rect.top
	);

	result = canvasMaker->CreateHwndRenderTarget(canvasProps,
		D2D1::HwndRenderTargetProperties(videoView, size), &videoCanvas);

	if (SUCCEEDED(result)) {
		HWND depthView = GetDlgItem(debugDialog, IDC_DEPTH);

		GetClientRect(depthView, &rect);
		
		size = D2D1::SizeU(
			rect.right - rect.left,
			rect.bottom - rect.top
		);

		result = canvasMaker->CreateHwndRenderTarget(canvasProps,
			D2D1::HwndRenderTargetProperties(depthView, size), &depthCanvas);
	}

	if (FAILED(result)) {
		unloadDebugTools();
	}

	return true;
}

BOOL CALLBACK Tweetris::debugProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	Tweetris * tweetris;
	std::basic_string<TCHAR> consoleMessage;
    switch (message) {
		case WM_INITDIALOG:
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
			return TRUE;
		case MY_UPDATE_CONSOLE:
			tweetris = (Tweetris *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			consoleMessage = tweetris->console.str();
			SetDlgItemText(hwndDlg, IDC_CONSOLE, consoleMessage.c_str());
			break;
		case MY_SET_FRAME_RATE:
			SetDlgItemInt(hwndDlg, IDC_FRAME_RATE, lParam, TRUE);
			break;
		case WM_CLOSE:
			DestroyWindow(hwndDlg);
			break;
		case WM_DESTROY:
			tweetris = (Tweetris *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			tweetris->uninitDebugDialog();
			EndDialog(hwndDlg, 0);
			break;
	}

    return FALSE;
}

void Tweetris::updateConsole() {
	if (! debugging) {
		return;
	}

	PostMessage(debugDialog, MY_UPDATE_CONSOLE, 0, 0);
}

DWORD CALLBACK Tweetris::frameCountingProc(LPVOID tweetris) {
	Tweetris * pThis = (Tweetris *) tweetris;
	LONG frameRate;

	while (pThis->debugging) {
		Sleep(1000);	
		frameRate = InterlockedExchange(&(pThis->frameCount), 0);
		PostMessage(pThis->debugDialog, MY_SET_FRAME_RATE, 0, frameRate);
	}

	return 0;
}


void Tweetris::unloadDebugTools() {
	if (depthCanvas != NULL) {
		depthCanvas->Release();
		depthCanvas = NULL;
	}

	if (videoCanvas != NULL) {
		videoCanvas->Release();
		videoCanvas = NULL;
	}
}

void Tweetris::uninitDebugDialog() {
	debugging = false;
	debugDialog = NULL;

	frameCount = 0;
	if (frameCountingThread != NULL) {
		WaitForSingleObject(frameCountingThread, INFINITE);
		CloseHandle(frameCountingThread);
		frameCountingThread = NULL;
	}

	unloadDebugTools();
}