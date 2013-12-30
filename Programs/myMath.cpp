#pragma once
#include <iostream>
#include <math.h>

#define G_PI 3.14159265358979323846f

typedef class POINT3D{
public:
	double x, y, z;
	double len;
	int whichWin;
	
	POINT3D(double tmpX = 0.0, double tmpY = 0.0, double tmpZ = 0.0){
		x = tmpX;
		y = tmpY;
		z = tmpZ;
	}


	// Operator Overloading
	POINT3D operator+(const POINT3D &pt1){
		POINT3D res;
		res.x = pt1.x + x;
		res.y = pt1.y + y;
		res.z = pt1.z + z;
		return res;
	}
	POINT3D operator-(const POINT3D &pt1){
		POINT3D res;
		res.x = x - pt1.x;
		res.y = y - pt1.y;
		res.z = z - pt1.z;
		return res;
	}
	POINT3D operator/(const double scale){
		POINT3D res;
		res.x = x / scale;
		res.y = y / scale;
		res.z = z / scale;
		return res;
	}
	POINT3D operator*(const double scale){
		POINT3D res;
		res.x = x * scale;
		res.y = y * scale;
		res.z = z * scale;
		return res;
	}
	bool operator==(const POINT3D &pt1){
		double threshold = 0.1;

		// if( ((pt1.x - threshold) <= x)&&(x <= (pt1.x + threshold)) ){
		// 	if( ((pt1.y - threshold) <= y)&&(y <= (pt1.y + threshold)) ){
		// 		if( ((pt1.z - threshold) <= z)&&(z <= (pt1.z + threshold)) )
		// 			return true;
		// 	}
		// }
		if( (pt1.x == x)&&(pt1.y == y)&&(pt1.z == z) ){
			return true;
		}
		return false;
	}
	// Overloading End
	void print(){
		printf("%lf %lf %lf\n", x, y, z);
	}
	void print(char *c){
		printf("[%s] %lf %lf %lf\n", c, x, y, z);
	}
	void getArr(float arr[3]){
		arr[0] = x;
		arr[1] = y;
		arr[2] = z;
	}
	void putArr(float arr[3]){
		x = arr[0];
		y = arr[1];
		z = arr[2];
	}
	inline void inverse(){
		x = -x;
		y = -y;
		z = -z;
	}
	void operator()(double tmpX, double tmpY, double tmpZ=0.0){
		x = tmpX;
		y = tmpY;
		z = tmpZ;
		len = sqrt(x*x + y*y + z*z);
	}
	double dot(POINT3D vec){
		double up = (x*vec.x + y*vec.y + z*vec.z);
		//double down = len*vec.len;
		return up;
	}
	inline double detXY(POINT3D temp){
		return( x*temp.x - y*temp.y );
	}
	void rot(int thx, int thy){
		double rx = thx * G_PI / 180.0;
		double ry = thx * G_PI / 180.0;
		
		//rot x
		y = y*cos(rx) - z*sin(rx);
		z = y*sin(rx) + z*cos(rx);
		//rot y
		x = x*cos(ry) - z*sin(ry);
		z = x*sin(ry) + z*cos(ry);
	}
	POINT3D cross(POINT3D node){
		POINT3D temp;
		temp((y*node.z) - (z*node.y), 
			 (z*node.x) - (x*node.z), 
			 (x*node.y) - (y*node.x));
		return temp;
	}
	POINT3D findVertex(POINT3D p0, double t){
		POINT3D temp;

		temp.x = p0.x + x*t;
		temp.y = p0.y + y*t;
		temp.z = p0.z + z*t;
		return temp;
	}
	inline void normalize(){
		double invMag = 1.0/magnitude();
		x *= invMag;
		y *= invMag;
		z *= invMag;
	}
	inline double magnitude(){
		return sqrtf( x*x + y*y + z*z );
	}
}POINT3D, VECTOR3D;



typedef class PLANE{
public:
	double para[4];		//x, y, z, const
	PLANE(){
		initial();
	}
	void initial(){
		for( int i=0 ; i<4 ; i++ )
			para[i] = 0.0;
	}
	double length(){
		// calculate normal vector length
		return sqrt(para[0]*para[0] + para[1]*para[1] + para[2]*para[2]);
	}
	void normalize(){
		double l = length();
		
		for( int i=0 ; i<4 ; i++ )
			para[i] /= l;
	}
	inline int ptPosition(double ptX, double ptY, double ptZ){
		double tmp = ptX*para[0] + ptY*para[1] + ptZ*para[2] + para[3];

		if( tmp == 0 )
			return 0;
		else if( tmp > 0 )
			return 1;
		else
			return -1;
	}
	inline double distance(double ptX, double ptY, double ptZ){
		return (ptX*para[0] + ptY*para[1] + ptZ*para[2] + para[3]);
	}
	template <class disType>
	inline disType distance(disType pt[3]){
		disType res = (pt[0]*para[0] + pt[1]*para[1] + pt[2]*para[2] + para[3]);
		return (res<0) ? -res : res;
	}

	void printer(){
		printf("[plane] %.3lf x + %.3lf y + %.3lf z = %.3lf\n", para[0], para[1], para[2], -para[3]);
	}
	
	template <class ptType>
	void pointOntPlane(ptType p1[3], ptType p2[3], int which){
		ptType v[3] = {p2[0] - p1[0], p2[1] - p1[1], p2[2] - p1[2]};
		
		float t = -distance(p1)/(distance(v)-para[3]);
		for( int i=0 ; i<3 ; i++ ){
			if( which == 0 )	// replace p1
				p1[i] = p1[i] + t*v[i];
			else				// replace p2
				p2[i] = p1[i] + t*v[i];
		}
	}
	template <class ptType>
	void projectToPlane(ptType out[3], ptType in[3]){
		float t = -( in[0]*para[0]+in[1]*para[1]+in[2]*para[2]+para[3] )/( para[0]*para[0]+para[1]*para[1]+para[2]*para[2] );

		for( int i=0 ; i<3 ; i++ )
			out[i] = in[i] + para[i]*t;
	}
	// () operator overloading
	void operator()(double getX, double getY, double getZ, double getCst){
		para[0] = getX;
		para[1] = getY;
		para[2] = getZ;
		para[3] = getCst;
		
		normalize();
	}
	void operator()(double getArr[4]){
		for( int i=0 ; i<4 ; i++ )
			para[i] = getArr[i];
	}
}PLANE;

