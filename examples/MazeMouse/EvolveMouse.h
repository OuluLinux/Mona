#ifndef EVOLVE_MOUSE
#define EVOLVE_MOUSE

#include <Mona/Mona.h>
#include <Mona/Log.h>

#include <CtrlLib/CtrlLib.h>
using namespace Upp;

// Evolution parameters.
#define FIT_POPULATION_SIZE                 20
#define NUM_MUTANTS                         10
#define NUM_OFFSPRING                       10
#define POPULATION_SIZE                     (FIT_POPULATION_SIZE + NUM_MUTANTS + NUM_OFFSPRING)
#define DEFAULT_NUM_MAZE_TESTS              1
#define DEFAULT_NUM_DOOR_TRAINING_TRIALS    50
#define DEFAULT_NUM_MAZE_TRAINING_TRIALS    20
#define DEFAULT_MUTATION_RATE               0.25
#define SAVE_FREQUENCY                      10

#endif
