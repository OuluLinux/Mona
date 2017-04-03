#include "Mona.h"

void GoalValue::Update(VALUE_SET& needs, VALUE_SET& need_deltas) {
	int    i, j;
	double v;
	update_count++;

	for (i = 0, j = needs.GetCount(); i < j; i++) {
		if (need_deltas.Get(i) > 0.0) {
			v = values.Get(i);
			values.Set(i, v - ((v / (double)update_count) *
							   mona->LEARNING_DECREASE_VELOCITY));
			continue;
		}

		if (need_deltas.Get(i) == 0.0) {
			if (needs.Get(i) > 0.0) {
				v = values.Get(i);
				values.Set(i, v - ((v / (double)update_count) *
								   mona->LEARNING_DECREASE_VELOCITY));
			}

			continue;
		}

		v = values.Get(i) + need_deltas.Get(i);

		if (needs.Get(i) == 0.0) {
			if (v >= 0.0)
				continue;
		}

		values.Set(i, values.Get(i) + ((-v / (double)update_count) *
									   mona->LEARNING_INCREASE_VELOCITY));
	}
}














/*LearningEvent::LearningEvent(Neuron* neuron) {
	this->neuron   = neuron;
	firing_strength = neuron->firing_strength;

	if (neuron->type == MEDIATOR)
		begin = ((Mediator*)neuron)->cause_begin;
	else
		begin = neuron->mona->event_clock;

	end = neuron->mona->event_clock;

	if (firing_strength > NEARLY_ZERO) {
		probability =
			pow(firing_strength, neuron->mona->FIRING_STRENGTH_LEARNING_DAMPER);
	}
	else
		probability = 0.0;

	int n = neuron->mona->need_count;
	needs.Reserve(n);

	for (int i = 0; i < n; i++)
		needs.Set(i, neuron->mona->homeostats[i]->GetNeed());
}*/

LearningEvent::LearningEvent() {
	neuron         = NULL;
	firing_strength = 0.0;
	begin.Set(0);
	end.Set(0);
	probability    = 0.0;
	needs.Clear();
}

void LearningEvent::SetNeuron(Neuron& neuron) {
	this->neuron   = &neuron;
	firing_strength = neuron.firing_strength;

	if (neuron.type == MEDIATOR)
		begin = dynamic_cast<Mediator&>(neuron).cause_begin;
	else
		begin = neuron.mona->event_clock;

	end = neuron.mona->event_clock;

	if (firing_strength > NEARLY_ZERO) {
		probability =
			pow(firing_strength, neuron.mona->FIRING_STRENGTH_LEARNING_DAMPER);
	}
	else
		probability = 0.0;

	int n = neuron.mona->need_count;
	needs.Reserve(n);

	for (int i = 0; i < n; i++)
		needs.Set(i, neuron.mona->homeostats[i]->GetNeed());
}

void LearningEvent::Serialize(Stream& fp) {
	fp % firing_strength % begin % end % probability;
	needs.Serialize(fp);

	if (fp.IsLoading()) neuron = NULL;
}
















GeneralizationEvent::GeneralizationEvent(Mediator& mediator, ENABLEMENT enabling) {
	this->mediator = &mediator;
	this->enabling = enabling;
	begin          = mediator.cause_begin;
	end            = mediator.mona->event_clock;
	int n = mediator.mona->need_count;
	needs.Reserve(n);

	for (int i = 0; i < n; i++)
		needs.Set(i, mediator.mona->homeostats[i]->GetNeed());
}

GeneralizationEvent::GeneralizationEvent() {
	mediator = NULL;
	enabling = 0.0;
	begin.Set(0);
	end.Set(0);
	needs.Clear();
}
