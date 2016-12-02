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
	struct music_list {															//����Ʈ ����ü
		CString path;
		CString songname;
	};
	vector<music_list> list;													//����Ʈ ����
	music_list nullstructure = { "","Song name" };								//null����ü
	/*MCI����ü*/
	PMCI_OPEN_PARMS mciOpen;
	PMCI_PLAY_PARMS mciPlay;
	PMCI_STATUS_PARMS mciStatus;
	MCI_DGV_SETAUDIO_PARMS Volume_params;

	CString Loaded_path;															//���
	DWORD volume;																//����
	int Loaded_index = 0;														//�ε���
	int flag = 0;																//�÷���
	void createP() {															//������ ����
		mciOpen = new MCI_OPEN_PARMS;
		mciPlay = new MCI_PLAY_PARMS;
		mciStatus = new MCI_STATUS_PARMS;
	}
	void removeP() {															//������ ����
		delete mciOpen;
		delete mciPlay;
		delete mciStatus;

	}
public:
	void Load() {																/*����&�ε� �Լ�*/
		createP();																//������ �Ҵ�
		if (getpathextension(Loaded_path) == "mp3") mciOpen->lpstrDeviceType = L"mpegvideo";	//����̽� Ÿ�� mp3
		else if (getpathextension(Loaded_path) == "wav") mciOpen->lpstrDeviceType = L"waveaudio";	//����̽� Ÿ�� mp3
		else mciOpen->lpstrDeviceType = L"mpegvideo";
		mciOpen->lpstrElementName = Loaded_path;									//���� ���� �ּ� ~~.mp3
		if (!mciSendCommand(Loaded_index, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE,
			(DWORD)(LPVOID)mciOpen)) {											//���� ���� -> mciOpen�� ����.
			Loaded_index = mciOpen->wDeviceID;									//����̽� �ε��� ����
			flag = STOPPED;														//�÷��� : �ε� �� ��������
		}
		else flag = UNLOADED;													//�÷��� : �ε� ����
	}
	void Close() {																/*����̽� ���� �Լ�*/
		removeP();																//������ ����
		flag = UNLOADED;														//�÷���
		mciSendCommand(Loaded_index, MCI_CLOSE, NULL, NULL);					//����̽� ����
	}
	void play() {																/*	�����Լ�	*/
		if (flag == STOPPED) {													//�����϶�		
			mciPlay->dwCallback = (DWORD)1;
			mciSendCommand(Loaded_index, MCI_PLAY, MCI_NOTIFY, (DWORD)mciPlay);	//���
			flag = PLAYING;														//�÷���
		}
		else if (flag == PAUSED) {												//�Ͻ������϶�
			mciSendCommand(Loaded_index, MCI_RESUME, 0, 0);						//�ٽ� ���
			flag = PLAYING;														//�÷���
		}
	}
	void stop() {																/*	�����Լ�	*/
		if (flag > STOPPED) {													//�Ͻ������̰ų� ������϶�
			mciSendCommand(Loaded_index, MCI_STOP, 0, NULL);					//����
			setPlayingtime(0);													//�ð踦 ó������
			flag = STOPPED;														//�÷���
		}
	}
	void pause() {																/* �Ͻ������Լ�	*/
		if (flag == PLAYING) {													//������϶�
			mciSendCommand(Loaded_index, MCI_PAUSE, 0, (DWORD)NULL);			//�Ͻ�����
			flag = PAUSED;														//�÷���
		}
	}
	void setVolume(DWORD volume) {												/*���� ���� �Լ�*/
		if (volume < 0) volume = 0;												//0���� ���� ��
		if (volume > 100) volume = 100;											//100���� Ŭ ��
		volume *= 10;															//*10 -> 0~1000����
		/*�����Ķ���� ����ü ����*/
		Volume_params.dwCallback = 0;											
		Volume_params.dwItem = MCI_DGV_SETAUDIO_VOLUME;
		Volume_params.dwValue = volume;											//���� ����
		Volume_params.dwOver = 0;
		Volume_params.lpstrAlgorithm = NULL;
		Volume_params.lpstrQuality = NULL;
		/*���� ����*/
		mciSendCommand(Loaded_index, MCI_SETAUDIO, MCI_DGV_SETAUDIO_VALUE | MCI_DGV_SETAUDIO_ITEM, (DWORD)(LPVOID)&Volume_params);
	}
	int getVolume() {
		MCI_STATUS_PARMS Volume_params;
		Volume_params.dwCallback = 0;
		Volume_params.dwItem = MCI_DGV_STATUS_VOLUME;							//�������� �� ����
		mciSendCommand(Loaded_index, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPVOID)&Volume_params);//��ɾ�
		return (int)(Volume_params.dwReturn / 10);								// ������ ����
	}
	void setPath(int index)						{						Loaded_path = list.at(index).path;	}	//��μ���
	bool isPlaying()							{ if (flag == PLAYING)	return true;	else return false;	}	//������ΰ�?
	bool isPaused()								{ if (flag == PAUSED)	return true;	else return false;	}	//�Ͻ��������ΰ�?
	void setFlag(int i)							{						flag = i;							}	//�÷��׼���
	int getFlag()								{						return flag;						}	//�÷��׸���
	CString getPath()							{						return Loaded_path;					}	//��� ����
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
	void vector_remove(int index)				{						list.erase(list.begin()+index);		}	//�ε��� ����
	music_list get_vector_item(int index)		{if(list.size()>0)return list.at(index); else return nullstructure; }//�ε����� ���� ������ ��ȯ
	int get_vector_size()						{						return list.size();					}	//���� ������ ����
	CString getpathextension(CString path) {
		int dot = path.ReverseFind('.');
		return path.Right(path.GetLength() - dot-1);
	}
};