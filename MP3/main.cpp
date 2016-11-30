#include<iostream>
#include<tchar.h>
#include<windows.h>
#include<shobjidl.h> 
#include"Mp3.h"		
#include<commctrl.h>
#include<thread>

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
	using namespace std;
/*매크로*/
	#define SONGNAME 1			//레이블
	#define PLAYINGLABEL 2	
	#define TOTALLABEL 3	
	#define SEPERATINGLABEL 4	
	#define PLAYBUTTON 101		//버튼
	#define STOPBUTTON 102		
	#define PAUSEBUTTON 103		
	#define ADDBUTTON 104
	#define EXITBUTTON 105
	#define DELETEBUTTON 106
	#define VOLUMEBAR 200		//트랙바
	#define PLAYINGBAR 201
	#define MUSICLIST 300		//리스트박스
	#define REPEATCHECK 400		//체크박스
	#define SEQUENTIALCHECK 401
/*전역 변수*/
	HWND hWnd;					//부모윈도우
	HWND songname;				//레이블
	HWND Playing_label;
	HWND total_label;
	HWND seperating_label;
	HWND play_button;			//버튼
	HWND stop_button;
	HWND pause_button;
	HWND ADD_button;
	HWND delete_button;
	HWND exit_button;
	HWND Volume_bar;			//트랙바
	HWND playing_bar;
	HWND music_list;			//리스트박스
	HWND repeat_check;
	HWND sequential_check;

	MP3* mp3 = new MP3;			//MUSIC PLAYER 클래스
	HINSTANCE g_hInst;
	INT list_index, playing_index; //인덱스
/*함수 선언*/
	vector<wstring>* openFile(HWND hWnd);
	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	VOID Music_insert(MP3* mp3);
	VOID Music_delete();
	VOID CreateButton(HWND hWnd);
	VOID Ready();
	VOID Play();
	VOID Stop();

vector<wstring>* openFile(HWND hWnd) {
		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		COMDLG_FILTERSPEC rgSpec[] = { { L"mp3", L"*.mp3" },{L"wav", L"*.wav"},{L"all files",L"*.*"} };
		if (SUCCEEDED(hr)) {
			IFileOpenDialog *pFileOpen;
			hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
			if (SUCCEEDED(hr)) {
				hr = pFileOpen->SetOptions(FOS_ALLOWMULTISELECT);
				hr = pFileOpen->SetFileTypes(3, rgSpec);
				hr = pFileOpen->Show(hWnd);
				if (SUCCEEDED(hr)) {
					IShellItemArray *psiaResult;
					hr = pFileOpen->GetResults(&psiaResult);
					if (SUCCEEDED(hr)) {
						
						DWORD dwNumItems = 0;
						vector<wstring> *strSelected = new vector<wstring>;
						hr = psiaResult->GetCount(&dwNumItems);
						for (DWORD i = 0; i < dwNumItems; i++) {
							IShellItem *psi = NULL;
							LPWSTR pszFilePath;
							hr = psiaResult->GetItemAt(i, &psi);
							hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
							if (SUCCEEDED(hr)) {
								strSelected->push_back(pszFilePath);
								CoTaskMemFree(pszFilePath);
							}
						}
						return strSelected;
					}
				}
				pFileOpen->Release();
			}
			CoUninitialize();
		}
		vector<wstring> none;
		return &none;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static HANDLE timer;
	switch (message) {
	case WM_COMMAND: {
		if (((HWND)lParam) && (HIWORD(wParam) == BN_CLICKED))
			switch (LOWORD(wParam)) {
				case PLAYBUTTON: {
					Play();
					break;
					}
				case STOPBUTTON: {
					Stop();
					break;
				}
				case PAUSEBUTTON: {
					mp3->pause();
					break;
				}
				case ADDBUTTON: {
					Music_insert(mp3);
					break;
				}
				case DELETEBUTTON: {
					list_index = SendMessage(music_list, LB_GETCURSEL, 0, 0);
					Music_delete();
					break;
				}
				case EXITBUTTON: {
					SendMessage(hWnd, WM_DESTROY, 0, 0);
					break;
				}
				default:		break;
			}
		else if ((HWND)lParam == music_list) {
			switch (LOWORD(wParam)) {
				case MUSICLIST: {
				switch (HIWORD(wParam)) {
				case LBN_SELCHANGE:
					list_index = SendMessage(music_list, LB_GETCURSEL, 0, 0); //포커싱 인덱스 호출
					break;
				case LBN_DBLCLK:
					list_index = SendMessage(music_list, LB_GETCURSEL, 0, 0); //포커싱 인덱스 호출
					Play();
					break;
				}
				

			}
				default: break;
			}
		}
		break;
	}
	case WM_HSCROLL: {
		if ((HWND)lParam == Volume_bar) {
			DWORD vol_pos = SendMessage(Volume_bar, TBM_GETPOS, 0, 0);	//볼륨 컨트롤 -> 값 가져오기
			mp3->setVolume(vol_pos);									//볼륨 설정
		}
		if ((HWND)lParam == playing_bar) {
			if (mp3->isPlaying()) {
				mp3->stop();
				DWORD pln_pos = SendMessage(playing_bar, TBM_GETPOS, 0, 0); 
				mp3->setPlayingtime(pln_pos * 1000);
				mp3->play();
			}
			else if (mp3->isPaused()) {
				mp3->stop();
				DWORD pln_pos = SendMessage(playing_bar, TBM_GETPOS, 0, 0);
				mp3->setPlayingtime(pln_pos * 1000);
			}
			else{
				DWORD pln_pos = SendMessage(playing_bar, TBM_GETPOS, 0, 0);
				mp3->setPlayingtime(pln_pos * 1000);
			}
		}
		break;
	}
	case WM_DESTROY: {
		delete mp3;
		PostQuitMessage(0);
		break;
	}
	case WM_CREATE: {
		CreateButton(hWnd); //버튼 생성 함수
		timer = (HANDLE)SetTimer(hWnd, 1, 100, NULL);
		break;}
	case WM_TIMER: {
		if (mp3->isPlaying()) {
			SetWindowText(Playing_label, mp3->getTimeFormat(mp3->GetPlayingtime()));
			SendMessage(playing_bar, TBM_SETPOS, TRUE, mp3->GetPlayingtime() / 1000);
		}
		if (mp3->GetPlayingtime() == mp3->getSongLength()) {
			mp3->stop();
			SendMessage(playing_bar, TBM_SETPOS, TRUE, 0);
			SetWindowText(Playing_label, mp3->getTimeFormat(mp3->GetPlayingtime()));
			if (SendMessage(repeat_check, BM_GETCHECK, 0, 0) == BST_CHECKED) 			//반복 체크
				if (SendMessage(sequential_check, BM_GETCHECK, 0, 0) == BST_CHECKED) {		//순차 체크
					list_index = (playing_index + 1)%mp3->get_vector_size();
					SendMessage(music_list, LB_SETCURSEL, list_index, NULL);
					Play();
				}
				else mp3->play();															//순차 비체크
			else {																		//반복 비체크
				if (SendMessage(sequential_check, BM_GETCHECK, 0, 0) == BST_CHECKED)		//순차 체크
					if ((playing_index + 1) != mp3->get_vector_size()) {					//마지막인지 체크
						list_index = (playing_index + 1);
						SendMessage(music_list, LB_SETCURSEL, list_index, NULL);
						Play();
					}
			}
		}
		break; }
	default: return DefWindowProc(hWnd, message, wParam, lParam);
	}
}
void refresh_listbox() {
	SendMessage(music_list, LB_RESETCONTENT, TRUE, NULL);
	for (int count = 0; count < mp3->get_vector_size(); count++)
		SendMessage(music_list, LB_ADDSTRING, TRUE, (LPARAM)mp3->get_vector_item(count).songname.GetString());
}
void Music_insert(MP3* mp3) {
	CString path;
	vector<wstring> *vec_tor = openFile(hWnd);
	for (unsigned int count = 0; count < vec_tor->size();count++) {
		path = vec_tor->at(count).c_str();
		if (path == L"") return;
		path.Replace(L"\\", L"/");
		for (int count = 0; count < mp3->get_vector_size(); count++)
			if (mp3->get_vector_item(count).path == path) {
				SendMessage(music_list, LB_SETCURSEL, count, NULL);
				list_index = count;
				return;
			}
		list_index = mp3->vector_add(path);
		refresh_listbox();
		SendMessage(music_list, LB_SETCURSEL, list_index, NULL);
	}
}
void Music_delete() {
	if (mp3->get_vector_size() >= 1){
		if (mp3->getPath() == mp3->get_vector_item(list_index).path) {
			mp3->stop();
			mp3->Close();
		}
		mp3->vector_remove(list_index);
		refresh_listbox();
		list_index = -1; //선택 인덱스가 없으면 -1로 
		playing_index = list_index;
	}
}
void Ready() {
	mp3->stop();
	if(mp3->getFlag())mp3->Close();
	if(mp3->get_vector_size() == 0) return;
	mp3->setPath(list_index);
	mp3->Load();
	SendMessage(playing_bar, TBM_SETRANGE, TRUE, MAKELONG(0, mp3->getSongLength() / 1000));
	SendMessage(playing_bar, TBM_SETPOS, TRUE, 0);
	SetWindowText(Playing_label, mp3->getTimeFormat(mp3->GetPlayingtime()));
	SetWindowText(total_label, mp3->getTimeFormat(mp3->getSongLength()));
	SetWindowText(songname, mp3->get_vector_item(list_index).songname);
}
void Play() {
	if (mp3->get_vector_size() == 0) Music_insert(mp3);
	if (list_index == -1) return;
	if (mp3->getPath() != mp3->get_vector_item(list_index).path) {
		Ready();
	}
	mp3->play();
	playing_index = list_index;
	mp3->setVolume(SendMessage(Volume_bar, TBM_GETPOS, 0, 0));
}
void Stop() {
	mp3->stop();
	playing_index = -1;
	SendMessage(playing_bar, TBM_SETPOS, TRUE, 0);
	SetWindowText(Playing_label, mp3->getTimeFormat(mp3->GetPlayingtime()));
}
void CreateButton(HWND hWnd) {
	/*곡 이름 레이블*/
	songname = CreateWindowEx(0,									//확장 가능성
		TEXT("static"),												//분류 : static
		TEXT("Song name"),											//텍스트
		WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_ENABLESELRANGE,	//
		0, 0,														//위치
		280, 20,													//사이즈
		hWnd,														//부모 : hWnd
		(HMENU)SONGNAME,											//Songname
		g_hInst,													//인스턴스
		NULL);														//?
	/* 시간 레이블*/
	Playing_label = CreateWindowEx(0,					//확장 가능성
		TEXT("static"),												//분류 : 트랙바
		TEXT("00:00"),													//텍스트
		WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_ENABLESELRANGE,	//
		195, 20,													//위치
		40, 20,														//사이즈
		hWnd,														//부모 : hWnd
		(HMENU)PLAYINGLABEL,										//Playing
		g_hInst,													//인스턴스
		NULL);														//?
	seperating_label = CreateWindowEx(0,							//확장 가능성
		TEXT("static"),												//분류 : static
		TEXT("/"),													//텍스트
		WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_ENABLESELRANGE,	//
		235, 20,													//위치
		5, 20,														//사이즈
		hWnd,														//부모 : hWnd
		(HMENU)SEPERATINGLABEL,										//seperator
		g_hInst,													//인스턴스
		NULL);														//?
	total_label = CreateWindowEx(0,									//확장 가능성
		TEXT("static"),												//분류 : static
		TEXT("00:00"),													//텍스트
		WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_ENABLESELRANGE,	//
		240, 20,													//위치
		40, 20,														//사이즈
		hWnd,														//부모 : hWnd
		(HMENU)TOTALLABEL,											//total
		g_hInst,													//인스턴스
		NULL);														//?
	/*재생 트랙바*/
	playing_bar = CreateWindowEx(0,									//확장 가능성
		TRACKBAR_CLASS,												//분류 : 트랙바
		NULL,														//
		WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_ENABLESELRANGE,	//
		0, 40,														//위치
		280, 20,													//사이즈
		hWnd,														//부모 : hWnd
		(HMENU)PLAYINGBAR,											//Playing
		g_hInst,													//인스턴스
		NULL);														//?
	SendMessage(playing_bar, TBM_SETRANGE, TRUE, MAKELONG(0, (int)mp3->getSongLength()));
	SendMessage(playing_bar, TBM_SETPOS, TRUE, 0);
	/*플레이 버튼*/
	HWND play_button = CreateWindowEx(TBSTYLE_FLAT,							//확장 가능성
		TEXT("BUTTON"),												//분류 : 버튼
		TEXT("Play"),													//버튼 텍스트
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,					//
		0, 60,														//위치
		60, 20,														//사이즈
		hWnd,														//부모 : hWnd
		(HMENU)PLAYBUTTON,											//PLAY
		g_hInst,													//인스턴스
		NULL);														//?
	/*일시정지 버튼*/
	pause_button = CreateWindowEx(0,								//확장 가능성
		TEXT("BUTTON"),												//분류 : 버튼
		TEXT("Pause"),													//버튼 텍스트
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,					//
		60, 60,														//위치
		60, 20,														//사이즈
		hWnd,														//부모 : hWnd
		(HMENU)PAUSEBUTTON,											//PAUSE
		g_hInst,													//인스턴스
		NULL);														//?
	/*중지 버튼*/
	HWND stop_button = CreateWindowEx(0,							//확장 가능성
		TEXT("BUTTON"),												//분류 : 버튼
		TEXT("Stop"),													//버튼 텍스트
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,					//
		120, 60,														//위치
		60, 20,														//사이즈
		hWnd,														//부모 : hWnd
		(HMENU)STOPBUTTON,											//STOP
		g_hInst,													//인스턴스
		NULL);														//?
	/*볼륨 트랙바*/
	Volume_bar = CreateWindowEx(0,									//확장 가능성
		TRACKBAR_CLASS,												//분류 : 트랙바
		NULL,														//
		WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_ENABLESELRANGE,	//
		180, 60,														//위치
		100, 20,													//사이즈
		hWnd,														//부모 : hWnd
		(HMENU)VOLUMEBAR,											//VOLUME
		g_hInst,													//인스턴스
		NULL);														//?
	SendMessage(Volume_bar, TBM_SETPOS, TRUE, 50);
	/*음악 리스트*/
	music_list = CreateWindowEx(0,									//확장 가능성
		TEXT("listbox"),												//분류 : 버튼
		NULL,													//버튼 텍스트
		WS_CHILD | WS_VISIBLE | LBS_DISABLENOSCROLL | WS_VSCROLL | BS_DEFPUSHBUTTON,					//
		0, 80,														//위치
		280, 100,													//사이즈
		hWnd,														//부모 : hWnd
		(HMENU)MUSICLIST,											//PAUSE
		g_hInst,													//인스턴스
		NULL);														//?
	/*곡 추가 버튼*/
	ADD_button = CreateWindowEx(0,									//확장 가능성
		TEXT("BUTTON"),												//분류 : 버튼
		TEXT("Add"),												//버튼 텍스트
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,					//
		0, 175,														//위치
		40, 20,														//사이즈
		hWnd,														//부모 : hWnd
		(HMENU)ADDBUTTON,											//PAUSE
		g_hInst,													//인스턴스
		NULL);														//?
	/*곡 삭제 버튼*/
	delete_button = CreateWindowEx(0,									//확장 가능성
		TEXT("BUTTON"),												//분류 : 버튼
		TEXT("Del"),												//버튼 텍스트
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,					//
		40, 175,														//위치
		40, 20,														//사이즈
		hWnd,														//부모 : hWnd
		(HMENU)DELETEBUTTON,											//PAUSE
		g_hInst,													//인스턴스
		NULL);														//?
	/*체크박스*/
	repeat_check = CreateWindowEx(0,
		TEXT("button"),
		TEXT("Repeat"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		80, 175,
		70, 20,
		hWnd,
		(HMENU)REPEATCHECK,
		g_hInst,
		NULL);
	sequential_check = CreateWindowEx(0,
		TEXT("button"),
		TEXT("Sequential"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		150, 175,
		90, 20,
		hWnd,
		(HMENU)SEQUENTIALCHECK,
		g_hInst,
		NULL);
	/*종료 버튼*/
	exit_button = CreateWindowEx(0,									//확장 가능성
		TEXT("BUTTON"),												//분류 : 버튼
		TEXT("Exit"),												//버튼 텍스트
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,					//
		240, 175,														//위치
		40, 20,														//사이즈
		hWnd,														//부모 : hWnd
		(HMENU)EXITBUTTON,											//PAUSE
		g_hInst,													//인스턴스
		NULL);														//?
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
													/* 윈도우를 위한 처리기 */
	MSG msg;													/* 프로그램에 전송된 메시지가 저장 */
	WNDCLASSEX wcex;											/* windowclass를 위한 자료 구조체 */
	TCHAR szClassName[] = TEXT("MyFirstProgram");

	int monitor_width = GetSystemMetrics(SM_CXFULLSCREEN);
	int monitor_height = GetSystemMetrics(SM_CYFULLSCREEN);


	/* 윈도우 구조체 */
	g_hInst = hInstance;
	wcex.hInstance = hInstance;
	wcex.lpszClassName = szClassName;
	wcex.lpfnWndProc = WndProc;									/* 이 함수는 윈도우에 의해 호출됩니다 */
	wcex.style = CS_DBLCLKS;									/* 더블 클릭을 잡아냄 */
	wcex.cbSize = sizeof(WNDCLASSEX);		
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);				/* 기본 아이콘과 포인터 사용 */
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.lpszMenuName = NULL;									/* 메뉴 사용안함 */
	wcex.cbClsExtra = 0;										/* 윈도우 클래스 뒤에 여우 바이트 없음 */
	wcex.cbWndExtra = 0;										/* 윈도우 인스턴스 또는 구조체 */
	wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);			// 창의 배경으로 윈도우의 기본 색상 사용
	if (!RegisterClassEx(&wcex)) return 0;						// 실패 시 프로그램 종료
	/*윈도우 폼*/
	hWnd = CreateWindowEx(0,									//확장가능성
		szClassName,											//클래스 이름
		TEXT("Muisc Player"),									//제목
		WS_POPUP | WS_VISIBLE | WS_SYSMENU | WS_THICKFRAME,										//앱윈도
		monitor_width -295,											//프로그램이 화면에 표시될 때의
		monitor_height - 210,											//윈도우 위치 선언
		295,210,												//사이즈
		HWND_DESKTOP,											//부모 지정 : 바탕화면이 부모
		NULL,													//메뉴 : 없음
		hInstance,												//프로그램 인스턴스
		NULL);													//윈도우 생성 자료 : 없음
	ShowWindow(hWnd, SW_SHOW);
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}