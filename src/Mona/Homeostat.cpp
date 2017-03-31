// For conditions of distribution and use, see copyright notice in mona.h

#include "Mona.h"

const Homeostat::ID Homeostat::NULL_ID = (ID)(-1);

// Homeostat constructors.
Homeostat::Homeostat()
{
   need         = 0.0;
   need_index    = 0;
   need_delta    = 0.0;
   mona         = NULL;
   periodic_need = 0.0;
   frequency    = 0;
   freq_timer    = 0;
}


Homeostat::Homeostat(int need_index, Mona *mona)
{
   need            = 0.0;
   this->need_index = need_index;
   need_delta       = 0.0;
   this->mona      = mona;
   periodic_need    = 0.0;
   frequency       = 0;
   freq_timer       = 0;
}


// Destructor.
Homeostat::~Homeostat()
{
   goals.Clear();
}


// Set periodic need.
void Homeostat::SetPeriodicNeed(int frequency, NEED need)
{
   this->frequency = frequency;
   periodic_need    = need;
   freq_timer       = 0;
}


// Clear periodic need.
void Homeostat::ClearPeriodicNeed()
{
   periodic_need = 0.0;
   frequency    = 0;
   freq_timer    = 0;
}


// Add sensory-response goal.
// Replace duplicate response and goal value.
int Homeostat::AddGoal(Vector<SENSOR>& sensors, SENSOR_MODE sensor_mode,
                       RESPONSE response, NEED goal_value)
{
   int goal_index, i;

   Vector<SENSOR> sensors_work;
   Mona::Receptor *receptor;
   SENSOR         distance;
   Goal           goal;

   // Check for duplicate.
   if ((goal_index = FindGoal(sensors, sensor_mode, false)) != -1)
   {
      ((Mona::Receptor *)goals[goal_index].receptor)->goals.SetValue(need_index, goal_value);
      if (goals[goal_index].response != NULL_RESPONSE)
      {
         // Clear old motor goal value.
         ((Mona::Motor *)goals[goal_index].motor)->goals.SetValue(need_index, 0.0);
      }
      goals[goal_index].response = response;
      if (response != NULL_RESPONSE)
      {
         goals[goal_index].motor = (void *)mona->FindMotorByResponse(response);
         ASSERT(goals[goal_index].motor != NULL);
         ((Mona::Motor *)goals[goal_index].motor)->goals.SetValue(need_index, 0.0);
      }
      else
      {
         goals[goal_index].motor = NULL;
      }
      goals[goal_index].goal_value = goal_value;
   }
   else
   {
      // Add entry.
      mona->ApplySensorMode(sensors, sensors_work, sensor_mode);
      for (i = 0; i < (int)sensors_work.GetCount(); i++)
      {
         goal.sensors.Add(sensors_work[i]);
      }
      goal.sensor_mode = sensor_mode;
      receptor        = mona->GetCentroidReceptor(sensors_work, sensor_mode, distance);
      if ((receptor == NULL) ||
          ((receptor != NULL) && (distance > mona->sensor_modes[sensor_mode]->resolution)))
      {
         receptor = mona->NewReceptor(sensors_work, sensor_mode);
      }
      receptor->goals.SetValue(need_index, goal_value);
      goal.receptor = (void *)receptor;
      goal.response = response;
      if (response != NULL_RESPONSE)
      {
         goal.motor = (void *)mona->FindMotorByResponse(response);
         ASSERT(goal.motor != NULL);
         ((Mona::Motor *)goal.motor)->goals.SetValue(need_index, 0.0);
      }
      else
      {
         goal.motor = NULL;
      }
      goal.goal_value = goal_value;
      goal.enabled   = true;
      goals.Add(goal);
      goal_index = (int)goals.GetCount() - 1;
   }
   return (goal_index);
}


int Homeostat::AddGoal(Vector<SENSOR>& sensors, SENSOR_MODE sensor_mode,
                       NEED goal_value)
{
   return (AddGoal(sensors, sensor_mode, NULL_RESPONSE, goal_value));
}


// Find index of goal matching sensors, sensor mode
// and response. Return -1 for no match.
int Homeostat::FindGoal(Vector<SENSOR>& sensors, SENSOR_MODE sensor_mode,
                        RESPONSE response)
{
   int i;

   if ((i = FindGoal(sensors, sensor_mode)) != -1)
   {
      if (goals[i].response == response)
      {
         return (i);
      }
      else
      {
         return (-1);
      }
   }
   return (-1);
}


// Find index of goal matching sensors and sensor mode.
// Return -1 for no match.
int Homeostat::FindGoal(Vector<SENSOR>& sensors, SENSOR_MODE sensor_mode)
{
   Vector<SENSOR> sensors_work;

   // Apply the sensor mode.
   mona->ApplySensorMode(sensors, sensors_work, sensor_mode);

   for (int i = 0; i < (int)goals.GetCount(); i++)
   {
      if ((goals[i].sensor_mode == sensor_mode) &&
          (Mona::Receptor::GetSensorDistance(&goals[i].sensors, &sensors_work)
           <= mona->sensor_modes[sensor_mode]->resolution))
      {
         return (i);
      }
   }
   return (-1);
}


// Get goal information at index.
bool Homeostat::GetGoalInfo(int goal_index, Vector<SENSOR>& sensors,
                            SENSOR_MODE& sensor_mode, RESPONSE& response,
                            NEED& goal_value, bool& enabled)
{
   if ((goal_index < 0) || (goal_index >= (int)goals.GetCount()))
   {
      return false;
   }
   sensors    = goals[goal_index].sensors;
   sensor_mode = goals[goal_index].sensor_mode;
   response   = goals[goal_index].response;
   goal_value  = goals[goal_index].goal_value;
   enabled    = goals[goal_index].enabled;
   return true;
}


// Is goal enabled?
bool Homeostat::IsGoalEnabled(int goal_index)
{
   if ((goal_index >= 0) && (goal_index < (int)goals.GetCount()))
   {
      return (goals[goal_index].enabled);
   }
   else
   {
      return false;
   }
}


// Enable goal.
bool Homeostat::EnableGoal(int goal_index)
{
   if ((goal_index >= 0) && (goal_index < (int)goals.GetCount()))
   {
      goals[goal_index].enabled = true;
      ((Mona::Receptor *)goals[goal_index].receptor)->goals.SetValue(need_index,
                                                                    goals[goal_index].goal_value);
      return true;
   }
   else
   {
      return false;
   }
}


// Disable goal.
bool Homeostat::DisableGoal(int goal_index)
{
   if ((goal_index >= 0) && (goal_index < (int)goals.GetCount()))
   {
      goals[goal_index].enabled = false;
      ((Mona::Receptor *)goals[goal_index].receptor)->goals.SetValue(need_index, 0.0);
      return true;
   }
   else
   {
      return false;
   }
}


// Remove goal at index.
bool Homeostat::RemoveGoal(int goal_index)
{
   if ((goal_index < 0) || (goal_index >= (int)goals.GetCount()))
   {
      return false;
   }

   // Clear goal value of associated receptors.
   ((Mona::Receptor *)goals[goal_index].receptor)->goals.SetValue(need_index, 0.0);

   // Clear goal value of associated motor.
   if (goals[goal_index].response != NULL_RESPONSE)
   {
      ((Mona::Motor *)goals[goal_index].motor)->goals.SetValue(need_index, 0.0);
   }

   // Remove entry.
   goals.erase(goals.Begin() + goal_index);

   return true;
}


// Remove neuron from goals.
void Homeostat::RemoveNeuron(void *neuron)
{
   int  i;
   bool done;

   done = false;
   while (!done)
   {
      done = true;
      for (i = 0; i < (int)goals.GetCount(); i++)
      {
         if ((goals[i].receptor == neuron) || (goals[i].motor == neuron))
         {
            ((Mona::Receptor *)goals[i].receptor)->goals.SetValue(need_index, 0.0);
            if (goals[i].response != NULL_RESPONSE)
            {
               ((Mona::Motor *)goals[i].motor)->goals.SetValue(need_index, 0.0);
            }
            goals.erase(goals.Begin() + i);
            done = false;
            break;
         }
      }
   }
}


// Update homeostat based on sensors.
void Homeostat::SensorsUpdate()
{
   int i;

   Vector<SENSOR> sensors;

   // Update the need value when sensors match.
   for (i = 0; i < (int)goals.GetCount(); i++)
   {
      mona->ApplySensorMode(mona->sensors, sensors, goals[i].sensor_mode);
      if (Mona::Receptor::GetSensorDistance(&goals[i].sensors, &sensors)
          <= mona->sensor_modes[goals[i].sensor_mode]->resolution)
      {
         if (goals[i].response != NULL_RESPONSE)
         {
            // Shift goal value to motor.
            ((Mona::Motor *)goals[i].motor)->goals.SetValue(need_index, goals[i].goal_value);
            ((Mona::Receptor *)goals[i].receptor)->goals.SetValue(need_index, 0.0);
         }
         else
         {
            need -= goals[i].goal_value;
            if (need < 0.0)
            {
               need = 0.0;
            }
         }
      }
   }
}


// Update homeostat based on response.
void Homeostat::ResponseUpdate()
{
   // Update the need value when response matches.
   for (int i = 0; i < (int)goals.GetCount(); i++)
   {
      if (goals[i].response != NULL_RESPONSE)
      {
         if (goals[i].response == mona->response)
         {
            need -= ((Mona::Motor *)goals[i].motor)->goals.GetValue(need_index);
            if (need < 0.0)
            {
               need = 0.0;
            }
         }
         ((Mona::Motor *)goals[i].motor)->goals.SetValue(need_index, 0.0);
         ((Mona::Receptor *)goals[i].receptor)->goals.SetValue(need_index, goals[i].goal_value);
      }
   }

   // Time to induce need?
   if (frequency > 0)
   {
      freq_timer++;
      if (freq_timer >= frequency)
      {
         freq_timer = 0;
         need      = periodic_need;
      }
   }
}


// Load homeostat.
void Homeostat::Load(char *filename)
{
   FILE *fp;

   if ((fp = FOPEN_READ(filename)) == NULL)
   {
      fprintf(stderr, "Cannot load homeostat from file %s\n", filename);
      exit(1);
   }
   Load(fp);
   FCLOSE(fp);
}


void Homeostat::Load(FILE *fp)
{
   int    i, j, p, q;
   Goal   g;
   SENSOR s;
   ID     id;

   FREAD_DOUBLE(&need, fp);
   FREAD_INT(&need_index, fp);
   FREAD_DOUBLE(&need_delta, fp);
   FREAD_DOUBLE(&periodic_need, fp);
   FREAD_INT(&frequency, fp);
   FREAD_INT(&freq_timer, fp);
   goals.Clear();
   FREAD_INT(&j, fp);
   for (i = 0; i < j; i++)
   {
      g.sensors.Clear();
      FREAD_INT(&q, fp);
      for (p = 0; p < q; p++)
      {
         FREAD_FLOAT(&s, fp);
         g.sensors.Add(s);
      }
      FREAD_INT(&g.sensor_mode, fp);
      FREAD_LONG_LONG(&id, fp);
      g.receptor = (void *)mona->FindByID(id);
      FREAD_INT(&g.response, fp);
      if (g.response != NULL_RESPONSE)
      {
         g.motor = mona->FindMotorByResponse(g.response);
      }
      else
      {
         g.motor = NULL;
      }
      FREAD_DOUBLE(&g.goal_value, fp);
      FREAD_BOOL(&g.enabled, fp);
      goals.Add(g);
   }
}


// Save homeostat.
void Homeostat::Store(char *filename)
{
   FILE *fp;

   if ((fp = FOPEN_WRITE(filename)) == NULL)
   {
      fprintf(stderr, "Cannot save homeostat to file %s\n", filename);
      exit(1);
   }
   Store(fp);
   FCLOSE(fp);
}


// When changing format increment FORMAT in mona.h
void Homeostat::Store(FILE *fp)
{
   int i, j, p, q;

   FWRITE_DOUBLE(&need, fp);
   FWRITE_INT(&need_index, fp);
   FWRITE_DOUBLE(&need_delta, fp);
   FWRITE_DOUBLE(&periodic_need, fp);
   FWRITE_INT(&frequency, fp);
   FWRITE_INT(&freq_timer, fp);
   j = (int)goals.GetCount();
   FWRITE_INT(&j, fp);
   for (i = 0; i < j; i++)
   {
      q = (int)goals[i].sensors.GetCount();
      FWRITE_INT(&q, fp);
      for (p = 0; p < q; p++)
      {
         FWRITE_FLOAT(&goals[i].sensors[p], fp);
      }
      FWRITE_INT(&goals[i].sensor_mode, fp);
      FWRITE_LONG_LONG(&((Mona::Receptor *)goals[i].receptor)->id, fp);
      FWRITE_INT(&goals[i].response, fp);
      FWRITE_DOUBLE(&goals[i].goal_value, fp);
      FWRITE_BOOL(&goals[i].enabled, fp);
   }
}


// Print homeostat.
void Homeostat::Print(FILE *out)
{
   fprintf(out, "<homeostat>");
   fprintf(out, "<need>%f</need>", need);
   fprintf(out, "<need_index>%d</need_index>", need_index);
   fprintf(out, "<need_delta>%f</need_delta>", need_delta);
   fprintf(out, "<periodic_need>%f</periodic_need>", periodic_need);
   fprintf(out, "<frequency>%d</frequency>", frequency);
   fprintf(out, "<sensory_response_goals>");
   for (int i = 0; i < (int)goals.GetCount(); i++)
   {
      fprintf(out, "<sensory_response_goal>");
      fprintf(out, "<sensors>");
      for (int j = 0; j < (int)goals[i].sensors.GetCount(); j++)
      {
         fprintf(out, "<value>%f</value>", goals[i].sensors[j]);
      }
      fprintf(out, "</sensors>");
      fprintf(out, "<sensor_mode>%d</sensor_mode>", goals[i].sensor_mode);
      if (goals[i].response == NULL_RESPONSE)
      {
         fprintf(out, "<response>null</response>");
      }
      else
      {
         fprintf(out, "<response>%d</response>", goals[i].response);
      }
      fprintf(out, "<goal_value>%f</goal_value>", goals[i].goal_value);
      if (goals[i].enabled)
      {
         fprintf(out, "<enabled>true</enabled>");
      }
      else
      {
         fprintf(out, "<enabled>false</enabled>");
      }
      fprintf(out, "</sensory_response_goal>");
   }
   fprintf(out, "</sensory_response_goals>");
   fprintf(out, "</homeostat>");
}
