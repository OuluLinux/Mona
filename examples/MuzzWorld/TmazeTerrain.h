// For conditions of distribution and use, see copyright notice in muzz.hpp

/*
    T-maze terrain.

    A T-maze contains a path of adjacent T-shaped cell patterns.
    TBE  or  EBT
    C        C
    S        S
    where S = start cell, C = channel cell, B = branch cell,
    T = terminal cell, E = end cell.
    The end cell of one pattern abuts the start cell of the next
    pattern such that the TBE cells of the first pattern are colinear with
    the SCB cells of the second pattern. The cells comprising a pattern have
    a block height of one, and are surrounded by cells of height two except
    where connected to another pattern.
    A T-maze contains no ramps or block heights other than one or two.
*/

#ifndef __TMAZE_TERRAIN__
#define __TMAZE_TERRAIN__

#include "BlockTerrain.h"

// T-maze terrain.
class TmazeTerrain : public BlockTerrain {
public:
	TmazeTerrain(int random_seed, int width, int height, double block_size);
	~TmazeTerrain();
	Vector<Tuple2<int, int> > tmaze_path;

private:

	// Tries for space-filling maze.
	enum { MAZE_GENERATIONS = 10 };

	void GenerateMaze();
	bool MapMaze(Vector<Tuple2<int, int> >& path, Block::DIRECTION);
	bool CheckCell(Vector<Tuple2<int, int> >& path, int x, int y);
	void GetBounds(Vector<Tuple2<int, int> >& path, int& xmin, int& ymin, int& xmax, int& ymax);
};
#endif
