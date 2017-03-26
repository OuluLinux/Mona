#ifndef _IntelCore_MazeSolver_h_
#define _IntelCore_MazeSolver_h_

#include <CtrlLib/CtrlLib.h>
using namespace Upp;


class Maze {
	Vector<byte> grid;
	Vector<int>  distance_map;
	int w, h;
	Point exit;
	
	void ClearGrid();
	void RefreshDistanceMap();
	void CarveRecursiveBacktrack(int x, int y);
	
public:
	Maze();
	
	void SetSize(int w, int h) {this->w = w; this->h = h;}
	void SetExit(int x, int y) {exit.x = x; exit.y = y;}
	void GenerateRecursiveBacktrack();
	Vector<Point> GetDistanceMapPath(int x, int y);
	Image GetImage(int gw, int gh, int bw);
	
};

#endif
