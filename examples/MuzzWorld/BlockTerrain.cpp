// For conditions of distribution and use, see copyright notice in muzz.hpp

// Block terrain implementation.

#include "BlockTerrain.h"

// Graphical block size.
const double BlockTerrain::default_block_size = 0.05f;

// Block identifier dispenser.
int BlockTerrain::Block::id_dispenser = 0;

// Initialize terrain.
void BlockTerrain::Init(int random_seed, int width, int height,
						int max_elevation, int min_platform_size, int max_platform_size,
						int max_platform_generations, int extra_ramps, double block_size) {
	ASSERT(width > 0 && height > 0);
	ASSERT(min_platform_size > 0 && max_platform_size >= min_platform_size);
	ASSERT(min_platform_size <= width && min_platform_size <= height);
	ASSERT(max_platform_size <= width && max_platform_size <= height);
	ASSERT(extra_ramps >= 0);
	this->random_seed              = random_seed;
	this->width                    = width;
	this->height                   = height;
	this->max_platform_elevation   = max_elevation;
	this->min_platform_size        = min_platform_size;
	this->max_platform_size        = max_platform_size;
	this->max_platform_generations = max_platform_generations;
	this->extra_ramps              = extra_ramps;
	this->block_size               = block_size;
	randomizer = new Random(this->random_seed);
	ASSERT(randomizer != NULL);
	displays_created = false;
	heightmap       = NULL;
	is_textures_loaded  = false;
	// Generate the terrain.
	Generate();
}


// Generate terrain.
void BlockTerrain::Generate() {
	int i, j, k;
	Vector<struct ConnectablePlatforms> connectable_platforms;
	Vector<ConnectableBlocks>    connectable_blocks;
	// Create blocks.
	blocks = new Block *[width];
	ASSERT(blocks != NULL);

	for (i = 0; i < width; i++) {
		blocks[i] = new Block[height];
		ASSERT(blocks[i] != NULL);
	}

	MarkPlatforms();
	saveBlocks = new Block *[width];
	ASSERT(saveBlocks != NULL);

	for (i = 0; i < width; i++) {
		saveBlocks[i] = new Block[height];
		ASSERT(saveBlocks[i] != NULL);
	}

	// Create and connect block platforms.
	for (k = 0; k < max_platform_generations; k++) {
		// create a platform.
		CreatePlatform();

		// Try to connect platforms with ramps.
		if (!ConnectPlatforms()) {
			for (i = 0; i < width; i++) {
				for (j = 0; j < height; j++)
					blocks[i][j] = saveBlocks[i][j];
			}
		}
	}

	// Add extra ramps.
	for (k = 0; k < extra_ramps; k++) {
		// Get connectable platforms.
		GetConnectablePlatforms(connectable_platforms, true);

		// All possible connections made?
		if (connectable_platforms.GetCount() == 0)
			break;

		// Select a pair of platforms to build a ramp between.
		i = randomizer->RAND_CHOICE((int)connectable_platforms.GetCount());
		GetConnectableBlocks(connectable_platforms[i].platforms[0],
							 connectable_platforms[i].platforms[1], connectable_blocks);
		// Select blocks and build ramp.
		ASSERT(connectable_blocks.GetCount() > 0);
		i = randomizer->RAND_CHOICE((int)connectable_blocks.GetCount());
		ConnectBlocks(connectable_blocks[i].x[0], connectable_blocks[i].y[0],
					  connectable_blocks[i].x[1], connectable_blocks[i].y[1]);
		// Re-mark platforms since new ramp may have partitioned them.
		MarkPlatforms();
	}

	// Build the terrain.
	Build();
}


// Destructor.
BlockTerrain::~BlockTerrain() {
	for (int i = 0; i < width; i++) {
		delete [] blocks[i];
		delete [] saveBlocks[i];
	}

	delete [] blocks;
	delete [] saveBlocks;
	delete randomizer;
	delete heightmap;
	glDeleteTextures(NUM_BLOCK_TEXTURES, block_textures);
	glDeleteLists(blockDisplay, 4);
}


// Create and mark a platform.
void BlockTerrain::CreatePlatform() {
	int i, j, x, y, w, h, e;
	// Determine platform dimensions and position.
	w = randomizer->RAND_CHOICE(max_platform_size - min_platform_size + 1) + min_platform_size;
	h = randomizer->RAND_CHOICE(max_platform_size - min_platform_size + 1) + min_platform_size;
	x = randomizer->RAND_CHOICE(width - w + 1);
	y = randomizer->RAND_CHOICE(height - h + 1);
	e = randomizer->RAND_CHOICE(max_platform_elevation + 1);

	for (i = 0; i < width; i++) {
		for (j = 0; j < height; j++) {
			saveBlocks[i][j]  = blocks[i][j];
			blocks[i][j].type = Block::PLATFORM;
		}
	}

	for (i = x; i < (x + w); i++) {
		for (j = y; j < (y + h); j++)
			blocks[i][j].elevation = e;
	}

	// Mark platforms.
	MarkPlatforms();
}


// Mark platforms.
void BlockTerrain::MarkPlatforms() {
	int i, j, mark;

	for (i = 0; i < width; i++) {
		for (j = 0; j < height; j++)
			blocks[i][j].platform = -1;
	}

	// Mark all platforms.
	mark = 0;

	for (i = 0; i < width; i++) {
		for (j = 0; j < height; j++) {
			if ((blocks[i][j].platform == -1) &&
				((blocks[i][j].type == Block::PLATFORM) || (blocks[i][j].type == Block::LANDING))) {
				// Mark continguous blocks on platform.
				MarkPlatform(mark, i, j);
				mark++;
			}
		}
	}
}


// Recursively mark a platform.
void BlockTerrain::MarkPlatform(int mark, int x, int y) {
	int lx, ly, crx, cry, clx, cly;
	blocks[x][y].platform = mark;

	if (blocks[x][y].type == Block::RAMP)
		return;

	if (x > 0) {
		if ((blocks[x - 1][y].elevation == blocks[x][y].elevation) &&
			(blocks[x - 1][y].platform == -1)) {
			if (blocks[x][y].type == Block::PLATFORM) {
				if (blocks[x - 1][y].type != Block::RAMP)
					MarkPlatform(mark, x - 1, y);
			}
			else {                                   // landing.
				if (blocks[x - 1][y].type == Block::RAMP) {
					getRampInfo(x - 1, y, lx, ly, crx, cry, clx, cly);

					if ((lx == x) && (ly == y))
						MarkPlatform(mark, x - 1, y);
				}
				else
					MarkPlatform(mark, x - 1, y);
			}
		}
	}

	if (y > 0) {
		if ((blocks[x][y - 1].elevation == blocks[x][y].elevation) &&
			(blocks[x][y - 1].platform == -1)) {
			if (blocks[x][y].type == Block::PLATFORM) {
				if (blocks[x][y - 1].type != Block::RAMP)
					MarkPlatform(mark, x, y - 1);
			}
			else {                                   // landing.
				if (blocks[x][y - 1].type == Block::RAMP) {
					getRampInfo(x, y - 1, lx, ly, crx, cry, clx, cly);

					if ((lx == x) && (ly == y))
						MarkPlatform(mark, x, y - 1);
				}
				else
					MarkPlatform(mark, x, y - 1);
			}
		}
	}

	if (x < width - 1) {
		if ((blocks[x + 1][y].elevation == blocks[x][y].elevation) &&
			(blocks[x + 1][y].platform == -1)) {
			if (blocks[x][y].type == Block::PLATFORM) {
				if (blocks[x + 1][y].type != Block::RAMP)
					MarkPlatform(mark, x + 1, y);
			}
			else {                                   // landing.
				if (blocks[x + 1][y].type == Block::RAMP) {
					getRampInfo(x + 1, y, lx, ly, crx, cry, clx, cly);

					if ((lx == x) && (ly == y))
						MarkPlatform(mark, x + 1, y);
				}
				else
					MarkPlatform(mark, x + 1, y);
			}
		}
	}

	if (y < height - 1) {
		if ((blocks[x][y + 1].elevation == blocks[x][y].elevation) &&
			(blocks[x][y + 1].platform == -1)) {
			if (blocks[x][y].type == Block::PLATFORM) {
				if (blocks[x][y + 1].type != Block::RAMP)
					MarkPlatform(mark, x, y + 1);
			}
			else {                                   // landing.
				if (blocks[x][y + 1].type == Block::RAMP) {
					getRampInfo(x, y + 1, lx, ly, crx, cry, clx, cly);

					if ((lx == x) && (ly == y))
						MarkPlatform(mark, x, y + 1);
				}
				else
					MarkPlatform(mark, x, y + 1);
			}
		}
	}
}


// Connect platforms with ramps into a fully connected terrain.
// Return true if full connection achieved.
bool BlockTerrain::ConnectPlatforms() {
	int  i, j, mark;
	bool connected;
	Vector<struct ConnectablePlatforms> connectable_platforms;
	Vector<ConnectableBlocks>    connectable_blocks;

	// Connect all platforms with ramps.
	while true {
		// Get connectable platforms.
		GetConnectablePlatforms(connectable_platforms);

		// All possible connections made?
		if (connectable_platforms.GetCount() == 0) {
			// Check for a single group that signifies connected terrain.
			MarkGroups();
			mark = blocks[0][0].group;

			for (i = 0; i < width; i++) {
				for (j = 0; j < height; j++) {
					if (blocks[i][j].group != mark)
						return false;
				}
			}

			return true;
		}

		// Select a pair of platforms to build a ramp between.
		i = randomizer->RAND_CHOICE((int)connectable_platforms.GetCount());
		GetConnectableBlocks(connectable_platforms[i].platforms[0],
							 connectable_platforms[i].platforms[1], connectable_blocks);
		// Select blocks and build ramp.
		ASSERT(connectable_blocks.GetCount() > 0);
		i = randomizer->RAND_CHOICE((int)connectable_blocks.GetCount());
		ConnectBlocks(connectable_blocks[i].x[0], connectable_blocks[i].y[0],
					  connectable_blocks[i].x[1], connectable_blocks[i].y[1]);
		// Re-mark platforms since new ramp may have partitioned them.
		MarkPlatforms();
		// Check for a single group that signifies connected terrain.
		MarkGroups();
		mark      = blocks[0][0].group;
		connected = true;

		for (i = 0; i < width && connected; i++) {
			for (j = 0; j < height && connected; j++) {
				if (blocks[i][j].group != mark)
					connected = false;
			}
		}

		if (connected)
			return true;
	}
}


// Mark groups.
// Connected platforms belong to the same group.
void BlockTerrain::MarkGroups() {
	int i, j;

	for (i = 0; i < width; i++) {
		for (j = 0; j < height; j++)
			blocks[i][j].group = -1;
	}

	// Mark all groups.
	for (i = 0; i < width; i++) {
		for (j = 0; j < height; j++) {
			if ((blocks[i][j].group == -1) &&
				((blocks[i][j].type == Block::PLATFORM) || (blocks[i][j].type == Block::LANDING))) {
				// Mark connected platforms with group.
				MarkGroup(blocks[i][j].platform, i, j);
			}
		}
	}
}


// Recursively mark a group.
void BlockTerrain::MarkGroup(int mark, int x, int y) {
	int lx, ly, crx, cry, clx, cly;
	blocks[x][y].group = mark;

	// Extend group marking across ramps to other platforms.
	if (blocks[x][y].type == Block::RAMP) {
		// Find connected ramp and mark the landing.
		getRampInfo(x, y, lx, ly, crx, cry, clx, cly);

		if (blocks[clx][cly].group == -1)
			MarkGroup(mark, clx, cly);

		return;
	}

	// Mark the platform.
	if (x > 0) {
		if ((blocks[x - 1][y].platform == blocks[x][y].platform) &&
			(blocks[x - 1][y].group == -1))
			MarkGroup(mark, x - 1, y);
	}

	if (y > 0) {
		if ((blocks[x][y - 1].platform == blocks[x][y].platform) &&
			(blocks[x][y - 1].group == -1))
			MarkGroup(mark, x, y - 1);
	}

	if (x < width - 1) {
		if ((blocks[x + 1][y].platform == blocks[x][y].platform) &&
			(blocks[x + 1][y].group == -1))
			MarkGroup(mark, x + 1, y);
	}

	if (y < height - 1) {
		if ((blocks[x][y + 1].platform == blocks[x][y].platform) &&
			(blocks[x][y + 1].group == -1))
			MarkGroup(mark, x, y + 1);
	}
}


// Get connectable platforms.
// Can specify platforms that are not already connected.
void BlockTerrain::GetConnectablePlatforms(
	Vector<struct ConnectablePlatforms>& connectable_platforms,
	bool                                 already_connected) {
	int i, j, k;
	Vector<int> platforms;
	Vector<ConnectableBlocks> connectable_blocks;
	struct ConnectablePlatforms      platformConnection;

	// List the platforms.
	for (i = 0; i < width; i++) {
		for (j = 0; j < height; j++) {
			for (k = 0; k < (int)platforms.GetCount(); k++) {
				if (platforms[k] == blocks[i][j].platform)
					break;
			}

			if (k == platforms.GetCount())
				platforms.Add(blocks[i][j].platform);
		}
	}

	// Sort by platform number.
	for (i = 0; i < (int)platforms.GetCount(); i++) {
		for (j = i + 1; j < (int)platforms.GetCount(); j++) {
			if (platforms[j] < platforms[i]) {
				k            = platforms[i];
				platforms[i] = platforms[j];
				platforms[j] = k;
			}
		}
	}

	// Find platforms having a connectable border.
	connectable_platforms.Clear();

	for (i = 0; i < (int)platforms.GetCount(); i++) {
		for (j = i + 1; j < (int)platforms.GetCount(); j++) {
			if (GetConnectableBlocks(platforms[i], platforms[j], connectable_blocks) &&
				!already_connected)
				continue;

			if (connectable_blocks.GetCount() > 0) {
				platformConnection.platforms[0] = platforms[i];
				platformConnection.platforms[1] = platforms[j];
				connectable_platforms.Add(platformConnection);
			}
		}
	}
}


// Get connectable blocks for given platforms.
// Return true if already connected.
bool BlockTerrain::GetConnectableBlocks(int platform1, int platform2,
										Vector<ConnectableBlocks>& connectable_blocks) {
	int x, y, lx, ly, crx, cry, clx, cly, e;
	struct ConnectableBlocks blockConnection;
	bool already_connected;
	connectable_blocks.Clear();
	already_connected = false;

	for (x = 0; x < width; x++) {
		for (y = 0; y < height; y++) {
			if (blocks[x][y].platform != platform1)
				continue;

			// Cannot convert a landing into a ramp.
			if (blocks[x][y].type == Block::LANDING)
				continue;

			// Already connected?
			if (blocks[x][y].type == Block::RAMP) {
				getRampInfo(x, y, lx, ly, crx, cry, clx, cly);

				if (blocks[crx][cry].platform == platform2)
					already_connected = true;

				continue;
			}

			// Check bordering blocks.
			if ((x > 1) && (x < width - 1)) {
				if ((blocks[x - 1][y].platform == platform2) &&
					(blocks[x - 1][y].type == Block::PLATFORM) &&
					(blocks[x - 2][y].platform == platform2) &&
					(blocks[x - 2][y].type != Block::RAMP) &&
					(blocks[x + 1][y].platform == platform1) &&
					(blocks[x + 1][y].type != Block::RAMP)) {
					// Check elevation difference.
					e = blocks[x][y].elevation - blocks[x - 1][y].elevation;

					if (e < 0)
						e = -e;

					if (e == 1) {
						blockConnection.x[0] = x;
						blockConnection.y[0] = y;
						blockConnection.x[1] = x - 1;
						blockConnection.y[1] = y;
						connectable_blocks.Add(blockConnection);
					}
				}
			}

			if ((y > 1) && (y < height - 1)) {
				if ((blocks[x][y - 1].platform == platform2) &&
					(blocks[x][y - 1].type == Block::PLATFORM) &&
					(blocks[x][y - 2].platform == platform2) &&
					(blocks[x][y - 2].type != Block::RAMP) &&
					(blocks[x][y + 1].platform == platform1) &&
					(blocks[x][y + 1].type != Block::RAMP)) {
					e = blocks[x][y].elevation - blocks[x][y - 1].elevation;

					if (e < 0)
						e = -e;

					if (e == 1) {
						blockConnection.x[0] = x;
						blockConnection.y[0] = y;
						blockConnection.x[1] = x;
						blockConnection.y[1] = y - 1;
						connectable_blocks.Add(blockConnection);
					}
				}
			}

			if ((x < width - 2) && (x > 0)) {
				if ((blocks[x + 1][y].platform == platform2) &&
					(blocks[x + 1][y].type == Block::PLATFORM) &&
					(blocks[x + 2][y].platform == platform2) &&
					(blocks[x + 2][y].type != Block::RAMP) &&
					(blocks[x - 1][y].platform == platform1) &&
					(blocks[x - 1][y].type != Block::RAMP)) {
					e = blocks[x][y].elevation - blocks[x + 1][y].elevation;

					if (e < 0)
						e = -e;

					if (e == 1) {
						blockConnection.x[0] = x;
						blockConnection.y[0] = y;
						blockConnection.x[1] = x + 1;
						blockConnection.y[1] = y;
						connectable_blocks.Add(blockConnection);
					}
				}
			}

			if ((y < height - 2) && (y > 0)) {
				if ((blocks[x][y + 1].platform == platform2) &&
					(blocks[x][y + 1].type == Block::PLATFORM) &&
					(blocks[x][y + 2].platform == platform2) &&
					(blocks[x][y + 2].type != Block::RAMP) &&
					(blocks[x][y - 1].platform == platform1) &&
					(blocks[x][y - 1].type != Block::RAMP)) {
					e = blocks[x][y].elevation - blocks[x][y + 1].elevation;

					if (e < 0)
						e = -e;

					if (e == 1) {
						blockConnection.x[0] = x;
						blockConnection.y[0] = y;
						blockConnection.x[1] = x;
						blockConnection.y[1] = y + 1;
						connectable_blocks.Add(blockConnection);
					}
				}
			}
		}
	}

	return (already_connected);
}


// Connect blocks with a ramp.
void BlockTerrain::ConnectBlocks(int block1x, int block1y, int block2x, int block2y) {
	if (block1x < block2x) {
		blocks[block1x - 1][block1y].type = Block::LANDING;
		blocks[block1x][block1y].type     = Block::RAMP;
		blocks[block2x + 1][block2y].type = Block::LANDING;
		blocks[block2x][block2y].type     = Block::RAMP;

		if (blocks[block1x][block1y].elevation > blocks[block2x][block2y].elevation) {
			blocks[block1x][block1y].rampDir = Block::WEST;
			blocks[block2x][block2y].rampDir = Block::WEST;
		}
		else {
			blocks[block1x][block1y].rampDir = Block::EAST;
			blocks[block2x][block2y].rampDir = Block::EAST;
		}

		return;
	}

	if (block1y < block2y) {
		blocks[block1x][block1y - 1].type = Block::LANDING;
		blocks[block1x][block1y].type     = Block::RAMP;
		blocks[block2x][block2y + 1].type = Block::LANDING;
		blocks[block2x][block2y].type     = Block::RAMP;

		if (blocks[block1x][block1y].elevation > blocks[block2x][block2y].elevation) {
			blocks[block1x][block1y].rampDir = Block::NORTH;
			blocks[block2x][block2y].rampDir = Block::NORTH;
		}
		else {
			blocks[block1x][block1y].rampDir = Block::SOUTH;
			blocks[block2x][block2y].rampDir = Block::SOUTH;
		}

		return;
	}

	if (block1x > block2x) {
		blocks[block1x + 1][block1y].type = Block::LANDING;
		blocks[block1x][block1y].type     = Block::RAMP;
		blocks[block2x - 1][block2y].type = Block::LANDING;
		blocks[block2x][block2y].type     = Block::RAMP;

		if (blocks[block1x][block1y].elevation > blocks[block2x][block2y].elevation) {
			blocks[block1x][block1y].rampDir = Block::EAST;
			blocks[block2x][block2y].rampDir = Block::EAST;
		}
		else {
			blocks[block1x][block1y].rampDir = Block::WEST;
			blocks[block2x][block2y].rampDir = Block::WEST;
		}

		return;
	}

	if (block1y > block2y) {
		blocks[block1x][block1y + 1].type = Block::LANDING;
		blocks[block1x][block1y].type     = Block::RAMP;
		blocks[block2x][block2y - 1].type = Block::LANDING;
		blocks[block2x][block2y].type     = Block::RAMP;

		if (blocks[block1x][block1y].elevation > blocks[block2x][block2y].elevation) {
			blocks[block1x][block1y].rampDir = Block::SOUTH;
			blocks[block2x][block2y].rampDir = Block::SOUTH;
		}
		else {
			blocks[block1x][block1y].rampDir = Block::NORTH;
			blocks[block2x][block2y].rampDir = Block::NORTH;
		}

		return;
	}
}


// Given a half-ramp location, get remaining ramp coordinates.
void BlockTerrain::getRampInfo(int ramp_x, int ramp_y,
							   int& landing_x, int& landing_y,
							   int& connected_ramp_x, int& connected_ramp_y,
							   int& connected_landing_x, int& connected_landing_y) {
	ASSERT(blocks[ramp_x][ramp_y].type == Block::RAMP);

	switch (blocks[ramp_x][ramp_y].rampDir) {
	case Block::NORTH:
		if (blocks[ramp_x][ramp_y - 1].type == Block::RAMP) {
			landing_x          = ramp_x;
			landing_y          = ramp_y + 1;
			connected_ramp_x    = ramp_x;
			connected_ramp_y    = ramp_y - 1;
			connected_landing_x = ramp_x;
			connected_landing_y = ramp_y - 2;
		}
		else {
			landing_x          = ramp_x;
			landing_y          = ramp_y - 1;
			connected_ramp_x    = ramp_x;
			connected_ramp_y    = ramp_y + 1;
			connected_landing_x = ramp_x;
			connected_landing_y = ramp_y + 2;
		}

		break;

	case Block::SOUTH:
		if (blocks[ramp_x][ramp_y + 1].type == Block::RAMP) {
			landing_x          = ramp_x;
			landing_y          = ramp_y - 1;
			connected_ramp_x    = ramp_x;
			connected_ramp_y    = ramp_y + 1;
			connected_landing_x = ramp_x;
			connected_landing_y = ramp_y + 2;
		}
		else {
			landing_x          = ramp_x;
			landing_y          = ramp_y + 1;
			connected_ramp_x    = ramp_x;
			connected_ramp_y    = ramp_y - 1;
			connected_landing_x = ramp_x;
			connected_landing_y = ramp_y - 2;
		}

		break;

	case Block::EAST:
		if (blocks[ramp_x + 1][ramp_y].type == Block::RAMP) {
			landing_x          = ramp_x - 1;
			landing_y          = ramp_y;
			connected_ramp_x    = ramp_x + 1;
			connected_ramp_y    = ramp_y;
			connected_landing_x = ramp_x + 2;
			connected_landing_y = ramp_y;
		}
		else {
			landing_x          = ramp_x + 1;
			landing_y          = ramp_y;
			connected_ramp_x    = ramp_x - 1;
			connected_ramp_y    = ramp_y;
			connected_landing_x = ramp_x - 2;
			connected_landing_y = ramp_y;
		}

		break;

	case Block::WEST:
		if (blocks[ramp_x - 1][ramp_y].type == Block::RAMP) {
			landing_x          = ramp_x + 1;
			landing_y          = ramp_y;
			connected_ramp_x    = ramp_x - 1;
			connected_ramp_y    = ramp_y;
			connected_landing_x = ramp_x - 2;
			connected_landing_y = ramp_y;
		}
		else {
			landing_x          = ramp_x - 1;
			landing_y          = ramp_y;
			connected_ramp_x    = ramp_x + 1;
			connected_ramp_y    = ramp_y;
			connected_landing_x = ramp_x + 2;
			connected_landing_y = ramp_y;
		}

		break;
	}
}


// Is block an upper half-ramp?
bool BlockTerrain::IsUpperRamp(int ramp_x, int ramp_y) {
	int x, y;

	if (blocks[ramp_x][ramp_y].type != Block::RAMP)
		return false;

	switch (blocks[ramp_x][ramp_y].rampDir) {
	case Block::NORTH:
		x = ramp_x;
		y = ramp_y - 1;
		break;

	case Block::SOUTH:
		x = ramp_x;
		y = ramp_y + 1;
		break;

	case Block::EAST:
		x = ramp_x + 1;
		y = ramp_y;
		break;

	case Block::WEST:
		x = ramp_x - 1;
		y = ramp_y;
		break;
	}

	if (blocks[x][y].type == Block::RAMP)
		return false;
	else
		return true;
}


// Print block terrain.
void BlockTerrain::Print(Stream& s) {
	int  i, j;
	char c;

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			switch (blocks[j][i].type) {
			case Block::PLATFORM:
				c = 'p';
				break;

			case Block::LANDING:
				c = 'l';
				break;

			case Block::RAMP:
				c = 'r';
				break;
			}

			fprintf(out, "%c/%d/%d ", c, blocks[j][i].platform, blocks[j][i].elevation);
		}

		fprintf(out, "\n");
	}
}


// Build the drawable blocks and a heightmap where
// height is defined as the Y dimension on the XZ plane.
void BlockTerrain::Build() {
	int    i, j, k, e;
	Vector vmin, vmax, vertex;
	Bounds bounds;
	Vector<Vector> vertices;
	Poly*           polygon;

	// Build displays.
	if (!displays_created) {
		blockDisplay = glGenLists(4);
		glNewList(blockDisplay, GL_COMPILE);
		DrawBlock();
		glEndList();
		rampSurfaceDisplay = blockDisplay + 1;
		glNewList(rampSurfaceDisplay, GL_COMPILE);
		DrawRampSurface();
		glEndList();
		leftRampDisplay = rampSurfaceDisplay + 1;
		glNewList(leftRampDisplay, GL_COMPILE);
		DrawLeftRamp();
		glEndList();
		rightRampDisplay = leftRampDisplay + 1;
		glNewList(rightRampDisplay, GL_COMPILE);
		DrawRightRamp();
		glEndList();
		displays_created = true;
	}

	// Assign textures to blocks.
	for (i = 0; i < width; i++) {
		for (j = 0; j < height; j++) {
			blocks[i][j].texture_indexes.Clear();
			e = blocks[i][j].elevation;

			if (IsUpperRamp(i, j))
				e--;

			for (k = 0; k <= e; k++)
				blocks[i][j].texture_indexes.Add(randomizer->RAND_CHOICE(NUM_BLOCK_TEXTURES - 1));
		}
	}

	// Get the terrain bounds and create the heightmap.
	vmin.x = -(block_size * 0.1f);
	vmin.y = 0.0;
	vmin.z = -(block_size * 0.1f);
	vmax.x = block_size * width * 1.1f;
	vmax.y = 0.0;
	vmax.z = block_size * height * 1.1f;
	bounds = Bounds(vmin, vmax);

	if (heightmap != NULL)
		delete heightmap;

	heightmap = new QuadTree(bounds);
	ASSERT(heightmap != NULL);

	// Create the terrain surface polygons and insert into heightmap.
	for (i = 0; i < width; i++) {
		for (j = 0; j < height; j++) {
			// Ramp?
			if (blocks[i][j].type == Block::RAMP) {
				if (IsUpperRamp(i, j)) {
					vertices.Clear();

					switch (blocks[i][j].rampDir) {
					case Block::NORTH:
						vertex.x = (double)i * block_size;
						vertex.y = ((double)blocks[i][j].elevation * block_size) + block_size;
						vertex.z = (double)j * block_size;
						vertices.Add(vertex);
						vertex.y -= block_size;
						vertex.z += block_size * 2.0;
						vertices.Add(vertex);
						vertex.x += block_size;
						vertices.Add(vertex);
						vertex.y += block_size;
						vertex.z -= block_size * 2.0;
						vertices.Add(vertex);
						break;

					case Block::SOUTH:
						vertex.x = ((double)i * block_size) + block_size;
						vertex.y = ((double)blocks[i][j].elevation * block_size) + block_size;
						vertex.z = ((double)j * block_size) + block_size;
						vertices.Add(vertex);
						vertex.y -= block_size;
						vertex.z -= block_size * 2.0;
						vertices.Add(vertex);
						vertex.x -= block_size;
						vertices.Add(vertex);
						vertex.y += block_size;
						vertex.z += block_size * 2.0;
						vertices.Add(vertex);
						break;

					case Block::EAST:
						vertex.x = ((double)i * block_size) + block_size;
						vertex.y = ((double)blocks[i][j].elevation * block_size) + block_size;
						vertex.z = (double)j * block_size;
						vertices.Add(vertex);
						vertex.y -= block_size;
						vertex.x -= block_size * 2.0;
						vertices.Add(vertex);
						vertex.z += block_size;
						vertices.Add(vertex);
						vertex.y += block_size;
						vertex.x += block_size * 2.0;
						vertices.Add(vertex);
						break;

					case Block::WEST:
						vertex.x = (double)i * block_size;
						vertex.y = ((double)blocks[i][j].elevation * block_size) + block_size;
						vertex.z = ((double)j * block_size) + block_size;
						vertices.Add(vertex);
						vertex.y -= block_size;
						vertex.x += block_size * 2.0;
						vertices.Add(vertex);
						vertex.z -= block_size;
						vertices.Add(vertex);
						vertex.y += block_size;
						vertex.x -= block_size * 2.0;
						vertices.Add(vertex);
						break;
					}

					polygon = new Poly(vertices);
					ASSERT(polygon != NULL);
					heightmap->Insert(polygon);
				}
			}
			else {
				// Insert block surface.
				vertices.Clear();
				vertex.x = (double)i * block_size;
				vertex.y = ((double)blocks[i][j].elevation * block_size) + block_size;
				vertex.z = (double)j * block_size;
				vertices.Add(vertex);
				vertex.z += block_size;
				vertices.Add(vertex);
				vertex.x += block_size;
				vertices.Add(vertex);
				vertex.z -= block_size;
				vertices.Add(vertex);
				polygon = new Poly(vertices);
				ASSERT(polygon != NULL);
				heightmap->Insert(polygon);
			}
		}
	}

	// Load the textures.
	if (!is_textures_loaded) {
		LoadTextures();
		is_textures_loaded = true;
	}
}


// Load the textures.
void BlockTerrain::LoadTextures() {
	char* path, *image;
	// Load the block textures.
	image = (char*)"images/A.bmp";
	path  = getResourcePath(image);

	if (path == NULL) {
		fprintf(stderr, "Cannot get texture path for %s\n", image);
		exit(1);
	}

	if (!CreateTexture(path, block_textures, 0)) {
		fprintf(stderr, "Cannot load texture for %s\n", path);
		exit(1);
	}

	free(path);
	image = (char*)"images/B.bmp";
	path  = getResourcePath(image);

	if (path == NULL) {
		fprintf(stderr, "Cannot get texture path for %s\n", image);
		exit(1);
	}

	if (!CreateTexture(path, block_textures, 1)) {
		fprintf(stderr, "Cannot load texture for %s\n", path);
		exit(1);
	}

	free(path);
	image = (char*)"images/C.bmp";
	path  = getResourcePath(image);

	if (path == NULL) {
		fprintf(stderr, "Cannot get texture path for %s\n", image);
		exit(1);
	}

	if (!CreateTexture(path, block_textures, 2)) {
		fprintf(stderr, "Cannot load texture for %s\n", path);
		exit(1);
	}

	free(path);
	image = (char*)"images/D.bmp";
	path  = getResourcePath(image);

	if (path == NULL) {
		fprintf(stderr, "Cannot get texture path for %s\n", image);
		exit(1);
	}

	if (!CreateTexture(path, block_textures, 3)) {
		fprintf(stderr, "Cannot load texture for %s\n", path);
		exit(1);
	}

	free(path);
	image = (char*)"images/E.bmp";
	path  = getResourcePath(image);

	if (path == NULL) {
		fprintf(stderr, "Cannot get texture path for %s\n", image);
		exit(1);
	}

	if (!CreateTexture(path, block_textures, 4)) {
		fprintf(stderr, "Cannot load texture for %s\n", path);
		exit(1);
	}

	free(path);
	image = (char*)"images/F.bmp";
	path  = getResourcePath(image);

	if (path == NULL) {
		fprintf(stderr, "Cannot get texture path for %s\n", image);
		exit(1);
	}

	if (!CreateTexture(path, block_textures, 5)) {
		fprintf(stderr, "Cannot load texture for %s\n", path);
		exit(1);
	}

	free(path);
	image = (char*)"images/G.bmp";
	path  = getResourcePath(image);

	if (path == NULL) {
		fprintf(stderr, "Cannot get texture path for %s\n", image);
		exit(1);
	}

	if (!CreateTexture(path, block_textures, 6)) {
		fprintf(stderr, "Cannot load texture for %s\n", path);
		exit(1);
	}

	free(path);
	image = (char*)"images/H.bmp";
	path  = getResourcePath(image);

	if (path == NULL) {
		fprintf(stderr, "Cannot get texture path for %s\n", image);
		exit(1);
	}

	if (!CreateTexture(path, block_textures, 7)) {
		fprintf(stderr, "Cannot load texture for %s\n", path);
		exit(1);
	}

	free(path);
	image = (char*)"images/I.bmp";
	path  = getResourcePath(image);

	if (path == NULL) {
		fprintf(stderr, "Cannot get texture path for %s\n", image);
		exit(1);
	}

	if (!CreateTexture(path, block_textures, 8)) {
		fprintf(stderr, "Cannot load texture for %s\n", path);
		exit(1);
	}

	free(path);
	image = (char*)"images/J.bmp";
	path  = getResourcePath(image);

	if (path == NULL) {
		fprintf(stderr, "Cannot get texture path for %s\n", image);
		exit(1);
	}

	if (!CreateTexture(path, block_textures, 9)) {
		fprintf(stderr, "Cannot load texture for %s\n", path);
		exit(1);
	}

	free(path);
	image = (char*)"images/K.bmp";
	path  = getResourcePath(image);

	if (path == NULL) {
		fprintf(stderr, "Cannot get texture path for %s\n", image);
		exit(1);
	}

	if (!CreateTexture(path, block_textures, 10)) {
		fprintf(stderr, "Cannot load texture for %s\n", path);
		exit(1);
	}

	free(path);
	image = (char*)"images/L.bmp";
	path  = getResourcePath(image);

	if (path == NULL) {
		fprintf(stderr, "Cannot get texture path for %s\n", image);
		exit(1);
	}

	if (!CreateTexture(path, block_textures, 11)) {
		fprintf(stderr, "Cannot load texture for %s\n", path);
		exit(1);
	}

	free(path);
	image = (char*)"images/M.bmp";
	path  = getResourcePath(image);

	if (path == NULL) {
		fprintf(stderr, "Cannot get texture path for %s\n", image);
		exit(1);
	}

	if (!CreateTexture(path, block_textures, 12)) {
		fprintf(stderr, "Cannot load texture for %s\n", path);
		exit(1);
	}

	free(path);
	image = (char*)"images/N.bmp";
	path  = getResourcePath(image);

	if (path == NULL) {
		fprintf(stderr, "Cannot get texture path for %s\n", image);
		exit(1);
	}

	if (!CreateTexture(path, block_textures, 13)) {
		fprintf(stderr, "Cannot load texture for %s\n", path);
		exit(1);
	}

	free(path);
	image = (char*)"images/O.bmp";
	path  = getResourcePath(image);

	if (path == NULL) {
		fprintf(stderr, "Cannot get texture path for %s\n", image);
		exit(1);
	}

	if (!CreateTexture(path, block_textures, 14)) {
		fprintf(stderr, "Cannot load texture for %s\n", path);
		exit(1);
	}

	free(path);
	image = (char*)"images/P.bmp";
	path  = getResourcePath(image);

	if (path == NULL) {
		fprintf(stderr, "Cannot get texture path for %s\n", image);
		exit(1);
	}

	if (!CreateTexture(path, block_textures, 15)) {
		fprintf(stderr, "Cannot load texture for %s\n", path);
		exit(1);
	}

	free(path);
	image = (char*)"images/Q.bmp";
	path  = getResourcePath(image);

	if (path == NULL) {
		fprintf(stderr, "Cannot get texture path for %s\n", image);
		exit(1);
	}

	if (!CreateTexture(path, block_textures, 16)) {
		fprintf(stderr, "Cannot load texture for %s\n", path);
		exit(1);
	}

	free(path);
	image = (char*)"images/R.bmp";
	path  = getResourcePath(image);

	if (path == NULL) {
		fprintf(stderr, "Cannot get texture path for %s\n", image);
		exit(1);
	}

	if (!CreateTexture(path, block_textures, 17)) {
		fprintf(stderr, "Cannot load texture for %s\n", path);
		exit(1);
	}

	free(path);
	image = (char*)"images/S.bmp";
	path  = getResourcePath(image);

	if (path == NULL) {
		fprintf(stderr, "Cannot get texture path for %s\n", image);
		exit(1);
	}

	if (!CreateTexture(path, block_textures, 18)) {
		fprintf(stderr, "Cannot load texture for %s\n", path);
		exit(1);
	}

	free(path);
	image = (char*)"images/T.bmp";
	path  = getResourcePath(image);

	if (path == NULL) {
		fprintf(stderr, "Cannot get texture path for %s\n", image);
		exit(1);
	}

	if (!CreateTexture(path, block_textures, 19)) {
		fprintf(stderr, "Cannot load texture for %s\n", path);
		exit(1);
	}

	free(path);
	image = (char*)"images/U.bmp";
	path  = getResourcePath(image);

	if (path == NULL) {
		fprintf(stderr, "Cannot get texture path for %s\n", image);
		exit(1);
	}

	if (!CreateTexture(path, block_textures, 20)) {
		fprintf(stderr, "Cannot load texture for %s\n", path);
		exit(1);
	}

	free(path);
	image = (char*)"images/V.bmp";
	path  = getResourcePath(image);

	if (path == NULL) {
		fprintf(stderr, "Cannot get texture path for %s\n", image);
		exit(1);
	}

	if (!CreateTexture(path, block_textures, 21)) {
		fprintf(stderr, "Cannot load texture for %s\n", path);
		exit(1);
	}

	free(path);
	image = (char*)"images/W.bmp";
	path  = getResourcePath(image);

	if (path == NULL) {
		fprintf(stderr, "Cannot get texture path for %s\n", image);
		exit(1);
	}

	if (!CreateTexture(path, block_textures, 22)) {
		fprintf(stderr, "Cannot load texture for %s\n", path);
		exit(1);
	}

	free(path);
	image = (char*)"images/X.bmp";
	path  = getResourcePath(image);

	if (path == NULL) {
		fprintf(stderr, "Cannot get texture path for %s\n", image);
		exit(1);
	}

	if (!CreateTexture(path, block_textures, 23)) {
		fprintf(stderr, "Cannot load texture for %s\n", path);
		exit(1);
	}

	free(path);
	image = (char*)"images/Y.bmp";
	path  = getResourcePath(image);

	if (path == NULL) {
		fprintf(stderr, "Cannot get texture path for %s\n", image);
		exit(1);
	}

	if (!CreateTexture(path, block_textures, 24)) {
		fprintf(stderr, "Cannot load texture for %s\n", path);
		exit(1);
	}

	free(path);
	image = (char*)"images/Z.bmp";
	path  = getResourcePath(image);

	if (path == NULL) {
		fprintf(stderr, "Cannot get texture path for %s\n", image);
		exit(1);
	}

	if (!CreateTexture(path, block_textures, 25)) {
		fprintf(stderr, "Cannot load texture for %s\n", path);
		exit(1);
	}

	free(path);
	image = (char*)"images/gray.bmp";
	path  = getResourcePath(image);

	if (path == NULL) {
		fprintf(stderr, "Cannot get texture path for %s\n", image);
		exit(1);
	}

	if (!CreateTexture(path, block_textures, 26)) {
		fprintf(stderr, "Cannot load texture for %s\n", path);
		exit(1);
	}

	free(path);
}


// Get terrain height (Y) and facet normal vector at given XZ coordinates.
void BlockTerrain::GetGeometry(double x, double z, double& y, Vector& normal) {
	Vector<Poly*> polygons;
	// Search for facet polygons at x, y coordinates.
	heightmap->Search(x, z, polygons);

	if (polygons.GetCount() == 0) {
		y        = 0.0;
		normal.x = 0.0;
		normal.y = 1.0;
		normal.z = 0.0;
		return;
	}

	// Assume single polygon match.
	normal = polygons[0]->plane.normal;

	if (normal.y > 0.0f) {
		// Solve plane equation for height.
		y = (-(normal.x * x) - (normal.z * z) - polygons[0]->plane.d) / normal.y;
	}
	else
		y = 0.0;
}


// Draw a block.
void BlockTerrain::DrawBlock() {
	BaseObject::DrawBlock(0.0f, block_size, 0.0f, block_size, 0.0f, block_size);
}


// Draw ramp surface.
void BlockTerrain::DrawRampSurface() {
	int     i;
	double d, h, s, w, x, y, z;
	// Draw a chevron pattern on the ramp surface.
	h = block_size / 2.0;
	w = (block_size * 2.236f * 0.25f) / (double)NUM_RAMP_STRIPES;
	d = ((block_size * 2.236f) - w) / ((double)NUM_RAMP_STRIPES + 1.0f);
	s = (d * 2.0f) / h;

	for (i = 0; i < NUM_RAMP_STRIPES; i++) {
		// Draw black chevron.
		glColor3f(0.0f, 0.0f, 0.0f);
		glBegin(GL_QUADS);
		// Left half of chevron.
		glNormal3f(0.0f, 0.894f, 0.447f);
		x = 0.0;
		y = d * (double)i * 0.447f;
		z = d * (double)i * 0.894f;
		z = (block_size * 2.0f) - z;
		glVertex3f(x, y, z);
		glNormal3f(0.0f, 0.894f, 0.447f);
		x  = h;
		y += s * x * 0.447f;
		z -= s * x * 0.894f;
		glVertex3f(x, y, z);
		glNormal3f(0.0f, 0.894f, 0.447f);
		y += w * 0.447f;
		z -= w * 0.894f;
		glVertex3f(x, y, z);
		glNormal3f(0.0f, 0.894f, 0.447f);
		x = 0.0;
		y = ((d * (double)i) + w) * 0.447f;
		z = ((d * (double)i) + w) * 0.894f;
		z = (block_size * 2.0f) - z;
		glVertex3f(x, y, z);
		// Right half of chevron.
		glNormal3f(0.0f, 0.894f, 0.447f);
		x  = h;
		y  = d * (double)i * 0.447f;
		z  = d * (double)i * 0.894f;
		z  = (block_size * 2.0f) - z;
		y += s * x * 0.447f;
		z -= s * x * 0.894f;
		glVertex3f(x, y, z);
		glNormal3f(0.0f, 0.894f, 0.447f);
		y -= s * x * 0.447f;
		z += s * x * 0.894f;
		x  = block_size;
		glVertex3f(x, y, z);
		glNormal3f(0.0f, 1.0f, 0.0f);
		y += w * 0.447f;
		z -= w * 0.894f;
		glVertex3f(x, y, z);
		glNormal3f(0.0f, 0.894f, 0.447f);
		x  = h;
		y += s * x * 0.447f;
		z -= s * x * 0.894f;
		glVertex3f(x, y, z);
		glEnd();
		// Draw white background.
		glColor3f(1.0f, 1.0f, 1.0f);

		if (i == 0) {
			// Draw bottom white triangle.
			glBegin(GL_TRIANGLES);
			glNormal3f(0.0f, 0.894f, 0.447f);
			x = 0.0;
			y = d * (double)i * 0.447f;
			z = d * (double)i * 0.894f;
			z = (block_size * 2.0f) - z;
			glVertex3f(x, y, z);
			glNormal3f(0.0f, 0.894f, 0.447f);
			x = block_size;
			glVertex3f(x, y, z);
			glNormal3f(0.0f, 0.894f, 0.447f);
			x  = h;
			y += s * x * 0.447f;
			z -= s * x * 0.894f;
			glVertex3f(x, y, z);
			glEnd();
		}
		else {
			// Draw white chevron.
			glBegin(GL_QUADS);
			// Left half of chevron.
			glNormal3f(0.0f, 0.894f, 0.447f);
			x = 0.0;
			y = d * (double)i * 0.447f;
			z = d * (double)i * 0.894f;
			z = (block_size * 2.0f) - z;
			glVertex3f(x, y, z);
			glNormal3f(0.0f, 0.894f, 0.447f);
			x = 0.0;
			y = ((d * (double)(i - 1)) + w) * 0.447f;
			z = (d * (double)(i - 1) + w) * 0.894f;
			z = (block_size * 2.0f) - z;
			glVertex3f(x, y, z);
			glNormal3f(0.0f, 0.894f, 0.447f);
			x  = h;
			y += s * x * 0.447f;
			z -= s * x * 0.894f;
			glVertex3f(x, y, z);
			glNormal3f(0.0f, 0.894f, 0.447f);
			x  = h;
			y  = d * (double)i * 0.447f;
			z  = d * (double)i * 0.894f;
			z  = (block_size * 2.0f) - z;
			y += s * x * 0.447f;
			z -= s * x * 0.894f;
			glVertex3f(x, y, z);
			// Right half of chevron.
			glNormal3f(0.0f, 0.894f, 0.447f);
			x  = h;
			y  = d * (double)i * 0.447f;
			z  = d * (double)i * 0.894f;
			z  = (block_size * 2.0f) - z;
			y += s * x * 0.447f;
			z -= s * x * 0.894f;
			glVertex3f(x, y, z);
			glNormal3f(0.0f, 0.894f, 0.447f);
			x  = h;
			y  = ((d * (double)(i - 1)) + w) * 0.447f;
			z  = ((d * (double)(i - 1)) + w) * 0.894f;
			z  = (block_size * 2.0f) - z;
			y += s * x * 0.447f;
			z -= s * x * 0.894f;
			glVertex3f(x, y, z);
			glNormal3f(0.0f, 1.0f, 0.0f);
			y -= s * x * 0.447f;
			z += s * x * 0.894f;
			x  = block_size;
			glVertex3f(x, y, z);
			glNormal3f(0.0f, 0.894f, 0.447f);
			x = block_size;
			y = d * (double)i * 0.447f;
			z = d * (double)i * 0.894f;
			z = (block_size * 2.0f) - z;
			glVertex3f(x, y, z);
			glEnd();
		}

		if (i == NUM_RAMP_STRIPES - 1) {
			// Draw top triangles.
			glBegin(GL_TRIANGLES);
			glNormal3f(0.0f, 0.894f, 0.447f);
			x = 0.0;
			y = ((d * (double)i) + w) * 0.447f;
			z = ((d * (double)i) + w) * 0.894f;
			z = (block_size * 2.0f) - z;
			glVertex3f(x, y, z);
			glNormal3f(0.0f, 0.894f, 0.447f);
			x  = h;
			y += s * x * 0.447f;
			z -= s * x * 0.894f;
			glVertex3f(x, y, z);
			glNormal3f(0.0f, 0.894f, 0.447f);
			x = 0.0;
			glVertex3f(x, y, z);
			glNormal3f(0.0f, 0.894f, 0.447f);
			x  = h;
			y  = ((d * (double)i) + w) * 0.447f;
			z  = ((d * (double)i) + w) * 0.894f;
			z  = (block_size * 2.0f) - z;
			y += s * x * 0.447f;
			z -= s * x * 0.894f;
			glVertex3f(x, y, z);
			glNormal3f(0.0f, 0.894f, 0.447f);
			y -= s * x * 0.447f;
			z += s * x * 0.894f;
			x  = block_size;
			glVertex3f(x, y, z);
			glNormal3f(0.0f, 0.894f, 0.447f);
			x  = block_size;
			y  = ((d * (double)i) + w) * 0.447f;
			z  = ((d * (double)i) + w) * 0.894f;
			z  = (block_size * 2.0f) - z;
			y += s * h * 0.447f;
			z -= s * h * 0.894f;
			glVertex3f(x, y, z);
			glEnd();
		}
	}

	glColor3f(1.0f, 1.0f, 1.0f);
}


// Draw ramp left side.
void BlockTerrain::DrawLeftRamp() {
	glColor3f(0.5f, 0.5f, 0.5f);
	glBegin(GL_TRIANGLES);
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, block_size * 2.0f);
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, block_size, 0.0f);
	glEnd();
	glColor3f(1.0f, 1.0f, 1.0f);
}


// Draw ramp right side.
void BlockTerrain::DrawRightRamp() {
	glColor3f(0.5f, 0.5f, 0.5f);
	glBegin(GL_TRIANGLES);
	glNormal3f(1.0f, 0.0f, 0.0f);
	glVertex3f(block_size, 0.0f, 0.0f);
	glNormal3f(1.0f, 0.0f, 0.0f);
	glVertex3f(block_size, block_size, 0.0f);
	glNormal3f(1.0f, 0.0f, 0.0f);
	glVertex3f(block_size, 0.0f, block_size * 2.0f);
	glEnd();
	glColor3f(1.0f, 1.0f, 1.0f);
}


// Draw block terrain.
void BlockTerrain::Draw() {
	int     i, j, k;
	double x, y, z;
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Draw blocks.
	for (i = 0; i < width; i++) {
		for (j = 0; j < height; j++) {
			// Draw a ramp?
			if ((blocks[i][j].type == Block::RAMP) && IsUpperRamp(i, j)) {
				glPushMatrix();

				switch (blocks[i][j].rampDir) {
				case Block::NORTH:
					x = (double)i * block_size;
					y = (double)blocks[i][j].elevation * block_size;
					z = (double)j * block_size;
					glTranslatef(x, y, z);
					break;

				case Block::SOUTH:
					x = ((double)i * block_size) + block_size;
					y = (double)blocks[i][j].elevation * block_size;
					z = ((double)j * block_size) + block_size;
					glTranslatef(x, y, z);
					glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
					break;

				case Block::EAST:
					x = ((double)i * block_size) + block_size;
					y = (double)blocks[i][j].elevation * block_size;
					z = (double)j * block_size;
					glTranslatef(x, y, z);
					glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
					break;

				case Block::WEST:
					x = (double)i * block_size;
					y = (double)blocks[i][j].elevation * block_size;
					z = ((double)j * block_size) + block_size;
					glTranslatef(x, y, z);
					glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
					break;
				}

				glCallList(rampSurfaceDisplay);
				glCallList(leftRampDisplay);
				glCallList(rightRampDisplay);
				glPopMatrix();
			}

			// Draw block stack.
			x = (double)i * block_size;
			z = (double)j * block_size;

			for (k = 0; k < (int)blocks[i][j].texture_indexes.GetCount(); k++) {
				y = (double)k * block_size;
				glPushMatrix();
				glTranslatef(x, y, z);
				glBindTexture(GL_TEXTURE_2D, block_textures[blocks[i][j].texture_indexes[k]]);
				glCallList(blockDisplay);
				glPopMatrix();
			}
		}
	}
}
