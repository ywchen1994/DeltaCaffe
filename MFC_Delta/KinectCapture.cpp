#include"stdafx.h"
#include "KinectCapture.h"



KinectCapture::KinectCapture()
{
	fColorOpened = false;
	fDepthOpened = false;
	fInfraredOpened = false;

	colorROI = cvRect(0, 0, 1920, 1080);
	depthROI = cvRect(0, 0, 512, 424);

	colorSize = cvSize(1920, 1080);
	depthSize = cvSize(512, 424);
}
KinectCapture::~KinectCapture()
{
}
void KinectCapture::Open(bool rgb, bool depth, bool Infrared)
{
	pSensor = nullptr;

	if (GetDefaultKinectSensor(&pSensor) != S_OK)	cerr<<"Get Sensor failed";
	if (pSensor->Open() != S_OK) cerr << "Can't open sensor";

	/**********************************************************************************************************/

	if (rgb)
	{
		IColorFrameSource* pColorFrameSource = nullptr;
		if (pSensor->get_ColorFrameSource(&pColorFrameSource) != S_OK)cerr << "Can't get color frame source";

		//Get frame description
		iWidth = 0;
		iHeight = 0;
		IFrameDescription* pFrameDescription = nullptr;
		if (pColorFrameSource->get_FrameDescription(&pFrameDescription) == S_OK)
		{
			pFrameDescription->get_Width(&iWidth);
			pFrameDescription->get_Height(&iHeight);
			uColorPointNum = iWidth*iHeight;//1920*1080個彩色畫素點

			pCSPoints = new CameraSpacePoint[uColorPointNum];
		}
		pFrameDescription->Release();
		pFrameDescription = nullptr;


		//get frame reader
		pColorFrameReader = nullptr;
		if (pColorFrameSource->OpenReader(&pColorFrameReader) != S_OK)cerr << "Can't get color frame reader";

		//release Frame source
		pColorFrameSource->Release();
		pColorFrameSource = nullptr;

		//Prepare OpenCV data
		mColorImg = cv::Mat(iHeight, iWidth, CV_8UC4);
		uColorBufferSize = iHeight * iWidth * 4 * sizeof(BYTE);

		//checkOpen(because it will delay few second to AcquireLatestFrame)
		bool checkOpen = false;
		while (checkOpen == false)
		{
			pColorFrame = nullptr;
			if (pColorFrameReader->AcquireLatestFrame(&pColorFrame) == S_OK)
			{
				if (pColorFrame->CopyConvertedFrameDataToArray(uColorBufferSize, mColorImg.data, ColorImageFormat_Bgra) == S_OK)
					checkOpen = true;
				else
				{
					cerr << "Data copy error";
					break;
				}
				pColorFrame->Release();
			}
		}
		fColorOpened = true;
	}

	/**********************************************************************************************************/

	if (depth)
	{

		IDepthFrameSource* pFrameSource = nullptr;
		if (pSensor->get_DepthFrameSource(&pFrameSource) != S_OK)cerr << "Can't get depth frame source";

		//Get frame description
		iDepthWidth = 0;
		iDepthHeight = 0;
		IFrameDescription* pFrameDescription = nullptr;
		if (pFrameSource->get_FrameDescription(&pFrameDescription) == S_OK)
		{
			pFrameDescription->get_Width(&iDepthWidth);
			pFrameDescription->get_Height(&iDepthHeight);
			uDepthPointNum = iDepthHeight*iDepthWidth;//512*424個深度畫素
			pDepthPoints = new UINT16[uDepthPointNum];//建立一個動態矩陣用來存16bits深度影像值
			pDSPoints = new CameraSpacePoint[uDepthPointNum];
		}


		pFrameDescription->Release();
		pFrameDescription = nullptr;

		//get some dpeth only meta
		UINT16 uDepthMin = 0;
		uDepthMax = 0;
		pFrameSource->get_DepthMinReliableDistance(&uDepthMin);
		pFrameSource->get_DepthMaxReliableDistance(&uDepthMax);

		//get frame reader
		pDepthFrameReader = nullptr;
		if (pFrameSource->OpenReader(&pDepthFrameReader) != S_OK) cerr << "Can't get color frame source";


		//perpare OpenCV

		mDepthImg = cv::Mat(iDepthHeight, iDepthWidth, CV_16UC1);
		mDepthImg8bit = cv::Mat(iDepthHeight, iDepthWidth, CV_8UC1);

		//release Frame source
		pFrameSource->Release();
		pFrameSource = nullptr;

		pCoordinateMapper = nullptr;/*初始化座標轉換指標*/
		if (pSensor->get_CoordinateMapper(&pCoordinateMapper) != S_OK)cerr << "get_CoordinateMapper failed";
		fDepthOpened = true;
	}

	/**********************************************************************************************************/

	if (Infrared)
	{
		IInfraredFrameSource* pFrameSource = nullptr;
		if (pSensor->get_InfraredFrameSource(&pFrameSource) != S_OK) cerr << "Can't get depth frame source";

		//Get frame description
		iInfraredWidth = 0;
		iInfraredHeight = 0;
		IFrameDescription* pFrameDescription = nullptr;
		if (pFrameSource->get_FrameDescription(&pFrameDescription) == S_OK)
		{
			pFrameDescription->get_Width(&iInfraredWidth);
			pFrameDescription->get_Height(&iInfraredHeight);
		}

		pFrameDescription->Release();
		pFrameDescription = nullptr;

		//get frame reader
		pInfraredFrameReader = nullptr;
		if (pFrameSource->OpenReader(&pInfraredFrameReader) != S_OK) cerr << "Can't get color frame reader";

		//release Frame source
		pFrameSource->Release();
		pFrameSource = nullptr;

		fInfraredOpened = true;
	}
}

void KinectCapture::Close()
{
	pColorFrameReader->Release();
	pColorFrameReader = nullptr;

	pDepthFrameReader->Release();
	pDepthFrameReader = nullptr;

	pInfraredFrameReader->Release();
	pInfraredFrameReader = nullptr;

	pSensor->Close();
	pSensor->Release();
	pSensor = nullptr;

	pCoordinateMapper->Release();/*釋放座標轉換指標*/
	pCoordinateMapper = nullptr;
	delete[]pCSPoints;
	delete[]pDSPoints;

}

void KinectCapture::setColorROISize(CvRect ROI, CvSize size)
{
	colorROI = ROI;
	colorSize = size;
}
void KinectCapture::setDepthROISize(CvRect ROI, CvSize size)
{
	depthROI = ROI;
	depthSize = size;
}

IplImage* KinectCapture::RGBAImage()
{
	if (fColorOpened == false) { cerr << "Need Open"; return NULL; }
	bool copyDone = false;
	IplImage* pImg = nullptr;
	pColorFrame = nullptr;

	if (pColorFrameReader->AcquireLatestFrame(&pColorFrame) == S_OK)
	{
		//Copy to OpenCV image
		if (pColorFrame->CopyConvertedFrameDataToArray(uColorBufferSize, mColorImg.data, ColorImageFormat_Bgra) == S_OK)
		{
			pImg = cvCloneImage(&(IplImage)mColorImg);
			copyDone = true;
		}
		else
			cerr << "Data copy error";

		pColorFrame->Release();
	}
	if (copyDone)
		return pImg;
	else
		return NULL;
}

IplImage * KinectCapture::DepthImage()
{
	if (fDepthOpened == false) { cerr << "Need Open"; return NULL; }
	bool fGet = false;
	IplImage* pImg = nullptr;

	if (pDepthFrameReader->AcquireLatestFrame(&pDepthFrame) == S_OK)
	{
		//copy the depth map to image

		pDepthFrame->CopyFrameDataToArray(iDepthWidth * iDepthHeight, reinterpret_cast<UINT16*>(mDepthImg.data));
		pDepthPoints = reinterpret_cast<UINT16*>(mDepthImg.data);

		//convert from 16bit to 8bit

		mDepthImg.convertTo(mDepthImg8bit, CV_8U, 255.0f / 825);
		pImg = cvCloneImage(&(IplImage)mDepthImg8bit);


		fGet = true;

		pDepthFrame->Release();
	}

	if (fGet)
		return pImg;
	else
		return NULL;
}

IplImage * KinectCapture::InfraredImage()
{
	if (fInfraredOpened == false) {
		cerr << "Need Open"; return NULL; }
	bool accessDone = false;
	IplImage* pImg = nullptr;
	IInfraredFrame* pFrame = nullptr;
	if (pInfraredFrameReader->AcquireLatestFrame(&pFrame) == S_OK)
	{
		UINT	uSize = 0;
		UINT16*	pBuffer = nullptr;
		if (pFrame->AccessUnderlyingBuffer(&uSize, &pBuffer) == S_OK)
		{
			cv::Mat mIRImg(iInfraredHeight, iInfraredWidth, CV_16UC1, pBuffer);
			pImg = cvCloneImage(&(IplImage)mIRImg);
			cvSetImageROI(pImg, depthROI);
			cvResize(pImg, cvCreateImage(depthSize, IPL_DEPTH_16U, 1), CV_INTER_AREA);
			accessDone = true;
		}
		else
			cerr << "Data access error";

		pFrame->Release();
	}

	if (accessDone)
		return pImg;
	else
		return NULL;
}
void KinectCapture::Color2CameraSpace(CvPoint RGBpoint, CvPoint3D32f *CameraSpace)
{

	if (pCoordinateMapper->MapColorFrameToCameraSpace(uDepthPointNum, pDepthPoints, uColorPointNum, pCSPoints) == S_OK)
	{
		int idx = RGBpoint.x + RGBpoint.y * iWidth;
		const CameraSpacePoint& rPt = pCSPoints[idx];
		*CameraSpace = { rPt.X ,rPt.Y,rPt.Z };
	}
}
void  KinectCapture::Depth2CameraSpace(CvPoint Depthpoint, CvPoint3D32f *CameraSpace)
{
	if (pCoordinateMapper->MapDepthFrameToCameraSpace(uDepthPointNum, pDepthPoints, uDepthPointNum, pDSPoints) == S_OK)
	{
		int depIdx = Depthpoint.x + Depthpoint.y * iDepthWidth;
		const CameraSpacePoint& rPt = pDSPoints[depIdx];
		*CameraSpace = { rPt.X ,rPt.Y,rPt.Z };
	}
}
void KinectCapture::Color2DepthSpace(CvPoint RGBpoint, CvPoint *Depthpoint)
{
	IDepthFrame* pDepthFrame = nullptr;
	UINT16*				pDepthPoints1 = new UINT16[uDepthPointNum];
	DepthSpacePoint*	pPointArray1 = new DepthSpacePoint[uColorPointNum];

	while (1)
	{
		if (pDepthFrameReader->AcquireLatestFrame(&pDepthFrame) == S_OK)
		{
			pDepthFrame->CopyFrameDataToArray(uDepthPointNum, pDepthPoints1);
			if (pCoordinateMapper->MapColorFrameToDepthSpace(uDepthPointNum, pDepthPoints1, uColorPointNum, pPointArray1) == S_OK)
			{
				const DepthSpacePoint& rPoint = pPointArray1[RGBpoint.y * mColorImg.cols + (mColorImg.cols - RGBpoint.x + 1)];
				*Depthpoint = { (int)rPoint.X,(int)rPoint.Y };
			}
			pDepthFrame->Release();
			delete[]pDepthPoints1;
			delete[]pPointArray1;
			break;
		}

	}
}
void KinectCapture::Depth2ColorSpace(CvPoint Depthpoint, CvPoint* RGBpoint)/*未完成*/
{

}