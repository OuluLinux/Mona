#include "Mona.h"



// Receptor constructor.
Receptor::Receptor() {
	type   = RECEPTOR;
	motive = 0.0;
}

void Receptor::Init(Vector<SENSOR>& centroid, SENSOR_MODE sensor_mode, Mona* mona) {
	ASSERT((int)centroid.GetCount() == mona->sensor_count);
	this->centroid.Clear();

	for (int i = 0; i < centroid.GetCount(); i++)
		this->centroid.Add(centroid[i]);

	this->sensor_mode = sensor_mode;
	Neuron::Init(mona);
	type   = RECEPTOR;
	motive = 0.0;
}


void Receptor::Connect(Mona* mona) {
	ASSERT(centroid.GetCount() == mona->sensor_count);
	Neuron::Connect(mona);
	type   = RECEPTOR;
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
	return (GetSensorDistance(centroid, sensors));
}


// Get distance between sensor vectors.
// Distance metric is Euclidean distance squared.
SENSOR Receptor::GetSensorDistance(const Vector<SENSOR>& sensorsA, const Vector<SENSOR>& sensorsB) {
	SENSOR d;
	SENSOR dist = 0.0;

	for (int i = 0; i < sensorsA.GetCount(); i++) {
		d     = sensorsA[i] - sensorsB[i];
		dist += (d * d);
	}

	return dist;
}


// RDTree sensor vector search.
SENSOR Receptor::PatternDistance(const Vector<SENSOR>& sensorsA, const Vector<SENSOR>& sensorsB) {
	return GetSensorDistance(sensorsA, sensorsB);
}


void Receptor::LoadPattern(Mona& mona, Vector<SENSOR>& sensors, Stream& fp) {
	ASSERT(fp.IsLoading());
	SENSOR s;
	
	sensors.SetCount(mona.sensor_count);
	for (int i = 0; i < mona.sensor_count; i++) {
		fp % s;
		sensors[i] = s;
	}
}


void Receptor::StorePattern(const Vector<SENSOR>& sensors_in, Stream& fp) {
	ASSERT(fp.IsStoring());
	for (int i = 0; i < sensors_in.GetCount(); i++) {
		SENSOR s = sensors_in[i];
		fp % s;
	}
}


void* Receptor::LoadClient(Mona& mona, Stream& fp) {
	ASSERT(fp.IsLoading());
	ID id;
	fp % id;
	return mona.FindByID(id);
}

void Receptor::StoreClient(void* receptor, Stream& fp) {
	ASSERT(fp.IsStoring());
	fp % ((Receptor*)receptor)->id;
}



void Receptor::DeletePattern(Vector<SENSOR>& pattern) {
	pattern.Clear();
}


// Update goal value.
void Receptor::UpdateGoalValue() {
	VALUE_SET needs, need_deltas;

	if (!mona->LEARN_RECEPTOR_GOAL_VALUE)
		return;

	needs.Reserve(mona->need_count);
	need_deltas.Reserve(mona->need_count);

	for (int i = 0; i < mona->need_count; i++) {
		needs.Set(i, mona->homeostats[i].GetNeed());
		need_deltas.Set(i, mona->homeostats[i].GetAndClearNeedDelta());
	}

	goals.Update(needs, need_deltas);
}


