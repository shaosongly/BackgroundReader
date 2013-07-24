// BackgroundReader.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "BackgroundReader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Ψһ��Ӧ�ó������

CWinApp theApp;

using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// ��ʼ�� MFC ����ʧ��ʱ��ʾ����
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: ���Ĵ�������Է���������Ҫ
			_tprintf(_T("����: MFC ��ʼ��ʧ��\n"));
			nRetCode = 1;
		}
		else
		{
			// TODO: �ڴ˴�ΪӦ�ó������Ϊ��д���롣
			const char* filename="G:/pedestrian_detection/trainData/neg_samples.dat";
			CvSize sz=cvSize(64,128);
			CvMat img;

			if(CHANNEL==3)
				img=cvMat(sz.height, sz.width, CV_8UC3,cvAlloc( sizeof( uchar ) * sz.height * sz.width*3 ) );
			else
				img=cvMat(sz.height, sz.width, CV_8UC1,cvAlloc( sizeof( uchar ) * sz.height * sz.width ) );
			cvNamedWindow("main",CV_WINDOW_AUTOSIZE);
			icvInitBackgroundReaders(filename,sz);
			for(int i=0;i<100;i++)
			{
				icvGetBackgroundImage( cvbgdata,cvbgreader,&img );
				cvShowImage("main",&img);
				cvWaitKey(1000);
			}
		}
	}
	else
	{
		// TODO: ���Ĵ�������Է���������Ҫ
		_tprintf(_T("����: GetModuleHandle ʧ��\n"));
		nRetCode = 1;
	}

	return nRetCode;
}
