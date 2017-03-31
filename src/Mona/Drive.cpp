// For conditions of distribution and use, see copyright notice in mona.h

#include "Mona.h"
#include <math.h>

// Drive need changes caused by goal values through network.
void
Mona::Drive()
{
   int      i;
   Neuron   *neuron;
   Receptor *receptor;
   Motor    *motor;
   Mediator *mediator;

   Vector<Mediator *>::Iterator mediator_iter;
   MotiveAccum                motiveAccum;
   VALUE_SET needs;

#ifdef MONA_TRACE
   if (trace_drive)
   {
      printf("***Drive phase***\n");
   }
#endif

   // Copy needs from homeostats to work set.
   needs.Reserve(need_count);
   for (i = 0; i < need_count; i++)
   {
      needs.set(i, homeostats[i]->GetNeed());
   }

#ifdef MONA_TRACE
   if (trace_drive)
   {
      printf("Needs: ");
      needs.Print();
   }
#endif

   // Initialize drive.
   for (i = 0; i < (int)receptors.GetCount(); i++)
   {
      neuron = (Neuron *)receptors[i];
      neuron->InitDrive(needs);
   }
   for (i = 0; i < (int)motors.GetCount(); i++)
   {
      neuron = (Neuron *)motors[i];
      neuron->InitDrive(needs);
   }
   for (mediator_iter = mediators.Begin();
        mediator_iter != mediators.End(); mediator_iter++)
   {
      neuron = (Neuron *)(*mediator_iter);
      neuron->InitDrive(needs);
   }

   // Drive.
   for (i = 0; i < (int)receptors.GetCount(); i++)
   {
      receptor = receptors[i];
      if (receptor->goals.GetValue() != 0.0)
      {
         motiveAccum.Init(needs);
         motiveAccum.AccumulateGoals(receptor->goals);
         if (motiveAccum.GetValue() != 0.0)
         {
            motiveAccum.Init(needs);
            ClearMotiveWork();
#ifdef MONA_TRACKING
            motiveAccum.drivers.Clear();
#endif
            receptor->Drive(motiveAccum);
            SetMotives();
         }
      }
   }

   for (i = 0; i < (int)motors.GetCount(); i++)
   {
      motor = motors[i];
      if (motor->goals.GetValue() != 0.0)
      {
         motiveAccum.Init(needs);
         motiveAccum.AccumulateGoals(motor->goals);
         if (motiveAccum.GetValue() != 0.0)
         {
            motiveAccum.Init(needs);
            motiveAccum.Init(needs);
            ClearMotiveWork();
#ifdef MONA_TRACKING
            motiveAccum.drivers.Clear();
#endif
            motor->Drive(motiveAccum);
            SetMotives();
         }
      }
   }

   for (mediator_iter = mediators.Begin();
        mediator_iter != mediators.End(); mediator_iter++)
   {
      mediator = *mediator_iter;
      NEED goal_value = mediator->goals.GetValue();
      if (goal_value != 0.0)
      {
         if (!mediator->IsGoalValueSubsumed())
         {
            motiveAccum.Init(needs);
            motiveAccum.AccumulateGoals(mediator->goals);
            if (motiveAccum.GetValue() != 0.0)
            {
               motiveAccum.Init(needs);
               ClearMotiveWork();
#ifdef MONA_TRACKING
               motiveAccum.drivers.Clear();
#endif
               mediator->Drive(motiveAccum);
               SetMotives();
            }
         }
      }
   }
   needs.Clear();

   // Finalize motives.
   FinalizeMotives();
}


// Is goal value subsumed by component?
bool
Mona::Mediator::IsGoalValueSubsumed()
{
   int    i;
   NEED   goal_value;
   Neuron *cause      = this->cause;
   Neuron *response   = this->response;
   Neuron *effect     = this->effect;
   bool   *goalBlocks = new bool[mona->need_count];

   ASSERT(goalBlocks != NULL);

   for (i = 0; i < mona->need_count; i++)
   {
      goalBlocks[i] = false;
   }
   while (effect != NULL)
   {
      for (i = 0; i < mona->need_count; i++)
      {
         goal_value = goals.GetValue(i);
         if (goal_value > 0.0)
         {
            if ((cause->goals.GetValue(i) >= goal_value) ||
                (effect->goals.GetValue(i) >= goal_value) ||
                ((response != NULL) && (response->goals.GetValue(i) >= goal_value)))
            {
               goalBlocks[i] = true;
            }
         }
         else
         {
            if ((cause->goals.GetValue(i) <= goal_value) ||
                (effect->goals.GetValue(i) <= goal_value) ||
                ((response != NULL) && (response->goals.GetValue(i) <= goal_value)))
            {
               goalBlocks[i] = true;
            }
         }
      }
      for (i = 0; i < mona->need_count; i++)
      {
         if (!goalBlocks[i])
         {
            break;
         }
      }
      if (i == mona->need_count)
      {
         break;
      }
      if (effect->type == MEDIATOR)
      {
         cause    = ((Mediator *)effect)->cause;
         response = ((Mediator *)effect)->response;
         effect   = ((Mediator *)effect)->effect;
      }
      else
      {
         effect = NULL;
      }
   }
   delete [] goalBlocks;
   if (effect != NULL)
   {
      return true;
   }
   else
   {
      return false;
   }
}


// Initialize neuron drive.
void
Mona::Neuron::InitDrive(VALUE_SET& needs)
{
   int           i;
   ENABLEMENT    up, down, e, ce, re, ee;
   Mediator      *mediator;
   struct Notify *notify;

   motive      = 0.0;
   motive_valid = false;
   motive_work.Init(needs);
   motive_work_valid = false;
#ifdef MONA_TRACKING
   tracker.motive_paths.Clear();
   tracker.motive_work_paths.Clear();
#endif

   /*
    * Assign drive weights to destinations:
    *
    * The input motive is divided into the "down" that
    * is driven to mediator component events and the "up"
    * that is driven to parents. The down value is proportional
    * to the mediator effective enablement. The remaining
    * fraction goes up. For receptors, all motive is driven
    * to parents; for motors, none is.
    *
    * For distributing down, each event is weighted according to
    * its share of enabling.
    *
    * For distributing up, parents for which the component is
    * an effect are weighted. For each of these, its proportion
    * is how much enablement it has available to enable the outer.
    */

   drive_weights.Clear();

   // Divide the down amount.
   switch (type)
   {
   case RECEPTOR:
      up = 1.0;
      break;

   case MOTOR:
      return;

   case MEDIATOR:
      mediator = (Mediator *)this;
      down     = mediator->effective_enablement;
      up       = 1.0 - down;
      e        = mediator->GetEnablement();
      ce       = re = ee = 0.0;

      // Distribute the down amount among components.
      re = (mediator->response_enablings.GetValue() / e) * down;
      if (mediator->response != NULL)
      {
         drive_weights[mediator->response] = re;
      }
      ee = mediator->effect_enablings.GetValue();
      ee = drive_weights[mediator->effect] = (ee / e) * down;
      ce = down - (re + ee);
      drive_weights[mediator->cause] = ce;
      break;
   }

   // Distribute the up amount among parents.
   for (i = 0; i < (int)notify_list.GetCount(); i++)
   {
      notify   = notify_list[i];
      mediator = notify->mediator;
      if (notify->eventType == EFFECT_EVENT)
      {
         drive_weights[mediator] = up *
                                  (1.0 - mediator->effective_enabling_weight);
      }
      else
      {
         drive_weights[mediator] = 0.0;
      }
   }
}


// Clear motive working accumulators.
void
Mona::ClearMotiveWork()
{
   int    i;
   Neuron *neuron;

   Vector<Mediator *>::Iterator mediator_iter;

   for (i = 0; i < (int)receptors.GetCount(); i++)
   {
      neuron = (Neuron *)receptors[i];
      neuron->ClearMotiveWork();
   }
   for (i = 0; i < (int)motors.GetCount(); i++)
   {
      neuron = (Neuron *)motors[i];
      neuron->ClearMotiveWork();
   }
   for (mediator_iter = mediators.Begin();
        mediator_iter != mediators.End(); mediator_iter++)
   {
      neuron = (Neuron *)(*mediator_iter);
      neuron->ClearMotiveWork();
   }
}


// Clear motive working accumulators.
void
Mona::Neuron::ClearMotiveWork()
{
   motive_work.reset();
   motive_work_valid = false;
#ifdef MONA_TRACKING
   tracker.motive_work_paths.Clear();
#endif
}


// Set motives.
void
Mona::SetMotives()
{
   int    i;
   Neuron *neuron;

   Vector<Mediator *>::Iterator mediator_iter;

   for (i = 0; i < (int)receptors.GetCount(); i++)
   {
      neuron = (Neuron *)receptors[i];
      neuron->SetMotive();
   }
   for (i = 0; i < (int)motors.GetCount(); i++)
   {
      neuron = (Neuron *)motors[i];
      neuron->SetMotive();
   }
   for (mediator_iter = mediators.Begin();
        mediator_iter != mediators.End(); mediator_iter++)
   {
      neuron = (Neuron *)(*mediator_iter);
      neuron->SetMotive();
   }
}


// Set neuron motive.
void
Mona::Neuron::SetMotive()
{
   MOTIVE m;

   m = motive_work.GetValue();
   if (!motive_valid || (motive < m))
   {
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
Mona::Neuron::AccumMotiveTracking()
{
   int i, j, k;

   if (type != MOTOR)
   {
      return;
   }
   for (i = 0; i < (int)tracker.motive_work_paths.GetCount(); i++)
   {
      if (tracker.motive_paths.GetCount() < Mona::MAX_DRIVER_TRACKS)
      {
         tracker.motive_paths.Add(tracker.motive_work_paths[i]);
      }
      else
      {
         for (j = 0, k = -1; j < (int)tracker.motive_work_paths.GetCount(); j++)
         {
            if (tracker.motive_work_paths[i].motive > tracker.motive_paths[j].motive)
            {
               if ((k == -1) || (tracker.motive_paths[j].motive < tracker.motive_paths[k].motive))
               {
                  k = j;
               }
            }
         }
         if (k != -1)
         {
            tracker.motive_paths[k] = tracker.motive_work_paths[i];
         }
      }
   }
}


#endif

// Finalize motives.
void
Mona::FinalizeMotives()
{
   int    i;
   Neuron *neuron;

   Vector<Mediator *>::Iterator mediator_iter;

   for (i = 0; i < (int)receptors.GetCount(); i++)
   {
      neuron = (Neuron *)receptors[i];
      neuron->FinalizeMotive();
   }
   for (i = 0; i < (int)motors.GetCount(); i++)
   {
      neuron = (Neuron *)motors[i];
      neuron->FinalizeMotive();
   }
   for (mediator_iter = mediators.Begin();
        mediator_iter != mediators.End(); mediator_iter++)
   {
      neuron = (Neuron *)(*mediator_iter);
      neuron->FinalizeMotive();
   }
}


// Finalize neuron motive.
void
Mona::Neuron::FinalizeMotive()
{
   motive = motive / mona->max_motive;
   if (motive > 1.0)
   {
      motive = 1.0;
   }
   if (motive < -1.0)
   {
      motive = -1.0;
   }
}


// Neuron drive.
void
Mona::Neuron::Drive(MotiveAccum motiveAccum)
{
   int         i;
   Mediator    *mediator;
   Receptor    *receptor;
   MOTIVE      m;
   WEIGHT      w;
   MotiveAccum accumWork;

   // Prevent looping.
   if (!motiveAccum.AddPath(this))
   {
      return;
   }

   // Accumulate need change due to goal value.
   if ((type != MEDIATOR) || !((Mediator *)this)->IsGoalValueSubsumed())
   {
      motiveAccum.AccumulateGoals(goals);
   }

   // Accumulate motive.
   // Store greater motive except for attenuated "pain".
   m = motiveAccum.GetValue();
   if (!motive_work_valid ||
       ((m >= NEARLY_ZERO) && ((m - motive_work.GetValue()) > NEARLY_ZERO)))
   {
      motive_work_valid = true;
      motive_work.LoadNeeds(motiveAccum);
   }
#ifndef MONA_TRACKING
   else
   {
      return;
   }
#else
   // Track motive.
   if (!TrackMotive(motiveAccum, accumWork))
   {
      return;
   }
#endif

#ifdef MONA_TRACE
   if (mona->trace_drive)
   {
      printf("Drive %llu, motive=%f\n", id, m);
   }
#endif

   switch (type)
   {
   // Distribute motive to component events.
   case MEDIATOR:
      mediator = (Mediator *)this;

      // Drive motive to cause event.
      if ((w = mediator->drive_weights[mediator->cause]) > NEARLY_ZERO)
      {
         accumWork.Configure(motiveAccum, w * (1.0 - mona->DRIVE_ATTENUATION));
         mediator->cause->Drive(accumWork);
      }

      // Drive motive to response event.
      if (mediator->response != NULL)
      {
         if ((w = mediator->drive_weights[mediator->response]) > NEARLY_ZERO)
         {
            accumWork.Configure(motiveAccum, w * (1.0 - mona->DRIVE_ATTENUATION));
            mediator->response->Drive(accumWork);
         }
      }

      // Drive motive to effect event.
      if ((w = mediator->drive_weights[mediator->effect]) > NEARLY_ZERO)
      {
         accumWork.Configure(motiveAccum, w * (1.0 - mona->DRIVE_ATTENUATION));
         mediator->effect->Drive(accumWork);
      }
      break;

   // Receptor drives motive to subset receptors.
   case RECEPTOR:
      receptor = (Receptor *)this;
      for (i = 0; i < (int)receptor->sub_sensor_modes.GetCount(); i++)
      {
         accumWork.Configure(motiveAccum, 1.0);
         receptor->sub_sensor_modes[i]->Drive(accumWork);
      }
      break;

   // Drive terminates on motor neurons.
   case MOTOR:
      return;
   }

   // Drive motive to parent mediators.
   for (i = 0; i < (int)notify_list.GetCount(); i++)
   {
      mediator = notify_list[i]->mediator;
      if ((w = drive_weights[mediator]) > NEARLY_ZERO)
      {
         w *= (1.0 - mona->DRIVE_ATTENUATION);
         accumWork.Configure(motiveAccum, w);
         mediator->DriveCause(accumWork);
      }
   }
}


// Drive mediator cause.
void
Mona::Mediator::DriveCause(MotiveAccum motiveAccum)
{
   int         i;
   Mediator    *mediator;
   MOTIVE      m;
   WEIGHT      w;
   MotiveAccum accumWork;

   // Accumulate motive.
   // Store greater motive except for attenuated "pain".
   m = motiveAccum.GetValue();
   if (!motive_work_valid ||
       ((m >= NEARLY_ZERO) && ((m - motive_work.GetValue()) > NEARLY_ZERO)))
   {
      motive_work_valid = true;
      motive_work.LoadNeeds(motiveAccum);
   }
#ifndef MONA_TRACKING
   else
   {
      return;
   }
#else
   // Track motive.
   if (!TrackMotive(motiveAccum, accumWork))
   {
      return;
   }
#endif

#ifdef MONA_TRACE
   if (mona->trace_drive)
   {
      printf("Drive cause %llu, motive=%f\n", id, m);
   }
#endif

   // Drive motive to cause event.
   if ((w = drive_weights[cause]) > NEARLY_ZERO)
   {
      accumWork.Configure(motiveAccum, w * (1.0 - mona->DRIVE_ATTENUATION));
      cause->Drive(accumWork);
   }

   // Drive motive to response event.
   if (response != NULL)
   {
      if ((w = drive_weights[response]) > NEARLY_ZERO)
      {
         accumWork.Configure(motiveAccum, w * (1.0 - mona->DRIVE_ATTENUATION));
         response->Drive(accumWork);
      }
   }

   // Drive motive to parent mediators.
   for (i = 0; i < (int)notify_list.GetCount(); i++)
   {
      mediator = notify_list[i]->mediator;
      if ((w = drive_weights[mediator]) > NEARLY_ZERO)
      {
         accumWork.Configure(motiveAccum, w * (1.0 - mona->DRIVE_ATTENUATION));
         mediator->DriveCause(accumWork);
      }
   }
}


#ifdef MONA_TRACKING
// Track motive.
// Returns true if tracking continues.
bool
Mona::Neuron::TrackMotive(MotiveAccum& in, MotiveAccum& out)
{
   int i, j;
   struct MotiveAccum::DriveElem e;
   struct Activation::DrivePath  d;
   VALUE_SET needs;

   for (i = 0; i < (int)in.drivers.GetCount(); i++)
   {
      if (in.drivers[i].neuron == this)
      {
         return false;
      }
      out.drivers.Add(in.drivers[i]);
      d.drivers.Add(in.drivers[i]);
   }
   needs.Reserve(mona->need_count);
   for (i = 0; i < mona->need_count; i++)
   {
      needs.set(i, mona->homeostats[i]->GetNeed());
   }
   d.motive_work.Init(needs);
   needs.Clear();
   d.motive_work.LoadNeeds(in);
   e.neuron  = this;
   e.motive  = d.motive_work.GetValue();
   e.motive /= mona->max_motive;
   if (e.motive > 1.0)
   {
      e.motive = 1.0;
   }
   if (e.motive < -1.0)
   {
      e.motive = -1.0;
   }
   d.motive = e.motive;
   out.drivers.Add(e);
   d.drivers.Add(e);
   if (tracker.motive_work_paths.GetCount() < Mona::MAX_DRIVER_TRACKS)
   {
      tracker.motive_work_paths.Add(d);
   }
   else
   {
      for (i = 0, j = -1; i < (int)tracker.motive_work_paths.GetCount(); i++)
      {
         if (d.motive_work.GetValue() > tracker.motive_work_paths[i].motive_work.GetValue())
         {
            if ((j == -1) ||
                (tracker.motive_work_paths[i].motive_work.GetValue() <
                 tracker.motive_work_paths[j].motive_work.GetValue()))
            {
               j = i;
            }
         }
      }
      if (j != -1)
      {
         tracker.motive_work_paths[j] = d;
      }
      else
      {
         d.motive_work.Clear();
         d.drivers.Clear();
         return false;
      }
   }
   return true;
}


#endif
