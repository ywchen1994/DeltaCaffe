
// MFC_DeltaDlg.h : ���Y��
//

#pragma once
#include "afxwin.h"
#include"KinectCapture.h"
#include"modbus.h"
#include "afxcmn.h"
#define ArmToWorkAddr 0x1010
#define ArmWorkDoneAddr 0x1020
#define ArmXAddr 0x1001
#define ArmYAddr 0x1002
#define ArmZAddr 0x1003
#define ArmThetaAddr 0x1004
#define ArmSendDoneAddr 0x1005
using namespace cv;
using namespace cv::dnn;
struct CthreadParam
{
public:
	HWND hWnd;
	LPVOID m_lpPara;
	UINT   m_case;
	BOOL m_blthreading;
};
// CMFC_DeltaDlg ��ܤ��
class CMFC_DeltaDlg : public CDialogEx
{
// �غc
public:
	CMFC_DeltaDlg(CWnd* pParent = NULL);	// �зǫغc�禡

// ��ܤ�����
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFC_DELTA_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �䴩


// �{���X��@
protected:
	HICON m_hIcon;

	// ���ͪ��T�������禡
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	static VideoCapture cap;
	static Mat frame_WebCam;
	static Net _net;
	modbus_t *mb;
	static KinectCapture kinect;
	afx_msg void OnBnClickedBtnstart();
	static UINT MythreadFun(LPVOID LParam);
	void Thread_Image_Cam(LPVOID lParam);
	void Thread_Image_Depth(LPVOID lParam);
	void ShowImage(cv::Mat Image, CWnd * pWnd);
	static bool ObjectToWork;
	static int ObjectCoordinate[4];
	int ToWork;
	CStatic m_ImageDepth;
	CthreadParam m_threadPara;
	CWinThread*  m_lpThread;
	CListCtrl m_LIST_SCARA;
	afx_msg void OnBnClickedScaraconnect();
	void Camera2SCARA(Mat DepthImage);

	void calcCircles(const Mat & input, vector<Vec3f>& circles);

    int  Caffe(Mat & input, Net & net1, const vector<Vec3f>& circles);
	void OnTimer(UINT_PTR nIDEvent);
	static Mat Img_Depth;
	afx_msg void OnBnClickedTest();
	CStatic m_Image_WebCam;
};
