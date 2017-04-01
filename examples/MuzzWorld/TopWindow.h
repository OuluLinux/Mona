#ifndef _MuzzWorld_TopWindow_h_
#define _MuzzWorld_TopWindow_h_

#include "Muzz.h"
#include "TmazeTerrain.h"

#include <CtrlLib/CtrlLib.h>
#include <Docking/Docking.h>
using namespace Upp;

/*

    The Muzz World.

    Muzz creatures interact with a block world.
    Each muzz has a mona neural network for a brain.

*/

/*
    (char *)"Checks:",
    (char *)"  Manual : \"Drive\" selected muzz",
    (char *)"   pause : pause world",
    (char *)"",
    (char *)"Buttons:",
    (char *)"   Reset : Reset world to initial state",
    (char *)"    step : Single-step world",
    (char *)"    Help : Show help",
    (char *)"    Quit : Terminate",
    (char *)"",
    (char *)"Keys:",
    (char *)"    Up arrow : Move forward",
    (char *)"  Left arrow : Turn left",
    (char *)" Right arrow : Turn right",
    (char *)"           e : Eat",
    (char *)"           d : Drink",
    (char *)"           p : Print selected muzz brain",
    (char *)"           s : Save world to file",
    (char *)"                 \"muzz.world\"",
    (char *)"           f : Full screen",

    (char *)"    Up arrow : Scroll camera up",
    (char *)"  Down arrow : Scroll camera down",
    (char *)"  Left arrow : Scroll camera left",
    (char *)" Right arrow : Scroll camera right",
    (char *)"  PgUp arrow : Zoom camera out",
    (char *)"PgDown arrow : Zoom camera in",
    (char *)"  Left mouse : Select/deselect muzz",
    (char *)"               and select view pane",
*/

// Muzz states.
#define MUZZ_FRUSTUM_ANGLE     60.0f
#define MUZZ_FRUSTUM_NEAR      0.01f
#define MUZZ_FRUSTUM_FAR       10.0f
#define MUZZ_TURN_DELTA        5.0f
#define MUZZ_MOVEMENT_DELTA    0.002f

// Run a muzz.
#define INVALID_RESPONSE    (-100)

struct MuzzState {
	int                            sensors[Muzz::NUM_SENSORS];
	int                            response;
	int                            x, z;
	int                            dir;
	BlockTerrain::Block::DIRECTION forward;
	BlockTerrain::Block::DIRECTION backward;
	int                            lookAtX, lookAtZ;
	int                            moveType;
	double                        moveAmount;
};

struct SensoryResponse {
	Vector<int> sensors;
	int         response;
	int         x, y, dir;
};

class MuzzWorld;

// Response search state.
class ResponseSearch {
public:
	struct SensoryResponse experience;
	
	MuzzWorld* world;
	
	ResponseSearch*         parent;
	int x, z, dir;
	BlockTerrain::Block::DIRECTION forward, backward;
	int          moveType;
	double      moveAmount;
	Muzz*         muzz;
	Vector<bool> mushrooms;
	int          depth;
	int          goalDist;

	// Constructor.
	// "Lobotomize" muzz to save space.
	ResponseSearch(int response, ResponseSearch* parent,
				   Muzz* parentMuzz, int depth);
				   
	// Destructor.
	~ResponseSearch();
	
	// Set city-block distance to closest needed goal.
	// Distance is zero when goals are achieved.
	// Return true if needed goals are available.
	bool SetGoalDist();
	bool Equals(ResponseSearch* r);
	
};

class MuzzWorld : public DockWindow {
	
protected:
	friend class ResponseSearch;
	
	
	int cycles;
	int cycle_counter;

	bool graphics;

	bool smooth_move;

	bool run_muzz_world_until_goals_gotten;
	bool forced_response_train;

	bool manual_mode;

	bool terrain_mode;
	bool wire_view;
	bool pause;
	bool step;
	bool annotate_graph;

	int  manual_response;

	int muzz_count;
	int mushroom_count;
	int pool_count;
	int response_index;

	// Training.
	int training_trial_count;
	int current_trial;
	int current_trial_step;
	int trial_reset_delay;
	int training_trial_resume;

	int frame_rate;
	int fps_counter;

	int current_muzz;
	int main_window;
	int random_seed, object_seed, terrain_seed;

	bool got_muzz_count, got_mushroom_count, got_pool_count, got_terrain_dimension;
	int terrain_type, terrain_dimension;

	String SaveFile, LoadFile;

	BlockTerrain terrain;
	TmazeTerrain maze_terrain;

	enum {STANDARD_TERRAIN = 0, TMAZE_TERRAIN = 1};
	enum {INIT_INDEX = (-1), ERROR_INDEX = (-2)};
	enum {BREADTH_SEARCH = 0, DEPTH_SEARCH = 1};

	Array<Muzz>				muzzes;
	Array<MuzzState>		muzz_states;
	Array<Mushroom>			mushrooms;
	Array<Pool>				pools;
	Array<SensoryResponse>	forced_response_sequence;
	Camera camera;

public:
	typedef MuzzWorld CLASSNAME;
	MuzzWorld();


	virtual void DockInit();

	void Store(ValueMap& map);
	void Load(const ValueMap& map);

	void StartPicking();
	void StopPicking();
	bool MoveMuzz(int muzz_index);
	void SenseMuzz(int muzz_index);
	void SenseMuzzDir(int muzz_index, int direction, double view[3], int& sense_x, int& sense_z, int& terrain, int& object);
	void RunMuzz(int muzz_index, int forcedResponse = INVALID_RESPONSE);
	void PlaceMuzz(int muzz_index, int placeX, int placeY, BlockTerrain::Block::DIRECTION place_dir);
	bool GetResponseSequenceToGoals(int muzz_index, Vector<struct SensoryResponse>& responseSequence, int SEARCH_TYPE, int max_searchDepth = (-1));
	bool SaveMuzzWorld(char* saveFile);
	void Idle();
	void ResetTraining();
	void ResetMuzzWorld();
	void InitMuzzWorld();
	void RunMuzzWorld();

};

#endif
