#include "precompiled.h"
#include "player.h"

PlayerProfile::PlayerProfile(D2D1::ColorF matched, 
							 D2D1::ColorF unmatched, 
							 D2D1::ColorF out) 
			   :matchedColor(matched), 
				unmatchedColor(unmatched), 
				outColor(out) {

	brushesLoaded	= false;
	matchedBrush		= NULL;
	unmatchedBrush	= NULL;
	outBrush	= NULL;
	
	index = 0;
}

bool PlayerProfile::loadBrushes(ID2D1HwndRenderTarget * canvas) {

	if (brushesLoaded) {
		return true;
	}

	HRESULT result = canvas->CreateSolidColorBrush(matchedColor, &matchedBrush);
	
	if (SUCCEEDED(result)) {
		result = canvas->CreateSolidColorBrush(unmatchedColor, &unmatchedBrush);
	}

	if (SUCCEEDED(result)) {
		result = canvas->CreateSolidColorBrush(outColor, &outBrush);
	}

	if (FAILED(result)) {
		cleanBrushes();
		return false;
	}

	brushesLoaded = true;
	return true;
}

void PlayerProfile::cleanBrushes() {
	if (matchedBrush != NULL) {
		matchedBrush->Release();
		matchedBrush = NULL;
	}

	if (unmatchedBrush != NULL) {
		unmatchedBrush->Release();
		unmatchedBrush = NULL;
	}

	if (outBrush != NULL) {
		outBrush->Release();
		outBrush = NULL;
	}

	brushesLoaded = false;
}

PlayerProfile::~PlayerProfile() {
	cleanBrushes();
}