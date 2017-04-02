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
