
// QRBarcodeReaderDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "QRBarcodeReader.h"
#include "QRBarcodeReaderDlg.h"
#include "afxdialogex.h"
#include "CSerialPort.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CQRBarcodeReaderDlg 대화 상자

CQRBarcodeReaderDlg::CQRBarcodeReaderDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_QRBARCODEREADER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CQRBarcodeReaderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, mEdt1);
}

BEGIN_MESSAGE_MAP(CQRBarcodeReaderDlg, CDialogEx)//이게 있어야 실행됨 
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(ID_BTN_URL, &CQRBarcodeReaderDlg::OnBnClickedBtnUrl)//함수(ID, 이름)
	ON_BN_CLICKED(IDC_BUTTON_SEND, &CQRBarcodeReaderDlg::OnBnClickedButtonSend)//시리얼 통신 
END_MESSAGE_MAP()


// CQRBarcodeReaderDlg 메시지 처리기

BOOL CQRBarcodeReaderDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다.  
	// 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는 프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	
	m_ser.OpenPort(4, 9600); // Port 번호와 Baud Rate은 환경에 맞게 변경 가능

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면 아래 코드가 필요합니다.  
//  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는 프레임워크에서 이 작업을 자동으로 수행합니다.

void CQRBarcodeReaderDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		//m_matImage = imread("cat.bmp", IMREAD_UNCHANGED);
		//CreateBitmapInfo(m_matImage.cols, m_matImage.rows, m_matImage.channels() * 8);
		//DrawImage();
		StartCamera();
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CQRBarcodeReaderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



int CQRBarcodeReaderDlg::CreateBitmapInfo(int w, int h, int bpp)
{
	// TODO: 여기에 구현 코드 추가.
	if (m_pBitmapInfo != NULL)
	{
		delete[] m_pBitmapInfo;
		m_pBitmapInfo = NULL;
	}

	if (bpp == 8)
		m_pBitmapInfo = (BITMAPINFO*)new BYTE[sizeof(BITMAPINFO) + 255 * sizeof(RGBQUAD)];
	else // 24 or 32bit
		m_pBitmapInfo = (BITMAPINFO*)new BYTE[sizeof(BITMAPINFO)];

	m_pBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_pBitmapInfo->bmiHeader.biPlanes = 1;
	m_pBitmapInfo->bmiHeader.biBitCount = bpp;
	m_pBitmapInfo->bmiHeader.biCompression = BI_RGB;
	m_pBitmapInfo->bmiHeader.biSizeImage = 0;
	m_pBitmapInfo->bmiHeader.biXPelsPerMeter = 0;
	m_pBitmapInfo->bmiHeader.biYPelsPerMeter = 0;
	m_pBitmapInfo->bmiHeader.biClrUsed = 0;
	m_pBitmapInfo->bmiHeader.biClrImportant = 0;

	if (bpp == 8)
	{
		for (int i = 0; i < 256; i++)
		{
			m_pBitmapInfo->bmiColors[i].rgbBlue = (BYTE)i;
			m_pBitmapInfo->bmiColors[i].rgbGreen = (BYTE)i;
			m_pBitmapInfo->bmiColors[i].rgbRed = (BYTE)i;
			m_pBitmapInfo->bmiColors[i].rgbReserved = 0;
		}
	}

	m_pBitmapInfo->bmiHeader.biWidth = w;
	m_pBitmapInfo->bmiHeader.biHeight = -h;
	return 0;
}


int CQRBarcodeReaderDlg::DrawImage()
{
	// TODO: 여기에 구현 코드 추가.
	CClientDC dc(GetDlgItem(IDC_PICTURE));
	CRect rect;
	
	GetDlgItem(IDC_PICTURE)->GetClientRect(&rect);

	SetStretchBltMode(dc.GetSafeHdc(), COLORONCOLOR);
	StretchDIBits(dc.GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), 0, 0, 
		m_matImage.cols, m_matImage.rows, m_matImage.data, m_pBitmapInfo, DIB_RGB_COLORS, SRCCOPY);

	// create a reader _ 여기서부터 zbar lib 불러오기 
	ImageScanner scanner;

	// configure the reader
	//scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);

	Mat grey;//선언
	CString a;//출력, 선언
	cvtColor(m_matImage, grey, COLOR_BGR2GRAY);//이미지를 grey로 변환
	uchar* raw = (uchar*)grey.data;//raw데이터로 만들어서
	int width = m_matImage.cols;
	int height = m_matImage.rows;

	// wrap image data
	Image image(width, height, "Y800", raw, width * height);

	// scan the image for barcodes
	int n = scanner.scan(image);//스캐닝
	dc.SetTextColor(RGB(255, 25, 2));
	a.Format(_T("No Code"));
	// extract results
	for (Image::SymbolIterator symbol = image.symbol_begin();
		symbol != image.symbol_end();
		++symbol)
	{
		dc.SetTextColor(RGB(2, 25, 255));
		string str = symbol->get_data();
		mEdt1.SetWindowTextW(CString(str.c_str()));
		a.Format(_T("OK: %s"), CString(str.c_str()));
	}

	// clean up
	image.set_data(NULL, 0);
	dc.DrawText(a, -1, &rect, DT_LEFT | DT_WORDBREAK);

	return 0;
}


void CQRBarcodeReaderDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (!capture->isOpened())
	{
		return;
	}

	if (nIDEvent == 1000)
	{
		if (!capture->read(m_matImage))
		{
			return;
		}

		CreateBitmapInfo(m_matImage.cols, m_matImage.rows, m_matImage.channels() * 8);
		DrawImage();
	}

	CDialogEx::OnTimer(nIDEvent);
}


int CQRBarcodeReaderDlg::StartCamera()
{
	// TODO: 여기에 구현 코드 추가.
	capture = new VideoCapture(0);

	if (!capture->isOpened())
	{
		printf("캠을 열 수 없습니다. \n");
		return FALSE;
	}

	SetTimer(1000, 100, NULL);
	return 0;
}


void CQRBarcodeReaderDlg::OnBnClickedBtnUrl()
{
	CString edtStr;
	mEdt1.GetWindowTextW(edtStr);
	ShellExecute(NULL, TEXT("open"), edtStr, NULL, NULL, SW_SHOW);
}


void CQRBarcodeReaderDlg::OnBnClickedButtonSend()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString cmd, data;

	cmd = "LCD_"; // 명령어 4자리
	data = "Connected"; // 데이터 임의로 가능

	m_ser.SendCommand(cmd, data);
}


/*
bool onSql(string str) {
	// <1> 데이터베이스 연결 ---> 교량 건설
	string connStr = "Server=192.168.22.23;Uid=root;Pwd=root;Database=board;Charset=UTF-8";
	MySqlConnection conn = new MySqlConnection(connStr);
	conn.Open();

	// <2> 짐(SQL)을 실어 나를 트럭을 준비
	MySqlCommand cmd = new MySqlCommand("", conn);

	// <3-1> 직원 테이블에 직원 세 명 추가(INSERT 쿼리문)
	String str = "INSERT INTO board(title, content, writer) " + "VALUES ('100', '1', 'Jack');";

	return true;
}
*/
