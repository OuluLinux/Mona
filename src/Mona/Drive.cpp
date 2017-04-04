#include "Mona.h"
#include <math.h>

// Drive need changes caused by goal values through network.
void
Mona::Drive() {
	MotiveAccum motive_accum;
	VALUE_SET needs;
	
	#ifdef MONA_TRACE
	if (trace_drive)
		printf("***Drive phase***\n");
	#endif
	
	ASSERT(need_count == homeostats.GetCount());
	
	// Copy needs from homeostats to work set.
	needs.Reserve(need_count);

	for (int i = 0; i < need_count; i++)
		needs.Set(i, homeostats[i].GetNeed());

	#ifdef MONA_TRACE
	if (trace_drive) {
		printf("Needs: ");
		needs.Print();
	}
	#endif

	// Initialize drive.
	for (int i = 0; i < receptors.GetCount(); i++) {
		Neuron& neuron = receptors[i];
		neuron.InitDrive(needs);
	}

	for (int i = 0; i < motors.GetCount(); i++) {
		Neuron& neuron = motors[i];
		neuron.InitDrive(needs);
	}

	for (int i = 0; i < mediators.GetCount(); i++) {
		Neuron& neuron = mediators[i];
		neuron.InitDrive(needs);
	}

	// Drive.
	for (int i = 0; i < receptors.GetCount(); i++) {
		Receptor& receptor = receptors[i];

		if (receptor.goals.GetValue() != 0.0) {
			motive_accum.Init(needs);
			motive_accum.AccumulateGoals(receptor.goals);

			if (motive_accum.GetValue() != 0.0) {
				motive_accum.Init(needs);
				ClearMotiveWork();
				
				#ifdef MONA_TRACKING
				motive_accum.drivers.Clear();
				#endif
				
				receptor.Drive(motive_accum);
				SetMotives();
			}
		}
	}

	for (int i = 0; i < motors.GetCount(); i++) {
		Motor& motor = motors[i];

		if (motor.goals.GetValue() != 0.0) {
			motive_accum.Init(needs);
			motive_accum.AccumulateGoals(motor.goals);

			if (motive_accum.GetValue() != 0.0) {
				motive_accum.Init(needs);
				motive_accum.Init(needs);
				ClearMotiveWork();
				
				#ifdef MONA_TRACKING
				motive_accum.drivers.Clear();
				#endif
				
				motor.Drive(motive_accum);
				SetMotives();
			}
		}
	}

	for (int i = 0; i < mediators.GetCount(); i++) {
		Mediator& mediator = mediators[i];
		
		NEED goal_value = mediator.goals.GetValue();

		if (goal_value != 0.0) {
			if (!mediator.IsGoalValueSubsumed()) {
				motive_accum.Init(needs);
				motive_accum.AccumulateGoals(mediator.goals);

				if (motive_accum.GetValue() != 0.0) {
					motive_accum.Init(needs);
					ClearMotiveWork();
					
					#ifdef MONA_TRACKING
					motive_accum.drivers.Clear();
					#endif
					
					mediator.Drive(motive_accum);
					SetMotives();
				}
			}
		}
	}

	needs.Clear();
	
	// Finalize motives.
	FinalizeMotives();
}

// Clear motive working accumulators.
void
Mona::ClearMotiveWork() {
	for (int i = 0; i < receptors.GetCount(); i++) {
		Neuron& neuron = receptors[i];
		neuron.ClearMotiveWork();
	}

	for (int i = 0; i < motors.GetCount(); i++) {
		Neuron& neuron = motors[i];
		neuron.ClearMotiveWork();
	}

	for (int i = 0; i < mediators.GetCount(); i++) {
		Neuron& neuron = mediators[i];
		neuron.ClearMotiveWork();
	}
}


// Set motives.
void
Mona::SetMotives() {
	for (int i = 0; i < receptors.GetCount(); i++) {
		Neuron& neuron = receptors[i];
		neuron.SetMotive();
	}

	for (int i = 0; i < motors.GetCount(); i++) {
		Neuron& neuron = motors[i];
		neuron.SetMotive();
	}

	for (int i = 0; i < mediators.GetCount(); i++) {
		Neuron& neuron = mediators[i];
		neuron.SetMotive();
	}
}




// Finalize motives.
void
Mona::FinalizeMotives() {
	for (int i = 0; i < receptors.GetCount(); i++) {
		Neuron& neuron = receptors[i];
		neuron.FinalizeMotive();
	}

	for (int i = 0; i < motors.GetCount(); i++) {
		Neuron& neuron = motors[i];
		neuron.FinalizeMotive();
	}

	for (int i = 0; i < mediators.GetCount(); i++) {
		Neuron& neuron = mediators[i];
		neuron.FinalizeMotive();
	}
}

