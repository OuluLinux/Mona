#include "Mona.h"
#include <math.h>

// Enablement processing.
void Mona::Enable() {
	
	#ifdef MONA_TRACE
	if (trace_enable)
		printf("***Enable phase***\n");
	#endif

	// Clear mediators.
	for (int i = 0; i < mediators.GetCount(); i++) {
		Mediator& mediator = mediators[i];
		mediator.firing_strength = 0.0;
		mediator.response_enablings.ClearNewInSet();
		mediator.effect_enablings.ClearNewInSet();
	}

	// Begin mediator event notifications.

	// Notify mediators of response firing events.
	// This will propagate enabling values to effect events.
	for (int i = 0; i < mediators.GetCount(); i++) {
		Mediator& mediator = mediators[i];

		if (mediator.response != NULL)
			mediator.ResponseFiring(mediator.response->firing_strength);
	}

	// Recursively notify mediators of effect firing events.
	// Effect events are notified regardless of firing magnitude, in order
	// to update enablement for both firing and expiration outcomes.
	// Cause firing events are recorded for later notification;
	// these are deferred to allow a cause to fire as soon as
	// its effect event fires or expires.
	cause_firings.Clear();

	for (int i = 0; i < receptors.GetCount(); i++) {
		Receptor& receptor = receptors[i];

		for (int j = 0; j < receptor.notify_list.GetCount(); j++) {
			Notify& notify     = receptor.notify_list[j];
			ASSERT(!notify.mediator.IsNull());
			Mediator& mediator = notify.mediator;

			if (notify.event_type == EFFECT_EVENT)
				mediator.EffectFiring(receptor.firing_strength);
		}
	}

	// Notify mediators of cause receptor firing events.
	for (int i = 0; i < receptors.GetCount(); i++) {
		Receptor& receptor = receptors[i];

		if (receptor.firing_strength > 0.0) {
			for (int j = 0; j < receptor.notify_list.GetCount(); j++) {
				Notify& notify     = receptor.notify_list[j];
				ASSERT(!notify.mediator.IsNull());
				Mediator& mediator = notify.mediator;

				if (notify.event_type == CAUSE_EVENT)
					mediator.CauseFiring(receptor.firing_strength, event_clock);
			}
		}
	}

	// Notify mediators of remaining cause firing events.
	for (int i = 0; i < cause_firings.GetCount(); i++) {
		FiringNotify& firing_notify = cause_firings[i];
		ASSERT(!firing_notify.notify->mediator.IsNull());
		Mediator& mediator = firing_notify.notify->mediator;
		mediator.CauseFiring(firing_notify.notify_strength, firing_notify.cause_begin);
	}
	cause_firings.Clear();

	// Retire timed-out enablings.
	for (int i = 0; i < mediators.GetCount(); i++) {
		Mediator& mediator = mediators[i];
		mediator.RetireEnablings();
	}

	// Update effective enablements.
	for (int i = 0; i < mediators.GetCount(); i++) {
		Mediator& mediator = mediators[i];
		mediator.effective_enablement_valid = false;
	}

	for (int i = 0; i < mediators.GetCount(); i++) {
		Mediator& mediator = mediators[i];
		mediator.UpdateEffectiveEnablement();
	}
}



// Expire response enablings.
void Mona::ExpireResponseEnablings(RESPONSE expiringResponse) {
	Vector<WEIGHT> expire_weights;

	for (int i = 0; i < mediators.GetCount(); i++) {
		Mediator& mediator = mediators[i];

		if ((mediator.response != NULL) && (dynamic_cast<Motor*>(mediator.response.r)->response == expiringResponse)) {
			double enablement = mediator.GetEnablement();
			expire_weights.Clear();

			for (int j = 0; j < mediator.response_enablings.enablings.GetCount(); j++) {
				Enabling& enabling = mediator.response_enablings.enablings[j];

				if (enabling.value > 0.0) {
					expire_weights.Add(enabling.value / enablement);
					mediator.base_enablement += enabling.value;
					enabling.value           = 0.0;
				}
			}

			for (i = 0; i < expire_weights.GetCount(); i++)
				mediator.UpdateEnablement(EXPIRE, expire_weights[i]);

			for (i = 0; i < mediator.notify_list.GetCount(); i++) {
				Notify& notify = mediator.notify_list[i];

				if (notify.event_type == EFFECT_EVENT)
					ExpireMediatorEnablings(notify.mediator);
			}
		}
	}
}


// Expire mediator enablings.
void Mona::ExpireMediatorEnablings(Mediator& mediator) {
	ENABLEMENT enablement;
	Vector<WEIGHT> expire_weights;
	
	enablement = mediator.GetEnablement();
	
	for(int i = 0; i < mediator.effect_enablings.enablings.GetCount(); i++) {
		Enabling& enabling = mediator.effect_enablings.enablings[i];

		if (enabling.value > 0.0) {
			expire_weights.Add(enabling.value / enablement);
			mediator.base_enablement += enabling.value;
			enabling.value           = 0.0;
		}
	}

	for (int i = 0; i < expire_weights.GetCount(); i++)
		mediator.UpdateEnablement(EXPIRE, expire_weights[i]);

	for (int i = 0; i < mediator.notify_list.GetCount(); i++) {
		Notify& notify = mediator.notify_list[i];

		if (notify.event_type == EFFECT_EVENT)
			ExpireMediatorEnablings(notify.mediator);
	}
}
