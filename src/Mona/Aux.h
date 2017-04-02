// For conditions of distribution and use, see copyright notice in mona.h

// Mona auxiliary classes.

#ifndef _Mona_Aux_h_
#define _Mona_Aux_h_

// Forward declarations.
class Neuron;
class EnablingSet;
class Mediator;
/*  class Receptor;
    class Motor;*/

// Sensor mode.
class SensorMode {
public:
	SENSOR_MODE         mode;
	Vector<bool>        mask;
	SENSOR              resolution;
	Vector<SENSOR_MODE> subsets;
	Vector<SENSOR_MODE> supersets;

	// Constructor.
	SensorMode() {
		mode       = 0;
		resolution = 0.0;
	}


	// Initialize.
	// Return mode.
	int Init(Vector<bool>& mask, SENSOR resolution,
			 Vector<SensorMode*>* sensor_modes) {
		int        i, j;
		bool       sub, super;
		SensorMode* s;
		// Initialize.
		this->mask.Clear();

		for (i = 0; i < (int)mask.GetCount(); i++)
			this->mask.Add(mask[i]);

		mode             = (int)sensor_modes->GetCount();
		this->resolution = resolution;

		// Determine subset/supersets.
		for (i = 0, j = (int)sensor_modes->GetCount(); i < j; i++) {
			s = (*sensor_modes)[i];
			subSuper(s, sub, super);

			if (sub && super)
				continue;

			if (sub) {
				if (!subTransitive(i, sensor_modes)) {
					subsets.Add(i);
					s->supersets.Add(j);
				}
			}
			else if (super) {
				if (!superTransitive(i, sensor_modes)) {
					supersets.Add(i);
					s->subsets.Add(j);
				}
			}
		}

		sensor_modes->Add(this);
		return (mode);
	}


	// Given mode is subset/superset?
	void subSuper(SensorMode* sensor_mode, bool& sub, bool& super) {
		sub = super = true;

		for (int i = 0; i < (int)mask.GetCount() && (sub || super); i++) {
			if (mask[i]) {
				if (!sensor_mode->mask[i])
					sub = false;
			}
			else {
				if (sensor_mode->mask[i])
					super = false;
			}
		}
	}


	// Given mode is a transitive subset?
	// if A->B and B->C, then C is transitive.
	bool subTransitive(int idx, Vector<SensorMode*>* sensor_modes) {
		bool       sub, super;
		SensorMode* s;

		for (int i = 0; i < (int)subsets.GetCount(); i++) {
			s = (*sensor_modes)[i];
			s->subSuper((*sensor_modes)[idx], sub, super);

			if (sub || s->subTransitive(idx, sensor_modes))
				return true;
		}

		return false;
	}


	// Given mode is a transitive superset?
	// if A->B and B->C, then C is transitive.
	bool superTransitive(int idx, Vector<SensorMode*>* sensor_modes) {
		bool       sub, super;
		SensorMode* s;

		for (int i = 0; i < (int)supersets.GetCount(); i++) {
			s = (*sensor_modes)[i];
			s->subSuper((*sensor_modes)[idx], sub, super);

			if (super || s->superTransitive(idx, sensor_modes))
				return true;
		}

		return false;
	}


	void Serialize(Stream& fp) {
		fp % mode % mask % resolution % subsets % supersets;
	}



	// Print.
	/*  void Print(FILE *out = stdout)
	    {
	    int i, j, size;

	    fprintf(out, "<mode>%d</mode>", mode);
	    fprintf(out, "<mask>");
	    size = (int)mask.GetCount();
	    for (i = 0; i < size; i++)
	    {
	      j = (int)mask[i];
	      fprintf(out, "%d", j);
	    }
	    fprintf(out, "</mask>");
	    fprintf(out, "<resolution>%f</resolution>", resolution);
	    fprintf(out, "<subsets>");
	    size = (int)subsets.GetCount();
	    for (i = 0; i < size; i++)
	    {
	      j = subsets[i];
	      fprintf(out, "%d", j);
	    }
	    fprintf(out, "</subsets>");
	    fprintf(out, "<supersets>");
	    size = (int)supersets.GetCount();
	    for (i = 0; i < size; i++)
	    {
	      j = supersets[i];
	      fprintf(out, "%d", j);
	    }
	    fprintf(out, "</supersets>");
	    }*/
};

// Goal value.
class GoalValue {
public:
	VALUE_SET values;
	COUNTER   update_count;
	Mona*      mona;

	// Constructors.
	GoalValue(int goal_count, Mona* mona) {
		values.Reserve(goal_count);
		update_count = 0;
		this->mona  = mona;
	}


	GoalValue() {
		update_count = 0;
		mona        = NULL;
	}


	// Initialize.
	void Init(int goal_count, Mona* mona) {
		values.Reserve(goal_count);
		this->mona = mona;
	}


	// Destructor.
	~GoalValue() {
		Clear();
	}


	// Get total goal value.
	inline NEED GetValue() {
		return (values.GetSum());
	}


	// Get specific goal value.
	inline NEED GetValue(int index) {
		return (values.Get(index));
	}


	// Set specific goal value.
	inline void SetValue(int index, NEED value) {
		values.Set(index, value);
	}


	// Set initial goals.
	inline void setGoals(VALUE_SET& goals) {
		values.Load(goals);
	}


	// Get number of goals.
	inline int GetGoalCount() {
		return (values.GetCount());
	}


	// Update.
	void Update(VALUE_SET& needs, VALUE_SET& need_deltas);


	// Clear.
	inline void Clear() {
		values.Clear();
		update_count = 0;
	}

	void Serialize(Stream& fp) {
		values.Serialize(fp);
		fp % update_count;
	}



	// Print.
	/*  void Print(FILE *out = stdout)
	    {
	    for (int i = 0; i < values.GetCount(); i++)
	    {
	      fprintf(out, "<value>%f</value>", values.Get(i));
	    }
	    fprintf(out, "<update_count>%llu</update_count>", update_count);
	    }*/
};

// Motive accumulator.
// Accumulates need change caused by goals.
class MotiveAccum {
public:
	VALUE_SET        base;
	VALUE_SET        delta;
	WEIGHT           weight;
	Vector<Neuron*> path;
	#ifdef MONA_TRACKING
	struct DriveElement {
		Neuron* neuron;
		MOTIVE motive;
	};
	Vector<struct DriveElement> drivers;
	#endif

	MotiveAccum(const MotiveAccum& src) {
		Panic("TODO");
	}

	// Constructor.
	MotiveAccum() {
		Clear();
	}


	// Destructor.
	~MotiveAccum() {
		Clear();
	}


	// Get value of accumulator.
	// This is the sum of the delta values
	// bounded by the base need values.
	inline MOTIVE GetValue() {
		double b, d, v;
		v = 0.0;

		for (int i = 0; i < delta.GetCount(); i++) {
			b = base.Get(i);
			d = delta.Get(i);

			if (d < 0.0) {
				if ((b + d) < 0.0)
					v += b;
				else
					v -= d;
			}
			else {
				if ((b + d) > 1.0)
					v -= (1.0 - b);
				else
					v -= d;
			}
		}

		return (v);
	}


	// Initialize accumulator.
	inline void Init(VALUE_SET& needs) {
		base.Load(needs);
		delta.Reserve(base.GetCount());
		delta.Zero();
		weight = 1.0;
		path.Clear();
	}


	// Configure accumulator.
	inline void Configure(MotiveAccum& accum, WEIGHT weight) {
		Init(accum.base);
		LoadNeeds(accum);
		Scale(weight);
		this->weight = accum.weight * weight;

		for (int i = 0; i < (int)accum.path.GetCount(); i++)
			path.Add(accum.path[i]);
	}


	// Load need delta values.
	void LoadNeeds(MotiveAccum& motive_accum) {
		delta.Load(motive_accum.delta);
	}


	// Accumulate need delta values.
	void AccumulateNeeds(MotiveAccum& motive_accum) {
		delta.Add(motive_accum.delta);
	}


	// Accumulate goal values.
	inline void AccumulateGoals(GoalValue& goals) {
		VALUE_SET v;
		v.Load(goals.values);
		v.Multiply(weight);
		delta.Subtract(v);
	}


	// Add a neuron to the path.
	// Return true if no loop detected.
	inline bool AddPath(Neuron* neuron) {
		for (int i = 0; i < (int)path.GetCount(); i++) {
			if (path[i] == neuron)
				return false;
		}

		path.Add(neuron);
		return true;
	}


	// Scale accumulator.
	inline void Scale(WEIGHT weight) {
		delta.Multiply(weight);
	}


	// Reset accumulator.
	inline void reset() {
		delta.Zero();
		weight = 1.0;
	}


	// Clear accumulator.
	inline void Clear() {
		base.Clear();
		delta.Clear();
		weight = 1.0;
		path.Clear();
	}

	void Serialize(Stream& fp) {
		base.Serialize(fp);
		delta.Serialize(fp);
		fp % weight;
	}



	// Print.
	/*  void Print(FILE *out = stdout)
	    {
	    int i;

	    fprintf(out, "<base>");
	    for (i = 0; i < base.GetCount(); i++)
	    {
	      fprintf(out, "<value>%f</value>", base.Get(i));
	    }
	    fprintf(out, "</base><delta>");
	    for (i = 0; i < delta.GetCount(); i++)
	    {
	      fprintf(out, "<value>%f</value>", delta.Get(i));
	    }
	    fprintf(out, "</delta><weight>%f</weight>", weight);
	    fprintf(out, "<value>%f</value>", GetValue());
	    }*/
};


// Event enabling.
class Enabling {
public:
	ENABLEMENT  value;
	MOTIVE      motive;
	int64       age;
	int         timer_index;
	EnablingSet* set;
	bool        new_in_set;
	Time        cause_begin;
	VALUE_SET   needs;

	// Constructor.
	Enabling(ENABLEMENT value, MOTIVE motive, int64 age,
			 int timer_index, Time cause_begin) {
		Clear();
		this->value      = value;
		this->motive     = motive;
		this->age        = age;
		this->timer_index = timer_index;
		this->cause_begin = cause_begin;
	}


	Enabling() {
		Clear();
	}


	// Destructor.
	~Enabling() {
		Clear();
	}


	// Set needs.
	void SetNeeds(Vector<Homeostat*>& homeostats) {
		int n = (int)homeostats.GetCount();
		needs.Reserve(n);

		for (int i = 0; i < n; i++)
			needs.Set(i, homeostats[i]->GetNeed());
	}


	// Clone.
	inline Enabling* Clone() {
		Enabling* enabling;
		enabling = new Enabling(value, motive, age,
								timer_index, cause_begin);
		ASSERT(enabling != NULL);
		enabling->new_in_set = new_in_set;
		enabling->needs.Load(needs);
		return (enabling);
	}


	// Clear enabling.
	inline void Clear() {
		value      = 0.0;
		motive     = 0.0;
		age        = 0;
		timer_index = -1;
		set        = NULL;
		new_in_set   = false;
		cause_begin.Set(0);
		needs.Clear();
	}

	void Serialize(Stream& fp) {
		fp % value % motive % age % timer_index % new_in_set % cause_begin;
		needs.Serialize(fp);

		if (fp.IsLoading()) set = NULL;
	}


	// Print.
	/*  void Print(FILE *out = stdout)
	    {
	    fprintf(out, "<value>%f</value>", value);
	    fprintf(out, "<motive>%f</motive>", value);
	    fprintf(out, "<age>%llu</age>", age);
	    fprintf(out, "<timer_index>%d</timer_index>", timer_index);
	    fprintf(out, "<new_in_set>");
	    if (new_in_set) { fprintf(out, "true"); } else{ fprintf(out, "false"); }
	    fprintf(out, "</new_in_set><cause_begin>%llu</cause_begin>", cause_begin);
	    fprintf(out, "<needs>");
	    int n = needs.GetCount();
	    for (int i = 0; i < n; i++)
	    {
	      fprintf(out, "<need>%f</need>", needs.Get(i));
	    }
	    fprintf(out, "/<needs>");
	    }*/
};

// Set of event enablings.
class EnablingSet {
public:
	Array<Enabling> enablings;

	// Constructor.
	EnablingSet() {
		Clear();
	}


	// Destructor.
	~EnablingSet() {
		Clear();
	}


	// Get size of set.
	inline int GetCount() {
		return ((int)enablings.GetCount());
	}


	// Insert an enabling.
	inline Enabling& Insert() {
		Panic("rename to Add");
		Enabling& e = enablings.Add();
		e.set      = this;
		e.new_in_set = true;
		return e;
	}


	// Remove an enabling.
	/*  inline void Remove(Enabling *enabling)
	    {
	    enablings.Remove(enabling);
	    }*/


	// Get enabling value.
	inline ENABLEMENT GetValue() {
		ENABLEMENT e;
		Array<Enabling>::Iterator list_iter;
		e = 0.0;

		for (list_iter = enablings.Begin();
			 list_iter != enablings.End(); list_iter++)
			e       += list_iter->value;

		return (e);
	}


	// Get value of new enablings.
	inline ENABLEMENT getNewValue() {
		ENABLEMENT e;
		Array<Enabling>::Iterator list_iter;
		e = 0.0;

		for (list_iter = enablings.Begin();
			 list_iter != enablings.End(); list_iter++) {
			if (list_iter->new_in_set)
				e += list_iter->value;
		}

		return (e);
	}


	// Get value of old enablings.
	inline ENABLEMENT getOldValue() {
		ENABLEMENT e;
		Array<Enabling>::Iterator list_iter;
		e = 0.0;

		for (list_iter = enablings.Begin();
			 list_iter != enablings.End(); list_iter++) {
			if (!list_iter->new_in_set)
				e += list_iter->value;
		}

		return (e);
	}


	// Clear new flags.
	inline void ClearNewInSet() {
		Array<Enabling>::Iterator list_iter;

		for (list_iter = enablings.Begin();
			 list_iter != enablings.End(); list_iter++)
			list_iter->new_in_set = false;
	}


	// Clear.
	inline void Clear() {
		enablings.Clear();
	}


	// Load.
	void Serialize(Stream& fp) {
		fp % enablings;

		if (fp.IsLoading()) {
			for (int i = 0; i < enablings.GetCount(); i++)
				enablings[i].set = this;
		}
	}


	// Print.
	/*  void Print(FILE *out = stdout)
	    {
	    Array<Enabling>::Iterator list_iter;

	    fprintf(out, "<enablingSet>");
	    for (list_iter = enablings.Begin();
	        list_iter != enablings.End(); list_iter++)
	    {
	      list_iter->Print(out);
	    }
	    fprintf(out, "</enablingSet>");
	    }*/
};

// Mediator event notifier.
struct Notify {
	Mediator*   mediator;
	EVENT_TYPE event_type;

	void Serialize(Stream& fp) {
	}
};

// Mediator firing notification.
struct FiringNotify {
	struct Notify* notify;
	WEIGHT        notify_strength;
	Time          cause_begin;
};

// Learning event.
class LearningEvent {
public:
	Neuron*      neuron;
	WEIGHT      firing_strength;
	Time        begin;
	Time        end;
	double probability;
	VALUE_SET   needs;

	LearningEvent(Neuron* neuron);
	LearningEvent();
	void Serialize(Stream& fp);

	// Print.
	/*  void Print(FILE *out = stdout)
	    {
	    if (neuron != NULL)
	    {
	      fprintf(out, "<neuron>\n");
	      switch (neuron->type)
	      {
	    #ifdef MONA_TRACKING
	      case RECEPTOR:
	         ((Receptor *)neuron)->Print((TRACKING_FLAGS)0, out);
	         fprintf(out, "\n");
	         break;

	      case MOTOR:
	         ((Motor *)neuron)->Print((TRACKING_FLAGS)0, out);
	         fprintf(false, false, out, "\n");
	         break;

	      case MEDIATOR:
	         ((Mediator *)neuron)->Print((TRACKING_FLAGS)0, out);
	         break;

	    #else
	      case RECEPTOR:
	         ((Receptor *)neuron)->Print(out);
	         fprintf(out, "\n");
	         break;

	      case MOTOR:
	         ((Motor *)neuron)->Print(out);
	         fprintf(out, "\n");
	         break;

	      case MEDIATOR:
	         ((Mediator *)neuron)->Print(out);
	         break;
	    #endif
	      }
	    }
	    else
	    {
	      fprintf(out, "<neuron>NULL");
	    }
	    fprintf(out, "</neuron>\n");
	    fprintf(out, "<firing_strength>%f</firing_strength>", firing_strength);
	    fprintf(out, "<begin>%llu</begin>", begin);
	    fprintf(out, "<end>%llu</end>", end);
	    fprintf(out, "<probability>%f</probability>", probability);
	    fprintf(out, "<needs>");
	    int n = needs.GetCount();
	    for (int i = 0; i < n; i++)
	    {
	      fprintf(out, "<need>%f</need>", needs.Get(i));
	    }
	    fprintf(out, "/<needs>");
	    }*/
};

// Generalization learning event.
class GeneralizationEvent {
public:
	Mediator*  mediator;
	ENABLEMENT enabling;
	Time       begin;
	Time       end;
	VALUE_SET  needs;

	GeneralizationEvent(Mediator* mediator, ENABLEMENT enabling);


	GeneralizationEvent();


	void Serialize(Stream& fp) {
		fp % enabling % begin % end;
		needs.Serialize(fp);

		if (fp.IsLoading()) mediator = NULL;
	}




	// Print.
	/*  void Print(FILE *out = stdout)
	    {
	    if (mediator != NULL)
	    {
	      fprintf(out, "<mediator>\n");
	    #ifdef MONA_TRACKING
	      mediator->Print((TRACKING_FLAGS)0, out);
	    #else
	      mediator->Print(out);
	    #endif
	    }
	    else
	    {
	      fprintf(out, "<mediator>NULL");
	    }
	    fprintf(out, "</mediator>\n");
	    fprintf(out, "<enabling>%f</enabling>", enabling);
	    fprintf(out, "<begin>%llu</begin>", begin);
	    fprintf(out, "<end>%llu</end>", end);
	    int n = needs.GetCount();
	    for (int i = 0; i < n; i++)
	    {
	      fprintf(out, "<need>%f</need>", needs.Get(i));
	    }
	    fprintf(out, "/<needs>");
	    }*/
};
#endif
