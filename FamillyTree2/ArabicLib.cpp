//---------------------------------------------------------------------------
#include <vcl.h>
#include <Usp10.h>
#pragma hdrstop
//---------------------------------------------------------------------------
#include "ArabicLib.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
int GetArabicOffset(HDC hdc)
{
    AnsiString Text = 'Ú';// 0xFEC9
	UINT Count = Text.Length();
 	wchar_t *Glyphs=new wchar_t[Count];

 	ConvertToArabic(hdc,Text,Glyphs,NULL,NULL,Count);
	int nOffset = 0xFEC9 - Glyphs[0];

 	if(Glyphs)	delete[] Glyphs;
    return nOffset;
}
//---------------------------------------------------------------------------
void ConvertToArabic(HDC hdc,AnsiString Text,UINT *Indices,UINT *Orders,int *pDx,UINT &Count)
{
	TEXTMETRIC TextMetrics;
    GetTextMetrics(hdc,&TextMetrics);
    UINT nOffset = 0;
    switch(TextMetrics.tmCharSet)
    {
    	case ARABIC_CHARSET:
        {
        	nOffset = GetArabicOffset(hdc);
         	break;
        }
    }

	Count = Text.Length();
 	wchar_t *Glyphs=new wchar_t[Count];

 	ConvertToArabic(hdc,Text,Glyphs,Orders,pDx,Count);

 	for(UINT i=0;i<Count;i++)
 	{
    	Indices[i] = nOffset + DWORD(Glyphs[i]);
 	}

 	if(Glyphs)	delete[] Glyphs;
}
//---------------------------------------------------------------------------
void ConvertToArabic(HDC hdc,AnsiString Text,wchar_t *Glyphs,UINT *Orders,int *pDx,UINT &Count)
{
 	GCP_RESULTS TextInfo;
 	DWORD Flag;
    char OutString[255];

// 	Flag = GetFontLanguageInfo(hdc);
 	Flag = GCP_REORDER;
    Flag |= GCP_DIACRITIC;
    Flag |= GCP_DISPLAYZWG;
    Flag |= GCP_GLYPHSHAPE;
    Flag |= GCP_JUSTIFY;
    Flag |= GCP_KASHIDA;
    Flag |= GCP_NEUTRALOVERRIDE;
    Flag |= GCP_NUMERICSLATIN;
    Flag |= GCP_NUMERICSLOCAL;
    Flag |= GCP_USEKERNING;
	Glyphs[0] = 0;

 	TextInfo.lStructSize=sizeof(GCP_RESULTS);
 	TextInfo.lpOutString=OutString;
 	TextInfo.lpOrder=Orders;
 	TextInfo.lpDx=pDx;
 	TextInfo.lpCaretPos=NULL;
 	TextInfo.lpClass=NULL;
 	TextInfo.lpGlyphs=Glyphs;
 	TextInfo.nGlyphs=Count;
 	TextInfo.nMaxFit=Count;
 	GetCharacterPlacement(hdc,Text.c_str(),Count,0,&TextInfo,Flag);
}
//---------------------------------------------------------------------------

