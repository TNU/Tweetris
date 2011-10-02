#include "precompiled.h"
#include "tweetris.h"

bool Tweetris::draw() {
	HRESULT result;

	if (! toolsLoaded) {
		loadTools();
		if (! toolsLoaded) {
			return false;
		}
	}

	canvas->BeginDraw();

	canvas->Clear(backgroundColor);
	canvas->DrawBitmap(videoBitmap, outputArea);
	//canvas->DrawBitmap(depthBitmap, outputArea);

	if (shape != NULL) {
		drawGrid();
	}

	result = canvas->EndDraw();

	if (result == D2DERR_RECREATE_TARGET) {
		unloadTools();
	}

	if (SUCCEEDED(result)) {
		InterlockedIncrement(&frameCount);
	}
	
	return true;
}

bool Tweetris::updateVideoBitmap() {
	const NUI_IMAGE_FRAME * videoFrame = NULL;
	HRESULT result = NuiImageStreamGetNextFrame(
					 videoStream, 0, &videoFrame);

	if (FAILED(result)) {
		return false;
	}
	
	NuiImageBuffer * videoTexture = videoFrame->pFrameTexture;
	KINECT_LOCKED_RECT LockedRect;
	videoTexture->LockRect(0, &LockedRect, NULL, 0);
	RGBQUAD * videoBuffer = (RGBQUAD *) LockedRect.pBits;

	D2D1_RECT_U videoRect = D2D1::RectU(0, 0, videoSize.width, videoSize.height);
	videoBitmap->CopyFromMemory(&videoRect, videoBuffer, videoTexture->Pitch());

	NuiImageStreamReleaseFrame(videoStream, videoFrame);
	return true;
}

RGBQUAD Tweetris::depthToColor(USHORT depth, USHORT player) {
    // transform 13-bit depth information into an 8-bit intensity appropriate
    // for display (we disregard information in most significant bit)
    BYTE l = 255 - (BYTE)(256 * depth / 0x8fff);

    RGBQUAD q;
    q.rgbRed = q.rgbBlue = q.rgbGreen = 0;
	q.rgbReserved = 255;

    switch(player)
    {
    case 0:
        q.rgbRed = 0;
        q.rgbBlue = 0;
        q.rgbGreen = 0;
        q.rgbReserved = 0;
        break;
    case 1:
        q.rgbRed = l;
        break;
    case 2:
        q.rgbGreen = l;
        break;
    case 3:
        q.rgbRed = l / 4;
        q.rgbGreen = l;
        q.rgbBlue = l;
        break;
    case 4:
        q.rgbRed = l;
        q.rgbGreen = l;
        q.rgbBlue = l / 4;
        break;
    case 5:
        q.rgbRed = l;
        q.rgbGreen = l / 4;
        q.rgbBlue = l;
        break;
    case 6:
        q.rgbRed = l / 2;
        q.rgbGreen = l / 2;
        q.rgbBlue = l;
        break;
    case 7:
        q.rgbRed = 255 - ( l / 2 );
        q.rgbGreen = 255 - ( l / 2 );
        q.rgbBlue = 255 - ( l / 2 );
    }

    return q;
}


void Tweetris::drawGrid() {

	for (int i = 0; i < grid.numBoxes; i++) {
		
		canvas->DrawRectangle(grid.boxes[i].rect, borderBrush);

		switch (grid.boxes[i].state) {
		case GridBox::IGNORED:
			canvas->FillRectangle(grid.boxes[i].rect, ignoredBrush);
			break;
		case GridBox::P1_UNMATCHED:
			canvas->FillRectangle(grid.boxes[i].rect, player1.unmatchedBrush);
			break;
		case GridBox::P1_MATCHED:
			canvas->FillRectangle(grid.boxes[i].rect, player1.matchedBrush);
			break;
		case GridBox::P1_OUT: 
			canvas->FillRectangle(grid.boxes[i].rect, player1.outBrush);
			break;
		case GridBox::P2_UNMATCHED:
			canvas->FillRectangle(grid.boxes[i].rect, player2.unmatchedBrush);
			break;
		case GridBox::P2_MATCHED:
			canvas->FillRectangle(grid.boxes[i].rect, player2.matchedBrush);
			break;
		case GridBox::P2_OUT: 
			canvas->FillRectangle(grid.boxes[i].rect, player2.outBrush);
			break;
		}
	}
	
	return;
}