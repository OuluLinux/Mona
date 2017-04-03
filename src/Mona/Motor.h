#ifndef _Mona_Motor_h_
#define _Mona_Motor_h_


// Motor neuron.
class Motor : public Neuron {
public:

	// Construct/destruct.
	Motor(RESPONSE response, Mona* mona);
	~Motor();

	// Response.
	RESPONSE response;

	// Is given motor a duplicate of this?
	bool IsDuplicate(Motor&);

	void Serialize(Stream& fp);

	// Print motor.
	/*  void Print(FILE *out = stdout);

	    #ifdef MONA_TRACKING
	    void Print(TRACKING_FLAGS tracking, FILE *out = stdout);
	    #endif*/
};


#endif
