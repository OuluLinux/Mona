#ifndef _Mona_Mona_h_
#define _Mona_Mona_h_

//#include <Core/Core.h>
//using namespace Upp;

//namespace Mona {};



// Tracing and event tracking compilation symbols:
// MONA_TRACE: use with run-time tracing flags below for execution flow trace.
// MONA_TRACKING: use with dump and print options for firing, enabling and drive
// event tracking.



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

enum {
	RECEPTOR,
	MOTOR,
	MEDIATOR
};

enum {
	CAUSE_EVENT,
	RESPONSE_EVENT,
	EFFECT_EVENT,
	INVALID_EVENT=-1
};

enum {
	EXPIRE,
	FIRE
};



enum { NULL_RESPONSE = Homeostat::NULL_RESPONSE };
enum { DEFAULT_RANDOM_SEED = 4517 };


#include "Aux.h"
#include "Neuron.h"
#include "Receptor.h"
#include "Motor.h"
#include "Mediator.h"


// Mona: sensory/response, neural network, and needs.
class Mona : public IdFinder {
public:

	// Content format.
	enum { FORMAT = 10 };

	// Data types.

	// Print version.
	static void printVersion(FILE* out = stdout);
	
	static const ID NULL_ID;

	// Construct/initialize.
	Mona();
	Mona(int sensor_count, int response_count, int need_count, int random_seed = DEFAULT_RANDOM_SEED);
	void InitParms();
	void InitNet(int sensor_count, int response_count, int need_count, int random_seed = DEFAULT_RANDOM_SEED);
	bool SetSensorResolution(SENSOR sensor_resolution);
	int AddSensorMode(Vector<bool>& sensor_mask);
	int AddSensorMode(Vector<bool>& sensor_mask, SENSOR sensor_resolution);

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
	void ApplySensorMode(Vector<SENSOR>& in, Vector<SENSOR>& out, SENSOR_MODE);
	void ApplySensorMode(Vector<SENSOR>& sensors, SENSOR_MODE);

	// Sensor centroid search spaces.
	// Each sensor mode defines a space.
	Vector<RDTree*> sensor_centroids;

	// Find the receptor having the centroid closest to
	// the sensor vector for the give sensor mode.
	Receptor* FindCentroidReceptor(Vector<SENSOR>& sensors, SENSOR_MODE sensor_mode, SENSOR& distance);

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
	Motor& GetMotorByResponse(RESPONSE response);

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
	Vector<Vector<LearningEvent> > learning_events;
	Vector<GeneralizationEvent>  generalization_events;

	// Mediator generation.
	void CreateMediator(const LearningEvent& effect_event);
	void GeneralizeMediator(const GeneralizationEvent& event);
	bool IsDuplicateMediator(Mediator& mediator);

	// Random numbers.
	int random_seed;
	//Random random;

	#ifdef MONA_TRACKING
	// Tracking flags.
	typedef unsigned int   TRACKING_FLAGS;
	enum { TRACK_FIRE = 1, TRACK_ENABLE = 2, TRACK_DRIVE = 4 };
	#endif


	// Network.
	Array<Receptor> receptors;
	Array<Motor>    motors;
	Array<Mediator>   mediators;

	// Add/delete neurons to/from network.
	Receptor* NewReceptor(Vector<SENSOR>& centroid, SENSOR_MODE sensor_mode);
	Motor* NewMotor(RESPONSE response);
	Mediator& AddMediator(ENABLEMENT enablement);
	//void DeleteNeuron(Neuron& neuron);
	void DeleteNeuron(Mediator& neuron);
	void DeleteNeuron(Receptor& neuron);

	// Clear memory.
	void ExpireResponseEnablings(RESPONSE);
	void ExpireMediatorEnablings(Mediator&);
	void ClearWorkingMemory();
	void ClearLongTermMemory();

	// Get mediators with worst and best utilities.
	Mediator* GetWorstMediator(int min_level = 0);
	Mediator* GetBestMediator(int min_level = 0);

	// Load network.
	void Load(String filename);
	void Store(String filename);
	void Serialize(Stream& fp);
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
