#include "MazeMouseApplication.h"


inline int Max(int a, int b) {return a > b ? a : b;}
inline int Min(int a, int b) {return a < b ? a : b;}

MazeCtrl::MazeCtrl() {
	app = NULL;
}

void MazeCtrl::LeftDown(Point p, dword keyflags) {
	
	for(int i = 0; i < screen_to_room.GetCount(); i++) {
		const Rect& r = screen_to_room.GetKey(i);
		if (r.Contains(p)) {
			Point pt = screen_to_room[i];
			int cx = pt.x;
			int cy = pt.y;
			Room& room = app->maze[cx][cy];
			room.selected = !room.selected;
			
			int& mouseX = app->mouseX;
			int& mouseY = app->mouseY;
			
			// Move mouse/cheese if necessary.
			Array<Array<Room> >& maze = app->maze;
			if (room.selected) {
				for (int i = 0; i < maze[cx].GetCount(); i++) {
					if ((i != cy) && maze[cx][i].selected) {
						maze[cx][i].selected = false;
		
						if (maze[cx][i].has_mouse)
							mouseX = mouseY = -1;
		
						maze[cx][i].has_mouse  = false;
						maze[cx][i].has_cheese = false;
					}
				}
		
				if ((mouseX != -1) && (mouseX > cx)) {
					maze[mouseX][mouseY].has_mouse = false;
					mouseX = mouseY = -1;
				}
		
				if (mouseX == -1) {
					maze[cx][cy].has_mouse = true;
					mouseX = cx;
					mouseY = cy;
				}
		
				if (cx == (app->end_maze_index + 1))
					maze[cx][cy].has_cheese = true;
			}
			else {
				if (maze[cx][cy].has_mouse) {
					mouseX = mouseY = -1;
		
					for (int i = cx + 1; (i < maze.GetCount()) && (mouseX == -1); i++) {
						for (int j = 0; (j < maze[i].GetCount()) && (mouseX == -1); j++) {
							if (maze[i][j].selected) {
								maze[i][j].has_mouse = true;
								mouseX = i;
								mouseY = j;
							}
						}
					}
				}
		
				maze[cx][cy].has_mouse  = false;
				maze[cx][cy].has_cheese = false;
			}
			
			Refresh();
			break;
		}
	}
}

void MazeCtrl::Paint(Draw& w) {
	Size sz = GetSize();
	
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	
	int max_horz = Max(1, app->maze.GetCount());
	int max_vert = 1;
	for(int i = 0; i < app->maze.GetCount(); i++)
		max_vert = Max(max_vert, app->maze[i].GetCount());
	
	double w_grid = sz.cx / max_horz;
	double h_grid = sz.cy / max_vert;
	int box_grid = Max(10, Min(w_grid, h_grid) * 0.5);
	int width = box_grid;
	int height = box_grid;
	
	VectorMap<int64, Tuple2<Point, Point> > connection_points;
	Vector<Room*> rooms;
	
	screen_to_room.Clear();
	
	Font fnt = Arial(14);
	
	for(int i = 0; i < app->maze.GetCount(); i++) {
		
		String title;
		if (i == 0)			title = "Start";
		else if (i == 1)	title = "Entry";
		else if (i > 1 && i < app->maze.GetCount()-2)	title = "Maze";
		else if (i == app->maze.GetCount() - 2) title = "Exit";
		else title = "Goal";
		Size txt_sz = GetTextSize(title, fnt);
		
		
		Array<Room>& vert = app->maze[i];
		for(int j = 0; j < vert.GetCount(); j++) {
			Room& room = vert[j];
			
			int vert_pos = (max_vert - vert.GetCount()) / 2 + j;
			int l = i			* w_grid; // left border of grid slot
			int t = vert_pos	* h_grid; // top ...
			int x = l + (w_grid - box_grid) / 2;
			int y = t + (h_grid - box_grid) / 2;
			
			if (j == 0) {
				int tx = l + (w_grid - txt_sz.cx) / 2;
				int ty = t + 4;
				id.DrawText(tx, ty, title, fnt, Black());
			}
			
			if (room.selected) {
				id.DrawRect(x, y, width, height, Black());
				id.DrawRect(x + 4, y + 4, width - 8, height - 8, White());
			}
			else {
				id.DrawRect(x, y, width, height, Black());
				id.DrawRect(x + 1, y + 1, width - 2, height - 2, White());
			}
		
			if (room.has_cheese)
				id.DrawImage(x + (width / 4), y + (height / 4), width / 2, height / 2, Images::cheese_image);
		
			if (room.has_mouse)
				id.DrawImage(x + (width / 4), y + (height / 4), width / 2, height / 2, Images::mouse_image);
			
			
			rooms.Add(&room);
			Tuple2<Point, Point>& pts = connection_points.GetAdd((int64)&room);
			pts.a = Point(x, y + height / 2);
			pts.b = Point(x + width, y + height / 2);
			
			screen_to_room.Add(RectC(x, y, width, height), Point(i,j));
		}
	}
	
	// Connect rooms
	for(int i = 0; i < rooms.GetCount(); i++) {
		Room& room = *rooms[i];
		Point& from = connection_points.Get((int64)&room).b;
		
		for(int j = 0; j < room.doors.GetCount(); j++) {
			if (!room.doors[j]) continue;
			Room& door = *room.doors[j];
			Point& to = connection_points.Get((int64)&door).a;
			
			id.DrawLine(from, to, 1, Black());
		}
	}
	
	w.DrawImage(0, 0, id);
}
