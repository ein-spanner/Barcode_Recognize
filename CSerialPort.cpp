// CESerial.cpp: implementation of the CCESerial class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "pch.h"
#include "CSerialPort.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define		STX				0x02
#define		ETX				0x03
#define		FRAMESIZE		0xFF



CSerialPort::CSerialPort()
{
	m_hCom = NULL;
	m_iPort = 0;
	m_bOpen = FALSE;
	m_nMaxBlock = 500;
	m_strPort = _T("");
	m_RecFrameDatas.init(1024);
}

CSerialPort::~CSerialPort()
{
	ClosePort();
}

/////////////////////////////////////////////////////////////////////////////// CSerialPort member functions
// �����Ʈ ����
bool CSerialPort::OpenPort(int Port, DWORD Baud)
{
	COMMTIMEOUTS	timeouts;

	// ���ι�ȣ ���
	m_iPort = Port;

	if(Port > 9)
	{
		m_strPort.Format(_T("\\\\?\\COM%d:"), Port);
	}
	else
	{
		m_strPort.Format(_T("COM%d:"), Port);		
	}
	
	// ��Ʈ����
	m_hCom = CreateFile(m_strPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    
	if(m_hCom == INVALID_HANDLE_VALUE)
	{
		MessageBox(0, L"Can't Open", L"���", MB_OKCANCEL);

		return FALSE;
	}
	// EV_RXCHAR event ����
	SetCommMask( m_hCom, EV_RXCHAR);	
	// InQueue, OutQueue ũ�� ����.
	SetupComm( m_hCom, 1024, 1024);	
	// ResetPort
	ResetPort();

	// timeout ����.
	timeouts.ReadIntervalTimeout = 500;  //0xFFFFFFFF;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.ReadTotalTimeoutConstant = 100;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 0;  //10000;
	SetCommTimeouts( m_hCom, &timeouts);

	// dcb ����
	if(!ConfigPort(Baud))
	{
		//AfxMessageBox(_T("Can't Config"));		
		return FALSE;
	}
	m_bOpen = TRUE;					// ������� ���� ���������� ��Ʈ�� ����
	
	return TRUE;
}

// �����Ʈ �ݱ�
void CSerialPort::ClosePort()
{
	m_bOpen = FALSE;

	SetCommMask(m_hCom, 0);
	ResetPort();
	
	m_iPort = 0;
	CloseHandle(m_hCom);
}

// �����͸� �۽��ϱ�
void CSerialPort::WritePort(BYTE* pData, DWORD dwWrite)
{
	DWORD	dwWritten, dwError, dwErrorFlags;
	COMSTAT comstat;

	if (! WriteFile( m_hCom, pData, dwWrite, &dwWritten, NULL))
	{
		//AfxMessageBox(_T("Can't Write"));
		dwError = GetLastError();
		dwWritten = 0;
		ClearCommError( m_hCom, &dwErrorFlags, &comstat);
	}
	
}

// ���ŵ� �����͸� �б�
DWORD CSerialPort::ReadPort(BYTE* pBuffer, DWORD dwR)
{
	DWORD	dwRead, dwReadCnt;//dwError, dwErrorFlags, ;

	dwRead = dwR;//min(dwR,comstat.cbInQue);
	
	ReadFile(m_hCom, pBuffer, dwRead, &dwReadCnt, NULL);		
	if (dwReadCnt == 0)
	{
		dwRead = 0;
	}

	return dwReadCnt;
}

void CSerialPort::ResetPort()
{
	// ��Ʈ ����.
	PurgeComm( m_hCom,	PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR);
}

BOOL CSerialPort::ConfigPort(DWORD Baud, int ByteSize, int Parity, int StopBits)
{
	// dcb ����
	m_DCB.DCBlength = sizeof(DCB);
	GetCommState( m_hCom, &m_DCB);	// ���� ���� ����.

	m_DCB.BaudRate = Baud;
	m_DCB.ByteSize = ByteSize;
	m_DCB.Parity = NOPARITY;
	m_DCB.StopBits = ONESTOPBIT;

	
	m_DCB.fOutxDsrFlow = FALSE;
	m_DCB.fDtrControl = DTR_CONTROL_DISABLE;
	m_DCB.fOutxCtsFlow = FALSE;
	
	
	m_DCB.fRtsControl = RTS_CONTROL_TOGGLE; 
	m_DCB.fRtsControl = RTS_CONTROL_DISABLE;

	m_DCB.fNull = FALSE;
	m_DCB.fInX = m_DCB.fOutX = FALSE;		// Xon, Xoff ������
//	m_DCB.XonChar = ASCII_XON;
//	m_DCB.XoffChar = ASCII_XOFF;
	m_DCB.XonLim = 0;//100;
	m_DCB.XoffLim = 0;//100;
	m_DCB.fBinary = TRUE;
	m_DCB.fParity = FALSE;

	if (! SetCommState( m_hCom, &m_DCB))	
		return FALSE;
	return TRUE;
}

BYTE CSerialPort::GetModemStatus()
{
	DWORD wd;
	BYTE rv = 0;

	GetCommModemStatus(m_hCom, &wd);
	if(wd & MS_CTS_ON) rv |= 0x01;
	if(wd & MS_DSR_ON) rv |= 0x02;
	if(wd & MS_RING_ON) rv |= 0x04;
	if(wd & MS_RLSD_ON) rv |= 0x08;

	return rv;
}

UINT CSerialPort::ReadThreadProc(LPVOID lParam)
{
	CSerialPort* pSerial = (CSerialPort*)lParam;

	DWORD		dwEvent;
	BYTE		lnData[1000];	//pSerial->m_nMaxBlock + 1]; // = new BYTE[pSerial->m_nMaxBlock + 1];
	DWORD		dwRead;			// ���� ����Ʈ��.
	BYTE*		pEnQueueData;

	// ��Ʈ�� �����ϴ� ����.
	while (pSerial->m_bOpen)
	{
		dwEvent = 0;

		// ������ �����ġ�� �̺�Ʈ�� �߻��ϱ⸦ ��ٸ�
		// WaitCommEvent(��ġ�ڵ�, �����̺�Ʈ ���� ������, ��ø����ü ������)
		WaitCommEvent(pSerial->m_hCom, &dwEvent, NULL);

		// �ѹ��ڰ� ���ŵǾ��ٸ�
		if ((dwEvent & EV_RXCHAR) == EV_RXCHAR)
		{
			// �޸� �ʱ�ȭ. InData��, ũ�Ⱑ pSerial->m_nMaxBlock�� �迭�� ���� 0���� �ʱ�ȭ
			memset(lnData, 0, pSerial->m_nMaxBlock);

			if(dwRead = pSerial->ReadPort(lnData, pSerial->m_nMaxBlock))
			{
				// �̺к��� ���� �Ͽ� ���α׷��� ������ ���������� ���� �Ͻñ� �ٶ��ϴ�. 
				// �Ʒ� �ڵ带 �ʿ��Ͻ� �Ľ� �ڵ�� ���� �Ͻñ� �ٶ��ϴ�.
				// �Ʒ� �ڵ�� STX DATA ETX �� ������ ó���Ǿ����ϴ�.
				//****************************[ Parsing - Start Code ] **********************************//
				if(dwRead <= FRAMESIZE)
				{
					for(DWORD i = 0; i < dwRead; i++)
					{
						// STX�� �ԷµǸ�
						if(lnData[i] == STX)
						{				
							// InData[i+(dwRead-1)]��° �����Ͱ� ETX���
							if(lnData[i + (dwRead-1)] == ETX)
							{															
								pEnQueueData = new BYTE[dwRead];
								memset(pEnQueueData, 0, dwRead);
																		
								for(DWORD j = 1; j < (dwRead - 1); j++)
								{
									pEnQueueData[j-1] = lnData[i+j];
								}

								pSerial->m_RecFrameDatas.insert(pEnQueueData);
								

								i += (dwRead - 1);

							} // if(lnData[i + (FRAMESIZE-1)] == ETX)
						} // if(lnData[i] == STX)
						else if((i + dwRead) > dwRead)
						{
							break;
						}
					} // for(int i = 0; i < dwRead; i++)
				} // if(dwRead >= FRAMESIZE)
				//****************************[ Parsing - End Code ] **********************************//
			
			
			} // if(dwRead = pSerial->ReadPort(lnData, pSerial->m_nMaxBlock))
						
		} // if ((dwEvent & EV_RXCHAR) == EV_RXCHAR)

		
	} // while
	
	pSerial->m_StopEvent.SetEvent();

	//delete[] lnData;
	return TRUE;
}

bool CSerialPort::ReadStart(int Port, DWORD Baud)
{
	OpenPort(Port, Baud);
	
	// ���Ž����� ����(���� üũ ����)
	m_pThread = (CWinThread*)AfxBeginThread((AFX_THREADPROC)ReadThreadProc, (LPVOID)this);
	
	if(m_pThread == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	return true;
}

void CSerialPort::ReadStop()
{
	ClosePort();

	Sleep(500);

	::WaitForSingleObject(m_StopEvent, INFINITE);
}

BYTE CSerialPort::GetCheckSum8(BYTE *pByte, int iStart, int iLen)
{
	BYTE checkSum = 0;
          
    for (int i = iStart; i < iLen; i++)
    {
		checkSum += pByte[i];
    }
    
    return checkSum;
}

void CSerialPort::SendCommand(CString strCmd, CString strData)
{
	BYTE	sendData[FRAMESIZE];
	BYTE	cmd[4];
	BYTE	data[10];
	int		i;

	memset(sendData, 0x20, FRAMESIZE);
    
	wcstombs((char*)cmd, strCmd, strCmd.GetLength());
	wcstombs((char*)data, strData, strData.GetLength());

	sendData[0] = STX;
	
	for(i = 1; i < 5; i++)
	{
		sendData[i] = cmd[i-1];
	}

	for(i = 0; i < strData.GetLength(); i++)
	{
		sendData[i + 5] = data[i];
	}

	sendData[FRAMESIZE - 2] = GetCheckSum8(sendData, 0, FRAMESIZE - 2);

	sendData[FRAMESIZE - 1] = ETX;

	WritePort(sendData, FRAMESIZE);
}
