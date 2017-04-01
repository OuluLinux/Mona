#ifndef _Mona_Mona_h_
#define _Mona_Mona_h_

//#include <Core/Core.h>
//using namespace Upp;

//namespace Mona {};



// Version (SCCS "what" format).
#define MONA_VERSION    "@(#)Mona version 5.2"
extern const char* MonaVersion;

// Tracing and event tracking compilation symbols:
// MONA_TRACE: use with run-time tracing flags below for execution flow trace.
// MONA_TRACKING: use with dump and print options for firing, enabling and drive
// event tracking.

#ifdef WIN32
	#define PUBLIC_API    __declspec(dllexport)
	#pragma warning( disable: 4251 )
#else
	#define PUBLIC_API
#endif

#include "Common.h"
#include "Homeostat.h"


typedef Homeostat::ID            ID;
typedef Homeostat::SENSOR        SENSOR;
typedef Homeostat::SENSOR_MODE   SENSOR_MODE;
typedef int                      RESPONSE;
typedef double                   RESPONSE_POTENTIAL;
typedef Homeostat::NEED          NEED;
typedef double                   MOTIVE;
typedef double                   ENABLEMENT;
typedef double                   UTILITY;
typedef double                   WEIGHT;
typedef unsigned long long       COUNTER;

enum NEURON_TYPE {
	RECEPTOR,
	MOTOR,
	MEDIATOR
};

enum EVENT_TYPE {
	CAUSE_EVENT,
	RESPONSE_EVENT,
	EFFECT_EVENT
};

enum EVENT_OUTCOME {
	EXPIRE,
	FIRE
};



enum { NULL_RESPONSE = Homeostat::NULL_RESPONSE };
enum { DEFAULT_RANDOM_SEED = 4517 };


#include "Aux.h"

// Neuron.
class Neuron {
public:

	static const int NULL_ID;

	// Initialize/clear.
	void Init(Mona* mona);
	void Clear();

	// Identifier.
	ID id;

	// Neuron type.
	NEURON_TYPE type;

	// Creation time.
	Time creation_time;

	// Firing strength.
	ENABLEMENT firing_strength;

	// Goal value.
	GoalValue goals;

	// Motive.
	MOTIVE motive;
	bool   motive_valid;
	void   Drive(MotiveAccum); // also copy in original
	void InitDrive(VALUE_SET& needs);
	void ClearMotiveWork();
	void SetMotive();
	void FinalizeMotive();

	MotiveAccum           motive_work;
	bool                  motive_work_valid;
	VectorMap<Neuron*, double> drive_weights;

	// Instinct?
	bool instinct;

	// Parent is instinct?
	bool HasInnerInstinct();

	// Event notification.
	Vector<struct Notify*> notify_list;

	// Neural network.
	Mona* mona;

	// Load.
	void Serialize(Stream& fp);

	// Save.
	void Store(Stream& fp);

	#ifdef MONA_TRACKING
	// Track neuron activity.
	class Activation {
	public:
		bool   fire;
		bool   enable;
		bool   drive;
		MOTIVE motive;
		struct DrivePath {
			Vector<struct MotiveAccum::DriveElem> drivers;
			MotiveAccum                           motive_work;
			MOTIVE                                motive;
		};
		Vector<struct DrivePath> motive_paths;
		Vector<struct DrivePath> motive_work_paths;
		void Clear() {
			fire   = enable = drive = false;
			motive = 0.0;
			motive_paths.Clear();
			motive_work_paths.Clear();
		}
	}
	tracker;

	// Track driven motive.
	bool TrackMotive(MotiveAccum& in, MotiveAccum& out);

	// Accumulate motive tracking.
	void AccumMotiveTracking();
	#endif
};

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
	Vector<Receptor*> sub_sensor_modes;
	Vector<Receptor*> super_sensor_modes;

	// Get distance from centroid to given sensor vector.
	SENSOR GetCentroidDistance(Vector<SENSOR>& sensors);

	// Get distance between sensor vectors.
	static SENSOR GetSensorDistance(Vector<SENSOR>* sensorsA, Vector<SENSOR>* sensorsB);

	// Is given receptor a duplicate of this?
	bool IsDuplicate(Receptor*);

	// Update goal value.
	void UpdateGoalValue();

	// Load receptor.
	void Serialize(Stream& fp);

	// Save receptor.
	void Store(Stream& fp);

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

// Motor neuron.
class Motor : public Neuron {
public:

	// Construct/destruct.
	Motor(RESPONSE response, Mona* mona);
	~Motor();

	// Response.
	RESPONSE response;

	// Is given motor a duplicate of this?
	bool IsDuplicate(Motor*);

	// Load motor.
	void Serialize(Stream& fp);

	// Save motor.
	void Store(Stream& fp);

	// Print motor.
	/*  void Print(FILE *out = stdout);

	    #ifdef MONA_TRACKING
	    void Print(TRACKING_FLAGS tracking, FILE *out = stdout);
	    #endif*/
};

// Mediator neuron.
// Mediates a set of neuron events.
class Mediator : public Neuron {
public:
	// Construct/destruct.
	Mediator(ENABLEMENT enablement, Mona* mona);
	~Mediator();

	// Level 0 mediator is composed of non-mediator neurons.
	// Level n mediator is composed of at most level n-1 mediators.
	int level;

	// Enablement.
	ENABLEMENT base_enablement;
	ENABLEMENT GetEnablement();
	void UpdateEnablement(EVENT_OUTCOME outcome, WEIGHT updateWeight);

	// Effective enablement.
	ENABLEMENT effective_enablement;
	WEIGHT     effective_enabling_weight;
	bool       effective_enablement_valid;
	void UpdateEffectiveEnablement();

	// Utility.
	UTILITY utility;
	WEIGHT  utility_weight;
	void UpdateUtility(WEIGHT updateWeight);
	UTILITY GetEffectiveUtility();

	// Update goal value.
	void UpdateGoalValue(VALUE_SET& needs);

	// Is goal value subsumed by component?
	bool IsGoalValueSubsumed();

	// Events.
	Neuron* cause;
	Neuron* response;
	Neuron* effect;
	void   AddEvent(EVENT_TYPE, Neuron*);

	// Enablings.
	EnablingSet response_enablings;
	EnablingSet effect_enablings;

	// Time of causation.
	Time cause_begin;

	// Event firing.
	void CauseFiring(WEIGHT notify_strength, Time cause_begin);
	void ResponseFiring(WEIGHT notify_strength);
	void EffectFiring(WEIGHT notify_strength);
	void RetireEnablings(bool force = false);

	// Drive.
	void DriveCause(MotiveAccum);

	// Is given mediator a duplicate of this?
	bool IsDuplicate(Mediator*);

	// Load mediator.
	void Serialize(Stream& fp);

	// Save mediator.
	void Store(Stream& fp);

	// Print mediator.
	/*  void Print(FILE *out = stdout);
	    void PrintBrief(FILE *out = stdout);

	    #ifdef MONA_TRACKING
	    void Print(TRACKING_FLAGS tracking, FILE *out = stdout);
	    void PrintBrief(TRACKING_FLAGS tracking, FILE *out = stdout);
	    void Print(TRACKING_FLAGS tracking, bool brief, int level, FILE *out);

	    #else
	    void Print(bool brief, int level, FILE *out);
	    #endif*/
};



// Mona: sensory/response, neural network, and needs.
class Mona {
public:

	// Content format.
	enum { FORMAT = 10 };

	// Data types.

	// Print version.
	static void printVersion(FILE* out = stdout);

	// Construct/initialize.
	Mona();
	Mona(int sensor_count, int response_count, int need_count, int random_seed = DEFAULT_RANDOM_SEED);
	void InitParms();
	void InitNet(int sensor_count, int response_count, int need_count, int random_seed = DEFAULT_RANDOM_SEED);
	bool SetSensorResolution(SENSOR sensorResolution);
	int AddSensorMode(Vector<bool>& sensorMask);
	int AddSensorMode(Vector<bool>& sensorMask, SENSOR sensorResolution);

	// Destructor.
	~Mona();

	// Include auxiliary classes.


	// Parameters.
	// Initialize, load, save, and print in mona.cpp and mona_jni.cpp
	ENABLEMENT MIN_ENABLEMENT;
	ENABLEMENT INITIAL_ENABLEMENT;
	WEIGHT     DRIVE_ATTENUATION;
	WEIGHT     FIRING_STRENGTH_LEARNING_DAMPER;
	WEIGHT     LEARNING_DECREASE_VELOCITY;
	WEIGHT     LEARNING_INCREASE_VELOCITY;
	WEIGHT     RESPONSE_RANDOMNESS;
	WEIGHT     UTILITY_ASYMPTOTE;
	int        DEFAULT_MAX_LEARNING_EFFECT_EVENT_INTERVAL;
	int        DEFAULT_NUM_EFFECT_EVENT_INTERVALS;
	int        MAX_MEDIATORS;
	int        MAX_MEDIATOR_LEVEL;
	int        MAX_RESPONSE_EQUIPPED_MEDIATOR_LEVEL;
	int        MIN_RESPONSE_UNEQUIPPED_MEDIATOR_LEVEL;
	SENSOR     SENSOR_RESOLUTION;
	int        LEARN_MEDIATOR_GOAL_VALUE_MIN_LEVEL;
	bool       LEARN_RECEPTOR_GOAL_VALUE;

	#ifdef MONA_TRACE
	// Tracing.
	bool trace_sense;
	bool trace_enable;
	bool trace_learn;
	bool trace_drive;
	bool trace_respond;
	#endif

	// Sensors.
	Vector<SENSOR> sensors;
	int            sensor_count;

	// Sensor modes.
	// A sensor mode is a set of sensors related
	// by "modality", analogous to sight, hearing,
	// touch. Sensing for a specific sensor mode
	// maps the next sensor vector to the closest
	// receptor in a set reserved for the mode. This
	// provides the network with selective attention
	// capabilities.
	Vector<SensorMode*> sensor_modes;
	void                 ApplySensorMode(Vector<SENSOR>& in, Vector<SENSOR>& out, SENSOR_MODE);
	void                 ApplySensorMode(Vector<SENSOR>& sensors, SENSOR_MODE);

	// Sensor centroid search spaces.
	// Each sensor mode defines a space.
	Vector<RDTree*> sensor_centroids;

	// Find the receptor having the centroid closest to
	// the sensor vector for the give sensor mode.
	Receptor* GetCentroidReceptor(Vector<SENSOR>& sensors,
								  SENSOR_MODE sensor_mode, SENSOR& distance);

	// Response.
	RESPONSE response;
	int      response_count;
	RESPONSE AddResponse();

	Vector<RESPONSE_POTENTIAL> response_potentials;
	RESPONSE_POTENTIAL         GetResponsePotential(RESPONSE);
	RESPONSE           response_override;
	RESPONSE_POTENTIAL response_override_potential;
	bool               OverrideResponse(RESPONSE);
	bool               OverrideResponseConditional(RESPONSE, RESPONSE_POTENTIAL);
	void ClearResponseOverride();
	Motor* FindMotorByResponse(RESPONSE response);

	// Needs.
	int                 need_count;
	Vector<Homeostat*> homeostats;

	// Need management.
	NEED GetNeed(int index);
	void SetNeed(int index, NEED value);
	void InflateNeed(int index);
	void SetPeriodicNeed(int need_index,
						 int frequency, NEED periodic_need);
	void ClearPeriodicNeed(int need_index);
	void PrintNeed(int index);

	// Goal management.
	int AddGoal(int need_index, Vector<SENSOR>& sensors,
				SENSOR_MODE sensor_mode, RESPONSE response, NEED goal_value);
	int AddGoal(int need_index, Vector<SENSOR>& sensors,
				SENSOR_MODE sensor_mode, NEED goal_value);
	int FindGoal(int need_index, Vector<SENSOR>& sensors,
				 SENSOR_MODE sensor_mode, RESPONSE response);
	int FindGoal(int need_index, Vector<SENSOR>& sensors,
				 SENSOR_MODE sensor_mode);
	int GetGoalCount(int need_index);
	bool GetGoalInfo(int need_index, int goal_index,
					 Vector<SENSOR>& sensors, SENSOR_MODE& sensor_mode,
					 RESPONSE& response, NEED& goal_value, bool& enabled);
	bool IsGoalEnabled(int need_index, int goal_index);
	bool EnableGoal(int need_index, int goal_index);
	bool DisableGoal(int need_index, int goal_index);
	bool RemoveGoal(int need_index, int goal_index);

	// Effect event intervals.
	// The maximum time for an effect event to fire after a cause fires.
	// 1. There must be at least one interval per mediator level.
	// 2. Response-equipped mediators are considered "immediate" mediators
	//    for which only the first interval is applicable with 100% weight.
	// 3. Customizable by application at initialization time. See initEffectIntervals.
	Vector<Vector<int> > effect_event_intervals;

	// Effect interval weights determine how enablement is distributed to enablings
	// timed by the effect event intervals. Causes that have more immediate effects
	// are plausibly more strongly related than those with more temporally distance
	// effects. This can be represented by weighting smaller intervals more heavily
	// than larger ones. effectIntervalWeights[n] must sum to 1.0
	// Customizable by application at initialization time. See initEffectIntervals.
	Vector<Vector<WEIGHT> > effect_event_interval_weights;

	// Maximum learning effect event intervals.
	// 1. These are the maximum firing intervals between events allowable
	//     for the learning of new mediators.
	// 2. These can be shorter than the intervals allowed for the events of
	//     existing mediators as a means of improving the quality and throttling
	//     the quantity of learned mediators.
	// 3. Customizable by application at initialization time. See initEffectIntervals.
	Vector<int> max_learning_effect_event_intervals;

	// Initialize effect event intervals and weights.
	void InitEffectEventIntervals();
	void InitEffectEventInterval(int level, int numIntervals);
	void InitEffectEventIntervalWeights();
	void InitEffectEventIntervalWeight(int level);
	void InitMaxLearningEffectEventIntervals();
	bool AuditDefaultEffectEventIntervals();

	// Behavior cycle.
	RESPONSE Cycle(Vector<SENSOR>& sensors);
	void Sense();
	void Enable();
	void Learn();
	void Drive();
	void Respond();

	// Cause event firing notifications.
	Vector<FiringNotify> cause_firings;

	// Motive.
	MOTIVE max_motive;
	void ClearMotiveWork();
	void SetMotives();
	void FinalizeMotives();

	// Unique identifier dispenser.
	COUNTER id_dispenser;

	// Event clock.
	Time event_clock;

	// Learning event lists.
	Vector<Vector<LearningEvent*> > learning_events;
	Vector<GeneralizationEvent*>  generalization_events;

	// Mediator generation.
	void CreateMediator(LearningEvent* event);
	void GeneralizeMediator(GeneralizationEvent* event);
	bool IsDuplicateMediator(Mediator*);

	// Random numbers.
	int random_seed;
	//Random random;

	#ifdef MONA_TRACKING
	// Tracking flags.
	typedef unsigned int   TRACKING_FLAGS;
	enum { TRACK_FIRE = 1, TRACK_ENABLE = 2, TRACK_DRIVE = 4 };
	#endif


	// Network.
	Vector<Receptor*> receptors;
	Vector<Motor*>    motors;
	Vector<Mediator*>   mediators;

	// Add/delete neurons to/from network.
	Receptor* NewReceptor(Vector<SENSOR>& centroid, SENSOR_MODE sensor_mode);
	Motor* NewMotor(RESPONSE response);
	Mediator* NewMediator(ENABLEMENT enablement);
	void DeleteNeuron(Neuron*);

	// Clear memory.
	void ExpireResponseEnablings(RESPONSE);
	void ExpireMediatorEnablings(Mediator*);
	void ClearWorkingMemory();
	void ClearLongTermMemory();

	// Get mediators with worst and best utilities.
	Mediator* GetWorstMediator(int min_level = 0);
	Mediator* GetBestMediator(int min_level = 0);

	// Load network.
	bool Load(String filename);
	bool Store(String filename);
	bool Serialize(Stream& fp);
	Neuron* FindByID(ID id);

	// Clear network.
	void Clear();

	// Print network.
	/*  bool Print(String filename);
	    void Print(FILE *out = stdout);
	    void PrintBrief(FILE *out = stdout);
	    void PrintParms(FILE *out = stdout);

	    #ifdef MONA_TRACKING
	    void Print(TRACKING_FLAGS tracking, FILE *out = stdout);
	    void PrintBrief(TRACKING_FLAGS tracking, FILE *out = stdout);
	    #endif

	    // Print helpers.
	    #ifdef MONA_TRACKING
	    void Print(TRACKING_FLAGS tracking, bool brief, FILE *out = stdout);

	    #else
	    void Print(bool brief, FILE *out = stdout);
	    #endif*/

	#ifdef MONA_TRACKING
	enum { MAX_DRIVER_TRACKS = 3 };

	// Clear tracking activity.
	void ClearTracking();
	#endif

private:
	// Clear variables.
	void ClearVars();
};

#endif
