/*
    Homeostat.
    Regulates:
    1. A need value based on environmental and timing conditions.
    2. Goal values that change the need value.
*/

#ifndef __HOMEOSTAT__
#define __HOMEOSTAT__

#include "Mona.h"

class Mona;

// Homeostat.
class Homeostat {
public:

	// Data types.
	typedef unsigned long long   ID;
	typedef double                SENSOR;
	typedef int                  SENSOR_MODE;
	typedef int                  RESPONSE;
	typedef double               NEED;
	static const ID NULL_ID;
	enum { NULL_RESPONSE = 0x7fffffff };

	// Sensory-response goal.
	// Environmental stimuli and response that produce need changes.
	// Used to update receptor neuron goal values.
	class Goal {
	public:
		Vector<SENSOR> sensors;
		SENSOR_MODE    sensor_mode;
		void*           receptor;
		RESPONSE       response;
		void*           motor;
		NEED           goal_value;
		bool           enabled;
		int id;

		void Serialize(Stream& s) {
			s % sensors % sensor_mode % id % response % goal_value % enabled;

			if (s.IsLoading()) {
				motor = NULL;
				receptor = NULL;
			}
		}
	};

	NEED         need;
	int          need_index;
	NEED         need_delta;
	Mona*         mona;
	NEED         periodic_need;
	int          frequency;
	int          freq_timer;
	Vector<Goal> goals;

	// Constructors.
	Homeostat();
	Homeostat(int need_index, Mona* mona);

	// Destructor.
	~Homeostat();

	// Get current need.
	inline NEED GetNeed() {
		return (need);
	}
	inline NEED GetAndClearNeedDelta() {
		NEED d = need_delta;
		need_delta = 0.0;
		return d;
	}


	// Set current need.
	inline void SetNeed(NEED need) {
		need_delta  = need - this->need;
		this->need = need;
	}


	// Set periodic need.
	void SetPeriodicNeed(int frequency, NEED need);
	void ClearPeriodicNeed();

	// Add goal.
	// Replace response and goal value of duplicate.
	int AddGoal(Vector<SENSOR>& sensors, SENSOR_MODE sensor_mode,
				RESPONSE response, NEED goal_value);
	int AddGoal(Vector<SENSOR>& sensors, SENSOR_MODE sensor_mode,
				NEED goal_value);

	// Add goal value to receptor if needed.
	void AddGoal(void* receptor);

	// Find index of goal matching sensors, sensor mode
	// and response. Return -1 for no match.
	int FindGoal(Vector<SENSOR>& sensors, SENSOR_MODE sensor_mode,
				 RESPONSE response);

	// Find index of goal matching sensors and sensor mode.
	// Return -1 for no match.
	int FindGoal(Vector<SENSOR>& sensors, SENSOR_MODE sensor_mode);

	// Get goal information at index.
	bool GetGoalInfo(int goal_index, Vector<SENSOR>& sensors,
					 SENSOR_MODE& sensor_mode, RESPONSE& response,
					 NEED& goal_value, bool& enabled);

	// Is goal enabled?
	bool IsGoalEnabled(int goal_index);

	// Enable goal.
	bool EnableGoal(int goal_index);

	// Disable goal.
	bool DisableGoal(int goal_index);

	// Remove goal.
	bool RemoveGoal(int goal_index);

	// Activate/deactivate goal with periodic need.
	bool ActivateGoal(int goal_index, int frequency, NEED periodic_need);
	bool DeactivateGoal(int goal_index);

	// Remove neuron from goals.
	void RemoveNeuron(void* neuron);

	// Update homeostat based on sensors.
	void SensorsUpdate();

	// Update homeostat based on response.
	void ResponseUpdate();

	// Load homeostat.
	void Load(String filename);
	void Serialize(Stream& fp);
	void Store(String filename);

	// Print homeostat.
	//void Print(FILE *out = stdout);
};

#endif
