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
/*��ũ��*/
	#define SONGNAME 1			//���̺�
	#define PLAYINGLABEL 2	
	#define TOTALLABEL 3	
	#define SEPERATINGLABEL 4	
	#define PLAYBUTTON 101		//��ư
	#define STOPBUTTON 102		
	#define PAUSEBUTTON 103		
	#define ADDBUTTON 104
	#define EXITBUTTON 105
	#define DELETEBUTTON 106
	#define VOLUMEBAR 200		//Ʈ����
	#define PLAYINGBAR 201
	#define MUSICLIST 300		//����Ʈ�ڽ�
	#define REPEATCHECK 400		//üũ�ڽ�
	#define SEQUENTIALCHECK 401
/*���� ����*/
	HWND hWnd;					//�θ�������
	HWND songname;				//���̺�
	HWND Playing_label;
	HWND total_label;
	HWND seperating_label;
	HWND play_button;			//��ư
	HWND stop_button;
	HWND pause_button;
	HWND ADD_button;
	HWND delete_button;
	HWND exit_button;
	HWND Volume_bar;			//Ʈ����
	HWND playing_bar;
	HWND music_list;			//����Ʈ�ڽ�
	HWND repeat_check;
	HWND sequential_check;

	MP3* mp3 = new MP3;			//MUSIC PLAYER Ŭ����
	HINSTANCE g_hInst;
	INT list_index, playing_index; //�ε���
/*�Լ� ����*/
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
					list_index = SendMessage(music_list, LB_GETCURSEL, 0, 0); //��Ŀ�� �ε��� ȣ��
					break;
				case LBN_DBLCLK:
					list_index = SendMessage(music_list, LB_GETCURSEL, 0, 0); //��Ŀ�� �ε��� ȣ��
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
			DWORD vol_pos = SendMessage(Volume_bar, TBM_GETPOS, 0, 0);	//���� ��Ʈ�� -> �� ��������
			mp3->setVolume(vol_pos);									//���� ����
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
		CreateButton(hWnd); //��ư ���� �Լ�
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
			if (SendMessage(repeat_check, BM_GETCHECK, 0, 0) == BST_CHECKED) 			//�ݺ� üũ
				if (SendMessage(sequential_check, BM_GETCHECK, 0, 0) == BST_CHECKED) {		//���� üũ
					list_index = (playing_index + 1)%mp3->get_vector_size();
					SendMessage(music_list, LB_SETCURSEL, list_index, NULL);
					Play();
				}
				else mp3->play();															//���� ��üũ
			else {																		//�ݺ� ��üũ
				if (SendMessage(sequential_check, BM_GETCHECK, 0, 0) == BST_CHECKED)		//���� üũ
					if ((playing_index + 1) != mp3->get_vector_size()) {					//���������� üũ
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
		list_index = -1; //���� �ε����� ������ -1�� 
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
	/*�� �̸� ���̺�*/
	songname = CreateWindowEx(0,									//Ȯ�� ���ɼ�
		TEXT("static"),												//�з� : static
		TEXT("Song name"),											//�ؽ�Ʈ
		WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_ENABLESELRANGE,	//
		0, 0,														//��ġ
		280, 20,													//������
		hWnd,														//�θ� : hWnd
		(HMENU)SONGNAME,											//Songname
		g_hInst,													//�ν��Ͻ�
		NULL);														//?
	/* �ð� ���̺�*/
	Playing_label = CreateWindowEx(0,					//Ȯ�� ���ɼ�
		TEXT("static"),												//�з� : Ʈ����
		TEXT("00:00"),													//�ؽ�Ʈ
		WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_ENABLESELRANGE,	//
		195, 20,													//��ġ
		40, 20,														//������
		hWnd,														//�θ� : hWnd
		(HMENU)PLAYINGLABEL,										//Playing
		g_hInst,													//�ν��Ͻ�
		NULL);														//?
	seperating_label = CreateWindowEx(0,							//Ȯ�� ���ɼ�
		TEXT("static"),												//�з� : static
		TEXT("/"),													//�ؽ�Ʈ
		WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_ENABLESELRANGE,	//
		235, 20,													//��ġ
		5, 20,														//������
		hWnd,														//�θ� : hWnd
		(HMENU)SEPERATINGLABEL,										//seperator
		g_hInst,													//�ν��Ͻ�
		NULL);														//?
	total_label = CreateWindowEx(0,									//Ȯ�� ���ɼ�
		TEXT("static"),												//�з� : static
		TEXT("00:00"),													//�ؽ�Ʈ
		WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_ENABLESELRANGE,	//
		240, 20,													//��ġ
		40, 20,														//������
		hWnd,														//�θ� : hWnd
		(HMENU)TOTALLABEL,											//total
		g_hInst,													//�ν��Ͻ�
		NULL);														//?
	/*��� Ʈ����*/
	playing_bar = CreateWindowEx(0,									//Ȯ�� ���ɼ�
		TRACKBAR_CLASS,												//�з� : Ʈ����
		NULL,														//
		WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_ENABLESELRANGE,	//
		0, 40,														//��ġ
		280, 20,													//������
		hWnd,														//�θ� : hWnd
		(HMENU)PLAYINGBAR,											//Playing
		g_hInst,													//�ν��Ͻ�
		NULL);														//?
	SendMessage(playing_bar, TBM_SETRANGE, TRUE, MAKELONG(0, (int)mp3->getSongLength()));
	SendMessage(playing_bar, TBM_SETPOS, TRUE, 0);
	/*�÷��� ��ư*/
	HWND play_button = CreateWindowEx(TBSTYLE_FLAT,							//Ȯ�� ���ɼ�
		TEXT("BUTTON"),												//�з� : ��ư
		TEXT("Play"),													//��ư �ؽ�Ʈ
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,					//
		0, 60,														//��ġ
		60, 20,														//������
		hWnd,														//�θ� : hWnd
		(HMENU)PLAYBUTTON,											//PLAY
		g_hInst,													//�ν��Ͻ�
		NULL);														//?
	/*�Ͻ����� ��ư*/
	pause_button = CreateWindowEx(0,								//Ȯ�� ���ɼ�
		TEXT("BUTTON"),												//�з� : ��ư
		TEXT("Pause"),													//��ư �ؽ�Ʈ
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,					//
		60, 60,														//��ġ
		60, 20,														//������
		hWnd,														//�θ� : hWnd
		(HMENU)PAUSEBUTTON,											//PAUSE
		g_hInst,													//�ν��Ͻ�
		NULL);														//?
	/*���� ��ư*/
	HWND stop_button = CreateWindowEx(0,							//Ȯ�� ���ɼ�
		TEXT("BUTTON"),												//�з� : ��ư
		TEXT("Stop"),													//��ư �ؽ�Ʈ
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,					//
		120, 60,														//��ġ
		60, 20,														//������
		hWnd,														//�θ� : hWnd
		(HMENU)STOPBUTTON,											//STOP
		g_hInst,													//�ν��Ͻ�
		NULL);														//?
	/*���� Ʈ����*/
	Volume_bar = CreateWindowEx(0,									//Ȯ�� ���ɼ�
		TRACKBAR_CLASS,												//�з� : Ʈ����
		NULL,														//
		WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_ENABLESELRANGE,	//
		180, 60,														//��ġ
		100, 20,													//������
		hWnd,														//�θ� : hWnd
		(HMENU)VOLUMEBAR,											//VOLUME
		g_hInst,													//�ν��Ͻ�
		NULL);														//?
	SendMessage(Volume_bar, TBM_SETPOS, TRUE, 50);
	/*���� ����Ʈ*/
	music_list = CreateWindowEx(0,									//Ȯ�� ���ɼ�
		TEXT("listbox"),												//�з� : ��ư
		NULL,													//��ư �ؽ�Ʈ
		WS_CHILD | WS_VISIBLE | LBS_DISABLENOSCROLL | WS_VSCROLL | BS_DEFPUSHBUTTON,					//
		0, 80,														//��ġ
		280, 100,													//������
		hWnd,														//�θ� : hWnd
		(HMENU)MUSICLIST,											//PAUSE
		g_hInst,													//�ν��Ͻ�
		NULL);														//?
	/*�� �߰� ��ư*/
	ADD_button = CreateWindowEx(0,									//Ȯ�� ���ɼ�
		TEXT("BUTTON"),												//�з� : ��ư
		TEXT("Add"),												//��ư �ؽ�Ʈ
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,					//
		0, 175,														//��ġ
		40, 20,														//������
		hWnd,														//�θ� : hWnd
		(HMENU)ADDBUTTON,											//PAUSE
		g_hInst,													//�ν��Ͻ�
		NULL);														//?
	/*�� ���� ��ư*/
	delete_button = CreateWindowEx(0,									//Ȯ�� ���ɼ�
		TEXT("BUTTON"),												//�з� : ��ư
		TEXT("Del"),												//��ư �ؽ�Ʈ
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,					//
		40, 175,														//��ġ
		40, 20,														//������
		hWnd,														//�θ� : hWnd
		(HMENU)DELETEBUTTON,											//PAUSE
		g_hInst,													//�ν��Ͻ�
		NULL);														//?
	/*üũ�ڽ�*/
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
	/*���� ��ư*/
	exit_button = CreateWindowEx(0,									//Ȯ�� ���ɼ�
		TEXT("BUTTON"),												//�з� : ��ư
		TEXT("Exit"),												//��ư �ؽ�Ʈ
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,					//
		240, 175,														//��ġ
		40, 20,														//������
		hWnd,														//�θ� : hWnd
		(HMENU)EXITBUTTON,											//PAUSE
		g_hInst,													//�ν��Ͻ�
		NULL);														//?
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
													/* �����츦 ���� ó���� */
	MSG msg;													/* ���α׷��� ���۵� �޽����� ���� */
	WNDCLASSEX wcex;											/* windowclass�� ���� �ڷ� ����ü */
	TCHAR szClassName[] = TEXT("MyFirstProgram");

	int monitor_width = GetSystemMetrics(SM_CXFULLSCREEN);
	int monitor_height = GetSystemMetrics(SM_CYFULLSCREEN);


	/* ������ ����ü */
	g_hInst = hInstance;
	wcex.hInstance = hInstance;
	wcex.lpszClassName = szClassName;
	wcex.lpfnWndProc = WndProc;									/* �� �Լ��� �����쿡 ���� ȣ��˴ϴ� */
	wcex.style = CS_DBLCLKS;									/* ���� Ŭ���� ��Ƴ� */
	wcex.cbSize = sizeof(WNDCLASSEX);		
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);				/* �⺻ �����ܰ� ������ ��� */
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.lpszMenuName = NULL;									/* �޴� ������ */
	wcex.cbClsExtra = 0;										/* ������ Ŭ���� �ڿ� ���� ����Ʈ ���� */
	wcex.cbWndExtra = 0;										/* ������ �ν��Ͻ� �Ǵ� ����ü */
	wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);			// â�� ������� �������� �⺻ ���� ���
	if (!RegisterClassEx(&wcex)) return 0;						// ���� �� ���α׷� ����
	/*������ ��*/
	hWnd = CreateWindowEx(0,									//Ȯ�尡�ɼ�
		szClassName,											//Ŭ���� �̸�
		TEXT("Muisc Player"),									//����
		WS_POPUP | WS_VISIBLE | WS_SYSMENU | WS_THICKFRAME,										//������
		monitor_width -295,											//���α׷��� ȭ�鿡 ǥ�õ� ����
		monitor_height - 210,											//������ ��ġ ����
		295,210,												//������
		HWND_DESKTOP,											//�θ� ���� : ����ȭ���� �θ�
		NULL,													//�޴� : ����
		hInstance,												//���α׷� �ν��Ͻ�
		NULL);													//������ ���� �ڷ� : ����
	ShowWindow(hWnd, SW_SHOW);
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}