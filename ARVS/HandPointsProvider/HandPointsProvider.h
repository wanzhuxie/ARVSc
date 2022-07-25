#ifndef HANDPOINTSPROVIDER_H
#define HANDPOINTSPROVIDER_H

#include "handpointsprovider_global.h"
#include <vector>
#include <iostream>
#include <string>
#include "Matrices.h"
#include "Plane.h"
using namespace std;


struct Point3D
{
	float X;
	float Y;
	float Z;

	Point3D(float dX=0, float dY=0, float dZ=0)
	{
		X=dX;
		Y=dY;
		Z=dZ;
	}

	float DistanceTo(const Point3D & mOtherPos)
	{
		return sqrtf((this->X - mOtherPos.X)*(this->X - mOtherPos.X)+(this->Y - mOtherPos.Y)*(this->Y - mOtherPos.Y));
	}

	float DistanceTo3D(const Point3D & mOtherPos)
	{
		return sqrtf((this->X - mOtherPos.X)*(this->X - mOtherPos.X)+(this->Y - mOtherPos.Y)*(this->Y - mOtherPos.Y)+(this->Z - mOtherPos.Z)*(this->Z - mOtherPos.Z));
	}
};
struct Vector2D
{
	float X;
	float Y;

	Vector2D(float dX=0, float dY=0)
	{
		X=dX;
		Y=dY;
	}

};

class HANDPOINTSPROVIDER_EXPORT HandPointsProvider
{
public:
	HandPointsProvider();
	~HandPointsProvider();

	//===function===
	bool Init();
	bool Close();
	vector<Point3D> GetAllHandPoints();
	float GetHandDiameter(float & dDiff=*(new float(0)));
	Point3D GetTipOfIndexFinger(bool bNewest=true);
	int GetGestureResult(bool bNewest=true);
	//transform to plam box
	bool GetTransfPoint();

	std::string GetFingerState(bool bNewest=true);
	vector<float> GetDistanceToPlamPlane();

	float GetVectorAngle(const Vector2D& vec1, const Vector2D& vec2);
	bool GetHandCount();

private:
	//===data===
	float _dHandDiameter;
	vector<Point3D> _vecHandPoints;

	Plane _plamPlane;
	bool _bRightHand;

};

#endif // HANDPOINTSPROVIDER_H
