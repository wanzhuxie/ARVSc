#include "HandPointsProvider.h"
#include "GeneralFunctions.h"
#include <io.h>
#include <fstream>

HandPointsProvider::HandPointsProvider(void)
{

}
HandPointsProvider::~HandPointsProvider(void)
{
}

bool HandPointsProvider::Init()
{
	if (_access("Data\\CloseHandPos" , 0)==0)
	{
		rename("Data\\CloseHandPos" , "Data\\NoCloseHandPos");
	}

	if (_access("Data\\GiveHandPos.py" , 0)==0)
	{
		system("start D:\\ProgramData\\Anaconda3\\pythonw Data\\GiveHandPos.py");
	}

	return true;
}
bool HandPointsProvider::Close()
{
	if (_access("Data\\NoCloseHandPos" , 0)==0)
	{
		rename("Data\\NoCloseHandPos" , "Data\\CloseHandPos");
	}

	return true;
}

vector<Point3D> HandPointsProvider::GetAllHandPoints()
{
	vector<Point3D> vecThisHandPos;
	ifstream ifs;
	ifs.open("Data\\HandPos.txt");
	if (ifs.is_open())
	{
		char buffer[512];
		ifs.read(buffer , 512);

		string strHandPos=buffer;
		vector<string> listData= SplitString(strHandPos,"\n");
		if (listData.size()>=21)
		{
			for (int a=0;a<21;a++)
			{
				vector<string> listOnePosData = SplitString(listData[a]," ");
				if (listOnePosData.size()==4)
				{
					int iId=atoi(listOnePosData[0].c_str());
					float iX=atof(listOnePosData[1].c_str());
					float iY=atof(listOnePosData[2].c_str());
					float iZ=atof(listOnePosData[3].c_str());
					vecThisHandPos.push_back(Point3D(iX , iY , iZ));
				}
			}
		}
		ifs.close();
	}

	//cout<<"all hand pos..."<<endl;
	//for (int a=0;a<vecThisHandPos.size();a++)
	//{
	//	cout<<a<<"	"<<vecThisHandPos[a].X<<"	"<<vecThisHandPos[a].Y<<endl;
	//}

	_vecHandPoints=vecThisHandPos;

	return _vecHandPoints;
}

Point3D HandPointsProvider::GetTipOfIndexFinger(bool bNewest)
{
	if (bNewest)
	{
		GetAllHandPoints();
	}
	if (_vecHandPoints.size()==21)
	{
		return _vecHandPoints[8];
	}

	return Point3D(-1,-1);
}

std::string HandPointsProvider::GetFingerState(bool bNewest)
{
	std::string iResult="";
	if (bNewest)
	{
		GetAllHandPoints();
	}

	if (_vecHandPoints.size() != 21)
		return iResult;


	//tip's index: 4, 8, 12, 16, 20
	vector<int> vecFingerTipIndex;
	vecFingerTipIndex.push_back(4);
	vecFingerTipIndex.push_back(8);
	vecFingerTipIndex.push_back(12);
	vecFingerTipIndex.push_back(16);
	vecFingerTipIndex.push_back(20);

	int iTipIndex=-1;

	//left or right
	bool bRight=false;
	if (_vecHandPoints[2].X < _vecHandPoints[17].X)
	{
		bRight=true;
	}
	else
	{
		bRight=false;
	}

	////Palm
	//int iPalmHight=_vecHandPoints[0].DistanceTo(_vecHandPoints[9]);
	//int iPalmWidth=_vecHandPoints[5].DistanceTo(_vecHandPoints[17]);
	//int iThumbLen=_vecHandPoints[2].DistanceTo(_vecHandPoints[4]);
	////²àÕÆ£¿
	//bool bSidePalm=false;
	//if (iPalmWidth<iPalmHight/2)
	//{
	//	bool bSidePalm=true;
	//}
	//else
	//{
	//	bool bSidePalm=false;
	//}


	//Thumb
	iTipIndex=vecFingerTipIndex[0];
	if (bRight 
		&& _vecHandPoints[iTipIndex].X < _vecHandPoints[iTipIndex-2].X
		&& _vecHandPoints[iTipIndex].X < _vecHandPoints[17].X)
	{
		iResult="1";
	}
	else	if (!bRight 
		&& _vecHandPoints[iTipIndex].X > _vecHandPoints[iTipIndex-2].X
		&& _vecHandPoints[iTipIndex].X > _vecHandPoints[17].X)
	{
		iResult="1";
	}
	else
	{
		iResult="0";
	}

	//Other four
	for (int a=1;a<5;a++)
	{
		iTipIndex=vecFingerTipIndex[a];
		if (_vecHandPoints[iTipIndex].Y < _vecHandPoints[iTipIndex-2].Y)
		{
			iResult+="1";
		}
		else
		{
			iResult+="0";
		}
	}

	return iResult;
}

bool HandPointsProvider::GetHandCount()
{
	return _vecHandPoints.size() / 21 ;
}


float HandPointsProvider::GetHandDiameter(float & dDiff)
{
	vector<Point3D> vecThisHandPos=GetAllHandPoints();

	float xMin=9999,xMax=-9999,yMin=9999,yMax=-9999;
	for (int a=0;a<vecThisHandPos.size();a++)
	{
		if (vecThisHandPos[a].X<xMin) xMin=vecThisHandPos[a].X;
		if (vecThisHandPos[a].X>xMax) xMax=vecThisHandPos[a].X;
		if (vecThisHandPos[a].Y<yMin) yMin=vecThisHandPos[a].Y;
		if (vecThisHandPos[a].Y>yMax) yMax=vecThisHandPos[a].Y;
	}

	float iLength=xMax-xMin;
	float iWidth=yMax-yMin;
	float dCurDia=sqrtf(iLength*iLength+iWidth*iWidth);
	dDiff=dCurDia-_dHandDiameter;
	_dHandDiameter=dCurDia;
	return _dHandDiameter;
}
float HandPointsProvider::GetVectorAngle(const Vector2D& vec1, const Vector2D& vec2)
{
	float t = (vec1.X * vec2.X + vec1.Y * vec2.Y) / (sqrt(pow(vec1.X, 2) + pow(vec1.Y, 2)) * sqrt(pow(vec2.X, 2) + pow(vec2.Y, 2)));
	float angle = acos(t) * (180 / PI);
	return angle;
}

//transform to plam box
vector<float> HandPointsProvider::GetDistanceToPlamPlane()
{
	Vector3 vec0_05(_vecHandPoints[5].X-_vecHandPoints[0].X , _vecHandPoints[5].Y-_vecHandPoints[0].Y,_vecHandPoints[5].Z-_vecHandPoints[0].Z);
	Vector3 vec0_09(_vecHandPoints[9].X-_vecHandPoints[0].X , _vecHandPoints[9].Y-_vecHandPoints[0].Y,_vecHandPoints[9].Z-_vecHandPoints[0].Z);
	Vector3 vec0_13(_vecHandPoints[13].X-_vecHandPoints[0].X , _vecHandPoints[13].Y-_vecHandPoints[0].Y,_vecHandPoints[13].Z-_vecHandPoints[0].Z);
	Vector3 vec0_17(_vecHandPoints[17].X-_vecHandPoints[0].X , _vecHandPoints[17].Y-_vecHandPoints[0].Y,_vecHandPoints[17].Z-_vecHandPoints[0].Z);

	Vector3 mNomal;
	if (_bRightHand)
	{
		mNomal=vec0_05.cross(vec0_17);
	}
	else
	{
		mNomal=vec0_17.cross(vec0_05);
	}

	Plane mPlane(mNomal ,Vector3(_vecHandPoints[0].X,_vecHandPoints[0].Y,_vecHandPoints[0].Z) );

	vector<float> vecDistance;
	for (int a=0;a<_vecHandPoints.size();a++)
	{
		vecDistance.push_back(mPlane.getDistance(Vector3(_vecHandPoints[a].X,_vecHandPoints[a].Y,_vecHandPoints[a].Z)));
	}


	return vecDistance;
}
