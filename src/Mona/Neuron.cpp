#include "Mona.h"



Atomic neuron_counter;



Neuron::Neuron() {
	mem_id = AtomicInc(neuron_counter);
}

// Initialize neuron.
void Neuron::Init(Mona* mona) {
	Clear();
	this->mona = mona;
	goals.Init(mona->need_count, mona);
}


// Clear neuron.
void Neuron::Clear() {
	id             = NULL_ID;
	creation_time.Set(INVALID_TIME);
	firing_strength = 0.0;
	goals.Clear();
	motive      = 0.0;
	motive_valid = false;
	motive_work.Clear();
	motive_work_valid = false;
	drive_weights.Clear();
	instinct = false;

	notify_list.Clear();
	
	#ifdef MONA_TRACKING
	tracker.fire   = false;
	tracker.enable = false;
	tracker.drive  = false;
	#endif
}


// Does neuron have an instinct parent?
bool Neuron::HasInnerInstinct() {
	for (int i = 0; i < notify_list.GetCount(); i++) {
		Mediator& mediator = notify_list[i].mediator;

		if (mediator.instinct || mediator.HasInnerInstinct())
			return true;
	}
	return false;
}


// Load neuron.
void Neuron::Serialize(Stream& fp) {
	fp % id % type % creation_time % firing_strength;
	goals.Serialize(fp);
	fp % motive % instinct % notify_list;
}





// Initialize neuron drive.
void Neuron::InitDrive(VALUE_SET& needs) {
	ENABLEMENT    up, down, e, ce, re, ee;
	
	motive      = 0.0;
	motive_valid = false;
	motive_work.Init(needs);
	motive_work_valid = false;
	
	#ifdef MONA_TRACKING
	tracker.motive_paths.Clear();
	tracker.motive_work_paths.Clear();
	#endif
	
	/*
	    Assign drive weights to destinations:

	    The input motive is divided into the "down" that
	    is driven to mediator component events and the "up"
	    that is driven to parents. The down value is proportional
	    to the mediator effective enablement. The remaining
	    fraction goes up. For receptors, all motive is driven
	    to parents; for motors, none is.

	    For distributing down, each event is weighted according to
	    its share of enabling.

	    For distributing up, parents for which the component is
	    an effect are weighted. For each of these, its proportion
	    is how much enablement it has available to enable the outer.
	*/
	drive_weights.Clear();

	// Divide the down amount.
	if (type == RECEPTOR) {
		up = 1.0;
	}
	else if (type == MOTOR) {
		return;
	}
	else if (type == MEDIATOR) {
		Mediator& mediator = dynamic_cast<Mediator&>(*this);
		down     = mediator.effective_enablement;
		up       = 1.0 - down;
		e        = mediator.GetEnablement();
		ce       = re = ee = 0.0;
		// Distribute the down amount among components.
		re = (mediator.response_enablings.GetValue() / e) * down;

		if (mediator.response != NULL)
			drive_weights.GetAdd(mediator.response->mem_id) = re;

		ee = mediator.effect_enablings.GetValue();
		ee = (drive_weights.GetAdd(mediator.effect->mem_id) = (ee / e) * down);
		ce = down - (re + ee);
		drive_weights.GetAdd(mediator.cause->mem_id) = ce;
	}

	// Distribute the up amount among parents.
	for (int i = 0; i < notify_list.GetCount(); i++) {
		Notify& notify = notify_list[i];
		Mediator& mediator = notify.mediator;

		if (notify.event_type == EFFECT_EVENT) {
			drive_weights.GetAdd(mediator.mem_id) = up * (1.0 - mediator.effective_enabling_weight);
		}
		else
			drive_weights.GetAdd(mediator.mem_id) = 0.0;
	}
}


// Clear motive working accumulators.
void Neuron::ClearMotiveWork() {
	motive_work.reset();
	motive_work_valid = false;
	#ifdef MONA_TRACKING
	tracker.motive_work_paths.Clear();
	#endif
}





// Set neuron motive.
void Neuron::SetMotive() {
	MOTIVE m;
	m = motive_work.GetValue();

	if (!motive_valid || (motive < m)) {
		motive_valid = true;
		motive      = m;
	}

	#ifdef MONA_TRACKING
	// Accumulate motive tracking.
	AccumMotiveTracking();
	#endif
}


#ifdef MONA_TRACKING
// Accumulate motive tracking.
void Neuron::AccumMotiveTracking() {
	if (type != MOTOR)
		return;

	for (i = 0; i < tracker.motive_work_paths.GetCount(); i++) {
		if (tracker.motive_paths.GetCount() < Mona::MAX_DRIVER_TRACKS)
			tracker.motive_paths.Add(tracker.motive_work_paths[i]);
		else {
			for (j = 0, k = -1; j < tracker.motive_work_paths.GetCount(); j++) {
				if (tracker.motive_work_paths[i].motive > tracker.motive_paths[j].motive) {
					if ((k == -1) || (tracker.motive_paths[j].motive < tracker.motive_paths[k].motive))
						k = j;
				}
			}

			if (k != -1)
				tracker.motive_paths[k] = tracker.motive_work_paths[i];
		}
	}
}


#endif





// Finalize neuron motive.
void Neuron::FinalizeMotive() {
	motive = motive / mona->max_motive;

	if (motive > 1.0)
		motive = 1.0;

	if (motive < -1.0)
		motive = -1.0;
}


// Neuron drive.
void Neuron::Drive(MotiveAccum motive_accum) {
	MOTIVE       m;
	WEIGHT       w;
	MotiveAccum  accum_work;

	// Prevent looping.
	if (!motive_accum.AddPath(this))
		return;

	// Accumulate need change due to goal value.
	if ((type != MEDIATOR) || !dynamic_cast<Mediator*>(this)->IsGoalValueSubsumed())
		motive_accum.AccumulateGoals(goals);

	// Accumulate motive.
	// Store greater motive except for attenuated "pain".
	m = motive_accum.GetValue();

	if (!motive_work_valid ||
		((m >= NEARLY_ZERO) && ((m - motive_work.GetValue()) > NEARLY_ZERO))) {
		motive_work_valid = true;
		motive_work.LoadNeeds(motive_accum);
	}

	#ifndef MONA_TRACKING
	else
		return;

	#else

	// Track motive.
	if (!TrackMotive(motive_accum, accum_work))
		return;

	#endif
	#ifdef MONA_TRACE

	if (mona->trace_drive)
		printf("Drive %llu, motive=%f\n", id, m);

	#endif

	// Distribute motive to component events.
	if (type == MEDIATOR) {
		Mediator& mediator = dynamic_cast<Mediator&>(*this);

		// Drive motive to cause event.
		if ((w = mediator.drive_weights.Get(mediator.cause->mem_id)) > NEARLY_ZERO) {
			accum_work.Configure(motive_accum, w * (1.0 - mona->DRIVE_ATTENUATION));
			mediator.cause->Drive(accum_work);
		}

		// Drive motive to response event.
		if (mediator.response != NULL) {
			if ((w = mediator.drive_weights.Get(mediator.response->mem_id)) > NEARLY_ZERO) {
				accum_work.Configure(motive_accum, w * (1.0 - mona->DRIVE_ATTENUATION));
				mediator.response->Drive(accum_work);
			}
		}

		// Drive motive to effect event.
		if ((w = mediator.drive_weights.Get(mediator.effect->mem_id)) > NEARLY_ZERO) {
			accum_work.Configure(motive_accum, w * (1.0 - mona->DRIVE_ATTENUATION));
			mediator.effect->Drive(accum_work);
		}
	}
	// Receptor drives motive to subset receptors.
	else if (type == RECEPTOR) {
		Receptor& receptor = dynamic_cast<Receptor&>(*this);
		
		for (int i = 0; i < receptor.sub_sensor_modes.GetCount(); i++) {
			accum_work.Configure(motive_accum, 1.0);
			receptor.sub_sensor_modes[i]->Drive(accum_work);
		}
	}
	// Drive terminates on motor neurons.
	else if (type == MOTOR) {
		return;
	}

	// Drive motive to parent mediators.
	for (int i = 0; i < notify_list.GetCount(); i++) {
		Mediator& mediator = notify_list[i].mediator;

		if ((w = drive_weights.Get(mediator.mem_id)) > NEARLY_ZERO) {
			w *= (1.0 - mona->DRIVE_ATTENUATION);
			accum_work.Configure(motive_accum, w);
			mediator.DriveCause(accum_work);
		}
	}
}




#ifdef MONA_TRACKING
// Track motive.
// Returns true if tracking continues.
bool Neuron::TrackMotive(MotiveAccum& in, MotiveAccum& out) {
	struct MotiveAccum::DriveElement e;
	struct Activation::DrivePath  d;
	VALUE_SET needs;

	for (i = 0; i < in.drivers.GetCount(); i++) {
		if (in.drivers[i].neuron == this)
			return false;

		out.drivers.Add(in.drivers[i]);
		d.drivers.Add(in.drivers[i]);
	}

	needs.Reserve(mona->need_count);

	for (i = 0; i < mona->need_count; i++)
		needs.Set(i, mona->homeostats[i].GetNeed());

	d.motive_work.Init(needs);
	needs.Clear();
	d.motive_work.LoadNeeds(in);
	e.neuron  = this;
	e.motive  = d.motive_work.GetValue();
	e.motive /= mona->max_motive;

	if (e.motive > 1.0)
		e.motive = 1.0;

	if (e.motive < -1.0)
		e.motive = -1.0;

	d.motive = e.motive;
	out.drivers.Add(e);
	d.drivers.Add(e);

	if (tracker.motive_work_paths.GetCount() < Mona::MAX_DRIVER_TRACKS)
		tracker.motive_work_paths.Add(d);
	else {
		for (i = 0, j = -1; i < tracker.motive_work_paths.GetCount(); i++) {
			if (d.motive_work.GetValue() > tracker.motive_work_paths[i].motive_work.GetValue()) {
				if ((j == -1) ||
					(tracker.motive_work_paths[i].motive_work.GetValue() <
					 tracker.motive_work_paths[j].motive_work.GetValue()))
					j = i;
			}
		}

		if (j != -1)
			tracker.motive_work_paths[j] = d;
		else {
			d.motive_work.Clear();
			d.drivers.Clear();
			return false;
		}
	}

	return true;
}


#endif

