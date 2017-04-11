#if 0

//***************************************************************************//
//* File Name: frameRate.cpp                                                *//
//*    Author: Chris McBride chris_a_mcbride@hotmail.com                    *//
//* Date Made: 04/06/02                                                     *//
//* File Desc: Frame rate counter for any project, windows specific.        *//
//* Rev. Date: 11/26/02                                                     *//
//* Rev. Desc: Added frame rate independence and UNIX functionality (TEP)   *//
//*                                                                         *//
//***************************************************************************//

#include "FrameRate.h"

// Initial speed factor.
double FrameRate::initialSpeedFactor = 0.0;

// Maximum speed factor.
double FrameRate::maxSpeedFactor = 5.0;

// Constructor.
FrameRate::FrameRate(double targetFPS) {
	this->targetFPS = targetFPS;
	FPS             = targetFPS;
	speedFactor     = initialSpeedFactor;
	m_frameCount    = 0;
	m_lastTime      = gettime();
}


// Update: call per frame.
void FrameRate::Update() {
	Time currentTime, delta;
	// Count the frame.
	m_frameCount++;
	// Get the time delta.
	currentTime = gettime();
	delta       = (currentTime - m_lastTime) / 1000;

	// Time to recalculate frame rate?
	if (delta >= FRAME_RECALC_FREQUENCY) {
		// Calculate new values.
		FPS = (float)m_frameCount / (float)delta;

		if (FPS > 0.0f)
			speedFactor = targetFPS / FPS;
		else
			speedFactor = 0.0;

		if (speedFactor > maxSpeedFactor)
			speedFactor = maxSpeedFactor;

		m_frameCount = 0;
		m_lastTime   = currentTime;
	}
}


// Reset.
void FrameRate::reset() {
	FPS          = targetFPS;
	speedFactor  = initialSpeedFactor;
	m_frameCount = 0;
	m_lastTime   = gettime();
}
#endif
