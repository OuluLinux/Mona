#ifndef _Mona_Neuron_h_
#define _Mona_Neuron_h_


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


#endif
