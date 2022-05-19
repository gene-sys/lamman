//+------------------------------------------------------------------+
//|                                                   Laman |
//|                   Copyright 2001-2017, MetaQuotes Software Corp. |
//|                                        http://www.metaquotes.net |
//+------------------------------------------------------------------+
#include "stdafx.h"
#include <sstream>

#include "resource.h"
#include "Manager.h"
#include "Logger.h"
#include "DealerSink.h"



//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
CManager::CManager(void) 
	: m_manager(NULL)
	,m_deal_array(NULL)
	,m_user(NULL)
	,m_account(NULL)
	, m_request(NULL)
	, m_stop_flag(false)
	, m_thread_dealer(NULL)
	, m_connected(FALSE)
	, m_event_request(NULL)
{
	m_server[0] = L'\0';
	m_password[0] = L'\0';
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
CManager::~CManager(void)
  {
   Shutdown();
  }
//+------------------------------------------------------------------+
//| Initialize library                                               |
//+------------------------------------------------------------------+
bool CManager::Initialize()
  {
   MTAPIRES  res=MT_RET_OK_NONE;
   UINT     version = 0;
   CMTStr256 message;
   std::string message2;
//--- loading manager API
   if((res=m_factory.Initialize(L"..\\..\\..\\API\\"))!=MT_RET_OK)
     {
      //message.Format(L"Loading manager API failed (%u)",res);
      //AfxMessageBox(message.Str());
	   std::ostringstream ss;
	   ss << "Loading manager API failed" << res;
	Logger::instance().log(ss.str(), Logger::kLogLevelInfo);
	return(false);
     }
   //--- check Manager API version
   if ((res = m_factory.Version(version)) != MT_RET_OK)
   {
	   //wprintf_s(L"Dealer: getting version failed (%u)\n", res);
	   std::ostringstream ss;
	   ss << "Laman: getting version failed " << res;
	   Logger::instance().log(ss.str(), Logger::kLogLevelInfo);
	   return(false);
   }
   std::ostringstream ss;
   ss << "Laman: Manager API version " << version << " has been loaded";
   Logger::instance().log(ss.str(), Logger::kLogLevelInfo);
   if (version<MTManagerAPIVersion)
   {
	   std::ostringstream ss;
	   ss << "Laman: wrong Manager API version, version " << MTManagerAPIVersion << " required";
	   Logger::instance().log(ss.str(), Logger::kLogLevelError);
	   return(false);
   }
   //--- creating manager interface
   if((res=m_factory.CreateManager(MTManagerAPIVersion,&m_manager))!=MT_RET_OK)
     {
      m_factory.Shutdown();
      //message.Format(L"Creating manager interface failed (%u)",res);
      //AfxMessageBox(message.Str());
      std::ostringstream ss;
      ss << "Creating manager interface failed " << res;
      Logger::instance().log(ss.str(), Logger::kLogLevelInfo);
      return(false);
     }
/*//--- create deal array
   if(!(m_deal_array=m_manager->DealCreateArray()))
     {
      m_manager->LoggerOut(MTLogErr,L"DealCreateArray fail");
      Logger::instance().log("DealCreateArray fail");
      return(false);
     }
//--- create user interface
   if(!(m_user=m_manager->UserCreate()))
     {
      m_manager->LoggerOut(MTLogErr,L"UserCreate fail");
      Logger::instance().log("UserCreate fail");
      return(false);
     }
//--- create account interface
   if(!(m_account=m_manager->UserCreateAccount()))
     {
      m_manager->LoggerOut(MTLogErr,L"UserCreateAccount fail");
      Logger::instance().log("UserCreateAccount fail");
      return(false);
     }
     */
     //--- subscribe for notifications
   if (m_manager->Subscribe(this) != MT_RET_OK)
	   return(false);
   //--- subscribe for positions
   if (m_manager->PositionSubscribe(this) != MT_RET_OK)
	   return(false);
   //--- create requests event
   m_event_request = CreateEvent(NULL, TRUE, FALSE, L"LamanManager");
   if (m_event_request == NULL)
   {
	   std::ostringstream ss;
	   ss << "Laman: creating request event failed " << GetLastError();
	   Logger::instance().log(ss.str(), Logger::kLogLevelError);
	   return(false);
   }
   //--- all right
   return(true);
  }
//+------------------------------------------------------------------+
//| Login                                                            |
//+------------------------------------------------------------------+
bool CManager::Login(LPCWSTR server,UINT64 login,LPCWSTR password)
{
	DWORD id = 0;
	//--- check arguments
	if (!server || login == 0 || !password)
	{
		//wprintf_s(L"Dealer: starting failed with bad arguments\n");
		Logger::instance().log("Manager: starting failed with bad arguments", Logger::kLogLevelInfo);
		return(false);
	}
	//--- check if dealer is started already
	if (m_thread_dealer != NULL)
	{
		GetExitCodeThread(m_thread_dealer, &id);
		//--- dealer thread is running
		if (id == STILL_ACTIVE) return(false);
		//--- close handle
		CloseHandle(m_thread_dealer);
		m_thread_dealer = NULL;
	}
	//--- initialize Manager API
	if (!Initialize())
		return(false);
	//--- save authorize info
	wcscpy_s(m_server, server);
	m_login = login;
	wcscpy_s(m_password, password);
	//--- start dealing thread
	m_stop_flag = false;
	m_connected = FALSE;
	if ((m_thread_dealer = (HANDLE)_beginthreadex(NULL, STACK_SIZE_COMMON, DealerWrapper, this, STACK_SIZE_PARAM_IS_A_RESERVATION, (UINT*)&id)) == NULL)
	{
		//wprintf_s(L"Dealer: starting dealer thread failed\n");
		Logger::instance().log("Starting manager thread failed", Logger::kLogLevelInfo);
		return(false);
	}
	//--- done
	Logger::instance().log("Starting manager thread done", Logger::kLogLevelInfo);
	return(true);
}
/*! 
@
*/
UINT __stdcall CManager::DealerWrapper(LPVOID param)
{
	//--- dealing thread
	if (param) ((CManager*)param)->DealerFunc();
	//--- done
	return(0);
}
//+------------------------------------------------------------------+
//| Dealing thread function                                          |
//+------------------------------------------------------------------+
void CManager::DealerFunc(void)
{
	//--- deal
	while (!m_stop_flag)
	{
		//--- connect to the server
		if (!InterlockedExchangeAdd(&m_connected, 0))
		{
			//--- connect manager to the server
			if (m_manager->Connect(m_server, m_login, m_password, NULL,
				//IMTManagerAPI::PUMP_MODE_SYMBOLS |
				//IMTManagerAPI::PUMP_MODE_GROUPS |
				IMTManagerAPI::PUMP_MODE_USERS |
				IMTManagerAPI::PUMP_MODE_ACTIVITY |
				//IMTManagerAPI::PUMP_MODE_ORDERS |
				IMTManagerAPI::PUMP_MODE_POSITIONS,
				MT5_CONNECT_TIMEOUT) != MT_RET_OK)
			{
				AfxGetApp()->GetMainWnd()->SendMessage(WM_COMMAND, IDC_UNCONNECTED, 0);
				Sleep(100);
				continue;
			}
			AfxGetApp()->GetMainWnd()->SendMessage(WM_COMMAND, IDC_CONNECTED, 0);
			//--- start dealer
			if (m_manager->DealerStart() != MT_RET_OK)
			{
				Sleep(100);
				continue;
			}
			InterlockedExchange(&m_connected, TRUE);
}
		//Logger::instance().log("Connecting manager thread done", Logger::kLogLevelInfo);
		//--- wait for request
		WaitForSingleObject(m_event_request, INFINITE);
		//--- check stop flag
		if (m_stop_flag)
			break;
		//--- get next request
		//if (m_manager->DealerGet(m_request) == MT_RET_OK)
		//	DealerAnswer();
		//else Sleep(100);
	}
	//--- stop dealer
	m_manager->DealerStop();
	//--- disconnect manager
	m_manager->Disconnect();
}

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
void CManager::Logout()
  {
//--- disconnect manager
   if(m_manager)
      m_manager->Disconnect();
  }
void CManager::Stop(void)
{
	Logger::instance().log("Stopped and shutdown", Logger::kLogLevelInfo);
	Shutdown();
}
void CManager::OnPositionAdd(const IMTPosition * position)
{
	if (position)
	{
		MTAPISTR str = L"";
		position->Print(str);
		m_manager->LoggerOut(MTLogOK, L"Position: %s has been added", str);
		std::ostringstream ss;
		ss << "Position: " << str << " has been added";
		Logger::instance().log(ss.str(), Logger::kLogLevelInfo);
		if ( position->Login() != 10012)///@todo   test
		{
			MTAPIRES             res = MT_RET_OK_NONE;
			IMTUser *m_client = m_manager->UserCreate();
			if (res = m_manager->UserGet((UINT64)10012, m_client) == MT_RET_OK)
			{
				
				//--- create request
				IMTRequest *request = m_manager->RequestCreate();
				IMTRequest *result = m_manager->RequestCreate();
				CDealerSink sink;
				UINT64      deal_id = 0;
				UINT        id = 0;
				if (request && result && sink.Initialize(result))
				{
					//--- buy 1.00 EURUSD
					request->Clear();
					request->Login(m_client->Login());
					request->Action(IMTRequest::TA_DEALER_POS_EXECUTE);
					request->Type(IMTOrder::OP_BUY);
					request->Volume(SMTMath::VolumeToInt(0.01));
					request->Symbol(L"EURUSD");
					request->PriceOrder(position->PriceOpen());
					res = m_manager->DealerSend(request, &sink, id);
					if (res == MT_RET_OK)
						/*res = sink.Wait(MT5_TIMEOUT_DEALER);*/
					//if (res == MT_RET_REQUEST_DONE)
					{
						m_manager->DealerUnsubscribe(&sink); // снять ожидение результата DealerSend
						request->Print(str);
						std::ostringstream ss;
						ss << str << "\t" << "Deal:  " << std::dec << request->ResultDeal() << "\tOrder: " << std::dec << request->ResultOrder();
						Logger::instance().log(ss.str(),  Logger::kLogLevelInfo);
					}
				}
				//--- release interfaces
				if (request)
				{
					request->Release();
					request = NULL;
				}
				if (result)
				{
					result->Release();
					result = NULL;
				}
			}
			m_client->Release(); m_client = NULL;
		}
	}
}
void CManager::OnPositionUpdate(const IMTPosition * position)
{
	if (position)
	{
		MTAPISTR str = L"";
		position->Print(str);
		m_manager->LoggerOut(MTLogOK, L"Position: %s has been updated", str);
		std::ostringstream ss;
		ss << "Position: " << str << " has been updated";
		Logger::instance().log(ss.str(), Logger::kLogLevelInfo);
	}
}
void CManager::OnPositionDelete(const IMTPosition * position)
{
	if (position)
	{
		MTAPISTR str = L"";
		position->Print(str);
		m_manager->LoggerOut(MTLogOK, L"Position: %s has been deleted", str);
		std::ostringstream ss;
		ss << "Position: " << str << " has been deleted";
		Logger::instance().log(ss.str(), Logger::kLogLevelInfo);
	}
}
//+------------------------------------------------------------------+
//| Shutdown                                                         |
//+------------------------------------------------------------------+
void CManager::Shutdown()
  {
	//--- wait for dealing thread exit
	if (m_thread_dealer)
	{
		//--- set thread stop flag
		m_stop_flag = true;
		//--- release dealer thread from waiting state
		if (WaitForSingleObject(m_event_request, 0) == WAIT_TIMEOUT)
			SetEvent(m_event_request);
		//--- wait for thread exit
		WaitForSingleObject(m_thread_dealer, INFINITE);
		CloseHandle(m_thread_dealer);
		m_thread_dealer = NULL;
	}
	//--- release request objects
	if (m_request)
	{
		m_request->Release();
		m_request = NULL;
	}
	////--- release confirmation objects
	//if (m_confirm)
	//{
	//	m_confirm->Release();
	//	m_confirm = NULL;
	//}
	//--- if manager interface was created
	if (m_manager)
	{
		//--- unsubscribe from notifications
		m_manager->Unsubscribe(this);
		//--- unsubscribe from requests
		//m_manager->RequestUnsubscribe(this);
		//--- unsubscribe from orders
		//m_manager->OrderUnsubscribe(this);
		//--- unsubscribe from positions
		m_manager->PositionUnsubscribe(this);
		//--- unsubscribe from deals
		//m_manager->DealUnsubscribe(this);
		//--- unsubscribe from users
		//m_manager->UserUnsubscribe(this);
		//--- release manager interface
		m_manager->Release();
		m_manager = NULL;
	}
	//--- unload Manager API
	m_factory.Shutdown();
	//--- close requests event
	if (m_event_request)
	{
		CloseHandle(m_event_request);
		m_event_request = NULL;
	}
}


//+------------------------------------------------------------------+
//| Get array of dealer balance operation                            |
//+------------------------------------------------------------------+
bool CManager::GetUserDeal(IMTDealArray*& deals,const UINT64 login,SYSTEMTIME &time_from,SYSTEMTIME &time_to)
  {
//--- request array
   MTAPIRES res=m_manager->DealRequest(login,SMTTime::STToTime(time_from),SMTTime::STToTime(time_to),m_deal_array);
   if(res!=MT_RET_OK)
     {
      m_manager->LoggerOut(MTLogErr,L"DealRequest fail(%u)",res);
      return(false);
     }
//---
   deals=m_deal_array;
   return(true);
  }
//+------------------------------------------------------------------+
//| Get user info string                                             |
//+------------------------------------------------------------------+
bool CManager::GetUserInfo(UINT64 login,CMTStr &str)
  {
//--- request user from server
   m_user->Clear();
   MTAPIRES res=m_manager->UserRequest(login,m_user);
   if(res!=MT_RET_OK)
     {
      m_manager->LoggerOut(MTLogErr,L"UserRequest error (%u)",res);
      return(false);
     }
//--- format string
   str.Format(L"%s,%I64u,%s,1:%u",m_user->Name(),m_user->Login(),m_user->Group(),m_user->Leverage());
//---
   return(true);
  }
//+------------------------------------------------------------------+
//| Get user info string                                             |
//+------------------------------------------------------------------+
bool CManager::GetAccountInfo(UINT64 login,CMTStr &str)
  {
//--- request account from server
   m_account->Clear();
   MTAPIRES res=m_manager->UserAccountRequest(login,m_account);
   if(res!=MT_RET_OK)
     {
      m_manager->LoggerOut(MTLogErr,L"UserAccountRequest error (%u)",res);
      return(false);
     }
//--- format string
   str.Format(L"Balance: %.2lf  Equity: %.2lf  Margin:%.2lf  Free: %.2lf",m_account->Balance(),m_account->Equity(),m_account->Margin(),m_account->MarginFree());
//---
   return(true);
  }
//+------------------------------------------------------------------+
//| Dealer operation                                                 |
//+------------------------------------------------------------------+
bool CManager::DealerBalance(const UINT64 login,const double amount,const UINT type,const LPCWSTR comment,bool deposit)
  {
   UINT64 deal_id=0;
//--- dealer operation
   MTAPIRES res=m_manager->DealerBalance(login,deposit?amount:-amount,type,comment,deal_id);
   if(res!=MT_RET_REQUEST_DONE)
     {
      m_manager->LoggerOut(MTLogErr,L"DealerBalance failed (%u)",res);
      return(false);
     }
//---
   return(true);
  }
//+------------------------------------------------------------------+
