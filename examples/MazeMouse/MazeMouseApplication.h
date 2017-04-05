#ifndef _MazeMouse_MazeMouseApplication_h_
#define _MazeMouse_MazeMouseApplication_h_

#include "Room.h"

#define LAYOUTFILE <MazeMouse/MazeMouseApplication.lay>
#include <CtrlCore/lay.h>

class MazeMouseApplication;

class MazeCtrl : public Ctrl {
	
	VectorMap<Rect, Point> screen_to_room;
	
public:
	typedef MazeCtrl CLASSNAME;
	MazeCtrl();
	
	virtual void Paint(Draw& w);
	virtual void LeftDown(Point p, dword keyflags);
	
	MazeMouseApplication* app;
	
};

class MazeMouseApplication : public TopWindow {

protected:
	friend class MazeCtrl;

	// Sizes.
	int WIDTH, HEIGHT;

	// Cheese need and goal.
	double CHEESE_NEED, CHEESE_GOAL;

	// Modes
	enum {TRAIN, RUN};

	// Random seed.
	int DEFAULT_RANDOM_SEED;

	// Step delay (ms).
	int STEP_DELAY;

	// Display update frequency (ms).
	int DISPLAY_DELAY;

	// Mouse and cheese images.
	String MOUSE_IMAGE_FILE, CHEESE_IMAGE_FILE;

	// Mouse squeak sound.
	String SQUEAK_SOUND_FILE;

	String HELP_TEXT;

	// Mona.
	Mona mona;
	Vector<SENSOR> sensors;

	// Maze.
	Array<Array<Room> > maze;
	int       begin_maze_index;
	int       end_maze_index;
	int       mouseX;
	int       mouseY;

	// Are maze room doors all visible?
	// Some of these doors are blocked.
	bool see_all_maze_doors;

	// Display.
	WithCtrlBar<ParentCtrl> ctrl_bar;
	MazeCtrl mazectrl;
	DocEdit help_lbl;
	
	int stuck_count;
	
	bool running, stopped;
	
public:
	typedef MazeMouseApplication CLASSNAME;
	
	// Constructor.
	MazeMouseApplication();
	~MazeMouseApplication();


	// Create Mona.
	void CreateMona();

	void Stop() {running = false; while (!stopped) Sleep(100);}
	
	// Run.
	void Runner();
	void Refresher();

	// Display.
	void StartTrial();
	void Reset();
	void ShowHelp();
	void Step();

	void EnableStart() {ctrl_bar.start_trial.Enable();}
	void SetResponse(String s) {ctrl_bar.response.SetData(s);}

	// Key press.
	virtual bool Key(dword key, int count);

};



#endif
