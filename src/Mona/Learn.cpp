// For conditions of distribution and use, see copyright notice in mona.h

/*
 * Learn environment by creating mediator neurons that mediate
 * cause-effect sequences of neuron firing events.
 */

#include "Mona.h"

void
Mona::Learn()
{
   int           i, j, k;
   LearningEvent *learning_event;

   list<LearningEvent *>::Iterator learning_eventItr;
   Motor    *motor;
   Receptor *receptor;
   Mediator *mediator;
   list<Mediator *>::Iterator mediator_iter;

#ifdef MONA_TRACE
   if (trace_learn)
   {
      printf("***Learn phase***\n");
   }
#endif

   // Purge obsolete events.
   j = MAX_RESPONSE_EQUIPPED_MEDIATOR_LEVEL;
   if (j > MAX_MEDIATOR_LEVEL)
   {
      j = MAX_MEDIATOR_LEVEL;
   }
   for (i = 0; i < (int)learning_events.GetCount(); i++)
   {
      for (learning_eventItr = learning_events[i].Begin();
           learning_eventItr != learning_events[i].End(); )
      {
         learning_event = *learning_eventItr;
         if (learning_event->firing_strength > NEARLY_ZERO)
         {
            // Is event still recent enough to form a new mediator?
            if (learning_event->neuron->type == MOTOR)
            {
               k = j;
            }
            else
            {
               k = i;
            }
            if ((int)(event_clock - learning_event->end) <= max_learning_effect_event_intervals[k])
            {
               learning_eventItr++;
            }
            else
            {
               learning_eventItr = learning_events[i].erase(learning_eventItr);
               delete learning_event;
            }
         }
         else
         {
            learning_eventItr = learning_events[i].erase(learning_eventItr);
            delete learning_event;
         }
      }
   }

   // Time-stamp and store significant events.
   for (i = 0; i < (int)receptors.GetCount(); i++)
   {
      receptor = receptors[i];
      if (receptor->firing_strength > NEARLY_ZERO)
      {
         learning_event = new LearningEvent(receptor);
         assert(learning_event != NULL);
         learning_events[0].Add(learning_event);
      }
   }
   for (i = 0; i < (int)motors.GetCount(); i++)
   {
      motor = motors[i];
      if (motor->firing_strength > NEARLY_ZERO)
      {
         learning_event = new LearningEvent(motor);
         assert(learning_event != NULL);
         learning_events[0].Add(learning_event);
      }
   }
   for (mediator_iter = mediators.Begin();
        mediator_iter != mediators.End(); mediator_iter++)
   {
      mediator = *mediator_iter;
      if (!mediator->instinct && (mediator->firing_strength > NEARLY_ZERO))
      {
         if ((mediator->level + 1) < (int)learning_events.GetCount())
         {
            learning_event = new LearningEvent(mediator);
            assert(learning_event != NULL);
            learning_events[mediator->level + 1].Add(learning_event);
         }
      }
   }

   // Create new mediators based on potential effect events.
   for (i = 0; i < (int)learning_events.GetCount(); i++)
   {
      for (learning_eventItr = learning_events[i].Begin();
           learning_eventItr != learning_events[i].End(); learning_eventItr++)
      {
         learning_event = *learning_eventItr;
         if ((learning_event->end == event_clock) &&
             ((learning_event->neuron->type == MEDIATOR) ||
              ((learning_event->neuron->type == RECEPTOR) &&
               (((Receptor *)learning_event->neuron)->sensor_mode == 0))))
         {
            CreateMediator(learning_event);
         }
      }
   }

   // Create mediators with generalized receptor effects.
   for (i = 0; i < (int)generalization_events.GetCount(); i++)
   {
      GeneralizeMediator(generalization_events[i]);
      delete generalization_events[i];
   }
   generalization_events.Clear();

   // Delete excess mediators.
   while ((int)mediators.GetCount() > MAX_MEDIATORS)
   {
      if ((mediator = GetWorstMediator()) == NULL)
      {
         break;
      }
      DeleteNeuron(mediator);
   }

   // Increment event clock.
   event_clock++;
}


// Create new mediators for given effect.
void
Mona::CreateMediator(LearningEvent *effectEvent)
{
   int           i, level;
   bool          effect_resp_eq, cause_resp_eq;
   LearningEvent *cause_event, *response_event, *learning_event;

   list<LearningEvent *>::Iterator cause_eventItr,
                                   response_eventItr;
   Vector<LearningEvent *>    tmpVector3f;
   Mediator                   *mediator;
   list<Mediator *>::Iterator mediator_iter;
   Vector<LearningEvent *>    candidates;
   PROBABILITY                accum_prob, choose_prob, p;

   // Check event firing strength.
   if (effectEvent->firing_strength <= NEARLY_ZERO)
   {
      return;
   }

   // Find cause event candidates.
   accum_prob = 0.0;
   if (effectEvent->neuron->type != MEDIATOR)
   {
      level        = 0;
      effect_resp_eq = true;
   }
   else
   {
      mediator = (Mediator *)effectEvent->neuron;
      level    = mediator->level + 1;
      if (level > MAX_MEDIATOR_LEVEL)
      {
         return;
      }
      if (mediator->response != NULL)
      {
         effect_resp_eq = true;
      }
      else
      {
         effect_resp_eq = false;
      }
   }
   for (cause_eventItr = learning_events[level].Begin();
        cause_eventItr != learning_events[level].End(); cause_eventItr++)
   {
      cause_event = *cause_eventItr;
      if (cause_event->firing_strength <= NEARLY_ZERO)
      {
         continue;
      }
      if (cause_event->neuron->type == MOTOR)
      {
         continue;
      }
      if (cause_event->end >= effectEvent->begin)
      {
         continue;
      }

      // The cause and effect must have equal response-equippage status.
      if ((level == 0) || (((Mediator *)cause_event->neuron)->response != NULL))
      {
         cause_resp_eq = true;
      }
      else
      {
         cause_resp_eq = false;
      }
      if (cause_resp_eq != effect_resp_eq)
      {
         continue;
      }

      // Save candidate.
      candidates.Add(cause_event);
      accum_prob += cause_event->probability;
   }

   // Choose causes and create mediators.
   while (true)
   {
      // Make a weighted probabilistic pick of a candidate.
      choose_prob = random.RAND_INTERVAL(0.0, accum_prob);
      double p = 0.0;
      for (int i = 0; i < (int)candidates.GetCount(); i++)
      {
         if (candidates[i] == NULL)
         {
            continue;
         }
         p += candidates[i]->probability;
         if (choose_prob <= p)
         {
            break;
         }
      }
      if (i == (int)candidates.GetCount())
      {
         break;
      }
      cause_event    = candidates[i];
      accum_prob    -= cause_event->probability;
      candidates[i] = NULL;

      // Make a probabilistic decision to create mediator.
      if (!random.RAND_CHANCE(effectEvent->probability *
                              cause_event->probability))
      {
         continue;
      }

      // Add a response?
      // The components must also be response-equipped.
      response_event = NULL;
      if ((level < MIN_RESPONSE_UNEQUIPPED_MEDIATOR_LEVEL) ||
          (effect_resp_eq &&
           (level <= MAX_RESPONSE_EQUIPPED_MEDIATOR_LEVEL) &&
           random.RAND_BOOL()))
      {
         tmpVector3f.Clear();
         for (response_eventItr = learning_events[0].Begin();
              response_eventItr != learning_events[0].End(); response_eventItr++)
         {
            response_event = *response_eventItr;
            if ((response_event->neuron->type == MOTOR) &&
                (response_event->firing_strength > NEARLY_ZERO) &&
                (response_event->end == cause_event->end + 1))
            {
               tmpVector3f.Add(response_event);
            }
         }
         if (tmpVector3f.GetCount() == 0)
         {
            continue;
         }
         response_event = tmpVector3f[random.RAND_CHOICE((int)tmpVector3f.GetCount())];
      }

      // Create the mediator.
      mediator = NewMediator(INITIAL_ENABLEMENT);
      mediator->AddEvent(CAUSE_EVENT, cause_event->neuron);
      if (response_event != NULL)
      {
         mediator->AddEvent(RESPONSE_EVENT, response_event->neuron);
      }
      mediator->AddEvent(EFFECT_EVENT, effectEvent->neuron);
      mediator->UpdateGoalValue(cause_event->needs);

      // Duplicate?
      if (IsDuplicateMediator(mediator))
      {
         DeleteNeuron(mediator);
         continue;
      }

      // Make new mediator available for learning.
      if ((mediator->level + 1) < (int)learning_events.GetCount())
      {
         mediator->causeBegin     = cause_event->begin;
         mediator->firing_strength = cause_event->firing_strength *
                                    effectEvent->firing_strength;
         learning_event = new LearningEvent(mediator);
         assert(learning_event != NULL);
         learning_events[mediator->level + 1].Add(learning_event);
      }

#ifdef MONA_TRACE
      if (trace_learn)
      {
         printf("Create mediator:\n");
         mediator->Print();
      }
#endif
   }
}


// Create generalized mediators.
void
Mona::GeneralizeMediator(GeneralizationEvent *generalizationEvent)
{
   LearningEvent *candidate_event, *learning_event;

   list<LearningEvent *>::Iterator candidate_event_iter;
   Receptor                *effect_receptor, *candidate_receptor;
   Mediator                *mediator;
   Vector<LearningEvent *> candidates;
   PROBABILITY             accum_prob, choose_prob, p;
   int i;

   // Find effect event candidates.
   accum_prob = 0.0;
   for (candidate_event_iter = learning_events[0].Begin();
        candidate_event_iter != learning_events[0].End(); candidate_event_iter++)
   {
      candidate_event = *candidate_event_iter;
      if (candidate_event->firing_strength <= NEARLY_ZERO)
      {
         continue;
      }
      if (candidate_event->neuron->type != RECEPTOR)
      {
         continue;
      }
      if (candidate_event->end != event_clock)
      {
         continue;
      }

      // Is this a direct superset of the mediator's effect?
      candidate_receptor = (Receptor *)candidate_event->neuron;
      effect_receptor    = (Receptor *)generalizationEvent->mediator->effect;
      for (i = 0; i < (int)effect_receptor->super_sensor_modes.GetCount(); i++)
      {
         if (effect_receptor->super_sensor_modes[i] == candidate_receptor)
         {
            // Save candidate.
            candidates.Add(candidate_event);
            accum_prob += candidate_event->probability;
            break;
         }
      }
   }

   // Choose effects and create mediators.
   while (true)
   {
      // Make a weighted probabilistic pick of a candidate.
      choose_prob = random.RAND_INTERVAL(0.0, accum_prob);
      for (i = 0, p = 0.0; i < (int)candidates.GetCount(); i++)
      {
         if (candidates[i] == NULL)
         {
            continue;
         }
         p += candidates[i]->probability;
         if (choose_prob <= p)
         {
            break;
         }
      }
      if (i == (int)candidates.GetCount())
      {
         break;
      }
      candidate_event = candidates[i];
      accum_prob     -= candidate_event->probability;
      candidates[i]  = NULL;

      // Make a probabilistic decision to create mediator.
      if (!random.RAND_CHANCE(generalizationEvent->enabling))
      {
         continue;
      }

      // Create the mediator.
      mediator = NewMediator(INITIAL_ENABLEMENT);
      mediator->AddEvent(CAUSE_EVENT, generalizationEvent->mediator->cause);
      if (generalizationEvent->mediator->response != NULL)
      {
         mediator->AddEvent(RESPONSE_EVENT, generalizationEvent->mediator->response);
      }
      mediator->AddEvent(EFFECT_EVENT, candidate_event->neuron);
      mediator->UpdateGoalValue(generalizationEvent->needs);

      // Duplicate?
      if (IsDuplicateMediator(mediator))
      {
         DeleteNeuron(mediator);
         continue;
      }

      // Make new mediator available for learning.
      if ((mediator->level + 1) < (int)learning_events.GetCount())
      {
         mediator->causeBegin     = generalizationEvent->begin;
         mediator->firing_strength = generalizationEvent->enabling *
                                    candidate_event->firing_strength;
         learning_event = new LearningEvent(mediator);
         assert(learning_event != NULL);
         learning_events[mediator->level + 1].Add(learning_event);
      }

#ifdef MONA_TRACE
      if (trace_learn)
      {
         printf("Create generalized mediator:\n");
         mediator->Print();
      }
#endif
   }
}


// Is mediator a duplicate?
bool Mona::IsDuplicateMediator(Mediator *mediator)
{
   int           i, j;
   struct Notify *notify;

   if (mediator->cause->notify_list.GetCount() < mediator->effect->notify_list.GetCount())
   {
      if ((mediator->response == NULL) ||
          (mediator->cause->notify_list.GetCount() < mediator->response->notify_list.GetCount()))
      {
         for (i = 0, j = (int)mediator->cause->notify_list.GetCount(); i < j; i++)
         {
            notify = mediator->cause->notify_list[i];
            if (notify->mediator != mediator)
            {
               if ((notify->mediator->response == mediator->response) &&
                   (notify->mediator->effect == mediator->effect))
               {
                  return true;
               }
            }
         }
         return false;
      }
      else
      {
         for (i = 0, j = (int)mediator->response->notify_list.GetCount(); i < j; i++)
         {
            notify = mediator->response->notify_list[i];
            if (notify->mediator != mediator)
            {
               if ((notify->mediator->cause == mediator->cause) &&
                   (notify->mediator->effect == mediator->effect))
               {
                  return true;
               }
            }
         }
         return false;
      }
   }
   else
   {
      if ((mediator->response == NULL) ||
          (mediator->effect->notify_list.GetCount() < mediator->response->notify_list.GetCount()))
      {
         for (i = 0, j = (int)mediator->effect->notify_list.GetCount(); i < j; i++)
         {
            notify = mediator->effect->notify_list[i];
            if (notify->mediator != mediator)
            {
               if ((notify->mediator->cause == mediator->cause) &&
                   (notify->mediator->response == mediator->response))
               {
                  return true;
               }
            }
         }
         return false;
      }
      else
      {
         for (i = 0, j = (int)mediator->response->notify_list.GetCount(); i < j; i++)
         {
            notify = mediator->response->notify_list[i];
            if (notify->mediator != mediator)
            {
               if ((notify->mediator->cause == mediator->cause) &&
                   (notify->mediator->effect == mediator->effect))
               {
                  return true;
               }
            }
         }
         return false;
      }
   }
}


// Clear working memory.
void Mona::ClearWorkingMemory()
{
   int      i;
   Receptor *receptor;
   Motor    *motor;
   Mediator *mediator;

   list<Mediator *>::Iterator      mediator_iter;
   LearningEvent                   *learning_event;
   list<LearningEvent *>::Iterator learning_eventItr;

   for (i = 0; i < (int)receptors.GetCount(); i++)
   {
      receptor = receptors[i];
      receptor->firing_strength = 0.0;
      receptor->motive         = 0.0;
#ifdef MONA_TRACKING
      receptor->tracker.Clear();
#endif
   }
   for (i = 0; i < (int)motors.GetCount(); i++)
   {
      motor = motors[i];
      motor->firing_strength = 0.0;
      motor->motive         = 0.0;
#ifdef MONA_TRACKING
      motor->tracker.Clear();
#endif
   }
   for (mediator_iter = mediators.Begin();
        mediator_iter != mediators.End(); mediator_iter++)
   {
      mediator = *mediator_iter;
      mediator->firing_strength = 0.0;
      mediator->motive         = 0.0;
      mediator->RetireEnablings(true);
#ifdef MONA_TRACKING
      mediator->tracker.Clear();
#endif
   }
   for (i = 0; i < (int)learning_events.GetCount(); i++)
   {
      for (learning_eventItr = learning_events[i].Begin();
           learning_eventItr != learning_events[i].End(); learning_eventItr++)
      {
         learning_event = *learning_eventItr;
         delete learning_event;
      }
      learning_events[i].Clear();
   }
}


// Clear long term memory.
void Mona::ClearLongTermMemory()
{
   Mediator *mediator;

   list<Mediator *>           tmp_mediators;
   list<Mediator *>::Iterator mediator_iter;

   // Must also clear working memory.
   ClearWorkingMemory();

   // Delete all non-instinct mediators.
   for (mediator_iter = mediators.Begin();
        mediator_iter != mediators.End();
        mediator_iter = mediators.Begin())
   {
      mediator = *mediator_iter;
      if (mediator->instinct || mediator->HasInnerInstinct())
      {
         tmp_mediators.Add(mediator);
         mediators.erase(mediator_iter);
      }
      else
      {
         DeleteNeuron((Mona::Neuron *)(*mediator_iter));
      }
   }
   mediators.Clear();
   for (mediator_iter = tmp_mediators.Begin();
        mediator_iter != tmp_mediators.End(); mediator_iter++)
   {
      mediator = *mediator_iter;
      mediators.Add(mediator);
   }
}
