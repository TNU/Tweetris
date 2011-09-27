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
		  P2_OUT} state;	// data

	static const int MAX_NUM_PLAYER_INDICES = 8;
	int fillByPlayer[MAX_NUM_PLAYER_INDICES]; // data

	GridBox();
};

class Grid {
public:
	const int numBoxes;

	float left, right, centre;
	float shiftDown;

	GridBox * boxes;
	float matchLimit, outLimit;

	Grid(int numBoxWide, int numBoxHigh, 
		 float left, float right, float centre, float shiftDown, 
		 float matchLimit, float outLimit);
	void resizeTo(const D2D1_RECT_F & rect);
	void resetData();
	void mark(LONG x, LONG y, int player);
	int analyzeData(int &player1, int &player2,	int * shape, float areaRatio);
	~Grid();

private:
	const int numBoxWide, numBoxHigh;

	float gridWidth, gridHeight;
	float boxWidth, boxHeight;
	float offsetLeft, offsetTop;
	float marginTop, marginLeft, marginRight;
};
