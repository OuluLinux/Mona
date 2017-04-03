#include "Mona.h"

// Sense environment.
void
Mona::Sense() {
	SENSOR_MODE sensor_mode;
	Vector<SENSOR>     sensors_work;
	SENSOR             distance;
	bool               add_receptor;
	Vector<Receptor*> old_receptor_set, new_receptor_set;
	
	#ifdef MONA_TRACE

	if (trace_sense) {
		printf("***Sense phase***\n");
		printf("Sensors: ");

		for (i = 0; i < sensors.GetCount(); i++)
			printf("%f ", sensors[i]);

		printf("\n");
	}

	#endif

	// Update need based on sensors.
	for (int i = 0; i < need_count; i++)
		homeostats[i]->SensorsUpdate();

	// Clear receptor firings.
	for (int i = 0; i < receptors.GetCount(); i++) {
		Receptor& receptor = receptors[i];
		receptor.firing_strength = 0.0;
	}

	// Add base sensor mode?
	if (sensor_modes.GetCount() == 0) {
		Vector<bool> mask;
		mask.SetCount(sensor_count, true);
		AddSensorMode(mask);
	}

	// Fire receptors matching sensor modes.
	for (int sm = 0; sm < sensor_modes.GetCount(); sm++) {
		SensorMode& sensor_mode = *sensor_modes[sm];
		
		// Apply sensor mode to sensors.
		ApplySensorMode(sensors, sensors_work, sm);
		
		// Get firing receptor based on sensor vector and mode.
		Receptor* found_receptor = FindCentroidReceptor(sensors_work, sm, distance);
		
		// Create a receptor that matches sensor vector?
		add_receptor = false;

		if ((found_receptor == NULL) || ((found_receptor != NULL) &&
								   (distance > sensor_mode.resolution) &&
								   (distance > NEARLY_ZERO))) {
			found_receptor = NewReceptor(sensors_work, sm);
			add_receptor = true;
		}
		
		Receptor& receptor = *found_receptor;

		// Incorporate into sensor mode sets.
		if (add_receptor) {
			for (int i = 0; i < old_receptor_set.GetCount(); i++) {
				
				Receptor& old = *old_receptor_set[i];
				
				bool old_sensormode_is_subset = false;
				
				for (int j = 0; j < sensor_mode.subsets.GetCount(); j++) {
					
					if (old.sensor_mode == sensor_mode.subsets[j]) {
						
						receptor.sub_sensor_modes.Add(&old);
						old.super_sensor_modes.Add(&receptor);
						old_sensormode_is_subset = true;
						break;
					}
				}

				if (old_sensormode_is_subset)
					continue;

				for (int j = 0; j < sensor_mode.supersets.GetCount(); j++) {
					
					if (old.sensor_mode == sensor_mode.supersets[j]) {
						receptor.super_sensor_modes.Add(&old);
						old.sub_sensor_modes.Add(&receptor);
						break;
					}
				}
			}
		}

		for (int i = 0; i < new_receptor_set.GetCount(); i++) {
			
			Receptor& new_ = *new_receptor_set[i];
			
			bool new_sensormode_is_subset = false;
			
			for (int j = 0; j < sensor_mode.subsets.GetCount(); j++) {
				
				if (new_.sensor_mode == sensor_mode.subsets[j]) {
					
					receptor.sub_sensor_modes.Add(&new_);
					new_.super_sensor_modes.Add(&receptor);
					new_sensormode_is_subset = true;
					break;
				}
			}
			
			if (new_sensormode_is_subset)
				continue;

			for (int j = 0; j < sensor_mode.supersets.GetCount(); j++) {
				
				if (new_.sensor_mode == sensor_mode.supersets[j]) {
					receptor.super_sensor_modes.Add(&new_);
					new_.sub_sensor_modes.Add(&receptor);
					break;
				}
			}
		}

		if (add_receptor)
			new_receptor_set.Add(&receptor);
		else
			old_receptor_set.Add(&receptor);

		// Fire receptor.
		receptor.firing_strength = 1.0;
		// Update receptor goal value.
		receptor.UpdateGoalValue();
		#ifdef MONA_TRACE

		if (trace_sense) {
			printf("Receptor firing: centroid=[ ");

			for (i = 0; i < sensor_count; i++)
				printf("%f ", receptor.centroid[i]);

			printf("], sensor_mode=%d\n", receptor.sensor_mode);
		}

		#endif
		#ifdef MONA_TRACKING
		receptor.tracker.fire = true;
		#endif
	}
}


// Apply sensor mode to sensors.
void Mona::ApplySensorMode(Vector<SENSOR>& sensors_in, Vector<SENSOR>& sensors_out, SENSOR_MODE sensor_mode) {
	
	// Defaulting to base sensor mode?
	if (sensor_modes.GetCount() == 0) {
		ASSERT(sensor_mode == 0);
		Vector<bool> mask;
		mask.SetCount(sensor_count, true);
		AddSensorMode(mask);
	}

	sensors_out.Clear();

	for (int i = 0; i < sensors_in.GetCount(); i++) {
		if (sensor_modes[sensor_mode]->mask[i])
			sensors_out.Add(sensors_in[i]);
		else
			sensors_out.Add(0.0);
	}
}


void Mona::ApplySensorMode(Vector<SENSOR>& sensors, SENSOR_MODE sensor_mode) {
	Vector<SENSOR> sensors_work;
	ApplySensorMode(sensors, sensors_work, sensor_mode);

	for (int i = 0; i < sensors.GetCount(); i++)
		sensors[i] = sensors_work[i];
}


// Find the receptor containing the centroid closest to
// the sensor vector for the current sensor mode.
// Also return the centroid-vector distance.
Receptor* Mona::FindCentroidReceptor(Vector<SENSOR>& sensors,
						  SENSOR_MODE sensor_mode, SENSOR& distance) {
	RDTree::RDSearch* result = sensor_centroids[sensor_mode]->Search((void*)&sensors, 1);

	if (result != NULL) {
		distance = result->distance;
		Receptor* receptor = result->node->client;
		delete result;
		return (receptor);
	}
	else {
		distance = 0.0;
		return NULL;
	}
}



// Set sensor resolution.
bool Mona::SetSensorResolution(SENSOR sensor_resolution) {
	if (receptors.GetCount() == 0) {
		SENSOR_RESOLUTION = sensor_resolution;
		return true;
	}
	else
		return false;
}


// Add sensor mode.
int Mona::AddSensorMode(Vector<bool>& sensor_mask, SENSOR sensor_resolution) {
	
	// Must add modes before cycling.
	if (receptors.GetCount() > 0)
		return -1;

	// Duplicate?
	for (int i = 0; i < sensor_modes.GetCount(); i++) {
		bool duplicate = true;

		for (int j = 0; j < sensor_mask.GetCount(); j++) {
			if (sensor_modes[i]->mask[j] != sensor_mask[j]) {
				duplicate = false;
				break;
			}
		}

		if (duplicate) {
			if (sensor_modes[i]->resolution == sensor_resolution)
				return (sensor_modes[i]->mode);
		}
	}

	// Create sensor mode.
	SensorMode* s = new SensorMode();
	ASSERT(s != NULL);
	s->Init(sensor_mask, sensor_resolution, sensor_modes);
	
	// Create associated centroid search tree.
	RDTree* t = new RDTree(Receptor::PatternDistance,
						   Receptor::DeletePattern);
	ASSERT(t != NULL);
	sensor_centroids.Add(t);
	return (s->mode);
}


int Mona::AddSensorMode(Vector<bool>& sensor_mask) {
	return (AddSensorMode(sensor_mask, SENSOR_RESOLUTION));
}

