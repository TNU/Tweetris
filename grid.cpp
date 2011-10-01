#include "precompiled.h"
#include "grid.h"
GridBox::GridBox() {
	rect = D2D1::RectF();
	state = GridBox::IGNORED;
}

Grid::Grid(int numBoxWide, int numBoxHigh, 
		   float left, float right, float centre, float shiftDown) :
		   numBoxWide(numBoxWide), numBoxHigh(numBoxHigh), numBoxes(numBoxWide *  numBoxHigh * 2),
		   left(left), right(right), centre(centre), shiftDown(shiftDown) {

	boxes = new GridBox[numBoxes];

	gridWidth = 0;
	gridHeight = 0;
	boxWidth = 0;
	boxHeight = 0;
	
	marginTop = 0;
	marginLeft = 0;
	marginRight = 0;
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

int Grid::locateBox(LONG x, LONG y) {
	
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
			return -1;
		}
	} else {
		return -1;
	}
	
	numBoxX = (int) ((x - start) / boxWidth) + boxOffset;
	numBoxY = (int) ((y - marginTop) / boxHeight);

	return numBoxY * numBoxWide * 2 + numBoxX;
}

float Grid::getBoxArea() {
	return boxWidth * boxHeight;
}

Grid::~Grid() {
	delete [] boxes;
}