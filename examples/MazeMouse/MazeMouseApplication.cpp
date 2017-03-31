#include "MazeMouseApplication.h"

MazeMouseApplication::MazeMouseApplication() {
	
	// Sizes.
	WIDTH  = 700;
	HEIGHT = 500;
	
	// Cheese need and goal.
	CHEESE_NEED = 1.0;
	CHEESE_GOAL = 0.5;

	// Modes
	TRAIN = 0;
	RUN   = 1;

	// Random seed.
	DEFAULT_RANDOM_SEED = 7671;

	// Step delay (ms).
	STEP_DELAY = 1000;

	// Display update frequency (ms).
	DISPLAY_DELAY = 50;

	// Mouse and cheese images.
	MOUSE_IMAGE_FILE  = "mouse.gif";
	CHEESE_IMAGE_FILE = "cheese.jpg";

	// Mouse squeak sound.
	SQUEAK_SOUND_FILE = "squeak.wav";

	HELP_TEXT =
		"Create a path for the mouse to follow to the cheese by clicking on a sequence of rooms from one of the","\n",
		"start rooms through the maze to one of the cheese goal rooms. Then train the mouse by selecting train","\n",
		"mode and running a few trials. You run a trial by clicking the start trial control which starts the","\n",
		"mouse moving through the rooms. When the mouse finishes moving, click stop trial. When the mouse is","\n",
		"training, it is being shown the correct responses to make in each room. Now use test mode to test the","\n",
		"mouse to see if it can find the cheese on its own!","\n",
		"","\n",
		"Now try something interesting! First click reset to make the mouse forget everything.","\n",
		"Then select just one of the start rooms, the entry, exit, and one of the goal rooms. The mouse will","\n",
		"\"hop\" over the maze. Train that a few times and test the mouse on it.","\n",
		"","\n",
		"Now train a path through just the maze by selecting the entry room, three connected maze rooms, and the","\n",
		"exit room. Don't bother to test that now since the mouse didn't find any cheese that would make it want","\n",
		"to do that on its own.","\n",
		"","\n",
		"Now comes the tricky part. Combine the paths that you trained separately into one path by selecting all","\n",
		"the rooms from the start to the entry, then through the maze to the exit, then to the cheese goal. Test","\n",
		"the mouse a few times to see if it can find the cheese. You may see it do an extra hop at the entry that","\n",
		"it doesn't need to, but that's OK. If the mouse can find the cheese that means that it can combine things","\n",
		"that it knows together to solve the problem!","\n",
		"","\n",
		"You can try other fancy stuff like training more than one start->entry->exit->goal room path, or changing","\n",
		"the maze path while keeping the start->entry->exit->goal room path the same.","\n",
		"","\n",
		"Type 'p' to print the network.";
	
	see_all_maze_doors = false;
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	// Set title.
	Title("Maze-learning Mouse");
	
	// Create mona.
	CreateMona(Random());
	
	
	/*// Add controls.
	controls = new JPanel();
	controls.setLayout(new FlowLayout());
	start = new JCheckBox("Start Trial");
	start.addItemListener(this);
	start.setToolTipText("Go mouse!");
	controls.Add(start);
	controls.Add(new JLabel("Mode:"));
	mode = new Choice();
	mode.Add("Train");
	mode.Add("Test");
	controls.Add(mode);
	controls.Add(new JLabel("Mouse Response:"));
	response = new TextField(10);
	response.setEditable(false);
	controls.Add(response);
	mute = new JCheckBox("Mute");
	mute.setToolTipText("No squeaking!");
	controls.Add(mute);
	reset = new JButton("Reset");
	reset.addActionListener(this);
	controls.Add(reset);
	help = new JButton("Help");
	help.addActionListener(this);
	controls.Add(help);
	screen.Add(controls, BorderLayout.SOUTH);
	helpDisplay = false;*/
	
	
	// Build maze.
	int x;
	int y;
	
	mazeSize		= Size(WIDTH, HEIGHT);
	roomSize		= Size(mazeSize.cx / 15, mazeSize.cy / 11);
	maze.SetCount(7);
	maze[0].SetCount(3);
	maze[1].SetCount(1);
	maze[2].SetCount(3);
	maze[3].SetCount(5);
	maze[4].SetCount(3);
	maze[5].SetCount(1);
	maze[6].SetCount(3);
	
	x              = roomSize.cx;
	y              = (mazeSize.cy / 2) - (roomSize.cy / 2);
	maze[0][1]     .Init(x, y, START_ROOM, 0, 1);
	maze[0][0]     .Init(x, y - (roomSize.cy * 2), START_ROOM, 0, 0);
	maze[0][2]     .Init(x, y + (roomSize.cy * 2), START_ROOM, 0, 2);
	begin_maze_index = 1;
	x             += (roomSize.cx * 2);
	y              = (mazeSize.cy / 2) - (roomSize.cy / 2);
	maze[1][0]     .Init(x, y, BEGIN_MAZE, 1, 0);
	maze[0][0].SetDoors(NULL, NULL, &maze[1][0]);
	maze[0][1].SetDoors(NULL, &maze[1][0], NULL);
	maze[0][2].SetDoors(&maze[1][0], NULL, NULL);
	x         += (roomSize.cx * 2);
	y          = (mazeSize.cy / 2) - (roomSize.cy / 2);
	maze[2][1] .Init(x, y, MAZE_ROOM, 2, 1);
	maze[2][0] .Init(x, y - (roomSize.cy * 2), MAZE_ROOM, 2, 0);
	maze[2][2] .Init(x, y + (roomSize.cy * 2), MAZE_ROOM, 2, 2);
	maze[1][0].SetDoors(&maze[2][0], &maze[2][1], &maze[2][2]);
	x         += (roomSize.cx * 2);
	y          = (mazeSize.cy / 2) - (roomSize.cy / 2);
	maze[3][2] .Init(x, y, MAZE_ROOM, 3, 2);
	maze[3][1] .Init(x, y - (roomSize.cy * 2), MAZE_ROOM, 3, 1);
	maze[3][0] .Init(x, y - (roomSize.cy * 4), MAZE_ROOM, 3, 0);
	maze[3][3] .Init(x, y + (roomSize.cy * 2), MAZE_ROOM, 3, 3);
	maze[3][4] .Init(x, y + (roomSize.cy * 4), MAZE_ROOM, 3, 4);
	maze[2][0].SetDoors(&maze[3][0], &maze[3][1], &maze[3][2]);
	maze[2][1].SetDoors(&maze[3][1], &maze[3][2], &maze[3][3]);
	maze[2][2].SetDoors(&maze[3][2], &maze[3][3], &maze[3][4]);
	x         += (roomSize.cx * 2);
	y          = (mazeSize.cy / 2) - (roomSize.cy / 2);
	maze[4][1] .Init(x, y, MAZE_ROOM, 4, 1);
	maze[4][0] .Init(x, y - (roomSize.cy * 2), MAZE_ROOM, 4, 0);
	maze[4][2] .Init(x, y + (roomSize.cy * 2), MAZE_ROOM, 4, 2);
	maze[3][0].SetDoors(NULL, NULL, &maze[4][0]);
	maze[3][1].SetDoors(NULL, &maze[4][0], &maze[4][1]);
	maze[3][2].SetDoors(&maze[4][0], &maze[4][1], &maze[4][2]);
	maze[3][3].SetDoors(&maze[4][1], &maze[4][2], NULL);
	maze[3][4].SetDoors(&maze[4][2], NULL, NULL);
	end_maze_index = 5;
	x           += (roomSize.cx * 2);
	y            = (mazeSize.cy / 2) - (roomSize.cy / 2);
	maze[5][0]   .Init(x, y, END_MAZE, 5, 0);
	maze[4][0].SetDoors(NULL, NULL, &maze[5][0]);
	maze[4][1].SetDoors(NULL, &maze[5][0], NULL);
	maze[4][2].SetDoors(&maze[5][0], NULL, NULL);
	x         += (roomSize.cx * 2);
	y          = (mazeSize.cy / 2) - (roomSize.cy / 2);
	maze[6][1] .Init(x, y, GOAL_ROOM, 6, 1);
	maze[6][0] .Init(x, y - (roomSize.cy * 2), GOAL_ROOM, 6, 0);
	maze[6][2] .Init(x, y + (roomSize.cy * 2), GOAL_ROOM, 6, 2);
	maze[5][0].SetDoors(&maze[6][0], &maze[6][1], &maze[6][2]);
	maze[6][0].SetDoors(NULL, NULL, NULL);
	maze[6][1].SetDoors(NULL, NULL, NULL);
	maze[6][2].SetDoors(NULL, NULL, NULL);
	mouseX = mouseY = 0;
	maze[0][0].has_mouse = true;
	maze[0][0].selected = true;
	maze[end_maze_index + 1][2].has_cheese = true;
	maze[end_maze_index + 1][2].selected  = true;
	
	// Show app.
	Pack();
	SetVisible(true);

/*
	mazeImage     = createImage(mazeSize.width, mazeSize.height);
	imageGraphics = mazeImage.getGraphics();
	// Set font.
	font = new Font("Helvetica", Font.PLAIN, 12);
	canvasGraphics.setFont(font);
	imageGraphics.setFont(font);
	fontMetrics = canvasGraphics.getFontMetrics();
	fontAscent  = fontMetrics.getMaxAscent();
	fontWidth   = fontMetrics.getMaxAdvance();
	fontHeight  = fontMetrics.GetHeight();
	// Get mouse and cheese images.
	Image mouse = null;
	mouse_image = null;
	Image cheese = null;
	cheese_image = null;
	int w1;
	int h1;
	int w2;
	int h2;
	URL imgURL = MazeMouse.class .getResource(MOUSE_IMAGE_FILE);

	if (imgURL != null)
		mouse = new ImageIcon(imgURL).getImage();

	if (mouse == null)
		error = "Cannot get image: " + MOUSE_IMAGE_FILE;

	imgURL = MazeMouse.class .getResource(CHEESE_IMAGE_FILE);

	if (imgURL != null)
		cheese = new ImageIcon(imgURL).getImage();

	if (cheese == null)
		error = "Cannot get image: " + CHEESE_IMAGE_FILE;

	if (mouse != null) {
		w1 = mouse.GetWidth(this);
		h1 = mouse.GetHeight(this);

		if ((w1 <= 0) || (h1 <= 0))
			error = "Invalid image: " + MOUSE_IMAGE_FILE;
		else {
			w2         = roomSize.width / 2;
			h2         = roomSize.height / 2;
			mouse_image = createImage(w2, h2);
			Graphics graphics = mouse_image.getGraphics();
			graphics.drawImage(mouse, 0, 0, w2 - 1, h2 - 1, 0, 0, w1 - 1,
							   h1 - 1, Color.white, this);
		}
	}

	if (cheese != null) {
		w1 = cheese.GetWidth(this);
		h1 = cheese.GetHeight(this);

		if ((w1 <= 0) || (h1 <= 0))
			error = "Invalid image: " + CHEESE_IMAGE_FILE;
		else {
			w2          = roomSize.width / 2;
			h2          = roomSize.height / 2;
			cheese_image = createImage(w2, h2);
			Graphics graphics = cheese_image.getGraphics();
			graphics.drawImage(cheese, 0, 0, w2 - 1, h2 - 1, 0, 0, w1 - 1,
							   h1 - 1, Color.white, this);
		}
	}

	// Load mouse squeak sound.
	squeakSound = null;
	URL url = MazeMouse.class .getResource(SQUEAK_SOUND_FILE);

	if (url != null)
		squeakSound = Applet.newAudioClip(url);

	if (squeakSound == null)
		error = "Cannot load sound file: " + SQUEAK_SOUND_FILE;
	else {
		// Playing and stopping ensures sound completely loaded.
		squeakSound.play();
		squeakSound.Stop();
	}

	if (displayThread == null) {
		displayThread = new Thread(this);
		displayThread.setPriority(Thread.MIN_PRIORITY);
		displayThread.start();
	}

	if ((stepThread == null) && (error == null)) {
		stepThread = new Thread(this);
		stepThread.setPriority(Thread.MIN_PRIORITY);
		stepThread.start();
	}
	*/
}

void MazeMouseApplication::CreateMona(int random_seed) {
	Vector<String> keys   = new Vector<String>();
	Vector<Object> values = new Vector<Object>();
	keys.Add("NUM_SENSORS");
	values.Add("5");
	keys.Add("NUM_RESPONSES");
	values.Add((HOP + 1) + "");
	keys.Add("NUM_NEEDS");
	values.Add("1");
	keys.Add("MAX_MEDIATOR_LEVEL");
	values.Add("1");
	keys.Add("RANDOM_SEED");
	values.Add(random_seed + "");
	mona = new Mona(keys, values);
	// Set a long second effect interval
	// for a higher level mediator.
	mona.setEffectEventIntervals(1, 2);
	mona.setEffectEventInterval(1, 0, 2, 0.5f);
	mona.setEffectEventInterval(1, 1, 10, 0.5f);
	// Set need and goal for cheese.
	mona.SetNeed(0, CHEESE_NEED);
	sensors    = new float[5];
	sensors[0] = 0.0f;
	sensors[1] = 0.0f;
	sensors[2] = 0.0f;
	sensors[3] = (float)GOAL_ROOM;
	sensors[4] = 1.0f;
	mona.AddGoal(0, sensors, 0, CHEESE_GOAL);
}

void MazeMouseApplication::Runner() {
	// Display loop.
	while ((Thread.currentThread() == displayThread) &&
		   !displayThread.isInterrupted()) {
		display();

		try {
			Sleep(DISPLAY_DELAY);
		}
		catch (InterruptedException e) {
			break;
		}
	}

	// Step loop.
	while ((Thread.currentThread() == stepThread) &&
		   !stepThread.isInterrupted()) {
		Step();

		try {
			Sleep(STEP_DELAY);
		}
		catch (InterruptedException e) {
			break;
		}
	}
}

void MazeMouseApplication::Step() {
	if (!start.isSelected() || (mouseX == -1) || (error != null))
		return;

	// Determine correct response.
	int cx = -1;
	int cy = -1;
	int cr = -1;
	int r  = -1;
	int i;
	int j;

	for (i = mouseX + 1; (i < maze.length) && (cx == -1); i++) {
		for (j = 0; (j < maze[i].length) && (cx == -1); j++) {
			if (maze[i][j].selected) {
				cx = i;
				cy = j;
			}
		}
	}

	if (cx == (mouseX + 1)) {
		for (i = 0; (i < maze[mouseX][mouseY].doors.length) && (cr == -1);
			 i++) {
			if ((maze[mouseX][mouseY].doors[i] != null) &&
				(maze[mouseX][mouseY].doors[i].cx == cx) &&
				(maze[mouseX][mouseY].doors[i].cy == cy)) {
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

	if ((mode.getSelectedIndex() == TRAIN) || (cr == WAIT))
		mona.OverrideResponse(cr);
	else
		mona.ClearResponseOverride();

	// Initiate sensory/response cycle.
	for (i = 0; i < maze[mouseX][mouseY].doors.length; i++) {
		if (maze[mouseX][mouseY].doors[i] != null)
			sensors[i] = 1.0f;
		else if (see_all_maze_doors && (mouseX > 1) && (mouseX < 5)) {
			// Maze room door is visible although possibly blocked.
			sensors[i] = 1.0f;
		}
		else
			sensors[i] = 0.0f;
	}

	sensors[3] = (float)maze[mouseX][mouseY].type;

	if (maze[mouseX][mouseY].has_cheese) {
		sensors[4] = 1.0f;
		// Eat the cheese!
		maze[mouseX][mouseY].has_cheese = false;

		if ((squeakSound != null) && !mute.isSelected())
			squeakSound.play();
	}
	else
		sensors[4] = 0.0f;

	// Move mouse based on response.
	r = mona.Cycle(sensors);

	if (r < WAIT)
		response.SetLabel("Door " + r);
	else if (r == WAIT)
		response.SetLabel("Wait");
	else
		response.SetLabel("Hop");

	// Move mouse.
	if (r == cr) {
		maze[mouseX][mouseY].has_mouse = false;
		mouseX = cx;
		mouseY = cy;
		maze[mouseX][mouseY].has_mouse = true;
	}
}

void MazeMouseApplication::ButtonPressed() {
	if (evt.getSource() == help) {
		helpDisplay = !helpDisplay;

		if (helpDisplay)
			help.SetLabel("Back");
		else
			help.SetLabel("Help");

		return;
	}

	if (error != null)
		return;

	if (evt.getSource() == reset) {
		// Erase mouse memory.
		mona.ClearWorkingMemory();
		mona.ClearLongTermMemory();

		// Clear maze.
		for (int i = 0; i < maze.length; i++) {
			for (int j = 0; j < maze[i].length; j++) {
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
		start.setSelected(false);
		start.SetLabel("Start Trial");
		response.SetLabel("");
	}
}

void MazeMouseApplication::ItemStateChanged() {
	if (error != null)
		return;

	if (evt.getSource() == start) {
		if (start.isSelected()) {
			start.SetLabel("Stop Trial");
			// Reset need.
			mona.SetNeed(0, CHEESE_NEED);
			// Clear working memory.
			mona.ClearWorkingMemory();
		}
		else {
			start.SetLabel("Start Trial");
			response.SetLabel("");

			// Place cheese in selected goal room.
			for (int i = 0; i < maze[end_maze_index + 1].length; i++) {
				if (maze[end_maze_index + 1][i].selected) {
					maze[end_maze_index + 1][i].has_cheese = true;
					break;
				}
			}

			// Move mouse back to leftmost selected room.
			if (mouseX != -1) {
				for (int i = 0; i < maze.length; i++) {
					for (int j = 0; j < maze[i].length; j++) {
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
}

void MazeMouseApplication::LeftDown(Point p, dword keyflags) {
	if (error != null)
		return;

	// Request keyboard focus.
	requestFocus();
	// Select/deselect room.
	bool found = false;
	int     cx    = 0;
	int     cy    = 0;

	for (int i = 0; (i < maze.length) && !found; i++) {
		for (int j = 0; (j < maze[i].length) && !found; j++) {
			if (maze[i][j].contains(evt.getX(), evt.getY())) {
				found = true;
				cx    = i;
				cy    = j;
			}
		}
	}

	if (!found)
		return;

	if (start.isSelected()) {
		toolkit.beep();
		return;
	}

	maze[cx][cy].selected = !maze[cx][cy].selected;

	// Move mouse/cheese if necessary.
	if (maze[cx][cy].selected) {
		for (int i = 0; i < maze[cx].length; i++) {
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

		if (cx == (end_maze_index + 1))
			maze[cx][cy].has_cheese = true;
	}
	else {
		if (maze[cx][cy].has_mouse) {
			mouseX = mouseY = -1;

			for (int i = cx + 1; (i < maze.length) && (mouseX == -1);
				 i++) {
				for (int j = 0; (j < maze[i].length) && (mouseX == -1);
					 j++) {
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
}

bool MazeMouseApplication::Key(dword key, int count) {
	int key = evt.getKeyCode();

	switch (key) {
	case 'p':
	case 'P':
		// Print neural network.
		mona.Print();
		return true;
	}
	return false;
}
