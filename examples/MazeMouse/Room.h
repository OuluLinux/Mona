#ifndef _MazeMouse_Room_h_
#define _MazeMouse_Room_h_

#include <Mona/Mona.h>
#include <CtrlLib/CtrlLib.h>
using namespace Upp;

#define IMAGECLASS Images
#define IMAGEFILE <MazeMouse/images.iml>
#include <Draw/iml_header.h>


// Room types
enum {START_ROOM, BEGIN_MAZE, MAZE_ROOM, END_MAZE, GOAL_ROOM, DEAD_ROOM};

// Responses
enum {TAKE_DOOR_0, TAKE_DOOR_1, TAKE_DOOR_2, WAIT, HOP};


class Room : public Ctrl {

protected:
	friend class MazeMouseApplication;
	friend class MazeCtrl;
	
	
	//Rect r;
	int     type;
	int     cx;
	int     cy;
	bool selected;
	bool has_mouse;
	bool has_cheese;
	Vector<Room*> doors;
	
public:
	Room() : type(-1), cx(-1), cy(-1), selected(false), has_mouse(false), has_cheese(false) {}
	void Init(int type) {this->type = type;}
	void SetPos(int x, int y) {cx = x; cy = y;}
	
	// Set room doors to other rooms.
	void SetDoors(Room* left, Room* fwd, Room* right) {
		doors.SetCount(0);
		doors.Add(left);
		doors.Add(right);
		doors.Add(fwd);
	}
	
};

#endif
