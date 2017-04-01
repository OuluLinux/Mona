#include "TimeUtils.h"
#include "GenUtils.h"

/*******************************************************************************************/
/*Timer                                                                                    */
/*                                                                                         */
/*******************************************************************************************/

#ifdef WIN32
	#include <windows.h>

	LARGE_INTEGER currentTime,
	frequency;
#else
	#include <cstdlib>
	#include <sys/time.h>
	#include <unistd.h>
	#define secondsToMicro 1000000
#endif


void Timer::intialize() {
	#ifdef WIN32
	QueryPerformanceFrequency(&frequency);
	#endif
}


__int64 Timer::getCurrentTime() {
	#ifdef WIN32
	QueryPerformanceCounter(&currentTime);
	return currentTime.QuadPart;
	#else
	timeval internalTime;
	gettimeofday(&internalTime, NULL);
	return (__int64) internalTime.tv_sec * secondsToMicro + internalTime.tv_usec;
	#endif
}

__int64 Timer::getPrecision() {
	#ifdef WIN32
	return frequency.QuadPart;
	#else
	return secondsToMicro;
	#endif
}

double Timer::getElapsedTimeSeconds(__int64 lastTime) {
	#ifdef WIN32
	return float(getCurrentTime() - lastTime) / frequency.QuadPart;
	#else
	return float(getCurrentTime() - lastTime) / secondsToMicro;
	#endif
}

/*******************************************************************************************/
/*FPSCounter                                                                               */
/*                                                                                         */
/*******************************************************************************************/

FPSCounter::FPSCounter() {
	Timer::intialize();
	frameInterval = 0.0;
	internalFPS   = 0.0;
	tickCounter   = 0.0;
	elapsedTime   = 0.0;
	frameStart    =    0;
	fps           = 0.0;
}

const double FPSCounter::getFrameInterval() const {
	return frameInterval;
}
const double FPSCounter::getElapsedTime()   const {
	return elapsedTime;
}
const double FPSCounter::getFPS()           const {
	return fps;
}

void FPSCounter::markFrameStart() {
	frameStart = Timer::getCurrentTime();
}

void FPSCounter::markFrameEnd() {
	if (frameStart) {
		frameInterval = Timer::getElapsedTimeSeconds(frameStart);
		tickCounter  += frameInterval;
		elapsedTime  += frameInterval;
		internalFPS++;

		if (tickCounter >= 1.0f) {
			fps          = internalFPS / tickCounter;
			internalFPS  = 0.0;
			tickCounter  = 0.0;
		}
	}
}

/*******************************************************************************************/
/*Benchmark                                                                                */
/*                                                                                         */
/*******************************************************************************************/

Benchmark::Benchmark(const String& logFilePath) : FPSCounter(), IOXMLObject("Benchmark") {
	setLogFilePath(logFilePath);
	framesCounter  =   0.0;
	elapsedTime    =   0.0;
	averageFPS     =   0.0;
	duration       =  10.0;
	enabled        =   true;
	minFPS         = 10.0e5;
	maxFPS         =   0.0;
}

bool   Benchmark::LoadXMLSettings(const TiXmlElement* element) {
	if (!XMLArbiter::inspectElementInfo(element, "Benchmark"))
		return  Logger::writeErrorLog("NULL Benchmark node");

	setLogFilePath(element->Attribute("logFilePath"));
	setDuration(XMLArbiter::fillComponents1f(element, "duration", duration));
	setEnabled(XMLArbiter::analyzeBooleanAttr(element, "enabled", enabled));
	return true;
}

bool Benchmark::exportXMLSettings(const String& xmlPath) {
	if (!xmlPath.GetCount()) {
		if (!logFilePath.GetCount())
			return Logger::writeErrorLog("Cannot export Benchmark results -> NULL log file path");
	}
	else
		logFilePath = xmlPath;

	ofstream exporter(logFilePath.c_str());
	exporter << "<BMResults  framesCount = \"" << framesCounter << "\"\n"
			 << "            elapsedTime = \"" << elapsedTime   << "\"\n"
			 << "            duration    = \"" << duration      << "\"\n"
			 << "            averageFPS  = \"" << averageFPS    << "\"\n"
			 << "            minFPS      = \"" << minFPS        << "\"\n"
			 << "            maxFPS      = \"" << maxFPS        << "\"/>\n";
	exporter.close();
	return true;
}

void Benchmark::setLogFilePath(const String& lFilePath) {
	logFilePath = lFilePath.GetCount() ? lFilePath : logFilePath;
}

const String& Benchmark::getLogFilePath() const {
	return logFilePath;
}

void Benchmark::markFrameEnd() {
	if (frameStart) {
		frameInterval = Timer::getElapsedTimeSeconds(frameStart);
		tickCounter  += frameInterval;
		elapsedTime  += frameInterval * enabled;
		internalFPS++;

		if (tickCounter >= 1.0f) {
			fps          = internalFPS / tickCounter;
			internalFPS  = 0.0;
			tickCounter  = 0.0;
		}
	}

	framesCounter += 1.0;

	if (elapsedTime >= duration || !enabled)
		return;

	if (fps && elapsedTime) {
		averageFPS = framesCounter / elapsedTime;
		minFPS     = (fps < minFPS) ? fps : minFPS;
		maxFPS     = (fps > maxFPS) ? fps : maxFPS;
	}
}

void   Benchmark::setEnabled(bool on) {
	enabled = on;
}

bool   Benchmark::isEnabled() const {
	return enabled;
}

bool Benchmark::isRunning()   const {
	return (elapsedTime < duration);
}

void Benchmark::setDuration(double dur) {
	duration = clamp(dur, 1.0f, 3600.0f);
}

double  Benchmark::getTotalDuration() const {
	return elapsedTime;
}
double  Benchmark::getAverageFrames() const {
	return averageFPS;
}
double  Benchmark::getTotalFrames()   const {
	return framesCounter;
}
double  Benchmark::getDuration()      const {
	return duration;
}
double  Benchmark::getMinFPS()        const {
	return minFPS;
}
double  Benchmark::getMaxFPS()        const {
	return maxFPS;
}
