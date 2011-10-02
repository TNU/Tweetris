#include "precompiled.h"
#include "tweetris.h"

void Tweetris::report(int player, const Shape * shapeCopy) {

	IWICBitmapClipper * clippedSnapshot = NULL;
	IWICBitmapScaler * scaledSnapshot = NULL;

	HRESULT result;
	
	snapshot->BeginDraw();
	snapshot->Clear();
	
	D2D1_RECT_F snapshotArea = D2D1::RectF(videoSize.width, videoSize.height, 0, 0);
	for (int i = 0; i < grid.numBoxes; i++) {
		if (player == 0 ? shapeCopy->boxes[i] == 1 :
		    shapeCopy->boxes[i] == player) {

			if (snapshotArea.left > grid.boxes[i].rect.left) {
				snapshotArea.left = grid.boxes[i].rect.left;
			}

			if (snapshotArea.top > grid.boxes[i].rect.top) {
				snapshotArea.top = grid.boxes[i].rect.top;
			}

			if (snapshotArea.right < grid.boxes[i].rect.right) {
				snapshotArea.right = grid.boxes[i].rect.right;
			}

			if (snapshotArea.bottom < grid.boxes[i].rect.bottom) {
				snapshotArea.bottom = grid.boxes[i].rect.bottom;
			}
				
			snapshot->DrawBitmap(snapshotBitmap, grid.boxes[i].rect, 1, 
									D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, 
									grid.boxes[i].rect);

			shapeBrush->SetColor(shapeCopy->color);
			snapshot->FillRectangle(grid.boxes[i].rect, shapeBrush);
		}
	}

	for (int i = 0; i < grid.numBoxes; i++) {
		if (player == 0 ? shapeCopy->boxes[i] == 1 :
		    shapeCopy->boxes[i] == player) {
			snapshot->DrawRectangle(grid.boxes[i].rect, borderBrush);
		}
	}
	
	result = snapshot->EndDraw();
	
	if (SUCCEEDED(result)) {
		result = snapshotMaker->CreateBitmapClipper(&clippedSnapshot);
	}
	
	WICRect clipArea;
	clipArea.X = (INT) floor(snapshotArea.left);
	clipArea.Y = (INT) floor(snapshotArea.top);
	clipArea.Width = (INT) ceil(snapshotArea.right - snapshotArea.left);
	clipArea.Height = (INT) ceil(snapshotArea.bottom - snapshotArea.top);
	if (SUCCEEDED(result)) {
		result = clippedSnapshot->Initialize(snapshotImage, &clipArea);
	}
	
	if (SUCCEEDED(result)) {
		result = snapshotMaker->CreateBitmapScaler(&scaledSnapshot);
	}
	
	if (SUCCEEDED(result)) {
		scaledSnapshot->Initialize(clippedSnapshot, 
								   clipArea.Width * snapshotScaleRatio, 
								   clipArea.Height * snapshotScaleRatio, 
								   WICBitmapInterpolationModeCubic);
	}

	std::stringstream messageMaker;
	const int NAME_MAX_LEN = 50;
	LPTSTR wname = new TCHAR[NAME_MAX_LEN];
	char name[NAME_MAX_LEN];

	int nameLen = 0;
	if (player != 0) {
		if (player == 1) {
			nameLen = GetDlgItemText(debugDialog, IDC_P1_NAME, wname, NAME_MAX_LEN);
		} else if (player == 2) {
			nameLen = GetDlgItemText(debugDialog, IDC_P2_NAME, wname, NAME_MAX_LEN);
		}

		if (nameLen != 0) {
			wcstombs(name, wname, NAME_MAX_LEN);
			messageMaker << name;
		} else {
			messageMaker << "Player " << player;
		}
		messageMaker << " has made a " << shapeCopy->name << " shape.";
	} else {
		messageMaker << "Nobody managed to make a " 
			         << shapeCopy->name << " shape.";
	}
	delete [] wname;
	console << messageMaker.str().c_str() << TEXT("\r\n");
	updateConsole();

	tweet(scaledSnapshot, messageMaker.str());

	if (scaledSnapshot != NULL) {
		scaledSnapshot->Release();
		scaledSnapshot = NULL;
	}

	if (clippedSnapshot != NULL) {
		clippedSnapshot->Release();
		clippedSnapshot = NULL;
	}
}

void Tweetris::tweet(IWICBitmapSource * image, const std::string & message) {
	
	IWICStream * snapshotStream = NULL;
	IWICBitmapEncoder * snapshotEncoder = NULL;
	IWICBitmapFrameEncode * snapshotFrame = NULL;
	IStream * imageStream = NULL;
		
	HRESULT result = snapshotMaker->CreateStream(&snapshotStream);

	if (SUCCEEDED(result)) {
		CreateStreamOnHGlobal(NULL, TRUE, &imageStream);
	}

	if (SUCCEEDED(result)) {
		result = snapshotStream->InitializeFromIStream(imageStream);
	}
	
	if (SUCCEEDED(result)) {
		result = snapshotMaker->CreateEncoder(GUID_ContainerFormatPng, NULL, &snapshotEncoder);
	}
	
	if (SUCCEEDED(result)) {
		result = snapshotEncoder->Initialize(snapshotStream, WICBitmapEncoderNoCache);
	}
	
	if (SUCCEEDED(result)) {
		result = snapshotEncoder->CreateNewFrame(&snapshotFrame, NULL);
	}
	
	if (SUCCEEDED(result)) {
		result = snapshotFrame->Initialize(NULL);
	}
	
    if (SUCCEEDED(result)) {	
		UINT width, height;
		image->GetSize(&width, &height);
		result = snapshotFrame->SetSize(width, height);
    }
	
	if (SUCCEEDED(result)) {
		WICPixelFormatGUID sourceFormat = GUID_WICPixelFormatDontCare;
		result = snapshotFrame->SetPixelFormat(&sourceFormat);
	}

	if (SUCCEEDED(result)) {
		snapshotFrame->WriteSource(image, NULL);
	}

	snapshotFrame->Commit();
	snapshotEncoder->Commit();
	
	if (snapshotFrame != NULL) {
		snapshotFrame->Release();
		snapshotFrame = NULL;
	}
	
	if (snapshotEncoder != NULL) {
		snapshotEncoder->Release();
		snapshotEncoder = NULL;
	}
	
	if (snapshotStream != NULL) {
		snapshotStream->Release();
		snapshotStream = NULL;
	}

	TwitpicData * data = new TwitpicData();
	data->imageStream = imageStream;
	data->message = message;

	EnterCriticalSection(&uploadQueueAccess);
	uploadQueue.push(data);
	LeaveCriticalSection(&uploadQueueAccess);

	SetEvent(uploadQueueEvent);
}

DWORD WINAPI Tweetris::uploadProc(LPVOID tweetris) {
	Tweetris * pThis = (Tweetris *) tweetris;
	const TwitpicData * data;
	bool noData;

	while (true) {
		// checks until there is data to upload to twitpic
		EnterCriticalSection(&pThis->uploadQueueAccess);
		noData = pThis->uploadQueue.empty();
		LeaveCriticalSection(&pThis->uploadQueueAccess);
		
		// waits until there is data to upload to twitpic
		while (noData) {
			WaitForSingleObject(pThis->uploadQueueEvent, INFINITE);
			
			EnterCriticalSection(&pThis->uploadQueueAccess);
			noData = pThis->uploadQueue.empty();
			LeaveCriticalSection(&pThis->uploadQueueAccess);
		}

		// get the data that are in the queue
		EnterCriticalSection(&pThis->uploadQueueAccess);
		data = pThis->uploadQueue.front();
		pThis->uploadQueue.pop();
		LeaveCriticalSection(&pThis->uploadQueueAccess);

		// if program wants to stop this thread
		if (data == NULL) {
			break;
		}
		
		bool success = pThis->upload(data);
		data->imageStream->Release();
		delete data;
	} 

	return 0;
}
bool Tweetris::upload(const TwitpicData * data) {
	// newline for HTTP POST
	PCSTR NL = "\r\n";
	
	// uses 32 random charaters as the boundary string
	const int BOUNDARY_LEN = 32;
	CHAR boundary[BOUNDARY_LEN + 1];
	for (int i = 0; i < BOUNDARY_LEN; i++) {
		boundary[i] = 'A' + rand() % 26;
	}
	boundary[BOUNDARY_LEN] = '\0';

	// creates string that contains the Content-Type header
	std::wstringstream headerMaker;
	headerMaker << "Content-Type: multipart/form-data; "
				   "boundary=\"" << boundary << "\"" << NL;

	// creates the string that contains the actual data uploaded
	std::stringstream postDataMaker;
	postDataMaker << "--" << boundary << NL
				  << "Content-Disposition: form-data; "
				  	 "name=\"media\"; filename=\"fillername.png\"" << NL
				  << "Content-Type: image/png" << NL
				  << "Content-Transfer-Encoding: binary" << NL
				  << NL;

	// writes the raw PNG image in binary
	STATSTG imageStat;
	LARGE_INTEGER start = {0};
	data->imageStream->Seek(start, STREAM_SEEK_SET, NULL);
	data->imageStream->Stat(&imageStat, STATFLAG_NONAME);
	ULONG imageSize = imageStat.cbSize.LowPart;
	ULONG imageReadSize = 0;
	BYTE * imageBytes = new BYTE[imageSize];
	HRESULT result = data->imageStream->Read(imageBytes, imageSize, &imageReadSize);
	
	postDataMaker.write((PCSTR) imageBytes, imageReadSize);

	delete [] imageBytes;

	if (FAILED(result)) {
		return false;
	}

	// writes the other variables (i.e. username, password, message)
	postDataMaker << NL
		          << "--" << boundary << NL
		          << "Content-Disposition: form-data; name=\"username\"" << NL
		          << NL << USERNAME << NL
		          
		          << "--" << boundary << NL
		          << "Content-Disposition: form-data; name=\"password\"" << NL 
		          << NL << PASSWORD << NL
		          
		          << "--" << boundary << NL
		          << "Content-Disposition: form-data; name=\"message\"" << NL 
		          << NL << data->message << NL
		          
		          << "--" << boundary << "--" << NL;

	std::string response = httpPostData(headerMaker.str(), postDataMaker.str());

	return (response.find("status=\"ok\"") != std::string::npos);
}


std::string Tweetris::httpPostData(const std::basic_string<TCHAR> & header, 
								   const std::string & data) {
	
	// creates the necessary handles and objects
	BOOL results = FALSE;
	HINTERNET session = NULL, connect = NULL, request = NULL;

	session = WinHttpOpen( L"Tweetris/1.0",
							WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
							WINHTTP_NO_PROXY_NAME,
							WINHTTP_NO_PROXY_BYPASS, 0);
	
	if (session != NULL) {
		connect = WinHttpConnect(session, L"twitpic.com", 
								  INTERNET_DEFAULT_PORT, 0);
	}

	if (connect != NULL) {
		request = WinHttpOpenRequest(connect, L"POST", 
									 TWITPIC_UPLOAD_AND_POST_API_URL,
									 NULL, WINHTTP_NO_REFERER, 
									 WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
	}

	// adds the header to the request
	if (request != NULL) {
		results = WinHttpAddRequestHeaders(request,
									       (LPCWSTR) header.c_str(), 
									       (DWORD) header.length(),
									       WINHTTP_ADDREQ_FLAG_ADD);
	}

	// sends the request synchronously
	if (request) {
		results = WinHttpSendRequest(request,
									 WINHTTP_NO_ADDITIONAL_HEADERS, 0,
									 WINHTTP_NO_REQUEST_DATA, 0, 
									 (DWORD) data.length(), 0);
	}

	// writes the data
	if (results) {
		LPDWORD written = 0;
		results = WinHttpWriteData(request, 
								   (LPCVOID) data.c_str(), 
								   (DWORD) data.length(), 
								   written); 
	}

	// get the response
	if (results) {
		results = WinHttpReceiveResponse(request, NULL);
	}

	// downloads to a buffer and writes to a stringstream
	LPSTR responseBuffer;
	DWORD size = 0, downloaded = 0;
	std::stringstream responseMaker;
	if (results) {
		do {
			size = 0;
			if ( !WinHttpQueryDataAvailable(request, &size)) {
				break;	// error
			}
			
			responseBuffer = new char[size + 1];

			// if no errors
			if (WinHttpReadData(request, 
								(LPVOID) responseBuffer,
								size, &downloaded)) {
				responseMaker.write(responseBuffer, downloaded);
			}

			delete [] responseBuffer;
		} while (size > 0);
	}

	// close all handles
	if (request != NULL) {
		WinHttpCloseHandle(request);
	}

	if (connect != NULL) { 
		WinHttpCloseHandle(connect);
	}

	if (session != NULL) {
		WinHttpCloseHandle(session);
	}

	// returns the response as a string
	return responseMaker.str();
}