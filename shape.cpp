#include "precompiled.h"
#include "shape.h"

const int Shape::I_BOXES[][24] = {
	 {0, 1, 0, 0, 2, 0,
	  0, 1, 0, 0, 2, 0, 
	  0, 1, 0, 0, 2, 0, 
	  0, 1, 0, 0, 2, 0}

	 /*,{0, 0, 1, 2, 0, 0,
		 0, 0, 1, 2, 0, 0, 
		 0, 0, 1, 2, 0, 0, 
		 0, 0, 1, 2, 0, 0}*/
};

const Shape Shape::I[] = {
	Shape("I", I_BOXES[0], D2D1::ColorF(D2D1::ColorF::Gray, 0.3f))
};

const int Shape::J_BOXES[][24] = {
	 {0, 0, 0, 0, 0, 0,
	  0, 1, 0, 0, 2, 0, 
	  0, 1, 0, 0, 2, 0, 
	  1, 1, 0, 2, 2, 0}
		
	,{0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 
	  1, 1, 1, 2, 2, 2, 
	  0, 0, 1, 0, 0, 2}

	,{0, 0, 0, 0, 0, 0,
	  0, 1, 1, 0, 2, 2, 
	  0, 1, 0, 0, 2, 0, 
	  0, 1, 0, 0, 2, 0}

	,{0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 
	  1, 0, 0, 2, 0, 0, 
	  1, 1, 1, 2, 2, 2}
};

const Shape Shape::J[] = {
	 Shape("J", J_BOXES[0], D2D1::ColorF(D2D1::ColorF::MediumOrchid, 0.25f))
	,Shape("J", J_BOXES[1], D2D1::ColorF(D2D1::ColorF::MediumOrchid, 0.25f))
	,Shape("J", J_BOXES[2], D2D1::ColorF(D2D1::ColorF::MediumOrchid, 0.25f))
	,Shape("J", J_BOXES[3], D2D1::ColorF(D2D1::ColorF::MediumOrchid, 0.25f))
};

const int Shape::L_BOXES[][24] = {
	{0, 0, 0, 0, 0, 0,
	 0, 1, 0, 0, 2, 0, 
	 0, 1, 0, 0, 2, 0, 
	 0, 1, 1, 0, 2, 2}

	,{0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 
	  0, 0, 1, 0, 0, 2, 
	  1, 1, 1, 2, 2, 2}

	,{0, 0, 0, 0, 0, 0,
	  1, 1, 0, 2, 2, 0, 
	  0, 1, 0, 0, 2, 0, 
	  0, 1, 0, 0, 2, 0}

	,{0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 
	  1, 1, 1, 2, 2, 2, 
	  1, 0, 0, 2, 0, 0}
};

const Shape Shape::L[] = {
	 Shape("L", L_BOXES[0], D2D1::ColorF(D2D1::ColorF::Gold, 0.25f))
	,Shape("L", L_BOXES[1], D2D1::ColorF(D2D1::ColorF::Gold, 0.25f))
	,Shape("L", L_BOXES[2], D2D1::ColorF(D2D1::ColorF::Gold, 0.25f))
	,Shape("L", L_BOXES[3], D2D1::ColorF(D2D1::ColorF::Gold, 0.25f))
};

const int Shape::O_BOXES[][24] = {
	{0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 
	 1, 1, 0, 2, 2, 0, 
	 1, 1, 0, 2, 2, 0},

	/*,{0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 
		0, 1, 1, 0, 2, 2, 
		0, 1, 1, 0, 2, 2}*/
};

const Shape Shape::O[] = {
	Shape("O", O_BOXES[0], D2D1::ColorF(D2D1::ColorF::LimeGreen, 0.25f))
};

const int Shape::Z_BOXES[][24] = {
	 {0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 
	  1, 1, 0, 2, 2, 0, 
	  0, 1, 1, 0, 2, 2}

	,{0, 0, 0, 0, 0, 0,
	  0, 0, 1, 0, 0, 2, 
	  0, 1, 1, 0, 2, 2, 
	  0, 1, 0, 0, 2, 0},

	/*,{0, 0, 0, 0, 0, 0,
		0, 1, 0, 0, 2, 0, 
		1, 1, 0, 2, 2, 0, 
		1, 0, 0, 2, 0, 0}*/
};

const Shape Shape::Z[] = {
	 Shape("Z", Z_BOXES[0], D2D1::ColorF(D2D1::ColorF::Red, 0.2f))
	,Shape("Z", Z_BOXES[1], D2D1::ColorF(D2D1::ColorF::Red, 0.2f))
};

const int Shape::T_BOXES[][24] = {

	 {0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 
	  1, 1, 1, 2, 2, 2, 
	  0, 1, 0, 0, 2, 0}

	,{0, 0, 0, 0, 0, 0,
	  1, 0, 0, 2, 0, 0, 
	  1, 1, 0, 2, 2, 0,
	  1, 0, 0, 2, 0, 0}

	,{0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 
	  0, 1, 0, 0, 2, 0, 
	  1, 1, 1, 2, 2, 2}

	,{0, 0, 0, 0, 0, 0,
	  0, 0, 1, 0, 0, 2, 
	  0, 1, 1, 0, 2, 2, 
	  0, 0, 1, 0, 0, 2}
};

const Shape Shape::T[] = {
	 Shape("T", T_BOXES[0], D2D1::ColorF(D2D1::ColorF::DarkOrange, 0.35f))
	,Shape("T", T_BOXES[1], D2D1::ColorF(D2D1::ColorF::DarkOrange, 0.35f))
	,Shape("T", T_BOXES[2], D2D1::ColorF(D2D1::ColorF::DarkOrange, 0.35f))
	,Shape("T", T_BOXES[3], D2D1::ColorF(D2D1::ColorF::DarkOrange, 0.35f))
};

const int Shape::S_BOXES[][24] = {
	   {0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 
		0, 1, 1, 0, 2, 2, 
		1, 1, 0, 2, 2, 0}

      ,{0, 0, 0, 0, 0, 0,
		0, 1, 0, 0, 2, 0, 
		0, 1, 1, 0, 2, 2, 
		0, 0, 1, 0, 0, 2}
	
	/*,{0, 0, 0, 0, 0, 0,
		1, 0, 0, 2, 0, 0, 
		1, 1, 0, 2, 2, 0, 
		0, 1, 0, 0, 2, 0}*/
};

const Shape Shape::S[] = {
	 Shape("S", S_BOXES[0], D2D1::ColorF(D2D1::ColorF::DeepSkyBlue, 0.25f))
	,Shape("S", S_BOXES[1], D2D1::ColorF(D2D1::ColorF::DeepSkyBlue, 0.25f))
};

const int Shape::ORI_COUNTS[] = {
	 ARRAYSIZE(I_BOXES)
	,ARRAYSIZE(J_BOXES)
	,ARRAYSIZE(L_BOXES)
	,ARRAYSIZE(O_BOXES)
	,ARRAYSIZE(Z_BOXES)
	,ARRAYSIZE(T_BOXES)
	,ARRAYSIZE(S_BOXES)
};

const int Shape::SHAPE_COUNT = ARRAYSIZE(ORI_COUNTS);

Shape::Shape(LPCSTR name, const int * boxes, D2D1::ColorF color) :
		name(name), boxes(boxes), color(color) {}
		
const Shape * Shape::getRandomShape() {
	int shape = rand() % SHAPE_COUNT + 1;
	return getShapeByLabel('0' + shape);
}

const Shape * Shape::getShapeByLabel(char shape, int ori) {
	switch (shape) {
	case '1': case 'i': case 'I':
		return getShape(I, 0, ori);

	case '2': case 'j': case 'J':
		return getShape(J, 1, ori);

	case '3': case 'l': case 'L':
		return getShape(L, 2, ori); 

	case '4': case 'o': case 'O':
		return getShape(O, 3, ori); 

	case '5': case 'z': case 'Z':
		return getShape(Z, 4, ori); 

	case '6': case 't': case 'T':
		return getShape(T, 5, ori); 

	case '7': case 's': case 'S':
		return getShape(S, 6, ori); 

	default:
		return NULL;
	}
}

const Shape * Shape::getShape(const Shape * shapeType, int shape, int ori) {
	if (shape < 0 || shape >= SHAPE_COUNT ||
	    ori < -1 || ori >= ORI_COUNTS[shape]) {
		return NULL;
	} 

	if (ori == -1) {
		return &shapeType[rand() % ORI_COUNTS[shape]];
	} else {
		return &shapeType[ori];
	}
}