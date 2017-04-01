//***************************************************************************//
//* File Name: baseObject.hpp                                               *//
//* Author:    Tom Portegys, portegys@ilstu.edu                             *//
//* Date Made: 07/25/02                                                     *//
//* File Desc: Class declaration representing a basic object.               *//
//* Rev. Date:                                                              *//
//* Rev. Desc:                                                              *//
//*                                                                         *//
//***************************************************************************//

#ifndef __BASE_OBJECT_HPP__
#define __BASE_OBJECT_HPP__

#include <Mona/Mona.h>
#include <GL/glut.h>
#include "Spacial.h"

class BaseObject {
public:

	// Constructor.
	BaseObject() {
		m_spacial = new cSpacial();
		ASSERT(m_spacial != NULL);
	}


	// Destructor.
	~BaseObject() {
		delete m_spacial;
	}


	// Set spacial state.
	cSpacial* getSpacial() {
		return (m_spacial);
	}

	// Set spacial state.
	void setSpacial(GLdouble pitch, GLdouble yaw, GLdouble roll,
					GLdouble pitchRate, GLdouble yawRate, GLdouble rollRate,
					GLdouble x, GLdouble y, GLdouble z, GLdouble scale, GLdouble speed);
	void setSpacial(cSpacial* spacial);

	// Clear spacial.
	void clearSpacial() {
		m_spacial->Clear();
	}

	// Rotations.
	GLdouble getPitch() {
		return (m_spacial->getPitch());
	}
	GLdouble getYaw()   {
		return (m_spacial->getYaw());
	}
	GLdouble getRoll()  {
		return (m_spacial->getRoll());
	}
	void setPitch(GLdouble pitch) {
		m_spacial->setPitch(pitch);
	}
	void setYaw(GLdouble yaw) {
		m_spacial->setYaw(yaw);
	}
	void setRoll(GLdouble roll) {
		m_spacial->setRoll(roll);
	}
	void addPitch(GLdouble pitch) {
		m_spacial->setPitch(m_spacial->getPitch() + pitch);
	}


	void addYaw(GLdouble yaw) {
		m_spacial->setYaw(m_spacial->getYaw() + yaw);
	}


	void addRoll(GLdouble roll) {
		m_spacial->setRoll(m_spacial->getRoll() + roll);
	}


	// Get direction vectors.
	void getRight(GLdouble* v)   {
		m_spacial->getRight(v);
	}
	void getUp(GLdouble* v)      {
		m_spacial->getUp(v);
	}
	void getForward(GLdouble* v) {
		m_spacial->getForward(v);
	}

	// Position.
	void getPosition(GLdouble* v) {
		v[0] = m_spacial->getX();
		v[1] = m_spacial->getY();
		v[2] = m_spacial->getZ();
	}


	void setPosition(GLdouble* v) {
		m_spacial->setX(v[0]);
		m_spacial->setY(v[1]);
		m_spacial->setZ(v[2]);
	}


	// Scale.
	GLdouble getScale() {
		return (m_spacial->getScale());
	}
	void setScale(GLdouble scale) {
		m_spacial->setScale(scale);
	}

	// Speed.
	GLdouble getSpeed() {
		return (m_spacial->getSpeed());
	}
	void setSpeed(GLdouble speed) {
		m_spacial->setSpeed(speed);
	}
	void addSpeed(GLdouble speed) {
		m_spacial->setSpeed(m_spacial->getSpeed() + speed);
	}


	// Update.
	void Update() {
		m_spacial->Update();
	}

	// Get model transformation matrix.
	void getModelTransform(GLdouble* matrix) {
		m_spacial->getModelTransform(matrix);
	}


	// Get world coordinates from local.
	void localToWorld(GLdouble* local, GLdouble* world) {
		m_spacial->localToWorld(local, world);
	}


	// Transform local point.
	void transformPoint(GLdouble* point) {
		m_spacial->transformPoint(point);
	}


	// Inverse transform local point.
	void inverseTransformPoint(GLdouble* point) {
		m_spacial->inverseTransformPoint(point);
	}


	// Get billboard (face toward) rotation to target point given in local coordinates.
	// Return axis and angle for rotation to accomplish billboard.
	// rotation[0-2]=axis, rotation[3]=angle
	void getBillboard(GLdouble* target, GLdouble* rotation) {
		m_spacial->getBillboard(target, rotation);
	}


	// Get billboard (face toward) rotation from source to target vectors.
	// Return axis and angle for rotation to accomplish billboard.
	// rotation[0-2]=axis, rotation[3]=angle
	void getBillboard(GLdouble* target, GLdouble* source, GLdouble* rotation) {
		m_spacial->getBillboard(target, source, rotation);
	}


	// Load an axis-angle rotation into quaternion.
	void LoadRotation(GLdouble angle, GLdouble* axis) {
		m_spacial->LoadRotation(angle, axis);
	}


	// Merge an axis-angle rotation into quaternion.
	void MergeRotation(GLdouble angle, GLdouble* axis) {
		m_spacial->MergeRotation(angle, axis);
	}


	// Draw axes.
	void drawAxes(GLdouble span = 1.0f);

	// Set block vertices.
	static void setBlockVertices(Vector3f* vertices, double xmin, double xmax, double ymin, double ymax, double zmin, double zmax);

	// Draw a block.
	static void drawBlock(double xmin, double xmax, double ymin, double ymax, double zmin, double zmax);

	// Draw block edges.
	static void drawBlockEdges(double xmin, double xmax, double ymin, double ymax, double zmin, double zmax);

	// Load object.
	void Load(String filename);
	void Serialize(Stream& fp);

	// Save object.
	void Store(String filename);
	void Store(Stream& fp);

	// Spacial properties.
	cSpacial* m_spacial;
};
#endif
