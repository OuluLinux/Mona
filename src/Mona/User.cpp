#include "Mona.h"

void Mona::SetEffectEventIntervals (int level, int intervals) {
	effect_event_intervals[level].SetCount(intervals);

	for (int i = 0; i < intervals; i++) {
		effect_event_intervals[level][i] = pow(2.0, level);
	}
	
	InitEffectEventIntervalWeights();
}

void Mona::SetEffectEventInterval(int level, int interval, int value, double weight) {
	effect_event_intervals[level][interval]       = value;
	effect_event_intervals[level][interval] = weight;
}
