#include<iostream>																//std
#include<mmsystem.h>															//MCI
#include<Digitalv.h>															//MCI DGV 
#include<atlstr.h>																//CString
#include<string>																//string
#include<vector>																//vector
#pragma comment(lib,"winmm.lib") 

/*player status*/
	#define UNLOADED	0
	#define STOPPED		1
	#define PLAYING		2
	#define PAUSED		3
/*namespace*/
	using namespace std;
/*class*/
class MP3 {
private:
	struct music_list {															//리스트 구조체
		CString path;
		CString songname;
	};
	vector<music_list> list;													//리스트 벡터
	music_list nullstructure = { "","Song name" };								//null구조체
	/*MCI구조체*/
	PMCI_OPEN_PARMS mciOpen;
	PMCI_PLAY_PARMS mciPlay;
	PMCI_STATUS_PARMS mciStatus;
	MCI_DGV_SETAUDIO_PARMS Volume_params;

	CString Loaded_path;															//경로
	DWORD volume;																//볼륨
	int Loaded_index = 0;														//인덱스
	int flag = 0;																//플래그
	void createP() {															//포인터 생성
		mciOpen = new MCI_OPEN_PARMS;
		mciPlay = new MCI_PLAY_PARMS;
		mciStatus = new MCI_STATUS_PARMS;
	}
	void removeP() {															//포인터 제거
		delete mciOpen;
		delete mciPlay;
		delete mciStatus;

	}
public:
	void Load() {																/*오픈&로드 함수*/
		createP();																//포인터 할당
		if (getpathextension(Loaded_path) == "mp3") mciOpen->lpstrDeviceType = L"mpegvideo";	//디바이스 타입 mp3
		else if (getpathextension(Loaded_path) == "wav") mciOpen->lpstrDeviceType = L"waveaudio";	//디바이스 타입 mp3
		else mciOpen->lpstrDeviceType = L"mpegvideo";
		mciOpen->lpstrElementName = Loaded_path;									//음악 파일 주소 ~~.mp3
		if (!mciSendCommand(Loaded_index, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE,
			(DWORD)(LPVOID)mciOpen)) {											//파일 오픈 -> mciOpen에 저장.
			Loaded_index = mciOpen->wDeviceID;									//디바이스 인덱스 설정
			flag = STOPPED;														//플래그 : 로드 후 정지상태
		}
		else flag = UNLOADED;													//플래그 : 로드 실패
	}
	void Close() {																/*디바이스 해제 함수*/
		removeP();																//포인터 제거
		flag = UNLOADED;														//플래그
		mciSendCommand(Loaded_index, MCI_CLOSE, NULL, NULL);					//디바이스 해제
	}
	void play() {																/*	시작함수	*/
		if (flag == STOPPED) {													//정지일때		
			mciPlay->dwCallback = (DWORD)1;
			mciSendCommand(Loaded_index, MCI_PLAY, MCI_NOTIFY, (DWORD)mciPlay);	//재생
			flag = PLAYING;														//플래그
		}
		else if (flag == PAUSED) {												//일시정지일때
			mciSendCommand(Loaded_index, MCI_RESUME, 0, 0);						//다시 재생
			flag = PLAYING;														//플래그
		}
	}
	void stop() {																/*	정지함수	*/
		if (flag > STOPPED) {													//일시중지이거나 재생중일때
			mciSendCommand(Loaded_index, MCI_STOP, 0, NULL);					//중지
			setPlayingtime(0);													//시계를 처음으로
			flag = STOPPED;														//플래그
		}
	}
	void pause() {																/* 일시정지함수	*/
		if (flag == PLAYING) {													//재생중일때
			mciSendCommand(Loaded_index, MCI_PAUSE, 0, (DWORD)NULL);			//일시정지
			flag = PAUSED;														//플래그
		}
	}
	void setVolume(DWORD volume) {												/*볼륨 조절 함수*/
		if (volume < 0) volume = 0;												//0보다 작을 때
		if (volume > 100) volume = 100;											//100보다 클 때
		volume *= 10;															//*10 -> 0~1000범위
		/*볼륨파라미터 구조체 정의*/
		Volume_params.dwCallback = 0;											
		Volume_params.dwItem = MCI_DGV_SETAUDIO_VOLUME;
		Volume_params.dwValue = volume;											//볼륨 설정
		Volume_params.dwOver = 0;
		Volume_params.lpstrAlgorithm = NULL;
		Volume_params.lpstrQuality = NULL;
		/*볼륨 설정*/
		mciSendCommand(Loaded_index, MCI_SETAUDIO, MCI_DGV_SETAUDIO_VALUE | MCI_DGV_SETAUDIO_ITEM, (DWORD)(LPVOID)&Volume_params);
	}
	int getVolume() {
		MCI_STATUS_PARMS Volume_params;
		Volume_params.dwCallback = 0;
		Volume_params.dwItem = MCI_DGV_STATUS_VOLUME;							//돌려받을 값 설정
		mciSendCommand(Loaded_index, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPVOID)&Volume_params);//명령어
		return (int)(Volume_params.dwReturn / 10);								// 볼륨값 리턴
	}
	void setPath(int index)						{						Loaded_path = list.at(index).path;	}	//경로설정
	bool isPlaying()							{ if (flag == PLAYING)	return true;	else return false;	}	//재생중인가?
	bool isPaused()								{ if (flag == PAUSED)	return true;	else return false;	}	//일시정지중인가?
	void setFlag(int i)							{						flag = i;							}	//플래그설정
	int getFlag()								{						return flag;						}	//플래그리턴
	CString getPath()							{						return Loaded_path;					}	//경로 리턴
	CString getSongname(CString path)			{						int slash=path.ReverseFind('/');						
																		int dot = path.ReverseFind('.');
																	return path.Mid(slash+1,dot-slash-1);	}
	CString getTimeFormat(DWORD time) {									CString result;
										result.Format(L"%02lu:%02lu",(time/60000),((time/1000)%60));
																		return result;						}
	DWORD getSongLength()						{						MCI_STATUS_PARMS time;
																		time.dwItem = MCI_STATUS_LENGTH;
										mciSendCommand(Loaded_index,MCI_STATUS,MCI_STATUS_ITEM,(DWORD)&time);
																		return time.dwReturn;				}
	DWORD GetPlayingtime()						{						MCI_STATUS_PARMS mciStatusParms;
													mciStatusParms.dwItem = MCI_STATUS_POSITION;
						mciSendCommand(Loaded_index, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)&mciStatusParms);
																		return mciStatusParms.dwReturn;		}
	void setPlayingtime(DWORD mci_to)			{						MCI_SEEK_PARMS mciSeekParms;											
																		mciSeekParms.dwTo = mci_to;
								mciSendCommand(Loaded_index,MCI_SEEK,MCI_TO,(DWORD)(LPVOID)&mciSeekParms);	}
	int vector_add(CString path)				{						music_list new_st;
																		new_st.path = path;
																		new_st.songname = getSongname(path);
																		list.push_back(new_st);
																		return list.size()-1;				}
	void vector_remove(int index)				{						list.erase(list.begin()+index);		}	//인덱스 제거
	music_list get_vector_item(int index)		{if(list.size()>0)return list.at(index); else return nullstructure; }//인덱스의 벡터 데이터 반환
	int get_vector_size()						{						return list.size();					}	//벡터 사이즈 리턴
	CString getpathextension(CString path) {
		int dot = path.ReverseFind('.');
		return path.Right(path.GetLength() - dot-1);
	}
};