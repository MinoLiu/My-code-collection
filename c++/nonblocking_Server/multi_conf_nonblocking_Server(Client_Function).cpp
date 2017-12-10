// multi_conf_nonblocking_Server(Client_Function).cpp  伺服器程式(定義客戶端函式)
/* 伺服器程式共有三個:
   multi_conf_nonblocking_Server.cpp
   multi_conf_nonblocking_Server(Client_Function).cpp
   multi_conf_nonblocking_Server(Client_Class).h

   配合的客戶端程式為: multi_conf_nonblocking_Client.cpp
*/
#include <stdio.h>
#include "multi_conf_nonblocking_Server(Client_Class).h"

/*
 * 構造函數
 */
CClient::CClient(const SOCKET sClient, const sockaddr_in &addrClient, int seq_, CRITICAL_SECTION& cs_ctlist)
{
	//初始化變數
	m_hThreadRecv = NULL;
	m_hThreadSend = NULL;
	m_socket = sClient;
	m_addr = addrClient;
	m_bConning = FALSE;	
	m_bExit = FALSE;
	memset(m_data.buf, 0, MAX_NUM_BUF);
	seq = seq_;
	m_cs_clientlist = cs_ctlist;

	//創建事件
	m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);//手動設置信號狀態，初始化為無信號狀態

	//初始化臨界區
	InitializeCriticalSection(&m_cs);
}


/*
 * 析構函數
 */
CClient::~CClient()
{
	closesocket(m_socket);			//關閉通訊端
	m_socket = INVALID_SOCKET;		//通訊端無效
	DeleteCriticalSection(&m_cs);	//釋放臨界區對象	
	CloseHandle(m_hEvent);			//釋放事件物件
}


/*
 * 取得主程式nonblocking_Server.cpp  的全域變數 clientlist 的位址
 */
BOOL CClient::GetClientList(CLIENTLIST& ctlist)
{
	m_clientlist = ctlist;
	return TRUE;
}


/*
 * 創建發送和接收資料執行緒
 */
BOOL CClient::StartRuning(void)
{
	m_bConning = TRUE;//設置連接狀態

	//創建接收資料執行緒
	unsigned long ulThreadId;
	m_hThreadRecv = CreateThread(NULL, 0, RecvDataThread, this, 0, &ulThreadId);
	if(NULL == m_hThreadRecv)
	{
		return FALSE;
	}else{
		CloseHandle(m_hThreadRecv);
	}

	//創建接收用戶端資料的執行緒
	m_hThreadSend =  CreateThread(NULL, 0, SendDataThread, this, 0, &ulThreadId);
	if(NULL == m_hThreadSend)
	{
		return FALSE;
	}else{
		CloseHandle(m_hThreadSend);
	}
	
	return TRUE;
}


/*
 * 接收用戶端資料
 */
DWORD  CClient::RecvDataThread(void* pParam)
{
	CClient *pClient = (CClient*)pParam;	//用戶端對象指標
	int		reVal;							//返回值
	char	temp[MAX_NUM_BUF];				//臨時變數

	memset(temp, 0, MAX_NUM_BUF);
	
	for (;pClient->m_bConning;)				//連接狀態
	{
		reVal = recv(pClient->m_socket, temp, MAX_NUM_BUF, 0);	//接收資料
		
		//處理錯誤返回值
		if (SOCKET_ERROR == reVal)
		{
			int nErrCode = WSAGetLastError();
			
			if ( WSAEWOULDBLOCK == nErrCode )	//接受資料緩衝區不可用
			{
				continue;						//繼續迴圈
			}else if (WSAENETDOWN == nErrCode ||//用戶端關閉了連接
					 WSAETIMEDOUT == nErrCode ||
					WSAECONNRESET == nErrCode )			
			{
				break;							//執行緒退出				
			}
		}
		
		//用戶端關閉了連接
		if ( reVal == 0)	
		{
			break;
		}
	
		//收到資料
		if (reVal > HEADERLEN)			
		{		
			pClient->HandleData(temp);		//處理資料

			SetEvent(pClient->m_hEvent);	//通知發送資料執行緒

			memset(temp, 0, MAX_NUM_BUF);	//清空臨時變數
		}
		
		Sleep(TIMEFOR_THREAD_CLIENT);		//執行緒睡眠
	}
	
	pClient->m_bConning = FALSE;			//與用戶端的連接斷開
	SetEvent(pClient->m_hEvent);			//通知發送資料執行緒退出
	
	return 0;								//執行緒退出
}

/*
 *  計算運算式,打包資料
 */
void CClient::HandleData(const char* pExpr)	
{
	memset(m_data.buf, 0, MAX_NUM_BUF);//清空m_data
	//如果是“byebye”或者“Byebye”
	if (BYEBYE == ((phdr)pExpr)->type)		
	{
		EnterCriticalSection(&m_cs);
		phdr pHeaderSend = (phdr)m_data.buf;				//發送的資料		
		pHeaderSend->type = BYEBYE;							//單詞類型
		pHeaderSend->len = HEADERLEN + strlen("OK");		//數據包長度
		memcpy(m_data.buf + HEADERLEN, "OK", strlen("OK"));	//複製資料到m_data"
		LeaveCriticalSection(&m_cs);
	}

	//算數運算式
	if (EXPRESSION == ((phdr)pExpr)->type)
	{
		int nFirNum;		//第一個數字
		int nSecNum;		//第二個數字
		char cOper;			//算數運算子
		int nResult;		//計算結果
		//格式化讀入資料
		sscanf(pExpr + HEADERLEN, "%d%c%d", &nFirNum, &cOper, &nSecNum);
		
		//計算
		switch(cOper)
		{
			case '+'://加
				{
					nResult = nFirNum + nSecNum;
					break;
				}
			case '-'://減
				{
					nResult = nFirNum - nSecNum;
					break;
				}
			case '*'://乘
				{
					nResult = nFirNum * nSecNum;				
					break;
				}
			case '/'://除
				{
					if (ZERO == nSecNum)//無效的數字
					{
						nResult = INVALID_NUM;
					}else
					{
						nResult = nFirNum / nSecNum;	
					}				
					break;
				}
			default:
				nResult = INVALID_OPERATOR;//無效操作符
				break;			
		}
		
		//將算數運算式和計算的結果寫入字元陣列中
		char temp[MAX_NUM_BUF];
		char cEqu = '=';
		sprintf(temp, "User_%d: %d%c%d%c%d", this->seq, nFirNum, cOper, nSecNum,cEqu, nResult);

		//打包數據
		EnterCriticalSection(&m_cs);
		phdr pHeaderSend = (phdr)m_data.buf;				//發送的資料		
		pHeaderSend->type = EXPRESSION;						//資料類型為算數運算式
		pHeaderSend->len = HEADERLEN + strlen(temp);		//數據包的長度
		memcpy(m_data.buf + HEADERLEN, temp, strlen(temp));	//複製資料到m_data
		LeaveCriticalSection(&m_cs);
		
	}
	
	//會話
	if (CONVERSATION == ((phdr)pExpr)->type)
	{
		//請完成打包對話的數據到m_data.buf
		char temp[200];
		EnterCriticalSection(&m_cs);
		phdr pHeaderSend = (phdr)m_data.buf;				//發送的資料		
		pHeaderSend->type = CONVERSATION;					//單詞類型
		sprintf(temp, "User_%d: %s", this->seq, pExpr+HEADERLEN);
		pHeaderSend->len = HEADERLEN + strlen(temp);		//數據包長度		
		memcpy(m_data.buf + HEADERLEN, temp, strlen(temp));	//複製資料到m_data
		LeaveCriticalSection(&m_cs);
	}
}


/*
 * //向用戶端發送資料
 */
DWORD CClient::SendDataThread(void* pParam)	
{
	int val;
	CClient *pClient = (CClient*)pParam;//轉換資料類型為CClient指針

	for (;pClient->m_bConning;)//連接狀態
	{	
		//收到事件通知
		if (WAIT_OBJECT_0 == WaitForSingleObject(pClient->m_hEvent, INFINITE))
		{
			//當用戶端的連接斷開時，接收資料執行緒先退出，然後該執行緒後退出，並設置退出標誌
			if (!pClient->m_bConning)
			{
				pClient->m_bExit = TRUE;
				break;
			}

			//傳送給本客戶端
			EnterCriticalSection(&pClient->m_cs);		
			phdr pHeader = (phdr)pClient->m_data.buf;
			int nSendlen = pHeader->len;
			/* 
			    請根據資料型態判斷如何傳送資料到本客戶端,同時也要對send()函式的返回錯誤進行處理
			*/
			if(EXPRESSION == pHeader->type || BYEBYE == pHeader->type)
			{
				val = send(pClient->m_socket, pClient->m_data.buf, nSendlen,0);
				
				//處理返回錯誤
				if(SOCKET_ERROR == val)
				{
					int nErrCode = WSAGetLastError();
					if (nErrCode == WSAEWOULDBLOCK)
					{
						continue;
					}else if(WSAENETDOWN == nErrCode ||
							WSAETIMEDOUT == nErrCode ||
							WSAECONNRESET == nErrCode)
					{
						LeaveCriticalSection(&pClient->m_cs);
						pClient->m_bConning = FALSE;
						pClient->m_bExit = TRUE;
						break;
					}else {
						LeaveCriticalSection(&pClient->m_cs);
						pClient->m_bConning = FALSE;
						pClient->m_bExit =TRUE;
						break;
					}
				}
			}
			

			//廣播至其他客戶
			EnterCriticalSection(&pClient->m_cs_clientlist);//進入臨界區 (針對 m_clientlist)
			CLIENTLIST::iterator iter = pClient->m_clientlist.begin();
			if(BYEBYE == pHeader->type) 
			{
				/**
				 * 如果為BYEBYE 將m_data.buf 與 type的內容更換
				 * 將使用者離開的訊息廣播給其他客戶
				 */
				char temp[100];
				char *pC = pClient->m_data.buf;
				sprintf(temp, "User_%d has been leaved", pClient->seq);				
				memcpy(pC + HEADERLEN, temp, strlen(temp));
				pHeader->type = CONVERSATION;
				pHeader->len = HEADERLEN + strlen(temp);
				nSendlen = HEADERLEN + strlen(temp);
			}
			for (iter; iter != pClient->m_clientlist.end();){
				CClient *pTemp = (CClient*)*iter;
				++iter;
				if(pClient->seq != pTemp->seq && (EXPRESSION == pHeader->type || CONVERSATION == pHeader->type))
				{
					val = send(pTemp->m_socket, pClient->m_data.buf, nSendlen,0);
					
					//處理返回錯誤
					if(SOCKET_ERROR == val)
					{
						int nErrCode = WSAGetLastError();
						if (nErrCode == WSAEWOULDBLOCK)
						{
							continue;
						}else if(WSAENETDOWN == nErrCode ||
								WSAETIMEDOUT == nErrCode ||
								WSAECONNRESET == nErrCode)
						{
							LeaveCriticalSection(&pClient->m_cs_clientlist);//離開臨界區
							LeaveCriticalSection(&pClient->m_cs);
							pClient->m_bConning = FALSE;
							pClient->m_bExit = TRUE;
							break;
						}else {
							LeaveCriticalSection(&pClient->m_cs_clientlist);//離開臨界區
							LeaveCriticalSection(&pClient->m_cs);
							pClient->m_bConning = FALSE;
							pClient->m_bExit =TRUE;
							break;
						}
					}
				}
			}
			LeaveCriticalSection(&pClient->m_cs_clientlist);//離開臨界區
			LeaveCriticalSection(&pClient->m_cs);//離開臨界區
			ResetEvent(&pClient->m_hEvent);//設置事件為無信號狀態	

			if(pClient->m_bExit == TRUE)//此客戶端非正常離線
			{
				break;
			}
		}
	}

	return 0;
}
