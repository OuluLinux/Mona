#ifndef _Mona_Mediator_h_
#define _Mona_Mediator_h_





// Mediator neuron.
// Mediates a set of neuron events.
class Mediator : public Neuron {
public:
	// Construct/destruct.
	Mediator();
	~Mediator();
	
	void Init(ENABLEMENT enablement, Mona* mona);
	
	// Level 0 mediator is composed of non-mediator neurons.
	// Level n mediator is composed of at most level n-1 mediators.
	int level;

	// Enablement.
	ENABLEMENT base_enablement;
	ENABLEMENT GetEnablement();
	void UpdateEnablement(int outcome, WEIGHT update_weight);

	// Effective enablement.
	ENABLEMENT effective_enablement;
	WEIGHT     effective_enabling_weight;
	bool       effective_enablement_valid;
	void UpdateEffectiveEnablement();

	// Utility.
	UTILITY utility;
	WEIGHT  utility_weight;
	void UpdateUtility(WEIGHT update_weight);
	UTILITY GetEffectiveUtility();

	// Update goal value.
	void UpdateGoalValue(const VALUE_SET& needs);

	// Is goal value subsumed by component?
	bool IsGoalValueSubsumed();

	// Events.
	NeuronID cause;
	NeuronID response;
	NeuronID effect;
	void   AddEvent(int, Neuron&);

	// Enablings.
	EnablingSet response_enablings;
	EnablingSet effect_enablings;

	// Time of causation.
	int64 cause_begin;

	// Event firing.
	void CauseFiring(WEIGHT notify_strength, int64 cause_begin);
	void ResponseFiring(WEIGHT notify_strength);
	void EffectFiring(WEIGHT notify_strength);
	void RetireEnablings(bool force = false);

	// Drive.
	void DriveCause(MotiveAccum&);

	// Is given mediator a duplicate of this?
	bool IsDuplicate(Mediator&);

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

typedef ClassID<Mediator> MediatorID;







// Mediator event notifier.
struct Notify : Moveable<Notify> {
	//ID id;
	int event_type;
	MediatorID   mediator;
	
	Notify() : event_type(INVALID_EVENT) {}
	
	void Serialize(Stream& fp) {
		fp % mediator % event_type;
		Panic("TODO: FindFrom in this caller");
	}
};

#endif
