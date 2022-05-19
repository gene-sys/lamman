//+------------------------------------------------------------------+
//|                                                   Laman |
//|                   Copyright 2001-2017, MetaQuotes Software Corp. |
//|                                        http://www.metaquotes.net |
//+------------------------------------------------------------------+
#pragma once
//#include "StdAfx.h"


//+------------------------------------------------------------------+
//| Manager                                                          |
//+------------------------------------------------------------------+
class CManager: public IMTManagerSink, public IMTPositionSink
  {
private:
   enum              constants
     {
      MT5_CONNECT_TIMEOUT=30000,       // connect timeout in milliseconds
      MT5_TIMEOUT_DEALER = 10000,
      STACK_SIZE_COMMON = 1024 * 1024,   // stack size for dealing thread in bytes
     };
   IMTManagerAPI    *m_manager;
   CMTManagerAPIFactory m_factory;
   IMTDealArray*     m_deal_array;
   IMTUser*          m_user;
   IMTUser*          m_client;
   IMTAccount*       m_account;
   IMTRequest       *m_request;        // request interface

   MTAPISTR          m_server;         // server address
   UINT64            m_login;          // dealer login
   MTAPISTR          m_password;       // dealer password
   volatile bool     m_stop_flag;      // dealing stop flag

   HANDLE            m_thread_dealer;  // dealing thread
   LONG              m_connected;      // connected flag
   HANDLE            m_event_request;  // request notifications event

public:
                     CManager(void);
                    ~CManager(void);
   //--- initialize, login
   bool              Login(LPCWSTR server,UINT64 login,LPCWSTR password); // =Start
   void              Logout();

   void              Stop(void);
   //--- IMTPositionSink implementation
   virtual void      OnPositionAdd(const IMTPosition* position);
   virtual void      OnPositionUpdate(const IMTPosition* position);
   virtual void      OnPositionDelete(const IMTPosition* position);

   //--- dealer operation
   bool              DealerBalance(const UINT64 login,const double amount,const UINT type,const LPCWSTR comment,bool deposit);
   //--- get info
   bool              GetUserDeal(IMTDealArray*& deals,const UINT64 login,SYSTEMTIME &time_from,SYSTEMTIME &time_to);
   bool              GetUserInfo(UINT64 login,CMTStr &str);
   bool              GetAccountInfo(UINT64 login,CMTStr &str);

private:
	bool              Initialize();
	void              Shutdown();
   static UINT __stdcall DealerWrapper(LPVOID param);
   void DealerFunc(void);
};
//+------------------------------------------------------------------+
