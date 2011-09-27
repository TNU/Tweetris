#include "precompiled.h"
#include "grid.h"
GridBox::GridBox() {
	rect = D2D1::RectF();
	state = GridBox::IGNORED;
}
Grid::Grid(int numBoxWide, int numBoxHigh, 
		   float left, float right, float centre, float shiftDown, 
		   float matchLimit, float outLimit) :
		   numBoxWide(numBoxWide), numBoxHigh(numBoxHigh), numBoxes(numBoxWide *  numBoxHigh * 2),
		   left(left), right(right), centre(centre), shiftDown(shiftDown),
		   matchLimit(matchLimit), outLimit(outLimit) {

	boxes = new GridBox[numBoxes];

	gridWidth = 0;
	gridHeight = 0;
	boxWidth = 0;
	boxHeight = 0;
	
	marginTop = 0;
	marginLeft = 0;
	marginRight = 0;
	
	resetData();
}

void Grid::resizeTo(const D2D1_RECT_F & rect) {
	float width = rect.right - rect.left;
	float height = rect.bottom - rect.top;

	gridWidth = width * (1 - left - centre - right) / 2;
	gridHeight = gridWidth / numBoxWide * numBoxHigh;
	boxWidth = gridWidth / numBoxWide;
	boxHeight = gridHeight / numBoxHigh;

	offsetLeft = rect.left;
	offsetTop = rect.top;
	marginTop = (height - gridHeight) / 2;
	marginLeft = width * left;
	marginRight = marginLeft + gridWidth + width * centre;
	
	float topStart = offsetTop + marginTop;
	float leftStart = offsetLeft + marginLeft;
	float rightStart = offsetLeft + marginRight;

	GridBox * boxIte = boxes;
	for (int k = 0; k < numBoxHigh; k ++) {
		for (int i = 0, j = numBoxWide; i < numBoxWide; i++, j++) {
			boxIte[i].rect.left = leftStart + i * boxWidth;
			boxIte[i].rect.top = topStart + k * boxHeight;
			boxIte[i].rect.right = boxIte[i].rect.left + boxWidth;
			boxIte[i].rect.bottom = boxIte[i].rect.top + boxHeight;
		
			boxIte[j].rect.left = rightStart + i * boxWidth;
			boxIte[j].rect.top = boxIte[i].rect.top;
			boxIte[j].rect.right = boxIte[j].rect.left + boxWidth;
			boxIte[j].rect.bottom = boxIte[i].rect.bottom;
		}
		
		boxIte += numBoxWide * 2;
	}
}

void Grid::resetData() {
	for (int i = 0; i < numBoxes; i++) {
		boxes[i].state = GridBox::IGNORED;

		for(int j = 0; j < GridBox::MAX_NUM_PLAYER_INDICES; j++) {
			boxes[i].fillByPlayer[j] = 0;
		}
	}
}

void Grid::mark(LONG x, LONG y, int player) {
	
	float start;
	int boxOffset = 0, numBoxX, numBoxY;

	if (y > marginTop && y <= marginTop + gridHeight) {
		if (x > marginLeft && x <= marginLeft + gridWidth) {
			start = marginLeft;
			boxOffset = 0;
		} else if (x > marginRight && x <= marginRight + gridWidth) {
			start = marginRight;
			boxOffset = numBoxWide;
		} else {
			return;
		}
	} else {
		return;
	}
	
	numBoxX = (int) ((x - start) / boxWidth) + boxOffset;
	numBoxY = (int) ((y - marginTop) / boxHeight);

	boxes[numBoxY * numBoxWide * 2 + numBoxX].fillByPlayer[player]++;

	return;
}

// reassigns the playerIndex for player 1 and player 2
// also sets the state of the boxes according to the shape
int Grid::analyzeData(int &player1, int &player2, int * shape, float areaRatio) {

	// first finds the index of the two players that fill
	// up the most of the screen
	
	// determines how much each player fills up the screen
	int playerFill[GridBox::MAX_NUM_PLAYER_INDICES] = {0};
	for (int i = 1; i < GridBox::MAX_NUM_PLAYER_INDICES; i++) {
		for (int j = 0; j < numBoxes; j++) {
			playerFill[i] += boxes[j].fillByPlayer[i];
		}
	}

	// finds the player that fills the screen the most
	int max = 0;
	for (int i = 1; i < GridBox::MAX_NUM_PLAYER_INDICES; i++) {
		if (playerFill[i] > playerFill[max]) {
			max = i;
		}
	}

	// finds the player that fills up the second most
	int sec = 0;
	for (int i = 1; i < GridBox::MAX_NUM_PLAYER_INDICES; i++) {
		if (playerFill[i] > playerFill[sec] && i != max) {
			sec = i;
		}
	} 

	// then try to match these two indices to the previous
	// player indices provided in the arguments
	
	if (max == 0) {
		player1 = 0;
		player2 = 0;
	} else if (sec == 0) {
		if (player2 != max) {
			player1 = max;
			player2 = 0;
		} else {
			player1 = 0;
			player2 = max;
		}
	} else {
		if (player2 != max || player1 != sec) {
			player1 = max;
			player2 = sec;
		} else {
			player1 = sec;
			player2 = max;
		}
	}

	// no shape, no winner 
	if (shape == NULL) {
		return 0;
	}

	bool p1Passed = true, p2Passed = true;

	// finally set the state of each box

	float boxArea = boxWidth * boxHeight * areaRatio;
	
	for (int i = 0; i < numBoxes; i++) {
		if (shape[i] == 0) {
			if (player1 != 0 && boxes[i].fillByPlayer[player1] / boxArea > outLimit) {
				boxes[i].state = GridBox::P1_OUT;
				p1Passed = false;
			} else if (player2 != 0 && boxes[i].fillByPlayer[player2] / boxArea > outLimit) {
				boxes[i].state = GridBox::P2_OUT;
				p2Passed = false;
			} else {
				boxes[i].state = GridBox::IGNORED;
			}

		} else if (shape[i] == 1) {
			if (player1 != 0 && boxes[i].fillByPlayer[player1] / boxArea > matchLimit) {
				boxes[i].state = GridBox::P1_MATCHED;
			} else {
				boxes[i].state = GridBox::P1_UNMATCHED;
				p1Passed = false;
			}

		} else if (shape[i] == 2) {
			if (player2 != 0 && boxes[i].fillByPlayer[player2] / boxArea > matchLimit) {
				boxes[i].state = GridBox::P2_MATCHED;
			} else {
				boxes[i].state = GridBox::P2_UNMATCHED;
				p2Passed = false;
			}
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

Grid::~Grid() {
	delete [] boxes;
}