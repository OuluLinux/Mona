#ifndef _MazeWorm_MazeWorm_h
#define _MazeWorm_MazeWorm_h

#include <CtrlLib/CtrlLib.h>
using namespace Upp;

#include "Maze.h"

#define IMAGECLASS MazeWormImg
#define IMAGEFILE <MazeWorm/MazeWorm.iml>
#include <Draw/iml_header.h>


class MazeWorm : public TopWindow {
	ImageCtrl ic;
	Maze maze;
	Vector<Point> path;
	int pos;
	int grid;
	One<ImageDraw> id;

public:
	typedef MazeWorm CLASSNAME;
	MazeWorm();

	void RefreshBoard();
	void NewMaze();

};

#endif
