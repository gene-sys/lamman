
// Laman.h: главный файл заголовка для приложения PROJECT_NAME
//

#pragma once
#include "stdafx.h"
#ifndef __AFXWIN_H__
	#error "включить stdafx.h до включения этого файла в PCH"
#endif

#include "resource.h"		// основные символы


// CLamanApp:
// Сведения о реализации этого класса: Laman.cpp
//

class CLamanApp : public CWinApp
{
public:
	CLamanApp();

// Переопределение
public:
	virtual BOOL InitInstance();

// Реализация

	DECLARE_MESSAGE_MAP()
};

extern CLamanApp theApp;
