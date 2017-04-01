#include "Mona.h"

// Respond.
void
Mona::Respond() {
	int                i, j;
	Motor*              motor;
	RESPONSE_POTENTIAL max;
	#ifdef MONA_TRACE

	if (trace_respond)
		printf("***Respond phase***\n");

	#endif

	// Get response potentials from motor motives.
	for (i = 0; i < response_count; i++)
		response_potentials[i] = 0.0;

	for (i = 0; i < (int)motors.GetCount(); i++) {
		motor = motors[i];
		response_potentials[motor->response] += motor->motive;
	}

	// Incorporate minimal randomness.
	for (i = 0; i < response_count; i++) {
		if (response_potentials[i] <= NEARLY_ZERO) {
			response_potentials[i] =
				(random.RAND_PROB() * RESPONSE_RANDOMNESS);
		}
	}

	// Make a random response selection?
	if (RandomChance(RESPONSE_RANDOMNESS)) {
		if (response_count > 0) {
			double* responseWeights = new double[response_count];
			ASSERT(responseWeights != NULL);

			for (i = 0; i < response_count; i++) {
				responseWeights[i] = response_potentials[i];

				if (i > 0)
					responseWeights[i] += responseWeights[i - 1];
			}

			int    j = response_count - 1;
			double n = RandomInterval(0.0, responseWeights[j]);

			for (i = 0; i < j; i++) {
				if (n <= responseWeights[i])
					break;
			}

			response = i;
			delete [] responseWeights;
		}
		else
			response = 0;
	}
	else {
		// Select maximum response potential.
		bool first = true;
		j = Random(response_count);

		for (i = response = 0, max = 0.0; i < response_count; i++) {
			if (first || (response_potentials[j] > max)) {
				first    = false;
				response = (RESPONSE)j;
				max      = response_potentials[j];
			}

			j++;

			if (j == response_count)
				j = 0;
		}
	}

	// Response overridden?
	if (response_override != NULL_RESPONSE) {
		// Conditional override?
		if (response_override_potential >= 0.0) {
			if (response_potentials[(int)response] < response_override_potential)
				response = response_override;
		}
		else
			response = response_override;

		response_override          = NULL_RESPONSE;
		response_override_potential = -1.0;
	}

	// Fire responding motor.
	for (i = 0; i < (int)motors.GetCount(); i++) {
		motor = motors[i];

		if (motor->response == response) {
			motor->firing_strength = 1.0;
			#ifdef MONA_TRACE

			if (trace_respond)
				printf("Motor firing: %llu\n", motor->id);

			#endif
			#ifdef MONA_TRACKING
			motor->tracker.fire = true;
			#endif
		}
		else
			motor->firing_strength = 0.0;
	}

	#ifdef MONA_TRACE

	if (trace_respond)
		printf("Response = %d\n", response);

	#endif

	// Update need based on response.
	for (i = 0; i < need_count; i++)
		homeostats[i]->ResponseUpdate();
}


// Add a response.
RESPONSE Mona::AddResponse() {
	int i;
	response_count++;
	response_potentials.SetCount(response_count);

	for (i = 0; i < response_count; i++)
		response_potentials[i] = 0.0;

	i = response_count - 1;
	NewMotor(i);
	return (i);
}


// Get response potential.
RESPONSE_POTENTIAL Mona::GetResponsePotential(RESPONSE response) {
	if ((response >= 0) && (response < response_count))
		return (response_potentials[response]);
	else
		return (0.0);
}


// Override response.
// Auto-cleared each cycle.
bool Mona::OverrideResponse(RESPONSE response_override) {
	if ((response_override >= 0) && (response_override < response_count)) {
		this->response_override    = response_override;
		response_override_potential = -1.0;
		return true;
	}
	else
		return false;
}


// Conditionally override response if potential is less than given value.
// Auto-cleared each cycle.
bool Mona::OverrideResponseConditional(RESPONSE           response_override,
									   RESPONSE_POTENTIAL response_override_potential) {
	if ((response_override >= 0) && (response_override < response_count)) {
		this->response_override          = response_override;
		this->response_override_potential = response_override_potential;
		return true;
	}
	else
		return false;
}


// Clear response override.
void Mona::ClearResponseOverride() {
	response_override          = NULL_RESPONSE;
	response_override_potential = -1.0;
}


// Find motor by response.
Motor* Mona::FindMotorByResponse(RESPONSE response) {
	Motor* motor;

	for (int i = 0; i < (int)motors.GetCount(); i++) {
		motor = motors[i];

		if (motor->response == response)
			return (motor);
	}

	return (NULL);
}
