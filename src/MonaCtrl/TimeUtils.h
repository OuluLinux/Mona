#ifndef Time_UTILS_H
#define Time_UTILS_H

#include "XMLUtils.h"

#ifndef WIN32
#define __int64 long long
#endif

/*******************************************************************************************/
/*Timer                                                                                    */
/*                                                                                         */
/*******************************************************************************************/

class Timer
{
  public:
    static void intialize();

  /*
   *Read the system's current time and return it
   **/
    static __int64 getCurrentTime();

   /*
    *Return the timer's precision (frequency)
    **/
    static __int64 getPrecision();

   /*
    *Return the elapsed time in seconds
    **/
    static double getElapsedTimeSeconds(__int64 lastTime);
};

/*******************************************************************************************/
/*FPSCounter                                                                               */
/*                                                                                         */
/*******************************************************************************************/

class FPSCounter
{
  public:
    FPSCounter();

    const double getFrameInterval() const;
    const double getElapsedTime()   const;
    const double getFPS()           const;

    virtual void  markFrameStart();
    virtual void  markFrameEnd();

  protected:
    __int64  frameStart;       // Mark the beginning of a frame
    double    frameInterval,    //The delay between two frames
             elapsedTime,
             internalFPS,      //An internal FPS counter
             tickCounter,      //This will count the clock ticks
             fps;              //The number of frames per second

};

/*******************************************************************************************/
/*Benchmark                                                                                */
/*                                                                                         */
/*******************************************************************************************/

class Benchmark : public FPSCounter, IOXMLObject
{
  private:
    String  logFilePath;
    double        framesCounter,
                 elapsedTime,
                 averageFPS,
                 duration,
                 minFPS,
                 maxFPS;
     bool        enabled;

  public:
    Benchmark(const String &logFilePath = "Results.xml");

    void    setLogFilePath(const String &logFilePath);
    const   String &getLogFilePath() const;

    void    setDuration(float);

    virtual bool exportXMLSettings(const String &xmlPath);
    virtual bool LoadXMLSettings(const TiXmlElement *element);

    virtual void  markFrameEnd();

    double  getTotalDuration() const;
    double  getAverageFrames() const;
    double  getTotalFrames()   const;
    double  getDuration()      const;
    double  getMinFPS()        const;
    double  getMaxFPS()        const;

    bool    exportResults();
    void    setEnabled(bool);

    bool    isEnabled()  const;
    bool    isRunning()  const;
};

#endif
