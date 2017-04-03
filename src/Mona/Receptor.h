#ifndef _Mona_Receptor_h_
#define _Mona_Receptor_h_



// Receptor neuron.
// Contains a centroid for a cluster of vectors
// in sensor values space.
// Fires when centroid is closest to sensor vector.
class Receptor : public Neuron {
public:

	// Construct/destruct.
	Receptor(Vector<SENSOR>& centroid, SENSOR_MODE sensor_mode, Mona* mona);
	~Receptor();

	// Centroid sensor vector.
	Vector<SENSOR> centroid;

	// Sensor mode and links to receptors
	// with subset and superset sensor modes.
	SENSOR_MODE        sensor_mode;
	Vector<ReceptorID> sub_sensor_modes;
	Vector<ReceptorID> super_sensor_modes;

	// Get distance from centroid to given sensor vector.
	SENSOR GetCentroidDistance(Vector<SENSOR>& sensors);

	// Get distance between sensor vectors.
	static SENSOR GetSensorDistance(Vector<SENSOR>* sensorsA, Vector<SENSOR>* sensorsB);

	// Is given receptor a duplicate of this?
	bool IsDuplicate(Receptor&);

	// Update goal value.
	void UpdateGoalValue();
	
	void Serialize(Stream& fp);
	
	// RDTree sensor vector search.
	static SENSOR PatternDistance(void* sensorsA, void* sensorsB);
	static void* LoadPattern(void* mona, Stream& fp);
	static void StorePattern(void* sensors, Stream& fp);
	static void* LoadClient(void* mona, Stream& fp);
	static void StoreClient(void* receptor, Stream& fp);
	static void DeletePattern(void* pattern);

	// Print receptor.
	/*      void Print(FILE *out = stdout);

	    #ifdef MONA_TRACKING
	    void Print(TRACKING_FLAGS tracking, FILE *out = stdout);
	    #endif*/
};


#endif
