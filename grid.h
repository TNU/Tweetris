#pragma once

#include "precompiled.h"

class GridBox {
public:
	D2D1_RECT_F rect;

	enum {P1_OUT = -3, 
		  P1_MATCHED,
		  P1_UNMATCHED,
		  IGNORED, 
		  P2_UNMATCHED, 
		  P2_MATCHED, 
		  P2_OUT} state;

	GridBox();
};

class Grid {
public:
	const int numBoxes;

	float left, right, centre;
	float shiftDown;

	GridBox * boxes;

	Grid(int numBoxWide, int numBoxHigh, 
		 float left, float right, float centre, float shiftDown);
	void resizeTo(const D2D1_RECT_F & rect);
	int locateBox(LONG x, LONG y);
	float getBoxArea();
	~Grid();

private:
	const int numBoxWide, numBoxHigh;

	float gridWidth, gridHeight;
	float boxWidth, boxHeight;
	float offsetLeft, offsetTop;
	float marginTop, marginLeft, marginRight;
};
