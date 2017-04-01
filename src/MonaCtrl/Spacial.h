//***************************************************************************//
//* File Name: spacial.hpp                                                  *//
//* Author:    Tom Portegys, portegys@ilstu.edu                             *//
//* Date Made: 07/25/02                                                     *//
//* File Desc: Class declaration representing spacial properties:           *//
//*             rotation, translation, scale, and speed.                    *//
//* Rev. Date: 3/6/2008                                                     *//
//* Rev. Desc:                                                              *//
//*                                                                         *//
//***************************************************************************//

#ifndef __SPACIAL_HPP__
#define __SPACIAL_HPP__

#ifdef _WIN32
	#ifndef WIN32
		#define WIN32
	#endif
#endif

#ifdef WIN32
	#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <assert.h>
#include <Mona/Quaternion.h>
#include <Mona/Matrix.h>
#include <Mona/Vector.h>

#ifndef _NO_NAMESPACE
	using namespace std;
	using namespace math;
	#define STD    std
#else
	#define STD
#endif

#ifndef _NO_TEMPLATE
	typedef matrix<GLfloat>   Matrix;
#else
	typedef matrix            Matrix;
#endif

class cSpacial {
public:

	// Rotation.
	GLdouble     m_pitchRate, m_yawRate, m_rollRate;
	GLdouble     m_rotmatrix[4][4];
	CQuaternion* m_qcalc;

	// Position.
	GLdouble m_x, m_y, m_z;

	// Scale.
	GLdouble m_scale;

	// Speed.
	GLdouble m_speed;

	// Constructors.
	cSpacial() {
		m_qcalc = new CQuaternion();
		ASSERT(m_qcalc != NULL);
		Clear();
	}


	cSpacial(GLdouble pitch, GLdouble yaw, GLdouble roll,
			 GLdouble pitchRate, GLdouble yawRate, GLdouble rollRate,
			 GLdouble x, GLdouble y, GLdouble z, GLdouble scale, GLdouble speed) {
		m_qcalc = new CQuaternion();
		ASSERT(m_qcalc != NULL);
		initialize(pitch, yaw, roll, pitchRate, yawRate, rollRate,
				   x, y, z, scale, speed);
	}


	inline void Clear() {
		initialize(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	}


	void initialize(GLdouble pitch, GLdouble yaw, GLdouble roll,
					GLdouble pitchRate, GLdouble yawRate, GLdouble rollRate,
					GLdouble x, GLdouble y, GLdouble z, GLdouble scale, GLdouble speed);

	// Destructor.
	~cSpacial() {
		delete m_qcalc;
	}

	// Get and set Euler angles.
	GLdouble getPitch();
	GLdouble getYaw();
	GLdouble getRoll();
	void setPitch(GLdouble pitch);
	void setYaw(GLdouble yaw);
	void setRoll(GLdouble roll);

	// Get and set angular rates.
	inline GLdouble getPitchRate() {
		return (m_pitchRate);
	}
	inline GLdouble getYawRate() {
		return (m_yawRate);
	}
	inline GLdouble getRollRate() {
		return (m_rollRate);
	}
	inline void setPitchRate(GLdouble pitchRate) {
		m_pitchRate = pitchRate;
	}
	inline void setYawRate(GLdouble yawRate) {
		m_yawRate = yawRate;
	}
	inline void setRollRate(GLdouble rollRate) {
		m_rollRate = rollRate;
	}

	// Get direction vectors.
	void getRight(GLdouble* v);
	void getUp(GLdouble* v);
	void getForward(GLdouble* v);

	// Get and set position.
	inline GLdouble getX() {
		return (m_x);
	}
	inline GLdouble getY() {
		return (m_y);
	}
	inline GLdouble getZ() {
		return (m_z);
	}
	inline void setX(GLdouble x) {
		m_x = x;
	}
	inline void setY(GLdouble y) {
		m_y = y;
	}
	inline void setZ(GLdouble z) {
		m_z = z;
	}

	// Get and set scale.
	inline GLdouble getScale() {
		return (m_scale);
	}
	inline void setScale(GLdouble scale) {
		m_scale = scale;
	}

	// Get and set speed.
	inline GLdouble getSpeed() {
		return (m_speed);
	}
	inline void setSpeed(GLdouble speed) {
		m_speed = speed;
	}

	// Update rotation and translation state.
	void Update();

	// Get Euler angles.
	inline void GetEulerAngles(float& pitch, float& yaw, float& roll) {
		m_qcalc->GetEulerAngles(pitch, yaw, roll);
	}


	// Load an axis-angle rotation.
	inline void LoadRotation(GLdouble angle, GLdouble* axis) {
		m_qcalc->LoadRotation(DegreesToRadians(angle), axis);
		BuildRotationMatrix();
	}


	// Merge an axis-angle rotation.
	inline void MergeRotation(GLdouble angle, GLdouble* axis) {
		m_qcalc->MergeRotation(DegreesToRadians(angle), axis);
		BuildRotationMatrix();
	}


	// Get billboard rotation to given target point.
	void getBillboard(GLdouble* target, GLdouble* rotation);

	// Get billboard rotation from source to target vectors.
	void getBillboard(GLdouble* target, GLdouble* source, GLdouble* rotation);

	// Build rotation matrix from quaternion.
	inline void BuildRotationMatrix() {
		m_qcalc->BuildRotationMatrix(m_rotmatrix);
	}


	// Get model transformation matrix.
	void getModelTransform(GLdouble* matrix);

	// Get world coordinates from local.
	void localToWorld(GLdouble* local, GLdouble* world);

	// Transform local point.
	inline void transformPoint(GLdouble* point) {
		localToWorld(point, point);
	}


	// Inverse transform local point.
	void inverseTransformPoint(GLdouble* point);

	// Point-to-point distance.
	static GLdouble pointDistance(GLdouble* p1, GLdouble* p2);

	// Normalize vector.
	inline static void normalize(GLdouble* v) {
		GLdouble d = (sqrt((v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2])));
		v[0] = v[0] / d;
		v[1] = v[1] / d;
		v[2] = v[2] / d;
	}


	// Clone spacial.
	cSpacial* Clone();

	// Load and save.
	void Serialize(Stream& fp);
	void Store(Stream& fp);
};
#endif
