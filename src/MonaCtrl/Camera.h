//***************************************************************************//
//* File Name: camera.hpp                                                   *//
//* Author:    Tom Portegys, portegys@ilstu.edu                             *//
//* Date Made: 07/25/02                                                     *//
//* File Desc: Class declaration representing a camera and frustum.         *//
//* Rev. Date:                                                              *//
//* Rev. Desc:                                                              *//
//*                                                                         *//
//***************************************************************************//

#ifndef __CAMERA_HPP__
#define __CAMERA_HPP__

#include "BaseObject.h"

class Camera : public BaseObject {
public:

	// Constructor.
	Camera() {
		cameraHistorySize  = 1;
		cameraHistory      = new struct CameraHistory[cameraHistorySize];
		cameraHistoryIndex = 0;
		extractFrustum();
	}


	// Destructor.
	~Camera() {
		delete cameraHistory;
	}


	// Set viewing frustum.
	void setFrustum(GLdouble angle, GLdouble aspect,
					GLdouble znear, GLdouble zfar);

	// Set camera view.
	void setView(GLdouble* eye, GLdouble* lookat, GLdouble* up);

	// Update camera view.
	void updateView(GLdouble* eye, GLdouble* lookat, GLdouble* up);

	// Place camera.
	void place();

	// Set camera spring-loaded parameter.
	// 1=fixed, >=springier
	void setCameraSpringiness(int size);

	// Is local point inside frustum (visible)?
	bool pointInFrustum(GLdouble* localPoint);

private:

	// Camera vector history for springiness calculation.
	struct CameraHistory {
		GLdouble e[3];                               // eye
		GLdouble f[3];                               // forward
		GLdouble u[3];                               // up
	}
	* cameraHistory;
	int cameraHistorySize;
	int cameraHistoryIndex;

	// Frustum bounding planes.
	GLdouble frustum[6][4];

	// Extract frustum.
	void extractFrustum();
};
#endif
