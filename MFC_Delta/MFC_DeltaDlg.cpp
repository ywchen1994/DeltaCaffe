
// MFC_DeltaDlg.cpp : 實作檔
//

#include "stdafx.h"
#include "MFC_Delta.h"
#include "MFC_DeltaDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFC_DeltaDlg 對話方塊

KinectCapture CMFC_DeltaDlg::kinect;
bool CMFC_DeltaDlg::ObjectToWork = false;
int CMFC_DeltaDlg::ObjectCoordinate[4] = { 0 };
Mat CMFC_DeltaDlg::Img_Depth;
CMFC_DeltaDlg::CMFC_DeltaDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MFC_DELTA_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFC_DeltaDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ImageDepth, m_ImageDepth);
	DDX_Control(pDX, IDC_LIST_SCARA, m_LIST_SCARA);
}

BEGIN_MESSAGE_MAP(CMFC_DeltaDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BtnStart, &CMFC_DeltaDlg::OnBnClickedBtnstart)
	ON_BN_CLICKED(Btn_SCARAConnect, &CMFC_DeltaDlg::OnBnClickedScaraconnect)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CMFC_DeltaDlg 訊息處理常式

BOOL CMFC_DeltaDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 設定此對話方塊的圖示。當應用程式的主視窗不是對話方塊時，
	// 框架會自動從事此作業
	SetIcon(m_hIcon, TRUE);			// 設定大圖示
	SetIcon(m_hIcon, FALSE);		// 設定小圖示




	WORD dwStyle = m_LIST_SCARA.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;
	dwStyle |= LVS_EX_GRIDLINES;
	m_LIST_SCARA.SetExtendedStyle(dwStyle);
	m_LIST_SCARA.ModifyStyle(0, LVS_REPORT);
	m_LIST_SCARA.SetExtendedStyle(LVS_EX_GRIDLINES);
	//CRect rec;
	//m_LIST_SCARA.GetWindowRect(rec);
	m_LIST_SCARA.InsertColumn(0, _T("To Work"), LVCFMT_LEFT, 80);
	m_LIST_SCARA.InsertColumn(1, _T("Done"), LVCFMT_LEFT, 80);
	m_LIST_SCARA.InsertColumn(2, _T("X"), LVCFMT_LEFT, 100);
	m_LIST_SCARA.InsertColumn(3, _T("Y"), LVCFMT_LEFT, 100);
	m_LIST_SCARA.InsertColumn(4, _T("Z"), LVCFMT_LEFT, 100);
	m_LIST_SCARA.InsertColumn(5, _T("Theta"), LVCFMT_LEFT, 100);

	m_ImageDepth.SetWindowPos(NULL, 10, 10, 512, 424, SWP_SHOWWINDOW);
	mb = modbus_new_tcp("192.168.1.3", 502);
	ToWork = 0;
	return TRUE;  // 傳回 TRUE，除非您對控制項設定焦點
}

// 如果將最小化按鈕加入您的對話方塊，您需要下列的程式碼，
// 以便繪製圖示。對於使用文件/檢視模式的 MFC 應用程式，
// 框架會自動完成此作業。

void CMFC_DeltaDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 繪製的裝置內容

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 將圖示置中於用戶端矩形
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 描繪圖示
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 當使用者拖曳最小化視窗時，
// 系統呼叫這個功能取得游標顯示。
HCURSOR CMFC_DeltaDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMFC_DeltaDlg::OnBnClickedBtnstart()
{
	m_threadPara.m_case = 1;
	m_threadPara.hWnd = m_hWnd;
	m_lpThread = AfxBeginThread(&CMFC_DeltaDlg::MythreadFun, (LPVOID)&m_threadPara);


}
UINT CMFC_DeltaDlg::MythreadFun(LPVOID LParam)
{
	CthreadParam* para = (CthreadParam*)LParam;
	CMFC_DeltaDlg* lpview = (CMFC_DeltaDlg*)(para->m_lpPara);
	para->m_blthreading = TRUE;

	switch (para->m_case)
	{
	case 0:

		break;
	case 1:
		lpview->Thread_Image_Depth(LParam);
		break;
	default:
		break;
	}

	para->m_blthreading = FALSE;
	para->m_case = 0xFF;
	return 0;

}
void CMFC_DeltaDlg::Thread_Image_Depth(LPVOID lParam)
{
	CthreadParam * Thread_Info = (CthreadParam *)lParam;
	CMFC_DeltaDlg * hWnd = (CMFC_DeltaDlg *)CWnd::FromHandle((HWND)Thread_Info->hWnd);
	kinect.Open(0, 1, 0);
	Mutex mtx;
	mtx.lock();
	while (1)
	{

		Mat Img_Depth = kinect.DepthImage();

		if (!Img_Depth.empty())
		{
			hWnd->ShowImage(Img_Depth, hWnd->GetDlgItem(IDC_ImageDepth));

		}
	}
	mtx.unlock();
}
void CMFC_DeltaDlg::ShowImage(cv::Mat Image, CWnd* pWnd)
{
	//Windows中顯示圖像存在一個4位元組對齊的問題, 也就是每一行的位元組數必須是4的倍數.
	//而Mat的資料是連續存儲的.一般Mat的資料格式為BGR, 也就是一個圖元3個位元組, 假設我的圖片一行有5個圖元, 那一行就是15個位元組, 這不符合MFC的資料對齊方式,
	//如果我們直接把Mat的data加個資料頭再顯示出來就可能會出錯.
	//手動4位元組對齊, 就是計算每行的位元組是不是4的倍數, 不是的話, 在後面補0
	//但是我們把圖片轉成RGBA之後, 一個圖元就是4個位元組, 不管你一行幾個圖元, 一直都是對齊的.

	cv::Mat imgTmp;
	CRect rect;
	pWnd->GetClientRect(&rect);
	cv::resize(Image, imgTmp, cv::Size(rect.Width(), rect.Height()));

	switch (imgTmp.channels())
	{
	case 1:
		cv::cvtColor(imgTmp, imgTmp, CV_GRAY2BGRA);
		break;
	case 3:
		cv::cvtColor(imgTmp, imgTmp, CV_BGR2BGRA);
		break;
	default:
		break;
	}
	int pixelBytes = imgTmp.channels()*(imgTmp.depth() + 1);
	BITMAPINFO bitInfo;
	bitInfo.bmiHeader.biBitCount = 8 * pixelBytes;
	bitInfo.bmiHeader.biWidth = imgTmp.cols;
	bitInfo.bmiHeader.biHeight = -imgTmp.rows;
	bitInfo.bmiHeader.biPlanes = 1;
	bitInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitInfo.bmiHeader.biCompression = BI_RGB;
	bitInfo.bmiHeader.biClrImportant = 0;
	bitInfo.bmiHeader.biClrUsed = 0;
	bitInfo.bmiHeader.biSizeImage = 0;
	bitInfo.bmiHeader.biXPelsPerMeter = 0;
	bitInfo.bmiHeader.biYPelsPerMeter = 0;

	CDC *pDC = pWnd->GetDC();
	::StretchDIBits(
		pDC->GetSafeHdc(),
		0, 0, rect.Width(), rect.Height(),
		0, 0, rect.Width(), rect.Height(),
		imgTmp.data,
		&bitInfo,
		DIB_RGB_COLORS,
		SRCCOPY
	);
	ReleaseDC(pDC);
}

void CMFC_DeltaDlg::OnBnClickedScaraconnect()
{
	int res = modbus_connect(mb);
	m_LIST_SCARA.InsertItem(0, _T("0,0"));
	m_LIST_SCARA.SetItemText(0, 1, _T("0,1"));

	if (res < 0)
	{
		MessageBox(L"Connect Fail");
		SetDlgItemText(IDC_SCARA_Statement, _T("Connect Fail"));
	}
	else
	{
		SetDlgItemText(IDC_SCARA_Statement, _T("Connectted"));
		SetTimer(0, 10, NULL);
	}
}
void CMFC_DeltaDlg::Camera2SCARA(Mat DepthImage)
{
	Mat Img_Dpth, Img_canny;

	cv::threshold(DepthImage, DepthImage, 0, 255, CV_THRESH_OTSU);

	vector<vector<cv::Point>> contours; // Vector for storing contour
	vector<Vec4i>hierarchy;
	findContours(DepthImage, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	cv::Point center;
	for (int i = 0; i < contours.size(); i++) // Iterate through each contour
	{
		double Area = contourArea(contours[i], false);
		if (Area > 50)
		{
			Mat temp = Mat::zeros(DepthImage.size(), CV_8UC1);
			drawContours(temp, contours, i, Scalar(255), CV_FILLED, 8, hierarchy);
			cv::Moments m = cv::moments(temp, false);
			center = cv::Point(m.m10 / m.m00, m.m01 / m.m00);
			break;
		}
	}


	CvPoint3D32f CameraSpace;
	kinect.Depth2CameraSpace(center, &CameraSpace);

	//ToDo 轉手臂座標
//	ObjectCoordinate[0]=
//	ObjectCoordinate[1]=
//	ObjectCoordinate[2]=
}

void CMFC_DeltaDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此加入您的訊息處理常式程式碼和 (或) 呼叫預設值
	switch (nIDEvent)
	{
	case 0:
	{
		uint16_t tab_reg[1] = { 0 };
		CString str;
		if (ObjectToWork)//有東西夾
		{
			ToWork = 1;
			modbus_write_register(mb, ArmToWorkAddr, ToWork);
			if (modbus_read_registers(mb, ArmWorkDoneAddr, 1, tab_reg))//讀取是否成功
			{
				str.Format(_T("%d"), ToWork);
				m_LIST_SCARA.InsertItem(0, str);
				str.Format(_T("%d"), tab_reg);
				m_LIST_SCARA.SetItemText(0, 1, str);
				switch (tab_reg[0])
				{
				case 1:
				{

					Mat Img_Depth_temp;
					Img_Depth.copyTo(Img_Depth_temp);
					Camera2SCARA(Img_Depth_temp);

					str.Format(_T("%d"), ObjectCoordinate[0]);
					m_LIST_SCARA.SetItemText(0, 2, str);
					str.Format(_T("%d"), ObjectCoordinate[1]);
					m_LIST_SCARA.SetItemText(0, 3, str);
					str.Format(_T("%d"), ObjectCoordinate[2]);
					m_LIST_SCARA.SetItemText(0, 4, str);
					str.Format(_T("%d"), ObjectCoordinate[3]);
					m_LIST_SCARA.SetItemText(0, 5, str);

					modbus_write_register(mb, ArmXAddr, ObjectCoordinate[0]);
					modbus_write_register(mb, ArmYAddr, ObjectCoordinate[1]);
					modbus_write_register(mb, ArmZAddr, ObjectCoordinate[2]);
					modbus_write_register(mb, ArmThetaAddr, ObjectCoordinate[3]);
					modbus_write_register(mb, ArmSendDone, 1);
				}
				break;
				case 2:
				{
				}
					break;
				default:
					break;
				}

			}
		}
		else
		{
			ToWork = 0;
			modbus_write_register(mb, ArmToWorkAddr, ToWork);
		}
	}
	break;
	default:
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}
