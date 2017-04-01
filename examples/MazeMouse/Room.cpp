#include "Room.h"

Room::Room(int x, int y, int type, int cx, int cy) {
	r = RectC(x, y, roomSize.width, roomSize.height);
	this->type = type;
	this->cx   = cx;
	this->cy   = cy;
	selected  = has_mouse = has_cheese = false;
}

// Set room doors to other rooms
void Room::SetDoors(Vector<Room>& doors) {
	this->doors <<= doors;
}

void Room::Paint(Draw& w) {
	ImageDraw& id(w.GetSize());

	//id.DrawRect(sz, White());

	if (selected) {
		id.DrawRect(x, y, width, height, Black());
		id.DrawRect(x + 4, y + 4, width - 8, height - 8, White());
	}
	else
		id.DrawRect(x, y, width, height, Black());

	if (has_cheese)
		id.DrawImage(x + (width / 4), y + (height / 4), cheese_image);

	if (has_mouse)
		id.DrawImage(x + (width / 4), y + (height / 4), mouse_imagemazeCanvas);

	for (int i = 0; i < doors.GetCount(); i++)
		id.DrawLine(x + width, y + (height / 2), to.x, to.y + (to.height / 2), 1, Black());

	w.DrawImage(id);
}
