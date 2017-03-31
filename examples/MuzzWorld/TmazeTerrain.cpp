#include "TmazeTerrain.h"

TmazeTerrain::TmazeTerrain(int random_seed, int width, int height, double block_size)
	: BlockTerrain(random_seed, width, height, 1, 1, 1, 0, 0, block_size){
	GenerateMaze();
}

TmazeTerrain::~TmazeTerrain() {
	tmaze_path.Clear();
}

void TmazeTerrain::GenerateMaze() {
	int i, j, xmin, xmax, ymin, ymax, xd, yd;
	Vector<Tuple2<int, int> > path;
	Tuple2<int, int>          s(0, 0);

	// Sanity check.
	if ((width < 5) || (height < 5)) {
		fprintf(stderr, "Terrain dimensions too small for T-maze\n");
		exit(1);
	}

	// Create a long random maze path.
	tmaze_path.Clear();

	for (i = 0; i < MAZE_GENERATIONS; i++) {
		path.Clear();
		path.Add(s);
		MapMaze(path, Block::NORTH);

		if (path.GetCount() > tmaze_path.GetCount()) {
			tmaze_path.Clear();
			tmaze_path = path;
		}
	}

	// Center path in terrain.
	GetBounds(tmaze_path, xmin, ymin, xmax, ymax);
	xd = 1 - xmin;
	yd = 1 - ymin;

	for (i = 0; i < (int)tmaze_path.GetCount(); i++) {
		tmaze_path[i].first  += xd;
		tmaze_path[i].second += yd;
	}

	// Build the terrain.
	for (i = 0; i < width; i++) {
		for (j = 0; j < height; j++)
			blocks[i][j].elevation = 1;
	}

	for (i = 0; i < (int)tmaze_path.GetCount(); i++)
		blocks[tmaze_path[i].first][tmaze_path[i].second].elevation = 0;

	Build();

	// Gray the floor.
	for (i = 0; i < width; i++) {
		for (j = 0; j < height; j++) {
			if (blocks[i][j].elevation == 0)
				blocks[i][j].texture_indexes[0] = NUM_BLOCK_TEXTURES - 1;
		}
	}
}


bool TmazeTerrain::MapMaze(Vector<Tuple2<int, int> >& path, Block::DIRECTION direction) {
	int              s, c, b, t, e, x, y, x1, y1, x2, y2, xc, yc, xb, yb;
	Block::DIRECTION nextdir;
	Tuple2<int, int> cell;
	// Check cell placements.
	s = (int)path.GetCount() - 1;
	c = s + 1;
	b = c + 1;
	t = b + 1;
	e = t + 1;
	x = path[s].first;
	y = path[s].second;

	switch (direction) {
	case Block::NORTH:
		x1 = x - 1;
		y1 = y + 2;

		if (!CheckCell(path, x1, y1))
			return false;

		xc = x;
		yc = y + 1;

		if (!CheckCell(path, xc, yc))
			return false;

		xb = x;
		yb = y + 2;

		if (!CheckCell(path, xb, yb))
			return false;

		x2 = x + 1;
		y2 = y + 2;

		if (!CheckCell(path, x2, y2))
			return false;

		break;

	case Block::EAST:
		x1 = x + 2;
		y1 = y + 1;

		if (!CheckCell(path, x1, y1))
			return false;

		xc = x + 1;
		yc = y;

		if (!CheckCell(path, xc, yc))
			return false;

		xb = x + 2;
		yb = y;

		if (!CheckCell(path, xb, yb))
			return false;

		x2 = x + 2;
		y2 = y - 1;

		if (!CheckCell(path, x2, y2))
			return false;

		break;

	case Block::SOUTH:
		x1 = x - 1;
		y1 = y - 2;

		if (!CheckCell(path, x1, y1))
			return false;

		xc = x;
		yc = y - 1;

		if (!CheckCell(path, xc, yc))
			return false;

		xb = x;
		yb = y - 2;

		if (!CheckCell(path, xb, yb))
			return false;

		x2 = x + 1;
		y2 = y - 2;

		if (!CheckCell(path, x2, y2))
			return false;

		break;

	case Block::WEST:
		x1 = x - 2;
		y1 = y + 1;

		if (!CheckCell(path, x1, y1))
			return false;

		xc = x - 1;
		yc = y;

		if (!CheckCell(path, xc, yc))
			return false;

		xb = x - 2;
		yb = y;

		if (!CheckCell(path, xb, yb))
			return false;

		x2 = x - 2;
		y2 = y - 1;

		if (!CheckCell(path, x2, y2))
			return false;

		break;
	}

	cell.first  = xc;
	cell.second = yc;
	path.Add(cell);
	cell.first  = xb;
	cell.second = yb;
	path.Add(cell);
	cell.first  = 0;
	cell.second = 0;
	path.Add(cell);
	path.Add(cell);

	// Choose next direction.
	switch (direction) {
	case Block::NORTH:
	case Block::SOUTH:
		if (randomizer->RAND_BOOL()) {
			nextdir        = Block::EAST;
			path[t].first  = x1;
			path[t].second = y1;
			path[e].first  = x2;
			path[e].second = y2;
		}
		else {
			nextdir        = Block::WEST;
			path[t].first  = x2;
			path[t].second = y2;
			path[e].first  = x1;
			path[e].second = y1;
		}

		break;

	case Block::EAST:
	case Block::WEST:
		if (randomizer->RAND_BOOL()) {
			nextdir        = Block::SOUTH;
			path[t].first  = x1;
			path[t].second = y1;
			path[e].first  = x2;
			path[e].second = y2;
		}
		else {
			nextdir        = Block::NORTH;
			path[t].first  = x2;
			path[t].second = y2;
			path[e].first  = x1;
			path[e].second = y1;
		}

		break;
	}

	if (MapMaze(path, nextdir))
		return true;

	// Try reverse direction.
	switch (nextdir) {
	case Block::NORTH:
		nextdir = Block::SOUTH;
		break;

	case Block::EAST:
		nextdir = Block::WEST;
		break;

	case Block::SOUTH:
		nextdir = Block::NORTH;
		break;

	case Block::WEST:
		nextdir = Block::EAST;
		break;
	}

	cell    = path[e];
	path[e] = path[t];
	path[t] = cell;
	MapMaze(path, nextdir);
	return true;
}


// Is cell placement OK?
bool TmazeTerrain::CheckCell(Vector<Tuple2<int, int> >& path,
							 int x, int y) {
	int i, j, x2, y2, xmin, xmax, ymin, ymax;
	// Check maze bounds.
	GetBounds(path, xmin, ymin, xmax, ymax);

	if ((x < xmin) && (width - (xmax - x) < 3))
		return false;

	if ((x > xmax) && (width - (x - xmin) < 3))
		return false;

	if ((y < ymin) && (height - (ymax - y) < 3))
		return false;

	if ((y > ymax) && (height - (y - ymin) < 3))
		return false;

	// Check adjacent cells.
	x2 = x - 1;
	y2 = y;

	for (i = 0, j = (int)path.GetCount() - 1; i < j; i++) {
		if ((path[i].first == x2) && (path[i].second == y2))
			return false;
	}

	x2 = x + 1;
	y2 = y;

	for (i = 0, j = (int)path.GetCount() - 1; i < j; i++) {
		if ((path[i].first == x2) && (path[i].second == y2))
			return false;
	}

	x2 = x;
	y2 = y - 1;

	for (i = 0, j = (int)path.GetCount() - 1; i < j; i++) {
		if ((path[i].first == x2) && (path[i].second == y2))
			return false;
	}

	x2 = x;
	y2 = y + 1;

	for (i = 0, j = (int)path.GetCount() - 1; i < j; i++) {
		if ((path[i].first == x2) && (path[i].second == y2))
			return false;
	}

	return true;
}


// Get path bounds.
void TmazeTerrain::GetBounds(Vector<Tuple2<int, int> >& path, int& xmin, int& ymin, int& xmax, int& ymax) {
	ASSERT(path.GetCount() > 0);

	for (int i = 0; i < (int)path.GetCount(); i++) {
		if (i == 0) {
			xmin = xmax = path[i].first;
			ymin = ymax = path[i].second;
		}
		else {
			if (xmin > path[i].first)
				xmin = path[i].first;

			if (xmax < path[i].first)
				xmax = path[i].first;

			if (ymin > path[i].second)
				ymin = path[i].second;

			if (ymax < path[i].second)
				ymax = path[i].second;
		}
	}
}
