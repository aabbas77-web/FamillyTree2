//---------------------------------------------------------------------------
#include <vcl.h>
#include <Math.h>
#include <Math.hpp>
#pragma hdrstop
//---------------------------------------------------------------------------
#include "cad_Events.h"
#ifdef _CNC_
	#include "Main.h"
	#include "Simulator.h"
	#include "Block.h"
	#include "Properties.h"
	#include "cnc_Messages.h"
#endif// _CNC_
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
AnsiString g_strUntitledFile = "Untitled";//GetMessage(75);// Untitled
//---------------------------------------------------------------------------
AnsiString g_strLastCommand = "";
//---------------------------------------------------------------------------
cnc_Units g_nUnit = cnc_MM;
cnc_AngleUnits g_nAngleUnit = cnc_Degree;
//---------------------------------------------------------------------------
double g_dDividingStep = 0.1;// mm
//---------------------------------------------------------------------------
bool g_bGenerateNamesList = true;
//---------------------------------------------------------------------------
AnsiString g_strFMask = "0.00";// "cm.mm,dm"
AnsiString g_strFAMask = "0.0";// "Angles"
//---------------------------------------------------------------------------
int g_nCurrToolIndex = 1;
//---------------------------------------------------------------------------
double g_dCurrDepth = 0.0;
//---------------------------------------------------------------------------
VHANDLE g_hMachineHeadIndex = -1;
double g_dMachineHeadWidthMM = 10.0;
double g_dMachineHeadHeightMM = 10.0;
//---------------------------------------------------------------------------
double g_dZStep = 1.0;
//---------------------------------------------------------------------------
double g_dAx = 1.0;
double g_dBx = 0.0;
double g_dAy = 1.0;
double g_dBy = 0.0;
//---------------------------------------------------------------------------
double g_dWorkingAreaX0MM = 0;// MM
double g_dWorkingAreaY0MM = 0;
double g_dWorkingAreaWidthMM = 1000;
double g_dWorkingAreaHeightMM = 1000;
//---------------------------------------------------------------------------
int g_nLanguageID = 0;
double g_dMachineWidthMM = 2000;
double g_dMachineHeightMM = 2000;
double g_dMinStepMM = 0.1;
//---------------------------------------------------------------------------
cnc_Tool g_pTools[c_nToolsCount];
int g_pDefaultToolsColors[c_nToolsCount] = {CAD_COLOR_RED,CAD_COLOR_YELLOW,CAD_COLOR_GREEN,CAD_COLOR_CYAN,CAD_COLOR_BLUE,CAD_COLOR_MAGENTA,CAD_COLOR_BROWN,CAD_COLOR_SALAD,CAD_COLOR_GRAY,CAD_COLOR_BLACK};
//---------------------------------------------------------------------------
//  : One dimension
// A: Angle dimension
//---------------------------------------------------------------------------
// dValue: always in "mm"
double cnc_OutC(double dValue)
{
	double dResult = dValue;
	switch(g_nUnit)
    {
     	case cnc_DM:
        {
        	dResult = dValue * 10.0;
         	break;
        }
     	case cnc_MM:
        {
        	dResult = dValue;
         	break;
        }
     	case cnc_CM:
        {
        	dResult = dValue / 10.0;
         	break;
        }
     	case cnc_M:
        {
        	dResult = dValue / 1000.0;
         	break;
        }
     	case cnc_KM:
        {
        	dResult = dValue / 1000000.0;
         	break;
        }
    }
    return dResult;
}
//---------------------------------------------------------------------------
// dValue: always in "mm"
AnsiString cnc_SOutC(double dValue)
{
    return FormatFloat(g_strFMask,cnc_OutC(dValue));
}
//---------------------------------------------------------------------------
// Without conversion
AnsiString cnc_SOut(double dValue)
{
    return FormatFloat(g_strFMask,dValue);
}
//---------------------------------------------------------------------------
// Return value: always in "mm"
double cnc_In(double dValue)
{
	double dResult = dValue;
	switch(g_nUnit)
    {
     	case cnc_DM:
        {
        	dResult = dValue / 10.0;
         	break;
        }
     	case cnc_MM:
        {
        	dResult = dValue;
         	break;
        }
     	case cnc_CM:
        {
        	dResult = dValue * 10.0;
         	break;
        }
     	case cnc_M:
        {
        	dResult = dValue * 1000.0;
         	break;
        }
     	case cnc_KM:
        {
        	dResult = dValue * 1000000.0;
         	break;
        }
    }
    return dResult;
}
//---------------------------------------------------------------------------
// Return value: always in "mm"
double cnc_SIn(AnsiString &strText)
{
    return cnc_In(StrToFloat(strText));
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// dValue: always in "radian"
double cnc_OutA(double dValue)
{
	double dResult = dValue;
	switch(g_nAngleUnit)
    {
     	case cnc_Radian:
        {
        	dResult = dValue;
         	break;
        }
     	case cnc_Degree:
        {
        	dResult = RadToDeg(dValue);
         	break;
        }
     	case cnc_Grad:
        {
        	dResult = RadToGrad(dValue);
         	break;
        }
     	case cnc_Cycle:
        {
        	dResult = RadToCycle(dValue);
         	break;
        }
    }
    return dResult;
}
//---------------------------------------------------------------------------
// dValue: always in "radian"
AnsiString cnc_SOutA(double dValue)
{
    return FormatFloat(g_strFAMask,cnc_OutA(dValue));
}
//---------------------------------------------------------------------------
// Return value: always in "radian"
double cnc_InA(double dValue)
{
	double dResult = dValue;
	switch(g_nAngleUnit)
    {
     	case cnc_Radian:
        {
        	dResult = dValue;
         	break;
        }
     	case cnc_Degree:
        {
        	dResult = DegToRad(dValue);
         	break;
        }
     	case cnc_Grad:
        {
        	dResult = GradToRad(dValue);
         	break;
        }
     	case cnc_Cycle:
        {
        	dResult = CycleToRad(dValue);
         	break;
        }
    }
    return dResult;
}
//---------------------------------------------------------------------------
// Return value: always in "radian"
double cnc_SInA(AnsiString &strText)
{
    return cnc_InA(StrToFloat(strText));
}
//---------------------------------------------------------------------------
//--------------------Extract Segments---------------------------------------
//---------------------------------------------------------------------------
void cad_ExtractSegments(VDWG hDwg,int hEntity,int nPath,int &nVertex,cad_Contour &Contour)
{
	int nPathSize;
	double x,y;
    shp_Point P;

    Contour.Points.clear();
    nPathSize = CadEntityGetPathSize(hEntity,nPath);
    for(int j=0;j<nPathSize;j++)
    {
        CadEntityGetVer(hEntity,nVertex++,&x,&y);
        P.x = x;
        P.y = y;
        Contour.Points.push_back(P);
    }

    cad_Contour ResContour;
	shp_FindBestContour(g_dEpsilon,Contour.Points,ResContour.Points);
    shp_Divide_Contour(g_dDividingStep,true,ResContour.Points,Contour.Points);
}
//---------------------------------------------------------------------------
void cad_ExtractSegments(VDWG hDwg,int hEntity,cad_Shape &Shape)
{
	int nLayer,nPaths;
	int nVertex;
    cad_Contour Contour;
    cad_Entity CadEntity;

    CadEntity.nToolIndex = stTool_1;
    CadEntity.dDepth = 0.0;
    if(CadEntityGetExDataSize(hEntity) > 0)
        CadEntityGetExData(hEntity,&CadEntity);
    else
    {
        CadEntityPutExData(hEntity,&CadEntity,sizeof(CadEntity));
    	CadPutDirty(hDwg,true);
    }

    Shape.cadEntity.nToolIndex = CadEntity.nToolIndex;
    Shape.cadEntity.dDepth = CadEntity.dDepth;

    Shape.Contours.clear();
    nVertex = 0;
    nPaths = CadEntityGetNumPaths(hEntity);
    for(int i=0;i<nPaths;i++)
    {
        cad_ExtractSegments(hDwg,hEntity,i,nVertex,Contour);
        if(!Contour.Points.empty())
        {
	        Shape.Contours.push_back(Contour);
        }
    }
}
//---------------------------------------------------------------------------
void cad_ExtractSegments(VDWG hDwg,int nLayer,bool bSelected,cad_Layer &Layer)
{
	int hEntity;
    cad_Shape Shape;

    Layer.Shapes.clear();
    hEntity = CadLayerGetFirstEntity(nLayer);
    while(hEntity)
    {
    	if(bSelected)
        {
			if(CadEntityGetSelected(hDwg,hEntity))
            {
		    	cad_ExtractSegments(hDwg,hEntity,Shape);
            	if(!Shape.Contours.empty())
    	        {
    			    Layer.Shapes.push_back(Shape);
                }
            }
        }
        else
        {
	    	cad_ExtractSegments(hDwg,hEntity,Shape);
            if(!Shape.Contours.empty())
            {
	    	    Layer.Shapes.push_back(Shape);
            }
        }
        hEntity = CadLayerGetNextEntity(nLayer,hEntity);
    }
}
//---------------------------------------------------------------------------
void cad_ExtractSegments_Ex(VDWG hDwg,int nLayer,bool bSelected,cad_Layer &Layer,int nToolIndex)
{
	int hEntity;
    cad_Shape Shape;
    cad_Entity CadEntity;

    hEntity = CadLayerGetFirstEntity(nLayer);
    while(hEntity)
    {
        CadEntity.nToolIndex = stTool_1;
        CadEntity.dDepth = 0.0;
	    if(CadEntityGetExDataSize(hEntity) > 0)
    		CadEntityGetExData(hEntity,&CadEntity);
        else
        {
        	CadEntityPutExData(hEntity,&CadEntity,sizeof(CadEntity));
    		CadPutDirty(hDwg,true);
        }

        if(CadEntity.nToolIndex == nToolIndex)
        {
            if(bSelected)
            {
                if(CadEntityGetSelected(hDwg,hEntity))
                {
                    cad_ExtractSegments(hDwg,hEntity,Shape);
                    if(!Shape.Contours.empty())
                    {
                        Layer.Shapes.push_back(Shape);
                    }
                }
            }
            else
            {
                cad_ExtractSegments(hDwg,hEntity,Shape);
                if(!Shape.Contours.empty())
                {
                    Layer.Shapes.push_back(Shape);
                }
            }
        }
        hEntity = CadLayerGetNextEntity(nLayer,hEntity);
    }
}
//---------------------------------------------------------------------------
void cad_ExtractSegments(VDWG hDwg,bool bSelected,cad_Drawing &Drawing)
{
	int nLayer;
	cad_Layer Layer;

    Drawing.Layers.clear();
	nLayer = CadGetFirstLayer(hDwg);
	while(nLayer)
    {
    	cad_ExtractSegments(hDwg,nLayer,bSelected,Layer);
        if(!Layer.Shapes.empty())
        {
	        Drawing.Layers.push_back(Layer);
        }
  		nLayer = CadGetNextLayer(hDwg,nLayer);
	}
}
//---------------------------------------------------------------------------
void cad_ExtractSegments_Ex(VDWG hDwg,bool bSelected,cad_Drawing &Drawing)
{
	int nLayer;
	cad_Layer Layer;

    Drawing.Layers.clear();
    for(int i=0;i<c_nToolsCount;i++)
    {
    	Layer.Shapes.clear();

        nLayer = CadGetFirstLayer(hDwg);
        while(nLayer)
        {
            cad_ExtractSegments_Ex(hDwg,nLayer,bSelected,Layer,i);
            nLayer = CadGetNextLayer(hDwg,nLayer);
        }

        if(!Layer.Shapes.empty())
        {
            Drawing.Layers.push_back(Layer);
        }
    }
}
//---------------------------------------------------------------------------
//--------------------Draw Segments------------------------------------------
//---------------------------------------------------------------------------
void cad_DrawSegments(VDWG hDwg,cad_Contour &Contour)
{
	if(Contour.Points.size() < 2)	return;
	int hEntity;
    shp_Point P;
    CadClearVertices();
    for(UINT i=0;i<Contour.Points.size();i++)
	{
    	P = Contour.Points[i];
        CadAddVertex(P.x,P.y,0.0);
    }
    hEntity = CadAddPolyline(hDwg,CAD_PLINE_LINEAR,false);
    EntCreateProc(hDwg,hEntity);
}
//---------------------------------------------------------------------------
void cad_DrawSegments(VDWG hDwg,cad_Shape &Shape)
{
	g_nCurrToolIndex = Shape.cadEntity.nToolIndex;
    g_dCurrDepth = Shape.cadEntity.dDepth;
    for(UINT i=0;i<Shape.Contours.size();i++)
	{
    	cad_DrawSegments(hDwg,Shape.Contours[i]);
    }
}
//---------------------------------------------------------------------------
void cad_DrawSegments(VDWG hDwg,cad_Layer &Layer)
{
    for(UINT i=0;i<Layer.Shapes.size();i++)
	{
    	cad_DrawSegments(hDwg,Layer.Shapes[i]);
    }
}
//---------------------------------------------------------------------------
void cad_DrawSegments(VDWG hDwg,cad_Drawing &Drawing)
{
    for(UINT i=0;i<Drawing.Layers.size();i++)
	{
    	cad_DrawSegments(hDwg,Drawing.Layers[i]);
    }
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool cnc_Load_PLT(AnsiString strFileName,cad_Drawing &Drawing)
{
	Drawing.Layers.clear();
	TStringList *pList = new TStringList();
	pList->LoadFromFile(strFileName);
    AnsiString strLine;
    bool bIsCorel = false;
    int nPos;
    shp_Point P;
    cad_Contour Contour;
    cad_Shape Shape;
    cad_Layer Layer;

    Shape.cadEntity.nToolIndex = stTool_1;
    Shape.cadEntity.dDepth = 0.0;
    for(int i=0;i<pList->Count;i++)
    {
		strLine = pList->Strings[i];
        if(UpperCase(strLine).Pos("PU") > 0)
        {
    		if(!Contour.Points.empty())
            {
	        	Shape.Contours.push_back(Contour);
    	    	Contour.Points.clear();
            }

			strLine.Delete(1,2);
			nPos = strLine.Pos(",");
            if(nPos <= 0)// Corel PLT
            {
				nPos = strLine.Pos(" ");
                bIsCorel = true;
            }

            P.x = StrToInt(strLine.SubString(1,nPos-1).Trim());
            strLine.Delete(1,nPos);
            strLine.Trim();

            if(bIsCorel)// 2D Coordinates
            {
                strLine.Delete(strLine.Length(),1);
                P.y = StrToInt(strLine);
            	Contour.Points.push_back(P);
                continue;
            }
            else
            {
                nPos = strLine.Pos(",");
                if(nPos <= 0)// 2D Coordinates
                {
                    strLine.Delete(strLine.Length(),1);
                    P.y = StrToInt(strLine);
            		Contour.Points.push_back(P);
                    continue;
                }
            }
            P.y = StrToInt(strLine.SubString(1,nPos-1).Trim());
            strLine.Delete(1,nPos);
            strLine.Trim();

            Contour.Points.push_back(P);
        }
        else
/*
        if(UpperCase(strLine).Pos("GZ") > 0)
        {
    		if(!Contour.Points.empty())
            {
	        	Shape.Contours.push_back(Contour);
    	    	Contour.Points.clear();
            }
        }
        else
*/        
        if(UpperCase(strLine).Pos("PD") > 0)
        {
			strLine.Delete(1,2);
			nPos = strLine.Pos(",");
            if(nPos <= 0)// Corel PLT
            {
				nPos = strLine.Pos(" ");
                bIsCorel = true;
            }

            P.x = StrToInt(strLine.SubString(1,nPos-1).Trim());
            strLine.Delete(1,nPos);
            strLine.Trim();

            if(bIsCorel)// 2D Coordinates
            {
                strLine.Delete(strLine.Length(),1);
                P.y = StrToInt(strLine);
            	Contour.Points.push_back(P);
                continue;
            }
            else
            {
                nPos = strLine.Pos(",");
                if(nPos <= 0)// 2D Coordinates
                {
                    strLine.Delete(strLine.Length(),1);
                    P.y = StrToInt(strLine);
            		Contour.Points.push_back(P);
                    continue;
                }
            }
            P.y = StrToInt(strLine.SubString(1,nPos-1).Trim());
            strLine.Delete(1,nPos);
            strLine.Trim();

            Contour.Points.push_back(P);
        }
        else
        if(UpperCase(strLine).Pos("SP") > 0)
        {
    		if(!Contour.Points.empty())
            {
	        	Shape.Contours.push_back(Contour);
    	    	Contour.Points.clear();
            }

        	if(!Shape.Contours.empty())
            {
    			Layer.Shapes.push_back(Shape);
                Shape.Contours.clear();
            }

			strLine.Delete(1,2);
			nPos = strLine.Pos(";");
//			nPos = strLine.Pos(" ");
            if(nPos >= 0)
            {
//        		Shape.cadEntity.nToolIndex = StrToInt(strLine.SubString(1,nPos-1).Trim())+stTool_1;
        		Shape.cadEntity.nToolIndex = StrToInt(strLine.SubString(1,nPos-1).Trim());
            }
        }
/*
        else
        if(UpperCase(strLine).Pos("EF") > 0)
        {
    		if(!Contour.Points.empty())
            {
	        	Shape.Contours.push_back(Contour);
    	    	Contour.Points.clear();
            }

        	if(!Shape.Contours.empty())
            {
    			Layer.Shapes.push_back(Shape);
                Shape.Contours.clear();
            }
        }
        else
        {
         	continue;
        }
*/        
    }

    if(!Contour.Points.empty())
    {
        Shape.Contours.push_back(Contour);
        Contour.Points.clear();
    }
    if(!Shape.Contours.empty())
    {
        Layer.Shapes.push_back(Shape);
        Shape.Contours.clear();
    }
    Drawing.Layers.push_back(Layer);
    if(pList)
    {
     	delete pList;
        pList = NULL;
    }
    return true;
}
//---------------------------------------------------------------------------
void cnc_Affine_Drawing(cad_Drawing &Drawing,double ax,double bx,double ay,double by)
{
    shp_Point P,NP;
    cad_Contour Contour;
    cad_Shape Shape;
    cad_Layer Layer;
    for(UINT i=0;i<Drawing.Layers.size();i++)
    {
    	Layer = Drawing.Layers[i];
        if(Layer.Shapes.empty())	continue;
    	for(UINT j=0;j<Layer.Shapes.size();j++)
        {
        	Shape = Layer.Shapes[j];
            if(Shape.Contours.empty())	continue;
            for(UINT k=0;k<Shape.Contours.size();k++)
            {
                Contour = Shape.Contours[k];
                for(UINT m=0;m<Contour.Points.size();m++)
                {
                    P = Contour.Points[m];
                    NP.x = ax*P.x+bx;
                    NP.y = ay*P.y+by;
                    Drawing.Layers[i].Shapes[j].Contours[k].Points[m] = NP;
                }
            }
        }
    }
}
//---------------------------------------------------------------------------
bool cnc_Save_CMD(AnsiString strFileName,cad_Drawing &Drawing)
{
	TStringList *pList = new TStringList();
    AnsiString strLine;
    shp_Point P;
    double x0,y0,z;
    cad_Contour Contour;
    cad_Shape Shape;
    cad_Layer Layer;

    pList->Add("IN;");
    pList->Add("PU 0,0,0;");
    pList->Add("GZ;");// Goto zero
    for(UINT i=0;i<Drawing.Layers.size();i++)
    {
    	Layer = Drawing.Layers[i];
        if(Layer.Shapes.empty())	continue;
    	for(UINT j=0;j<Layer.Shapes.size();j++)
        {
        	Shape = Layer.Shapes[j];
            if(Shape.Contours.empty())	continue;
            if(Shape.Contours[0].Points.empty())	continue;

            P = Shape.Contours[0].Points[0];
            P.x *= 10.0;
            P.y *= 10.0;

            g_dZStep = g_pTools[Shape.cadEntity.nToolIndex-stTool_1].dZStep;
//            x0 = g_pTools[Shape.cadEntity.nToolIndex-stTool_1].x0;
//            y0 = g_pTools[Shape.cadEntity.nToolIndex-stTool_1].y0;
            z = g_dZStep*10.0;

            int nZCount;
            try
            {
	            if(Shape.cadEntity.dDepth < g_dZStep)	Shape.cadEntity.dDepth = g_dZStep;
    	        nZCount = Shape.cadEntity.dDepth / g_dZStep;
            }
            catch(...)
            {
				nZCount = 1;
            }

			strLine = "PD "+IntToStr((int)P.x)+","+IntToStr((int)P.y)+","+IntToStr((int)z)+";";
            pList->Add(strLine);
//			strLine = "PD "+IntToStr((int)(P.x-x0))+","+IntToStr((int)(P.y-y0))+","+IntToStr((int)z)+";";
//            pList->Add(strLine);
			strLine = FormatFloat("SP0 ",Shape.cadEntity.nToolIndex)+IntToStr((int)P.x)+","+IntToStr((int)P.y)+","+IntToStr((int)(Shape.cadEntity.dDepth*10.0))+";";
            pList->Add(strLine);// Select pen
//    		pList->Add(FormatFloat("SP0",Shape.cadEntity.nToolIndex)+";");// Select pen
            for(int n=0;n<nZCount;n++)
            {
            	if(n % 2 == 0)
                {
                    for(UINT k=0;k<Shape.Contours.size();k++)
                    {
                        Contour = Shape.Contours[k];
                        for(UINT m=0;m<Contour.Points.size();m++)
                        {
//                        	if((n == 0) && (m == 0))	continue;// don`t remove this point for closing curve correctly
                            P = Contour.Points[m];
                            P.x *= 10.0;
                            P.y *= 10.0;
                            strLine = "PD "+IntToStr((int)P.x)+","+IntToStr((int)P.y)+","+IntToStr((int)z)+";";
                            pList->Add(strLine);
                        }
                    }
                }
                else
                {
                    for(int k=Shape.Contours.size()-1;k>=0;k--)
                    {
                        Contour = Shape.Contours[k];
                        for(int m=Contour.Points.size()-1-1;m>=0+1;m--)// Remove repeated points
//                        for(int m=Contour.Points.size()-1;m>=0;m--)// Remove repeated points
                        {
                            P = Contour.Points[m];
                            P.x *= 10.0;
                            P.y *= 10.0;
                            strLine = "PD "+IntToStr((int)P.x)+","+IntToStr((int)P.y)+","+IntToStr((int)z)+";";
                            pList->Add(strLine);
                        }
                    }
                }
                z += g_dZStep*10.0; 
            }
        	pList->Add("PU 0,0,0;");
        }
    }
    pList->Add("GZ;");// Goto zero
    pList->Add("EF;");// EOF
    if(pList)
    {
		pList->SaveToFile(strFileName);
     	delete pList;
        pList = NULL;
    }
    return true;
}
//---------------------------------------------------------------------------
bool cnc_Save_CMD_Ex(AnsiString strFileName,cad_Drawing &Drawing)
{
	TStringList *pList = new TStringList();
    AnsiString strLine;
    shp_Point P;
    double z;
    cad_Contour Contour;
    cad_Shape Shape;
    cad_Layer Layer;
    int nPrevToolIndex = -1;

    pList->Add("IN;");
    pList->Add("PU 0,0,0;");
    pList->Add("GZ;");// Goto zero
    for(UINT i=0;i<Drawing.Layers.size();i++)
    {
    	Layer = Drawing.Layers[i];
        if(Layer.Shapes.empty())	continue;
    	for(UINT j=0;j<Layer.Shapes.size();j++)
        {
        	Shape = Layer.Shapes[j];
            if(Shape.Contours.empty())	continue;
            if(Shape.Contours[0].Points.empty())	continue;

            P = Shape.Contours[0].Points[0];
            P.x *= 10.0;
            P.y *= 10.0;

            g_dZStep = g_pTools[Shape.cadEntity.nToolIndex-stTool_1].dZStep;
            z = g_dZStep*10.0;
            
			strLine = "PD "+IntToStr((int)P.x)+","+IntToStr((int)P.y)+","+IntToStr((int)z)+";";
            pList->Add(strLine);

            if(Shape.cadEntity.nToolIndex != nPrevToolIndex)
            {
				strLine = FormatFloat("SP0 ",Shape.cadEntity.nToolIndex)+IntToStr((int)P.x)+","+IntToStr((int)P.y)+","+IntToStr((int)z)+";";
	    		pList->Add(strLine);// Select pen
//	    		pList->Add(FormatFloat("SP0",Shape.cadEntity.nToolIndex)+";");// Select pen
            	nPrevToolIndex = Shape.cadEntity.nToolIndex;
            }

            int nZCount;
            try
            {
	            if(Shape.cadEntity.dDepth < g_dZStep)	Shape.cadEntity.dDepth = g_dZStep;
    	        nZCount = Shape.cadEntity.dDepth / g_dZStep;
            }
            catch(...)
            {
				nZCount = 1;
            }
            for(int n=0;n<nZCount;n++)
            {
            	if(n % 2 == 0)
                {
                    for(UINT k=0;k<Shape.Contours.size();k++)
                    {
                        Contour = Shape.Contours[k];
                        for(UINT m=0;m<Contour.Points.size();m++)
                        {
//                        	if((n == 0) && (m == 0))	continue;// don`t remove this point for closing curve correctly
                            P = Contour.Points[m];
                            P.x *= 10.0;
                            P.y *= 10.0;
                            strLine = "PD "+IntToStr((int)P.x)+","+IntToStr((int)P.y)+","+IntToStr((int)z)+";";
                            pList->Add(strLine);
                        }
                    }
                }
                else
                {
                    for(int k=Shape.Contours.size()-1;k>=0;k--)
                    {
                        Contour = Shape.Contours[k];
                        for(int m=Contour.Points.size()-1-1;m>=0+1;m--)// Remove repeated points
//                        for(int m=Contour.Points.size()-1;m>=0;m--)// Remove repeated points
                        {
                            P = Contour.Points[m];
                            P.x *= 10.0;
                            P.y *= 10.0;
                            strLine = "PD "+IntToStr((int)P.x)+","+IntToStr((int)P.y)+","+IntToStr((int)z)+";";
                            pList->Add(strLine);
                        }
                    }
                }
                z += g_dZStep*10.0; 
            }
        	pList->Add("PU 0,0,0;");
        }
    }
    pList->Add("GZ;");// Goto zero
    pList->Add("EF;");// EOF
    if(pList)
    {
		pList->SaveToFile(strFileName);
     	delete pList;
        pList = NULL;
    }
    return true;
}
//---------------------------------------------------------------------------
bool cnc_Save_PLT(AnsiString strFileName,cad_Drawing &Drawing)
{
	TStringList *pList = new TStringList();
    AnsiString strLine;
    shp_Point P;
    cad_Contour Contour;
    cad_Shape Shape;
    cad_Layer Layer;
    bool bFirst;

    pList->Add("IN;");
    pList->Add("PU 0,0;");
    for(UINT i=0;i<Drawing.Layers.size();i++)
    {
    	Layer = Drawing.Layers[i];
        if(Layer.Shapes.empty())	continue;
    	for(UINT j=0;j<Layer.Shapes.size();j++)
        {
        	Shape = Layer.Shapes[j];
        	if(Shape.Contours.empty())	continue;
    		pList->Add(FormatFloat("SP0",Shape.cadEntity.nToolIndex)+";");// Select pen
         	for(UINT k=0;k<Shape.Contours.size();k++)
            {
            	Contour = Shape.Contours[k];
                bFirst = true;
                for(UINT m=0;m<Contour.Points.size();m++)
                {
                 	P = Contour.Points[m];
			        if(bFirst)
                    {
        				bFirst = false;
				        strLine = "PU "+IntToStr((int)P.x)+","+IntToStr((int)P.y)+";";
    	    			pList->Add(strLine);
                    }
			        else
                    {
			            strLine = "PD "+IntToStr((int)P.x)+","+IntToStr((int)P.y)+";";
        				pList->Add(strLine);
                    }
                }
            }
        }
    }
    pList->Add("PU 0,0;");
    if(pList)
    {
		pList->SaveToFile(strFileName);
     	delete pList;
        pList = NULL;
    }
    return true;
}
//---------------------------------------------------------------------------

bool cad_Export_PLT(VDWG hDwg,AnsiString strFileName)
{
    cad_Drawing Drawing;
	cad_ExtractSegments(hDwg,false,Drawing);
    double Ax,Bx,Ay,By;
    Ax = 1.0/g_dAx;
    Bx = -g_dBx/g_dAx;
    Ay = 1.0/g_dAy;
    By = -g_dBy/g_dAy;
    cnc_Affine_Drawing(Drawing,Ax,Bx,Ay,By);
    return cnc_Save_PLT(strFileName,Drawing);
}
//---------------------------------------------------------------------------

bool cad_Export_CMD(VDWG hDwg,AnsiString strFileName)
{
    cad_Drawing Drawing;
	cad_ExtractSegments(hDwg,false,Drawing);
    return cnc_Save_CMD(strFileName,Drawing);
}
//---------------------------------------------------------------------------

bool cad_Export_CMD_Ex(VDWG hDwg,AnsiString strFileName)
{
    cad_Drawing Drawing;
	cad_ExtractSegments_Ex(hDwg,false,Drawing);
    return cnc_Save_CMD(strFileName,Drawing);
//    return cnc_Save_CMD_Ex(strFileName,Drawing);
}
//---------------------------------------------------------------------------

bool cad_Import_PLT(VDWG hDwg,AnsiString strFileName)
{
	cad_Drawing Drawing;
    if(!cnc_Load_PLT(strFileName,Drawing))	return false;
    cnc_Affine_Drawing(Drawing,g_dAx,g_dBx,g_dAy,g_dBy);
	cad_DrawSegments(hDwg,Drawing);
    CadUpdate(hDwg);
    return true;
}
//---------------------------------------------------------------------------
void cad_SelectJoinedEntities(VDWG hDwg)
{
	CadSelClear(hDwg);
	VHANDLE hL,hE;
	int nEntityType;
	hL = CadGetFirstLayer(hDwg);
	while(hL)
    {
	  	hE = CadLayerGetFirstEntity(hL);
	  	while(hE)
        {
            nEntityType = CadEntityGetType(hE);
            switch(nEntityType)
            {
                case CAD_ENT_LINE:
                {
    				CadSelectEntity(hDwg,hE,true);
                    break;
                }
        		case CAD_ENT_POLYLINE:
                {
                    int FitType = CadPlineGetFit(hE);
                    switch(FitType)
                    {
                        case CAD_PLINE_LINEAR:
                        {
    						CadSelectEntity(hDwg,hE,true);
                            break;
                        }
                    }
                    break;
                }
/*
                case CAD_ENT_ARC:
                {
                    int et = CadArcGetType(hE);
                    switch(et)
                    {
                        case CAD_ARC:
                        {
    						CadSelectEntity(hDwg,hE,true);
                            break;
                        }
                    }
                 	break;
                }
*/                
            }
    		hE = CadLayerGetNextEntity(hL,hE);
  		}
  		hL = CadGetNextLayer(hDwg,hL);
	}
}
//---------------------------------------------------------------------------
//-------------------Custom Commands-----------------------------------------
//---------------------------------------------------------------------------
/*
// Square
void _stdcall Func_SQUARE(VDWG hDwg)
{
	char szText[256];
  	int status;
  	double x1,y1,z1;
  	double x2,y2,z2;

m1:
	// Input first corner:
  	CadCmdPrompt(hDwg,GetMessage(52).c_str(),"");
  	status = CadCmdUserInput(hDwg);
  	switch(status)
    {
    	case CAD_CCS_LBUTTON:
        {
            // user have clicked left mouse button inside of VeCAD Window
            CadCmdGetInputPoint(hDwg,&x1,&y1,&z1);
            // save the entered point in a command buffer, in order to read it in the "Drag" function
            CadCmdAddPoint(hDwg,x1,y1,z1);
            // set this point as a base for relative coordinates, which can be
            // entered in a command line for the second point (for example: @10,20)
            CadCmdSetBasePoint(hDwg,x1,y1,z1);
            break;
        }
    	case CAD_CCS_STRING:
        {
            // user have input coordinates value in a command line
            CadCmdGetInputStr(hDwg,szText);
            // convert the string into point coordinates
            if(CadCmdStrToPoint(hDwg,szText,&x1,&y1,&z1))
            {
                // save the entered point in a command buffer, in order to read it in the "Drag" function
                CadCmdAddPoint(hDwg,x1,y1,z1);
                // set this point as a base for relative coordinates, which can be
                // entered in a command line for the second point (for example: @10,20)
                CadCmdSetBasePoint(hDwg,x1,y1,z1);
            }
            else
            {
                // Invalid point
                CadCmdPrompt(hDwg,GetMessage(53).c_str(),"");
                // repeat the input
                goto m1;
            }
            break;
        }
    	case CAD_CCS_RBUTTON:   // user have clicked right mouse button
    	case CAD_CCS_ESCAPE:    // user have pressed <Esc> kay
    	default:
        {
      		// finish the command
      		return;
        }
  	}

m2:
	// Input second corner:
  	CadCmdPrompt(hDwg,GetMessage(54).c_str(),"");
  	status = CadCmdUserInput(hDwg);
  	switch(status)
    {
    	case CAD_CCS_LBUTTON:
        {
            // user have clicked left mouse button inside of VeCAD Window
            CadCmdGetInputPoint(hDwg,&x2,&y2,&z2);
            // save the entered point in a command buffer, in order to read it in the "Drag" function
            CadCmdAddPoint(hDwg,x2,y2,z2);
            // set this point as a base for relative coordinates, which can be
            // entered in a command line for the second point (for example: @10,20)
            CadCmdSetBasePoint(hDwg,x2,y2,z2);
      		break;
        }
    	case CAD_CCS_STRING:
        {
            // user have input coordinates value in a command line
            CadCmdGetInputStr(hDwg,szText);
            // convert the string into point coordinates
            if(CadCmdStrToPoint(hDwg,szText,&x2,&y2,&z2))
            {
                // save the entered point in a command buffer, in order to read it in the "Drag" function
                CadCmdAddPoint(hDwg,x2,y2,z2);
                // set this point as a base for relative coordinates, which can be
                // entered in a command line for the second point (for example: @10,20)
                CadCmdSetBasePoint(hDwg,x2,y2,z2);
            }
            else
            {
                // invalid point
                CadCmdPrompt(hDwg,GetMessage(53).c_str(),"");
                // repeat the input
                goto m2;
            }
      		break;
        }
    	case CAD_CCS_RBUTTON:
    	case CAD_CCS_ESCAPE:
    	default:
        {
      		// finish the command
      		return;
        }
  	}

  	// create a polyline entity
	cad_Square data;
    double dW;
    dW = sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));

	data.x = x1;
	data.y = y1;
	data.z = z1;
	data.Size = 2*dW;
    data.RotAngle = vuGetAngle(x1,y1,z1,x2,y2,z2,CAD_PLANE_XY);
	cad_AddSquare(hDwg,data);

  	CadUpdate(hDwg);
    cad_ShowText(1,"");
}
//---------------------------------------------------------------------------
void _stdcall Drag_SQUARE(VDWG hDwg,int nPoints,double x,double y,double z)
{
  	static double x1,y1,z1;
    double dW;
	cad_Square data;
    switch(nPoints)
    {
  		case 1:
        {
            // one point is already entered, get it from a command buffer
            CadCmdGetPoint(hDwg,0,&x1,&y1,&z1);

            // draw temporary rectangle
            dW = sqrt((x-x1)*(x-x1)+(y-y1)*(y-y1));

            data.x = x1;
            data.y = y1;
            data.z = z1;
            data.Size = 2*dW;
    		data.RotAngle = vuGetAngle(x1,y1,z1,x,y,z,CAD_PLANE_XY);
            cad_DrawSquare(data,false);

            CadDrawLine(x1,y1,z1,x,y,z);
            // Size // Rotation
            cad_ShowText(1,GetMessage(27)+": "+cnc_SOutC(data.Size)+" , "+GetMessage(23)+": "+cnc_SOutA(data.RotAngle));
            break;
        }
    }
}
//---------------------------------------------------------------------------
// Vertical line
void _stdcall Func_VLine(VDWG hDwg)
{
	char szText[256];
  	int status;
  	double x1,y1,z1;
  	double x2,y2,z2;

m1:
	// Input first point:
  	CadCmdPrompt(hDwg,GetMessage(52).c_str(),"");
  	status = CadCmdUserInput(hDwg);
  	switch(status)
    {
    	case CAD_CCS_LBUTTON:
        {
            // user have clicked left mouse button inside of VeCAD Window
            CadCmdGetInputPoint(hDwg,&x1,&y1,&z1);
            // save the entered point in a command buffer, in order to read it in the "Drag" function
            CadCmdAddPoint(hDwg,x1,y1,z1);
            // set this point as a base for relative coordinates, which can be
            // entered in a command line for the second point (for example: @10,20)
            CadCmdSetBasePoint(hDwg,x1,y1,z1);
            break;
        }
    	case CAD_CCS_STRING:
        {
            // user have input coordinates value in a command line
            CadCmdGetInputStr(hDwg,szText);
            // convert the string into point coordinates
            if(CadCmdStrToPoint(hDwg,szText,&x1,&y1,&z1))
            {
                // save the entered point in a command buffer, in order to read it in the "Drag" function
                CadCmdAddPoint(hDwg,x1,y1,z1);
                // set this point as a base for relative coordinates, which can be
                // entered in a command line for the second point (for example: @10,20)
                CadCmdSetBasePoint(hDwg,x1,y1,z1);
            }
            else
            {
                // invalid point
                CadCmdPrompt(hDwg,GetMessage(53).c_str(),"");
                // repeat the input
                goto m1;
            }
            break;
        }
    	case CAD_CCS_RBUTTON:   // user have clicked right mouse button
    	case CAD_CCS_ESCAPE:    // user have pressed <Esc> kay
    	default:
        {
      		// finish the command
      		return;
        }
  	}

m2:
	// Input second point:
  	CadCmdPrompt(hDwg,GetMessage(54).c_str(),"");
  	status = CadCmdUserInput(hDwg);
  	switch(status)
    {
    	case CAD_CCS_LBUTTON:
        {
            // user have clicked left mouse button inside of VeCAD Window
            CadCmdGetInputPoint(hDwg,&x2,&y2,&z2);
            // save the entered point in a command buffer, in order to read it in the "Drag" function
            CadCmdAddPoint(hDwg,x2,y2,z2);
            // set this point as a base for relative coordinates, which can be
            // entered in a command line for the second point (for example: @10,20)
            CadCmdSetBasePoint(hDwg,x2,y2,z2);
      		break;
        }
    	case CAD_CCS_STRING:
        {
            // user have input coordinates value in a command line
            CadCmdGetInputStr(hDwg,szText);
            // convert the string into point coordinates
            if(CadCmdStrToPoint(hDwg,szText,&x2,&y2,&z2))
            {
                // save the entered point in a command buffer, in order to read it in the "Drag" function
                CadCmdAddPoint(hDwg,x2,y2,z2);
                // set this point as a base for relative coordinates, which can be
                // entered in a command line for the second point (for example: @10,20)
                CadCmdSetBasePoint(hDwg,x2,y2,z2);
            }
            else
            {
                // invalid point
                CadCmdPrompt(hDwg,GetMessage(53).c_str(),"");
                // repeat the input
                goto m2;
            }
      		break;
        }
    	case CAD_CCS_RBUTTON:
    	case CAD_CCS_ESCAPE:
    	default:
        {
      		// finish the command
      		return;
        }
  	}

    // Create vertical line
    VHANDLE hEntity;
    hEntity = CadAddLine(hDwg,x1,y1,z1,x1,y2,z2);
    EntCreateProc(hDwg,hEntity);

  	CadUpdate(hDwg);
    cad_ShowText(1,"");
}
//---------------------------------------------------------------------------
void _stdcall Drag_VLine(VDWG hDwg,int nPoints,double x,double y,double z)
{
  	static double x1,y1,z1;
    switch(nPoints)
    {
  		case 1:
        {
            // one point is already entered, get it from a command buffer
            CadCmdGetPoint(hDwg,0,&x1,&y1,&z1);

            // draw temporary vertical line
            CadDrawLine(x1,y1,z1,x1,y,z);
            // Length
            cad_ShowText(1,GetMessage(7)+": "+cnc_SOutC(abs(y-y1)));
            break;
        }
    }
}
//---------------------------------------------------------------------------
// Horizontal line
void _stdcall Func_HLine(VDWG hDwg)
{
	char szText[256];
  	int status;
  	double x1,y1,z1;
  	double x2,y2,z2;

m1:
	// Input first point:
  	CadCmdPrompt(hDwg,GetMessage(52).c_str(),"");
  	status = CadCmdUserInput(hDwg);
  	switch(status)
    {
    	case CAD_CCS_LBUTTON:
        {
            // user have clicked left mouse button inside of VeCAD Window
            CadCmdGetInputPoint(hDwg,&x1,&y1,&z1);
            // save the entered point in a command buffer, in order to read it in the "Drag" function
            CadCmdAddPoint(hDwg,x1,y1,z1);
            // set this point as a base for relative coordinates, which can be
            // entered in a command line for the second point (for example: @10,20)
            CadCmdSetBasePoint(hDwg,x1,y1,z1);
            break;
        }
    	case CAD_CCS_STRING:
        {
            // user have input coordinates value in a command line
            CadCmdGetInputStr(hDwg,szText);
            // convert the string into point coordinates
            if(CadCmdStrToPoint(hDwg,szText,&x1,&y1,&z1))
            {
                // save the entered point in a command buffer, in order to read it in the "Drag" function
                CadCmdAddPoint(hDwg,x1,y1,z1);
                // set this point as a base for relative coordinates, which can be
                // entered in a command line for the second point (for example: @10,20)
                CadCmdSetBasePoint(hDwg,x1,y1,z1);
            }
            else
            {
                // invalid point
                CadCmdPrompt(hDwg,GetMessage(53).c_str(),"");
                // repeat the input
                goto m1;
            }
            break;
        }
    	case CAD_CCS_RBUTTON:   // user have clicked right mouse button
    	case CAD_CCS_ESCAPE:    // user have pressed <Esc> kay
    	default:
        {
      		// finish the command
      		return;
        }
  	}

m2:
	// Input second point:
  	CadCmdPrompt(hDwg,GetMessage(54).c_str(),"");
  	status = CadCmdUserInput(hDwg);
  	switch(status)
    {
    	case CAD_CCS_LBUTTON:
        {
            // user have clicked left mouse button inside of VeCAD Window
            CadCmdGetInputPoint(hDwg,&x2,&y2,&z2);
            // save the entered point in a command buffer, in order to read it in the "Drag" function
            CadCmdAddPoint(hDwg,x2,y2,z2);
            // set this point as a base for relative coordinates, which can be
            // entered in a command line for the second point (for example: @10,20)
            CadCmdSetBasePoint(hDwg,x2,y2,z2);
      		break;
        }
    	case CAD_CCS_STRING:
        {
            // user have input coordinates value in a command line
            CadCmdGetInputStr(hDwg,szText);
            // convert the string into point coordinates
            if(CadCmdStrToPoint(hDwg,szText,&x2,&y2,&z2))
            {
                // save the entered point in a command buffer, in order to read it in the "Drag" function
                CadCmdAddPoint(hDwg,x2,y2,z2);
                // set this point as a base for relative coordinates, which can be
                // entered in a command line for the second point (for example: @10,20)
                CadCmdSetBasePoint(hDwg,x2,y2,z2);
            }
            else
            {
                // invalid point
                CadCmdPrompt(hDwg,GetMessage(53).c_str(),"");
                // repeat the input
                goto m2;
            }
      		break;
        }
    	case CAD_CCS_RBUTTON:
    	case CAD_CCS_ESCAPE:
    	default:
        {
      		// finish the command
      		return;
        }
  	}

    // Create horizontal line
    VHANDLE hEntity;
    hEntity = CadAddLine(hDwg,x1,y1,z1,x2,y1,z2);
    EntCreateProc(hDwg,hEntity);

  	CadUpdate(hDwg);
    cad_ShowText(1,"");
}
//---------------------------------------------------------------------------
void _stdcall Drag_HLine(VDWG hDwg,int nPoints,double x,double y,double z)
{
  	static double x1,y1,z1;
    switch(nPoints)
    {
  		case 1:
        {
            // one point is already entered, get it from a command buffer
            CadCmdGetPoint(hDwg,0,&x1,&y1,&z1);

            // draw temporary vertical line
            CadDrawLine(x1,y1,z1,x,y1,z);
            // Length
            cad_ShowText(1,GetMessage(7)+": "+cnc_SOutC(abs(x-x1)));
            break;
        }
    }
}
//---------------------------------------------------------------------------
// Insert block
VHANDLE g_hInsBlock = 0;
void _stdcall Func_InsBlock(VDWG hDwg)
{
	char szText[256];
  	int status;
  	double x1,y1,z1;

m1:
	// Input insertion point:
  	CadCmdPrompt(hDwg,GetMessage(55).c_str(),"");
  	status = CadCmdUserInput(hDwg);
  	switch(status)
    {
    	case CAD_CCS_LBUTTON:
        {
            // user have clicked left mouse button inside of VeCAD Window
            CadCmdGetInputPoint(hDwg,&x1,&y1,&z1);
            // save the entered point in a command buffer, in order to read it in the "Drag" function
            CadCmdAddPoint(hDwg,x1,y1,z1);
            // set this point as a base for relative coordinates, which can be
            // entered in a command line for the second point (for example: @10,20)
            CadCmdSetBasePoint(hDwg,x1,y1,z1);
            break;
        }
    	case CAD_CCS_STRING:
        {
            // user have input coordinates value in a command line
            CadCmdGetInputStr(hDwg,szText);
            // convert the string into point coordinates
            if(CadCmdStrToPoint(hDwg,szText,&x1,&y1,&z1))
            {
                // save the entered point in a command buffer, in order to read it in the "Drag" function
                CadCmdAddPoint(hDwg,x1,y1,z1);
                // set this point as a base for relative coordinates, which can be
                // entered in a command line for the second point (for example: @10,20)
                CadCmdSetBasePoint(hDwg,x1,y1,z1);
            }
            else
            {
                // invalid point
                CadCmdPrompt(hDwg,GetMessage(53).c_str(),"");
                // repeat the input
                goto m1;
            }
            break;
        }
    	case CAD_CCS_RBUTTON:   // user have clicked right mouse button
    	case CAD_CCS_ESCAPE:    // user have pressed <Esc> kay
    	default:
        {
      		// finish the command
      		return;
        }
  	}

    // insert block
    IDOBJ idBlock = CadBlockGetID(g_hInsBlock);
    if(idBlock <= 0)	return;
    VHANDLE hEntity;
    hEntity = CadAddInsBlock(hDwg,idBlock,x1,y1,z1,1.0,0.0);
    EntCreateProc(hDwg,hEntity);

  	CadUpdate(hDwg);
    cad_ShowText(1,"");
}
//---------------------------------------------------------------------------
void _stdcall Drag_InsBlock(VDWG hDwg,int nPoints,double x,double y,double z)
{
    switch(nPoints)
    {
  		case 0:
        {
            // draw temporary insertion block
            CadDrawBlock(hDwg,g_hInsBlock,x,y,z,1,1,1,0);
            break;
        }
    }
}
//---------------------------------------------------------------------------
// Frame
void _stdcall Func_FRAME(VDWG hDwg)
{
	char szText[256];
  	int status;
  	double x1,y1,z1;
  	double x2,y2,z2;
  	double x3,y3,z3;

m1:
	// Input center point:
  	CadCmdPrompt(hDwg,GetMessage(56).c_str(),"");
  	status = CadCmdUserInput(hDwg);
  	switch(status)
    {
    	case CAD_CCS_LBUTTON:
        {
            // user have clicked left mouse button inside of VeCAD Window
            CadCmdGetInputPoint(hDwg,&x1,&y1,&z1);
            // save the entered point in a command buffer, in order to read it in the "Drag" function
            CadCmdAddPoint(hDwg,x1,y1,z1);
            // set this point as a base for relative coordinates, which can be
            // entered in a command line for the second point (for example: @10,20)
            CadCmdSetBasePoint(hDwg,x1,y1,z1);
            break;
        }
    	case CAD_CCS_STRING:
        {
            // user have input coordinates value in a command line
            CadCmdGetInputStr(hDwg,szText);
            // convert the string into point coordinates
            if(CadCmdStrToPoint(hDwg,szText,&x1,&y1,&z1))
            {
                // save the entered point in a command buffer, in order to read it in the "Drag" function
                CadCmdAddPoint(hDwg,x1,y1,z1);
                // set this point as a base for relative coordinates, which can be
                // entered in a command line for the second point (for example: @10,20)
                CadCmdSetBasePoint(hDwg,x1,y1,z1);
            }
            else
            {
                // invalid point
                CadCmdPrompt(hDwg,GetMessage(53).c_str(),"");
                // repeat the input
                goto m1;
            }
            break;
        }
    	case CAD_CCS_RBUTTON:   // user have clicked right mouse button
    	case CAD_CCS_ESCAPE:    // user have pressed <Esc> kay
    	default:
        {
      		// finish the command
      		return;
        }
  	}

m2:
	// Input outter corner:
  	CadCmdPrompt(hDwg,GetMessage(57).c_str(),"");
  	status = CadCmdUserInput(hDwg);
  	switch(status)
    {
    	case CAD_CCS_LBUTTON:
        {
            // user have clicked left mouse button inside of VeCAD Window
            CadCmdGetInputPoint(hDwg,&x2,&y2,&z2);
            // save the entered point in a command buffer, in order to read it in the "Drag" function
            CadCmdAddPoint(hDwg,x2,y2,z2);
            // set this point as a base for relative coordinates, which can be
            // entered in a command line for the second point (for example: @10,20)
            CadCmdSetBasePoint(hDwg,x2,y2,z2);
      		break;
        }
    	case CAD_CCS_STRING:
        {
            // user have input coordinates value in a command line
            CadCmdGetInputStr(hDwg,szText);
            // convert the string into point coordinates
            if(CadCmdStrToPoint(hDwg,szText,&x2,&y2,&z2))
            {
                // save the entered point in a command buffer, in order to read it in the "Drag" function
                CadCmdAddPoint(hDwg,x2,y2,z2);
                // set this point as a base for relative coordinates, which can be
                // entered in a command line for the second point (for example: @10,20)
                CadCmdSetBasePoint(hDwg,x2,y2,z2);
            }
            else
            {
                // invalid point
                CadCmdPrompt(hDwg,GetMessage(53).c_str(),"");
                // repeat the input
                goto m2;
            }
      		break;
        }
    	case CAD_CCS_RBUTTON:
    	case CAD_CCS_ESCAPE:
    	default:
        {
      		// finish the command
      		return;
        }
  	}

m3:
	// Input inner corner:
  	CadCmdPrompt(hDwg,GetMessage(58).c_str(),"");
  	status = CadCmdUserInput(hDwg);
  	switch(status)
    {
    	case CAD_CCS_LBUTTON:
        {
            // user have clicked left mouse button inside of VeCAD Window
            CadCmdGetInputPoint(hDwg,&x3,&y3,&z3);
            // save the entered point in a command buffer, in order to read it in the "Drag" function
            CadCmdAddPoint(hDwg,x3,y3,z3);
            // set this point as a base for relative coordinates, which can be
            // entered in a command line for the second point (for example: @10,20)
            CadCmdSetBasePoint(hDwg,x3,y3,z3);
      		break;
        }
    	case CAD_CCS_STRING:
        {
            // user have input coordinates value in a command line
            CadCmdGetInputStr(hDwg,szText);
            // convert the string into point coordinates
            if(CadCmdStrToPoint(hDwg,szText,&x3,&y3,&z3))
            {
                // save the entered point in a command buffer, in order to read it in the "Drag" function
                CadCmdAddPoint(hDwg,x3,y3,z3);
                // set this point as a base for relative coordinates, which can be
                // entered in a command line for the second point (for example: @10,20)
                CadCmdSetBasePoint(hDwg,x3,y3,z3);
            }
            else
            {
                // invalid point
                CadCmdPrompt(hDwg,GetMessage(53).c_str(),"");
                // repeat the input
                goto m3;
            }
      		break;
        }
    	case CAD_CCS_RBUTTON:
    	case CAD_CCS_ESCAPE:
    	default:
        {
      		// finish the command
      		return;
        }
  	}

	cad_Frame data;
	data.x = x1;
	data.y = y1;
	data.z = z1;
	data.width = 2*fabs(x2-x1);
	data.height = 2*fabs(y2-y1);
    data.dx = fabs(x2-x3);
    data.dy = fabs(y2-y3);
    data.InRadius = 0;
    data.OutRadius = 5;
    data.rotation = 0;
	cad_AddFrame(hDwg,data);

  	CadUpdate(hDwg);
    cad_ShowText(1,"");
}
//---------------------------------------------------------------------------
void _stdcall Drag_FRAME(VDWG hDwg,int nPoints,double x,double y,double z)
{
  	static double x1,y1,z1;
  	static double x2,y2,z2;
    double dW;
	cad_Square data;
    switch(nPoints)
    {
  		case 1:
        {
            // one point is already entered, get it from a command buffer
            CadCmdGetPoint(hDwg,0,&x1,&y1,&z1);

            cad_Frame data;
            data.x = x1;
            data.y = y1;
            data.z = z1;
            data.width = 2*fabs(x-x1);
            data.height = 2*fabs(y-y1);
            data.dx = 0;
            data.dy = 0;
            data.InRadius = 0;
            data.OutRadius = 5;
            data.rotation = 0;
            cad_DrawFrame(data);

            CadDrawLine(x1,y1,z1,x,y,z);
            // Width // Height
            cad_ShowText(1,GetMessage(27)+": "+cnc_SOutC(data.width)+" , "+GetMessage(28)+": "+cnc_SOutC(data.height));
            break;
        }
  		case 2:
        {
            // one point is already entered, get it from a command buffer
            CadCmdGetPoint(hDwg,1,&x2,&y2,&z2);

            cad_Frame data;
            data.x = x1;
            data.y = y1;
            data.z = z1;
            data.width = 2*fabs(x2-x1);
            data.height = 2*fabs(y2-y1);
            data.dx = fabs(x2-x);
            data.dy = fabs(y2-y);
            data.InRadius = 0;
            data.OutRadius = 5;
            data.rotation = 0;
            cad_DrawFrame(data);

            CadDrawLine(x1,y1,z1,x,y,z);
            // dx // dy
            cad_ShowText(1,GetMessage(40)+": "+cnc_SOutC(data.dx)+" , "+GetMessage(41)+": "+cnc_SOutC(data.dy));
            break;
        }
    }
}
*/
//---------------------------------------------------------------------------
// Properties
void _stdcall Func_PROP(VDWG hDwg)
{
#ifdef _CNC_
	FormMain->AEditEntPropExecute(NULL);
#endif _CNC_//_CNC_
/*
	char szText[256];
  	int status;
  	double x1,y1,z1;

m1:
	// Input insertion point:
  	CadCmdPrompt(hDwg,GetMessage(55).c_str(),"");
  	status = CadCmdUserInput(hDwg);
  	switch(status)
    {
    	case CAD_CCS_LBUTTON:
        {
            // user have clicked left mouse button inside of VeCAD Window
            CadCmdGetInputPoint(hDwg,&x1,&y1,&z1);
            // save the entered point in a command buffer, in order to read it in the "Drag" function
            CadCmdAddPoint(hDwg,x1,y1,z1);
            // set this point as a base for relative coordinates, which can be
            // entered in a command line for the second point (for example: @10,20)
            CadCmdSetBasePoint(hDwg,x1,y1,z1);
            break;
        }
    	case CAD_CCS_STRING:
        {
            // user have input coordinates value in a command line
            CadCmdGetInputStr(hDwg,szText);
            // convert the string into point coordinates
            if(CadCmdStrToPoint(hDwg,szText,&x1,&y1,&z1))
            {
                // save the entered point in a command buffer, in order to read it in the "Drag" function
                CadCmdAddPoint(hDwg,x1,y1,z1);
                // set this point as a base for relative coordinates, which can be
                // entered in a command line for the second point (for example: @10,20)
                CadCmdSetBasePoint(hDwg,x1,y1,z1);
            }
            else
            {
                // invalid point
                CadCmdPrompt(hDwg,GetMessage(53).c_str(),"");
                // repeat the input
                goto m1;
            }
            break;
        }
    	case CAD_CCS_RBUTTON:   // user have clicked right mouse button
    	case CAD_CCS_ESCAPE:    // user have pressed <Esc> kay
    	default:
        {
      		// finish the command
      		return;
        }
  	}

    // insert block
    IDOBJ idBlock = CadBlockGetID(g_hInsBlock);
    if(idBlock <= 0)	return;
    VHANDLE hEntity;
    hEntity = CadAddInsBlock(hDwg,idBlock,x1,y1,z1,1.0,0.0);
    EntCreateProc(hDwg,hEntity);

  	CadUpdate(hDwg);
    cad_ShowText(1,"");
*/    
}
//---------------------------------------------------------------------------
void _stdcall Drag_PROP(VDWG hDwg,int nPoints,double x,double y,double z)
{
/*
    switch(nPoints)
    {
  		case 0:
        {
            // draw temporary insertion block
            CadDrawBlock(hDwg,g_hInsBlock,x,y,z,1,1,1,0);
            break;
        }
    }
*/
}
//---------------------------------------------------------------------------
//-------------------Utility-------------------------------------------------
//---------------------------------------------------------------------------
/*
void cad_ShowText(int nIndex,AnsiString strText)
{
#ifdef _CNC_
	if((nIndex < 0) || (nIndex >= FormMain->StatusBar->Panels->Count))	return;
	FormMain->StatusBar->Panels->Items[nIndex]->Text = strText;
#endif _CNC_//_CNC_
}
//---------------------------------------------------------------------------
AnsiString GetEntityName(int hEntity)
{
	int hEntityType,hEntityID;
    AnsiString strEntityName;

    hEntityType = CadEntityGetType(hEntity);
    hEntityID = CadEntityGetID(hEntity);
    // Unknown
    strEntityName = GetMessage(59)+FormatFloat("0",hEntityID);
    switch(hEntityType)
    {
        case CAD_ENT_POINT:
        {
        	// Point
            strEntityName = GetMessage(5)+FormatFloat("0",hEntityID);
            break;
        }
        case CAD_ENT_LINE:
        {
        	// Line
            strEntityName = GetMessage(6)+FormatFloat("0",hEntityID);
            break;
        }
        case CAD_ENT_POLYLINE:
        {
        	// Polyline
            strEntityName = GetMessage(9)+FormatFloat("0",hEntityID);
            int FitType = CadPlineGetFit(hEntity);
            switch(FitType)
            {
             	case CAD_PLINE_LINEAR:
                {
                	// Polyline
            		strEntityName = GetMessage(9)+FormatFloat("0",hEntityID);
                 	break;
                }
             	case CAD_PLINE_QUAD:
                {
                	// QB-Spline
            		strEntityName = GetMessage(60)+FormatFloat("0",hEntityID);
                 	break;
                }
             	case CAD_PLINE_CUBIC:
                {
                	// CB-Spline
            		strEntityName = GetMessage(61)+FormatFloat("0",hEntityID);
                 	break;
                }
             	case CAD_PLINE_BEZIER:
                {
                	// Bezier
            		strEntityName = GetMessage(62)+FormatFloat("0",hEntityID);
                 	break;
                }
             	case CAD_PLINE_SPLINE:
                {
                	// Spline
            		strEntityName = GetMessage(63)+FormatFloat("0",hEntityID);
                 	break;
                }
             	case CAD_PLINE_ROUND:
                {
                	// R-Polyline
            		strEntityName = GetMessage(64)+FormatFloat("0",hEntityID);
                 	break;
                }
             	case CAD_PLINE_LINQUAD:
                {
                	// Q-Polyline
            		strEntityName = GetMessage(65)+FormatFloat("0",hEntityID);
                 	break;
                }
            }
            break;
        }
        case CAD_ENT_RECT:
        {
        	// Rect
            strEntityName = GetMessage(26)+FormatFloat("0",hEntityID);
            break;
        }
        case CAD_ENT_MLINE:
        {
        	// MLine
            strEntityName = GetMessage(12)+FormatFloat("0",hEntityID);
            break;
        }
        case CAD_ENT_ARC:
        {
        	// Arc
            strEntityName = GetMessage(15)+FormatFloat("0",hEntityID);
            int et = CadArcGetType(hEntity);
            switch(et)
            {
                case CAD_ARC:
                {
                	// Arc
            		strEntityName = GetMessage(15)+FormatFloat("0",hEntityID);
                    break;
                }
                case CAD_CIRCLE:
                {
					// Circle
            		strEntityName = GetMessage(18)+FormatFloat("0",hEntityID);
                    break;
                }
                case CAD_ELLIPSE:
                {
                	// Ellipse
            		strEntityName = GetMessage(19)+FormatFloat("0",hEntityID);
                    break;
                }
            }
            break;
        }
        case CAD_ENT_TEXT:
        {
        	// Text
            strEntityName = GetMessage(68)+FormatFloat("0",hEntityID);
            break;
        }
        case CAD_ENT_INSBLOCK:
        {
        	// Block
            strEntityName = GetMessage(29)+FormatFloat("0",hEntityID);
            break;
        }
        case CAD_ENT_IMAGE:
        {
        	// Bitmap
            strEntityName = GetMessage(69)+FormatFloat("0",hEntityID);
            break;
        }
        case CAD_ENT_HATCH:
        {
        	// Hatch
            strEntityName = GetMessage(70)+FormatFloat("0",hEntityID);
            break;
        }
        case CAD_ENT_DIM:
        {
        	// Dim
            strEntityName = GetMessage(71)+FormatFloat("0",hEntityID);
            break;
        }
        case CAD_ENT_LEADER:
        {
        	// Leader
            strEntityName = GetMessage(72)+FormatFloat("0",hEntityID);
            break;
        }
        case CAD_ENT_ATTRIB:
        {
        	// Attrib
            strEntityName = GetMessage(73)+FormatFloat("0",hEntityID);
            break;
        }
        case CAD_ENT_CUSTOM:
        {
        	// Custom
            strEntityName = GetMessage(74)+FormatFloat("0",hEntityID);
            int et = CadCustomGetType(hEntity);
            switch(et)
            {
                case CAD_TYPE_SQUARE:
                {
                	// Square
            		strEntityName = GetMessage(37)+FormatFloat("0",hEntityID);
                    break;
                }
                case CAD_TYPE_MACINE_HEAD:
                {
                	// Machine head
            		strEntityName = GetMessage(115)+FormatFloat("0",hEntityID);
                    break;
                }
                case CAD_TYPE_FRAME:
                {
                	// Frame
            		strEntityName = GetMessage(39)+FormatFloat("0",hEntityID);
                    break;
                }
            }
            break;
        }
    }
    return strEntityName;
}
*/
//---------------------------------------------------------------------------
//--------------------Events-------------------------------------------------
//---------------------------------------------------------------------------
void CALLBACK EntCreateProc(VDWG hDwg,VHANDLE hEnt)
{
/*
    if((hDwg == FormBlock->hDwg) && (CadEntityGetType(hEnt) == CAD_ENT_POINT))
    {
    	double x,y,z;
    	CadPointGetCoord(hEnt,&x,&y,&z);
        FormBlock->EditX->Text = cnc_SOutC(x);
        FormBlock->EditY->Text = cnc_SOutC(y);
    	CadEntityErase(hEnt,true);
    	CadUpdate(hDwg);
		return;
    }

    if(hDwg == FormMain->hDwg)
    {
        if(g_bGenerateNamesList)
        {
            AnsiString strEntityName;
            strEntityName = GetEntityName(hEnt);
            FormMain->ListBoxEntities->Items->AddObject(strEntityName,(TObject *)hEnt);
        }

        cad_Entity cadEntity;
        cadEntity.nToolIndex = g_nCurrToolIndex;
        cadEntity.dDepth = g_dCurrDepth;
        CadEntityPutExData(hEnt,&cadEntity,sizeof(cadEntity));

		CadEntityPutColor(hEnt,g_pTools[g_nCurrToolIndex-stTool_1].nColor);
    	CadPutDirty(hDwg,true);
        CadUpdate(hDwg);
    }
*/    
}
//---------------------------------------------------------------------------
void CALLBACK EntCopyProc(VDWG hDwg,VHANDLE hEntNew,VHANDLE hEntSrc)
{
/*
    if(hDwg == FormMain->hDwg)
    {
        if(g_bGenerateNamesList)
        {
            AnsiString strEntityName;
            strEntityName = GetEntityName(hEntNew);
            FormMain->ListBoxEntities->Items->AddObject(strEntityName,(TObject *)hEntNew);
        }
    }
*/
}
//---------------------------------------------------------------------------
void CALLBACK EntSelectProc(VDWG hDwg,VHANDLE hEnt,BOOL bSelect,BOOL bFinal)
{
	if(hEnt == NULL)	return;
/*
    if(bSelect)
    {
        int nIndex;
        nIndex = FormMain->ListBoxEntities->Items->IndexOfObject((TObject *)hEnt);
        if(nIndex >= 0)
        {
            FormMain->ListBoxEntities->ItemIndex = nIndex;
            FormMain->ActivateEntity(hEnt);
        }
    }
    else
    {
    	FormMain->ListBoxEntities->ItemIndex = -1;
        FormMain->ActivateEntity(-1);
    }
*/    
}
//---------------------------------------------------------------------------
void CALLBACK EntEraseProc(VDWG hDwg,VHANDLE hEnt)
{
/*
	if(hEnt == g_hMachineHeadIndex)
    {
     	CadSetReturnInt(true);
        return;
    }

    int nIndex;
    nIndex = FormMain->ListBoxEntities->Items->IndexOfObject((TObject *)hEnt);
    if(nIndex >= 0)
    {
    	FormMain->ListBoxEntities->Items->Delete(nIndex);
    }
    FormMain->ListBoxEntities->ItemIndex = -1;
    FormMain->ActivateEntity(-1);
*/
}
//---------------------------------------------------------------------------
void CALLBACK EntChangeProc(VDWG hDwg,VHANDLE hEnt)
{
/*
	if(!CadEntityGetDeleted(hEnt))
    {
	    FormMain->ActivateEntity(hEnt);
    }
*/
}
//---------------------------------------------------------------------------
void CALLBACK PromptProc(VDWG hDwg,LPCTSTR szText)
{
//	cad_ShowText(2,szText);
/*
	AnsiString strText = szText;
    int nPos;
    nPos = strText.Pos(":");
    if(nPos > 0)
    {
    	g_strLastCommand = strText.SubString(1,nPos-1);
     	strText.Delete(1,nPos);
    }
    else
    {
     	g_strLastCommand = szText;
    }
    strText.Trim();
    g_strLastCommand.Trim();
	FormMain->PanelCommandLine->Caption = strText;
*/
}
//---------------------------------------------------------------------------
void CALLBACK MouseMoveProc(VDWG hDwg,int Button,int Key,int Xwin,
							int Ywin,double Xdwg,double Ydwg,double Zdwg)
{
/*
	AnsiString strText = cnc_SOutC(Xdwg)+" , "+cnc_SOutC(Ydwg);
	FormMain->PanelCoordinates->Caption = strText;
	FormSimulator->StatusBar->Panels->Items[0]->Text = strText;
*/
//	cad_ShowText(0,strText);
}
//---------------------------------------------------------------------------
void CALLBACK MouseDownProc(VDWG hDwg,int Button,int Key,int Xwin,
							int Ywin,double Xdwg,double Ydwg,double Zdwg)
{
	if(Button == CAD_RBUTTON)
    {
	    // CadSetReturnInt(true);
    }
}
//---------------------------------------------------------------------------
void CALLBACK MouseDblClkProc(VDWG hDwg,int Button,int Key,int Xwin,
							int Ywin,double Xdwg,double Ydwg,double Zdwg)
{
/*
	if(Button == CAD_LBUTTON)
    {
	    CadSetReturnInt(true);// To hide property dialog

        VHANDLE nEntityIndex;
        nEntityIndex = CadGetEntityByCursor(hDwg);
        if(nEntityIndex)
        {
	        FormMain->ShowEntityProperty(nEntityIndex);
			FormProperties->Show();
        }
    }
*/
}
//---------------------------------------------------------------------------
void CALLBACK FontReplaceProc(VDWG hDwg,LPCTSTR szFont,int bBigFont)
{
	CadSetReturnStr(szFont);
}
//---------------------------------------------------------------------------
void CALLBACK SelCodepageProc(VDWG hDwg)
{
	CadSetReturnInt(CAD_CP_1256);// Arabic Code Page
//	CadSetReturnInt(CAD_CP_1252);// Latin1 Code Page
}
//---------------------------------------------------------------------------
void CALLBACK CmdStartProc(VDWG hDwg,int Command,int nSteps)
{

}
//---------------------------------------------------------------------------
void CALLBACK CmdLBDownProc(VDWG hDwg,int Command,int Step,double* pX,double* pY,double* pZ)
{

}
//---------------------------------------------------------------------------
void CALLBACK ExecuteProc(VDWG hDwg,int Command)
{
	switch(Command)
    {
     	case CAD_CMD_UNDO:
     	case CAD_CMD_REDO:
        {
        	//FormMain->BuildEntitiesList();
         	break;
        }
    }
}
//---------------------------------------------------------------------------
void CALLBACK CmdFinishProc(VDWG hDwg,int Command)
{
/*
	switch(Command)
    {
     	case CAD_CMD_EXPLODE:
        {
			FormMain->BuildEntitiesList();
         	break;
        }
     	case CAD_CMD_JOIN:
        {
			FormMain->BuildEntitiesList();
         	break;
        }
     	case CAD_CMD_UNDO:
     	case CAD_CMD_REDO:
        {
			//FormMain->BuildEntitiesList();
         	break;
        }
     	case CAD_CMD_TRIM:
        {
			FormMain->BuildEntitiesList();
         	break;
        }
     	case CAD_CMD_EXTEND:
        {
			FormMain->BuildEntitiesList();
         	break;
        }
    }
*/
}
//---------------------------------------------------------------------------
void CALLBACK RegenProc(VDWG hDwg,int Step,int Percent,int NumEnt)
{
	switch(Step)
    {
		case 0:// start of the regeneration
        {
         	break;
        }
		case 1:// regeneration is underway
        {
         	break;
        }
		case 2:// end of the regeneration
        {
         	break;
        }
    }
}
//---------------------------------------------------------------------------
void CALLBACK PaintProc(VDWG hDwg,HDC hDrawDC,int Mode,int WinWidth,int WinHeight, 
	double DwgLeft,double DwgBottom,double DwgRight,double DwgTop,double Scale)
{
	if(Mode == 0)// Before drawing
    {
    	if(CadGetExDataSize(hDwg) < sizeof(cnc_DrawingInfo))	return;
    	cnc_DrawingInfo DInfo;
        CadGetExData(hDwg,&DInfo);

    	CadDrawAddVertex(DInfo.m_dWorkingAreaX0MM,DInfo.m_dWorkingAreaY0MM,0);
    	CadDrawAddVertex(DInfo.m_dWorkingAreaX0MM+DInfo.m_dWorkingAreaWidthMM,DInfo.m_dWorkingAreaY0MM,0);
    	CadDrawAddVertex(DInfo.m_dWorkingAreaX0MM+DInfo.m_dWorkingAreaWidthMM,DInfo.m_dWorkingAreaY0MM+DInfo.m_dWorkingAreaHeightMM,0);
    	CadDrawAddVertex(DInfo.m_dWorkingAreaX0MM,DInfo.m_dWorkingAreaY0MM+DInfo.m_dWorkingAreaHeightMM,0);
    	CadDrawAddVertex(DInfo.m_dWorkingAreaX0MM,DInfo.m_dWorkingAreaY0MM,0);
		CadDrawPolyline();

        int PtMode,PtSize;
        PtSize = sqrt(DInfo.m_dWorkingAreaWidthMM*DInfo.m_dWorkingAreaWidthMM+DInfo.m_dWorkingAreaHeightMM*DInfo.m_dWorkingAreaHeightMM)/20.0;
        PtMode = CAD_PNT_X | CAD_PNT_CIRCLE | CAD_PNT_SQUARE;
        CadDrawPoint(DInfo.m_dWorkingAreaX0MM,DInfo.m_dWorkingAreaY0MM,0,PtMode,PtSize);
    	CadDrawPoint(DInfo.m_dWorkingAreaX0MM+DInfo.m_dWorkingAreaWidthMM,DInfo.m_dWorkingAreaY0MM,0,PtMode,PtSize);
    	CadDrawPoint(DInfo.m_dWorkingAreaX0MM+DInfo.m_dWorkingAreaWidthMM,DInfo.m_dWorkingAreaY0MM+DInfo.m_dWorkingAreaHeightMM,0,PtMode,PtSize);
    	CadDrawPoint(DInfo.m_dWorkingAreaX0MM,DInfo.m_dWorkingAreaY0MM+DInfo.m_dWorkingAreaHeightMM,0,PtMode,PtSize);
    }
    else// After drawing
    {
    }
}   
//---------------------------------------------------------------------------
//--------------------Custom Entities----------------------------------------
//---------------------------------------------------------------------------
void cad_MoveEntity(VHANDLE hEnt,double x,double y,double z)
{
  	int et = CadCustomGetType(hEnt);
  	switch(et)
    {
    	case CAD_TYPE_SQUARE:
        {
  			cad_Square data;
            CadCustomGetData(hEnt,&data);
            data.x = x;
            data.y = y;
            data.z = z;
            CadCustomPutData(hEnt,&data,sizeof(data));
      		break;
        }
    	case CAD_TYPE_MACINE_HEAD:
        {
            cad_MachineHead data;
            CadCustomGetData(hEnt,&data);
            data.x = x;
            data.y = y;
            data.z = z;
            CadCustomPutData(hEnt,&data,sizeof(data));
      		break;
        }
    	case CAD_TYPE_FRAME:
        {
  			cad_Frame data;
            CadCustomGetData(hEnt,&data);
            data.x = x;
            data.y = y;
            data.z = z;
            CadCustomPutData(hEnt,&data,sizeof(data));
      		break;
        }
  	}
}
//---------------------------------------------------------------------------
void CALLBACK EventCEntGetName(VDWG hDwg,VHANDLE hEnt)
{
  	int et = CadCustomGetType(hEnt);
  	switch(et)
    {
    	case CAD_TYPE_SQUARE:
        {
         	CadSetReturnStr("Sqaure");
            break;
        }
    	case CAD_TYPE_MACINE_HEAD:
        {
         	CadSetReturnStr("Mahine head");
            break;
        }
        case CAD_TYPE_FRAME:
        {
         	CadSetReturnStr("Frame");
            break;
        }
  	}
}
//---------------------------------------------------------------------------
void CALLBACK EventCEntGetExts(VDWG hDwg,VHANDLE hEnt,double* pXmin,double* pYmin,double* pZmin,double* pXmax,double* pYmax,double* pZmax)
{
  	int et = CadCustomGetType(hEnt);
  	switch(et)
    {
    	case CAD_TYPE_SQUARE:
        {
  			cad_Square data;
      		CadCustomGetData(hEnt,&data);
      		*pXmin = *pXmax = data.x;
      		*pYmin = *pYmax = data.y;
      		*pZmin = *pZmax = data.z;
      		break;
        }
    	case CAD_TYPE_MACINE_HEAD:
        {
  			cad_MachineHead data;
      		CadCustomGetData(hEnt,&data);
      		*pXmin = *pXmax = data.x;
      		*pYmin = *pYmax = data.y;
      		*pZmin = *pZmax = data.z;
      		break;
        }
    	case CAD_TYPE_FRAME:
        {
  			cad_Frame data;
      		CadCustomGetData(hEnt,&data);
      		*pXmin = *pXmax = data.x;
      		*pYmin = *pYmax = data.y;
      		*pZmin = *pZmax = data.z;
      		break;
        }
  	}
}
//---------------------------------------------------------------------------
void CALLBACK EventCEntNumGrips(VDWG hDwg,VHANDLE hEnt,int* pN)
{
  	int et = CadCustomGetType(hEnt);
  	switch(et)
    {
    	case CAD_TYPE_SQUARE:
        {
         	*pN = 3;
            break;
        }
    	case CAD_TYPE_MACINE_HEAD:
        {
         	*pN = 3;
            break;
        }
    	case CAD_TYPE_FRAME:
        {
         	*pN = 6;
            break;
        }
  	}
}
//---------------------------------------------------------------------------
void CALLBACK EventCEntGetGrip(VDWG hDwg,VHANDLE hEnt,UINT iGrip,double* pX,double* pY,double* pZ)
{
  	int Plane = CAD_PLANE_XY;  // Current projection planes
  	int et = CadCustomGetType(hEnt);
  	switch(et)
    {
    	case CAD_TYPE_SQUARE:
        {
  			cad_Square data;
      		CadCustomGetData(hEnt,&data);
      		*pX = data.x;
      		*pY = data.y;
            *pZ = data.z;
      		switch(iGrip)
            {
        		case 0:  // center grip
                {
          			break;
                }
        		case 1:  // rotation grip
                {
          			vuPolarPoint(pX,pY,pZ,data.RotAngle,data.Size/4.0,Plane);
          			break;
                }
        		case 2:  // Size grip
                {
          			vuPolarPoint(pX,pY,pZ,data.RotAngle,data.Size/2.0,Plane);
          			break;
                }
            }
      		break;
      	}
    	case CAD_TYPE_MACINE_HEAD:
        {
  			cad_MachineHead data;
      		CadCustomGetData(hEnt,&data);
      		*pX = data.x;
      		*pY = data.y;
            *pZ = data.z;
      		switch(iGrip)
            {
        		case 0:  // center grip
                {
          			break;
                }
        		case 1:  // rotation grip
                {
          			vuPolarPoint(pX,pY,pZ,data.RotAngle,data.w/4.0,Plane);
          			break;
                }
        		case 2:  // Size grip
                {
          			vuPolarPoint(pX,pY,pZ,data.RotAngle,data.w/2.0,Plane);
          			break;
                }
            }
      		break;
      	}
    	case CAD_TYPE_FRAME:
        {
  			cad_Frame data;
      		CadCustomGetData(hEnt,&data);
      		*pX = data.x;
      		*pY = data.y;
            *pZ = data.z;
      		switch(iGrip)
            {
        		case 0:  // center grip
                {
          			break;
                }
        		case 1:  // rotation grip
                {
          			vuPolarPoint(pX,pY,pZ,data.rotation+0,3*data.width/4.0,Plane);
          			break;
                }
        		case 2:  // Width grip
                {
          			vuPolarPoint(pX,pY,pZ,data.rotation+0,data.width/2.0,Plane);
          			break;
                }
        		case 3:  // Height grip
                {
          			vuPolarPoint(pX,pY,pZ,data.rotation+shp_PI2,data.height/2.0,Plane);
          			break;
                }
        		case 4:  // dx grip
                {
          			vuPolarPoint(pX,pY,pZ,data.rotation+0,data.width/2.0-data.dx,Plane);
          			break;
                }
        		case 5:  // dy grip
                {
          			vuPolarPoint(pX,pY,pZ,data.rotation+shp_PI2,data.height/2.0-data.dy,Plane);
          			break;
                }
            }
      		break;
      	}
  	}
}
//---------------------------------------------------------------------------
void CALLBACK EventCEntMoveGrip(VDWG hDwg,VHANDLE hEnt,UINT iGrip,double dx,double dy,double dz)
{
  	int Plane = CAD_PLANE_XY;  // Current projection plane
  	int et = CadCustomGetType(hEnt);
  	double x,y,z;
  	switch(et)
    {
    	case CAD_TYPE_SQUARE:
        {
  			cad_Square data;
      		CadCustomGetData(hEnt,&data);
      		switch(iGrip)
            {
        		case 0:  // move entity
                {
          			data.x += dx;
          			data.y += dy;
          			data.z += dz;
          			break;
                }
        		case 1:  // rotate
                {
          			EventCEntGetGrip(hDwg,hEnt,iGrip,&x,&y,&z);
          			x += dx;
          			y += dy;
          			z += dz;
          			data.RotAngle = vuGetAngle(data.x,data.y,data.z,x,y,z,Plane);
          			break;
                }
        		case 2:  //change Size
                {
          			EventCEntGetGrip(hDwg,hEnt,iGrip,&x,&y,&z);
          			x += dx;
          			y += dy;
          			z += dz;
          			data.Size = vuGetDist(data.x,data.y,data.z,x,y,z,Plane)*2.0;
          			break;
                }
            }
      		CadCustomPutData(hEnt,&data,sizeof(data));
	      	break;
      	}
    	case CAD_TYPE_MACINE_HEAD:
        {
  			cad_MachineHead data;
      		CadCustomGetData(hEnt,&data);
      		switch(iGrip)
            {
        		case 0:  // move entity
                {
          			data.x += dx;
          			data.y += dy;
          			data.z += dz;
          			break;
                }
        		case 1:  // rotate
                {
          			EventCEntGetGrip(hDwg,hEnt,iGrip,&x,&y,&z);
          			x += dx;
          			y += dy;
          			z += dz;
          			data.RotAngle = vuGetAngle(data.x,data.y,data.z,x,y,z,Plane);
          			break;
                }
        		case 2:  //change Size
                {
          			EventCEntGetGrip(hDwg,hEnt,iGrip,&x,&y,&z);
          			x += dx;
          			y += dy;
          			z += dz;
          			data.w = vuGetDist(data.x,data.y,data.z,x,y,z,Plane)*2.0;
          			data.h = vuGetDist(data.x,data.y,data.z,x,y,z,Plane)*2.0;
          			break;
                }
            }
      		CadCustomPutData(hEnt,&data,sizeof(data));
	      	break;
      	}
    	case CAD_TYPE_FRAME:
        {
  			cad_Frame data;
      		CadCustomGetData(hEnt,&data);
      		switch(iGrip)
            {
        		case 0:  // move entity
                {
          			data.x += dx;
          			data.y += dy;
          			data.z += dz;
          			break;
                }
        		case 1:  // rotate
                {
          			EventCEntGetGrip(hDwg,hEnt,iGrip,&x,&y,&z);
          			x += dx;
          			y += dy;
          			z += dz;
          			data.rotation = vuGetAngle(data.x,data.y,data.z,x,y,z,Plane);
          			break;
                }
        		case 2:  //change width
                {
          			EventCEntGetGrip(hDwg,hEnt,iGrip,&x,&y,&z);
          			x += dx;
          			y += dy;
          			z += dz;
          			data.width = vuGetDist(data.x,data.y,data.z,x,y,z,Plane)*2.0;
          			break;
                }
        		case 3:  //change height
                {
          			EventCEntGetGrip(hDwg,hEnt,iGrip,&x,&y,&z);
          			x += dx;
          			y += dy;
          			z += dz;
          			data.height = vuGetDist(data.x,data.y,data.z,x,y,z,Plane)*2.0;
          			break;
                }
        		case 4:  //change dx
                {
          			EventCEntGetGrip(hDwg,hEnt,iGrip,&x,&y,&z);
          			x += dx;
          			y += dy;
          			z += dz;
          			data.dx = data.width/2.0-vuGetDist(data.x,data.y,data.z,x,y,z,Plane);
          			break;
                }
        		case 5:  //change dy
                {
          			EventCEntGetGrip(hDwg,hEnt,iGrip,&x,&y,&z);
          			x += dx;
          			y += dy;
          			z += dz;
          			data.dy = data.height/2.0-vuGetDist(data.x,data.y,data.z,x,y,z,Plane);
          			break;
                }
            }
      		CadCustomPutData(hEnt,&data,sizeof(data));
	      	break;
      	}
  	}
}
//---------------------------------------------------------------------------
void CALLBACK EventCEntMove(VDWG hDwg,VHANDLE hEnt,double dx,double dy,double dz)
{
  	int et = CadCustomGetType(hEnt);
  	switch(et)
    {
    	case CAD_TYPE_SQUARE:
        {
  			cad_Square data;
      		CadCustomGetData(hEnt,&data);
      		data.x += dx;
      		data.y += dy;
      		data.z += dz;
      		CadCustomPutData(hEnt,&data,sizeof(data));
      		break;
        }
    	case CAD_TYPE_MACINE_HEAD:
        {
  			cad_MachineHead data;
      		CadCustomGetData(hEnt,&data);
      		data.x += dx;
      		data.y += dy;
      		data.z += dz;
      		CadCustomPutData(hEnt,&data,sizeof(data));
      		break;
        }
    	case CAD_TYPE_FRAME:
        {
  			cad_Frame data;
      		CadCustomGetData(hEnt,&data);
      		data.x += dx;
      		data.y += dy;
      		data.z += dz;
      		CadCustomPutData(hEnt,&data,sizeof(data));
      		break;
        }
  	}
}
//---------------------------------------------------------------------------
void CALLBACK EventCEntRotate(VDWG hDwg,VHANDLE hEnt,double x0,double y0,double z0,double Angle,int Plane)
{
  	int et = CadCustomGetType(hEnt);
  	switch(et)
    {
    	case CAD_TYPE_SQUARE:
        {
  			cad_Square data;
      		CadCustomGetData(hEnt,&data);
      		vuRotatePoint(&data.x,&data.y,&data.z,x0,y0,z0,Angle,Plane);
      		data.RotAngle += Angle;
      		CadCustomPutData(hEnt,&data,sizeof(data));
      		break;
        }
    	case CAD_TYPE_MACINE_HEAD:
        {
  			cad_MachineHead data;
      		CadCustomGetData(hEnt,&data);
      		vuRotatePoint(&data.x,&data.y,&data.z,x0,y0,z0,Angle,Plane);
      		data.RotAngle += Angle;
      		CadCustomPutData(hEnt,&data,sizeof(data));
      		break;
        }
    	case CAD_TYPE_FRAME:
        {
  			cad_Square data;
      		CadCustomGetData(hEnt,&data);
      		vuRotatePoint(&data.x,&data.y,&data.z,x0,y0,z0,Angle,Plane);
      		data.RotAngle += Angle;
      		CadCustomPutData(hEnt,&data,sizeof(data));
      		break;
        }
  	}
}
//---------------------------------------------------------------------------
void CALLBACK EventCEntScale(VDWG hDwg,VHANDLE hEnt,double x0,double y0,double z0,double ScaleX,double ScaleY,double ScaleZ)
{
  	int et = CadCustomGetType(hEnt);
  	switch(et)
    {
    	case CAD_TYPE_SQUARE:
        {
  			cad_Square data;
      		CadCustomGetData(hEnt,&data);
      		vuScalePoint(&data.x,&data.y,&data.z,x0,y0,z0,ScaleX,ScaleY,ScaleZ);
      		data.Size *= ScaleX;
      		CadCustomPutData(hEnt,&data,sizeof(data));
      		break;
        }
    	case CAD_TYPE_MACINE_HEAD:
        {
  			cad_MachineHead data;
      		CadCustomGetData(hEnt,&data);
      		vuScalePoint(&data.x,&data.y,&data.z,x0,y0,z0,ScaleX,ScaleY,ScaleZ);
      		data.w *= ScaleX;
      		data.h *= ScaleY;
      		CadCustomPutData(hEnt,&data,sizeof(data));
      		break;
        }
    	case CAD_TYPE_FRAME:
        {
  			cad_Frame data;
      		CadCustomGetData(hEnt,&data);
      		vuScalePoint(&data.x,&data.y,&data.z,x0,y0,z0,ScaleX,ScaleY,ScaleZ);
      		data.width *= ScaleX;
      		data.height *= ScaleY;
      		data.dx *= ScaleX;
      		data.dy *= ScaleY;
      		CadCustomPutData(hEnt,&data,sizeof(data));
      		break;
        }
  	}
}
//---------------------------------------------------------------------------
void CALLBACK EventCEntMirror(VDWG hDwg,VHANDLE hEnt,double A1,double B1,int Plane)
{
  	double x,y,z;
  	int et = CadCustomGetType(hEnt);
  	switch(et)
    {
    	case CAD_TYPE_SQUARE:
        {
  			cad_Square data;
      		CadCustomGetData(hEnt,&data);
      		EventCEntGetGrip(hDwg,hEnt,1,&x,&y,&z);
      		vuMirrorPoint(&x,&y,&z,A1,B1,Plane);
      		vuMirrorPoint(&data.x,&data.y,&data.z,A1,B1,Plane);
      		data.RotAngle = vuGetAngle(data.x,data.y,data.z,x,y,z,Plane);
      		CadCustomPutData(hEnt,&data,sizeof(data));
      		break;
        }
    	case CAD_TYPE_MACINE_HEAD:
        {
  			cad_MachineHead data;
      		CadCustomGetData(hEnt,&data);
      		EventCEntGetGrip(hDwg,hEnt,1,&x,&y,&z);
      		vuMirrorPoint(&x,&y,&z,A1,B1,Plane);
      		vuMirrorPoint(&data.x,&data.y,&data.z,A1,B1,Plane);
      		data.RotAngle = vuGetAngle(data.x,data.y,data.z,x,y,z,Plane);
      		CadCustomPutData(hEnt,&data,sizeof(data));
      		break;
        }
    	case CAD_TYPE_FRAME:
        {
  			cad_Frame data;
      		CadCustomGetData(hEnt,&data);
      		EventCEntGetGrip(hDwg,hEnt,1,&x,&y,&z);
      		vuMirrorPoint(&x,&y,&z,A1,B1,Plane);
      		vuMirrorPoint(&data.x,&data.y,&data.z,A1,B1,Plane);
      		data.rotation = vuGetAngle(data.x,data.y,data.z,x,y,z,Plane);
      		CadCustomPutData(hEnt,&data,sizeof(data));
      		break;
        }
  	}
}
//---------------------------------------------------------------------------
void CALLBACK EventCEntDisplay(VDWG hDwg,VHANDLE hEnt)
{
  	int et = CadCustomGetType(hEnt);
    switch(et)
    {
  		case CAD_TYPE_SQUARE:
        {
            cad_Square data;
  			bool bFill;
            CadCustomGetData(hEnt,&data);
            bFill = CadEntityGetFilled(hEnt);
            cad_DrawSquare(data,bFill);
            break;
        }
  		case CAD_TYPE_MACINE_HEAD:
        {
            cad_MachineHead data;
            CadCustomGetData(hEnt,&data);
            cad_DrawMachineHead(data);
            break;
        }
  		case CAD_TYPE_FRAME:
        {
            cad_Frame data;
            CadCustomGetData(hEnt,&data);
            cad_DrawFrame(data);
            break;
        }
    }
}
//---------------------------------------------------------------------------
void cad_DrawSquare(const cad_Square& data,bool bFill)
{
  	double x[5],y[5],z=0.0;
  	int i;

  	x[0] = x[4] = x[3] = data.x - data.Size / 2.0;
  	y[0] = y[4] = y[1] = data.y - data.Size / 2.0;
  	x[1] = x[2] = x[0] + data.Size;
  	y[2] = y[3] = y[0] + data.Size;
  	for(i=0;i<5;i++)
    {
    	vuRotatePoint(x+i,y+i,&z,data.x,data.y,data.z,data.RotAngle,CAD_PLANE_XY);
    	CadDrawAddVertex(x[i],y[i],data.z);
  	}
  	if(bFill)
    {
    	CadDrawPolygon();
  	}
    else
    {
    	CadDrawPolyline();
  	}
}
//---------------------------------------------------------------------------
void cad_AddSquare(VDWG hDwg,cad_Square& data)
{
	int hEntity;
//	hEntity = CadAddCustom(hDwg,CAD_TYPE_SQUARE,&data,sizeof(data));
	hEntity = CadAddRect(hDwg,data.x,data.y,data.z,data.Size,data.Size,data.RotAngle);
    EntCreateProc(hDwg,hEntity);
    CadPutDirty(hDwg,true);
}
//---------------------------------------------------------------------------
void cad_AddSquare(VDWG hDwg,double x,double y,double z,double Size,double RotAngle)
{
    cad_Square data;
    data.x = x;
    data.y = y;
    data.z = z;
    data.Size = Size;
    data.RotAngle = RotAngle;
    cad_AddSquare(hDwg,data);
}
//---------------------------------------------------------------------------
void cad_DrawMachineHead(const cad_MachineHead& data)
{
	// Draw outter frame
  	double x[5],y[5],z=0.0;
  	x[0] = x[4] = x[3] = data.x - data.w / 2.0;
  	y[0] = y[4] = y[1] = data.y - data.h / 2.0;
  	x[1] = x[2] = x[0] + data.w;
  	y[2] = y[3] = y[0] + data.h;
  	for(int i=0;i<5;i++)
    {
    	vuRotatePoint(x+i,y+i,&z,data.x,data.y,data.z,data.RotAngle,CAD_PLANE_XY);
    	CadDrawAddVertex(x[i],y[i],data.z);
  	}
    CadDrawPolyline();

	cnc_Tool Tool;
    double X,Y,r;
    int nVers = 4;
	for(int i=0;i<c_nToolsCount;i++)
    {
		Tool = g_pTools[i];
        X = data.x+Tool.x0;
        Y = data.y+Tool.y0;
        r = Tool.r;
    	vuRotatePoint(&X,&Y,&z,data.x,data.y,data.z,data.RotAngle,CAD_PLANE_XY);
        CadDrawSet(CAD_CE_COLOR,Tool.nColor);
		CadDrawGenCircle(X,Y,z,r,nVers);
		CadDrawPolyline();
    }
}
//---------------------------------------------------------------------------
VHANDLE cad_AddMachineHead(VDWG hDwg,cad_MachineHead& data)
{
	VHANDLE hEntity;
	hEntity = CadAddCustom(hDwg,CAD_TYPE_MACINE_HEAD,&data,sizeof(data));
    EntCreateProc(hDwg,hEntity);
    CadPutDirty(hDwg,true);
    return hEntity;
}
//---------------------------------------------------------------------------
VHANDLE cad_AddMachineHead(VDWG hDwg,double x,double y,double z,double w,double h,double RotAngle)
{
    cad_MachineHead data;
    data.x = x;
    data.y = y;
    data.z = z;
    data.w = w;
    data.h = h;
    data.RotAngle = RotAngle;
    return cad_AddMachineHead(hDwg,data);
}
//---------------------------------------------------------------------------
void cad_DrawFrame(const cad_Frame& data)
{
	double w2,h2,dw,dh,r;
    double x[12],y[12],z;

    r = data.OutRadius;
    w2 = data.width/2.0;
    h2 = data.height/2.0;
    dw = w2-r;
    dh = h2-r;
    x[0] = data.x+dw;
    y[0] = data.y+dh;
    x[1] = data.x+dw;
    y[1] = data.y+h2;
    x[2] = data.x-dw;
    y[2] = data.y+h2;
    x[3] = data.x-dw;
    y[3] = data.y+dh;
    x[4] = data.x-w2;
    y[4] = data.y+dh;
    x[5] = data.x-w2;
    y[5] = data.y-dh;
    x[6] = data.x-dw;
    y[6] = data.y-dh;
    x[7] = data.x-dw;
    y[7] = data.y-h2;
    x[8] = data.x+dw;
    y[8] = data.y-h2;
    x[9] = data.x+dw;
    y[9] = data.y-dh;
    x[10] = data.x+w2;
    y[10] = data.y-dh;
    x[11] = data.x+w2;
    y[11] = data.y+dh;
  	for(int i=0;i<12;i++)
    {
    	vuRotatePoint(&x[i],&y[i],&z,data.x,data.y,data.z,data.rotation,CAD_PLANE_XY);
    }
	CadDrawGenArc(x[0],y[0],data.z,r,data.rotation+0,shp_PI2,CAD_GL_CURVEQUALITY);
	CadDrawPolyline();
	CadDrawLine(x[1],y[1],0,x[2],y[2],0);
	CadDrawGenArc(x[3],y[3],data.z,r,data.rotation+shp_PI2,shp_PI2,CAD_GL_CURVEQUALITY);
	CadDrawPolyline();
	CadDrawLine(x[4],y[4],0,x[5],y[5],0);
	CadDrawGenArc(x[6],y[6],data.z,r,data.rotation+shp_PI,shp_PI2,CAD_GL_CURVEQUALITY);
	CadDrawPolyline();
	CadDrawLine(x[7],y[7],0,x[8],y[8],0);
	CadDrawGenArc(x[9],y[9],data.z,r,data.rotation+3*shp_PI2,shp_PI2,CAD_GL_CURVEQUALITY);
	CadDrawPolyline();
	CadDrawLine(x[10],y[10],0,x[11],y[11],0);

    r = data.InRadius;
    w2 = (data.width-2*data.dx)/2.0;
    h2 = (data.height-2*data.dy)/2.0;
    dw = w2-r;
    dh = h2-r;
    x[0] = data.x+dw;
    y[0] = data.y+dh;
    x[1] = data.x+dw;
    y[1] = data.y+h2;
    x[2] = data.x-dw;
    y[2] = data.y+h2;
    x[3] = data.x-dw;
    y[3] = data.y+dh;
    x[4] = data.x-w2;
    y[4] = data.y+dh;
    x[5] = data.x-w2;
    y[5] = data.y-dh;
    x[6] = data.x-dw;
    y[6] = data.y-dh;
    x[7] = data.x-dw;
    y[7] = data.y-h2;
    x[8] = data.x+dw;
    y[8] = data.y-h2;
    x[9] = data.x+dw;
    y[9] = data.y-dh;
    x[10] = data.x+w2;
    y[10] = data.y-dh;
    x[11] = data.x+w2;
    y[11] = data.y+dh;
  	for(int i=0;i<12;i++)
    {
    	vuRotatePoint(&x[i],&y[i],&z,data.x,data.y,data.z,data.rotation,CAD_PLANE_XY);
    }
	CadDrawGenArc(x[0],y[0],data.z,r,data.rotation+0,shp_PI2,CAD_GL_CURVEQUALITY);
	CadDrawPolyline();
	CadDrawLine(x[1],y[1],0,x[2],y[2],0);
	CadDrawGenArc(x[3],y[3],data.z,r,data.rotation+shp_PI2,shp_PI2,CAD_GL_CURVEQUALITY);
	CadDrawPolyline();
	CadDrawLine(x[4],y[4],0,x[5],y[5],0);
	CadDrawGenArc(x[6],y[6],data.z,r,data.rotation+shp_PI,shp_PI2,CAD_GL_CURVEQUALITY);
	CadDrawPolyline();
	CadDrawLine(x[7],y[7],0,x[8],y[8],0);
	CadDrawGenArc(x[9],y[9],data.z,r,data.rotation+3*shp_PI2,shp_PI2,CAD_GL_CURVEQUALITY);
	CadDrawPolyline();
	CadDrawLine(x[10],y[10],0,x[11],y[11],0);
}
//---------------------------------------------------------------------------
void cad_AddFrame(VDWG hDwg,cad_Frame& data)
{
	int hEntity;
//	hEntity = CadAddCustom(hDwg,CAD_TYPE_FRAME,&data,sizeof(data));
	// Outter rectangle
	hEntity = CadAddRect(hDwg,data.x,data.y,data.z,data.width,data.height,data.rotation);
	CadRectPutRadius(hEntity,data.OutRadius);	
    EntCreateProc(hDwg,hEntity);
	// Inner rectangle
	hEntity = CadAddRect(hDwg,data.x,data.y,data.z,data.width-2*data.dx,data.height-2*data.dy,data.rotation);
	CadRectPutRadius(hEntity,data.InRadius);
    EntCreateProc(hDwg,hEntity);

    CadPutDirty(hDwg,true);
}
//---------------------------------------------------------------------------
void cad_AddFrame(VDWG hDwg,double x,double y,double width,double height,double dx,double dy,double InRadius,double OutRadius,double rotation)
{
	cad_Frame data;
	data.x = x;
    data.y = y;
    data.z = 0.0;
    data.width = width;
    data.height = height;
    data.dx = dx;
    data.dy = dy;
    data.InRadius = InRadius;
    data.OutRadius = OutRadius;
    data.rotation = rotation;
	cad_AddFrame(hDwg,data);
}
//---------------------------------------------------------------------------
int cad_AddSpiral(VDWG hDwg,cad_Spiral &data)
{
	if(data.dStep <= 0.0)	return -1;
    // Draw
    double r;
    double x,y;
    double t=0.0;
    int n = data.dTours*360.0/data.dStep;
    CadClearVertices();
    for(int i=0;i<=n+1;i++)
    {
    	r = data.a*t*t/1000000.0+data.v*t/1000.0+data.r0;
		x = data.x+r*cos(DegToRad(t));
		y = data.y+r*sin(DegToRad(t));
    	CadAddVertex(x,y,data.z);
        if(i < 1)
	        t += 0.1;
        else
        if(i < n)
	        t += data.dStep;
        else
	        t += 0.1;
    }
	int hEntity;
    if(data.bLines)
	    hEntity = CadAddPolyline(hDwg,CAD_PLINE_LINEAR,false);
    else
	    hEntity = CadAddPolyline(hDwg,CAD_PLINE_SPLINE,false);
    EntCreateProc(hDwg,hEntity);
    CadPutDirty(hDwg,true);
    return hEntity;
}
//---------------------------------------------------------------------------

