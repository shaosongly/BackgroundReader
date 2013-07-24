// BackgroundReader.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "BackgroundReader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象

CWinApp theApp;

using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// 初始化 MFC 并在失败时显示错误
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: 更改错误代码以符合您的需要
			_tprintf(_T("错误: MFC 初始化失败\n"));
			nRetCode = 1;
		}
		else
		{
			// TODO: 在此处为应用程序的行为编写代码。
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
		// TODO: 更改错误代码以符合您的需要
		_tprintf(_T("错误: GetModuleHandle 失败\n"));
		nRetCode = 1;
	}

	return nRetCode;
}
