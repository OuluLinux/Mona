#include "Mona.h"
#include <math.h>

// Drive need changes caused by goal values through network.
void
Mona::Drive() {
	int      i;
	Neuron*   neuron;
	Receptor* receptor;
	Motor*    motor;
	Mediator* mediator;
	Vector<Mediator*>::Iterator mediator_iter;
	MotiveAccum                motive_accum;
	VALUE_SET needs;
	#ifdef MONA_TRACE

	if (trace_drive)
		printf("***Drive phase***\n");

	#endif
	// Copy needs from homeostats to work set.
	needs.Reserve(need_count);

	for (i = 0; i < need_count; i++)
		needs.Set(i, homeostats[i]->GetNeed());

	#ifdef MONA_TRACE

	if (trace_drive) {
		printf("Needs: ");
		needs.Print();
	}

	#endif

	// Initialize drive.
	for (i = 0; i < (int)receptors.GetCount(); i++) {
		neuron = (Neuron*)receptors[i];
		neuron->InitDrive(needs);
	}

	for (i = 0; i < (int)motors.GetCount(); i++) {
		neuron = (Neuron*)motors[i];
		neuron->InitDrive(needs);
	}

	for (mediator_iter = mediators.Begin();
		 mediator_iter != mediators.End(); mediator_iter++) {
		neuron = (Neuron*)(*mediator_iter);
		neuron->InitDrive(needs);
	}

	// Drive.
	for (i = 0; i < (int)receptors.GetCount(); i++) {
		receptor = receptors[i];

		if (receptor->goals.GetValue() != 0.0) {
			motive_accum.Init(needs);
			motive_accum.AccumulateGoals(receptor->goals);

			if (motive_accum.GetValue() != 0.0) {
				motive_accum.Init(needs);
				ClearMotiveWork();
				#ifdef MONA_TRACKING
				motive_accum.drivers.Clear();
				#endif
				receptor->Drive(motive_accum);
				SetMotives();
			}
		}
	}

	for (i = 0; i < (int)motors.GetCount(); i++) {
		motor = motors[i];

		if (motor->goals.GetValue() != 0.0) {
			motive_accum.Init(needs);
			motive_accum.AccumulateGoals(motor->goals);

			if (motive_accum.GetValue() != 0.0) {
				motive_accum.Init(needs);
				motive_accum.Init(needs);
				ClearMotiveWork();
				#ifdef MONA_TRACKING
				motive_accum.drivers.Clear();
				#endif
				motor->Drive(motive_accum);
				SetMotives();
			}
		}
	}

	for (mediator_iter = mediators.Begin();
		 mediator_iter != mediators.End(); mediator_iter++) {
		mediator = *mediator_iter;
		NEED goal_value = mediator->goals.GetValue();

		if (goal_value != 0.0) {
			if (!mediator->IsGoalValueSubsumed()) {
				motive_accum.Init(needs);
				motive_accum.AccumulateGoals(mediator->goals);

				if (motive_accum.GetValue() != 0.0) {
					motive_accum.Init(needs);
					ClearMotiveWork();
					#ifdef MONA_TRACKING
					motive_accum.drivers.Clear();
					#endif
					mediator->Drive(motive_accum);
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
	int    i;
	Neuron* neuron;
	Vector<Mediator*>::Iterator mediator_iter;

	for (i = 0; i < (int)receptors.GetCount(); i++) {
		neuron = (Neuron*)receptors[i];
		neuron->ClearMotiveWork();
	}

	for (i = 0; i < (int)motors.GetCount(); i++) {
		neuron = (Neuron*)motors[i];
		neuron->ClearMotiveWork();
	}

	for (mediator_iter = mediators.Begin();
		 mediator_iter != mediators.End(); mediator_iter++) {
		neuron = (Neuron*)(*mediator_iter);
		neuron->ClearMotiveWork();
	}
}


// Set motives.
void
Mona::SetMotives() {
	int    i;
	Neuron* neuron;
	Vector<Mediator*>::Iterator mediator_iter;

	for (i = 0; i < (int)receptors.GetCount(); i++) {
		neuron = (Neuron*)receptors[i];
		neuron->SetMotive();
	}

	for (i = 0; i < (int)motors.GetCount(); i++) {
		neuron = (Neuron*)motors[i];
		neuron->SetMotive();
	}

	for (mediator_iter = mediators.Begin();
		 mediator_iter != mediators.End(); mediator_iter++) {
		neuron = (Neuron*)(*mediator_iter);
		neuron->SetMotive();
	}
}




// Finalize motives.
void
Mona::FinalizeMotives() {
	int    i;
	Neuron* neuron;
	Vector<Mediator*>::Iterator mediator_iter;

	for (i = 0; i < (int)receptors.GetCount(); i++) {
		neuron = (Neuron*)receptors[i];
		neuron->FinalizeMotive();
	}

	for (i = 0; i < (int)motors.GetCount(); i++) {
		neuron = (Neuron*)motors[i];
		neuron->FinalizeMotive();
	}

	for (mediator_iter = mediators.Begin();
		 mediator_iter != mediators.End(); mediator_iter++) {
		neuron = (Neuron*)(*mediator_iter);
		neuron->FinalizeMotive();
	}
}

