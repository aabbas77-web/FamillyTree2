//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "Main.h"
#include "ArabicLib.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFormMain *FormMain;
//---------------------------------------------------------------------------
__fastcall TFormMain::TFormMain(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::DrawText(double x,double y,AnsiString strText)
{
    VHANDLE hObj;
    AnsiString strNewText;
//            strText = "��� ���� ������ ������";
//            strText = "���_����_������______������";
//            strText = "���";
//            strText = "��� �";
//            strText = "\\U+00B0\\U+00B1\\U+2205\\U+20AC\\U+00A9";
//            strText = "\\U+FEC9";
//			strText = "\\U+FEC9\\U+FCEF\\U+FECA\\U+FECC\\U+FECB";
//			strText = "Autodesk \\OAutoCAD\\o 2000";
//			strText = "Autodesk \nAutoCAD\t 2000";

	// Inverse Phrase
    fm_Line Line;
    char c;
    strNewText = "";
    for(int i=1;i<=strText.Length();i++)
    {
    	c = strText[i];
        if(isspace(c) || iscntrl(c) || ispunct(c))
        {
            Line.push_back(strNewText);
            strNewText = "";
        }
        else
        {
            strNewText += strText[i];
            if(i == strText.Length())
            {
                Line.push_back(strNewText);
                strNewText = "";
            }
        }
    }

    strText = "";
    for(int i=Line.size()-1;i>=0;i--)
    {
        strText += Line[i]+" ";
    }

    UINT Count = strText.Length();
    UINT *Indices = new UINT[Count];
    UINT *Orders = new UINT[Count];
    int *pDx = new int[Count];
    ConvertToArabic(LabelFont->Canvas->Handle,strText,Indices,Orders,pDx,Count);
    strText = "";
    for(UINT i=0;i<Count;i++)
    {
        if(Indices[i] != 0xFCEF)// Space
        {
            strText += "\\U+"+IntToHex(int(Indices[i]),4);
        }
    }
    if(Indices)	delete[] Indices;
    if(Orders)	delete[] Orders;
    if(pDx)		delete[] pDx;

    double dRot = 0.0;
	double w,h;

    hObj = CadAddMText(hDwg,strText.c_str(),1000.0,x,y,0,CAD_TA_BOTLEFT,dRot);
    CadMTextPutPoint(hObj,x,y,0);
    //CadMTextPutAngle(hObj,dRot);

    w = CadMTextGetAW(hObj);
    h = CadMTextGetAH(hObj);
    CadAddRect(hDwg,x+w/2,y+h/2,0,w,h,dRot);
}
//---------------------------------------------------------------------------
static int DirsCount;
static int         Err;
static AnsiString  strDirName;
static AnsiString  strBaseDirName;
static bool Continue;
static TTreeNode *pParent;
static fm_Info Info;
static double x,y;
static int s_nIndex;
//---------------------------------------------------------------------------
void __fastcall TFormMain::SearchTree()
{
    TSearchRec	Sr;
    Err = FindFirst("*.*",faAnyFile,Sr);
    while(Continue &&(!Err))
    {
        Application->ProcessMessages();
        // Begin Recursion
        if((Sr.Attr & faDirectory) && (Sr.Name[1] != '.'))
        {
            strDirName = ExpandFileName(Sr.Name);

            fm_Info *pInfo = new fm_Info();
            pInfo->strName = ExtractRelativePath(strBaseDirName,strDirName);
            pInfo->x = x;
            pInfo->y = y;
            pParent = TreeView->Items->AddChildObject(pParent,Sr.Name,pInfo);

            DrawText(x,y,Sr.Name);
            x += 20;

            DirsCount++;
            SetCurrentDir(Sr.Name);

		    if((s_nIndex >= 0) && (s_nIndex < pFamillyPath->Count) && (pFamillyPath->Strings[s_nIndex] == Sr.Name))
            {
			    s_nIndex++;
            	SearchTree();
            }
            pParent = pParent->Parent;
            pInfo = (fm_Info *)pParent->Data;
            x = pInfo->x;
            y += 8;
            SetCurrentDir("..");
        }
        // End Recursion
        Err = FindNext(Sr);
    }

    // Show all
	CadUpdate(hDwg);
	CadExecute(hDwg,hVecWnd,CAD_CMD_ZOOM_EXT);
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::ParseFamillyPath(AnsiString strPath,TStrings *pList)
{
	pList->Clear();
    char c;
    AnsiString strText;
    strText = "";
    for(int i=1;i<=strPath.Length();i++)
    {
		c = strPath[i];
        if(c == '\\')
        {
        	if(strText != "")	pList->Add(strText);
			strText = "";
        }
        else
        if(i == strPath.Length())
        {
			strText += c;
        	if(strText != "")	pList->Add(strText);
        }
        else
        {
			strText += c;
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::FindFamillyTree(AnsiString strRootPath,AnsiString strPath,TStrings *pList)
{
	pList->Clear();
	TStrings *pPathList = new TStringList();
	ParseFamillyPath(strPath,pPathList);
    AnsiString strText,strCurrPath,strName;
    strCurrPath = strRootPath;
    for(int i=0;i<pPathList->Count;i++)
    {
    	if(i < pPathList->Count-1)
	    	strName = pPathList->Strings[i+1];
        else
	    	strName = "";
		strCurrPath = IncludeTrailingBackslash(strCurrPath+pPathList->Strings[i]);
    	strText = "";

    	SetCurrentDir(strCurrPath);
        TSearchRec	Sr;
        Err = FindFirst("*.*",faDirectory,Sr);
        while(!Err)
        {
            if((Sr.Attr & faDirectory) && (Sr.Name[1] != '.'))
            {
            	if(Sr.Name != strName)
                {
	                strText += Sr.Name+"   ,   ";
                }
            }
            Err = FindNext(Sr);
        }

        if(strText != "")
        {
	        strText = strName+"   /   "+strText;
    	    pList->Add(strText);
        }
    }
    if(pPathList)
    {
		delete pPathList;
        pPathList = NULL;
    }
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::LoadFamilly1Click(TObject *Sender)
{
	AnsiString Dir = strFamilliesPath;
  	if(!SelectDirectory(Dir,TSelectDirOpts()<<sdAllowCreate<<sdPerformCreate<<sdPrompt,0))	return;
    Caption = strCaption+" [ "+Dir+" ]";

    strBaseDirName = IncludeTrailingBackslash(Dir);
    AnsiString strRootPath = ExtractFilePath(ExcludeTrailingBackslash(Dir));

    FindFamillyTree(strRootPath,EditFamillyPath->Text,pFamillyPath);
    pFamillyPath->SaveToFile("c:\\test.txt");

    ParseFamillyPath(EditFamillyPath->Text,pFamillyPath);
//    pFamillyPath->SaveToFile("c:\\test.txt");

    ClearDrawing();
    Dir = ExtractFileName(ExcludeTrailingBackslash(Dir));
    x = 0;
    y = 0;
    s_nIndex = 0;

    TreeView->Items->Clear();
//    pParent = TreeView->Items->Add(NULL,Dir);

    fm_Info *pInfo = new fm_Info();
    pInfo->strName = strBaseDirName;
    pInfo->x = x;
    pInfo->y = y;
    pParent = TreeView->Items->AddChildObject(NULL,Dir,pInfo);

    DirsCount = 0;
    if(pFamillyPath->Strings[s_nIndex] == Dir)
    {
	    s_nIndex++;
    	SetCurrentDir(strBaseDirName);
	    Continue = true;
    	SearchTree();
    }
//    TreeView->FullCollapse();
//    TreeView->FullExpand();
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::SaveFamilly1Click(TObject *Sender)
{
	AnsiString Dir = strFamilliesPath;
  	if(!SelectDirectory(Dir,TSelectDirOpts()<<sdAllowCreate<<sdPerformCreate<<sdPrompt,0))	return;
    Dir = IncludeTrailingBackslash(Dir);

    fm_Info *pInfo;
	TTreeNode *CurItem = TreeView->Items->GetFirstNode();
	while(CurItem)
	{
    	pInfo = (fm_Info *)CurItem->Data;
        if(pInfo != NULL)
        {
			ForceDirectories(Dir+pInfo->strName);
        }
  		CurItem = CurItem->GetNext();
	}
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::ClearDrawing()
{
	CadSelectEntity(hDwg,NULL,true);
	CadSelErase(hDwg);
    CadUpdate(hDwg);

    VHANDLE hTextStyle;
    hTextStyle = CadGetCurTStyle(hDwg);
//    CadTStylePutFont(hTextStyle,"courier.vcf");
    CadTStylePutFont(hTextStyle,"Traditional Arabic.vcf");
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::NewDocument(AnsiString strFileName)
{
    hDwg = CadCreate();
    hVecWnd = CadWndCreate(hDwg,PanelDisplay->Handle,CAD_WS_DEFAULT,0,0,100,100);
  	CadExecute(hDwg,hVecWnd,CAD_CMD_SETFOCUS);
    
	PanelDisplayResize(NULL);

//    VHANDLE hPage = CadAddPage(hDwg,"Drawing",CAD_PAPER_USER,
//    							CAD_PAPER_ALBUM,g_dMachineWidthMM,
//                                g_dMachineHeightMM);
//	CadSetCurPage(hDwg,hPage);
//  	VHANDLE hModelPage = CadGetPageByName(hDwg,"Model");
//  	if(hModelPage)
//    {
//    	CadDeletePage(hDwg,hModelPage);
//  	}
//  	CadRegen(hDwg);
//  	CadExecute(hDwg,hVecWnd,CAD_CMD_ZOOM_EXT);

    // Machine Head
//    g_hMachineHeadIndex = CadAddRect(hDwg,0,0,0,m_dWorkingAreaWidthMM/20.0, m_dWorkingAreaHeightMM/20.0,0);
//    g_hMachineHeadIndex = CadAddRect(hDwg,0,0,0,20.0, 20.0,0);
//	CadEntityPutColor(g_hMachineHeadIndex,1);

//	CadLayerPutLock(CadGetCurLayer(hDwg),true);

	ClearDrawing();
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::FormCreate(TObject *Sender)
{
	strCaption = Caption;
    strAppPath = IncludeTrailingBackslash(ExtractFilePath(Application->ExeName));
    strFamilliesPath = strAppPath+"Families";

    pFamillyPath = new TStringList();

    bIsFirst = true;

	m_strAppTitle = Caption;
   
    // register you copy of VeCAD
  	CadRegistration( 1950125322 );

  	// define events procedures

    CadOnEventEntCreate(EntCreateProc);
    CadOnEventEntCopy(EntCopyProc);
    CadOnEventEntSelect(EntSelectProc);
    CadOnEventEntErase(EntEraseProc);
    CadOnEventEntChange(EntChangeProc);
    CadOnEventPrompt(PromptProc);
    CadOnEventMouseMove(MouseMoveProc);
    CadOnEventMouseDown(MouseDownProc);
	CadOnEventMouseDblClk(MouseDblClkProc);
    CadOnEventFontReplace(FontReplaceProc);
    CadOnEventSelCodepage(SelCodepageProc);
    CadOnEventExecute(ExecuteProc);
    CadOnEventCmdStart(CmdStartProc);
    CadOnEventCmdLBDown(CmdLBDownProc);
    CadOnEventCmdFinish(CmdFinishProc);
    CadOnEventRegen(RegenProc);
    CadOnEventPaint(PaintProc);

	// Custom entity events
  	CadOnEventCEntGetName(EventCEntGetName);
  	CadOnEventCEntGetExts(EventCEntGetExts);
  	CadOnEventCEntNumGrips(EventCEntNumGrips);
  	CadOnEventCEntGetGrip(EventCEntGetGrip);
  	CadOnEventCEntMoveGrip(EventCEntMoveGrip);
  	CadOnEventCEntMove(EventCEntMove);
  	CadOnEventCEntRotate(EventCEntRotate);
  	CadOnEventCEntScale(EventCEntScale);
  	CadOnEventCEntMirror(EventCEntMirror);
  	CadOnEventCEntDisplay(EventCEntDisplay);

    // set default assignment for accelerator keys
    CadAccelSetDefault();

    // set VeCAD properties
    CadSetShowCross(CAD_TRUE);
	CadSetMsgTitle("FamillyTree");

/*
    TheEnd=false;

    // Register your copy of Vecad.dll
    vlRegistration(ALISOFT_REG_CODE_5_2);

    // Set message handler function
    vlSetMsgHandler(DwgProc);

    // Create VeCAD StatusBar
    vlStatBarCreate( PanelDisplay->Handle, &SBarH );

    // Create VeCAD window, size will be set in OnSize()
    ghwVec = vlWndCreate( PanelDisplay->Handle, VL_WS_CHILD|VL_WS_SCROLL|VL_WS_BORDER, 0,0,400,300 );
    if(ghwVec)
    {
        ::PostMessage( PanelDisplay->Handle, WM_SIZE, 0, 0 );
        vlPropPut( VD_WND_EMPTYTEXT, (int)ghwVec,(c_strCompany+" "+m_strAppTitle).c_str());
    }
*/
    NewDocument("FamillyTree");
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::FormDestroy(TObject *Sender)
{
	if(pFamillyPath)
    {
		delete pFamillyPath;
        pFamillyPath = NULL;
    }

	CadDelete(hDwg);

    fm_Info *pInfo;
	TTreeNode *CurItem = TreeView->Items->GetFirstNode();
	while(CurItem)
	{
    	pInfo = (fm_Info *)CurItem->Data;
        if(pInfo != NULL)
        {
        	delete pInfo;
            pInfo = NULL;
        }
  		CurItem = CurItem->GetNext();
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::PanelDisplayResize(TObject *Sender)
{
    if(hVecWnd <= 0)	return;
    if(hDwg <= 0)	return;
	int W,H;
    // get size of the form window
    vuGetWindowSize(PanelDisplay->Handle,&W,&H);
    if((W > 0) && (H > 0))
    {
        CadWndResize(hVecWnd,0,0,W,H);

  		CadExecute(hDwg,hVecWnd,CAD_CMD_ZOOM_EXT);
    }
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::Exit1Click(TObject *Sender)
{
	Close();	
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
	switch(Key)
    {
		case VK_ESCAPE:
        {
        	Continue = false;
         	break;
        }
    }
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::Button1Click(TObject *Sender)
{
//	CadExecute(hDwg,hVecWnd,CAD_CMD_DRAW_TEXT);
	CadExecute(hDwg,hVecWnd,CAD_CMD_DRAW_MTEXT);
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::Button2Click(TObject *Sender)
{
	CadExecute(hDwg,hVecWnd,CAD_CMD_DRAW_LEADER);
}
//---------------------------------------------------------------------------

