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

void Tweetris::clearTallies() {
	for (int i = 0; i < grid.numBoxes; i++) {
		playerTally[i] = 0;
		for (int j = 0; j < MAX_PLAYERS; j++) {
			boxTally[i][j] = 0;
		}
	}
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
	
	int numPixels = depthSize.width * depthSize.height;
	RGBQUAD * depthColors = new RGBQUAD[numPixels];
	for (int i = 0; i < numPixels; i++) {
		depthColors[i].rgbBlue = 0;
		depthColors[i].rgbRed = 0;
		depthColors[i].rgbGreen = 0;
		depthColors[i].rgbReserved = 0;
	}

	clearTallies();

	USHORT depth;
	int player, boxIndex;

	LONG colorX, colorY;
	for (LONG y = 0; y < depthSize.height; y++) {
		for (LONG x = 0; x < depthSize.width; x++) {

			depth = *bufferRun & 0xfff8;
			player = *bufferRun & 7;

			NuiImageGetColorPixelCoordinatesFromDepthPixel(
									NUI_IMAGE_RESOLUTION_640x480,
									0, x, y, depth, &colorX, &colorY);

			boxIndex = grid.locateBox(colorX, colorY);
			if (boxIndex != -1) {
				boxTally[boxIndex][player]++;
			}
			
			if (depthColors != NULL && 
				x > 0 && x < depthSize.width &&
				y > 0 && y < depthSize.height) {
					depthColors[y * depthSize.width + x] = depthToColor(depth, player);
			}

			bufferRun++;
		}
	}
	
	// keeps a copy of the current shape for concurrency
	int * shapeCopy = shape;
	int winner = findWinner(shapeCopy);
	switch(winner) {
	case 1: // player 1 wins
		break;
	case 2: // player 2 wins
		break;
	case -1: // timed out
		report(1, shapeCopy);
		shape = NULL;
		break;
	case 0:
	default:
		break;
	}

	D2D1_RECT_U depthRect = D2D1::RectU(0, 0, depthSize.width, depthSize.height);
	depthBitmap->CopyFromMemory(&depthRect, depthColors, depthSize.width * sizeof(RGBQUAD));
	delete [] depthColors;

	NuiImageStreamReleaseFrame(depthStream, depthFrame);

	draw();

	return true;
}

// reassigns the index for player 1 and player 2
// also sets the state of the boxes according to the shape
int Tweetris::findWinner(int * shapeCopy) {

	// no shape, no winner 
	if (shapeCopy == NULL) {
		return 0;
	}

	for (int i = 0; i < grid.numBoxes; i++) {
		switch (shape[i]) {
		case 0: 
			grid.boxes[i].state = GridBox::IGNORED;
			break;
		case 1:
			grid.boxes[i].state = GridBox::P1_UNMATCHED;
			break;
		case 2:
			grid.boxes[i].state = GridBox::P2_UNMATCHED;
			break;
		}
	}
	
	// see if it's a timeout
	if (playStartTime != 0 && 
		GetTickCount() > playStartTime + allowedPlayTime) {
		return -1;
	}

	// first finds the index of the two players that fill
	// up the most of the screen
	
	// determines how much each player fills up the screen
	for (int i = 1; i < MAX_PLAYERS; i++) {
		for (int j = 0; j < grid.numBoxes; j++) {
			playerTally[i] += boxTally[j][i];
		}
	}

	// finds the player that fills the screen the most
	int max = 0;
	for (int i = 1; i < MAX_PLAYERS; i++) {
		if (playerTally[i] > playerTally[max]) {
			max = i;
		}
	}

	// finds the player that fills up the second most
	int sec = 0;
	for (int i = 1; i < MAX_PLAYERS; i++) {
		if (playerTally[i] > playerTally[sec] && i != max) {
			sec = i;
		}
	} 

	// tries to reassign the kinect player indices to the players

	if (player2.index != max || player1.index != sec) {
		player1.index = max;
		player2.index = sec;
	} else {
		player1.index = sec;
		player2.index = max;
	}

	bool p1Passed = true, p2Passed = true;

	// finally set the state of each box
	
	float boxArea = grid.getBoxArea() 
				  * (depthSize.width * depthSize.height) 
				  / (videoSize.width * videoSize.height);
	
	for (int i = 0; i < grid.numBoxes; i++) {
		switch (shapeCopy[i]) {
			case 0:
				if (player1.index != 0 && boxTally[i][player1.index] / boxArea > outLimit) {
					grid.boxes[i].state = GridBox::P1_OUT;
					p1Passed = false;
				} else if (player2.index != 0 && boxTally[i][player2.index] / boxArea > outLimit) {
					grid.boxes[i].state = GridBox::P2_OUT;
					p2Passed = false;
				}
				break;

			case 1:
				if (player1.index != 0 && boxTally[i][player1.index] / boxArea > matchLimit) {
					grid.boxes[i].state = GridBox::P1_MATCHED;
				} else {
					p1Passed = false;
				}
				break;

			case 2:
				if (player2.index != 0 && boxTally[i][player2.index] / boxArea > matchLimit) {
					grid.boxes[i].state = GridBox::P2_MATCHED;
				} else {
					p2Passed = false;
				}
				break;
		}
	}

	if (p1Passed) {
		return 1;
	} else if (p2Passed) {
		return 2;
	} else {
		return 0;
	}
}
