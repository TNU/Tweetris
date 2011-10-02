#include "precompiled.h"
#include "tweetris.h"
#include "shape.h"

void Tweetris::selectShape(char command) {
	const Shape * newShape = NULL;
	
	switch (command) {
	case '0': case 'c': case 'C':
		shape = NULL;
		break;

	case '8': case 'n': case 'N': case 'r': case 'R':
		shape = Shape::getRandomShape();
		playStartTime = GetTickCount();
		break;

	default:
		newShape = Shape::getShapeByLabel(command);

		if (newShape != NULL) {
			shape = newShape;
			playStartTime = GetTickCount();
		}
	} 
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
	
	// keeps a copy of the current shape for concurrency
	const Shape * shapeCopy = shape;
	RGBQUAD * depthColors = NULL;
	int depthToVideoRatio = videoSize.width / depthSize.width;
	if (debugging) {
		depthColors = new RGBQUAD[numPixels];
		for (int i = 0; i < numPixels; i++) {
			depthColors[i].rgbBlue = 0;
			depthColors[i].rgbRed = 0;
			depthColors[i].rgbGreen = 0;
			depthColors[i].rgbReserved = 0;
		}
	}

	clearTallies();

	USHORT depth;
	int player, boxIndex;

	LONG colorX, colorY;
	for (LONG y = 0; y < (LONG) depthSize.height; y++) {
		for (LONG x = 0; x < (LONG) depthSize.width; x++) {

			depth = *bufferRun & 0xfff8;
			player = *bufferRun & 7;

			NuiImageGetColorPixelCoordinatesFromDepthPixel(
									NUI_IMAGE_RESOLUTION_640x480,
									0, x, y, depth, &colorX, &colorY);

			boxIndex = grid.locateBox(colorX, colorY);
			if (boxIndex != -1) {
				boxTally[boxIndex][player]++;
			}
			
			colorX /= depthToVideoRatio;
			colorY /= depthToVideoRatio;
			if (depthColors != NULL &&  
				colorX > 0 && colorX < (LONG) depthSize.width &&
				colorY > 0 && colorY < (LONG) depthSize.height) {
					depthColors[colorY * depthSize.width + colorX] = depthToColor(depth, player);
			}

			bufferRun++;
		}
	}

	if (depthColors != NULL) {
		D2D1_RECT_U depthRect = D2D1::RectU(0, 0, depthSize.width, depthSize.height);
		depthBitmap->CopyFromMemory(&depthRect, depthColors, depthSize.width * sizeof(RGBQUAD));
		delete [] depthColors;
	}

	NuiImageStreamReleaseFrame(depthStream, depthFrame);

	int winner = findWinner(shapeCopy);
	switch(winner) {
	case 1: // player 1 wins
		report(1, shapeCopy);
		selectShape('r');
		break;
	case 2: // player 2 wins
		report(2, shapeCopy);
		selectShape('r');
		break;
	case -1: // timed out
		report(0, shapeCopy);
		selectShape('r');
		break;
	case 0:
	default:
		break;
	}

	draw();

	return true;
}

// reassigns the index for player 1 and player 2
// also sets the state of the boxes according to the shape
int Tweetris::findWinner(const Shape * shapeCopy) {

	// no shape, no winner 
	if (shapeCopy == NULL) {
		return 0;
	}

	for (int i = 0; i < grid.numBoxes; i++) {
		switch (shapeCopy->boxes[i]) {
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
		switch (shapeCopy->boxes[i]) {
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
