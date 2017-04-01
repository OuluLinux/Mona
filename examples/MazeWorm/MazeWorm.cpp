#include "MazeWorm.h"

MazeWorm::MazeWorm() {
	Title("Maze Demo");
	Sizeable().MaximizeBox();
	Add(ic.SizePos());
	grid = 10;
	SetRect(0, 0, 640, 480);
	NewMaze();
	PostCallback(THISBACK(RefreshBoard));
}

void MazeWorm::NewMaze() {
	id.Clear();
	Size sz = GetSize();
	int grid_w = sz.cx / grid;
	int grid_h = sz.cy / grid;
	maze.SetSize(grid_w, grid_h);
	maze.SetExit(grid_w - 1, grid_h / 2);
	maze.GenerateRecursiveBacktrack();
	path = maze.GetDistanceMapPath(0, grid_h / 2);
	pos = 1;
}

void MazeWorm::RefreshBoard() {
	if (!id) {
		Image im = maze.GetImage(grid, grid, 1);
		Size sz(im.GetSize());
		id = new ImageDraw(sz);
		id->DrawImage(0, 0, im);
	}

	if (pos < path.GetCount()) {
		Point pt1 = path[pos - 1];
		Point pt2 = path[pos];
		int x1 = pt1.x * grid + grid / 2;
		int y1 = pt1.y * grid + grid / 2;
		int x2 = pt2.x * grid + grid / 2;
		int y2 = pt2.y * grid + grid / 2;
		id->DrawLine(x1, y1, x2, y2, grid / 2, Color(255, 0, 0));
		pos++;
		ic.SetImage(*id);
	}
	else
		NewMaze();

	Sleep(20);
	PostCallback(THISBACK(RefreshBoard));
}
