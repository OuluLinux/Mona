#include "Mona.h"

const Homeostat::ID Homeostat::NULL_ID = (ID)(-1);

// Homeostat constructors.
Homeostat::Homeostat() {
	need         = 0.0;
	need_index    = 0;
	need_delta    = 0.0;
	mona         = NULL;
	periodic_need = 0.0;
	frequency    = 0;
	freq_timer    = 0;
}


Homeostat::Homeostat(int need_index, Mona* mona) {
	need            = 0.0;
	this->need_index = need_index;
	need_delta       = 0.0;
	this->mona      = mona;
	periodic_need    = 0.0;
	frequency       = 0;
	freq_timer       = 0;
}


// Destructor.
Homeostat::~Homeostat() {
	goals.Clear();
}


// Set periodic need.
void Homeostat::SetPeriodicNeed(int frequency, NEED need) {
	this->frequency = frequency;
	periodic_need    = need;
	freq_timer       = 0;
}


// Clear periodic need.
void Homeostat::ClearPeriodicNeed() {
	periodic_need = 0.0;
	frequency    = 0;
	freq_timer    = 0;
}


// Add sensory-response goal.
// Replace duplicate response and goal value.
int Homeostat::AddGoal(Vector<SENSOR>& sensors, SENSOR_MODE sensor_mode,
					   RESPONSE response, NEED goal_value) {
	int goal_index, i;
	Vector<SENSOR> sensors_work;
	Receptor* receptor;
	SENSOR         distance;
	Goal           goal;

	// Check for duplicate.
	if ((goal_index = FindGoal(sensors, sensor_mode, false)) != -1) {
		((Receptor*)goals[goal_index].receptor)->goals.SetValue(need_index, goal_value);

		if (goals[goal_index].response != NULL_RESPONSE) {
			// Clear old motor goal value.
			((Motor*)goals[goal_index].motor)->goals.SetValue(need_index, 0.0);
		}

		goals[goal_index].response = response;

		if (response != NULL_RESPONSE) {
			goals[goal_index].motor = (void*)mona->FindMotorByResponse(response);
			ASSERT(goals[goal_index].motor != NULL);
			((Motor*)goals[goal_index].motor)->goals.SetValue(need_index, 0.0);
		}
		else
			goals[goal_index].motor = NULL;

		goals[goal_index].goal_value = goal_value;
	}
	else {
		// Add entry.
		mona->ApplySensorMode(sensors, sensors_work, sensor_mode);

		for (i = 0; i < (int)sensors_work.GetCount(); i++)
			goal.sensors.Add(sensors_work[i]);

		goal.sensor_mode = sensor_mode;
		receptor        = mona->GetCentroidReceptor(sensors_work, sensor_mode, distance);

		if ((receptor == NULL) ||
			((receptor != NULL) && (distance > mona->sensor_modes[sensor_mode]->resolution)))
			receptor = mona->NewReceptor(sensors_work, sensor_mode);

		receptor->goals.SetValue(need_index, goal_value);
		goal.receptor = (void*)receptor;
		goal.response = response;

		if (response != NULL_RESPONSE) {
			goal.motor = (void*)mona->FindMotorByResponse(response);
			ASSERT(goal.motor != NULL);
			((Motor*)goal.motor)->goals.SetValue(need_index, 0.0);
		}
		else
			goal.motor = NULL;

		goal.goal_value = goal_value;
		goal.enabled   = true;
		goals.Add(goal);
		goal_index = (int)goals.GetCount() - 1;
	}

	return (goal_index);
}


int Homeostat::AddGoal(Vector<SENSOR>& sensors, SENSOR_MODE sensor_mode,
					   NEED goal_value) {
	return (AddGoal(sensors, sensor_mode, NULL_RESPONSE, goal_value));
}


// Find index of goal matching sensors, sensor mode
// and response. Return -1 for no match.
int Homeostat::FindGoal(Vector<SENSOR>& sensors, SENSOR_MODE sensor_mode,
						RESPONSE response) {
	int i;

	if ((i = FindGoal(sensors, sensor_mode)) != -1) {
		if (goals[i].response == response)
			return (i);
		else
			return (-1);
	}

	return (-1);
}


// Find index of goal matching sensors and sensor mode.
// Return -1 for no match.
int Homeostat::FindGoal(Vector<SENSOR>& sensors, SENSOR_MODE sensor_mode) {
	Vector<SENSOR> sensors_work;
	// Apply the sensor mode.
	mona->ApplySensorMode(sensors, sensors_work, sensor_mode);

	for (int i = 0; i < (int)goals.GetCount(); i++) {
		if ((goals[i].sensor_mode == sensor_mode) &&
			(Receptor::GetSensorDistance(&goals[i].sensors, &sensors_work)
			 <= mona->sensor_modes[sensor_mode]->resolution))
			return (i);
	}

	return (-1);
}


// Get goal information at index.
bool Homeostat::GetGoalInfo(int goal_index, Vector<SENSOR>& sensors,
							SENSOR_MODE& sensor_mode, RESPONSE& response,
							NEED& goal_value, bool& enabled) {
	if ((goal_index < 0) || (goal_index >= (int)goals.GetCount()))
		return false;

	sensors    = goals[goal_index].sensors;
	sensor_mode = goals[goal_index].sensor_mode;
	response   = goals[goal_index].response;
	goal_value  = goals[goal_index].goal_value;
	enabled    = goals[goal_index].enabled;
	return true;
}


// Is goal enabled?
bool Homeostat::IsGoalEnabled(int goal_index) {
	if ((goal_index >= 0) && (goal_index < (int)goals.GetCount()))
		return (goals[goal_index].enabled);
	else
		return false;
}


// Enable goal.
bool Homeostat::EnableGoal(int goal_index) {
	if ((goal_index >= 0) && (goal_index < (int)goals.GetCount())) {
		goals[goal_index].enabled = true;
		((Receptor*)goals[goal_index].receptor)->goals.SetValue(need_index,
																goals[goal_index].goal_value);
		return true;
	}
	else
		return false;
}


// Disable goal.
bool Homeostat::DisableGoal(int goal_index) {
	if ((goal_index >= 0) && (goal_index < (int)goals.GetCount())) {
		goals[goal_index].enabled = false;
		((Receptor*)goals[goal_index].receptor)->goals.SetValue(need_index, 0.0);
		return true;
	}
	else
		return false;
}


// Remove goal at index.
bool Homeostat::RemoveGoal(int goal_index) {
	if ((goal_index < 0) || (goal_index >= (int)goals.GetCount()))
		return false;

	// Clear goal value of associated receptors.
	((Receptor*)goals[goal_index].receptor)->goals.SetValue(need_index, 0.0);

	// Clear goal value of associated motor.
	if (goals[goal_index].response != NULL_RESPONSE)
		((Motor*)goals[goal_index].motor)->goals.SetValue(need_index, 0.0);

	// Remove entry.
	goals.Remove(goals.Begin() + goal_index);
	return true;
}


// Remove neuron from goals.
void Homeostat::RemoveNeuron(void* neuron) {
	int  i;
	bool done;
	done = false;

	while (!done) {
		done = true;

		for (i = 0; i < (int)goals.GetCount(); i++) {
			if ((goals[i].receptor == neuron) || (goals[i].motor == neuron)) {
				((Receptor*)goals[i].receptor)->goals.SetValue(need_index, 0.0);

				if (goals[i].response != NULL_RESPONSE)
					((Motor*)goals[i].motor)->goals.SetValue(need_index, 0.0);

				goals.Remove(goals.Begin() + i);
				done = false;
				break;
			}
		}
	}
}


// Update homeostat based on sensors.
void Homeostat::SensorsUpdate() {
	int i;
	Vector<SENSOR> sensors;

	// Update the need value when sensors match.
	for (i = 0; i < (int)goals.GetCount(); i++) {
		mona->ApplySensorMode(mona->sensors, sensors, goals[i].sensor_mode);

		if (Receptor::GetSensorDistance(&goals[i].sensors, &sensors)
			<= mona->sensor_modes[goals[i].sensor_mode]->resolution) {
			if (goals[i].response != NULL_RESPONSE) {
				// Shift goal value to motor.
				((Motor*)goals[i].motor)->goals.SetValue(need_index, goals[i].goal_value);
				((Receptor*)goals[i].receptor)->goals.SetValue(need_index, 0.0);
			}
			else {
				need -= goals[i].goal_value;

				if (need < 0.0)
					need = 0.0;
			}
		}
	}
}


// Update homeostat based on response.
void Homeostat::ResponseUpdate() {
	// Update the need value when response matches.
	for (int i = 0; i < (int)goals.GetCount(); i++) {
		if (goals[i].response != NULL_RESPONSE) {
			if (goals[i].response == mona->response) {
				need -= ((Motor*)goals[i].motor)->goals.GetValue(need_index);

				if (need < 0.0)
					need = 0.0;
			}

			((Motor*)goals[i].motor)->goals.SetValue(need_index, 0.0);
			((Receptor*)goals[i].receptor)->goals.SetValue(need_index, goals[i].goal_value);
		}
	}

	// Time to induce need?
	if (frequency > 0) {
		freq_timer++;

		if (freq_timer >= frequency) {
			freq_timer = 0;
			need      = periodic_need;
		}
	}
}


// Load homeostat.
void Homeostat::Load(String filename) {
	LoadFromFile(*this, filename);
}


void Homeostat::Serialize(Stream& fp) {
	fp % need % need_index % need_delta % periodic_need % frequency % freq_timer % goals;

	if (fp.IsLoading()) {
		for (int i = 0; i < goals.GetCount(); i++) {
			Goal& g = goals[i];
			g.receptor = (void*)mona->FindByID(g.id);

			if (g.response != NULL_RESPONSE)
				g.motor = mona->FindMotorByResponse(g.response);
			else
				g.motor = NULL;
		}
	}
}


// Save homeostat.
void Homeostat::Store(String filename) {
	StoreToFile(*this, filename);
}


// Print homeostat.
/*  void Homeostat::Print(FILE *out)
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
    }*/
