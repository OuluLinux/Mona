#include "TopWindow.h"


MuzzWorld::MuzzWorld() {
	int i;
	cycles       = -1;
	cycle_counter = 0;
	// Show graphics.
	graphics = true;
	// Smooth movements.
	smooth_move			= true;
	run_muzz_world_until_goals_gotten = false;
	forced_response_train = false;
	manual_mode			= false;
	terrain_mode			= true;
	wire_view           = false;
	pause              = false;
	step               = false;
	annotate_graph		= true;
	manual_response     = INVALID_RESPONSE;
	muzz_count = default_muzz_count;
	mushroom_count    = default_mushroom_count;
	pool_count = default_pool_count;
	response_index = INIT_INDEX;
	// Training.
	training_trial_count   = -1;
	current_trial        = 0;
	current_trial_step    = 0;
	trial_reset_delay     = -1;
	training_trial_resume = -1;
	frame_rate = 100;
	got_muzz_count        = false;
	got_mushroom_count     = false;
	got_pool_count         = false;
	got_terrain_dimension = false;
	terrain_type            = STANDARD_TERRAIN;
	terrain_dimension       = default_TERRAIN_DIMENSION;

	if (!graphics) {
		ASSERT(!pause);
		ASSERT(cycles >= 0);
	}

	if (terrain_type == TMAZE_TERRAIN) {
		ASSERT(muzz_count == 1);
		ASSERT(mushroom_count == 1);
		ASSERT(pool_count == 0);
	}

	ASSERT_(!((training_trial_count >= 0) && (muzz_count != 1)), "Number of muzzes must equal 1 for training");
	ASSERT_(!(forced_response_train && (training_trial_count < 0)), "Forced response training option invalid without number of training trials option");

	// If graphics and not counting cycles and not training, smooth movements.
	if (graphics && (cycles < 0) && (training_trial_count < 0))
		smooth_move = true;
	else
		smooth_move = false;

	// Initialize.
	InitMuzzWorld();
	// Run.
	RunMuzzWorld();
}

#define SERIALIZE(x, y)

void MuzzWorld::Store(ValueMap& map) {
	SERIALIZE(cycles, cycles);
	SERIALIZE(initHunger, Muzz::INIT_HUNGER);
	SERIALIZE(initThirst, Muzz::INIT_THIRST);
	SERIALIZE(nummuzzes, muzz_count);
	SERIALIZE(nummushrooms, mushroom_count);
	SERIALIZE(numpools, pool_count);
	SERIALIZE(load, LoadFile);
	SERIALIZE(save, SaveFile);
	SERIALIZE(random_seed, random_seed);
	SERIALIZE(objectSeed, object_seed);
	//SERIALIZE(TmazeTerrain, );
	// terrain_type = TMAZE_TERRAIN;
	SERIALIZE(terrainSeed, terrain_seed);
	SERIALIZE(terrainSize, terrain_dimension);
	//SERIALIZE(nographics, );
	// graphics = false;
	//SERIALIZE(pause, );
	// pause = true;
	SERIALIZE(numTrainingTrials, training_trial_count);
	//SERIALIZE(forcedResponseTrain, );
	// forced_response_train = true;
}

void MuzzWorld::Load(const ValueMap& map) {
}


void MuzzWorld::DockInit() {
}

void MuzzWorld::Runmuzzes() {
	int  i;
	bool moveDone, gotGoals, foundGoals;
	// Complete movements for previous responses before proceeding.
	moveDone = true;

	for (i = 0; i < muzz_count; i++) {
		if (!MoveMuzz(i))
			moveDone = false;
	}

	if (!moveDone)
		return;

	// Determine sensory input for muzzes.
	for (i = 0; i < muzz_count; i++)
		SenseMuzz(i);

	// Manual mode?
	if (manual_mode) {
		// Only current muzz can run.
		if (current_muzz != -1) {
			// Manual response provided?
			if (manual_response != INVALID_RESPONSE) {
				// Override muzz with manual response.
				muzzes[current_muzz],brain->response_override = manual_response;
				RunMuzz(current_muzz);
				muzzes[current_muzz],brain->response_override = Mona::NULL_RESPONSE;
				// Wait for next manual response.
				manual_response = INVALID_RESPONSE;
			}

			// Reset training when leaving manual mode.
			if (current_trial < training_trial_count)
				training_trial_resume = current_trial;
		}

		return;
	}

	// Resume training?
	if (training_trial_resume != -1) {
		i = training_trial_resume;
		ResetTraining();
		ResetMuzzWorld();
		current_trial        = i;
		training_trial_resume = -1;

		if (current_trial > training_trial_count)
			current_trial = training_trial_count;
	}

	// One-time search for forced response training? Much faster.
	if (forced_response_train && (response_index == INIT_INDEX) &&
		(current_trial < training_trial_count)) {
		if (GetResponseSequenceToGoals(0, forced_response_sequence, DEPTH_SEARCH))
			response_index = 0;
		else
			response_index = ERROR_INDEX;
	}

	// muzzes already have goals?
	gotGoals = true;

	for (i = 0; i < muzz_count; i++) {
		if (!muzzes[i]->has_food || !muzzes[i]->has_water)
			gotGoals = false;
	}

	// Run the muzzes.
	if (gotGoals)
		foundGoals = false;
	else
		foundGoals = true;

	gotGoals = true;

	for (i = 0; i < muzz_count; i++) {
		// For forced response training, get response to obtain food and water.
		if (forced_response_train && (current_trial < training_trial_count)) {
			if ((response_index >= 0) && (response_index <
									   (int)forced_response_sequence.GetCount())) {
				muzzes[i]->brain->response_override =
					forced_response_sequence[response_index].response;
				response_index++;
			}
			else
				muzzes[i]->brain->response_override = Mona::NULL_RESPONSE;
		}

		// Run the muzz.
		RunMuzz(i);

		// Goals not gotten?
		if (!muzzes[i]->has_food || !muzzes[i]->has_water)
			gotGoals = foundGoals = false;
	}

	// Increment training trial.
	if (current_trial < training_trial_count) {
		// Increment trial step.
		current_trial_step++;

		// Trial complete?
		if (gotGoals) {
			if (trial_reset_delay == -1)
				trial_reset_delay = 1;
			else
				trial_reset_delay--;
		}

		if (trial_reset_delay == 0) {
			// Reset for next trial.
			trial_reset_delay = -1;
			current_trial++;
			current_trial_step = 0;

			if (forced_response_train && (response_index >= 0))
				response_index = 0;

			ResetMuzzWorld();
		}
	}
}


// Idle muzzes.
void MuzzWorld::Idlemuzzes() {
	// Complete movements for previous responses.
	for (int i = 0; i < muzz_count; i++)
		MoveMuzz(i);
}


// Reset muzz world.
void MuzzWorld::ResetMuzzWorld() {
	int i;

	for (i = 0; i < muzz_count; i++) {
		muzzes[i]->brain->ResponseOverride = Mona::NULL_RESPONSE;
		muzzes[i]->Reset();

		if (mushroom_count == 0)
			muzzes[i]->ClearNeed(Muzz::FOOD);

		if (pool_count == 0)
			muzzes[i]->ClearNeed(Muzz::WATER);

		muzz_states[i].response   = Muzz::WAIT;
		muzz_states[i].moveType   = Muzz::FORWARD;
		muzz_states[i].moveAmount = 0.0;
	}

	for (i = 0; i < mushroom_count; i++)
		mushrooms[i]->SetAlive(true);
}


// Move muzz.
// Returns true when movement is complete.
bool MuzzWorld::MoveMuzz(int i) {
	int     j, x, z, x2, z2, x3, z3;
	double a, p[3], p2[3], f[3], u[3];
	bool    move;

	// Continue queued movement.
	if (muzz_states[i].moveAmount > 0.0)
		move = true;
	else
		move = false;

	muzzes[i]->GetPosition(p);
	x = (int)(p[0] / terrain.block_size);
	z = (int)(p[2] / terrain.block_size);

	if (move) {
		switch (muzz_states[i].moveType) {
		case Muzz::FORWARD:
			if (smooth_move) {
				a = MUZZ_MOVEMENT_DELTA * frame_rate.speedFactor;

				if (a > muzz_states[i].moveAmount)
					a = muzz_states[i].moveAmount;
			}
			else
				a = muzz_states[i].moveAmount;

			muzz_states[i].moveAmount -= a;
			muzzes[i]->Forward(a);
			// Check for collision.
			muzzes[i]->GetPosition(p);
			x2 = (int)(p[0] / terrain.block_size);
			z2 = (int)(p[2] / terrain.block_size);

			if ((x != x2) || (z != z2)) {
				for (j = 0; j < muzz_count; j++) {
					if (j == i)
						continue;

					muzzes[j]->GetPosition(p2);
					x3 = (int)(p2[0] / terrain.block_size);
					z3 = (int)(p2[2] / terrain.block_size);

					if ((x2 == x3) && (z2 == z3)) {
						muzzes[i]->Backward(a);
						muzz_states[i].moveAmount = 0.0;
						break;
					}
				}
			}

			break;

		case Muzz::RIGHT:
			if (smooth_move) {
				a = MUZZ_TURN_DELTA * frame_rate.speedFactor;

				if (a > muzz_states[i].moveAmount)
					a = muzz_states[i].moveAmount;
			}
			else
				a = muzz_states[i].moveAmount;

			muzz_states[i].moveAmount -= a;
			muzzes[i]->Right(a);
			break;

		case Muzz::LEFT:
			if (smooth_move) {
				a = MUZZ_TURN_DELTA * frame_rate.speedFactor;

				if (a > muzz_states[i].moveAmount)
					a = muzz_states[i].moveAmount;
			}
			else
				a = muzz_states[i].moveAmount;

			muzz_states[i].moveAmount -= a;
			muzzes[i]->Left(a);
			break;
		}
	}

	// Continue movement?
	if (muzz_states[i].moveAmount > 0.0f)
		return false;

	muzz_states[i].moveAmount = 0.0;
	// Center muzz on block, orient orthogonally and
	// store position and direction.
	muzzes[i]->GetPosition(p);
	x = (int)(p[0] / terrain.block_size);
	z = (int)(p[2] / terrain.block_size);

	if (move) {
		p2[0] = ((double)x + 0.5f) * terrain.block_size;
		p2[1] = p[1];
		p2[2] = ((double)z + 0.5f) * terrain.block_size;

		for (j = 0; j < 3; j++)
			p2[j] = p[j] + ((p2[j] - p[j]) * 0.1f);

		muzzes[i]->SetPosition(p2);
	}

	muzzes[i]->getForward(f);
	f[1] = 0.0;
	cSpacial::normalize(f);
	u[0] = 0.0;
	u[1] = 1.0;
	u[2] = 0.0;

	if (fabs(f[0]) > fabs(f[2])) {
		if (f[0] < 0.0f) {
			muzz_states[i].dir      = 3;
			muzz_states[i].forward  = BlockTerrain::Block::WEST;
			muzz_states[i].backward = BlockTerrain::Block::EAST;

			if (move) {
				muzzes[i]->LoadRotation(90.0f, u);
				muzzes[i]->Forward(0.0f);
			}
		}
		else {
			muzz_states[i].dir      = 1;
			muzz_states[i].forward  = BlockTerrain::Block::EAST;
			muzz_states[i].backward = BlockTerrain::Block::WEST;

			if (move) {
				muzzes[i]->LoadRotation(270.0f, u);
				muzzes[i]->Forward(0.0f);
			}
		}
	}
	else {
		if (f[2] < 0.0f) {
			muzz_states[i].dir      = 0;
			muzz_states[i].forward  = BlockTerrain::Block::NORTH;
			muzz_states[i].backward = BlockTerrain::Block::SOUTH;

			if (move) {
				muzzes[i]->LoadRotation(180.0f, u);
				muzzes[i]->Forward(0.0f);
			}
		}
		else {
			muzz_states[i].dir      = 2;
			muzz_states[i].forward  = BlockTerrain::Block::SOUTH;
			muzz_states[i].backward = BlockTerrain::Block::NORTH;

			if (move) {
				muzzes[i]->LoadRotation(0.0f, u);
				muzzes[i]->Forward(0.0f);
			}
		}
	}

	muzzes[i]->GetPosition(p);
	muzz_states[i].x = (int)(p[0] / terrain.block_size);
	muzz_states[i].z = (int)(p[2] / terrain.block_size);
	return true;
}


// Sense muzz world.
void MuzzWorld::SenseMuzz(int i) {
	int     terrain, object, lookx, lookz;
	double view[3];
	// Determine sensory input.
	muzzes[i]->getRight(view);

	for (int j = 0; j < 3; j++)
		view[j] = -view[j];

	senseMuzzDir(i, (muzz_states[i].dir + 1) % 4, view, lookx, lookz, terrain, object);

	if ((terrain == Muzz::WALL) || (terrain == Muzz::DROP) ||
		((object == Muzz::MUSHROOM) && muzzes[i]->has_food) ||
		(object == Muzz::MUZZ))
		muzz_states[i].sensors[Muzz::RIGHT_SENSOR] = Muzz::CLOSED;
	else
		muzz_states[i].sensors[Muzz::RIGHT_SENSOR] = Muzz::OPEN;

	muzzes[i]->getRight(view);
	senseMuzzDir(i, (muzz_states[i].dir + 3) % 4, view, lookx, lookz, terrain, object);

	if ((terrain == Muzz::WALL) || (terrain == Muzz::DROP) ||
		((object == Muzz::MUSHROOM) && muzzes[i]->has_food) ||
		(object == Muzz::MUZZ))
		muzz_states[i].sensors[Muzz::LEFT_SENSOR] = Muzz::CLOSED;
	else
		muzz_states[i].sensors[Muzz::LEFT_SENSOR] = Muzz::OPEN;

	muzzes[i]->getForward(view);
	senseMuzzDir(i, muzz_states[i].dir, view, lookx, lookz, terrain, object);
	muzz_states[i].sensors[Muzz::TERRAIN_SENSOR] = terrain;
	muzz_states[i].sensors[Muzz::OBJECT_SENSOR]  = object;

	if ((terrain == Muzz::WALL) || (terrain == Muzz::DROP) ||
		((object == Muzz::MUSHROOM) && muzzes[i]->has_food) ||
		(object == Muzz::MUZZ))
		muzz_states[i].sensors[Muzz::FORWARD_SENSOR] = Muzz::CLOSED;
	else
		muzz_states[i].sensors[Muzz::FORWARD_SENSOR] = Muzz::OPEN;

	muzz_states[i].lookAtX = lookx;
	muzz_states[i].lookAtZ = lookz;
}


// Sense muzz world in a given direction and view vector.
void MuzzWorld::SenseMuzzDir(int muzz_index, int direction, double view[3],
							 int& sense_x, int& sense_z, int& terrain, int& object) {
	int     i, h, x, z, x2, z2, x3, z3;
	double p[3], p2[3], v[3];
	BlockTerrain::Block::DIRECTION forward, backward;
	muzzes[muzz_index]->GetPosition(p);
	x = (int)(p[0] / terrain.block_size);
	z = (int)(p[2] / terrain.block_size);

	switch (direction) {
	case 0:                                        // North.
		if (z == 0) {
			terrain = Muzz::DROP;
			object  = Muzz::EMPTY;
			return;
		}
		else {
			sense_x = x2 = x;
			sense_z = z2 = z - 1;
		}

		forward  = BlockTerrain::Block::NORTH;
		backward = BlockTerrain::Block::SOUTH;
		break;

	case 1:                                        // East.
		if (x == terrain.width - 1) {
			terrain = Muzz::DROP;
			object  = Muzz::EMPTY;
			return;
		}
		else {
			sense_x = x2 = x + 1;
			sense_z = z2 = z;
		}

		forward  = BlockTerrain::Block::EAST;
		backward = BlockTerrain::Block::WEST;
		break;

	case 2:                                        // South.
		if (z == terrain.height - 1) {
			terrain = Muzz::DROP;
			object  = Muzz::EMPTY;
			return;
		}
		else {
			sense_x = x2 = x;
			sense_z = z2 = z + 1;
		}

		forward  = BlockTerrain::Block::SOUTH;
		backward = BlockTerrain::Block::NORTH;
		break;

	case 3:                                        // West.
		if (x == 0) {
			terrain = Muzz::DROP;
			object  = Muzz::EMPTY;
			return;
		}
		else {
			sense_x = x2 = x - 1;
			sense_z = z2 = z;
		}

		forward  = BlockTerrain::Block::WEST;
		backward = BlockTerrain::Block::EAST;
		break;
	}

	// Object in view?
	// Cannot see objects when crosswise on ramp.
	if ((terrain.blocks[x][z].type != BlockTerrain::Block::RAMP) ||
		((terrain.blocks[x][z].type == BlockTerrain::Block::RAMP) &&
		 ((terrain.blocks[x][z].rampDir == forward) ||
		  (terrain.blocks[x][z].rampDir == backward)))) {
		// Determine point in view.
		x3    = (int)(p[0] / terrain.block_size);
		p2[0] = ((double)x3 + 0.5f) * terrain.block_size;
		p2[1] = p[1];
		z3    = (int)(p[2] / terrain.block_size);
		p2[2] = ((double)z3 + 0.5f) * terrain.block_size;

		for (i = 0; i < 3; i++)
			v[i] = view[i];

		cSpacial::normalize(v);

		for (i = 0; i < 3; i++)
			p[i] = p2[i] + (v[i] * terrain.block_size);

		for (i = 0; i < muzz_count; i++) {
			if (i == muzz_index)
				continue;

			muzzes[i]->GetPosition(p2);

			if (fabs(cSpacial::pointDistance(p, p2)) < (terrain.block_size * 0.4f)) {
				x3 = (int)(p2[0] / terrain.block_size);
				z3 = (int)(p2[2] / terrain.block_size);

				if (terrain.blocks[x3][z3].type != BlockTerrain::Block::RAMP)
					terrain = Muzz::PLATFORM;
				else {
					if (terrain.blocks[x3][z3].rampDir == forward)
						terrain = Muzz::RAMP_UP;
					else
						terrain = Muzz::RAMP_DOWN;
				}

				object = Muzz::MUZZ;
				return;
			}
		}

		for (i = 0; i < mushroom_count; i++) {
			if (!mushrooms[i]->IsAlive())
				continue;

			mushrooms[i]->GetPosition(p2);

			if (fabs(cSpacial::pointDistance(p, p2)) < (terrain.block_size * 0.4f)) {
				terrain = Muzz::PLATFORM;
				object  = Muzz::MUSHROOM;
				return;
			}
		}

		for (i = 0; i < pool_count; i++) {
			pools[i]->GetPosition(p2);

			if (fabs(cSpacial::pointDistance(p, p2)) < (terrain.block_size * 0.4f)) {
				terrain = Muzz::PLATFORM;
				object  = Muzz::POOL;
				return;
			}
		}
	}

	// View terrain.
	h = terrain.blocks[x2][z2].elevation - terrain.blocks[x][z].elevation;

	switch (terrain.blocks[x][z].type) {
	case BlockTerrain::Block::PLATFORM:
		switch (terrain.blocks[x2][z2].type) {
		case BlockTerrain::Block::PLATFORM:
		case BlockTerrain::Block::LANDING:
			if (h < 0) {
				terrain = Muzz::DROP;
				object  = Muzz::EMPTY;
			}
			else if (h > 0) {
				terrain = Muzz::WALL;
				#if (SENSE_BLOCK_ID == 1)
				object = (terrain.blocks[x2][z2].id << 8) |
						 (unsigned char )((int)'A' + terrain.blocks[x2][z2].texture_indexes[terrain.blocks[x][z].elevation + 1]);
				#else
				object = (int)'A' + terrain.blocks[x2][z2].texture_indexes[terrain.blocks[x][z].elevation + 1];
				#endif
			}
			else {
				terrain = Muzz::PLATFORM;
				#if (SENSE_BLOCK_ID == 1)
				object = (terrain.blocks[x2][z2].id << 8) |
						 (unsigned char )((int)'A' + terrain.blocks[x2][z2].texture_indexes[terrain.blocks[x][z].elevation]);
				#else
				object = (int)'A' + terrain.blocks[x2][z2].texture_indexes[terrain.blocks[x][z].elevation];
				#endif
			}

			break;

		case BlockTerrain::Block::RAMP:
			if (h < 0) {
				terrain = Muzz::DROP;
				object  = Muzz::EMPTY;
			}
			else if (h > 0) {
				terrain = Muzz::WALL;
				object  = Muzz::EMPTY;
			}
			else {
				if (terrain.IsUpperRamp(x2, z2)) {
					terrain = Muzz::DROP;
					object  = Muzz::EMPTY;
				}
				else {
					terrain = Muzz::WALL;
					object  = Muzz::EMPTY;
				}
			}

			break;
		}

		break;

	case BlockTerrain::Block::LANDING:
		switch (terrain.blocks[x2][z2].type) {
		case BlockTerrain::Block::PLATFORM:
		case BlockTerrain::Block::LANDING:
			if (h < 0) {
				terrain = Muzz::DROP;
				object  = Muzz::EMPTY;
			}
			else if (h > 0) {
				terrain = Muzz::WALL;
				#if (SENSE_BLOCK_ID == 1)
				object = (terrain.blocks[x2][z2].id << 8) |
						 (unsigned char )((int)'A' + terrain.blocks[x2][z2].texture_indexes[terrain.blocks[x][z].elevation + 1]);
				#else
				object = (int)'A' + terrain.blocks[x2][z2].texture_indexes[terrain.blocks[x][z].elevation + 1];
				#endif
			}
			else {
				terrain = Muzz::PLATFORM;
				#if (SENSE_BLOCK_ID == 1)
				object = (terrain.blocks[x2][z2].id << 8) |
						 (unsigned char )((int)'A' + terrain.blocks[x2][z2].texture_indexes[terrain.blocks[x][z].elevation]);
				#else
				object = (int)'A' + terrain.blocks[x2][z2].texture_indexes[terrain.blocks[x][z].elevation];
				#endif
			}

			break;

		case BlockTerrain::Block::RAMP:
			if (terrain.blocks[x2][z2].rampDir == forward) {
				terrain = Muzz::RAMP_UP;
				object  = Muzz::EMPTY;
			}
			else if (terrain.blocks[x2][z2].rampDir == backward) {
				terrain = Muzz::RAMP_DOWN;
				object  = Muzz::EMPTY;
			}
			else if (h < 0) {
				terrain = Muzz::DROP;
				object  = Muzz::EMPTY;
			}
			else if (h > 0) {
				terrain = Muzz::WALL;
				object  = Muzz::EMPTY;
			}
			else {
				if (terrain.IsUpperRamp(x2, z2)) {
					terrain = Muzz::DROP;
					object  = Muzz::EMPTY;
				}
				else {
					terrain = Muzz::WALL;
					object  = Muzz::EMPTY;
				}
			}

			break;
		}

		break;

	case BlockTerrain::Block::RAMP:
		switch (terrain.blocks[x2][z2].type) {
		case BlockTerrain::Block::PLATFORM:
			if (h < 0) {
				terrain = Muzz::DROP;
				object  = Muzz::EMPTY;
			}
			else if (h > 0) {
				terrain = Muzz::WALL;
				object  = Muzz::EMPTY;
			}
			else {
				if (terrain.IsUpperRamp(x, z)) {
					terrain = Muzz::WALL;
					object  = Muzz::EMPTY;
				}
				else {
					terrain = Muzz::DROP;
					object  = Muzz::EMPTY;
				}
			}

			break;

		case BlockTerrain::Block::LANDING:
			if ((terrain.blocks[x][z].rampDir == forward) ||
				(terrain.blocks[x][z].rampDir == backward)) {
				terrain = Muzz::PLATFORM;
				#if (SENSE_BLOCK_ID == 1)
				object = (terrain.blocks[x2][z2].id << 8) |
						 (unsigned char )((int)'A' + terrain.blocks[x2][z2].texture_indexes[terrain.blocks[x][z].elevation]);
				#else
				object = (int)'A' + terrain.blocks[x2][z2].texture_indexes[terrain.blocks[x][z].elevation];
				#endif
			}
			else if (h < 0) {
				terrain = Muzz::DROP;
				object  = Muzz::EMPTY;
			}
			else if (h > 0) {
				terrain = Muzz::WALL;
				object  = Muzz::EMPTY;
			}
			else {
				if (terrain.IsUpperRamp(x, z)) {
					terrain = Muzz::WALL;
					object  = Muzz::EMPTY;
				}
				else {
					terrain = Muzz::DROP;
					object  = Muzz::EMPTY;
				}
			}

			break;

		case BlockTerrain::Block::RAMP:
			if ((terrain.blocks[x][z].rampDir == forward) &&
				(terrain.blocks[x2][z2].rampDir == forward)) {
				terrain = Muzz::RAMP_UP;
				object  = Muzz::EMPTY;
			}
			else if ((terrain.blocks[x][z].rampDir == backward) &&
					 (terrain.blocks[x2][z2].rampDir == backward)) {
				terrain = Muzz::RAMP_DOWN;
				object  = Muzz::EMPTY;
			}
			else if (h < 0) {
				terrain = Muzz::DROP;
				object  = Muzz::EMPTY;
			}
			else if (h > 0) {
				terrain = Muzz::WALL;
				object  = Muzz::EMPTY;
			}
			else {
				if (terrain.IsUpperRamp(x, z)) {
					terrain = Muzz::DROP;
					object  = Muzz::EMPTY;
				}
				else if (terrain.IsUpperRamp(x2, z2)) {
					terrain = Muzz::WALL;
					object  = Muzz::EMPTY;
				}
				else {
					terrain = Muzz::DROP;
					object  = Muzz::EMPTY;
				}
			}

			break;
		}

		break;
	}
}


// Run muzz.
void MuzzWorld::RunMuzz(int i, int forcedResponse) {
	int     j, dir, x, z, x2, z2, x3, z3;
	bool    hadFood;
	double p[3], p2[3], f[3];
	BlockTerrain::Block::DIRECTION forward;
	BlockTerrain::Block::DIRECTION backward;
	// Get muzz response.
	hadFood = muzzes[i]->has_food;

	if (forcedResponse == INVALID_RESPONSE) {
		// Let brain decide.
		muzz_states[i].response = muzzes[i]->Cycle(muzz_states[i].sensors);
	}
	else {
		// Force response.
		muzz_states[i].response = forcedResponse;

		if ((muzz_states[i].sensors[Muzz::OBJECT_SENSOR] == Muzz::MUSHROOM) &&
			(muzz_states[i].response == Muzz::EAT))
			muzzes[i]->has_food = true;

		if ((muzz_states[i].sensors[Muzz::OBJECT_SENSOR] == Muzz::POOL) &&
			(muzz_states[i].response == Muzz::DRINK))
			muzzes[i]->has_water = true;
	}

	// Process response.
	x        = muzz_states[i].x;
	z        = muzz_states[i].z;
	dir      = muzz_states[i].dir;
	forward  = muzz_states[i].forward;
	backward = muzz_states[i].backward;

	switch (muzz_states[i].response) {
	case Muzz::WAIT:
		break;

	case Muzz::FORWARD:
		muzz_states[i].moveType   = Muzz::FORWARD;
		muzz_states[i].moveAmount = terrain.block_size;

		if ((muzz_states[i].sensors[Muzz::OBJECT_SENSOR] == Muzz::MUSHROOM) ||
			(muzz_states[i].sensors[Muzz::OBJECT_SENSOR] == Muzz::MUZZ)) {
			muzz_states[i].moveAmount = 0.0;
			break;
		}

		switch (dir) {
		case 0:                                     // North.
			if (z == 0) {
				muzz_states[i].moveAmount = 0.0;
				return;
			}

			x2 = x;
			z2 = z - 1;
			break;

		case 1:                                     // East.
			if (x == terrain.width - 1) {
				muzz_states[i].moveAmount = 0.0;
				return;
			}

			x2 = x + 1;
			z2 = z;
			break;

		case 2:                                     // South.
			if (z == terrain.height - 1) {
				muzz_states[i].moveAmount = 0.0;
				return;
			}

			x2 = x;
			z2 = z + 1;
			break;

		case 3:                                     // West.
			if (x == 0) {
				muzz_states[i].moveAmount = 0.0;
				return;
			}

			x2 = x - 1;
			z2 = z;
			break;
		}

		if (terrain.blocks[x][z].type == BlockTerrain::Block::RAMP) {
			if ((terrain.blocks[x][z].rampDir != forward) &&
				(terrain.blocks[x][z].rampDir != backward)) {
				muzz_states[i].moveAmount = 0.0;
				return;
			}
		}

		if (((terrain.blocks[x][z].type != BlockTerrain::Block::RAMP) ||
			 (terrain.blocks[x2][z2].type != BlockTerrain::Block::RAMP)) &&
			(terrain.blocks[x][z].elevation != terrain.blocks[x2][z2].elevation))
			muzz_states[i].moveAmount = 0.0;
		else if ((terrain.blocks[x][z].type != BlockTerrain::Block::RAMP) &&
				 (terrain.blocks[x2][z2].type == BlockTerrain::Block::RAMP) &&
				 (terrain.blocks[x2][z2].rampDir != forward) &&
				 (terrain.blocks[x2][z2].rampDir != backward))
			muzz_states[i].moveAmount = 0.0;
		else {
			switch (terrain.blocks[x][z].type) {
			case BlockTerrain::Block::LANDING:
				if (terrain.blocks[x2][z2].type == BlockTerrain::Block::RAMP) {
					muzz_states[i].moveAmount = (terrain.block_size * 0.5f) +
											   (terrain.block_size * (2.236f / 4.0f));
				}

				break;

			case BlockTerrain::Block::RAMP:
				if (terrain.blocks[x2][z2].type == BlockTerrain::Block::RAMP)
					muzz_states[i].moveAmount = terrain.block_size * (2.236f / 2.0f);
				else if (terrain.blocks[x2][z2].type == BlockTerrain::Block::LANDING) {
					muzz_states[i].moveAmount = (terrain.block_size * 0.5f) +
											   (terrain.block_size * (2.236f / 4.0f));
				}

				break;
			}
		}

		break;

	case Muzz::RIGHT:
		muzz_states[i].moveType   = Muzz::RIGHT;
		muzz_states[i].moveAmount = 90.0;
		break;

	case Muzz::LEFT:
		muzz_states[i].moveType   = Muzz::LEFT;
		muzz_states[i].moveAmount = 90.0;
		break;

	case Muzz::EAT:
		if (!hadFood && muzzes[i]->has_food &&
			(muzz_states[i].sensors[Muzz::OBJECT_SENSOR] == Muzz::MUSHROOM)) {
			muzzes[i]->GetPosition(p);
			x3    = (int)(p[0] / terrain.block_size);
			p2[0] = ((double)x3 + 0.5f) * terrain.block_size;
			p2[1] = p[1];
			z3    = (int)(p[2] / terrain.block_size);
			p2[2] = ((double)z3 + 0.5f) * terrain.block_size;
			muzzes[i]->getForward(f);
			cSpacial::normalize(f);

			for (j = 0; j < 3; j++)
				p[j] = p2[j] + (f[j] * terrain.block_size);

			for (j = 0; j < mushroom_count; j++) {
				if (!mushrooms[j]->IsAlive())
					continue;

				mushrooms[j]->GetPosition(p2);

				if (fabs(cSpacial::pointDistance(p, p2)) < (terrain.block_size * 0.4f))
					mushrooms[j]->SetAlive(false);
			}
		}

		break;

	case Muzz::DRINK:
		break;
	}
}


// Place muzz in world without changing its initial stored placement.
void MuzzWorld::PlaceMuzz(int muzz_index, int placeX, int placeZ, BlockTerrain::Block::DIRECTION place_dir) {
	double p[3];
	Vector  n;
	// Clear pending movement.
	muzz_states[muzz_index].moveType   = Muzz::FORWARD;
	muzz_states[muzz_index].moveAmount = 0.0;
	muzzes[muzz_index]->clearSpacial();
	p[0] = ((double)placeX * terrain.block_size) + (terrain.block_size * 0.5f);
	p[2] = ((double)placeZ * terrain.block_size) + (terrain.block_size * 0.5f);
	terrain.GetGeometry(p[0], p[2], p[1], n);
	muzzes[muzz_index]->SetPosition(p);

	switch (place_dir) {
	case BlockTerrain::Block::NORTH:
		muzzes[muzz_index]->setYaw(0.0f);
		break;

	case BlockTerrain::Block::EAST:
		muzzes[muzz_index]->setYaw(90.0f);
		break;

	case BlockTerrain::Block::SOUTH:
		muzzes[muzz_index]->setYaw(180.0f);
		break;

	case BlockTerrain::Block::WEST:
		muzzes[muzz_index]->setYaw(270.0f);
		break;
	}

	muzzes[muzz_index]->Forward(0.0f);
}


// Reset training.
void MuzzWorld::ResetTraining() {
	current_trial        = 0;
	current_trial_step    = 0;
	trial_reset_delay     = -1;
	training_trial_resume = -1;

	if (response_index > 0)
		response_index = 0;
}

void MuzzWorld::Idle() {
	// Check cycles.
	if ((cycles >= 0) && (!pause || step) && (Mode == TERRAIN_MODE)) {
		cycle_counter++;

		if (cycle_counter > cycles) {
			termMuzzWorld();
			glutDestroyWindow(main_window);
			exit(0);
		}
	}

	// Check for completion of training trials.
	// pause to allow manual test trial.
	static bool testpause = false;

	if (!testpause && (current_trial == training_trial_count)) {
		pause = true;
		pauseCheck->setChecked(true);
		testpause = true;
	}

	// Run muzzes.
	if (Mode != HELP_MODE) {
		bool smoothMoveSave = smooth_move;

		if (step) {
			smooth_move = false;
			pause      = false;
		}

		if (!pause)
			runmuzzes();
		else
			idlemuzzes();

		smooth_move = smoothMoveSave;

		if (step) {
			pause = true;
			pauseCheck->setChecked(true);
			step = false;
		}
	}

	// Re-display.
	glutPostRedisplay();
}


// Initialize.
void MuzzWorld::InitMuzzWorld() {
	int       i, j, k, x, z, x2, z2;
	double     color[3];
	FILE*      fp;
	Random*    objectrandomizer;
	double   p[3], p2[3];
	const int maxtries = 1000;
	// Initialize graphics (even if turned off).
	glutInitWindowSize(WindowWidth, WindowHeight);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	main_window = glutCreateWindow("Muzz World");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialKeyboard);
	glutIdleFunc(idle);
	glutMouseFunc(mouseClicked);
	glutMotionFunc(mouseDragged);
	glutPassiveMotionFunc(mouseMoved);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glPointSize(3.0);
	glLineWidth(1.0);

	// Load?
	if (LoadFile != NULL) {
		if ((fp = FOPEN_READ(LoadFile)) == NULL) {
			fprintf(stderr, "Cannot load from file %s\n", LoadFile);
			glutDestroyWindow(main_window);
			exit(1);
		}

		FREAD_INT(&muzz_count, fp);
		FREAD_INT(&mushroom_count, fp);
		FREAD_INT(&pool_count, fp);
		FREAD_LONG(&RandomSeed, fp);
		randomizer = new Random(RandomSeed);
		ASSERT(randomizer != NULL);
		randomizer->RAND_LOAD(fp);
		FREAD_LONG(&ObjectSeed, fp);
		FREAD_INT(&i, fp);
		terrain_type = (TERRAIN_TYPE)i;
		FREAD_LONG(&TerrainSeed, fp);
		FREAD_INT(&terrain_dimension, fp);
	}
	else {
		if (RandomSeed == INVALID_int) {
			RandomSeed = (int)time(NULL);
			randomizer = new Random(RandomSeed);
			ASSERT(randomizer != NULL);
		}

		if (ObjectSeed == INVALID_int)
			ObjectSeed = RandomSeed;

		if (TerrainSeed == INVALID_int)
			TerrainSeed = RandomSeed;

		fp = NULL;
	}

	// Create terrain.
	if (terrain_type == STANDARD_TERRAIN) {
		Terrain = new BlockTerrain(TerrainSeed, terrain_dimension, terrain_dimension,
								   BlockTerrain::default_max_platform_elevation, BlockTerrain::default_min_platform_size,
								   max((int)BlockTerrain::default_min_platform_size, terrain_dimension / 2),
								   BlockTerrain::default_max_platform_generations, BlockTerrain::default_extra_ramps,
								   BlockTerrain::default_block_size);
		ASSERT(Terrain != NULL);
	}
	else {
		maze_terrain = new TmazeTerrain(TerrainSeed, terrain_dimension, terrain_dimension,
									   BlockTerrain::default_block_size);
		ASSERT(maze_terrain != NULL);
		Terrain = (BlockTerrain*)maze_terrain;
	}

	TerrainViewPosition[0] = ((float)terrain.width * terrain.block_size) / 2.0;
	TerrainViewPosition[1] = TERRAIN_INITIAL_VIEW_height;
	TerrainViewPosition[2] = ((float)terrain.width * terrain.block_size) / 2.0;
	// Create objects based on object seed.
	objectrandomizer = new Random(ObjectSeed);
	ASSERT(objectrandomizer != NULL);
	// Create muzzes.
	muzzes.resize(muzz_count);
	muzz_states.resize(muzz_count);

	for (i = 0; i < muzz_count; i++) {
		for (k = 0; k < maxtries; k++) {
			for (j = 0; j < 3; j++)
				color[j] = objectrandomizer->RAND_INTERVAL(0.0, 1.0);

			muzzes[i] = new Muzz(color, Terrain, objectrandomizer->RAND(), randomizer);
			ASSERT(muzzes[i] != NULL);

			if (terrain_type == TMAZE_TERRAIN) {
				muzzes[i]->Place(maze_terrain->tmaze_path[0].first,
								 maze_terrain->tmaze_path[0].second, BlockTerrain::Block::NORTH);
			}

			if (fp != NULL) {
				muzzes[i]->Load(fp);
				break;
			}
			else {
				// Check for overlay.
				muzzes[i]->GetPosition(p);
				x = (int)(p[0] / terrain.block_size);
				z = (int)(p[2] / terrain.block_size);

				for (j = 0; j < i; j++) {
					muzzes[j]->GetPosition(p2);
					x2 = (int)(p2[0] / terrain.block_size);
					z2 = (int)(p2[2] / terrain.block_size);

					if ((x == x2) && (z == z2))
						break;
				}

				if (j < i) {
					delete muzzes[i];
					muzzes[i] = NULL;
					continue;
				}
				else
					break;
			}
		}

		if (k == maxtries) {
			fprintf(stderr, "Cannot place muzz on terrain\n");
			glutDestroyWindow(main_window);
			exit(1);
		}
	}

	if (fp != NULL) {
		FREAD_INT(&current_muzz, fp);

		for (i = 0; i < muzz_count; i++) {
			for (j = 0; j < Muzz::NUM_SENSORS; j++)
				FREAD_INT(&muzz_states[i].sensors[j], fp);

			FREAD_INT(&muzz_states[i].response, fp);
			FREAD_INT(&muzz_states[i].x, fp);
			FREAD_INT(&muzz_states[i].z, fp);
			FREAD_INT(&muzz_states[i].dir, fp);
			FREAD_INT(&j, fp);
			muzz_states[i].forward = (BlockTerrain::Block::DIRECTION)j;
			FREAD_INT(&j, fp);
			muzz_states[i].backward = (BlockTerrain::Block::DIRECTION)j;
			FREAD_INT(&muzz_states[i].lookAtX, fp);
			FREAD_INT(&muzz_states[i].lookAtZ, fp);
			FREAD_INT(&muzz_states[i].moveType, fp);
			FREAD_FLOAT(&muzz_states[i].moveAmount, fp);
		}
	}
	else {
		if (muzz_count == 0)
			current_muzz = -1;
		else
			current_muzz = 0;

		for (i = 0; i < muzz_count; i++) {
			muzz_states[i].response   = Muzz::WAIT;
			muzz_states[i].moveType   = Muzz::FORWARD;
			muzz_states[i].moveAmount = 0.0;
		}
	}

	// Create mushrooms.
	mushrooms.resize(mushroom_count);

	for (i = 0; i < mushroom_count; i++) {
		for (k = 0; k < maxtries; k++) {
			mushrooms[i] = new Mushroom(MushroomColor, Terrain, objectrandomizer);
			ASSERT(mushrooms[i] != NULL);

			if (terrain_type == TMAZE_TERRAIN) {
				j = (int)maze_terrain->tmaze_path.GetCount() - 1;
				mushrooms[i]->Place(maze_terrain->tmaze_path[j].first,
									maze_terrain->tmaze_path[j].second);
			}

			if (fp != NULL) {
				mushrooms[i]->Load(fp);
				break;
			}
			else {
				// Check for overlay.
				mushrooms[i]->GetPosition(p);
				x = (int)(p[0] / terrain.block_size);
				z = (int)(p[2] / terrain.block_size);

				for (j = 0; j < muzz_count; j++) {
					muzzes[j]->GetPosition(p2);
					x2 = (int)(p2[0] / terrain.block_size);
					z2 = (int)(p2[2] / terrain.block_size);

					if ((x == x2) && (z == z2))
						break;
				}

				if (j < muzz_count) {
					delete mushrooms[i];
					mushrooms[i] = NULL;
					continue;
				}

				for (j = 0; j < i; j++) {
					mushrooms[j]->GetPosition(p2);
					x2 = (int)(p2[0] / terrain.block_size);
					z2 = (int)(p2[2] / terrain.block_size);

					if ((x == x2) && (z == z2))
						break;
				}

				if (j < i) {
					delete mushrooms[i];
					mushrooms[i] = NULL;
					continue;
				}
				else
					break;
			}
		}

		if (k == maxtries) {
			fprintf(stderr, "Cannot place mushroom on terrain\n");
			glutDestroyWindow(main_window);
			exit(1);
		}
	}

	// Create water pools.
	pools.resize(pool_count);

	for (i = 0; i < pool_count; i++) {
		for (k = 0; k < maxtries; k++) {
			pools[i] = new Pool(PoolColor, Terrain, objectrandomizer);
			ASSERT(pools[i] != NULL);

			if (fp != NULL) {
				pools[i]->Load(fp);
				break;
			}
			else {
				// Check for overlay.
				// OK to overlay with muzz.
				pools[i]->GetPosition(p);
				x = (int)(p[0] / terrain.block_size);
				z = (int)(p[2] / terrain.block_size);

				for (j = 0; j < mushroom_count; j++) {
					mushrooms[j]->GetPosition(p2);
					x2 = (int)(p2[0] / terrain.block_size);
					z2 = (int)(p2[2] / terrain.block_size);

					if ((x == x2) && (z == z2))
						break;
				}

				if (j < mushroom_count) {
					delete pools[i];
					pools[i] = NULL;
					continue;
				}

				for (j = 0; j < i; j++) {
					pools[j]->GetPosition(p2);
					x2 = (int)(p2[0] / terrain.block_size);
					z2 = (int)(p2[2] / terrain.block_size);

					if ((x == x2) && (z == z2))
						break;
				}

				if (j < i) {
					delete pools[i];
					pools[i] = NULL;
					continue;
				}
				else
					break;
			}
		}

		if (k == maxtries) {
			fprintf(stderr, "Cannot place pool on terrain\n");
			glutDestroyWindow(main_window);
			exit(1);
		}
	}

	delete objectrandomizer;

	// Load misc. items.
	if (fp != NULL) {
		FREAD_INT(&WindowWidth, fp);
		FREAD_INT(&WindowHeight, fp);
		GUIheight = WindowHeight / 8;
		FREAD_INT(&i, fp);
		ViewSelection = (VIEW_SELECTION)i;

		for (i = 0; i < 3; i++)
			FREAD_FLOAT(&TerrainViewPosition[i], fp);

		FREAD_INT(&i, fp);
		Mode = (MODE)i;
		FREAD_BOOL(&terrain_mode, fp);
		FREAD_BOOL(&wire_view, fp);
		FCLOSE(fp);
		fp = NULL;
	}

	if (graphics) {
		// Initialize camera.
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		camera.SetPosition(TerrainViewPosition);
		camera.setPitch(-90.0f);
		// Initialize GUI.
		GLeeInit();
		char* path = getResourcePath((char*)"images/");
		ASSERT(path != NULL);

		if (path == NULL) {
			fprintf(stderr, "Cannot find images\n");
			exit(1);
		}

		MediaPathManager::registerPath(path);
		free(path);
		path = getResourcePath((char*)"GUI/");
		ASSERT(path != NULL);

		if (path == NULL) {
			fprintf(stderr, "Cannot find GUI files\n");
			exit(1);
		}

		MediaPathManager::registerPath(path);
		free(path);
		guiFrame = new GUIFrame();
		ASSERT(guiFrame != NULL);

		if (!guiFrame->GUIPanel::LoadXMLSettings("MuzzGUILayout.xml")) {
			fprintf(stderr, "Cannot load MuzzGUILayout.xml\n");
			exit(1);
		}

		guiFrame->setGUIEventListener(&handler);
		manualCheck = (GUICheckBox*)guiFrame->getWidgetByCallbackString("manual");
		manualCheck->setAlphaFadeScale(1000.0);

		if (!terrain_mode)
			manualCheck->setChecked(true);

		pauseCheck = (GUICheckBox*)guiFrame->getWidgetByCallbackString("pause");
		pauseCheck->setAlphaFadeScale(1000.0);

		if (pause)
			pauseCheck->setChecked(true);

		// Configure viewports.
		glutReshapeWindow(WindowWidth, WindowHeight);
		configureviewports();
	}
}


// Run.
void MuzzWorld::RunMuzzWorld() {
	// Reset muzzes for training?
	if (training_trial_count >= 0) {
		resetTraining();
		resetMuzzWorld();
	}

	cycle_counter = 0;

	if (graphics) {
		// Main loop - does not return.
		frame_rate.Reset();
		glutMainLoop();
	}
	else {
		// Run muzzes.
		while (cycle_counter < cycles) {
			// Return when all muzzes have goals?
			if (run_muzz_world_until_goals_gotten) {
				int i;

				for (i = 0; i < muzz_count; i++) {
					if (muzzes[i] != NULL) {
						if (muzzes[i]->has_food && (muzzes[i]->has_foodCycle == -1))
							muzzes[i]->has_foodCycle = cycle_counter;

						if (muzzes[i]->has_water && (muzzes[i]->has_waterCycle == -1))
							muzzes[i]->has_waterCycle = cycle_counter;
					}
				}

				for (i = 0; i < muzz_count; i++) {
					if (muzzes[i] != NULL) {
						if (!muzzes[i]->has_water || !muzzes[i]->has_food)
							break;
					}
				}

				if (i == muzz_count)
					return;
			}

			// Return when training trials are completed?
			if (current_trial == training_trial_count)
				return;

			// Run a cycle.
			runmuzzes();
			cycle_counter++;
		}
	}
}


// Save muzz world.
bool MuzzWorld::SaveMuzzWorld(char* saveFile) {
	int  i, j;
	Stream& fp;

	if ((fp = FOPEN_WRITE(saveFile)) == NULL)
		return false;

	FWRITE_INT(&muzz_count, fp);
	FWRITE_INT(&mushroom_count, fp);
	FWRITE_INT(&pool_count, fp);
	FWRITE_LONG(&RandomSeed, fp);
	randomizer->RAND_SAVE(fp);
	FWRITE_LONG(&ObjectSeed, fp);
	i = (int)terrain_type;
	FWRITE_INT(&i, fp);
	FWRITE_LONG(&TerrainSeed, fp);
	FWRITE_INT(&terrain_dimension, fp);

	for (i = 0; i < muzz_count; i++)
		muzzes[i]->Store(fp);

	FWRITE_INT(&current_muzz, fp);

	for (i = 0; i < muzz_count; i++) {
		for (j = 0; j < Muzz::NUM_SENSORS; j++)
			FWRITE_INT(&muzz_states[i].sensors[j], fp);

		FWRITE_INT(&muzz_states[i].response, fp);
		FWRITE_INT(&muzz_states[i].x, fp);
		FWRITE_INT(&muzz_states[i].z, fp);
		FWRITE_INT(&muzz_states[i].dir, fp);
		j = (int)muzz_states[i].forward;
		FWRITE_INT(&j, fp);
		j = (int)muzz_states[i].backward;
		FWRITE_INT(&j, fp);
		FWRITE_INT(&muzz_states[i].lookAtX, fp);
		FWRITE_INT(&muzz_states[i].lookAtZ, fp);
		FWRITE_INT(&muzz_states[i].moveType, fp);
		FWRITE_FLOAT(&muzz_states[i].moveAmount, fp);
	}

	for (i = 0; i < mushroom_count; i++)
		mushrooms[i]->Store(fp);

	for (i = 0; i < pool_count; i++)
		pools[i]->Store(fp);

	FWRITE_INT(&WindowWidth, fp);
	FWRITE_INT(&WindowHeight, fp);
	i = (int)ViewSelection;
	FWRITE_INT(&i, fp);

	for (i = 0; i < 3; i++)
		FWRITE_FLOAT(&TerrainViewPosition[i], fp);

	i = (int)Mode;
	FWRITE_INT(&i, fp);
	FWRITE_BOOL(&terrain_mode, fp);
	FWRITE_BOOL(&wire_view, fp);
	FCLOSE(fp);
	return true;
}


// Terminate.
void MuzzWorld::TermMuzzWorld() {
	int i;

	// Save?
	if (SaveFile != NULL) {
		if (!saveMuzzWorld(SaveFile)) {
			fprintf(stderr, "Cannot save to file %s\n", SaveFile);
			glutDestroyWindow(main_window);
			exit(1);
		}
	}

	// Release storage.
	for (i = 0; i < muzz_count; i++) {
		delete muzzes[i];
		muzzes[i] = NULL;
	}

	for (i = 0; i < mushroom_count; i++) {
		if (mushrooms[i] != NULL) {
			delete mushrooms[i];
			mushrooms[i] = NULL;
		}
	}

	for (i = 0; i < pool_count; i++) {
		if (pools[i] != NULL) {
			delete pools[i];
			pools[i] = NULL;
		}
	}

	if (maze_terrain != NULL) {
		delete maze_terrain;
		maze_terrain = NULL;
		Terrain     = NULL;
	}

	if (Terrain != NULL) {
		delete Terrain;
		Terrain = NULL;
	}

	if (randomizer != NULL) {
		delete randomizer;
		randomizer = NULL;
	}

	if (guiFrame != NULL) {
		delete guiFrame;
		guiFrame = NULL;
	}

	forced_response_sequence.Clear();
}



// Get response sequence to obtain food and water.
// Breadth search is optimal, depth search is fast.
bool MuzzWorld::GetResponseSequenceToGoals(int muzz_index,
										   Vector<struct SensoryResponse>& responseSequence, int SEARCH_TYPE searchType,
										   int max_searchDepth) {
	int  i, j;
	bool ret, expand, unblocked;
	Vector<ResponseSearch*>           closed;
	Vector<ResponseSearch*>           open;
	Vector<ResponseSearch*>::Iterator iter;
	ResponseSearch*                         current, *outer;
	Vector<struct SensoryResponse>           responseList;
	Vector<struct SensoryResponse>::Iterator responseItr;
	Muzz*                   saveMuzz     = muzzes[muzz_index];
	struct MuzzState       saveMuzzState = muzz_states[muzz_index];
	Vector<bool>           savemushrooms;
	struct SensoryResponse experience;
	double                p[3], f[3];
	// To restore muzz ID dispenser after search.
	int id_dispenser = Muzz::id_dispenser;
	// Create search space root.
	current = new ResponseSearch(INVALID_RESPONSE, NULL,
								 muzzes[muzz_index], 0);
	ASSERT(current != NULL);
	muzzes[muzz_index]->GetPosition(p);
	current->x = (int)(p[0] / terrain.block_size);
	current->z = (int)(p[2] / terrain.block_size);
	muzzes[muzz_index]->getForward(f);
	f[1] = 0.0;
	cSpacial::normalize(f);

	if (fabs(f[0]) > fabs(f[2])) {
		if (f[0] < 0.0f) {
			current->dir      = 3;
			current->forward  = BlockTerrain::Block::WEST;
			current->backward = BlockTerrain::Block::EAST;
		}
		else {
			current->dir      = 1;
			current->forward  = BlockTerrain::Block::EAST;
			current->backward = BlockTerrain::Block::WEST;
		}
	}
	else {
		if (f[2] < 0.0f) {
			current->dir      = 0;
			current->forward  = BlockTerrain::Block::NORTH;
			current->backward = BlockTerrain::Block::SOUTH;
		}
		else {
			current->dir      = 2;
			current->forward  = BlockTerrain::Block::SOUTH;
			current->backward = BlockTerrain::Block::NORTH;
		}
	}

	current->moveType   = muzz_states[muzz_index].moveType;
	current->moveAmount = muzz_states[muzz_index].moveAmount;
	savemushrooms.resize(mushroom_count);

	for (i = 0; i < mushroom_count; i++)
		current->mushrooms[i] = savemushrooms[i] = mushrooms[i]->IsAlive();

	unblocked = current->setGoalDist();
	open.Add(current);
	// Search for food and water.
	responseSequence.Clear();
	ret = false;

	while (open.GetCount() > 0 && unblocked) {
		iter     = open.Begin();
		current = *iter;
		open.Remove(iter);

		// Found food and water?
		if (current->muzz->has_food && current->muzz->has_water) {
			// Load response sequence.
			closed.Add(current);
			ret = true;

			while (current->experience.response != INVALID_RESPONSE) {
				current->experience.x   = current->x;
				current->experience.y   = current->z;
				current->experience.dir = current->dir;
				responseList.push_front(current->experience);
				current = current->parent;
			}

			for (responseItr = responseList.Begin();
				 responseItr != responseList.End(); responseItr++)
				responseSequence.Add(*responseItr);

			break;
		}

		// Expand outerren?
		if ((max_searchDepth == -1) || (current->depth < max_searchDepth)) {
			expand = true;

			// Check for duplicate.
			for (iter = closed.Begin(); iter != closed.End(); iter++) {
				if (current->equals(*iter)) {
					expand = false;
					break;
				}
			}
		}
		else
			expand = false;

		closed.Add(current);

		if (expand) {
			// Expand possible responses.
			for (i = 0; i < Muzz::NUM_RESPONSES; i++) {
				if (i == Muzz::WAIT)
					continue;

				outer = new ResponseSearch(i, current, current->muzz,
										   current->depth + 1);
				ASSERT(outer != NULL);
				open.Add(outer);
				muzz_states[muzz_index].moveType   = current->moveType;
				muzz_states[muzz_index].moveAmount = current->moveAmount;

				for (j = 0; j < mushroom_count; j++)
					mushrooms[j]->SetAlive(current->mushrooms[j]);

				muzzes[muzz_index] = outer->muzz;
				moveMuzz(muzz_index);
				senseMuzz(muzz_index);
				runMuzz(muzz_index, i);

				for (j = 0; j < saveMuzz->brain->sensor_count; j++)
					outer->experience.sensors.Add(muzz_states[muzz_index].sensors[j]);

				outer->x          = muzz_states[muzz_index].x;
				outer->z          = muzz_states[muzz_index].z;
				outer->dir        = muzz_states[muzz_index].dir;
				outer->forward    = muzz_states[muzz_index].forward;
				outer->backward   = muzz_states[muzz_index].backward;
				outer->moveType   = muzz_states[muzz_index].moveType;
				outer->moveAmount = muzz_states[muzz_index].moveAmount;

				for (j = 0; j < mushroom_count; j++)
					outer->mushrooms[j] = mushrooms[j]->IsAlive();

				unblocked = outer->setGoalDist();
			}

			if (searchType == DEPTH_SEARCH) {
				// Sort open list in ascending order of goal distance.
				open.sort(ltcmpSearch);
			}
		}
	}

	// Clean up.
	for (iter = closed.Begin(); iter != closed.End(); iter++)
		delete *iter;

	closed.Clear();

	for (iter = open.Begin(); iter != open.End(); iter++)
		delete *iter;

	open.Clear();
	muzzes[muzz_index]     = saveMuzz;
	muzz_states[muzz_index] = saveMuzzState;

	for (i = 0; i < mushroom_count; i++)
		mushrooms[i]->SetAlive(savemushrooms[i]);

	// Pad response sequence with a "no-op".
	if (ret) {
		experience.response = Muzz::WAIT;
		responseSequence.Add(experience);
	}

	// Restore muzz ID dispenser after search.
	Muzz::id_dispenser = id_dispenser;
	return (ret);
}
