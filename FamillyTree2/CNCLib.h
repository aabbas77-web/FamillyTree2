//---------------------------------------------------------------------------
#ifndef CNCLibH
#define CNCLibH
//---------------------------------------------------------------------------
#include <vector.h>
//---------------------------------------------------------------------------
const double cnc_2PI = 6.283185307179586476925286766559;
//---------------------------------------------------------------------------
typedef enum{stTool_1=1,stTool_2,stTool_3,stTool_4,stTool_5,stTool_6,stTool_7,stTool_8,stTool_9,stTool_10,stJump,stNormal,stGotoZero,stEOF} cnc_StitchType;
//---------------------------------------------------------------------------
//const AnsiString c_strFloatMask = "0";
const AnsiString c_strFloatMask = "0.0000";
//---------------------------------------------------------------------------
struct cnc_Point
{
	double x,y;
};
//---------------------------------------------------------------------------
struct cnc_Stitch
{
	Byte cmd;// Command
	double x,y,z;// Position
    double Vx,Vy,Vz;//Speed
    double Ax,Ay,Az;// Acceleration
    double dDelayMS;// TimeOut for this segment in ms
};
//---------------------------------------------------------------------------
struct cnc_Packet// Size = 40 bytes
{
	DWORD cmd;// Command
	DWORD x,y,z;// Position
    DWORD Vx,Vy,Vz;//Speed
    DWORD Ax,Ay,Az;// Acceleration
};
//---------------------------------------------------------------------------
struct cnc_Scale
{
	double Ax,Bx;
    double Ay,By;
};
//---------------------------------------------------------------------------
struct cnc_Rect
{
	double left,top,right,bottom;
    double Width()
    {
     	return (right - left + 1.0);
    }
    double Height()
    {
     	return (bottom - top + 1.0);
    }
};
//---------------------------------------------------------------------------
typedef vector<cnc_Stitch> cnc_File;
//---------------------------------------------------------------------------
double cnc_Distance(cnc_Stitch &p1,cnc_Stitch &p2);
double cnc_Distance(cnc_Point &p1,cnc_Point &p2);
//---------------------------------------------------------------------------
bool cnc_LoadCMDFile(AnsiString strFileName,cnc_File &File);
//bool cnc_SaveCMDFile(AnsiString strFileName,cnc_File &File);
//---------------------------------------------------------------------------
cnc_Rect cnc_CalculateFrame(cnc_File &CNCFile);
void cnc_InitializeCNCFile(TRect &R,bool bKeepRatio,cnc_File &CNCFile);
void cnc_DrawCNCFile(TCanvas *pCanvas,TRect &R,bool bKeepRatio,cnc_File &CNCFile);
TPoint cnc_InitializeSimulator(TCanvas *pCanvas,TRect &R,bool bKeepRatio,cnc_File &CNCFile);
TPoint cnc_DrawStitch(TCanvas *pCanvas,cnc_Stitch &CNCStitch);
//---------------------------------------------------------------------------
extern double cnc_Ax;
extern double cnc_Bx;
extern double cnc_Ay;
extern double cnc_By;
//---------------------------------------------------------------------------
#endif
