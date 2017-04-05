#include "Mona.h"




// Mediator constructor.
Mediator::Mediator() {
	type                     = MEDIATOR;
	level                    = 0;
	effective_enablement      = 0.0;
	effective_enabling_weight  = 0.0;
	effective_enablement_valid = false;
	utility_weight            = 0.0;
	cause      = response = effect = NULL;
	cause_begin = 0;
	base_enablement = 0;
}

void Mediator::Init(ENABLEMENT enablement, Mona* mona) {
	Neuron::Init(mona);
	type                     = MEDIATOR;
	level                    = 0;
	base_enablement           = enablement;
	effective_enablement      = 0.0;
	effective_enabling_weight  = 0.0;
	effective_enablement_valid = false;
	utility_weight            = 0.0;
	UpdateUtility(0.0);
	cause      = response = effect = NULL;
	cause_begin = 0;
}


// Mediator destructor.
Mediator::~Mediator() {
	struct Notify* notify;
	//Vector<struct Notify*>::Iterator notify_iter;

	if (!cause.IsNull()) {
		for(int i = 0; i < cause->notify_list.GetCount(); i++) {
			Notify& notify = cause->notify_list[i];

			if (notify.mediator == this) {
				cause->notify_list.Remove(i);
				i--;
			}
		}
	}

	cause = NULL;

	if (!response.IsNull()) {
		for(int i = 0; i < response->notify_list.GetCount(); i++) {
			Notify& notify = response->notify_list[i];

			if (notify.mediator == this) {
				response->notify_list.Remove(i);
				i--;
			}
		}
	}

	response = NULL;

	if (!effect.IsNull()) {
		for(int i = 0; i < effect->notify_list.GetCount(); i++) {
			Notify& notify = effect->notify_list[i];

			if (notify.mediator == this) {
				effect->notify_list.Remove(i);
				i--;
			}
		}
	}

	effect = NULL;
	response_enablings.Clear();
	effect_enablings.Clear();
	Clear();
}


// Update mediator utility using its enablement
// and an asymptotic increment based on use.
void Mediator::UpdateUtility(WEIGHT update_weight) {
	WEIGHT w;
	// Check for floating point overflow.
	w = utility_weight + update_weight + mona->UTILITY_ASYMPTOTE;

	if (w > (utility_weight + update_weight))
		utility_weight += update_weight;

	// Boost utility.
	w       = utility_weight / (mona->UTILITY_ASYMPTOTE + utility_weight);
	utility = ((UTILITY)GetEnablement() + w) / 2.0;
}


// Get mediator effective utility,
// which is the greatest utility of itself and its parents.
UTILITY Mediator::GetEffectiveUtility() {
	UTILITY  best_utility, utility_work;
	best_utility = utility;

	for (int i = 0; i < notify_list.GetCount(); i++) {
		Mediator& mediator = notify_list[i].mediator;
		utility_work = mediator.GetEffectiveUtility();

		if (utility_work > best_utility)
			best_utility = utility_work;
	}

	return best_utility;
}


// Add mediator event.
void Mediator::AddEvent(int type, Neuron& neuron) {

	switch (type) {
		case CAUSE_EVENT:
			ASSERT(cause.IsNull());
			cause = &neuron;
			break;
	
		case RESPONSE_EVENT:
			ASSERT(response.IsNull());
			response = &neuron;
			break;
	
		case EFFECT_EVENT:
			ASSERT(effect.IsNull());
			effect = &neuron;
			break;
			
		default: Panic("Unknown event");
	}

	if (neuron.type == MEDIATOR) {
		Mediator& mediator = dynamic_cast<Mediator&>(neuron);

		if (mediator.level + 1 > level) {
			level = mediator.level + 1;
			ASSERT(level <= mona->MAX_MEDIATOR_LEVEL);
		}
	}

	Notify& notify = neuron.notify_list.Add();
	notify.mediator = this;
	notify.event_type = type;

	// Sort by type: effect, response, cause.
	int count = neuron.notify_list.GetCount();
	for (int i = 0; i < count; i++) {
		for (int k = i + 1; k < count; k++) {
			switch (neuron.notify_list[k].event_type) {
				
			case EFFECT_EVENT:
				switch (neuron.notify_list[i].event_type) {
				case EFFECT_EVENT:
					break;

				case RESPONSE_EVENT:
				case CAUSE_EVENT:
					notify = neuron.notify_list[i];
					neuron.notify_list[i] = neuron.notify_list[k];
					neuron.notify_list[k] = notify;
					break;
				}

				break;

			case RESPONSE_EVENT:
				switch (neuron.notify_list[i].event_type) {
				case EFFECT_EVENT:
					break;

				case RESPONSE_EVENT:
					break;

				case CAUSE_EVENT:
					notify = neuron.notify_list[i];
					neuron.notify_list[i] = neuron.notify_list[k];
					neuron.notify_list[k] = notify;
					break;
				}

				break;

			case CAUSE_EVENT:
				break;
				
			}
		}
	}
}


// Is given mediator a duplicate of this?
bool Mediator::IsDuplicate(Mediator& mediator) {
	if (cause != mediator.cause)
		return false;

	if (response != mediator.response)
		return false;

	if (effect != mediator.effect)
		return false;

	return true;
}


// Load mediator.
void Mediator::Serialize(Stream& fp) {
	Neuron::Serialize(fp);
	
	fp % level % base_enablement % utility % utility_weight;
	fp % cause % response % effect % response_enablings % effect_enablings % cause_begin;
}


// Save mediator.
// When changing format increment FORMAT in mona.h
/*void Mediator::Store(Stream& fp) {
	((Neuron*)this)->Store(fp);
	FWRITE_INT(&level, fp);
	FWRITE_DOUBLE(&base_enablement, fp);
	FWRITE_DOUBLE(&utility, fp);
	FWRITE_DOUBLE(&utility_weight, fp);
	FWRITE_LONG_LONG(&cause->id, fp);

	if (response != NULL) {
		i = 1;
		FWRITE_INT(&i, fp);
		FWRITE_LONG_LONG(&response->id, fp);
	}
	else {
		i = 0;
		FWRITE_INT(&i, fp);
	}

	FWRITE_LONG_LONG(&effect->id, fp);
	response_enablings.Store(fp);
	effect_enablings.Store(fp);
	FWRITE_LONG_LONG(&cause_begin, fp);
}*/

/*
    // Print mediator.
    #ifdef MONA_TRACKING
    void Mediator::Print(FILE *out)
    {
    Print((TRACKING_FLAGS)0, out);
    }


    void Mediator::Print(TRACKING_FLAGS tracking, FILE *out)
    {
    Print(tracking, false, 0, out);
    }


    #else
    void Mediator::Print(FILE *out)
    {
    Print(false, 0, out);
    }


    #endif

    #ifdef MONA_TRACKING
    void Mediator::PrintBrief(FILE *out)
    {
    Print((TRACKING_FLAGS)0, true, 0, out);
    }


    void Mediator::PrintBrief(TRACKING_FLAGS tracking, FILE *out)
    {
    Print(tracking, true, 0, out);
    }


    #else
    void Mediator::PrintBrief(FILE *out)
    {
    Print(true, 0, out);
    }


    #endif

    #ifdef MONA_TRACKING
    void Mediator::Print(TRACKING_FLAGS tracking, bool brief,
                           int level, FILE *out)
    #else
    void Mediator::Print(bool brief, int level, FILE *out)
    #endif
    {
    
    for (i = 0; i < level; i++)
    {
      fprintf(out, "    ");
    }
    fprintf(out, "<mediator>\n");
    for (i = 0; i < level; i++)
    {
      fprintf(out, "    ");
    }
    fprintf(out, "  <id>%llu</id>\n", id);
    for (i = 0; i < level; i++)
    {
      fprintf(out, "    ");
    }
    fprintf(out, "  <level>%d</level>\n", this->level);
    for (i = 0; i < level; i++)
    {
      fprintf(out, "    ");
    }
    fprintf(out, "  <creation_time>%llu</creation_time>\n", this->creation_time);
    for (i = 0; i < level; i++)
    {
      fprintf(out, "    ");
    }
    if (instinct)
    {
      fprintf(out, "  <instinct>true</instinct>\n");
    }
    else
    {
      fprintf(out, "  <instinct>false</instinct>\n");
    }
    for (i = 0; i < level; i++)
    {
      fprintf(out, "    ");
    }
    fprintf(out, "  <enablement>%f</enablement>\n", GetEnablement());
    for (i = 0; i < level; i++)
    {
      fprintf(out, "    ");
    }
    fprintf(out, "  <utility>%f</utility>\n", utility);
    for (i = 0; i < level; i++)
    {
      fprintf(out, "    ");
    }
    fprintf(out, "  <utility_weight>%f</utility_weight>\n", utility_weight);
    for (i = 0; i < level; i++)
    {
      fprintf(out, "    ");
    }
    fprintf(out, "  <goals>");
    goals.Print(out);
    fprintf(out, "</goals>\n");
    #ifdef MONA_TRACKING
    if ((tracking & TRACK_FIRE) && tracker.fire)
    {
      for (i = 0; i < level; i++)
      {
         fprintf(out, "    ");
      }
      fprintf(out, "  <fire>true</fire>\n");
    }
    if ((tracking & TRACK_ENABLE) && tracker.enable)
    {
      for (i = 0; i < level; i++)
      {
         fprintf(out, "    ");
      }
      fprintf(out, "  <enable>true</enable>\n");
    }
    if ((tracking & TRACK_DRIVE) && tracker.drive)
    {
      for (i = 0; i < level; i++)
      {
         fprintf(out, "    ");
      }
      fprintf(out, "  <motive>%f</motive>\n", tracker.motive);
    }
    #endif
    for (i = 0; i < level; i++)
    {
      fprintf(out, "    ");
    }
    fprintf(out, "  <cause>\n");
    if (cause->type == MEDIATOR)
    {
      if (brief)
      {
         for (i = 0; i <= level; i++)
         {
            fprintf(out, "    ");
         }
         fprintf(out, "<mediator><id>%llu</id></mediator>\n", cause->id);
      }
      else
      {
    #ifdef MONA_TRACKING
         ((Mediator *)cause)->Print(tracking, false, level + 1, out);
    #else
         ((Mediator *)cause)->Print(false, level + 1, out);
    #endif
      }
    }
    else
    {
      if (brief)
      {
         for (i = 0; i <= level; i++)
         {
            fprintf(out, "    ");
         }
         fprintf(out, "<receptor><id>%llu</id></receptor>\n", cause->id);
      }
      else
      {
         for (i = 0; i <= level; i++)
         {
            fprintf(out, "    ");
         }
    #ifdef MONA_TRACKING
         ((Receptor *)cause)->Print(tracking, out);
    #else
         ((Receptor *)cause)->Print(out);
    #endif
         fprintf(out, "\n");
      }
    }
    for (i = 0; i < level; i++)
    {
      fprintf(out, "    ");
    }
    fprintf(out, "  </cause>\n");
    if (response != NULL)
    {
      if (brief)
      {
         for (i = 0; i < level; i++)
         {
            fprintf(out, "    ");
         }
         fprintf(out, "  <response>\n");
         for (i = 0; i <= level; i++)
         {
            fprintf(out, "    ");
         }
         fprintf(out, "<motor><id>%llu</id></motor>\n", response->id);
         for (i = 0; i < level; i++)
         {
            fprintf(out, "    ");
         }
         fprintf(out, "  </response>\n");
      }
      else
      {
         for (i = 0; i < level; i++)
         {
            fprintf(out, "    ");
         }
         fprintf(out, "  <response>\n");
         for (i = 0; i <= level; i++)
         {
            fprintf(out, "    ");
         }
    #ifdef MONA_TRACKING
         ((Motor *)response)->Print(tracking, out);
    #else
         ((Motor *)response)->Print(out);
    #endif
         fprintf(out, "\n");
         for (i = 0; i < level; i++)
         {
            fprintf(out, "    ");
         }
         fprintf(out, "  </response>\n");
      }
    }
    for (i = 0; i < level; i++)
    {
      fprintf(out, "    ");
    }
    fprintf(out, "  <effect>\n");
    if (effect->type == MEDIATOR)
    {
      if (brief)
      {
         for (i = 0; i <= level; i++)
         {
            fprintf(out, "    ");
         }
         fprintf(out, "<mediator><id>%llu</id></mediator>\n", effect->id);
      }
      else
      {
    #ifdef MONA_TRACKING
         ((Mediator *)effect)->Print(tracking, false, level + 1, out);
    #else
         ((Mediator *)effect)->Print(false, level + 1, out);
    #endif
      }
    }
    else
    {
      if (brief)
      {
         for (i = 0; i <= level; i++)
         {
            fprintf(out, "    ");
         }
         fprintf(out, "<receptor><id>%llu</id></receptor>\n", effect->id);
      }
      else
      {
         for (i = 0; i <= level; i++)
         {
            fprintf(out, "    ");
         }
    #ifdef MONA_TRACKING
         ((Receptor *)effect)->Print(tracking, out);
    #else
         ((Receptor *)effect)->Print(out);
    #endif
         fprintf(out, "\n");
      }
    }
    for (i = 0; i < level; i++)
    {
      fprintf(out, "    ");
    }
    fprintf(out, "  </effect>\n");
    for (i = 0; i < level; i++)
    {
      fprintf(out, "    ");
    }
    fprintf(out, "</mediator>\n");
    }
*/



// Firing of mediator cause event.
void Mediator::CauseFiring(WEIGHT notify_strength, int64 cause_begin) {
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
	for (int i = 0; i < mona->effect_event_interval_weights[level].GetCount(); i++) {
		enablement2 = delta * mona->effect_event_interval_weights[level][i];

		if (enablement2 > 0.0) {
			if (response != NULL) {
				Enabling& enabling = response_enablings.Add();
				enabling.Init(enablement2, motive, 0, i, cause_begin);
				enabling.SetNeeds(mona->homeostats);
				
				#ifdef MONA_TRACKING
				response->tracker.enable = true;
				#endif
			}
			else {
				Enabling& enabling = effect_enablings.Add();
				enabling.Init(enablement2, motive, 0, i, cause_begin);
				enabling.SetNeeds(mona->homeostats);
				
				#ifdef MONA_TRACKING
				effect->tracker.enable = true;
				#endif
			}
		}
	}
}


// Firing of mediator response event.
void Mediator::ResponseFiring(WEIGHT notify_strength) {
	ENABLEMENT                 enablement;

	// Transfer enablings to effect event.
	for(int i = 0; i < response_enablings.enablings.GetCount(); i++) {
		Enabling& enabling = response_enablings.enablings[i];
		enablement = enabling.value * notify_strength;
		base_enablement += (enabling.value - enablement);
		enabling.value = 0.0;

		if (enablement > 0.0) {
			Enabling& enabling2 = effect_enablings.Add();
			enabling.Clone(enabling2);
			enabling2.value    = enablement;
			enabling2.motive   = motive;
			enabling2.age      = 1;
			
			#ifdef MONA_TRACKING
			effect->tracker.enable = true;
			#endif
		}
	}
}


// Firing of mediator effect event.
void Mediator::EffectFiring(WEIGHT notify_strength) {
	ENABLEMENT          e, enablement;
	WEIGHT              strength;
	FiringNotify        cause_firing;
	Vector<WEIGHT>      fire_weights, expire_weights;
	bool                parent_context;
	
	// If parent enabling context active, then parent's
	// enablement will be updated instead of current mediator.
	parent_context = false;

	for (int i = 0; i < notify_list.GetCount(); i++) {
		Notify& notify   = notify_list[i];
		Mediator& mediator = notify.mediator;

		if (notify.event_type == EFFECT_EVENT) {
			if (mediator.effect_enablings.GetOldValue() > 0.0) {
				parent_context = true;
				break;
			}
		}
	}

	// Accumulate effect enablement.
	cause_begin     = INVALID_TIME;
	enablement      = GetEnablement();
	firing_strength = 0.0;
	
	for(int i = 0; i < effect_enablings.enablings.GetCount(); i++) {
		Enabling& enabling = effect_enablings.enablings[i];

		if (enabling.value > 0.0) {
			if ((cause_begin == INVALID_TIME) || (enabling.cause_begin > cause_begin))
				cause_begin = enabling.cause_begin;
		}

		if ((e = notify_strength * enabling.value) > 0.0) {
			
			// Accumulate firing strength.
			firing_strength += e;
			
			//  Restore base enablement.
			enabling.value -= e;
			base_enablement  += e;

			// Save weight for enablement and utility updates.
			if (!parent_context)
				fire_weights.Add(e / enablement);

			// Update goal value.
			UpdateGoalValue(enabling.needs);
		}

		// Handle expired enablement.
		if ((enabling.value > 0.0) &&
			(enabling.age >= mona->effect_event_intervals[level][enabling.timer_index])) {
				
			if (!parent_context)
				expire_weights.Add(enabling.value / enablement);

			// Failed mediator might be generalizable.
			if (!instinct && (effect->type == RECEPTOR)) {
				GeneralizationEvent& event = mona->generalization_events.Add();
				event.Init(*this, enabling.value);
			}

			// Restore enablement.
			base_enablement += enabling.value;
			enabling.value = 0.0;
		}
	}

	// Update enablement and utility for firing enablings.
	for (int i = 0; i < fire_weights.GetCount(); i++) {
		UpdateEnablement(FIRE, fire_weights[i]);
		UpdateUtility(fire_weights[i]);
	}

	// Update enablement for expired enablings.
	for (int i = 0; i < expire_weights.GetCount(); i++)
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

	for (int i = 0; i < notify_list.GetCount(); i++) {
		Notify& notify = notify_list[i];
		Mediator& mediator = notify.mediator;

		if (notify.event_type == EFFECT_EVENT)
			mediator.EffectFiring(strength);
		else if (strength > 0.0) {
			// Record cause notification.
			cause_firing.notify          = &notify;
			cause_firing.notify_strength = strength;
			cause_firing.cause_begin     = cause_begin;
			mona->cause_firings.Add(cause_firing);
		}
	}
}


// Get enablement including portions in enablings.
ENABLEMENT Mediator::GetEnablement() {
	return (base_enablement +
			response_enablings.GetValue() +
			effect_enablings.GetValue());
}


// Update enablement.
void Mediator::UpdateEnablement(int outcome, WEIGHT update_weight) {
	ENABLEMENT e1, e2;
	double     r;
	Enabling*   enabling;
	//Array<Enabling>::Iterator enabling_iter;

	// Instinct enablement cannot be updated.
	if (instinct)
		return;

	// Compute new enablement.
	e1 = e2 = GetEnablement();

	if (outcome == FIRE) {
		if (update_weight > 0.0)
			e1 += (1.0 - e1) * update_weight * mona->LEARNING_INCREASE_VELOCITY;
		else
			e1 += (e1 - mona->MIN_ENABLEMENT) * update_weight * mona->LEARNING_DECREASE_VELOCITY;
	}
	else {                                         // EXPIRE
		if (update_weight > 0.0)
			e1 += (mona->MIN_ENABLEMENT - e1) * update_weight * mona->LEARNING_DECREASE_VELOCITY;
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
	for(int i = 0; i < response_enablings.enablings.GetCount(); i++) {
		response_enablings.enablings[i].value *= r;
	}
	
	for(int i = 0; i < effect_enablings.enablings.GetCount(); i++) {
		effect_enablings.enablings[i].value *= r;
	}
}


// Update mediator effective enablement.
// The effective enablement combines a mediator's enablement
// with the enablements of its overlying mediator hierarchy
// to determine its ability to predict its effect event.
void Mediator::UpdateEffectiveEnablement() {
	ENABLEMENT e, et;
	
	if (effective_enablement_valid)
		return;

	effective_enablement_valid = true;
	// Update parent effective enablements
	// and determine combined enabling effect.
	e = 1.0;

	for (int i = 0; i < notify_list.GetCount(); i++) {
		Notify& notify   = notify_list[i];
		Mediator& mediator = notify.mediator;

		if (notify.event_type == EFFECT_EVENT) {
			mediator.UpdateEffectiveEnablement();
			e *= (1.0 - (mediator.effective_enablement *
						 mediator.effective_enabling_weight));
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
void Mediator::RetireEnablings(bool force) {
	
	// Age and retire response enablings.
	for(int i = 0; i < response_enablings.enablings.GetCount(); ) {
		Enabling& enabling = response_enablings.enablings[i];
		enabling.age++;

		if (force || (enabling.age > 1)) {
			base_enablement += enabling.value;
			response_enablings.enablings.Remove(i);
		}
		else
			i++;
	}

	// Age and retire effect enablings.
	for(int i = 0; i < effect_enablings.enablings.GetCount(); ) {
		Enabling& enabling = effect_enablings.enablings[i];
		enabling.age++;

		if (force ||
			(enabling.age > mona->effect_event_intervals[level][enabling.timer_index])) {
			base_enablement += enabling.value;
			effect_enablings.enablings.Remove(i);
		}
		else
			i++;
	}
}


// Update goal value.
void Mediator::UpdateGoalValue(const VALUE_SET& needs) {
	NEED      need;
	VALUE_SET needs_base, need_deltas;

	if (level < mona->LEARN_MEDIATOR_GOAL_VALUE_MIN_LEVEL)
		return;

	needs_base.Reserve(mona->need_count);
	need_deltas.Reserve(mona->need_count);

	for (int i = 0; i < mona->need_count; i++) {
		needs_base.Set(i, mona->homeostats[i].GetNeed());
		need = needs_base.Get(i) - needs.Get(i);

		if (need > 1.0)
			need = 1.0;

		if (need < -1.0)
			need = -1.0;

		need_deltas.Set(i, need);
	}

	goals.Update(needs_base, need_deltas);
}



// Is goal value subsumed by component?
bool Mediator::IsGoalValueSubsumed() {
	NEED goal_value;
	
	Neuron* cause      = this->cause;
	Neuron* response   = this->response;
	Neuron* effect     = this->effect;
	
	Vector<bool>   goal_blocks;
	goal_blocks.SetCount(mona->need_count, false);

	while (effect != NULL) {
		for (int i = 0; i < mona->need_count; i++) {
			goal_value = goals.GetValue(i);

			if (goal_value > 0.0) {
				if ((cause->goals.GetValue(i) >= goal_value) ||
					(effect->goals.GetValue(i) >= goal_value) ||
					((response != NULL) && (response->goals.GetValue(i) >= goal_value)))
					goal_blocks[i] = true;
			}
			else {
				if ((cause->goals.GetValue(i) <= goal_value) ||
					(effect->goals.GetValue(i) <= goal_value) ||
					((response != NULL) && (response->goals.GetValue(i) <= goal_value)))
					goal_blocks[i] = true;
			}
		}
		
		int j = -1;
		for (int i = 0; i < mona->need_count; i++) {
			if (!goal_blocks[i]) {
				j = i;
				break;
			}
		}

		if (j == -1)
			break;

		if (effect->type == MEDIATOR) {
			Mediator* m = dynamic_cast<Mediator*>(effect);
			ASSERT(m);
			cause    = m->cause;
			response = m->response;
			effect   = m->effect;
		}
		else
			effect = NULL;
	}
	
	if (effect != NULL)
		return true;
	else
		return false;
}



// Drive mediator cause.
void Mediator::DriveCause(MotiveAccum& motive_accum) {
	MOTIVE      m;
	WEIGHT      w;
	MotiveAccum accum_work;
	
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
		printf("Drive cause %llu, motive=%f\n", id, m);
	#endif

	// Drive motive to cause event.
	if ((w = drive_weights.Get(cause->mem_id)) > NEARLY_ZERO) {
		accum_work.Configure(motive_accum, w * (1.0 - mona->DRIVE_ATTENUATION));
		cause->Drive(accum_work);
	}

	// Drive motive to response event.
	if (!response.IsNull()) {
		if ((w = drive_weights.Get(response->mem_id)) > NEARLY_ZERO) {
			accum_work.Configure(motive_accum, w * (1.0 - mona->DRIVE_ATTENUATION));
			response->Drive(accum_work);
		}
	}

	// Drive motive to parent mediators.
	for (int i = 0; i < notify_list.GetCount(); i++) {
		Mediator& mediator = notify_list[i].mediator;

		if ((w = drive_weights.Get(mediator.mem_id)) > NEARLY_ZERO) {
			accum_work.Configure(motive_accum, w * (1.0 - mona->DRIVE_ATTENUATION));
			mediator.DriveCause(accum_work);
		}
	}
}


