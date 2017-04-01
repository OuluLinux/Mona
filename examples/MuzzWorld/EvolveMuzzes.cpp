#if 0

// For conditions of distribution and use, see copyright notice in muzz.hpp

// Evolve muzzes by mutating and recombining brain parameters.

#include "Evolvemuzzes.h"

// Version (SCCS "what" format).
const char* MuzzEvolveVersion = MUZZ_EVOLVE_VERSION;

// Print version.
void
printEvolveVersion(FILE* out = stdout) {
	fprintf(out, "%s\n", &MuzzEvolveVersion[4]);
}


// Usage.
char* Usage2[] = {
	(char*)"To run:",
	(char*)"  evolve_muzzes",
	(char*)"      -generations <evolution generations>",
	(char*)"      [-muzzcycles (muzz cycles per run)]",
	(char*)"      [-initHunger (initial hunger)]",
	(char*)"      [-initThirst (initial thirst)]",
	(char*)"      [-input <evolution input file name> (for run continuation)]",
	(char*)"      -output <evolution output file name>",
	(char*)"      [-mutation_rate <mutation rate>]",
	(char*)"      [-random_seed <random seed> (for new run)]",
	(char*)"      [-objectSeed <object generation seed>]",
	(char*)"      [-terrainSeed <terrain generation seed>]",
	(char*)"      [-terrainSize <terrain size = dimension x dimension> (minimum=2)]",
	(char*)"      [-minTrainingTrials (minimum number of training trials per test trial)]",
	(char*)"      [-maxTrainingTrials (maximum number of training trials per test trial)]",
	(char*)"      [-logfile <log file name>]",
	(char*)"      [-ignoreInterrupts (ignore interrupts)]",
	#ifdef WIN32
	(char*)"      [-attachConsole (attach to console)]",
	#endif
	(char*)"To run an evolved muzz with graphics:",
	(char*)"  evolve_muzzes",
	(char*)"      -muzz <identifier> (? to list available muzzes)",
	(char*)"      [-muzzcycles (number of cycles)]",
	(char*)"      [-initHunger (initial hunger)]",
	(char*)"      [-initThirst (initial thirst)]",
	(char*)"      -input <evolution input file name>",
	(char*)"      [-pause (start paused)]",
	(char*)"To print version:",
	(char*)"  evolve_muzzes",
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
void PrintInfo2(char* buf) {
	fprintf(stderr, "%s\n", buf);

	if (Log::LOGGING_FLAG == LOG_TO_FILE) {
		sprintf(Log::messageBuf, "%s", buf);
		Log::logInformation();
	}
}


// Print usage.
void PrintUsage2() {
	for (int i = 0; Usage2[i] != NULL; i++)
		PrintInfo2(Usage2[i]);
}


// Evolution generation.
int generation = 0;

// generation_count to run.
int generation_count;

// Mutation rate.
double mutation_rate = DEFAULT_MUTATION_RATE;

// Number of training trials.
int min_training_trials = DEFAULT_MIN_TRAINING_TRIALS;
int max_training_trials = DEFAULT_MAX_TRAINING_TRIALS;

// Population file names.
char* input_file_name;
char* output_file_name;

// Save default muzz.
Muzz* SaveMuzz;

// Parameter mutator.
class ParmMutator {
public:

	// Probability of random mutation.
	static double int_MUTATION;

	typedef enum {
		INTEGER_PARM, FLOAT_PARM, DOUBLE_PARM
	}
	PARM_TYPE;
	PARM_TYPE type;
	enum { PARM_NAME_SIZE = 50 };
	char   name[PARM_NAME_SIZE];
	void*  parm;
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
			if (randomizer->RAND_CHANCE(int_MUTATION))
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

					if (i < (int)min)
						i = (int)min;
				}

				*(int*)parm = i;
			}

			break;

		case FLOAT_PARM:
			if (randomizer->RAND_CHANCE(int_MUTATION))
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
			if (randomizer->RAND_CHANCE(int_MUTATION))
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

double ParmMutator::int_MUTATION = 0.1;

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
		// RESPONSE_intNESS.
		param_mutator =
			new ParmMutator(ParmMutator::DOUBLE_PARM, (char*)"RESPONSE_intNESS",
							(void*)&brain->RESPONSE_intNESS, 0.01, 0.2, 0.01);
		ASSERT(param_mutator != NULL);
		param_mutators.Add(param_mutator);
	}


	// Destructor.
	~BrainParmMutator() {
		for (int i = 0; i < (int)param_mutators.GetCount(); i++)
			delete param_mutators[i];

		param_mutators.Clear();
	}


	// Mutate.
	void Mutate() {
		for (int i = 0; i < (int)param_mutators.GetCount(); i++)
			param_mutators[i]->Mutate();

		// Initialize effect event intervals and weights.
		brain->InitEffectEventIntervals();
		brain->InitEffectEventIntervalWeights();
		brain->InitMaxLearningEffectEventIntervals();
		// Initialize brain with new parameters.
		Muzz::InitBrain(brain);
	}


	// Copy parameters from given brain.
	void Copy(BrainParmMutator* from) {
		for (int i = 0; i < (int)param_mutators.GetCount(); i++)
			param_mutators[i]->Copy(from->param_mutators[i]);

		// Initialize effect intervals.
		brain->InitEffectEventIntervals();
		brain->InitEffectEventIntervalWeights();
		brain->InitMaxLearningEffectEventIntervals();
		// Initialize brain with new parameters.
		Muzz::InitBrain(brain);
	}


	// Randomly merge parameters from given brains.
	void MindMeld(BrainParmMutator* from1, BrainParmMutator* from2) {
		for (int i = 0; i < (int)param_mutators.GetCount(); i++) {
			if (randomizer->RAND_BOOL())
				param_mutators[i]->Copy(from1->param_mutators[i]);
			else
				param_mutators[i]->Copy(from2->param_mutators[i]);
		}

		// Initialize effect intervals.
		brain->InitEffectEventIntervals();
		brain->InitEffectEventIntervalWeights();
		brain->InitMaxLearningEffectEventIntervals();
		// Initialize brain with new parameters.
		Muzz::InitBrain(brain);
	}
};

// Population member.
class Member {
public:

	Muzz*   muzz;
	double fitness;
	int    generation;

	// Brain parameter mutator.
	BrainParmMutator* brain_parm_mutator;

	// Muzz placement.
	static double MuzzX, MuzzY, MuzzDir;

	// Constructors.
	Member(int generation = 0) {
		double color[3];

		for (int i = 0; i < 3; i++)
			color[i] = randomizer->RAND_INTERVAL(0.0, 1.0);

		muzz = new Muzz(color, Terrain, randomizer->RAND(), randomizer);
		ASSERT(muzz != NULL);
		muzz->Place(MuzzX, MuzzY, MuzzDir);
		// Create brain parameter mutator.
		brain_parm_mutator = new BrainParmMutator(muzz->brain);
		ASSERT(brain_parm_mutator != NULL);
		brain_parm_mutator->Mutate();
		fitness          = 0.0;
		this->generation = generation;
	}


	// Destructor.
	~Member() {
		if (brain_parm_mutator != NULL)
			delete brain_parm_mutator;

		if (muzz != NULL)
			delete muzz;
	}


	// Evaluate.
	// Fitness is determined by cycle count to goals,
	// so lesser value is fitter.
	void Evaluate(int trainTrials) {
		int foodCycle, waterCycle;
		// Use member muzz.
		muzzes[0] = muzz;

		// Run muzz world.
		if (graphics) {
			// Should not return.
			resetMuzzWorld();
			runMuzzWorld();
			muzzes[0] = NULL;
			return;
		}

		// Training?
		if (trainTrials > 0) {
			forced_response_train = true;
			training_trial_count   = trainTrials;
			runMuzzWorld();
			forced_response_train = false;
			training_trial_count   = -1;
		}

		// Test run.
		resetMuzzWorld();
		training_trial_count            = -1;
		run_muzz_world_until_goals_gotten = true;
		runMuzzWorld();
		run_muzz_world_until_goals_gotten = false;
		foodCycle  = muzzes[0]->has_foodCycle;
		waterCycle = muzzes[0]->has_waterCycle;
		muzzes[0] = NULL;

		if (foodCycle != -1) {
			if (waterCycle != -1) {
				if (foodCycle > waterCycle)
					sprintf(Log::messageBuf, "Found food and water at cycle %d", foodCycle);
				else
					sprintf(Log::messageBuf, "Found food and water at cycle %d", waterCycle);
			}
			else
				sprintf(Log::messageBuf, "Found only food at cycle %d", foodCycle);
		}
		else {
			if (waterCycle != -1)
				sprintf(Log::messageBuf, "Found only water at cycle %d", waterCycle);
			else
				sprintf(Log::messageBuf, "Food and water not found");
		}

		Log::logInformation();

		// Determine fitness.
		if (foodCycle == -1)
			foodCycle = cycles;

		if (waterCycle == -1)
			waterCycle = cycles;

		fitness = (double)max(foodCycle, waterCycle);
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
		muzz->Serialze(fp);
		fp % fitness % generation;
	}



	// Print.
	void Print() {
		printf("fitness=%f, generation=%d\n", getFitness(), generation);
		printf("muzz brain:\n");
		muzz->PrintBrain();
	}
};

// Muzz placement in world.
double Member::MuzzX   = 0.0;
double Member::MuzzY   = 0.0;
double Member::MuzzDir = 0.0;

// Population.
Member* population[POPULATION_SIZE];

// Start/end functions.
void LogParameters();
void LoadPopulation(Stream& fp);
void StorePopulation(Stream& fp);

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
int main2(int argc, char* argv[]) {
	int  i;
	Stream& fp;
	int  muzzID;
	char buf[200];
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
		// Process glut args.
		glutInit(&argc, argv);
		// Parse arguments.
		generation_count   = -1;
		input_file_name = output_file_name = NULL;
		RandomSeed    = INVALID_int;
		muzzID        = -1;
		bool got_terrain_dimension  = false;
		bool gotmin_training_trials = false;
		bool gotmax_training_trials = false;

		for (i = 1; i < argc; i++) {
			if (strcmp(argv[i], "-generations") == 0) {
				i++;

				if (i >= argc) {
					PrintUsage2();
					exit(1);
				}

				generation_count = atoi(argv[i]);

				if (generation_count < 0) {
					PrintUsage2();
					exit(1);
				}

				continue;
			}

			if (strcmp(argv[i], "-muzzcycles") == 0) {
				i++;

				if (i >= argc) {
					PrintUsage2();
					exit(1);
				}

				if ((cycles = atoi(argv[i])) < 0) {
					PrintUsage2();
					exit(1);
				}

				continue;
			}

			if (strcmp(argv[i], "-initHunger") == 0) {
				i++;

				if (i >= argc) {
					PrintUsage2();
					exit(1);
				}

				Muzz::INIT_HUNGER = atof(argv[i]);

				if ((Muzz::INIT_HUNGER < 0.0) || (Muzz::INIT_HUNGER > 1.0)) {
					PrintUsage2();
					exit(1);
				}

				continue;
			}

			if (strcmp(argv[i], "-initThirst") == 0) {
				i++;

				if (i >= argc) {
					PrintUsage2();
					exit(1);
				}

				Muzz::INIT_THIRST = atof(argv[i]);

				if ((Muzz::INIT_THIRST < 0.0) || (Muzz::INIT_THIRST > 1.0)) {
					PrintUsage2();
					exit(1);
				}

				continue;
			}

			if (strcmp(argv[i], "-input") == 0) {
				i++;

				if (i >= argc) {
					PrintUsage2();
					exit(1);
				}

				input_file_name = argv[i];
				continue;
			}

			if (strcmp(argv[i], "-output") == 0) {
				i++;

				if (i >= argc) {
					PrintUsage2();
					exit(1);
				}

				output_file_name = argv[i];
				continue;
			}

			if (strcmp(argv[i], "-mutation_rate") == 0) {
				i++;

				if (i >= argc) {
					PrintUsage2();
					exit(1);
				}

				mutation_rate = atof(argv[i]);

				if (mutation_rate < 0.0) {
					PrintUsage2();
					exit(1);
				}

				continue;
			}

			if (strcmp(argv[i], "-random_seed") == 0) {
				i++;

				if (i >= argc) {
					PrintUsage2();
					exit(1);
				}

				RandomSeed = (int)atoi(argv[i]);
				continue;
			}

			if (strcmp(argv[i], "-objectSeed") == 0) {
				i++;

				if (i >= argc) {
					PrintUsage2();
					exit(1);
				}

				ObjectSeed = (int)atoi(argv[i]);
				continue;
			}

			if (strcmp(argv[i], "-terrainSeed") == 0) {
				i++;

				if (i >= argc) {
					PrintUsage2();
					exit(1);
				}

				TerrainSeed = (int)atoi(argv[i]);
				continue;
			}

			if (strcmp(argv[i], "-terrainSize") == 0) {
				i++;

				if (i >= argc) {
					PrintUsage2();
					exit(1);
				}

				if ((terrain_dimension = atoi(argv[i])) < 2) {
					PrintError((char*)"Invalid terrain dimension");
					PrintUsage2();
					exit(1);
				}

				got_terrain_dimension = true;
				continue;
			}

			if (strcmp(argv[i], "-logfile") == 0) {
				i++;

				if (i >= argc) {
					PrintUsage2();
					exit(1);
				}

				Log::LOGGING_FLAG = LOG_TO_FILE;
				Log::setlog_file_name(argv[i]);
				continue;
			}

			if (strcmp(argv[i], "-muzz") == 0) {
				i++;

				if (i >= argc) {
					PrintUsage2();
					exit(1);
				}

				if (argv[i][0] == '?')
					muzzID = -2;
				else {
					muzzID = atoi(argv[i]);

					if (muzzID < 0) {
						PrintError((char*)"Invalid muzz identifier");
						PrintUsage2();
						exit(1);
					}
				}

				continue;
			}

			if (strcmp(argv[i], "-minTrainingTrials") == 0) {
				i++;

				if (i >= argc) {
					PrintUsage2();
					exit(1);
				}

				min_training_trials = atoi(argv[i]);

				if (min_training_trials < 0) {
					PrintError((char*)"Invalid minimum number of training trials");
					PrintUsage2();
					exit(1);
				}

				gotmin_training_trials = true;
				continue;
			}

			if (strcmp(argv[i], "-maxTrainingTrials") == 0) {
				i++;

				if (i >= argc) {
					PrintUsage2();
					exit(1);
				}

				max_training_trials = atoi(argv[i]);

				if (max_training_trials < 0) {
					PrintError((char*)"Invalid maximum number of training trials");
					PrintUsage2();
					exit(1);
				}

				gotmax_training_trials = true;
				continue;
			}

			if (strcmp(argv[i], "-pause") == 0) {
				pause = true;
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
				printVersion();
				Mona::printVersion();
				exit(0);
			}

			PrintUsage2();
			exit(1);
		}

		if (muzzID == -1) {
			if (generation_count == -1) {
				PrintError((char*)"generation_count option required");
				PrintUsage2();
				exit(1);
			}

			if (output_file_name == NULL) {
				PrintError((char*)"Output file required");
				PrintUsage2();
				exit(1);
			}

			if ((input_file_name != NULL) &&
				((RandomSeed != INVALID_int) ||
				 (ObjectSeed != INVALID_int) ||
				 (TerrainSeed != INVALID_int))) {
				PrintError((char*)"Random seeds are loaded from file");
				PrintUsage2();
				exit(1);
			}

			if ((input_file_name != NULL) && got_terrain_dimension) {
				PrintError((char*)"Terrain dimension is loaded from file");
				PrintUsage2();
				exit(1);
			}

			if (pause) {
				PrintError((char*)"pause option invalid without muzz identifier");
				PrintUsage2();
				exit(1);
			}

			if (gotmin_training_trials && gotmax_training_trials && (min_training_trials > max_training_trials)) {
				PrintError((char*)"Minimum number of training trials cannot exceed maximum");
				PrintUsage2();
				exit(1);
			}

			if (gotmin_training_trials && (min_training_trials > max_training_trials))
				max_training_trials = min_training_trials;

			if (gotmax_training_trials && (min_training_trials > max_training_trials))
				min_training_trials = max_training_trials;
		}
		else {
			if (input_file_name == NULL) {
				PrintError((char*)"Input file required with muzz identifier");
				PrintUsage2();
				exit(1);
			}

			if (output_file_name != NULL) {
				PrintError((char*)"Output file invalid with muzz identifier");
				PrintUsage2();
				exit(1);
			}

			if (generation_count != -1) {
				PrintError((char*)"generation_count option invalid with muzz identifier");
				PrintUsage2();
				exit(1);
			}

			if ((RandomSeed != INVALID_int) ||
				(ObjectSeed != INVALID_int) ||
				(TerrainSeed != INVALID_int)) {
				PrintError((char*)"Random seeds are loaded from file");
				PrintUsage2();
				exit(1);
			}

			if (got_terrain_dimension) {
				PrintError((char*)"Terrain dimension is loaded from file");
				PrintUsage2();
				exit(1);
			}

			if (gotmin_training_trials || gotmax_training_trials) {
				PrintError((char*)"Training trials options invalid with muzz identifier");
				PrintUsage2();
				exit(1);
			}
		}

		// Seed random numbers.
		if (input_file_name != NULL) {
			if ((RandomSeed != INVALID_int) ||
				(ObjectSeed != INVALID_int) ||
				(TerrainSeed != INVALID_int)) {
				PrintError((char*)"Random seeds are loaded from input file");
				PrintUsage2();
				exit(1);
			}

			if (got_terrain_dimension) {
				PrintError((char*)"Terrain dimension is loaded from input file");
				PrintUsage2();
				exit(1);
			}

			if ((fp = FOPEN_READ(input_file_name)) == NULL) {
				sprintf(buf, "Cannot load population file %s", input_file_name);
				PrintError(buf);
				exit(1);
			}

			FREAD_LONG(&RandomSeed, fp);
			randomizer = new Random(RandomSeed);
			ASSERT(randomizer != NULL);
			randomizer->RAND_LOAD(fp);
			FREAD_LONG(&ObjectSeed, fp);
			FREAD_LONG(&TerrainSeed, fp);
			FREAD_INT(&terrain_dimension, fp);
			FREAD_INT(&generation, fp);
		}
		else {
			fp = NULL;

			if (RandomSeed == INVALID_int)
				RandomSeed = (int)time(NULL);

			randomizer = new Random(RandomSeed);
			ASSERT(randomizer != NULL);
			randomizer->SRAND(RandomSeed);

			if (ObjectSeed == INVALID_int)
				ObjectSeed = RandomSeed;

			if (TerrainSeed == INVALID_int)
				TerrainSeed = RandomSeed;
		}

		if (muzzID == -1) {
			LOG("Initializing evolve:");
			sprintf(Log::messageBuf, "generations=%d", generation_count);
			Log::logInformation();

			if (input_file_name != NULL) {
				sprintf(Log::messageBuf, "input=%s", input_file_name);
				Log::logInformation();
			}

			sprintf(Log::messageBuf, "output=%s", output_file_name);
			Log::logInformation();
		}

		// Initialize muzz world.
		if (muzzID == -1) {
			graphics = false;

			if (cycles == -1)
				cycles = default_MUZZ_CYCLES;
		}

		SaveFile   = LoadFile = NULL;
		smooth_move = false;
		initMuzzWorld();
		Member::MuzzX   = muzzes[0]->GetPlaceX();
		Member::MuzzY   = muzzes[0]->GetPlaceY();
		Member::MuzzDir = muzzes[0]->GetPlaceDirection();
		// Remove default muzz since population member will be used.
		SaveMuzz  = muzzes[0];
		muzzes[0] = NULL;

		// Log run parameters.
		if (muzzID == -1)
			LogParameters();

		// Create population.
		if (fp == NULL) {
			for (i = 0; i < POPULATION_SIZE; i++) {
				population[i] = new Member(0);
				ASSERT(population[i] != NULL);
			}
		}
		else {
			// Continue run.
			LoadPopulation(fp);
			FCLOSE(fp);
		}

		// Running a muzz?
		if (muzzID != -1) {
			if (muzzID == -2) {
				sprintf(buf, "Muzz identifiers:");
				printf("%s\n", buf);

				if (Log::LOGGING_FLAG == LOG_TO_FILE) {
					sprintf(Log::messageBuf, "%s", buf);
					Log::logInformation();
				}

				for (i = 0; i < POPULATION_SIZE; i++) {
					sprintf(buf, "%d", population[i]->muzz->id);
					printf("%s\n", buf);

					if (Log::LOGGING_FLAG == LOG_TO_FILE) {
						sprintf(Log::messageBuf, "%s", buf);
						Log::logInformation();
					}
				}
			}
			else {
				for (i = 0; i < POPULATION_SIZE; i++) {
					if ((population[i] != NULL) && (population[i]->muzz->id == muzzID))
						break;
				}

				if (i == POPULATION_SIZE) {
					PrintError((char*)"Invalid muzz identifier");
					exit(1);
				}

				if (cycles == -1)
					smooth_move = true;
				else
					smooth_move = false;

				population[i]->Evaluate(0);
			}
		}
		else {
			// Evolution loop.
			LOG("Begin evolve:");

			for (i = 0; i < generation_count; i++, generation++) {
				sprintf(Log::messageBuf, "generation=%d", generation);
				Log::logInformation();
				Evolve();

				// Save population?
				if ((i % SAVE_FREQUENCY) == 0) {
					if ((fp = FOPEN_WRITE(output_file_name)) == NULL) {
						sprintf(buf, "Cannot save to population file %s", output_file_name);
						PrintError(buf);
						exit(1);
					}

					FWRITE_LONG(&RandomSeed, fp);
					randomizer->RAND_SAVE(fp);
					FWRITE_LONG(&ObjectSeed, fp);
					FWRITE_LONG(&TerrainSeed, fp);
					FWRITE_INT(&terrain_dimension, fp);
					FWRITE_INT(&generation, fp);
					StorePopulation(fp);
					FCLOSE(fp);
				}
			}

			// Save population.
			if ((fp = FOPEN_WRITE(output_file_name)) == NULL) {
				sprintf(buf, "Cannot save to population file %s", output_file_name);
				PrintError(buf);
				exit(1);
			}

			FWRITE_LONG(&RandomSeed, fp);
			randomizer->RAND_SAVE(fp);
			FWRITE_LONG(&ObjectSeed, fp);
			FWRITE_LONG(&TerrainSeed, fp);
			FWRITE_INT(&terrain_dimension, fp);
			FWRITE_INT(&generation, fp);
			StorePopulation(fp);
			FCLOSE(fp);
		}

		// Release memory.
		for (i = 0; i < POPULATION_SIZE; i++) {
			if (population[i] != NULL) {
				delete population[i];
				population[i] = NULL;
			}
		}

		muzzes[0] = SaveMuzz;
		termMuzzWorld();

		// Close log.
		if (muzzID == -1)
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
	return (0);
}


// Log run parameters.
void LogParameters() {
	LOG("Evolve Parameters:");
	sprintf(Log::messageBuf, "RandomSeed = %lu", RandomSeed);
	Log::logInformation();
	sprintf(Log::messageBuf, "ObjectSeed = %lu", ObjectSeed);
	Log::logInformation();
	sprintf(Log::messageBuf, "TerrainSeed = %lu", TerrainSeed);
	Log::logInformation();
	sprintf(Log::messageBuf, "FIT_POPULATION_SIZE = %d", FIT_POPULATION_SIZE);
	Log::logInformation();
	sprintf(Log::messageBuf, "NUM_MUTANTS = %d", NUM_MUTANTS);
	Log::logInformation();
	sprintf(Log::messageBuf, "NUM_OFFSPRING = %d", NUM_OFFSPRING);
	Log::logInformation();
	sprintf(Log::messageBuf, "mutation_rate = %f", mutation_rate);
	Log::logInformation();
	sprintf(Log::messageBuf, "MUZZ_CYCLES = %d", cycles);
	Log::logInformation();
	LOG("Muzz Parameters:");
	sprintf(Log::messageBuf, "INIT_HUNGER = %f", Muzz::INIT_HUNGER);
	Log::logInformation();
	sprintf(Log::messageBuf, "INIT_THIRST = %f", Muzz::INIT_THIRST);
	Log::logInformation();
	sprintf(Log::messageBuf, "EAT_GOAL_VALUE = %f", Muzz::EAT_GOAL_VALUE);
	Log::logInformation();
	sprintf(Log::messageBuf, "DRINK_GOAL_VALUE = %f", Muzz::DRINK_GOAL_VALUE);
	Log::logInformation();
	sprintf(Log::messageBuf, "min_training_trials = %d", min_training_trials);
	Log::logInformation();
	sprintf(Log::messageBuf, "max_training_trials = %d", max_training_trials);
	Log::logInformation();
	LOG("Terrain Parameters:");
	sprintf(Log::messageBuf, "width = %d", terrain.width);
	Log::logInformation();
	sprintf(Log::messageBuf, "height = %d", terrain.height);
	Log::logInformation();
	sprintf(Log::messageBuf, "max_platform_elevation = %d", terrain.max_platform_elevation);
	Log::logInformation();
	sprintf(Log::messageBuf, "min_platform_size = %d", terrain.min_platform_size);
	Log::logInformation();
	sprintf(Log::messageBuf, "max_platform_size = %d", terrain.max_platform_size);
	Log::logInformation();
	sprintf(Log::messageBuf, "max_platform_generations = %d", terrain.max_platform_generations);
	Log::logInformation();
	sprintf(Log::messageBuf, "extra_ramps = %d", terrain.extra_ramps);
	Log::logInformation();
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
	int i, j;
	LOG("Evaluate:");

	for (i = 0; i < POPULATION_SIZE; i++) {
		Muzz::InitBrain(population[i]->muzz->brain);
		// Evaluate.
		j = randomizer->RAND_CHOICE(max_training_trials - min_training_trials + 1) + min_training_trials;
		population[i]->Evaluate(j);
		sprintf(Log::messageBuf, "  Member=%d, Muzz=%d, Fitness=%f, generation=%d",
				i, population[i]->muzz->id, population[i]->getFitness(), population[i]->generation);
		Log::logInformation();
	}
}


// Prune unfit members.
void Prune() {
	double min;
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

			if ((m == -1) || (member->getFitness() < min)) {
				m   = j;
				min = member->getFitness();
			}
		}

		member           = population[m];
		population[m]    = NULL;
		fitpopulation[i] = member;
		sprintf(Log::messageBuf, "  Muzz=%d, Fitness=%f, generation=%d",
				member->muzz->id, member->getFitness(), member->generation);
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
		sprintf(Log::messageBuf, "  Member=%d, Muzz=%d -> Member=%d, Muzz=%d",
				j, member->muzz->id, FIT_POPULATION_SIZE + i, mutant->muzz->id);
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
		sprintf(Log::messageBuf, "  Members=%d,%d, muzzes=%d,%d -> Member=%d, Muzz=%d",
				j, k, member1->muzz->id, member2->muzz->id, FIT_POPULATION_SIZE + NUM_MUTANTS + i, offspring->muzz->id);
		Log::logInformation();
		// Combine parent brains into offspring.
		offspring->MindMeld(member1, member2);
	}
}
#endif
