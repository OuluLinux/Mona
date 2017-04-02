#include "Mona.h"





// Initialize neuron.
void
Neuron::Init(Mona* mona) {
	Clear();
	this->mona = mona;
	goals.Init(mona->need_count, mona);
}


// Clear neuron.
void
Neuron::Clear() {
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

	for (int i = 0; i < (int)notify_list.GetCount(); i++)
		delete notify_list[i];

	notify_list.Clear();
	#ifdef MONA_TRACKING
	tracker.fire   = false;
	tracker.enable = false;
	tracker.drive  = false;
	#endif
}


// Does neuron have an instinct parent?
bool
Neuron::HasInnerInstinct() {
	Mediator* mediator;

	for (int i = 0; i < (int)notify_list.GetCount(); i++) {
		mediator = notify_list[i]->mediator;

		if (mediator->instinct ||
			mediator->HasInnerInstinct())
			return true;
	}

	return false;
}


// Load neuron.
void Neuron::Serialize(Stream& fp) {
	fp % id % type % creation_time % firing_strength;
	goals.Serialize(fp);
	fp % motive % instinct % notify_list;
	int           i, j;
	struct Notify* notify;
	Clear();
	FREAD_LONG_LONG(&id, fp);
	FREAD_INT(&i, fp);
	type = (NEURON_TYPE)i;
	FREAD_LONG_LONG(&creation_time, fp);
	FREAD_DOUBLE(&firing_strength, fp);
	goals.Load(fp);
	FREAD_DOUBLE(&motive, fp);
	FREAD_BOOL(&instinct, fp);
	notify_list.Clear();
	FREAD_INT(&i, fp);
	notify_list.SetCount(i);

	for (i = 0; i < (int)notify_list.GetCount(); i++) {
		notify = new struct Notify;
		ASSERT(notify != NULL);
		notify->mediator = (Mediator*)new ID;
		ASSERT(notify->mediator != NULL);
		FREAD_LONG_LONG((ID*)notify->mediator, fp);
		FREAD_INT(&j, fp);
		notify->event_type = (EVENT_TYPE)j;
		notify_list[i]     = notify;
	}
}


// Save neuron.
// When changing format increment FORMAT in mona.h
void Neuron::Store(Stream& fp) {
	int           i, j;
	struct Notify* notify;
	FWRITE_LONG_LONG(&id, fp);
	i = (int)type;
	FWRITE_INT(&i, fp);
	FWRITE_LONG_LONG(&creation_time, fp);
	FWRITE_DOUBLE(&firing_strength, fp);
	goals.Store(fp);
	FWRITE_DOUBLE(&motive, fp);
	FWRITE_BOOL(&instinct, fp);
	i = (int)notify_list.GetCount();
	FWRITE_INT(&i, fp);

	for (i = 0; i < (int)notify_list.GetCount(); i++) {
		notify = notify_list[i];
		FWRITE_LONG_LONG(&notify->mediator->id, fp);
		j = (int)notify->event_type;
		FWRITE_INT(&j, fp);
	}
}




// Initialize neuron drive.
void
Neuron::InitDrive(VALUE_SET& needs) {
	int           i;
	ENABLEMENT    up, down, e, ce, re, ee;
	Mediator*      mediator;
	struct Notify* notify;
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
	switch (type) {
	case RECEPTOR:
		up = 1.0;
		break;

	case MOTOR:
		return;

	case MEDIATOR:
		mediator = (Mediator*)this;
		down     = mediator->effective_enablement;
		up       = 1.0 - down;
		e        = mediator->GetEnablement();
		ce       = re = ee = 0.0;
		// Distribute the down amount among components.
		re = (mediator->response_enablings.GetValue() / e) * down;

		if (mediator->response != NULL)
			drive_weights[mediator->response] = re;

		ee = mediator->effect_enablings.GetValue();
		ee = drive_weights[mediator->effect] = (ee / e) * down;
		ce = down - (re + ee);
		drive_weights[mediator->cause] = ce;
		break;
	}

	// Distribute the up amount among parents.
	for (i = 0; i < (int)notify_list.GetCount(); i++) {
		notify   = notify_list[i];
		mediator = notify->mediator;

		if (notify->event_type == EFFECT_EVENT) {
			drive_weights[mediator] = up *
									  (1.0 - mediator->effective_enabling_weight);
		}
		else
			drive_weights[mediator] = 0.0;
	}
}


// Clear motive working accumulators.
void
Neuron::ClearMotiveWork() {
	motive_work.reset();
	motive_work_valid = false;
	#ifdef MONA_TRACKING
	tracker.motive_work_paths.Clear();
	#endif
}





// Set neuron motive.
void
Neuron::SetMotive() {
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
void
Neuron::AccumMotiveTracking() {
	int i, j, k;

	if (type != MOTOR)
		return;

	for (i = 0; i < (int)tracker.motive_work_paths.GetCount(); i++) {
		if (tracker.motive_paths.GetCount() < Mona::MAX_DRIVER_TRACKS)
			tracker.motive_paths.Add(tracker.motive_work_paths[i]);
		else {
			for (j = 0, k = -1; j < (int)tracker.motive_work_paths.GetCount(); j++) {
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
void
Neuron::FinalizeMotive() {
	motive = motive / mona->max_motive;

	if (motive > 1.0)
		motive = 1.0;

	if (motive < -1.0)
		motive = -1.0;
}


// Neuron drive.
void
Neuron::Drive(MotiveAccum motive_accum) {
	int         i;
	Mediator*    mediator;
	Receptor*    receptor;
	MOTIVE      m;
	WEIGHT      w;
	MotiveAccum accum_work;

	// Prevent looping.
	if (!motive_accum.AddPath(this))
		return;

	// Accumulate need change due to goal value.
	if ((type != MEDIATOR) || !((Mediator*)this)->IsGoalValueSubsumed())
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

	switch (type) {
	// Distribute motive to component events.
	case MEDIATOR:
		mediator = (Mediator*)this;

		// Drive motive to cause event.
		if ((w = mediator->drive_weights[mediator->cause]) > NEARLY_ZERO) {
			accum_work.Configure(motive_accum, w * (1.0 - mona->DRIVE_ATTENUATION));
			mediator->cause->Drive(accum_work);
		}

		// Drive motive to response event.
		if (mediator->response != NULL) {
			if ((w = mediator->drive_weights[mediator->response]) > NEARLY_ZERO) {
				accum_work.Configure(motive_accum, w * (1.0 - mona->DRIVE_ATTENUATION));
				mediator->response->Drive(accum_work);
			}
		}

		// Drive motive to effect event.
		if ((w = mediator->drive_weights[mediator->effect]) > NEARLY_ZERO) {
			accum_work.Configure(motive_accum, w * (1.0 - mona->DRIVE_ATTENUATION));
			mediator->effect->Drive(accum_work);
		}

		break;

	// Receptor drives motive to subset receptors.
	case RECEPTOR:
		receptor = (Receptor*)this;

		for (i = 0; i < (int)receptor->sub_sensor_modes.GetCount(); i++) {
			accum_work.Configure(motive_accum, 1.0);
			receptor->sub_sensor_modes[i]->Drive(accum_work);
		}

		break;

	// Drive terminates on motor neurons.
	case MOTOR:
		return;
	}

	// Drive motive to parent mediators.
	for (i = 0; i < (int)notify_list.GetCount(); i++) {
		mediator = notify_list[i]->mediator;

		if ((w = drive_weights[mediator]) > NEARLY_ZERO) {
			w *= (1.0 - mona->DRIVE_ATTENUATION);
			accum_work.Configure(motive_accum, w);
			mediator->DriveCause(accum_work);
		}
	}
}




#ifdef MONA_TRACKING
// Track motive.
// Returns true if tracking continues.
bool
Neuron::TrackMotive(MotiveAccum& in, MotiveAccum& out) {
	int i, j;
	struct MotiveAccum::DriveElement e;
	struct Activation::DrivePath  d;
	VALUE_SET needs;

	for (i = 0; i < (int)in.drivers.GetCount(); i++) {
		if (in.drivers[i].neuron == this)
			return false;

		out.drivers.Add(in.drivers[i]);
		d.drivers.Add(in.drivers[i]);
	}

	needs.Reserve(mona->need_count);

	for (i = 0; i < mona->need_count; i++)
		needs.Set(i, mona->homeostats[i]->GetNeed());

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
		for (i = 0, j = -1; i < (int)tracker.motive_work_paths.GetCount(); i++) {
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

