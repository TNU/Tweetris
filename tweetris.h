#pragma once

#include "precompiled.h"
#include "player.h"
#include "grid.h"

class Tweetris {

public:	
	static LRESULT CALLBACK mainProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	HWND debugDialog;
	bool initDebugDialog(HINSTANCE hInstance, int nCmdShow);
	void uninitDebugDialog();

	// console to write to, the text only gets displayed
	// when updateConsole is called
	std::basic_stringstream<TCHAR>  console;
	void updateConsole();
	
	static const int MAX_PLAYERS = 8;

	Tweetris(HWND outputWindow);
	~Tweetris();
	
private:
	// state variables used to signal threads to stop
	volatile bool running;
	volatile bool debugging;
	
	// debug dialog related declarations
	static CONST UINT MY_UPDATE_CONSOLE = WM_APP + 0x0100;
	static CONST UINT MY_SET_FRAME_RATE = WM_APP + 0x0101;
	static BOOL CALLBACK debugProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

	// frame rate counting declarations
	LONG frameCount;
	HANDLE frameCountingThread;
	static DWORD CALLBACK frameCountingProc(LPVOID tweetris);

	// Direct2D painter setup declarations
	HWND outputWindow;
	D2D1_RECT_F outputArea;
	void resize();			

	ID2D1Factory * canvasMaker;
	ID2D1HwndRenderTarget * canvas;

	D2D1_SIZE_U videoSize;
	ID2D1Bitmap * videoBitmap;
	D2D1_SIZE_U depthSize;
	ID2D1Bitmap * depthBitmap;

	bool startPainter();
	void stopPainter();
	
	D2D1::ColorF ignoredColor;
	ID2D1SolidColorBrush * ignoredBrush;
	D2D1::ColorF borderColor;
	ID2D1SolidColorBrush * borderBrush;
	D2D1::ColorF backgroundColor;
	PlayerProfile player1, player2;
	bool draw();   

	// used to reload all direct2d device dependent
	// resources on D2DERR_RECREATE_TARGET
	bool toolsLoaded;		// painting tools
	bool loadTools();		
	void unloadTools();

	// Kinect interface declarations
	HANDLE newVideoEvent, newDepthEvent, paintEvent, resizeEvent;
	HANDLE videoStream, depthStream;
	HANDLE painterThread;
	static DWORD CALLBACK painterProc(LPVOID tweetris);

	// WIC declarations
	IWICImagingFactory * snapshotMaker;
	IWICBitmap * snapshotImage;
	ID2D1RenderTarget * snapshot;
	ID2D1Bitmap * snapshotBitmap;
	void drawToSnapshot(int player, int * shape);

	// painting functions
	bool updateVideoBitmap();
	Grid grid;
	void drawGrid();
	RGBQUAD depthToColor(USHORT depth, USHORT player);

	// game functions
	int * shape;
	void selectShape(char shapeLabel = '0');
	
	float matchLimit, outLimit;
	bool checkPlayers();
	int * playerTally;
	int ** boxTally;
	void clearTallies();
	int findWinner(int * shapeCopy);
	
	CONST DWORD allowedPlayTime;
	DWORD playStartTime;
};