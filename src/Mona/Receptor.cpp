#include "Mona.h"



// Receptor constructor.
Receptor::Receptor(Vector<SENSOR>& centroid,
				   SENSOR_MODE sensor_mode, Mona* mona) {
	ASSERT((int)centroid.GetCount() == mona->sensor_count);
	this->centroid.Clear();

	for (int i = 0; i < centroid.GetCount(); i++)
		this->centroid.Add(centroid[i]);

	this->sensor_mode = sensor_mode;
	Init(mona);
	type   = RECEPTOR;
	motive = 0.0;
}


// Receptor destructor.
Receptor::~Receptor() {
	sub_sensor_modes.Clear();
	super_sensor_modes.Clear();
	Clear();
}


// Is given receptor a duplicate of this?
bool Receptor::IsDuplicate(Receptor& receptor) {
	if (sensor_mode != receptor.sensor_mode)
		return false;

	if (GetCentroidDistance(receptor.centroid) == 0.0)
		return true;
	else
		return false;
}


// Load receptor.
void Receptor::Serialize(Stream& fp) {
	Neuron::Serialize(fp);
	fp % sensor_mode % centroid % sub_sensor_modes % super_sensor_modes;
}



/*
    // Print receptor.
    #ifdef MONA_TRACKING
    void Receptor::Print(FILE *out)
    {
    Print((TRACKING_FLAGS)0, out);
    }


    void Receptor::Print(TRACKING_FLAGS tracking, FILE *out)
    #else
    void Receptor::Print(FILE *out)
    #endif
    {
    fprintf(out, "<receptor><id>%llu</id>", id);
    fprintf(out, "<sensor_mode>%d</sensor_mode>", sensor_mode);
    fprintf(out, "<centroid>");
    for (int i = 0; i < mona->sensor_count; i++)
    {
      fprintf(out, "<value>%f</value>", centroid[i]);
    }
    fprintf(out, "</centroid>");
    fprintf(out, "<goals>");
    goals.Print(out);
    fprintf(out, "</goals>");
    if (instinct)
    {
      fprintf(out, "<instinct>true</instinct>");
    }
    else
    {
      fprintf(out, "<instinct>false</instinct>");
    }
    #ifdef MONA_TRACKING
    if ((tracking & TRACK_FIRE) && tracker.fire)
    {
      fprintf(out, "<fire>true</fire>");
    }
    if ((tracking & TRACK_ENABLE) && tracker.enable)
    {
      fprintf(out, "<enable>true</enable>");
    }
    if ((tracking & TRACK_DRIVE) && tracker.drive)
    {
      fprintf(out, "<motive>%f</motive>", tracker.motive);
    }
    #endif
    fprintf(out, "</receptor>");
    }
*/




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

	for (int i = 0; i < sensorsA->GetCount(); i++) {
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


void Receptor::StorePattern(void* sensors_in, Stream& fp) {
	SENSOR s;
	Vector<SENSOR>* sensors = (Vector<SENSOR>*)sensors_in;
#error check this
	for (int i = 0; i < sensors->GetCount(); i++) {
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


