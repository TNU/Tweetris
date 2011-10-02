#pragma once

#include "precompiled.h"

class Shape {
private:
	static const int I_BOXES[][24];
	static const Shape I[];
	static const int J_BOXES[][24];
	static const Shape J[];
	static const int L_BOXES[][24];
	static const Shape L[];
	static const int O_BOXES[][24];
	static const Shape O[];
	static const int Z_BOXES[][24];
	static const Shape Z[];
	static const int T_BOXES[][24];
	static const Shape T[];
	static const int S_BOXES[][24];
	static const Shape S[];

	static const int ORI_COUNTS[];
	
	static const int SHAPE_COUNT;
	
	Shape(LPCSTR name, const int * boxes, D2D1::ColorF color);
	static const Shape * getShape(const Shape shapeType[], int shape, int orientation = -1);

public:

	static const Shape * getShapeByLabel(char shape, int ori = -1);
	static const Shape * getRandomShape();

	LPCSTR name;
	const int * const boxes;
	D2D1::ColorF color;
};