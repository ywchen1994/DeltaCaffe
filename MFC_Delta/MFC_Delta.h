
// MFC_Delta.h : PROJECT_NAME ���ε{�����D�n���Y��
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�� PCH �]�t���ɮ׫e���]�t 'stdafx.h'"
#endif

#include "resource.h"		// �D�n�Ÿ�


// CMFC_DeltaApp: 
// �аѾ\��@�����O�� MFC_Delta.cpp
//

class CMFC_DeltaApp : public CWinApp
{
public:
	CMFC_DeltaApp();

// �мg
public:
	virtual BOOL InitInstance();

// �{���X��@

	DECLARE_MESSAGE_MAP()
};

extern CMFC_DeltaApp theApp;