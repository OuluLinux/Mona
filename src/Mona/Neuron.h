#ifndef _Mona_Neuron_h_
#define _Mona_Neuron_h_


class Neuron {
public:
	
	Neuron();
	virtual ~Neuron() {}
	
	static const int NULL_ID;

	// Initialize/clear.
	void Init(Mona* mona);
	void Connect(Mona* mona);
	void Clear();

	// Identifier.
	ID id;
	int mem_id;

	// Neuron type.
	int type;

	// Creation time.
	int64 creation_time;

	// Firing strength.
	ENABLEMENT firing_strength;

	// Goal value.
	GoalValue goals;

	// Motive.
	MOTIVE motive;
	bool   motive_valid;
	void   Drive(MotiveAccum&); // also copy in original
	void InitDrive(VALUE_SET& needs);
	void ClearMotiveWork();
	void SetMotive();
	void FinalizeMotive();

	MotiveAccum           motive_work;
	bool                  motive_work_valid;
	VectorMap<int, double> drive_weights;

	// Instinct?
	bool instinct;

	// Parent is instinct?
	bool HasInnerInstinct();

	// Event notification.
	Vector<Notify> notify_list;

	// Neural network.
	Mona* mona;

	void Serialize(Stream& fp);

	#ifdef MONA_TRACKING
	// Track neuron activity.
	class Activation {
	public:
		bool   fire;
		bool   enable;
		bool   drive;
		MOTIVE motive;
		struct DrivePath {
			Vector<struct MotiveAccum::DriveElement> drivers;
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

typedef ClassID<Neuron> NeuronID;




// Learning event.
class LearningEvent : Moveable<LearningEvent> {
public:
	NeuronID    neuron;
	WEIGHT      firing_strength;
	int64       begin;
	int64       end;
	double probability;
	VALUE_SET   needs;

	//LearningEvent(Neuron* neuron);
	LearningEvent();
	virtual ~LearningEvent() {}
	
	void Serialize(Stream& fp);
	
	void SetNeuron(Neuron& neuron);
	
	template <class T>
	T* GetNeuron() const {return dynamic_cast<T*>(neuron.r);}
	
	
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

#endif
