
// LamanDlg.cpp: файл реализации
//

#include "stdafx.h"
#include <stdlib.h>
#include "Laman.h"
#include "LamanDlg.h"
#include "afxdialogex.h"
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
//#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Диалоговое окно CLamanDlg



CLamanDlg::CLamanDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_LAMAN_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLamanDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CLamanDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CLamanDlg::OnBnClickedButton1)
	ON_WM_SYSCOMMAND()
	ON_BN_CLICKED(IDCANCEL, &CLamanDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// Обработчики сообщений CLamanDlg

BOOL CLamanDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Задает значок для этого диалогового окна.  Среда делает это автоматически,
	//  если главное окно приложения не является диалоговым
	SetIcon(m_hIcon, TRUE);			// Крупный значок
	SetIcon(m_hIcon, FALSE);		// Мелкий значок
	try {
		/* Create a connection */
		sql::Driver *driver = get_driver_instance();
		std::unique_ptr<sql::Connection> con( driver->connect("tcp://127.0.0.1:3306", "root", "") );
		con->setSchema("work");/* Connect to the MySQL test database */
		std::unique_ptr<sql::PreparedStatement> pstmt;
		std::unique_ptr<sql::ResultSet> res;
		pstmt.reset(con->prepareStatement("SELECT * FROM sign"));
		res.reset(pstmt->executeQuery());
		while (res->next()) {
			std::wstring tstr;
			StringToWString(tstr, std::string(res->getString(1).c_str()));
			this->SetDlgItemTextW(IDC_EDIT1, tstr.c_str());
			StringToWString(tstr, std::string(res->getString(2).c_str()));
			this->SetDlgItemTextW(IDC_EDIT2, tstr.c_str());
			StringToWString(tstr, std::string(res->getString(3).c_str()));
			this->SetDlgItemTextW(IDC_EDIT3, tstr.c_str());
			StringToWString(tstr, std::string(res->getString(4).c_str()));
			this->SetDlgItemTextW(IDC_EDIT4, tstr.c_str());
		}
		this->SetDlgItemTextW(IDC_EDIT5, L"Database connected");
	}
	catch (sql::SQLException &e) {
		std::ostringstream err_str;
		err_str << "# ERR: " << e.what() << " (MySQL error code: " << e.getErrorCode()
			<< ", SQLState: " << e.getSQLState() << " )";
		std::string estr = err_str.str();
		std::wstring terrstr(estr.begin(), estr.end());
		this->SetDlgItemTextW(IDC_EDIT5, terrstr.c_str());
	}

	return TRUE;  // возврат значения TRUE, если фокус не передан элементу управления
}

// При добавлении кнопки свертывания в диалоговое окно нужно воспользоваться приведенным ниже кодом,
//  чтобы нарисовать значок.  Для приложений MFC, использующих модель документов или представлений,
//  это автоматически выполняется рабочей областью.

void CLamanDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // контекст устройства для рисования

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Выравнивание значка по центру клиентского прямоугольника
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Нарисуйте значок
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// Система вызывает эту функцию для получения отображения курсора при перемещении
//  свернутого окна.
HCURSOR CLamanDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CLamanDlg::StringToWString(std::wstring & ws, const std::string & s)
{
	std::wstring ws_tmp(s.begin(), s.end());
	ws = ws_tmp;
}

void CLamanDlg::OnBnClickedButton1()
{
	wchar_t login[64];
	wchar_t password[64];
	wchar_t server[64];
	wchar_t port[8];
	//---
	this->GetDlgItemTextW(IDC_EDIT3, login, _countof(login));
	this->GetDlgItemTextW(IDC_EDIT1, server, _countof(server));
	this->GetDlgItemTextW(IDC_EDIT4, password, _countof(password));
	this->GetDlgItemTextW(IDC_EDIT2, port, _countof(port));
	int len = swprintf_s(server, 64, L"%s:%s", server,port);
	this->SetDlgItemTextW(IDC_EDIT5, L"Connect in process...");
	//--- manager login
	if (!m_manager.Login(server, _wcstoui64(login, NULL, 10), password)) {
		this->SetDlgItemTextW(IDC_EDIT5, L"Start fail");
	}
}


void CLamanDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (nID == SC_CLOSE)
	{
		this->SetDlgItemTextW(IDC_EDIT5, L"Shutdown proccess");
	}
	CDialogEx::OnSysCommand(nID, lParam);
}


void CLamanDlg::OnBnClickedCancel()
{
	// TODO: добавьте свой код обработчика уведомлений
	m_manager.Stop();
	CDialogEx::OnCancel();
}


BOOL CLamanDlg::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	switch (message) {
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_UNCONNECTED) {
			this->SetDlgItemTextW(IDC_EDIT5, L"Is no connection");
		}
		if (LOWORD(wParam) == IDC_CONNECTED) {
			this->SetDlgItemTextW(IDC_EDIT5, L"Connection is correct");
		}
		break;
	}
	return CDialogEx::OnWndMsg(message, wParam, lParam, pResult);
}
