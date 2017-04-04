/*
    Homeostat.
    Regulates:
    1. A need value based on environmental and timing conditions.
    2. Goal values that change the need value.
*/

#ifndef _Mona_Homeostat_h_
#define _Mona_Homeostat_h_

class Mona;
class Receptor;
class Motor;
class Neuron;


struct IdFinder {
	
	virtual Neuron* FindByID(int id) = 0;
};

template <class T>
struct ClassID : Moveable<ClassID<T> > {
	T* r;
	int64 id;
	
	
	typedef ClassID<T> ClassT;
	
	ClassID() : id(-1), r(NULL) {}
	ClassID(T* r) : r(r), id(-1) {}
	
	void Serialize(Stream& s) {
		s % id;
		if (s.IsLoading()) {
			r = NULL;
		}
	}
	
	bool operator != (T* cmp) const {return cmp != r;}
	bool operator == (T* cmp) const {return cmp == r;}
	
	bool operator != (const ClassT& cmp) const {return cmp.r != r;}
	bool operator == (const ClassT& cmp) const {return cmp.r == r;}
	
	bool IsNull() const {return r == NULL;}
	
	bool FindFrom(IdFinder* mona) {
		if (id == -1) {r = NULL; return false;}
		Neuron* n = mona->FindByID(id);
		if (!n) return false;
		r = dynamic_cast<T*>(n);
		return r != NULL;
	}
	
	T* operator->() {return r;}
	const T* operator->() const {return r;}
	
	operator T&() {return *r;}
	operator const T&() const {return *r;}
	
	operator T*() {return r;}
	
	template <class A>
	A& As() {return dynamic_cast<A&>(*r);}
	
};

typedef ClassID<Receptor> ReceptorID;

// Homeostat.
class Homeostat : Moveable<Homeostat> {
public:

	// Data types.
	typedef unsigned long long   ID;
	typedef double               SENSOR;
	typedef int                  SENSOR_MODE;
	typedef int                  RESPONSE;
	typedef double               NEED;
	static const ID NULL_ID;
	enum { NULL_RESPONSE = 0x7fffffff };

	// Sensory-response goal.
	// Environmental stimuli and response that produce need changes.
	// Used to update receptor neuron goal values.
	class Goal : Moveable<Goal> {
	public:
		Vector<SENSOR> sensors;
		SENSOR_MODE    sensor_mode;
		RESPONSE       response;
		Motor*         motor;
		NEED           goal_value;
		bool           enabled;
		ReceptorID     receptor;
		
		Goal() {}
		Goal(const Goal& g) {*this = g;}
		void operator=(const Goal& g) {
			Panic("TODO");
		}
		
		void Serialize(Stream& s) {
			s % sensors % sensor_mode % receptor % response % goal_value % enabled;
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
	//Homeostat(int need_index, Mona* mona);
	void Init(int need_index, Mona* mona);

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
	void RemoveNeuron(Neuron* neuron);

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
