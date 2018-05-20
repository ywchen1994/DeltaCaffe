#pragma once
#include<stdio.h>
#include<iostream>
#include "opencv2\opencv.hpp"
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/utils/trace.hpp>
#include"Kinect.h"
using namespace std;
class KinectCapture
{
public:
	KinectCapture();
	~KinectCapture();



	//Open(RGBImage, DepthImage, InfraredImage)
	void Open(bool rgb, bool depth, bool Infrared);

	void Close();
	IKinectSensor* pSensor;

	void setColorROISize(CvRect ROI, CvSize size);
	void setDepthROISize(CvRect ROI, CvSize size);

	IplImage* RGBAImage();
	bool fColorOpened;
	IColorFrameReader* pColorFrameReader;
	IColorFrame* pColorFrame;
	cv::Mat mColorImg;
	UINT uColorBufferSize;
	CvRect colorROI;
	CvSize colorSize;


	IplImage* DepthImage();


	bool fDepthOpened;
	IDepthFrameReader* pDepthFrameReader;
	int iDepthWidth, iDepthHeight;
	UINT16 uDepthMax;
	cv::Mat mDepthImg;
	cv::Mat mDepthImg8bit;
	CvRect depthROI;
	CvSize depthSize;


	IplImage* InfraredImage();
	bool fInfraredOpened;
	IInfraredFrameReader* pInfraredFrameReader;
	int iInfraredWidth, iInfraredHeight;
	IDepthFrame* pDepthFrame;


	void Color2DepthSpace(CvPoint RGBpoint, CvPoint * Depthpoint);
	void Color2CameraSpace(CvPoint RGBpoint, CvPoint3D32f * CameraSpace);
	void Depth2CameraSpace(CvPoint Depthpoint, CvPoint3D32f * CameraSpace);
	void Depth2ColorSpace(CvPoint Depthpoint, CvPoint * RGBpoint);
	ICoordinateMapper* pCoordinateMapper;
	UINT uDepthPointNum;
	UINT uColorPointNum;
	UINT16*				pDepthPoints = new UINT16[uDepthPointNum];
	DepthSpacePoint*	pPointArray = new DepthSpacePoint[uColorPointNum];
	CameraSpacePoint* pCSPoints = nullptr;//用來儲存彩色影像到攝影機座標的指標
	CameraSpacePoint* pDSPoints = nullptr;//用來儲存深度影像到攝影機座標的指標
	int iHeight;
	int iWidth;
};

