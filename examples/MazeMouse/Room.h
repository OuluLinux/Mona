#ifndef _MazeMouse_Room_h_
#define _MazeMouse_Room_h_

#include <Mona/Mona.h>
#include <CtrlLib/CtrlLib.h>
using namespace Upp;

// Room types
enum {START_ROOM, BEGIN_MAZE, MAZE_ROOM, END_MAZE, GOAL_ROOM, DEAD_ROOM};

// Responses
enum {TAKE_DOOR_0, TAKE_DOOR_1, TAKE_DOOR_2, WAIT, HOP};


class Room : public Ctrl {

protected:
	Rect r;
	int     type;
	int     cx;
	int     cy;
	bool selected;
	bool has_mouse;
	bool has_cheese;
	Vector<Room> doors;

public:
	Room() : type(-1), cx(-1), cy(-1), selected(false), has_mouse(false), has_cheese(false) {}
	void Init(int x, int y, int type, int cx, int cy);
	void SetDoors(Vector<Room>& doors);
	virtual void Paint(Draw& w);
};

#endif
