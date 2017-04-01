#include "Mona.h"

// Sense environment.
void
Mona::Sense() {
	int         i, j;
	SENSOR_MODE sensor_mode;
	Vector<SENSOR>     sensors_work;
	Receptor*           receptor;
	SENSOR             distance;
	bool               add_receptor;
	Vector<Receptor*> oldReceptorSet, newReceptorSet;
	#ifdef MONA_TRACE

	if (trace_sense) {
		printf("***Sense phase***\n");
		printf("Sensors: ");

		for (i = 0; i < (int)sensors.GetCount(); i++)
			printf("%f ", sensors[i]);

		printf("\n");
	}

	#endif

	// Update need based on sensors.
	for (i = 0; i < need_count; i++)
		homeostats[i]->SensorsUpdate();

	// Clear receptor firings.
	for (i = 0; i < (int)receptors.GetCount(); i++) {
		receptor = receptors[i];
		receptor->firing_strength = 0.0;
	}

	// Add base sensor mode?
	if (sensor_modes.GetCount() == 0) {
		Vector<bool> mask;

		for (i = 0; i < sensor_count; i++)
			mask.Add(true);

		AddSensorMode(mask);
	}

	// Fire receptors matching sensor modes.
	for (sensor_mode = 0; sensor_mode < (int)sensor_modes.GetCount(); sensor_mode++) {
		// Apply sensor mode to sensors.
		ApplySensorMode(sensors, sensors_work, sensor_mode);
		// Get firing receptor based on sensor vector and mode.
		receptor = GetCentroidReceptor(sensors_work, sensor_mode, distance);
		// Create a receptor that matches sensor vector?
		add_receptor = false;

		if ((receptor == NULL) || ((receptor != NULL) &&
								   (distance > sensor_modes[sensor_mode]->resolution) &&
								   (distance > NEARLY_ZERO))) {
			receptor    = NewReceptor(sensors_work, sensor_mode);
			add_receptor = true;
		}

		// Incorporate into sensor mode sets.
		if (add_receptor) {
			for (i = 0; i < (int)oldReceptorSet.GetCount(); i++) {
				for (j = 0; j < (int)sensor_modes[sensor_mode]->subsets.GetCount(); j++) {
					if (oldReceptorSet[i]->sensor_mode == sensor_modes[sensor_mode]->subsets[j]) {
						receptor->sub_sensor_modes.Add(oldReceptorSet[i]);
						oldReceptorSet[i]->super_sensor_modes.Add(receptor);
						break;
					}
				}

				if (j < (int)sensor_modes[sensor_mode]->subsets.GetCount())
					continue;

				for (j = 0; j < (int)sensor_modes[sensor_mode]->supersets.GetCount(); j++) {
					if (oldReceptorSet[i]->sensor_mode == sensor_modes[sensor_mode]->supersets[j]) {
						receptor->super_sensor_modes.Add(oldReceptorSet[i]);
						oldReceptorSet[i]->sub_sensor_modes.Add(receptor);
						break;
					}
				}
			}
		}

		for (i = 0; i < (int)newReceptorSet.GetCount(); i++) {
			for (j = 0; j < (int)sensor_modes[sensor_mode]->subsets.GetCount(); j++) {
				if (newReceptorSet[i]->sensor_mode == sensor_modes[sensor_mode]->subsets[j]) {
					receptor->sub_sensor_modes.Add(newReceptorSet[i]);
					newReceptorSet[i]->super_sensor_modes.Add(receptor);
					break;
				}
			}

			if (j < (int)sensor_modes[sensor_mode]->subsets.GetCount())
				continue;

			for (j = 0; j < (int)sensor_modes[sensor_mode]->supersets.GetCount(); j++) {
				if (newReceptorSet[i]->sensor_mode == sensor_modes[sensor_mode]->supersets[j]) {
					receptor->super_sensor_modes.Add(newReceptorSet[i]);
					newReceptorSet[i]->sub_sensor_modes.Add(receptor);
					break;
				}
			}
		}

		if (add_receptor)
			newReceptorSet.Add(receptor);
		else
			oldReceptorSet.Add(receptor);

		// Fire receptor.
		receptor->firing_strength = 1.0;
		// Update receptor goal value.
		receptor->UpdateGoalValue();
		#ifdef MONA_TRACE

		if (trace_sense) {
			printf("Receptor firing: centroid=[ ");

			for (i = 0; i < sensor_count; i++)
				printf("%f ", receptor->centroid[i]);

			printf("], sensor_mode=%d\n", receptor->sensor_mode);
		}

		#endif
		#ifdef MONA_TRACKING
		receptor->tracker.fire = true;
		#endif
	}
}


// Apply sensor mode to sensors.
void Mona::ApplySensorMode(Vector<SENSOR>& sensorsIn,
						   Vector<SENSOR>& sensorsOut, SENSOR_MODE sensor_mode) {
	int i;

	// Defaulting to base sensor mode?
	if (sensor_modes.GetCount() == 0) {
		ASSERT(sensor_mode == 0);
		Vector<bool> mask;

		for (i = 0; i < sensor_count; i++)
			mask.Add(true);

		AddSensorMode(mask);
	}

	sensorsOut.Clear();

	for (i = 0; i < (int)sensorsIn.GetCount(); i++) {
		if (sensor_modes[sensor_mode]->mask[i])
			sensorsOut.Add(sensorsIn[i]);
		else
			sensorsOut.Add(0.0f);
	}
}


void Mona::ApplySensorMode(Vector<SENSOR>& sensors, SENSOR_MODE sensor_mode) {
	Vector<SENSOR> sensors_work;
	ApplySensorMode(sensors, sensors_work, sensor_mode);

	for (int i = 0; i < (int)sensors.GetCount(); i++)
		sensors[i] = sensors_work[i];
}


// Find the receptor containing the centroid closest to
// the sensor vector for the current sensor mode.
// Also return the centroid-vector distance.
Receptor*
Mona::GetCentroidReceptor(Vector<SENSOR>& sensors,
						  SENSOR_MODE sensor_mode, SENSOR& distance) {
	RDTree::RDSearch* result = sensor_centroids[sensor_mode]->Search((void*)&sensors, 1);

	if (result != NULL) {
		distance = result->distance;
		Receptor* receptor = (Receptor*)(result->node->client);
		delete result;
		return (receptor);
	}
	else {
		distance = 0.0;
		return (NULL);
	}
}


// Get distance from centroid to given sensor vector.
SENSOR Receptor::GetCentroidDistance(Vector<SENSOR>& sensors) {
	return (GetSensorDistance(&centroid, &sensors));
}


// Get distance between sensor vectors.
// Distance metric is Euclidean distance squared.
SENSOR Receptor::GetSensorDistance(Vector<SENSOR>* sensorsA,
								   Vector<SENSOR>* sensorsB) {
	SENSOR d;
	SENSOR dist = 0.0;

	for (int i = 0; i < (int)sensorsA->GetCount(); i++) {
		d     = (*sensorsA)[i] - (*sensorsB)[i];
		dist += (d * d);
	}

	return (dist);
}


// RDTree sensor vector search.
SENSOR Receptor::PatternDistance(void* sensorsA, void* sensorsB) {
	return (GetSensorDistance((Vector<SENSOR>*)sensorsA, (Vector<SENSOR>*)sensorsB));
}


void* Receptor::LoadPattern(void* mona, Stream& fp) {
	SENSOR s;
	Vector<SENSOR>* sensors = new Vector<SENSOR>();
	ASSERT(sensors != NULL);

	for (int i = 0; i < ((Mona*)mona)->sensor_count; i++) {
		FREAD_FLOAT(&s, fp);
		sensors->Add(s);
	}

	return ((void*)sensors);
}


void Receptor::StorePattern(void* sensorsIn, Stream& fp) {
	SENSOR s;
	Vector<SENSOR>* sensors = (Vector<SENSOR>*)sensorsIn;

	for (int i = 0; i < (int)sensors->GetCount(); i++) {
		s = (*sensors)[i];
		FWRITE_FLOAT(&s, fp);
	}
}


void* Receptor::LoadClient(void* mona, Stream& fp) {
	ID id;
	FREAD_LONG_LONG(&id, fp);
	return ((void*)((Mona*)mona)->FindByID(id));
}


void Receptor::StoreClient(void* receptor, Stream& fp) {
	FWRITE_LONG_LONG(&((Receptor*)receptor)->id, fp);
}


void Receptor::DeletePattern(void* pattern) {
	delete (Vector<SENSOR>*)pattern;
}


// Set sensor resolution.
bool Mona::SetSensorResolution(SENSOR sensorResolution) {
	if ((int)receptors.GetCount() == 0) {
		SENSOR_RESOLUTION = sensorResolution;
		return true;
	}
	else
		return false;
}


// Add sensor mode.
int Mona::AddSensorMode(Vector<bool>& sensorMask, SENSOR sensorResolution) {
	// Must add modes before cycling.
	if ((int)receptors.GetCount() > 0)
		return (-1);

	// Duplicate?
	for (int i = 0; i < (int)sensor_modes.GetCount(); i++) {
		bool duplicate = true;

		for (int j = 0; j < (int)sensorMask.GetCount(); j++) {
			if (sensor_modes[i]->mask[j] != sensorMask[j]) {
				duplicate = false;
				break;
			}
		}

		if (duplicate) {
			if (sensor_modes[i]->resolution == sensorResolution)
				return (sensor_modes[i]->mode);
		}
	}

	// Create sensor mode.
	SensorMode* s = new SensorMode();
	ASSERT(s != NULL);
	s->Init(sensorMask, sensorResolution, &sensor_modes);
	// Create associated centroid search tree.
	RDTree* t = new RDTree(Receptor::PatternDistance,
						   Receptor::DeletePattern);
	ASSERT(t != NULL);
	sensor_centroids.Add(t);
	return (s->mode);
}


int Mona::AddSensorMode(Vector<bool>& sensorMask) {
	return (AddSensorMode(sensorMask, SENSOR_RESOLUTION));
}


// Update goal value.
void Receptor::UpdateGoalValue() {
	VALUE_SET needs, need_deltas;

	if (!mona->LEARN_RECEPTOR_GOAL_VALUE)
		return;

	needs.Reserve(mona->need_count);
	need_deltas.Reserve(mona->need_count);

	for (int i = 0; i < mona->need_count; i++) {
		needs.Set(i, mona->homeostats[i]->GetNeed());
		need_deltas.Set(i, mona->homeostats[i]->GetAndClearNeedDelta());
	}

	goals.Update(needs, need_deltas);
}
