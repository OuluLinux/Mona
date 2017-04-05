#include "EvolveMouse.h"

/*

// Usage.
char* Usage[] = {
	(char*)"To run:",
	(char*)"  evolve_mouse",
	(char*)"      [-generations <evolution generations>]",
	(char*)"      [-numMazeTests (number of maze tests)]",
	(char*)"      [-numDoorTrainingTrials (number of door association training trials)]",
	(char*)"      [-numMazeTrainingTrials (number of training trials per maze test)]",
	(char*)"      [-mutation_rate <mutation rate>]",
	(char*)"      [-random_seed <random seed> (for new run)]",
	(char*)"      [-print (print mouse brain parameters)]",
	(char*)"      [-input <evolution input file name>]",
	(char*)"      [-output <evolution output file name>]",
	(char*)"      [-logfile <log file name>]",
	(char*)"      [-ignoreInterrupts (ignore interrupts)]",
	#ifdef WIN32
	(char*)"      [-attachConsole (attach to console)]",
	#endif
	(char*)"To print version:",
	(char*)"  evolve_mouse",
	(char*)"      [-version (print version)]",
	NULL
};

// Print and log error.
void PrintError(char* buf) {
	fprintf(stderr, "%s\n", buf);

	if (Log::LOGGING_FLAG == LOG_TO_FILE) {
		sprintf(Log::messageBuf, "%s", buf);
		Log::logError();
	}
}


// Print and log information.
void printInfo(char* buf) {
	fprintf(stderr, "%s\n", buf);

	if (Log::LOGGING_FLAG == LOG_TO_FILE) {
		sprintf(Log::messageBuf, "%s", buf);
		Log::logInformation();
	}
}


// Print usage.
void printUsage() {
	for (int i = 0; Usage[i] != NULL; i++)
		printInfo(Usage[i]);
}*/


// Evolution generation.
int generation = 0;

// generation_count to run.
int generation_count = -1;

// Number of maze tests.
int maze_test_count = DEFAULT_NUM_MAZE_TESTS;

// Number of door association training trials.
int door_training_trial_count = DEFAULT_NUM_DOOR_TRAINING_TRIALS;

// Number of maze training trials.
int maze_training_trial_count = DEFAULT_NUM_MAZE_TRAINING_TRIALS;

// Are maze room doors all visible?
// Some of these doors are blocked.
bool see_all_maze_doors = true;

// Mutation rate.
double mutation_rate = DEFAULT_MUTATION_RATE;

// Random numbers.
int RandomSeed  = INVALID_RANDOM;
Random* randomizer = NULL;

// Print mouse brain parameters.
bool Print = false;

// Population file names.
char* input_file_name  = NULL;
char* output_file_name = NULL;

// Room types.
#define START_ROOM     0
#define BEGIN_MAZE     1
#define MAZE_ROOM      2
#define END_MAZE       3
#define GOAL_ROOM      4
#define DEAD_ROOM      5

// Responses.
#define TAKE_DOOR_0    0
#define TAKE_DOOR_1    1
#define TAKE_DOOR_2    2
#define WAIT           3
#define HOP            4

// Cheese need and goal.
#define CHEESE_NEED    1.0
#define CHEESE_GOAL    0.5

// Maze-learning task.
int door_associations[3];
Vector<Vector<int> > maze_paths;

// Maze room.
/*class Room {
public:

	int            type;
	int            cx;
	int            cy;
	bool           has_cheese;
	Vector<Room*> doors;

	Room(int type, int cx, int cy) {
		this->type = type;
		this->cx   = cx;
		this->cy   = cy;
	}


	// Set room doors to other rooms.
	void SetDoors(Room* left, Room* fwd, Room* right) {
		doors.SetCount(0);

		if (left)	doors.Add(left);

		if (right)	doors.Add(right);

		if (fwd)	doors.Add(fwd);
	}
};*/

// Maze.
Vector<Room*> maze[7];
int            begin_maze_index;
int            end_maze_index;

// Mouse position.
int mouseX;
int mouseY;

// Build maze.
void buildMaze() {
	// Build maze.
	int x;
	int y;
	maze_size		= Size(WIDTH, HEIGHT);
	room_size		= Size(maze_size.cx / 15, maze_size.cy / 11);
	maze.SetCount(7);
	maze[0].SetCount(3);
	maze[1].SetCount(1);
	maze[2].SetCount(3);
	maze[3].SetCount(5);
	maze[4].SetCount(3);
	maze[5].SetCount(1);
	maze[6].SetCount(3);
	x              = room_size.cx;
	y              = (maze_size.cy / 2) - (room_size.cy / 2);
	maze[0][1]     .Init(x, y, START_ROOM, 0, 1);
	maze[0][0]     .Init(x, y - (room_size.cy * 2), START_ROOM, 0, 0);
	maze[0][2]     .Init(x, y + (room_size.cy * 2), START_ROOM, 0, 2);
	begin_maze_index = 1;
	x             += (room_size.cx * 2);
	y              = (maze_size.cy / 2) - (room_size.cy / 2);
	maze[1][0]     .Init(x, y, BEGIN_MAZE, 1, 0);
	maze[0][0].SetDoors(NULL, NULL, &maze[1][0]);
	maze[0][1].SetDoors(NULL, &maze[1][0], NULL);
	maze[0][2].SetDoors(&maze[1][0], NULL, NULL);
	x         += (room_size.cx * 2);
	y          = (maze_size.cy / 2) - (room_size.cy / 2);
	maze[2][1] .Init(x, y, MAZE_ROOM, 2, 1);
	maze[2][0] .Init(x, y - (room_size.cy * 2), MAZE_ROOM, 2, 0);
	maze[2][2] .Init(x, y + (room_size.cy * 2), MAZE_ROOM, 2, 2);
	maze[1][0].SetDoors(&maze[2][0], &maze[2][1], &maze[2][2]);
	x         += (room_size.cx * 2);
	y          = (maze_size.cy / 2) - (room_size.cy / 2);
	maze[3][2] .Init(x, y, MAZE_ROOM, 3, 2);
	maze[3][1] .Init(x, y - (room_size.cy * 2), MAZE_ROOM, 3, 1);
	maze[3][0] .Init(x, y - (room_size.cy * 4), MAZE_ROOM, 3, 0);
	maze[3][3] .Init(x, y + (room_size.cy * 2), MAZE_ROOM, 3, 3);
	maze[3][4] .Init(x, y + (room_size.cy * 4), MAZE_ROOM, 3, 4);
	maze[2][0].SetDoors(&maze[3][0], &maze[3][1], &maze[3][2]);
	maze[2][1].SetDoors(&maze[3][1], &maze[3][2], &maze[3][3]);
	maze[2][2].SetDoors(&maze[3][2], &maze[3][3], &maze[3][4]);
	x         += (room_size.cx * 2);
	y          = (maze_size.cy / 2) - (room_size.cy / 2);
	maze[4][1] .Init(x, y, MAZE_ROOM, 4, 1);
	maze[4][0] .Init(x, y - (room_size.cy * 2), MAZE_ROOM, 4, 0);
	maze[4][2] .Init(x, y + (room_size.cy * 2), MAZE_ROOM, 4, 2);
	maze[3][0].SetDoors(NULL, NULL, &maze[4][0]);
	maze[3][1].SetDoors(NULL, &maze[4][0], &maze[4][1]);
	maze[3][2].SetDoors(&maze[4][0], &maze[4][1], &maze[4][2]);
	maze[3][3].SetDoors(&maze[4][1], &maze[4][2], NULL);
	maze[3][4].SetDoors(&maze[4][2], NULL, NULL);
	end_maze_index = 5;
	x           += (room_size.cx * 2);
	y            = (maze_size.cy / 2) - (room_size.cy / 2);
	maze[5][0]   .Init(x, y, END_MAZE, 5, 0);
	maze[4][0].SetDoors(NULL, NULL, &maze[5][0]);
	maze[4][1].SetDoors(NULL, &maze[5][0], NULL);
	maze[4][2].SetDoors(&maze[5][0], NULL, NULL);
	x         += (room_size.cx * 2);
	y          = (maze_size.cy / 2) - (room_size.cy / 2);
	maze[6][1] .Init(x, y, GOAL_ROOM, 6, 1);
	maze[6][0] .Init(x, y - (room_size.cy * 2), GOAL_ROOM, 6, 0);
	maze[6][2] .Init(x, y + (room_size.cy * 2), GOAL_ROOM, 6, 2);
	maze[5][0].SetDoors(&maze[6][0], &maze[6][1], &maze[6][2]);
	maze[6][0].SetDoors(NULL, NULL, NULL);
	maze[6][1].SetDoors(NULL, NULL, NULL);
	maze[6][2].SetDoors(NULL, NULL, NULL);
	mouseX = mouseY = 0;
	maze[0][0].has_mouse = true;
	maze[0][0].selected = true;
	maze[end_maze_index + 1][2].has_cheese = true;
	maze[end_maze_index + 1][2].selected  = true;
	
	mouseX = mouseY = 0;
}


// Initialize mouse brain.
void InitBrain(Mona* mouse) {
	Vector<SENSOR> goal_sensors;
	mouse->InitNet(5, HOP + 1, 1, randomizer->RAND());
	// Set a long second effect interval
	// for a higher level mediator.
	mouse->effect_event_intervals[1].SetCount(2);
	mouse->effect_event_interval_weights[1].SetCount(2);
	mouse->effect_event_intervals[1][0]       = 2;
	mouse->effect_event_interval_weights[1][0] = 0.5;
	mouse->effect_event_intervals[1][1]       = 10;
	mouse->effect_event_interval_weights[1][1] = 0.5;
	// Set need and goal for cheese.
	mouse->SetNeed(0, CHEESE_NEED);
	goal_sensors.Add(0.0);
	goal_sensors.Add(0.0);
	goal_sensors.Add(0.0);
	goal_sensors.Add((double)GOAL_ROOM);
	goal_sensors.Add(1.0);
	mouse->homeostats[0]->AddGoal(goal_sensors, 0,
								  Mona::NULL_RESPONSE, CHEESE_GOAL);
}


// Parameter mutator.
class ParmMutator {
public:

	// Probability of random mutation.
	static double RANDOM_MUTATION;

	typedef enum {
		INTEGER_PARM, FLOAT_PARM, DOUBLE_PARM
	}
	PARM_TYPE;
	PARM_TYPE type;
	enum { PARM_NAME_SIZE = 50 };
	char   name[PARM_NAME_SIZE];
	void*   parm;
	double min;
	double max;
	double delta;

	// Constructors.
	ParmMutator() {
		type    = DOUBLE_PARM;
		name[0] = '\0';
		parm    = NULL;
		min     = max = delta = 0.0;
	}


	ParmMutator(PARM_TYPE type, char* name, void* parm,
				double min, double max, double delta) {
		this->type = type;
		strncpy(this->name, name, PARM_NAME_SIZE - 1);
		this->parm  = parm;
		this->min   = min;
		this->max   = max;
		this->delta = delta;
	}


	// Mutate parameter.
	void Mutate() {
		int    i;
		double  f;
		double d;

		if (!randomizer->RAND_CHANCE(mutation_rate))
			return;

		switch (type) {
		case INTEGER_PARM:
			if (randomizer->RAND_CHANCE(RANDOM_MUTATION))
				*(int*)parm = (int)randomizer->RAND_INTERVAL(min, max + 0.99);
			else {
				i = *(int*)parm;

				if (randomizer->RAND_BOOL()) {
					i += (int)delta;

					if (i > (int)max)
						i = (int)max;
				}
				else {
					i -= (int)delta;

					if (i < min)
						i = (int)min;
				}

				*(int*)parm = i;
			}

			break;

		case FLOAT_PARM:
			if (randomizer->RAND_CHANCE(RANDOM_MUTATION))
				*(float*)parm = (float)randomizer->RAND_INTERVAL(min, max);
			else {
				f = *(float*)parm;

				if (randomizer->RAND_BOOL()) {
					f += (float)delta;

					if (f > (float)max)
						f = (float)max;
				}
				else {
					f -= (float)delta;

					if (f < (float)min)
						f = (float)min;
				}

				*(float*)parm = f;
			}

			break;

		case DOUBLE_PARM:
			if (randomizer->RAND_CHANCE(RANDOM_MUTATION))
				*(double*)parm = (double)randomizer->RAND_INTERVAL(min, max);
			else {
				d = *(double*)parm;

				if (randomizer->RAND_BOOL()) {
					d += delta;

					if (d > max)
						d = max;
				}
				else {
					d -= delta;

					if (d < min)
						d = min;
				}

				*(double*)parm = d;
			}

			break;
		}
	}


	// Copy parameter from given one.
	void Copy(ParmMutator* from) {
		switch (type) {
		case INTEGER_PARM:
			*(int*)parm = *(int*)from->parm;
			break;

		case FLOAT_PARM:
			*(float*)parm = *(float*)from->parm;
			break;

		case DOUBLE_PARM:
			*(double*)parm = *(double*)from->parm;
			break;
		}
	}
};
double ParmMutator::RANDOM_MUTATION = 0.1;

// Brain parameter mutator.
class BrainParmMutator {
public:

	// Brain.
	Mona* brain;

	// Parameter mutators.
	Vector<ParmMutator*> param_mutators;

	// Constructor.
	BrainParmMutator(Mona* brain) {
		this->brain = brain;
		// INITIAL_ENABLEMENT.
		ParmMutator* param_mutator =
			new ParmMutator(ParmMutator::DOUBLE_PARM, (char*)"INITIAL_ENABLEMENT",
							(void*)&brain->INITIAL_ENABLEMENT, 0.1, 1.0, 0.1);
		ASSERT(param_mutator != NULL);
		param_mutators.Add(param_mutator);
		// DRIVE_ATTENUATION.
		param_mutator =
			new ParmMutator(ParmMutator::DOUBLE_PARM, (char*)"DRIVE_ATTENUATION",
							(void*)&brain->DRIVE_ATTENUATION, 0.0, 1.0, 0.1);
		ASSERT(param_mutator != NULL);
		param_mutators.Add(param_mutator);
		// LEARNING_DECREASE_VELOCITY.
		param_mutator =
			new ParmMutator(ParmMutator::DOUBLE_PARM, (char*)"LEARNING_DECREASE_VELOCITY",
							(void*)&brain->LEARNING_DECREASE_VELOCITY, 0.1, 0.9, 0.1);
		ASSERT(param_mutator != NULL);
		param_mutators.Add(param_mutator);
		// LEARNING_INCREASE_VELOCITY.
		param_mutator =
			new ParmMutator(ParmMutator::DOUBLE_PARM, (char*)"LEARNING_INCREASE_VELOCITY",
							(void*)&brain->LEARNING_INCREASE_VELOCITY, 0.1, 0.9, 0.1);
		ASSERT(param_mutator != NULL);
		param_mutators.Add(param_mutator);
		// FIRING_STRENGTH_LEARNING_DAMPER.
		param_mutator =
			new ParmMutator(ParmMutator::DOUBLE_PARM, (char*)"FIRING_STRENGTH_LEARNING_DAMPER",
							(void*)&brain->FIRING_STRENGTH_LEARNING_DAMPER, 0.05, 0.9, 0.05);
		ASSERT(param_mutator != NULL);
		param_mutators.Add(param_mutator);
	}


	// Destructor.
	~BrainParmMutator() {
		for (int i = 0; i < param_mutators.GetCount(); i++)
			delete param_mutators[i];

		param_mutators.Clear();
	}


	// Mutate.
	void Mutate() {
		for (int i = 0; i < param_mutators.GetCount(); i++)
			param_mutators[i]->Mutate();

		// Initialize brain with new parameters.
		InitBrain(brain);
	}


	// Copy parameters from given brain.
	void Copy(BrainParmMutator* from) {
		for (int i = 0; i < param_mutators.GetCount(); i++)
			param_mutators[i]->Copy(from->param_mutators[i]);

		// Initialize brain with new parameters.
		InitBrain(brain);
	}


	// Randomly merge parameters from given brains.
	void MindMeld(BrainParmMutator* from1, BrainParmMutator* from2) {
		for (int i = 0; i < param_mutators.GetCount(); i++) {
			if (randomizer->RAND_BOOL())
				param_mutators[i]->Copy(from1->param_mutators[i]);
			else
				param_mutators[i]->Copy(from2->param_mutators[i]);
		}

		// Initialize brain with new parameters.
		InitBrain(brain);
	}
};

// Population member.
class Member {
public:

	static int id_dispenser;

	int    id;
	Mona*   mouse;
	double fitness;
	int    generation;

	// Brain parameter mutator.
	BrainParmMutator* brain_parm_mutator;

	// Constructors.
	Member(int generation = 0) {
		id = id_dispenser;
		id_dispenser++;
		// Create the mouse brain.
		mouse = new Mona();
		ASSERT(mouse != NULL);
		InitBrain(mouse);
		// Create brain parameter mutator.
		brain_parm_mutator = new BrainParmMutator(mouse);
		ASSERT(brain_parm_mutator != NULL);
		brain_parm_mutator->Mutate();
		fitness          = 0.0;
		this->generation = generation;
	}


	// Destructor.
	~Member() {
		if (brain_parm_mutator != NULL)
			delete brain_parm_mutator;

		if (mouse != NULL)
			delete mouse;
	}


	// Evaluate.
	// Fitness is determined number of correct tests.
	void Evaluate() {
		int i, j, k;
		Vector<int> path;
		// Clear brain.
		InitBrain(mouse);

		// Train door associations.
		for (i = 0; i < door_training_trial_count; i++) {
			for (j = 0; j < 3; j++) {
				path.Clear();
				path.Add(2 - j);
				path.Add(HOP);
				path.Add(door_associations[j]);
				path.Add(WAIT);
				RunTrial(0, j, path, true);
			}
		}

		fitness = 0.0;

		for (i = 0; i < maze_test_count; i++) {
			// Train maze path.
			for (j = 0; j < maze_training_trial_count; j++) {
				path.Clear();
				path = maze_paths[i];
				path.Add(WAIT);
				RunTrial(begin_maze_index, 0, path, true);
			}

			// Test maze.
			for (j = 0; j < 3; j++) {
				path.Clear();
				path.Add(2 - j);

				for (k = 0; k < maze_paths[i].GetCount(); k++)
					path.Add(maze_paths[i][k]);

				path.Add(door_associations[j]);
				path.Add(WAIT);

				if (RunTrial(0, j, path, false))
					fitness += 1.0;
			}
		}
	}


	// Run a trial.
	bool RunTrial(int x, int y, Vector<int>& path, bool train) {
		int i, j, cx, cy, response;
		Vector<SENSOR> sensors;
		bool                 hopped;
		// Set up sensors.
		sensors.SetCount(5);
		// Clear working memory.
		mouse->ClearWorkingMemory();
		// Reset need.
		mouse->SetNeed(0, CHEESE_NEED);
		// Place mouse.
		mouseX = x;
		mouseY = y;
		// Run maze path.
		hopped = false;

		for (i = 0; i < path.GetCount(); i++) {
			// Override response?
			if (train || (path[i] == WAIT))
				mouse->response_override = path[i];

			// Prepare sensors.
			for (j = 0; j < 3; j++) {
				if (maze[mouseX][mouseY]->doors[j] != NULL)
					sensors[j] = 1.0;
				else if (see_all_maze_doors &&
						 (mouseX > begin_maze_index) && (mouseX < end_maze_index)) {
					// Maze room door is visible although possibly blocked.
					sensors[j] = 1.0;
				}
				else
					sensors[j] = 0.0;
			}

			sensors[3] = (SENSOR)maze[mouseX][mouseY]->type;

			if (mouseX == end_maze_index + 1)
				sensors[4] = 1.0;
			else
				sensors[4] = 0.0;

			// Initiate sensory/response cycle.
			response = mouse->Cycle(sensors);
			// Clear override.
			mouse->response_override = Mona::NULL_RESPONSE;

			// Successful trial?
			if (path[i] == WAIT)
				return true;

			// Hop at start of maze is OK.
			if ((mouseX == begin_maze_index) && (response == HOP) &&
				(path[i] != HOP) && !hopped) {
				hopped = true;
				i--;
				continue;
			}

			// Wrong response ends trial unsuccessfully.
			if (response != path[i])
				return false;

			// Move mouse.
			if (response == HOP) {
				mouseX = end_maze_index;
				mouseY = 0;
			}
			else {
				cx     = (maze[mouseX][mouseY]->doors[response])->cx;
				cy     = (maze[mouseX][mouseY]->doors[response])->cy;
				mouseX = cx;
				mouseY = cy;
			}
		}

		return false;
	}


	// Get fitness.
	double getFitness() {
		return (fitness);
	}


	// Create mutated member by varying brain parameters.
	void Mutate(Member* member) {
		brain_parm_mutator->Copy(member->brain_parm_mutator);
		brain_parm_mutator->Mutate();
	}


	// Merge given member brain parameters into this brain.
	void MindMeld(Member* member1, Member* member2) {
		brain_parm_mutator->MindMeld(member1->brain_parm_mutator,
									 member2->brain_parm_mutator);
	}

	void Serialize(Stream& fp) {
		fp % id;
		mouse->Serialize(fp);
		fp % fitness % generation;
	}

	// Print.
	void Print(FILE* out = stdout) {
		fprintf(out, "id=%d, fitness=%f, generation=%d\n",
				id, getFitness(), generation);
		fprintf(out, "mouse brain:\n");
		mouse->PrintParms(out);
	}
};
int Member::id_dispenser = 0;

// Population.
Member* population[POPULATION_SIZE];

// Start/end functions.
void LogParameters();
void Print();
void Load();
void Store();
void LoadPopulation(Stream& fp);
void StorePopulation(Stream& fp);

// Create maze-learning task.
void createTask();

// Evolve functions.
void Evolve(), Evaluate(), Prune(), Mutate(), DoMating();

#ifdef WIN32
	#ifdef _DEBUG
		// For Windows memory checking, set CHECK_MEMORY = 1.
		#define CHECK_MEMORY         0
		#if (CHECK_MEMORY == 1)
			#define MEMORY_CHECK_FILE    "memory.txt"
			#include <crtdbg.h>
		#endif
	#endif
#endif

// Main.
int main(int argc, char* argv[]) {
	int i;
	#if (CHECK_MEMORY == 1)
	{
	#endif
		#ifdef WIN32

		// Attach to parent console?
		if (_isatty(1)) {
			for (i = 1; i < argc; i++) {
				if (strcmp(argv[i], "-attachConsole") == 0)
					break;
			}

			if (i < argc) {
				FreeConsole();

				if (AttachConsole(ATTACH_PARENT_PROCESS)) {
					freopen("CONOUT$", "w", stdout);
					freopen("CONOUT$", "w", stderr);
					freopen("CONIN$", "r", stdin);
				}
			}
		}

		#endif
		// Initialize logging..
		Log::LOGGING_FLAG = LOG_TO_PRINT;

		// Parse arguments.
		for (i = 1; i < argc; i++) {
			if (strcmp(argv[i], "-generations") == 0) {
				i++;

				if (i >= argc) {
					printUsage();
					exit(1);
				}

				generation_count = atoi(argv[i]);

				if (generation_count < 0) {
					printUsage();
					exit(1);
				}

				continue;
			}

			if (strcmp(argv[i], "-numMazeTests") == 0) {
				i++;

				if (i >= argc) {
					printUsage();
					exit(1);
				}

				maze_test_count = atoi(argv[i]);

				if (maze_test_count < 0) {
					PrintError((char*)"Invalid number of maze tests");
					printUsage();
					exit(1);
				}

				continue;
			}

			if (strcmp(argv[i], "-numDoorTrainingTrials") == 0) {
				i++;

				if (i >= argc) {
					printUsage();
					exit(1);
				}

				door_training_trial_count = atoi(argv[i]);

				if (door_training_trial_count < 0) {
					PrintError((char*)"Invalid number of door association training trials");
					printUsage();
					exit(1);
				}

				continue;
			}

			if (strcmp(argv[i], "-numMazeTrainingTrials") == 0) {
				i++;

				if (i >= argc) {
					printUsage();
					exit(1);
				}

				maze_training_trial_count = atoi(argv[i]);

				if (maze_training_trial_count < 0) {
					PrintError((char*)"Invalid number of maze association training trials");
					printUsage();
					exit(1);
				}

				continue;
			}

			if (strcmp(argv[i], "-mutation_rate") == 0) {
				i++;

				if (i >= argc) {
					printUsage();
					exit(1);
				}

				mutation_rate = atof(argv[i]);

				if (mutation_rate < 0.0) {
					printUsage();
					exit(1);
				}

				continue;
			}

			if (strcmp(argv[i], "-random_seed") == 0) {
				i++;

				if (i >= argc) {
					printUsage();
					exit(1);
				}

				RandomSeed = (RANDOM)atoi(argv[i]);
				continue;
			}

			if (strcmp(argv[i], "-print") == 0) {
				Print = true;
				continue;
			}

			if (strcmp(argv[i], "-input") == 0) {
				i++;

				if (i >= argc) {
					printUsage();
					exit(1);
				}

				input_file_name = argv[i];
				continue;
			}

			if (strcmp(argv[i], "-output") == 0) {
				i++;

				if (i >= argc) {
					printUsage();
					exit(1);
				}

				output_file_name = argv[i];
				continue;
			}

			if (strcmp(argv[i], "-logfile") == 0) {
				i++;

				if (i >= argc) {
					printUsage();
					exit(1);
				}

				Log::LOGGING_FLAG = LOG_TO_FILE;
				Log::setlog_file_name(argv[i]);
				continue;
			}

			if (strcmp(argv[i], "-ignoreInterrupts") == 0) {
				signal(SIGINT, SIG_IGN);
				continue;
			}

			#ifdef WIN32

			if (strcmp(argv[i], "-attachConsole") == 0)
				continue;

			#endif

			if (strcmp(argv[i], "-version") == 0) {
				printEvolveVersion();
				Mona::printVersion();
				exit(0);
			}

			printUsage();
			exit(1);
		}

		// Check operation combinations.
		if (generation_count >= 0) {
			if (!Print && (output_file_name == NULL)) {
				PrintError((char*)"No print or file output");
				printUsage();
				exit(1);
			}
		}
		else {
			if (!Print) {
				PrintError((char*)"Must run or print");
				printUsage();
				exit(1);
			}
		}

		// Build the maze.
		buildMaze();

		// Initialize.
		if (input_file_name != NULL) {
			// Load population.
			if (RandomSeed != INVALID_RANDOM) {
				PrintError((char*)"Random seed is loaded from input file");
				printUsage();
				exit(1);
			}

			Load();
		}
		else {
			if (RandomSeed == INVALID_RANDOM)
				RandomSeed = (RANDOM)time(NULL);

			randomizer = new Random(RandomSeed);
			ASSERT(randomizer != NULL);
			randomizer->SRAND(RandomSeed);
			// Create maze-learning task.
			createTask();

			// Create population.
			for (i = 0; i < POPULATION_SIZE; i++) {
				population[i] = new Member(0);
				ASSERT(population[i] != NULL);
			}
		}

		// Log run parameters.
		if (generation_count >= 0) {
			LOG("Initializing evolve:");

			if (generation_count >= 0) {
				sprintf(Log::messageBuf, "generations=%d", generation_count);
				Log::logInformation();
			}

			if (input_file_name != NULL) {
				sprintf(Log::messageBuf, "input=%s", input_file_name);
				Log::logInformation();
			}

			if (output_file_name != NULL) {
				sprintf(Log::messageBuf, "output=%s", output_file_name);
				Log::logInformation();
			}

			LogParameters();
			// Evolution loop.
			LOG("Begin evolve:");

			for (i = 0; i < generation_count; i++, generation++) {
				sprintf(Log::messageBuf, "generation=%d", generation);
				Log::logInformation();
				Evolve();

				// Save population?
				if (output_file_name != NULL) {
					if ((i % SAVE_FREQUENCY) == 0)
						Store();
				}
			}
		}

		// Save population.
		if (output_file_name != NULL)
			Store();

		// Print population.
		if (Print)
			Print();

		// Release memory.
		for (i = 0; i < POPULATION_SIZE; i++) {
			if (population[i] != NULL) {
				delete population[i];
				population[i] = NULL;
			}
		}

		// Close log.
		if (generation_count >= 0)
			LOG("End evolve");

		Log::close();
		#if (CHECK_MEMORY == 1)
	}
	// Check for memory leaks.
	printf("Checking for memory leaks, report in file %s\n",
		   MEMORY_CHECK_FILE);
	HANDLE hFile = CreateFile(
					   MEMORY_CHECK_FILE,
					   GENERIC_WRITE,
					   FILE_SHARE_WRITE,
					   NULL,
					   OPEN_ALWAYS,
					   0,
					   NULL
				   );

	if (hFile == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "Cannot open memory check file %s",
				MEMORY_CHECK_FILE);
		exit(1);
	}

	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, hFile);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR, hFile);

	if (!_CrtDumpMemoryLeaks())
		printf("No memory leaks\n");
	else
		printf("Memory leaks found\n");

	CloseHandle(hFile);
		#endif
	return 0;
}


// Log run parameters.
void LogParameters() {
	LOG("Evolve Parameters:");
	sprintf(Log::messageBuf, "FIT_POPULATION_SIZE = %d", FIT_POPULATION_SIZE);
	Log::logInformation();
	sprintf(Log::messageBuf, "NUM_MUTANTS = %d", NUM_MUTANTS);
	Log::logInformation();
	sprintf(Log::messageBuf, "NUM_OFFSPRING = %d", NUM_OFFSPRING);
	Log::logInformation();
	sprintf(Log::messageBuf, "maze_test_count = %d", maze_test_count);
	Log::logInformation();
	sprintf(Log::messageBuf, "door_training_trial_count = %d", door_training_trial_count);
	Log::logInformation();
	sprintf(Log::messageBuf, "maze_training_trial_count = %d", maze_training_trial_count);
	Log::logInformation();
	sprintf(Log::messageBuf, "mutation_rate = %f", mutation_rate);
	Log::logInformation();
	sprintf(Log::messageBuf, "RandomSeed = %lu", RandomSeed);
	Log::logInformation();
	sprintf(Log::messageBuf, "Door associations: 0->%d, 1->%d, 2->%d",
			door_associations[0], door_associations[1], door_associations[2]);
	Log::logInformation();
	LOG("Maze paths (doors):");

	for (int i = 0; i < maze_paths.GetCount(); i++) {
		sprintf(Log::messageBuf, "%d %d %d %d",
				maze_paths[i][0], maze_paths[i][1], maze_paths[i][2], maze_paths[i][3]);
		Log::logInformation();
	}
}


// Print population.
void Print() {
	if ((Log::LOGGING_FLAG == LOG_TO_FILE) ||
		(Log::LOGGING_FLAG == LOG_TO_BOTH)) {
		if (Log::logfp != NULL) {
			for (int i = 0; i < POPULATION_SIZE; i++)
				population[i]->Print(Log::logfp);

			fflush(Log::logfp);
		}
	}

	if ((Log::LOGGING_FLAG == LOG_TO_PRINT) ||
		(Log::LOGGING_FLAG == LOG_TO_BOTH)) {
		for (int i = 0; i < POPULATION_SIZE; i++)
			population[i]->Print();
	}
}


// Load evolve.
void Load() {
	int  i, j, k, n;
	Stream& fp;
	char buf[200];
	ASSERT(input_file_name != NULL);

	if ((fp = FOPEN_READ(input_file_name)) == NULL) {
		sprintf(buf, "Cannot load population file %s", input_file_name);
		PrintError(buf);
		exit(1);
	}

	FREAD_INT(&Member::id_dispenser, fp);
	FREAD_LONG(&RandomSeed, fp);
	randomizer = new Random(RandomSeed);
	ASSERT(randomizer != NULL);
	randomizer->RAND_LOAD(fp);
	FREAD_INT(&generation, fp);

	for (i = 0; i < 3; i++)
		FREAD_INT(&door_associations[i], fp);

	maze_paths.Clear();
	FREAD_INT(&i, fp);
	maze_paths.SetCount(i);

	for (j = 0; j < i; j++) {
		for (k = 0; k < 4; k++) {
			FREAD_INT(&n, fp);
			maze_paths[j].Add(n);
		}
	}

	LoadPopulation(fp);
	FCLOSE(fp);
}


// Save evolve.
void Store() {
	int  i, j, k, n;
	Stream& fp;
	char buf[200];
	ASSERT(output_file_name != NULL);

	if ((fp = FOPEN_WRITE(output_file_name)) == NULL) {
		sprintf(buf, "Cannot save to population file %s", output_file_name);
		PrintError(buf);
		exit(1);
	}

	FWRITE_INT(&Member::id_dispenser, fp);
	FWRITE_LONG(&RandomSeed, fp);
	randomizer->RAND_SAVE(fp);
	FWRITE_INT(&generation, fp);

	for (i = 0; i < 3; i++)
		FWRITE_INT(&door_associations[i], fp);

	i = (int)maze_paths.GetCount();
	FWRITE_INT(&i, fp);

	for (j = 0; j < i; j++) {
		for (k = 0; k < 4; k++) {
			n = maze_paths[j][k];
			FWRITE_INT(&n, fp);
		}
	}

	StorePopulation(fp);
	FCLOSE(fp);
}


// Load evolution population.
void LoadPopulation(Stream& fp) {
	for (int i = 0; i < POPULATION_SIZE; i++) {
		population[i] = new Member();
		ASSERT(population[i] != NULL);
		population[i]->Load(fp);
	}
}


// Save evolution population.
void StorePopulation(Stream& fp) {
	for (int i = 0; i < POPULATION_SIZE; i++)
		population[i]->Store(fp);
}


// Create maze-learning task.
void createTask() {
	int i, j, k;
	door_associations[0] = randomizer->RAND_CHOICE(3);
	door_associations[1] = door_associations[0];

	while (door_associations[1] == door_associations[0])
		door_associations[1] = randomizer->RAND_CHOICE(3);

	door_associations[2] = door_associations[0];

	while (door_associations[2] == door_associations[0] ||
		   door_associations[2] == door_associations[1])
		door_associations[2] = randomizer->RAND_CHOICE(3);

	maze_paths.Clear();
	maze_paths.SetCount(maze_test_count);

	for (i = 0; i < maze_test_count; i++) {
		while true {
		maze_paths[i].Clear();

			for (j = k = 0; j < 4; j++) {
				maze_paths[i].Add(randomizer->RAND_CHOICE(3));
				k += (maze_paths[i][j] - 1);
			}

			if (k == 0)
				break;
		}
	}
}


// Evolution generation.
void Evolve() {
	// Evaluate member fitness.
	Evaluate();
	// Prune unfit members.
	Prune();
	// Create new members by mutation.
	Mutate();
	// Create new members by mating.
	DoMating();
}


// Evaluate member fitnesses.
void Evaluate() {
	LOG("Evaluate:");

	for (int i = 0; i < POPULATION_SIZE; i++) {
		population[i]->Evaluate();
		sprintf(Log::messageBuf, "  Member=%d, Mouse=%d, Fitness=%f, generation=%d",
				i, population[i]->id, population[i]->getFitness(),
				population[i]->generation);
		Log::logInformation();
	}
}


// Prune unfit members.
void Prune() {
	double max;
	int    i, j, m;
	Member* member;
	Member* fitpopulation[FIT_POPULATION_SIZE];
	LOG("Select:");

	for (i = 0; i < FIT_POPULATION_SIZE; i++) {
		m = -1;

		for (j = 0; j < POPULATION_SIZE; j++) {
			member = population[j];

			if (member == NULL)
				continue;

			if ((m == -1) || (member->getFitness() > max)) {
				m   = j;
				max = member->getFitness();
			}
		}

		member           = population[m];
		population[m]    = NULL;
		fitpopulation[i] = member;
		sprintf(Log::messageBuf, "  Mouse=%d, Fitness=%f, generation=%d",
				member->id, member->getFitness(), member->generation);
		Log::logInformation();
	}

	for (i = 0; i < POPULATION_SIZE; i++) {
		if (population[i] != NULL) {
			delete population[i];
			population[i] = NULL;
		}
	}

	for (i = 0; i < FIT_POPULATION_SIZE; i++)
		population[i] = fitpopulation[i];
}


// Mutate members.
void Mutate() {
	int    i, j;
	Member* member, *mutant;
	LOG("Mutate:");

	for (i = 0; i < NUM_MUTANTS; i++) {
		// Select a fit member to mutate.
		j      = randomizer->RAND_CHOICE(FIT_POPULATION_SIZE);
		member = population[j];
		// Create mutant member.
		mutant = new Member(member->generation + 1);
		ASSERT(mutant != NULL);
		population[FIT_POPULATION_SIZE + i] = mutant;
		sprintf(Log::messageBuf, "  Member=%d, Mouse=%d -> Member=%d, Mouse=%d",
				j, member->id, FIT_POPULATION_SIZE + i,
				mutant->id);
		Log::logInformation();
		// Mutate.
		mutant->Mutate(member);
	}
}


// Produce offspring by "mind melding" parent brain parameters.
void DoMating() {
	int    i, j, k;
	Member* member1, *member2, *offspring;
	LOG("Mate:");

	if (FIT_POPULATION_SIZE < 2)
		return;

	for (i = 0; i < NUM_OFFSPRING; i++) {
		// Select a pair of fit members to mate.
		j       = randomizer->RAND_CHOICE(FIT_POPULATION_SIZE);
		member1 = population[j];

		while ((k = randomizer->RAND_CHOICE(FIT_POPULATION_SIZE)) == j) {
		}

		member2 = population[k];
		// Create offspring.
		offspring = new Member((member1->generation > member2->generation ?
								member1->generation : member2->generation) + 1);
		ASSERT(offspring != NULL);
		population[FIT_POPULATION_SIZE + NUM_MUTANTS + i] = offspring;
		sprintf(Log::messageBuf, "  Members=%d,%d, Mice=%d,%d -> Member=%d, Mouse=%d",
				j, k, member1->id, member2->id,
				FIT_POPULATION_SIZE + NUM_MUTANTS + i, offspring->id);
		Log::logInformation();
		// Combine parent brains into offspring.
		offspring->MindMeld(member1, member2);
	}
}
