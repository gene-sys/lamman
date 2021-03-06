
// LamanDlg.h: файл заголовка
//
#pragma once
#include <sstream>
#include "Manager.h"

// Диалоговое окно CLamanDlg
class CLamanDlg : public CDialogEx
{
// Создание
public:
	CLamanDlg(CWnd* pParent = nullptr);	// стандартный конструктор

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LAMAN_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// поддержка DDX/DDV


// Реализация
protected:
	HICON m_hIcon;

	// Созданные функции схемы сообщений
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	CManager          m_manager;
	CEdit* m_editor;
	void StringToWString(std::wstring &ws, const std::string &s);
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnBnClickedCancel();
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
};
