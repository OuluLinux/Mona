

// This is the old server connecting version

#if 0

#ifndef _MazeMouse_MazeMouse_h_
#define _MazeMouse_MazeMouse_h_

#include <Mona/Mona.h>
using namespace Mona;

#include <CtrlLib/CtrlLib.h>
using namespace Upp;




/*
    Maze-learning mouse.

    A client for maze learning task. Server runs mona neural network.

    The user can train a mouse to learn a path through a maze to obtain cheese.
    The mouse is initially presented with one of a set of three possible doors
    that leads to the beginning of a maze. That door choice can be associated
    with a door choice at the end of the maze in order to obtain the cheese.
    The intermediate task is navigating a path through the maze.

    Reference:
    T. E. Portegys, "An Application of Context-Learning in a Goal-Seeking Neural Network",
    The IASTED International Conference on Computational Intelligence (CI 2005).
    See tom.portegys.com/research/ci2005.pdf
*/

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
	
	Room(int x, int y, int type, int cx, int cy);
	void SetDoors(Vector<Room>& doors);
	virtual void Paint(Draw& w);
};



class MazeMouse : public TopWindow {


	double CHEESE_NEED, CHEESE_GOAL;
	int STEP_DELAY, DISPLAY_DELAY;
	int TRAIN, RUN;

	String MOUSE_IMAGE_FILE, CHEESE_IMAGE_FILE, SQUEAK_SOUND_FILE;

	// Are maze room doors all visible?
	// Some of these doors are blocked.
	bool see_all_maze_doors;

	// Server.
	String         serverHost;
	int            serverPort;
	InetAddress    serverAddress;
	Socket         socket;
	BufferedReader in;
	PrintWriter    out;

	// Maze.
	Array<Array<Room> > maze;
	Size roomSize;
	int       begin_maze_index;
	int       end_maze_index;
	int       mouseX;
	int       mouseY;

	// Display.
	/*Size screen_size;
	Canvas    mazeCanvas;
	Size mazeSize;
	Graphics  canvasGraphics;
	Image     mazeImage;
	Graphics  imageGraphics;
	String    error;*/

	// Controls.
	/*JPanel    controls;
	JCheckBox start;
	Choice    mode;
	TextField response;
	JCheckBox mute;
	JButton   reset;*/
	bool   logging;

	// Font.
	/*Font        font;
	FontMetrics fontMetrics;*/
	/*int         fontAscent;
	int         fontWidth;
	int         fontHeight;*/

	// Images and sound.
	Image     mouse_image;
	Image     cheese_image;
	AudioClip squeakSound;

	// Threads.
	/*private Thread displayThread;
	private Thread stepThread;
	Toolkit        toolkit;*/


	void Init();
	void Start();
	void Stop();
	void Run();
	void Display();
	void Step();
	void ActionPerformed(ActionEvent evt);
	void ItemStateChanged(ItemEvent evt);

	virtual void LeftDown(Point p, dword keyflags);
	virtual bool Key(dword key, int count);


}



#endif
#endif
