#include "stdafx.h"
#include "Shlobj.h"
#include "CommDlg.h"
#include <opencv2\opencv.hpp>
#include <cv.h>
#include <iostream>
#include <string>
#include <fstream>
#include <cstring>
#include "dirent.h"
#include <cmath>
#include <windows.h>
using namespace std;
using namespace cv;

string path;	//the folder path...
DIR *dir;
struct dirent *entry;


char* ConvertLPWSTRToLPSTR(LPWSTR lpwszStrIn)
{
	LPSTR pszOut = NULL;
	if (lpwszStrIn != NULL)
	{
		int nInputStrLen = wcslen(lpwszStrIn);

		// Double NULL Termination  
		int nOutputStrLen = WideCharToMultiByte(CP_ACP, 0, lpwszStrIn, nInputStrLen, NULL, 0, 0, 0) + 2;
		pszOut = new char[nOutputStrLen];

		if (pszOut)
		{
			memset(pszOut, 0x00, nOutputStrLen);
			WideCharToMultiByte(CP_ACP, 0, lpwszStrIn, nInputStrLen, pszOut, nOutputStrLen, 0, 0);
		}
	}
	return pszOut;
}

void TcharToChar(const TCHAR * tchar, char * _char)
{
	int iLength; 
	iLength = WideCharToMultiByte(CP_ACP, 0, tchar, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, tchar, -1, _char, iLength, NULL, NULL);
}

void VideoProcessing()	//Process the video and save it after processing
{

	string videoName = entry->d_name;
	string fileName=path + "\\" + videoName;
	char* tempChar = new char[fileName.size() + 1];
	strcpy(tempChar, fileName.c_str());
	cout << "process the video " << tempChar << " now" << endl;
	VideoCapture capture(fileName);
	int framesNum = capture.get(CV_CAP_PROP_FRAME_COUNT);
	int FPS = capture.get(CV_CAP_PROP_FPS);
	Mat* originImages = new Mat[framesNum];
	Mat* grayImages = new Mat[framesNum];
	Mat* backgroundImages = new Mat[framesNum];								//background frames
	Mat* foregroundImages = new Mat[framesNum];								//foreground frames
	vector<Mat>* channels=new vector<Mat>[framesNum];						//R channel,G channel,B channel
	Mat* darkImages = new Mat[framesNum];
	cout << "the video has " << framesNum << " frames" << endl;
	for (int i = 0; i < framesNum; i++)
	{
		if (capture.read(originImages[i]) != NULL)
		{
			cvtColor(originImages[i], grayImages[i], COLOR_RGB2GRAY);		//transfer the rgb images into gray images
			backgroundImages[i] = grayImages[i].clone();
			foregroundImages[i] = grayImages[i].clone();
			darkImages[i] = grayImages[i].clone();
		}
		else
		{
			cout << "error"<<endl;
		}
	}
	int height = grayImages[0].rows;
	int width = grayImages[0].cols;
	for (int k = 1; k < framesNum; k++)						//background images extraction...
	{
		for (int i = 0; i < height; i++)
		{
			uchar* goriData = grayImages[0].ptr<uchar>(i);
			uchar* bfData= backgroundImages[k-1].ptr<uchar>(i);
			uchar* gfData = grayImages[k-1].ptr<uchar>(i);
			uchar* bData = backgroundImages[k].ptr<uchar>(i);
			uchar* gData = grayImages[k].ptr<uchar>(i);
			for (int j = 0; j < width; j++)
			{
				if (abs(gData[j] - gfData[j])>0)
				{
					bData[j] = 0.01*bfData[j] + 0.01*gData[j] + 0.98*goriData[j];
				}
				else
				{
					bData[j] = bfData[j - 1];
				}
			}
		}
	}
	for (int k = 0; k < framesNum; k++)
	{
		for (int i = 0; i < height; i++)
		{
			uchar* foreData = foregroundImages[k].ptr<uchar>(i);
			for (int j = 0; j < width; j++)
			{
				foreData[j] = 0.0f;
			}
		}
	}

	for (int k = 1; k < framesNum; k++)						//compute foreground images
	{
		for (int i = 0; i < height; i++)
		{
			uchar* gData = grayImages[k].ptr<uchar>(i);
			uchar* bData = backgroundImages[k].ptr<uchar>(i);
			uchar* fData = foregroundImages[k].ptr<uchar>(i);
			for (int j = 0; j < width; j++)
			{
				if (fabs(gData[j] - bData[j]) > 30.6)
				{
					fData[j] = 255;
				}
			}
		}
	}
	for (int k = 0; k < framesNum;k++)														//compute darkchannel images...
	{
		split(originImages[k], channels[k]);
		for (int i = 0; i < height; i++)
		{
			uchar* rData = channels[k][0].ptr<uchar>(i);
			uchar* gData = channels[k][1].ptr<uchar>(i);
			uchar* bData = channels[k][2].ptr<uchar>(i);
			uchar* darkData = darkImages[k].ptr<uchar>(i);
			uchar* fData = foregroundImages[k].ptr<uchar>(i);
			for (int j = 0; j < width; j++)
			{
				darkData[j] = min(rData[j], gData[j]);
				darkData[j] = min(darkData[j], bData[j]);
				if (darkData[j] > 120)
				{
					if (fData[j] == 255)
					{
						darkData[j] = 255;
					}
					else
					{
						darkData[j] = 0;
					}
				}
				else
				{
					darkData[j] = 0;
				}
			}
		}

	}
	Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
	for (int k = 0; k < framesNum; k++)
	{
		morphologyEx(darkImages[k], darkImages[k], MORPH_OPEN, element);
	}
	for (int k = 0; k < framesNum; k++)
	{
		for (int i = 0; i < height; i++)
		{
			uchar* rgbData = originImages[k].ptr<uchar>(i);
			uchar* dData = darkImages[k].ptr<uchar>(i);
			int countColor = 0;
			int countGray = 0;
			for (int j = 0; j < width*3; j++)
			{
				if (countColor == 0)
				{
					rgbData[j] = rgbData[j] * (dData[countGray] / 255);
					countColor++;
				}
				else if (countColor == 1)
				{
					rgbData[j]=rgbData[j]* (dData[countGray] / 255);
					countColor++;
				}
				else if (countColor == 2)
				{
					rgbData[j] = rgbData[j] * (dData[countGray] / 255);
					countColor = 0;
					countGray++;
				}
				/*if (j < width)
				{
					rgbData[j] = rgbData[j] * (dData[j] / 255);
				}
				else if (width <= j&&j < 2 * width)
				{
					rgbData[j] = rgbData[j] * (dData[j - width] / 255);
				}
				else if (2 * width <= j&&j < 3 * width)
				{
					rgbData[j] = rgbData[j] * (dData[j - 2*width] / 255);
				}*/
			}
		}
	}
	string tempPath = path+ "\\VideosWithSmokeMotionMasks";
	size_t size = path.length();
	wchar_t *buffer = new wchar_t[size + 1];
	MultiByteToWideChar(CP_ACP, 0, path.c_str(), size, buffer, size * sizeof(wchar_t));
	buffer[size] = 0; 
	CreateDirectory(buffer,NULL);
	CvVideoWriter  *frame_writer;
	videoName[videoName.length() - 3]='a';
	videoName[videoName.length() - 2]='v';
	videoName[videoName.length() - 1]='i';
	videoName = tempPath + "\\" + videoName;
	frame_writer = cvCreateVideoWriter(videoName.c_str(), CV_FOURCC('M', 'J', 'P', 'G'), FPS, cvSize(width, height), 1);
	for (int k=0;k<framesNum;k++)
	{
		cvWriteFrame(frame_writer, &IplImage(originImages[k]));
	}
	cvReleaseVideoWriter(&frame_writer);
	delete []tempChar;
	delete []originImages;
	delete []grayImages;
	delete []backgroundImages;
	delete []darkImages;
	delete []channels;
}
void ChooseFolder()
{
	while (true)
	{
		TCHAR szBuffer[MAX_PATH] = { 0 };
		BROWSEINFO bi;
		ZeroMemory(&bi, sizeof(BROWSEINFO));
		bi.hwndOwner = NULL;
		bi.pszDisplayName = szBuffer;      
		bi.lpszTitle = _T("Choose your videos folder:");
		bi.ulFlags = BIF_RETURNFSANCESTORS;
		LPITEMIDLIST idl = SHBrowseForFolder(&bi);
		if (NULL == idl)
		{
			continue;
		}
		SHGetPathFromIDList(idl, szBuffer);
		char sz[MAX_PATH];
		TcharToChar(szBuffer, sz);
		path = sz;
		if ((dir = opendir(sz)) == NULL)
			cout << "Error opening" << endl;
		else
		{
			break;																	//if choose the folder able to open, break
		}
	}
}
void main()
{
	ChooseFolder();
	entry = readdir(dir);
	entry = readdir(dir);
	while ((entry = readdir(dir)) != NULL)
	{
		char* videoName = entry->d_name;
		int nameLength = strlen(videoName);
		char first = videoName[nameLength - 3];
		char second = videoName[nameLength - 2];
		char third = videoName[nameLength - 1];
		if ((first == 'm'&&second == 'p'&&third == '4') || (first == 'a'&&second == 'v'&&third == 'i'))  //only support two kinds of formats here
		{

		}
		else
		{
			continue;
		}
		VideoProcessing();
	}
	cout << path << endl;
}