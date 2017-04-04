#include "Mona.h"

double RandomInterval(double min, double max) {
	return Randomf() * (max - min) + min;
}

bool RandomChance(double p) {
	if (p <= 0.0)
		return false;

	if (Randomf() <= p)
		return true;

	return false;
}

bool RandomBoolean() {
	return ((Random(65536) % 2) == 0 ? false : true);
}

