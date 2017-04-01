#ifndef __QUATERNION_HPP__
#define __QUATERNION_HPP__

#include "Vector.h"

class CQuaternion {
public:

	// Quaternion.
	double m_quat[4];

	// Constructors.
	CQuaternion();
	CQuaternion(double quat[4]);

	// Initialize.
	void Init(double quat[4]);

	// Clear.
	void Clear();

	// Vector3f operations.
	void VecZero(double*);
	void VecSet(double*, double, double, double);
	void VecSub(const double*, const double*, double*);
	void VecCopy(const double*, double*);
	void VecCross(const double*, const double*, double*);
	double VecLength(const double*);
	void VecScale(double*, double);
	void VecNormal(double*);
	double VecDot(const double*, const double*);
	void VecAdd(const double*, const double*, double*);

	// Quaternion operations
	void AddQuats(CQuaternion& q1, CQuaternion& q2, CQuaternion& dest);
	void MulQuats(CQuaternion& q1, CQuaternion& q2, CQuaternion& dest);
	void NormalizeQuat();
	void BuildRotationMatrix(double m[4][4]); // Build a rotation matrix given a quaternion rotation.
	void LoadRotation(double angle, double* axis); // Load an axis-angle rotation into quaternion.
	void MergeRotation(double angle, double* axis); // Merge an axis-angle rotation into quaternion.
	void MakeQFromEulerAngles(double pitch, double yaw, double roll); // Make quaternion from Euler angles.
	void GetEulerAngles(double& pitch, double& yaw, double& roll); // Get Euler angles from quaternion.
	CQuaternion* Clone(); // Clone.

	// Load and save.
	void Serialize(Stream& fp);

private:

	int m_normal_count;                             // Normalization counter.
};
#endif
