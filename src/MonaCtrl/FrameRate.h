//***************************************************************************//
//* File Name: frameRate.hpp                                                *//
//*    Author: Chris McBride chris_a_mcbride@hotmail.com                    *//
//* Date Made: 04/06/02                                                     *//
//* File Desc: Frame rate counter for any project, windows specific.        *//
//* Rev. Date: 11/26/02                                                     *//
//* Rev. Desc: Added frame rate independence and UNIX functionality (TEP)   *//
//*                                                                         *//
//***************************************************************************//
#ifndef __FRAMERATE_HPP__
#define __FRAMERATE_HPP__

#include <time.h>
#include <Mona/Mona.h>

class FrameRate
{
public:

   // Frame rate recalculation frequency (secs).
   enum { FRAME_RECALC_FREQUENCY=2 };

   double        targetFPS;                        // Target frames per second (FPS).
   double        FPS;                              // Current FPS.
   double        speedFactor;                      // Frame rate independence speed factor.
   static double initialSpeedFactor;
   static double maxSpeedFactor;

   // Constructor.
   FrameRate(double targetFPS);

   // Update: call per frame.
   void Update();

   // Reset.
   void reset();

private:

   int  m_frameCount;
   Time m_lastTime;
};
#endif
