#include "Mona.h"
#include <math.h>

// Enablement processing.
void
Mona::Enable() {
	int      i, j, k;
	Receptor* receptor;
	Mediator* mediator;
	Vector<Mediator*>::Iterator          mediator_iter;
	struct Notify*                       notify;
	struct FiringNotify                 firing_notify;
	Vector<FiringNotify>::Iterator firing_notify_iter;
	#ifdef MONA_TRACE

	if (trace_enable)
		printf("***Enable phase***\n");

	#endif

	// Clear mediators.
	for (mediator_iter = mediators.Begin();
		 mediator_iter != mediators.End(); mediator_iter++) {
		mediator = *mediator_iter;
		mediator->firing_strength = 0.0;
		mediator->response_enablings.ClearNewInSet();
		mediator->effect_enablings.ClearNewInSet();
	}

	// Begin mediator event notifications.

	// Notify mediators of response firing events.
	// This will propagate enabling values to effect events.
	for (mediator_iter = mediators.Begin();
		 mediator_iter != mediators.End(); mediator_iter++) {
		mediator = *mediator_iter;

		if (mediator->response != NULL)
			mediator->ResponseFiring(mediator->response->firing_strength);
	}

	// Recursively notify mediators of effect firing events.
	// Effect events are notified regardless of firing magnitude, in order
	// to update enablement for both firing and expiration outcomes.
	// Cause firing events are recorded for later notification;
	// these are deferred to allow a cause to fire as soon as
	// its effect event fires or expires.
	cause_firings.Clear();

	for (i = 0; i < (int)receptors.GetCount(); i++) {
		receptor = receptors[i];

		for (j = 0, k = (int)receptor->notify_list.GetCount(); j < k; j++) {
			notify   = receptor->notify_list[j];
			mediator = notify->mediator;

			if (notify->event_type == EFFECT_EVENT)
				mediator->EffectFiring(receptor->firing_strength);
		}
	}

	// Notify mediators of cause receptor firing events.
	for (i = 0; i < (int)receptors.GetCount(); i++) {
		receptor = receptors[i];

		if (receptor->firing_strength > 0.0) {
			for (j = 0, k = (int)receptor->notify_list.GetCount(); j < k; j++) {
				notify   = receptor->notify_list[j];
				mediator = notify->mediator;

				if (notify->event_type == CAUSE_EVENT)
					mediator->CauseFiring(receptor->firing_strength, event_clock);
			}
		}
	}

	// Notify mediators of remaining cause firing events.
	for (firing_notify_iter = cause_firings.Begin();
		 firing_notify_iter != cause_firings.End(); firing_notify_iter++) {
		firing_notify = *firing_notify_iter;
		mediator     = firing_notify.notify->mediator;
		mediator->CauseFiring(firing_notify.notify_strength,
							  firing_notify.cause_begin);
	}

	cause_firings.Clear();

	// Retire timed-out enablings.
	for (mediator_iter = mediators.Begin();
		 mediator_iter != mediators.End(); mediator_iter++) {
		mediator = *mediator_iter;
		mediator->RetireEnablings();
	}

	// Update effective enablements.
	for (mediator_iter = mediators.Begin();
		 mediator_iter != mediators.End(); mediator_iter++) {
		mediator = *mediator_iter;
		mediator->effective_enablement_valid = false;
	}

	for (mediator_iter = mediators.Begin();
		 mediator_iter != mediators.End(); mediator_iter++) {
		mediator = *mediator_iter;
		mediator->UpdateEffectiveEnablement();
	}
}


// Firing of mediator cause event.
void
Mediator::CauseFiring(WEIGHT notify_strength, Time cause_begin) {
	int        i;
	Enabling*   enabling;
	ENABLEMENT delta, enablement2;
	#ifdef MONA_TRACKING
	tracker.enable = true;
	#endif
	// Determine notification and enabling strength.
	delta = notify_strength * base_enablement;

	if (delta <= 0.0)
		return;

	base_enablement -= delta;

	// Distribute enablement to next neuron.
	for (i = 0; i < (int)mona->effect_event_interval_weights[level].GetCount(); i++) {
		enablement2 = delta * mona->effect_event_interval_weights[level][i];

		if (enablement2 > 0.0) {
			enabling = new Enabling(enablement2, motive, 0, i, cause_begin);
			ASSERT(enabling != NULL);
			enabling->SetNeeds(mona->homeostats);

			if (response != NULL) {
				response_enablings.Insert(enabling);
				#ifdef MONA_TRACKING
				response->tracker.enable = true;
				#endif
			}
			else {
				effect_enablings.Insert(enabling);
				#ifdef MONA_TRACKING
				effect->tracker.enable = true;
				#endif
			}
		}
	}
}


// Firing of mediator response event.
void
Mediator::ResponseFiring(WEIGHT notify_strength) {
	Enabling* enabling, *enabling2;
	Array<Enabling>::Iterator enabling_iter;
	ENABLEMENT                 enablement;

	// Transfer enablings to effect event.
	for (enabling_iter = response_enablings.enablings.Begin();
		 enabling_iter != response_enablings.enablings.End(); enabling_iter++) {
		enabling        = *enabling_iter;
		enablement      = enabling->value * notify_strength;
		base_enablement += (enabling->value - enablement);
		enabling->value = 0.0;

		if (enablement > 0.0) {
			enabling2         = enabling->Clone();
			enabling2->value  = enablement;
			enabling2->motive = motive;
			enabling2->age    = 1;
			effect_enablings.Insert(enabling2);
			#ifdef MONA_TRACKING
			effect->tracker.enable = true;
			#endif
		}
	}
}


// Firing of mediator effect event.
void
Mediator::EffectFiring(WEIGHT notify_strength) {
	int      i;
	Enabling* enabling;
	Array<Enabling>::Iterator enabling_iter;
	ENABLEMENT                 e, enablement;
	WEIGHT              strength;
	struct Notify*       notify;
	Mediator*            mediator;
	struct FiringNotify causeFiring;
	Vector<WEIGHT>      fire_weights, expire_weights;
	bool                parentContext;
	// If parent enabling context active, then parent's
	// enablement will be updated instead of current mediator.
	parentContext = false;

	for (i = 0; i < (int)notify_list.GetCount(); i++) {
		notify   = notify_list[i];
		mediator = notify->mediator;

		if (notify->event_type == EFFECT_EVENT) {
			if (mediator->effect_enablings.getOldValue() > 0.0) {
				parentContext = true;
				break;
			}
		}
	}

	// Accumulate effect enablement.
	cause_begin     = INVALID_TIME;
	enablement     = GetEnablement();
	firing_strength = 0.0;

	for (enabling_iter = effect_enablings.enablings.Begin();
		 enabling_iter != effect_enablings.enablings.End(); enabling_iter++) {
		enabling = *enabling_iter;

		if (enabling->value > 0.0) {
			if ((cause_begin == INVALID_TIME) || (enabling->cause_begin > cause_begin))
				cause_begin = enabling->cause_begin;
		}

		if ((e = notify_strength * enabling->value) > 0.0) {
			// Accumulate firing strength.
			firing_strength += e;
			//  Restore base enablement.
			enabling->value -= e;
			base_enablement  += e;

			// Save weight for enablement and utility updates.
			if (!parentContext)
				fire_weights.Add(e / enablement);

			// Update goal value.
			UpdateGoalValue(enabling->needs);
		}

		// Handle expired enablement.
		if ((enabling->value > 0.0) &&
			(enabling->age >= mona->effect_event_intervals[level][enabling->timer_index])) {
			if (!parentContext)
				expire_weights.Add(enabling->value / enablement);

			// Failed mediator might be generalizable.
			if (!instinct && (effect->type == RECEPTOR)) {
				GeneralizationEvent* event = new GeneralizationEvent(this, enabling->value);
				ASSERT(event != NULL);
				mona->generalization_events.Add(event);
			}

			// Restore enablement.
			base_enablement += enabling->value;
			enabling->value = 0.0;
		}
	}

	// Update enablement and utility for firing enablings.
	for (i = 0; i < (int)fire_weights.GetCount(); i++) {
		UpdateEnablement(FIRE, fire_weights[i]);
		UpdateUtility(fire_weights[i]);
	}

	// Update enablement for expired enablings.
	for (i = 0; i < (int)expire_weights.GetCount(); i++)
		UpdateEnablement(EXPIRE, expire_weights[i]);

	#ifdef MONA_TRACE

	if ((firing_strength > 0.0) && mona->trace_enable)
		printf("Mediator firing: %llu\n", id);

	#endif
	#ifdef MONA_TRACKING

	if (firing_strength > 0.0)
		tracker.fire = true;

	#endif
	// Notify parent mediators.
	strength = firing_strength / enablement;

	for (i = 0; i < (int)notify_list.GetCount(); i++) {
		notify   = notify_list[i];
		mediator = notify->mediator;

		if (notify->event_type == EFFECT_EVENT)
			mediator->EffectFiring(strength);
		else if (strength > 0.0) {
			// Record cause notification.
			causeFiring.notify         = notify;
			causeFiring.notify_strength = strength;
			causeFiring.cause_begin     = cause_begin;
			mona->cause_firings.Add(causeFiring);
		}
	}
}


// Get enablement including portions in enablings.
ENABLEMENT
Mediator::GetEnablement() {
	return (base_enablement +
			response_enablings.GetValue() +
			effect_enablings.GetValue());
}


// Update enablement.
void
Mediator::UpdateEnablement(EVENT_OUTCOME outcome, WEIGHT updateWeight) {
	ENABLEMENT e1, e2;
	double     r;
	Enabling*   enabling;
	Array<Enabling>::Iterator enabling_iter;

	// Instinct enablement cannot be updated.
	if (instinct)
		return;

	// Compute new enablement.
	e1 = e2 = GetEnablement();

	if (outcome == FIRE) {
		if (updateWeight > 0.0)
			e1 += (1.0 - e1) * updateWeight * mona->LEARNING_INCREASE_VELOCITY;
		else
			e1 += (e1 - mona->MIN_ENABLEMENT) * updateWeight * mona->LEARNING_DECREASE_VELOCITY;
	}
	else {                                         // EXPIRE
		if (updateWeight > 0.0)
			e1 += (mona->MIN_ENABLEMENT - e1) * updateWeight * mona->LEARNING_DECREASE_VELOCITY;
	}

	if (e1 < mona->MIN_ENABLEMENT)
		e1 = mona->MIN_ENABLEMENT;

	if (e1 > 1.0)
		e1 = 1.0;

	// Get scaling ratio of new to old enablement.
	if (e2 <= NEARLY_ZERO) {
		base_enablement = e1;
		r = 0.0;
	}
	else {
		r = e1 / e2;
		base_enablement *= r;
	}

	// Scale enablings.
	for (enabling_iter = response_enablings.enablings.Begin();
		 enabling_iter != response_enablings.enablings.End(); enabling_iter++) {
		enabling         = *enabling_iter;
		enabling->value *= r;
	}

	for (enabling_iter = effect_enablings.enablings.Begin();
		 enabling_iter != effect_enablings.enablings.End(); enabling_iter++) {
		enabling         = *enabling_iter;
		enabling->value *= r;
	}
}


// Update mediator effective enablement.
// The effective enablement combines a mediator's enablement
// with the enablements of its overlying mediator hierarchy
// to determine its ability to predict its effect event.
void
Mediator::UpdateEffectiveEnablement() {
	ENABLEMENT    e, et;
	struct Notify* notify;
	Mediator*      mediator;

	if (effective_enablement_valid)
		return;

	effective_enablement_valid = true;
	// Update parent effective enablements
	// and determine combined enabling effect.
	e = 1.0;

	for (int i = 0; i < (int)notify_list.GetCount(); i++) {
		notify   = notify_list[i];
		mediator = notify->mediator;

		if (notify->event_type == EFFECT_EVENT) {
			mediator->UpdateEffectiveEnablement();
			e *= (1.0 - (mediator->effective_enablement *
						 mediator->effective_enabling_weight));
		}
	}

	// Effective enablement combines parent and current enablements.
	et = GetEnablement();
	e *= (1.0 - et);
	e  = 1.0 - e;

	if (e < 0.0)
		e = 0.0;

	if (e > 1.0)
		e = 1.0;

	effective_enablement = e;
	// Determine effective enabling weight.
	effective_enabling_weight = effect_enablings.GetValue() / et;
}


// Retire enablings.
void
Mediator::RetireEnablings(bool force) {
	Enabling* enabling;
	Array<Enabling>::Iterator enabling_iter;

	// Age and retire response enablings.
	for (enabling_iter = response_enablings.enablings.Begin();
		 enabling_iter != response_enablings.enablings.End(); ) {
		enabling = *enabling_iter;
		enabling->age++;

		if (force || (enabling->age > 1)) {
			enabling_iter     = response_enablings.enablings.Remove(enabling_iter);
			base_enablement += enabling->value;
			delete enabling;
		}
		else
			enabling_iter++;
	}

	// Age and retire effect enablings.
	for (enabling_iter = effect_enablings.enablings.Begin();
		 enabling_iter != effect_enablings.enablings.End(); ) {
		enabling = *enabling_iter;
		enabling->age++;

		if (force ||
			(enabling->age > mona->effect_event_intervals[level][enabling->timer_index])) {
			enabling_iter     = effect_enablings.enablings.Remove(enabling_iter);
			base_enablement += enabling->value;
			delete enabling;
		}
		else
			enabling_iter++;
	}
}


// Update goal value.
void Mediator::UpdateGoalValue(VALUE_SET& needs) {
	NEED      need;
	VALUE_SET needsBase, need_deltas;

	if (level < mona->LEARN_MEDIATOR_GOAL_VALUE_MIN_LEVEL)
		return;

	needsBase.Reserve(mona->need_count);
	need_deltas.Reserve(mona->need_count);

	for (int i = 0; i < mona->need_count; i++) {
		needsBase.Set(i, mona->homeostats[i]->GetNeed());
		need = needsBase.Get(i) - needs.Get(i);

		if (need > 1.0)
			need = 1.0;

		if (need < -1.0)
			need = -1.0;

		need_deltas.Set(i, need);
	}

	goals.Update(needsBase, need_deltas);
}


// Expire response enablings.
void
Mona::ExpireResponseEnablings(RESPONSE expiringResponse) {
	int           i;
	Mediator*      mediator;
	Enabling*      enabling;
	ENABLEMENT    enablement;
	struct Notify* notify;
	Vector<WEIGHT>             expire_weights;
	Vector<Mediator*>::Iterator mediator_iter;
	Array<Enabling>::Iterator enabling_iter;

	for (mediator_iter = mediators.Begin();
		 mediator_iter != mediators.End(); mediator_iter++) {
		mediator = *mediator_iter;

		if ((mediator->response != NULL) && (((Motor*)mediator->response)->response == expiringResponse)) {
			enablement = mediator->GetEnablement();
			expire_weights.Clear();

			for (enabling_iter = mediator->response_enablings.enablings.Begin();
				 enabling_iter != mediator->response_enablings.enablings.End(); enabling_iter++) {
				enabling = *enabling_iter;

				if (enabling->value > 0.0) {
					expire_weights.Add(enabling->value / enablement);
					mediator->base_enablement += enabling->value;
					enabling->value           = 0.0;
				}
			}

			for (i = 0; i < (int)expire_weights.GetCount(); i++)
				mediator->UpdateEnablement(EXPIRE, expire_weights[i]);

			for (i = 0; i < (int)mediator->notify_list.GetCount(); i++) {
				notify = mediator->notify_list[i];

				if (notify->event_type == EFFECT_EVENT)
					ExpireMediatorEnablings(notify->mediator);
			}
		}
	}
}


// Expire mediator enablings.
void
Mona::ExpireMediatorEnablings(Mediator* mediator) {
	Enabling*   enabling;
	ENABLEMENT enablement;
	Vector<WEIGHT>             expire_weights;
	Array<Enabling>::Iterator enabling_iter;
	struct Notify*              notify;
	enablement = mediator->GetEnablement();

	for (enabling_iter = mediator->effect_enablings.enablings.Begin();
		 enabling_iter != mediator->effect_enablings.enablings.End(); enabling_iter++) {
		enabling = *enabling_iter;

		if (enabling->value > 0.0) {
			expire_weights.Add(enabling->value / enablement);
			mediator->base_enablement += enabling->value;
			enabling->value           = 0.0;
		}
	}

	for (int i = 0; i < (int)expire_weights.GetCount(); i++)
		mediator->UpdateEnablement(EXPIRE, expire_weights[i]);

	for (int i = 0; i < (int)mediator->notify_list.GetCount(); i++) {
		notify = mediator->notify_list[i];

		if (notify->event_type == EFFECT_EVENT)
			ExpireMediatorEnablings(notify->mediator);
	}
}
