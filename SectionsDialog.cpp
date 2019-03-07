#include "stdafx.h"
#include "PE32Viewer.h"
#include "PE32PropsView.h"
#include "PE32Decoder.h"
#include "PE32Misc.h"

#define STRSAFE_LIB
#include <strsafe.h>
#include <time.h>
// CPE32PropsView

#include <stdarg.h>         // for va_start, etc.

INT_PTR CALLBACK CPE32PropsView::ChildSectionsDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			CPE32PropsView* pView(reinterpret_cast<CPE32PropsView*>(lParam));
			HWND hTree(::GetDlgItem(hDlg, IDC_SECTIONS_TREE));

			if (::IsThemeActive() && ::IsAppThemed())
			{
				::SetWindowTheme(hDlg, VSCLASS_AEROWIZARD, nullptr);
				::EnableThemeDialogTexture(hDlg, ETDT_ENABLE);
				::SetWindowTheme(hTree, VSCLASS_EXPLORER, nullptr);
			}

			PIMAGE_SECTION_HEADER pImgSectionsDescriptors(nullptr);
			pView ->mf_pPE32 ->GetSectionsPtr(&pImgSectionsDescriptors);

			PIMAGE_FILE_HEADER pImgFileHdr(nullptr);
			pView ->mf_pPE32 ->GetFileHeaderPtr(&pImgFileHdr);

			HTREEITEM htiSect(nullptr);

			for (WORD nSectionIndex(0); nSectionIndex < pImgFileHdr ->NumberOfSections; ++nSectionIndex)
			{
				char szSectionNameA[IMAGE_SIZEOF_SHORT_NAME + 1] = { 0 };
				for (UINT nCharIndex(0); nCharIndex < IMAGE_SIZEOF_SHORT_NAME; ++nCharIndex)
					if (pImgSectionsDescriptors[nSectionIndex].Name[nCharIndex])
						szSectionNameA[nCharIndex] = pImgSectionsDescriptors[nSectionIndex].Name[nCharIndex];
					else
						break;

				HTREEITEM htiCurrentSect(AddItemToTreeViewA(hTree, nullptr, htiSect, szSectionNameA));
				htiSect = htiCurrentSect;

				CComBSTR Line;
				//	Virtual size
				FormatResource(Line, IDS_SECTION_VIRTUAL_SIZE, pImgSectionsDescriptors[nSectionIndex].Misc.VirtualSize);
				AddItemToTreeView(hTree, htiCurrentSect, TVI_LAST, Line);
				//	Virtual address
				FormatResource(Line, IDS_SECTION_VIRTUAL_ADDRESS, pImgSectionsDescriptors[nSectionIndex].VirtualAddress);
				AddItemToTreeView(hTree, htiCurrentSect, TVI_LAST, Line);
				//	The size of the initialized data on disk
				FormatResource(Line, IDS_SECTION_SIZE_OF_RAW_DATA, pImgSectionsDescriptors[nSectionIndex].SizeOfRawData);
				AddItemToTreeView(hTree, htiCurrentSect, TVI_LAST, Line);
				//	A file pointer to the first page within the COFF file
				FormatResource(Line, IDS_SECTION_PTR_TO_RAW_DATA, pImgSectionsDescriptors[nSectionIndex].PointerToRawData);
				AddItemToTreeView(hTree, htiCurrentSect, TVI_LAST, Line);
				//	Relocation entries
				FormatResource(Line, IDS_SECTION_PTR_TO_RELOCATIONS, pImgSectionsDescriptors[nSectionIndex].PointerToRelocations);
				AddItemToTreeView(hTree, htiCurrentSect, TVI_LAST, Line);
				//	Pointer To Linenumbers
				FormatResource(Line, IDS_SECTION_PTR_TO_LINENUMBERS, pImgSectionsDescriptors[nSectionIndex].PointerToLinenumbers);
				AddItemToTreeView(hTree, htiCurrentSect, TVI_LAST, Line);
				//	Number Of Relocations 
				FormatResource(Line, IDS_SECTION_NUMBER_OF_RELOCATIONS, pImgSectionsDescriptors[nSectionIndex].NumberOfRelocations);
				AddItemToTreeView(hTree, htiCurrentSect, TVI_LAST, Line);
				//	Number Of Linenumbers  
				FormatResource(Line, IDS_SECTION_NUMBER_OF_LINENUMBERS, pImgSectionsDescriptors[nSectionIndex].NumberOfLinenumbers);
				AddItemToTreeView(hTree, htiCurrentSect, TVI_LAST, Line);
				//	Section's characteristics
				FormatResource(Line, IDS_SECTION_CHARACTERISTICS, pImgSectionsDescriptors[nSectionIndex].Characteristics);
				HTREEITEM htiSectChars(AddItemToTreeView(hTree, htiCurrentSect, TVI_LAST, Line));

				if (pImgSectionsDescriptors[nSectionIndex].Characteristics)
				{
					if (IMAGE_SCN_TYPE_NO_PAD & pImgSectionsDescriptors[nSectionIndex].Characteristics)
					{
						CResString SectionCharsLine(IDS_SECTION_CHARS_TYPE_NO_PAD);
						AddItemToTreeView(hTree, htiSectChars, TVI_LAST, SectionCharsLine.m_bszResString);
					}
					if (IMAGE_SCN_CNT_CODE & pImgSectionsDescriptors[nSectionIndex].Characteristics)
					{
						CResString SectionCharsLine(IDS_SECTION_CHARS_CNT_CODE);
						AddItemToTreeView(hTree, htiSectChars, TVI_LAST, SectionCharsLine.m_bszResString);
					}
					if (IMAGE_SCN_CNT_INITIALIZED_DATA & pImgSectionsDescriptors[nSectionIndex].Characteristics)
					{
						CResString SectionCharsLine(IDS_SECTION_CHARS_CNT_INITIALIZED_DATA);
						AddItemToTreeView(hTree, htiSectChars, TVI_LAST, SectionCharsLine.m_bszResString);
					}
					if (IMAGE_SCN_CNT_UNINITIALIZED_DATA & pImgSectionsDescriptors[nSectionIndex].Characteristics)
					{
						CResString SectionCharsLine(IDS_SECTION_CHARS_CNT_UNINITIALIZED_DATA);
						AddItemToTreeView(hTree, htiSectChars, TVI_LAST, SectionCharsLine.m_bszResString);
					}
					if (IMAGE_SCN_NO_DEFER_SPEC_EXC & pImgSectionsDescriptors[nSectionIndex].Characteristics)
					{
						CResString SectionCharsLine(IDS_SECTION_CHARS_SCN_NO_DEFER_SPEC_EXC);
						AddItemToTreeView(hTree, htiSectChars, TVI_LAST, SectionCharsLine.m_bszResString);
					}
					if (IMAGE_SCN_GPREL & pImgSectionsDescriptors[nSectionIndex].Characteristics)
					{
						CResString SectionCharsLine(IDS_SECTION_CHARS_SCN_GPREL);
						AddItemToTreeView(hTree, htiSectChars, TVI_LAST, SectionCharsLine.m_bszResString);
					}
					if (IMAGE_SCN_MEM_DISCARDABLE & pImgSectionsDescriptors[nSectionIndex].Characteristics)
					{
						CResString SectionCharsLine(IDS_SECTION_CHARS_SCN_MEM_DISCARDABLE);
						AddItemToTreeView(hTree, htiSectChars, TVI_LAST, SectionCharsLine.m_bszResString);
					}
					if (IMAGE_SCN_MEM_NOT_CACHED & pImgSectionsDescriptors[nSectionIndex].Characteristics)
					{
						CResString SectionCharsLine(IDS_SECTION_CHARS_SCN_MEM_DISCARDABLE);
						AddItemToTreeView(hTree, htiSectChars, TVI_LAST, SectionCharsLine.m_bszResString);
					}
					if (IMAGE_SCN_MEM_NOT_PAGED & pImgSectionsDescriptors[nSectionIndex].Characteristics)
					{
						CResString SectionCharsLine(IDS_SECTION_CHARS_SCN_MEM_NOT_PAGED);
						AddItemToTreeView(hTree, htiSectChars, TVI_LAST, SectionCharsLine.m_bszResString);
					}
					if (IMAGE_SCN_MEM_SHARED & pImgSectionsDescriptors[nSectionIndex].Characteristics)
					{
						CResString SectionCharsLine(IDS_SECTION_CHARS_SCN_MEM_SHARED);
						AddItemToTreeView(hTree, htiSectChars, TVI_LAST, SectionCharsLine.m_bszResString);
					}
					if (IMAGE_SCN_MEM_EXECUTE & pImgSectionsDescriptors[nSectionIndex].Characteristics)
					{
						CResString SectionCharsLine(IDS_SECTION_CHARS_SCN_MEM_EXECUTE);
						AddItemToTreeView(hTree, htiSectChars, TVI_LAST, SectionCharsLine.m_bszResString);
					}
					if (IMAGE_SCN_MEM_READ & pImgSectionsDescriptors[nSectionIndex].Characteristics)
					{
						CResString SectionCharsLine(IDS_SECTION_CHARS_SCN_MEM_READ);
						AddItemToTreeView(hTree, htiSectChars, TVI_LAST, SectionCharsLine.m_bszResString);
					}
					if (IMAGE_SCN_MEM_WRITE & pImgSectionsDescriptors[nSectionIndex].Characteristics)
					{
						CResString SectionCharsLine(IDS_SECTION_CHARS_SCN_MEM_WRITE);
						AddItemToTreeView(hTree, htiSectChars, TVI_LAST, SectionCharsLine.m_bszResString);
					}
				}
			}

			::MoveWindow(hDlg, pView ->mf_rcDisplay.left/* + ::GetSystemMetrics(SM_CXDLGFRAME)*/, pView ->mf_rcDisplay.top/* + ::GetSystemMetrics(SM_CYDLGFRAME)*/, pView ->mf_rcDisplay.right - pView ->mf_rcDisplay.left/* - 2 * ::GetSystemMetrics(SM_CXDLGFRAME)*/, pView ->mf_rcDisplay.bottom - pView ->mf_rcDisplay.top/* - 2 * ::GetSystemMetrics(SM_CYDLGFRAME)*/, FALSE);
			RECT rcClient;
			::GetClientRect(hDlg, &rcClient);
			--rcClient.bottom;
			--rcClient.right;
			::MoveWindow(hTree, rcClient.left, rcClient.top, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, FALSE);
			::SetWindowPos(hTree, HWND_TOP, 0, 0, 0, 0,  SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
		}
		return 1;
	}
	return 0;
}
