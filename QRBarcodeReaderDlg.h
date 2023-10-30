
// QRBarcodeReaderDlg.h: 헤더 파일
//

#pragma once


// CQRBarcodeReaderDlg 대화 상자
class CQRBarcodeReaderDlg : public CDialogEx
{
// 생성입니다.
public:
	CQRBarcodeReaderDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_QRBARCODEREADER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	BITMAPINFO* m_pBitmapInfo;
	Mat m_matImage;
	VideoCapture* capture;
	CSerialPort m_ser;

	int CreateBitmapInfo(int w, int h, int bpp);
	int DrawImage();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	int StartCamera();
	CEdit mEdt1;
	afx_msg void OnBnClickedBtnUrl();
	afx_msg void OnBnClickedButtonSend();
};
