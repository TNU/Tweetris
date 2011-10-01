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

	// displays the debugging dialog
	ShowWindow(debugDialog,nCmdShow);
	UpdateWindow(debugDialog);

	frameCountingThread = CreateThread(NULL, 0, frameCountingProc, this, 0, NULL);
	if (frameCountingThread == NULL) {
		return false;
	}

	debugging = true;

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


void Tweetris::uninitDebugDialog() {
	debugging = false;
	debugDialog = NULL;

	frameCount = 0;
	if (frameCountingThread != NULL) {
		WaitForSingleObject(frameCountingThread, INFINITE);
		CloseHandle(frameCountingThread);
		frameCountingThread = NULL;
	}
}