#include "precompiled.h"
#include "tweetris.h"
#include "shapes.h"

void Tweetris::selectShape(char shapeLabel) {
	playStartTime = 0;

	int ori = 0;
	switch (shapeLabel) {
		case '0': case 'c': case 'C':
			shape = NULL; 
			return;
		case '1': case 'i': case 'I':
			ori = rand() % Shapes::oriCount[0];
			shape = Shapes::I[ori]; 
			break;
		case '2': case 'j': case 'J':
			ori = rand() % Shapes::oriCount[1];
			shape = Shapes::J[ori]; 
			break;
		case '3': case 'l': case 'L':
			ori = rand() % Shapes::oriCount[2];
			shape = Shapes::L[ori]; 
			break;
		case '4': case 'o': case 'O':
			ori = rand() % Shapes::oriCount[3];
			shape = Shapes::O[ori]; 
			break;
		case '5': case 'z': case 'Z':
			ori = rand() % Shapes::oriCount[4];
			shape = Shapes::Z[ori]; 
			break;
		case '6': case 't': case 'T':
			ori = rand() % Shapes::oriCount[5];
			shape = Shapes::T[ori]; 
			break;
		case '7': case 's': case 'S':
			ori = rand() % Shapes::oriCount[6];
			shape = Shapes::S[ori]; 
			break;
		default:
			return;
	}
	
	playStartTime = GetTickCount();
}

bool Tweetris::checkPlayers() {

	const NUI_IMAGE_FRAME * depthFrame = NULL;
	HRESULT result = NuiImageStreamGetNextFrame(
					 depthStream, 0, &depthFrame);

	if (FAILED(result)) {
		return false;
	}
	
	NuiImageBuffer * depthTexture = depthFrame->pFrameTexture;
	KINECT_LOCKED_RECT LockedRect;
	depthTexture->LockRect(0, &LockedRect, NULL, 0);

	USHORT * bufferRun = (USHORT *) LockedRect.pBits;
	
	RGBQUAD * depthColors = NULL;
	if (debugging) {
		int numPixels = depthSize.width * depthSize.height;
		depthColors = new RGBQUAD[numPixels];
		for (int i = 0; i < numPixels; i++) {
			depthColors[i].rgbBlue = 0;
			depthColors[i].rgbRed = 0;
			depthColors[i].rgbGreen = 0;
			depthColors[i].rgbReserved = 0;
		}
	}

	grid.resetData();
	USHORT depth;
	int player;

	LONG colorX, colorY;
	for (LONG y = 0; y < depthSize.height; y++) {
		for (LONG x = 0; x < depthSize.width; x++) {

			depth = *bufferRun & 0xfff8;
			player = *bufferRun & 7;

			NuiImageGetColorPixelCoordinatesFromDepthPixel(
									NUI_IMAGE_RESOLUTION_640x480,
									0, x, y, depth, &colorX, &colorY);

			grid.mark(colorX, colorY, player);
			
			if (depthColors != NULL && 
				x > 0 && x < depthSize.width &&
				y > 0 && y < depthSize.height) {
					depthColors[y * depthSize.width + x] = depthToColor(depth, player);
			}

			bufferRun++;
		}
	}

	float areaRatio = (float) (depthSize.width * depthSize.height) / (videoSize.width * videoSize.height);

	int winner;
	if (playStartTime != 0 && 
		GetTickCount() > playStartTime + allowedPlayTime) {
		shape = NULL;
	}
	winner = grid.analyzeData(player1.index, player2.index, shape, areaRatio);

	if (depthColors != NULL) {
		D2D1_RECT_U depthRect = D2D1::RectU(0, 0, depthSize.width, depthSize.height);
		depthBitmap->CopyFromMemory(&depthRect, depthColors, depthSize.width * sizeof(RGBQUAD));
		delete depthColors;
	}


	NuiImageStreamReleaseFrame(depthStream, depthFrame);

	draw(false);

	return true;
}