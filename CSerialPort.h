// NcdSerial.h: interface for the CNcdSerial class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NCDSERIAL_H__CFB10258_463B_4F2F_A7B0_313F4CC4A581__INCLUDED_)
#define AFX_NCDSERIAL_H__CFB10258_463B_4F2F_A7B0_313F4CC4A581__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Afxmt.h>
#include "CQueue.h"


class CSerialPort : public CObject 
{
public:
	CSerialPort();
	virtual ~CSerialPort();

public:
	HANDLE				m_hCom;
	int					m_iPort;				// 포트의 번호를 구분하기 위한 것
	BOOL				m_bOpen;

public:
	bool				OpenPort(int Port, DWORD Baud);			// 1부터 시작
	void				ClosePort();
	void				WritePort(BYTE* pData, DWORD dwWrite);
	DWORD				ReadPort(BYTE* pBuffer, DWORD dwR);
	void				ResetPort();
	BOOL				ConfigPort(DWORD Baud, int ByteSize=8, int Parity=0, int StopBits=0);
	BYTE				GetModemStatus();


	static UINT			ReadThreadProc(LPVOID lParam);

// Overrides
public:
	void SendCommand(CString strCmd, CString strData);
	static BYTE			GetCheckSum8(BYTE* pByte, int iStart, int iLen);
	CWinThread*			m_pThread;	
	void				ReadStop();
	bool				ReadStart(int Port, DWORD Baud);
	CEvent				m_StopEvent;
	int					m_nMaxBlock;
	CQueue<byte*>		m_RecFrameDatas;



// Implementation
protected:
	DCB					m_DCB;
	CString				m_strPort;				// 1 -> COM1
	
private:
	
};

#endif // !defined(AFX_NCDSERIAL_H__CFB10258_463B_4F2F_A7B0_313F4CC4A581__INCLUDED_)
