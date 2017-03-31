// For conditions of distribution and use, see copyright notice in mona.h

// Mona auxiliary classes.

#ifndef __MONA_AUX__
#define __MONA_AUX__

// Forward declarations.
class Neuron;
class Receptor;
class Motor;
class Mediator;
class EnablingSet;

// Sensor mode.
class SensorMode
{
public:
   SENSOR_MODE         mode;
   Vector<bool>        mask;
   SENSOR              resolution;
   Vector<SENSOR_MODE> subsets;
   Vector<SENSOR_MODE> supersets;

   // Constructor.
   SensorMode()
   {
      mode       = 0;
      resolution = 0.0f;
   }


   // Initialize.
   // Return mode.
   int Init(Vector<bool>& mask, SENSOR resolution,
            Vector<SensorMode *> *sensor_modes)
   {
      int        i, j;
      bool       sub, super;
      SensorMode *s;

      // Initialize.
      this->mask.Clear();
      for (i = 0; i < (int)mask.GetCount(); i++)
      {
         this->mask.Add(mask[i]);
      }
      mode             = (int)sensor_modes->GetCount();
      this->resolution = resolution;

      // Determine subset/supersets.
      for (i = 0, j = (int)sensor_modes->GetCount(); i < j; i++)
      {
         s = (*sensor_modes)[i];
         subSuper(s, sub, super);
         if (sub && super) { continue; }
         if (sub)
         {
            if (!subTransitive(i, sensor_modes))
            {
               subsets.Add(i);
               s->supersets.Add(j);
            }
         }
         else if (super)
         {
            if (!superTransitive(i, sensor_modes))
            {
               supersets.Add(i);
               s->subsets.Add(j);
            }
         }
      }
      sensor_modes->Add(this);
      return (mode);
   }


   // Given mode is subset/superset?
   void subSuper(SensorMode *sensor_mode, bool& sub, bool& super)
   {
      sub = super = true;
      for (int i = 0; i < (int)mask.GetCount() && (sub || super); i++)
      {
         if (mask[i])
         {
            if (!sensor_mode->mask[i])
            {
               sub = false;
            }
         }
         else
         {
            if (sensor_mode->mask[i])
            {
               super = false;
            }
         }
      }
   }


   // Given mode is a transitive subset?
   // if A->B and B->C, then C is transitive.
   bool subTransitive(int idx, Vector<SensorMode *> *sensor_modes)
   {
      bool       sub, super;
      SensorMode *s;

      for (int i = 0; i < (int)subsets.GetCount(); i++)
      {
         s = (*sensor_modes)[i];
         s->subSuper((*sensor_modes)[idx], sub, super);
         if (sub || s->subTransitive(idx, sensor_modes))
         {
            return true;
         }
      }
      return (false);
   }


   // Given mode is a transitive superset?
   // if A->B and B->C, then C is transitive.
   bool superTransitive(int idx, Vector<SensorMode *> *sensor_modes)
   {
      bool       sub, super;
      SensorMode *s;

      for (int i = 0; i < (int)supersets.GetCount(); i++)
      {
         s = (*sensor_modes)[i];
         s->subSuper((*sensor_modes)[idx], sub, super);
         if (super || s->superTransitive(idx, sensor_modes))
         {
            return true;
         }
      }
      return (false);
   }


   // Load.
   void Load(FILE *fp)
   {
      int  i, j, size;
      bool v;

      FREAD_INT(&mode, fp);
      mask.Clear();
      subsets.Clear();
      supersets.Clear();
      FREAD_INT(&size, fp);
      for (i = 0; i < size; i++)
      {
         FREAD_BOOL(&v, fp);
         mask.Add(v);
      }
      FREAD_FLOAT(&resolution, fp);
      FREAD_INT(&size, fp);
      for (i = 0; i < size; i++)
      {
         FREAD_INT(&j, fp);
         subsets.Add(j);
      }
      FREAD_INT(&size, fp);
      for (i = 0; i < size; i++)
      {
         FREAD_INT(&j, fp);
         supersets.Add(j);
      }
   }


   // Save.
   // When changing format increment FORMAT in mona.h
   void Store(FILE *fp)
   {
      int  i, j, size;
      bool v;

      FWRITE_INT(&mode, fp);
      size = (int)mask.GetCount();
      FWRITE_INT(&size, fp);
      for (i = 0; i < size; i++)
      {
         v = mask[i];
         FWRITE_BOOL(&v, fp);
      }
      FWRITE_FLOAT(&resolution, fp);
      size = (int)subsets.GetCount();
      FWRITE_INT(&size, fp);
      for (i = 0; i < size; i++)
      {
         j = subsets[i];
         FWRITE_INT(&j, fp);
      }
      size = (int)supersets.GetCount();
      FWRITE_INT(&size, fp);
      for (i = 0; i < size; i++)
      {
         j = supersets[i];
         FWRITE_INT(&j, fp);
      }
   }


   // Print.
   void Print(FILE *out = stdout)
   {
      int i, j, size;

      fprintf(out, "<mode>%d</mode>", mode);
      fprintf(out, "<mask>");
      size = (int)mask.GetCount();
      for (i = 0; i < size; i++)
      {
         j = (int)mask[i];
         fprintf(out, "%d", j);
      }
      fprintf(out, "</mask>");
      fprintf(out, "<resolution>%f</resolution>", resolution);
      fprintf(out, "<subsets>");
      size = (int)subsets.GetCount();
      for (i = 0; i < size; i++)
      {
         j = subsets[i];
         fprintf(out, "%d", j);
      }
      fprintf(out, "</subsets>");
      fprintf(out, "<supersets>");
      size = (int)supersets.GetCount();
      for (i = 0; i < size; i++)
      {
         j = supersets[i];
         fprintf(out, "%d", j);
      }
      fprintf(out, "</supersets>");
   }
};

// Goal value.
class GoalValue
{
public:
   VALUE_SET values;
   COUNTER   update_count;
   Mona      *mona;

   // Constructors.
   GoalValue(int goal_count, Mona *mona)
   {
      values.Reserve(goal_count);
      update_count = 0;
      this->mona  = mona;
   }


   GoalValue()
   {
      update_count = 0;
      mona        = NULL;
   }


   // Initialize.
   void Init(int goal_count, Mona *mona)
   {
      values.Reserve(goal_count);
      this->mona = mona;
   }


   // Destructor.
   ~GoalValue()
   {
      Clear();
   }


   // Get total goal value.
   inline NEED GetValue()
   {
      return (values.GetSum());
   }


   // Get specific goal value.
   inline NEED GetValue(int index)
   {
      return (values.Get(index));
   }


   // Set specific goal value.
   inline void SetValue(int index, NEED value)
   {
      values.set(index, value);
   }


   // Set initial goals.
   inline void setGoals(VALUE_SET& goals)
   {
      values.Load(goals);
   }


   // Get number of goals.
   inline int GetGoalCount()
   {
      return (values.GetCount());
   }


   // Update.
   void update(VALUE_SET& needs, VALUE_SET& need_deltas)
   {
      int    i, j;
      double v;

      update_count++;
      for (i = 0, j = needs.GetCount(); i < j; i++)
      {
         if (need_deltas.Get(i) > 0.0)
         {
            v = values.Get(i);
            values.set(i, v - ((v / (double)update_count) *
                               mona->LEARNING_DECREASE_VELOCITY));
            continue;
         }
         if (need_deltas.Get(i) == 0.0)
         {
            if (needs.Get(i) > 0.0)
            {
               v = values.Get(i);
               values.set(i, v - ((v / (double)update_count) *
                                  mona->LEARNING_DECREASE_VELOCITY));
            }
            continue;
         }
         v = values.Get(i) + need_deltas.Get(i);
         if (needs.Get(i) == 0.0)
         {
            if (v >= 0.0)
            {
               continue;
            }
         }
         values.set(i, values.Get(i) + ((-v / (double)update_count) *
                                        mona->LEARNING_INCREASE_VELOCITY));
      }
   }


   // Clear.
   inline void Clear()
   {
      values.Clear();
      update_count = 0;
   }


   // Load.
   void Load(FILE *fp)
   {
      Clear();
      values.Load(fp);
      FREAD_LONG_LONG(&update_count, fp);
   }


   // Save.
   void Store(FILE *fp)
   {
      values.Store(fp);
      FWRITE_LONG_LONG(&update_count, fp);
   }


   // Print.
   void Print(FILE *out = stdout)
   {
      for (int i = 0; i < values.GetCount(); i++)
      {
         fprintf(out, "<value>%f</value>", values.Get(i));
      }
      fprintf(out, "<update_count>%llu</update_count>", update_count);
   }
};

// Motive accumulator.
// Accumulates need change caused by goals.
class MotiveAccum
{
public:
   VALUE_SET        base;
   VALUE_SET        delta;
   WEIGHT           weight;
   Vector<Neuron *> path;
#ifdef MONA_TRACKING
   struct DriveElem
   {
      Neuron *neuron;
      MOTIVE motive;
   };
   Vector<struct DriveElem> drivers;
#endif

   // Constructor.
   MotiveAccum()
   {
      Clear();
   }


   // Destructor.
   ~MotiveAccum()
   {
      Clear();
   }


   // Get value of accumulator.
   // This is the sum of the delta values
   // bounded by the base need values.
   inline MOTIVE GetValue()
   {
      double b, d, v;

      v = 0.0;
      for (int i = 0; i < delta.GetCount(); i++)
      {
         b = base.Get(i);
         d = delta.Get(i);
         if (d < 0.0)
         {
            if ((b + d) < 0.0) { v += b; } else{ v -= d; }
         }
         else
         {
            if ((b + d) > 1.0) { v -= (1.0 - b); } else{ v -= d; }
         }
      }
      return (v);
   }


   // Initialize accumulator.
   inline void Init(VALUE_SET& needs)
   {
      base.Load(needs);
      delta.Reserve(base.GetCount());
      delta.Zero();
      weight = 1.0;
      path.Clear();
   }


   // Configure accumulator.
   inline void Configure(MotiveAccum& accum, WEIGHT weight)
   {
      Init(accum.base);
      LoadNeeds(accum);
      Scale(weight);
      this->weight = accum.weight * weight;
      for (int i = 0; i < (int)accum.path.GetCount(); i++)
      {
         path.Add(accum.path[i]);
      }
   }


   // Load need delta values.
   void LoadNeeds(MotiveAccum& motiveAccum)
   {
      delta.Load(motiveAccum.delta);
   }


   // Accumulate need delta values.
   void AccumulateNeeds(MotiveAccum& motiveAccum)
   {
      delta.Add(motiveAccum.delta);
   }


   // Accumulate goal values.
   inline void AccumulateGoals(GoalValue& goals)
   {
      VALUE_SET v;

      v.Load(goals.values);
      v.Multiply(weight);
      delta.Subtract(v);
   }


   // Add a neuron to the path.
   // Return true if no loop detected.
   inline bool AddPath(Neuron *neuron)
   {
      for (int i = 0; i < (int)path.GetCount(); i++)
      {
         if (path[i] == neuron) { return (false); }
      }
      path.Add(neuron);
      return true;
   }


   // Scale accumulator.
   inline void Scale(WEIGHT weight)
   {
      delta.Multiply(weight);
   }


   // Reset accumulator.
   inline void reset()
   {
      delta.Zero();
      weight = 1.0;
   }


   // Clear accumulator.
   inline void Clear()
   {
      base.Clear();
      delta.Clear();
      weight = 1.0;
      path.Clear();
   }


   // Load.
   void Load(FILE *fp)
   {
      Clear();
      base.Load(fp);
      delta.Load(fp);
      FREAD_DOUBLE(&weight, fp);
   }


   // Save.
   // When changing format increment FORMAT in mona.h
   void Store(FILE *fp)
   {
      base.Store(fp);
      delta.Store(fp);
      FWRITE_DOUBLE(&weight, fp);
   }


   // Print.
   void Print(FILE *out = stdout)
   {
      int i;

      fprintf(out, "<base>");
      for (i = 0; i < base.GetCount(); i++)
      {
         fprintf(out, "<value>%f</value>", base.Get(i));
      }
      fprintf(out, "</base><delta>");
      for (i = 0; i < delta.GetCount(); i++)
      {
         fprintf(out, "<value>%f</value>", delta.Get(i));
      }
      fprintf(out, "</delta><weight>%f</weight>", weight);
      fprintf(out, "<value>%f</value>", GetValue());
   }
};


// Event enabling.
class Enabling {
public:
   ENABLEMENT  value;
   MOTIVE      motive;
   TIME        age;
   int         timer_index;
   EnablingSet *set;
   bool        new_in_set;
   TIME        cause_begin;
   VALUE_SET   needs;

   // Constructor.
   Enabling(ENABLEMENT value, MOTIVE motive, TIME age,
            int timer_index, TIME cause_begin)
   {
      Clear();
      this->value      = value;
      this->motive     = motive;
      this->age        = age;
      this->timer_index = timer_index;
      this->cause_begin = cause_begin;
   }


   Enabling()
   {
      Clear();
   }


   // Destructor.
   ~Enabling()
   {
      Clear();
   }


   // Set needs.
   void SetNeeds(Vector<Homeostat *>& homeostats)
   {
      int n = (int)homeostats.GetCount();

      needs.Reserve(n);
      for (int i = 0; i < n; i++)
      {
         needs.set(i, homeostats[i]->GetNeed());
      }
   }


   // Clone.
   inline Enabling *Clone()
   {
      Enabling *enabling;

      enabling = new Enabling(value, motive, age,
                              timer_index, cause_begin);
      ASSERT(enabling != NULL);
      enabling->new_in_set = new_in_set;
      enabling->needs.Load(needs);
      return (enabling);
   }


   // Clear enabling.
   inline void Clear()
   {
      value      = 0.0;
      motive     = 0.0;
      age        = 0;
      timer_index = -1;
      set        = NULL;
      new_in_set   = false;
      cause_begin = 0;
      needs.Clear();
   }


   // Load.
   void Load(FILE *fp)
   {
      Clear();
      FREAD_DOUBLE(&value, fp);
      FREAD_DOUBLE(&motive, fp);
      FREAD_LONG_LONG(&age, fp);
      FREAD_INT(&timer_index, fp);
      set = NULL;
      FREAD_BOOL(&new_in_set, fp);
      FREAD_LONG_LONG(&cause_begin, fp);
      needs.Load(fp);
   }


   // Save.
   // When changing format increment FORMAT in mona.h
   void Store(FILE *fp)
   {
      FWRITE_DOUBLE(&value, fp);
      FWRITE_DOUBLE(&motive, fp);
      FWRITE_LONG_LONG(&age, fp);
      FWRITE_INT(&timer_index, fp);
      FWRITE_BOOL(&new_in_set, fp);
      FWRITE_LONG_LONG(&cause_begin, fp);
      needs.Store(fp);
   }


   // Print.
   void Print(FILE *out = stdout)
   {
      fprintf(out, "<value>%f</value>", value);
      fprintf(out, "<motive>%f</motive>", value);
      fprintf(out, "<age>%llu</age>", age);
      fprintf(out, "<timer_index>%d</timer_index>", timer_index);
      fprintf(out, "<new_in_set>");
      if (new_in_set) { fprintf(out, "true"); } else{ fprintf(out, "false"); }
      fprintf(out, "</new_in_set><cause_begin>%llu</cause_begin>", cause_begin);
      fprintf(out, "<needs>");
      int n = needs.GetCount();
      for (int i = 0; i < n; i++)
      {
         fprintf(out, "<need>%f</need>", needs.Get(i));
      }
      fprintf(out, "/<needs>");
   }
};

// Set of event enablings.
class EnablingSet
{
public:
   Vector<Enabling *> enablings;

   // Constructor.
   EnablingSet()
   {
      Clear();
   }


   // Destructor.
   ~EnablingSet()
   {
      Clear();
   }


   // Get size of set.
   inline int GetCount()
   {
      return ((int)enablings.GetCount());
   }


   // Insert an enabling.
   inline void Insert(Enabling *enabling)
   {
      enabling->set      = this;
      enabling->new_in_set = true;
      enablings.Add(enabling);
   }


   // Remove an enabling.
   inline void Remove(Enabling *enabling)
   {
      enablings.Remove(enabling);
   }


   // Get enabling value.
   inline ENABLEMENT GetValue()
   {
      Enabling   *enabling;
      ENABLEMENT e;

      Vector<Enabling *>::Iterator listItr;

      e = 0.0;
      for (listItr = enablings.Begin();
           listItr != enablings.End(); listItr++)
      {
         enabling = *listItr;
         e       += enabling->value;
      }
      return (e);
   }


   // Get value of new enablings.
   inline ENABLEMENT getNewValue()
   {
      Enabling   *enabling;
      ENABLEMENT e;

      Vector<Enabling *>::Iterator listItr;

      e = 0.0;
      for (listItr = enablings.Begin();
           listItr != enablings.End(); listItr++)
      {
         enabling = *listItr;
         if (enabling->new_in_set)
         {
            e += enabling->value;
         }
      }
      return (e);
   }


   // Get value of old enablings.
   inline ENABLEMENT getOldValue()
   {
      Enabling   *enabling;
      ENABLEMENT e;

      Vector<Enabling *>::Iterator listItr;

      e = 0.0;
      for (listItr = enablings.Begin();
           listItr != enablings.End(); listItr++)
      {
         enabling = *listItr;
         if (!enabling->new_in_set)
         {
            e += enabling->value;
         }
      }
      return (e);
   }


   // Clear new flags.
   inline void clearNewInSet()
   {
      Enabling *enabling;

      Vector<Enabling *>::Iterator listItr;

      for (listItr = enablings.Begin();
           listItr != enablings.End(); listItr++)
      {
         enabling           = *listItr;
         enabling->new_in_set = false;
      }
   }


   // Clear.
   inline void Clear()
   {
      Enabling *enabling;

      Vector<Enabling *>::Iterator listItr;

      for (listItr = enablings.Begin();
           listItr != enablings.End(); listItr++)
      {
         enabling = *listItr;
         delete enabling;
      }
      enablings.Clear();
   }


   // Load.
   void Load(FILE *fp)
   {
      int      size;
      Enabling *enabling;

      Clear();
      FREAD_INT(&size, fp);
      for (int i = 0; i < size; i++)
      {
         enabling = new Enabling();
         ASSERT(enabling != NULL);
         enabling->Load(fp);
         enabling->set = this;
         enablings.Add(enabling);
      }
   }


   // Save.
   // When changing format increment FORMAT in mona.h
   void Store(FILE *fp)
   {
      int      size;
      Enabling *enabling;

      Vector<Enabling *>::Iterator listItr;

      size = (int)enablings.GetCount();
      FWRITE_INT(&size, fp);
      for (listItr = enablings.Begin();
           listItr != enablings.End(); listItr++)
      {
         enabling = *listItr;
         enabling->Store(fp);
      }
   }


   // Print.
   void Print(FILE *out = stdout)
   {
      Enabling *enabling;

      Vector<Enabling *>::Iterator listItr;

      fprintf(out, "<enablingSet>");
      for (listItr = enablings.Begin();
           listItr != enablings.End(); listItr++)
      {
         enabling = *listItr;
         enabling->Print(out);
      }
      fprintf(out, "</enablingSet>");
   }
};

// Mediator event notifier.
struct Notify
{
   Mediator   *mediator;
   EVENT_TYPE eventType;
};

// Mediator firing notification.
struct FiringNotify
{
   struct Notify *notify;
   WEIGHT        notify_strength;
   TIME          cause_begin;
};

// Learning event.
class LearningEvent
{
public:
   Neuron      *neuron;
   WEIGHT      firing_strength;
   TIME        begin;
   TIME        end;
   PROBABILITY probability;
   VALUE_SET   needs;

   LearningEvent(Neuron *neuron)
   {
      this->neuron   = neuron;
      firing_strength = neuron->firing_strength;
      if (neuron->type == MEDIATOR)
      {
         begin = ((Mediator *)neuron)->cause_begin;
      }
      else
      {
         begin = neuron->mona->event_clock;
      }
      end = neuron->mona->event_clock;
      if (firing_strength > NEARLY_ZERO)
      {
         probability =
            pow(firing_strength, neuron->mona->FIRING_STRENGTH_LEARNING_DAMPER);
      }
      else
      {
         probability = 0.0;
      }
      int n = neuron->mona->need_count;
      needs.Reserve(n);
      for (int i = 0; i < n; i++)
      {
         needs.set(i, neuron->mona->homeostats[i]->GetNeed());
      }
   }


   LearningEvent()
   {
      neuron         = NULL;
      firing_strength = 0.0;
      begin          = end = 0;
      probability    = 0.0;
      needs.Clear();
   }


   // Load.
   void Load(FILE *fp)
   {
      neuron = (Neuron *)new ID;
      ASSERT(neuron != NULL);
      FREAD_LONG_LONG((ID *)neuron, fp);
      FREAD_DOUBLE(&firing_strength, fp);
      FREAD_LONG_LONG(&begin, fp);
      FREAD_LONG_LONG(&end, fp);
      FREAD_DOUBLE(&probability, fp);
      needs.Load(fp);
   }


   // Save.
   // When changing format increment FORMAT in mona.h
   void Store(FILE *fp)
   {
      FWRITE_LONG_LONG(&neuron->id, fp);
      FWRITE_DOUBLE(&firing_strength, fp);
      FWRITE_LONG_LONG(&begin, fp);
      FWRITE_LONG_LONG(&end, fp);
      FWRITE_DOUBLE(&probability, fp);
      needs.Store(fp);
   }


   // Print.
   void Print(FILE *out = stdout)
   {
      if (neuron != NULL)
      {
         fprintf(out, "<neuron>\n");
         switch (neuron->type)
         {
#ifdef MONA_TRACKING
         case RECEPTOR:
            ((Receptor *)neuron)->Print((TRACKING_FLAGS)0, out);
            fprintf(out, "\n");
            break;

         case MOTOR:
            ((Motor *)neuron)->Print((TRACKING_FLAGS)0, out);
            fprintf(false, false, out, "\n");
            break;

         case MEDIATOR:
            ((Mediator *)neuron)->Print((TRACKING_FLAGS)0, out);
            break;

#else
         case RECEPTOR:
            ((Receptor *)neuron)->Print(out);
            fprintf(out, "\n");
            break;

         case MOTOR:
            ((Motor *)neuron)->Print(out);
            fprintf(out, "\n");
            break;

         case MEDIATOR:
            ((Mediator *)neuron)->Print(out);
            break;
#endif
         }
      }
      else
      {
         fprintf(out, "<neuron>NULL");
      }
      fprintf(out, "</neuron>\n");
      fprintf(out, "<firing_strength>%f</firing_strength>", firing_strength);
      fprintf(out, "<begin>%llu</begin>", begin);
      fprintf(out, "<end>%llu</end>", end);
      fprintf(out, "<probability>%f</probability>", probability);
      fprintf(out, "<needs>");
      int n = needs.GetCount();
      for (int i = 0; i < n; i++)
      {
         fprintf(out, "<need>%f</need>", needs.Get(i));
      }
      fprintf(out, "/<needs>");
   }
};

// Generalization learning event.
class GeneralizationEvent
{
public:
   Mediator   *mediator;
   ENABLEMENT enabling;
   TIME       begin;
   TIME       end;
   VALUE_SET  needs;

   GeneralizationEvent(Mediator *mediator, ENABLEMENT enabling)
   {
      this->mediator = mediator;
      this->enabling = enabling;
      begin          = mediator->cause_begin;
      end            = mediator->mona->event_clock;
      int n = mediator->mona->need_count;
      needs.Reserve(n);
      for (int i = 0; i < n; i++)
      {
         needs.set(i, mediator->mona->homeostats[i]->GetNeed());
      }
   }


   GeneralizationEvent()
   {
      mediator = NULL;
      enabling = 0.0;
      begin    = end = 0;
      needs.Clear();
   }


   // Load.
   void Load(FILE *fp)
   {
      mediator = (Mediator *)new ID;
      ASSERT(mediator != NULL);
      FREAD_LONG_LONG((ID *)mediator, fp);
      FREAD_DOUBLE(&enabling, fp);
      FREAD_LONG_LONG(&begin, fp);
      FREAD_LONG_LONG(&end, fp);
      needs.Load(fp);
   }


   // Save.
   // When changing format increment FORMAT in mona.h
   void Store(FILE *fp)
   {
      FWRITE_LONG_LONG(&mediator->id, fp);
      FWRITE_DOUBLE(&enabling, fp);
      FWRITE_LONG_LONG(&begin, fp);
      FWRITE_LONG_LONG(&end, fp);
      needs.Store(fp);
   }


   // Print.
   void Print(FILE *out = stdout)
   {
      if (mediator != NULL)
      {
         fprintf(out, "<mediator>\n");
#ifdef MONA_TRACKING
         mediator->Print((TRACKING_FLAGS)0, out);
#else
         mediator->Print(out);
#endif
      }
      else
      {
         fprintf(out, "<mediator>NULL");
      }
      fprintf(out, "</mediator>\n");
      fprintf(out, "<enabling>%f</enabling>", enabling);
      fprintf(out, "<begin>%llu</begin>", begin);
      fprintf(out, "<end>%llu</end>", end);
      int n = needs.GetCount();
      for (int i = 0; i < n; i++)
      {
         fprintf(out, "<need>%f</need>", needs.Get(i));
      }
      fprintf(out, "/<needs>");
   }
};
#endif
