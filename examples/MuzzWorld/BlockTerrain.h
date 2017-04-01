// For conditions of distribution and use, see copyright notice in muzz.hpp

/*

    Block terrain: a grid of blocks piled to various elevations
    that is connected by ramps such that the top of every block
    is reachable.

    A platform is a horizontal rectangle of blocks impressed into
    the terrain. Multiple platforms overlay to produce a terraced
    effect.

    A ramp can connect platforms that have an elevation difference
    of one. It is composed of a pair of inclined blocks, one rising from
    the lower platform and the other sinking from the upper platform to
    form a 30 degree incline.

    A landing is the level block that terminates a ramp. There is a
    landing at the upper and lower ends to ensure continuity with
    a platform.

*/

#ifndef __BLOCK_TERRAIN__
#define __BLOCK_TERRAIN__

#include <MonaCtrl/MonaCtrl.h>
#include <CtrlLib/CtrlLib.h>
using namespace Upp;

// Block terrain.
class BlockTerrain {
public:

	// Block.
	class Block {
	public:

		// Identifier dispenser.
		static int id_dispenser;

		// Block type.
		typedef enum {
			PLATFORM,
			LANDING,
			RAMP
		} TYPE;

		// Direction.
		typedef enum {
			NORTH = 0,
			EAST  = 1,
			SOUTH = 2,
			WEST  = 3,
			NONE  = 4
		} DIRECTION;

		int         id;
		int         elevation;
		TYPE        type;
		DIRECTION   rampDir;
		int         platform;
		int         group;
		Vector<int> texture_indexes;

		Block() {
			id = id_dispenser;
			id_dispenser++;
			elevation = 0;
			type      = PLATFORM;
			rampDir   = NONE;
			platform  = group = -1;
		}
	};

	// Parameters:

	// Random seed.
	enum { default_random_seed = 4517 };
	int random_seed;

	//Random randomizer;

	// Terrain width and height.
	enum { default_width = 4, default_height = 4 };
	int width;
	int height;

	// Maximum platform elevation.
	enum { default_max_platform_elevation = 5 };
	int max_platform_elevation;

	// Minimum/maximum platform sizes.
	enum { default_min_platform_size = 2, default_max_platform_size = 2 };
	int min_platform_size;
	int max_platform_size;

	// Maximum platform generations.
	enum { default_max_platform_generations = 10 };
	int max_platform_generations;

	// Extra ramps.
	enum { default_extra_ramps = 0 };
	int extra_ramps;

	// Block dimension size.
	static const double default_block_size;
	double              block_size;

	// Ramp stripes.
	enum { NUM_RAMP_STRIPES = 10 };

	// Terrain blocks
	Array<Block> blocks;

	//Block** saveBlocks;

	// Constructors.
	BlockTerrain() {
		Init(random_seed, default_width, default_height,
			 default_max_platform_elevation, default_min_platform_size,
			 default_max_platform_size, default_max_platform_generations,
			 default_extra_ramps, default_block_size);
	}


	BlockTerrain(int random_seed) {
		Init(random_seed, default_width, default_height,
			 default_max_platform_elevation, default_min_platform_size,
			 default_max_platform_size, default_max_platform_generations,
			 default_extra_ramps, default_block_size);
	}


	BlockTerrain(int random_seed, int width, int height,
				 int max_elevation, int min_platform_size, int max_platform_size,
				 int max_platform_generations, int extra_ramps, double block_size) {
		Init(random_seed, width, height,
			 max_elevation, min_platform_size, max_platform_size,
			 max_platform_generations, extra_ramps, block_size);
	}


	~BlockTerrain();
	void Print(Stream& s);
	void GetGeometry(double x, double z, double& y, Vector3f& normal);
	bool IsUpperRamp(int ramp_x, int ramp_y);
	void Draw();

protected:

	void Init(int random_seed, int width, int height,
			  int max_elevation, int min_platform_size, int max_platform_size,
			  int max_platform_generations, int extra_ramps, double block_size);

	// Platforms that are connectable by a ramp.
	struct ConnectablePlatforms {
		int platforms[2];
	};

	// Blocks that are connectable by a ramp.
	struct ConnectableBlocks {
		int x[2];
		int y[2];
	};

	void Generate();
	void GenerateTMaze();
	void CreatePlatform();
	void MarkPlatforms();
	void MarkPlatform(int mark, int x, int y);
	bool ConnectPlatforms();
	void MarkGroups();
	void MarkGroup(int mark, int x, int y);
	void GetConnectablePlatforms(Vector<ConnectablePlatforms>& connectable_platforms, bool already_connected = false);
	bool GetConnectableBlocks(int platform1, int platform2, Vector<ConnectableBlocks>& connectable_blocks);
	void ConnectBlocks(int block1x, int block1y, int block2x, int block2y);
	void GetRampInfo(int ramp_x, int ramp_y,
					 int& landing_x, int& landing_y,
					 int& connected_ramp_x, int& connected_ramp_y,
					 int& connected_landing_x, int& connected_landing_y);
	void Build();

	void DrawBlock();
	void DrawRampSurface();
	void DrawLeftRamp();
	void DrawRightRamp();

	// OpenGL displays for drawing.
	/*	 bool   displays_created;
		GLuint blockDisplay;
		GLuint rampSurfaceDisplay;
		GLuint leftRampDisplay;
		GLuint rightRampDisplay;
	*/
	// Terrain height map.
	// Height is defined as the Y dimension on the XZ plane.
	QuadTree* heightmap;

	// Textures.
	enum { NUM_BLOCK_TEXTURES = 27 };
	GLuint block_textures[NUM_BLOCK_TEXTURES];
	bool   is_textures_loaded;
	void LoadTextures();
};
#endif
