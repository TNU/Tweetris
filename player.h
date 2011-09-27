#pragma once

#include "precompiled.h"

class PlayerProfile {
public:
	
	int index;
	ID2D1SolidColorBrush * matchedBrush;
	ID2D1SolidColorBrush * unmatchedBrush;
	ID2D1SolidColorBrush * outBrush;

	PlayerProfile(D2D1::ColorF matched, 
				  D2D1::ColorF unmatched, 
				  D2D1::ColorF out);

	bool loadBrushes(ID2D1HwndRenderTarget * canvas);
	void cleanBrushes();

	~PlayerProfile();

private: 
	D2D1::ColorF matchedColor;
	D2D1::ColorF unmatchedColor;
	D2D1::ColorF outColor;
	
	bool brushesLoaded;
};
