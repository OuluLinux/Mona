#include "MazeMouseApplication.h"

MazeMouseApplication::MazeMouseApplication() {
	mazectrl.app = this;
	
	// Sizes.
	WIDTH  = 700;
	HEIGHT = 500;
	
	// Cheese need and goal.
	CHEESE_NEED = 1.0;
	CHEESE_GOAL = 0.5;
	
	// Random seed.
	DEFAULT_RANDOM_SEED = 7671;
	
	// Step delay (ms).
	STEP_DELAY = 250;
	
	// Display update frequency (ms).
	DISPLAY_DELAY = 50;
	
	// Mouse and cheese images.
	MOUSE_IMAGE_FILE  = "mouse.gif";
	CHEESE_IMAGE_FILE = "cheese.jpg";
	
	// Mouse squeak sound.
	SQUEAK_SOUND_FILE = "squeak.wav";
	HELP_TEXT =
		"Create a path for the mouse to follow to the cheese by clicking on a sequence of rooms from one of the\n"
		"start rooms through the maze to one of the cheese goal rooms. Then train the mouse by selecting train\n"
		"mode and running a few trials. You run a trial by clicking the start trial control which starts the\n"
		"mouse moving through the rooms. When the mouse finishes moving, click stop trial. When the mouse is\n"
		"training, it is being shown the correct responses to make in each room. Now use test mode to test the\n"
		"mouse to see if it can find the cheese on its own!\n"
		"\n"
		"Now try something interesting! First click reset to make the mouse forget everything.\n"
		"Then select just one of the start rooms, the entry, exit, and one of the goal rooms. The mouse will\n"
		"\"hop\" over the maze. Train that a few times and test the mouse on it.\n"
		"\n"
		"Now train a path through just the maze by selecting the entry room, three connected maze rooms, and the\n"
		"exit room. Don't bother to test that now since the mouse didn't find any cheese that would make it want\n"
		"to do that on its own.\n"
		"\n"
		"Now comes the tricky part. Combine the paths that you trained separately into one path by selecting all\n"
		"the rooms from the start to the entry, then through the maze to the exit, then to the cheese goal. Test\n"
		"the mouse a few times to see if it can find the cheese. You may see it do an extra hop at the entry that\n"
		"it doesn't need to, but that's OK. If the mouse can find the cheese that means that it can combine things\n"
		"that it knows together to solve the problem!\n"
		"\n"
		"You can try other fancy stuff like training more than one start->entry->exit->goal room path, or changing\n"
		"the maze path while keeping the start->entry->exit->goal room path the same.\n"
		//"\n"
		//"Type 'p' to print the network."
		;
	see_all_maze_doors = false;
	
	// Set title.
	Title("Maze-learning Mouse");
	Icon(Images::cheese_image());
	Sizeable().MaximizeBox().MinimizeBox();
	
	// Create mona.
	CreateMona();
	
	// Add controls.
	Add(mazectrl.HSizePos().VSizePos(0,30));
	Add(help_lbl.HSizePos().VSizePos(0,30));
	help_lbl.Hide();
	help_lbl.SetData(HELP_TEXT);
	CtrlLayout(ctrl_bar);
	Add(ctrl_bar.BottomPos(0, 30).HSizePos());
	ctrl_bar.train_mode.Add("Train");
	ctrl_bar.train_mode.Add("Test");
	ctrl_bar.start_trial <<= THISBACK(StartTrial);
	ctrl_bar.reset <<= THISBACK(Reset);
	ctrl_bar.help <<= THISBACK(ShowHelp);
	ctrl_bar.train_mode.SetIndex(0);
	
	// Build maze.
	maze.SetCount(7);
	maze[0].SetCount(3);
	maze[1].SetCount(1);
	maze[2].SetCount(3);
	maze[3].SetCount(5);
	maze[4].SetCount(3);
	maze[5].SetCount(1);
	maze[6].SetCount(3);
	maze[0][1] .Init(START_ROOM);
	maze[0][0] .Init(START_ROOM);
	maze[0][2] .Init(START_ROOM);
	begin_maze_index = 1;
	maze[1][0]  .Init(BEGIN_MAZE);
	maze[0][0].SetDoors(NULL, NULL, &maze[1][0]);
	maze[0][1].SetDoors(NULL, &maze[1][0], NULL);
	maze[0][2].SetDoors(&maze[1][0], NULL, NULL);
	maze[2][1] .Init(MAZE_ROOM);
	maze[2][0] .Init(MAZE_ROOM);
	maze[2][2] .Init(MAZE_ROOM);
	maze[1][0].SetDoors(&maze[2][0], &maze[2][1], &maze[2][2]);
	maze[3][2] .Init(MAZE_ROOM);
	maze[3][1] .Init(MAZE_ROOM);
	maze[3][0] .Init(MAZE_ROOM);
	maze[3][3] .Init(MAZE_ROOM);
	maze[3][4] .Init(MAZE_ROOM);
	maze[2][0].SetDoors(&maze[3][0], &maze[3][1], &maze[3][2]);
	maze[2][1].SetDoors(&maze[3][1], &maze[3][2], &maze[3][3]);
	maze[2][2].SetDoors(&maze[3][2], &maze[3][3], &maze[3][4]);
	maze[4][1] .Init(MAZE_ROOM);
	maze[4][0] .Init(MAZE_ROOM);
	maze[4][2] .Init(MAZE_ROOM);
	maze[3][0].SetDoors(NULL, NULL, &maze[4][0]);
	maze[3][1].SetDoors(NULL, &maze[4][0], &maze[4][1]);
	maze[3][2].SetDoors(&maze[4][0], &maze[4][1], &maze[4][2]);
	maze[3][3].SetDoors(&maze[4][1], &maze[4][2], NULL);
	maze[3][4].SetDoors(&maze[4][2], NULL, NULL);
	end_maze_index = 5;
	maze[5][0]   .Init(END_MAZE);
	maze[4][0].SetDoors(NULL, NULL, &maze[5][0]);
	maze[4][1].SetDoors(NULL, &maze[5][0], NULL);
	maze[4][2].SetDoors(&maze[5][0], NULL, NULL);
	maze[6][1] .Init(GOAL_ROOM);
	maze[6][0] .Init(GOAL_ROOM);
	maze[6][2] .Init(GOAL_ROOM);
	maze[5][0].SetDoors(&maze[6][0], &maze[6][1], &maze[6][2]);
	maze[6][0].SetDoors(NULL, NULL, NULL);
	maze[6][1].SetDoors(NULL, NULL, NULL);
	maze[6][2].SetDoors(NULL, NULL, NULL);
	mouseX = mouseY = 0;
	maze[0][0].has_mouse = true;
	maze[0][0].selected = true;
	maze[end_maze_index + 1][2].has_cheese = true;
	maze[end_maze_index + 1][2].selected  = true;
	
	for(int i = 0; i < maze.GetCount(); i++) {
		for(int j = 0; j < maze[i].GetCount(); j++) {
			maze[i][j].SetPos(i, j);
		}
	}
	
	running = false;
	stopped = true;
}

MazeMouseApplication::~MazeMouseApplication() {
	Stop();
}

void MazeMouseApplication::StartTrial() {
	Stop();
	ctrl_bar.start_trial.Disable();
	Thread::Start(THISBACK(Runner));
}

void MazeMouseApplication::Reset() {
	Stop();
	
	// Erase mouse memory.
	mona.ClearWorkingMemory();
	mona.ClearLongTermMemory();

	// Clear maze.
	for (int i = 0; i < maze.GetCount(); i++) {
		for (int j = 0; j < maze[i].GetCount(); j++) {
			maze[i][j].selected  = false;
			maze[i][j].has_mouse  = false;
			maze[i][j].has_cheese = false;
		}
	}

	mouseX = mouseY = 0;
	maze[0][0].selected = true;
	maze[0][0].has_mouse = true;
	maze[end_maze_index + 1][2].selected  = true;
	maze[end_maze_index + 1][2].has_cheese = true;
	
	ctrl_bar.response.SetData("");
	
	PostCallback(THISBACK(Refresher));
}

void MazeMouseApplication::ShowHelp() {
	if (help_lbl.IsShown())
		help_lbl.Hide();
	else
		help_lbl.Show();
}

void MazeMouseApplication::CreateMona() {
	
	mona.MAX_MEDIATOR_LEVEL = 1;
	mona.Init(5, HOP+1, 1);
	
	// Set a long second effect interval
	// for a higher level mediator.
	mona.SetEffectEventIntervals(1, 2);
	mona.SetEffectEventInterval(1, 0, 2, 0.5);
	mona.SetEffectEventInterval(1, 1, 10, 0.5);
	
	// Set need and goal for cheese.
	mona.SetNeed(0, CHEESE_NEED);
	
	sensors.SetCount(5);
	sensors[0] = 0.0;
	sensors[1] = 0.0;
	sensors[2] = 0.0;
	sensors[3] = (float)GOAL_ROOM;
	sensors[4] = 1.0;
	
	mona.AddGoal(0, sensors, 0, CHEESE_GOAL);
}

void MazeMouseApplication::Runner() {
	running = true;
	stopped = false;
	
	
    // Reset need.
    mona.SetNeed(0, CHEESE_NEED);

    // Clear working memory.
    mona.ClearWorkingMemory();
    
	// Step loop.
	stuck_count = 0;
	while (!Thread::IsShutdownThreads() && running) {
		Step();
		PostCallback(THISBACK(Refresher));
		if (running)
			Sleep(STEP_DELAY);
	}
	stopped = true;
	PostCallback(THISBACK(EnableStart));
}

void MazeMouseApplication::Refresher() {
	mazectrl.Refresh();
}

void MazeMouseApplication::Step() {
	
	//if (!start.isSelected() || (mouseX == -1) || (error != null))
	//	return;
	if (mouseX == -1)
		return;
	

	// Determine correct response.
	int cx = -1;
	int cy = -1;
	int cr = -1;
	int r  = -1;
	
	bool has_goal = false;
	for(int i = 0; i < maze.Top().GetCount(); i++) {
		if (maze.Top()[i].selected) {
			has_goal = true;
			break;
		}
	}
	
	for (int i = mouseX + 1; (i < maze.GetCount()) && (cx == -1); i++) {
		for (int j = 0; (j < maze[i].GetCount()) && (cx == -1); j++) {
			if (maze[i][j].selected) {
				cx = i;
				cy = j;
			}
		}
	}

	if (cx == (mouseX + 1)) {
		for (int i = 0; (i < maze[mouseX][mouseY].doors.GetCount()) && (cr == -1); i++) {
			if (maze[mouseX][mouseY].doors[i] != NULL &&
				(maze[mouseX][mouseY].doors[i]->cx == cx) &&
				(maze[mouseX][mouseY].doors[i]->cy == cy)) {
				cr = i;
				break;
			}
		}
	}
	else {
		if (cx != -1)
			cr = HOP;
		else {
			cx = mouseX;
			cy = mouseY;
			cr = WAIT;
		}
	}

	if ((ctrl_bar.train_mode.GetIndex() == TRAIN) || (cr == WAIT))
		mona.OverrideResponse(cr);
	else
		mona.ClearResponseOverride();

	// Initiate sensory/response cycle.
	for (int i = 0; i < maze[mouseX][mouseY].doors.GetCount(); i++) {
		if (maze[mouseX][mouseY].doors[i] != NULL)
			sensors[i] = 1.0;
		else {
			
			if (see_all_maze_doors && (mouseX > 1) && (mouseX < maze.GetCount()-2)) {
				// Maze room door is visible although possibly blocked.
				sensors[i] = 1.0;
			}
			else
				sensors[i] = 0.0;
		}
	}

	sensors[3] = (float)maze[mouseX][mouseY].type;

	if (maze[mouseX][mouseY].has_cheese) {
		sensors[4] = 1.0;
		
		// Eat the cheese!
		maze[mouseX][mouseY].has_cheese = false;

		//if ((squeakSound != null) && !mute.isSelected())
		//	squeakSound.play();
	}
	else
		sensors[4] = 0.0;

	// Move mouse based on response.
	r = mona.Cycle(sensors);

	
	if (r < WAIT)
		PostCallback(THISBACK1(SetResponse, "Door " + IntStr(r)));
	else if (r == WAIT)
		PostCallback(THISBACK1(SetResponse, "Wait"));
	else
		PostCallback(THISBACK1(SetResponse, "Hop"));

	// Move mouse.
	bool stuck_in_last =
		stuck_count > 5 ||
		(mouseX == cx && mouseX == maze.GetCount()-1 && cr == WAIT) ||
		(!has_goal && mouseX == maze.GetCount()-2 && cr == WAIT);
	
	if (r == cr) {
		maze[mouseX][mouseY].has_mouse = false;
		mouseX = cx;
		mouseY = cy;
		maze[mouseX][mouseY].has_mouse = true;
	}
	else {
		stuck_count++;
	}
	
	if (stuck_in_last) {
		running = false;
		
		// Place cheese in selected goal room.
		for (int i = 0; i < maze[end_maze_index + 1].GetCount(); i++) {
			if (maze[end_maze_index + 1][i].selected) {
				maze[end_maze_index + 1][i].has_cheese = true;
				break;
			}
		}

		// Move mouse back to leftmost selected room.
		if (mouseX != -1) {
			for (int i = 0; i < maze.GetCount(); i++) {
				for (int j = 0; j < maze[i].GetCount(); j++) {
					if (maze[i][j].selected) {
						maze[mouseX][mouseY].has_mouse = false;
						maze[i][j].has_mouse           = true;
						mouseX = i;
						mouseY = j;
						return;
					}
				}
			}
		}
	}
}



bool MazeMouseApplication::Key(dword key, int count) {
	/*int key = evt.getKeyCode();

	switch (key) {
	case 'p':
	case 'P':
		// Print neural network.
		mona.Print();
		return true;
	}
	*/
	return false;
}
