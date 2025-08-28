//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
USERES("FamillyTree.res");
USEFORM("Main.cpp", FormMain);
USEUNIT("CadApi.cpp");
USEUNIT("cad_Events.cpp");
USEUNIT("ArabicLib.cpp");
USEFORM("Tree.cpp", FormTree);
//---------------------------------------------------------------------------
#include "cadapi.h"
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	try
	{
        // load CNC.dll
    	if(!CadLoadLibrary())
        {
            MessageDlg("CNC.DLL is not found",mtError,TMsgDlgButtons()<<mbOK,0);
            exit(0);
        }
        else
        {
			Application->Initialize();
			Application->CreateForm(__classid(TFormMain), &FormMain);
		Application->CreateForm(__classid(TFormTree), &FormTree);
		Application->Run();
        }
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	return 0;
}
//---------------------------------------------------------------------------
