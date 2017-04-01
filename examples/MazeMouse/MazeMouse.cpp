#if 0

#include "MazeMouse.h"


MazeMouse::MazeMouse() {
	CHEESE_NEED = 1.0;
	CHEESE_GOAL = 0.5;
	// Modes
	TRAIN = 0;
	RUN   = 1;
	STEP_DELAY = 1000;
	DISPLAY_DELAY = 50;
	MOUSE_IMAGE_FILE  = "mouse.gif";
	CHEESE_IMAGE_FILE = "cheese.jpg";
	QUEAK_SOUND_FILE = "squeak.wav";
	// Are maze room doors all visible?
	// Some of these doors are blocked.
	see_all_maze_doors = false;
}

void MazeMouse::Init() {
	toolkit = Toolkit.getDefaultToolkit();
	// Identify server.
	error  = null;
	socket = null;
	in     = null;
	out    = null;

	if ((serverHost = getParameter("ServerHost")) == null)
		serverHost = DEFAULT_SERVER_HOST;

	String s;

	if ((s = getParameter("ServerPort")) == null)
		serverPort = DEFAULT_SERVER_PORT;
	else
		serverPort = Integer.parseInt(s);

	serverAddress = null;

	try {
		serverAddress = InetAddress.getByName(serverHost);
	}
	catch (Exception e) {
		error = "Cannot get address for host " + serverHost + ": " + e.toString();
	}

	// Create the maze display.
	JPanel screen = (JPanel)getContentPane();
	screen.setLayout(new FlowLayout());
	screen_size = getSize();
	mazeSize   = new Size(screen_size.width,
						  (int)((double)screen_size.height * .90));
	mazeCanvas = new Canvas();
	mazeCanvas.setSize(mazeSize.width, mazeSize.height);
	mazeCanvas.addMouseListener(new MazeMouseListener());
	mazeCanvas.addKeyListener(new MazeKeyListener());
	screen.Add(mazeCanvas);
	mazeImage = createImage(mazeSize.width, mazeSize.height);
	// Get font.
	font = new Font("Helvetica", Font.BOLD, 12);
	// Add controls.
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
	screen.Add(controls);
	logging = false;
	// No threads yet.
	displayThread = stepThread = null;
	// Build maze.
	int x;
	int y;
	Room[] doors;
	roomSize       = new Size(mazeSize.width / 15, mazeSize.height / 11);
	maze           = new Room[7][];
	maze[0]        = new Room[3];
	x              = roomSize.width;
	y              = (mazeSize.height / 2) - (roomSize.height / 2);
	maze[0][1]     = new Room(x, y, START_ROOM, 0, 1);
	maze[0][0]     = new Room(x, y - (roomSize.height * 2), START_ROOM, 0, 0);
	maze[0][2]     = new Room(x, y + (roomSize.height * 2), START_ROOM, 0, 2);
	begin_maze_index = 1;
	maze[1]        = new Room[1];
	x             += (roomSize.width * 2);
	y              = (mazeSize.height / 2) - (roomSize.height / 2);
	maze[1][0]     = new Room(x, y, BEGIN_MAZE, 1, 0);
	doors          = new Room[3];
	doors[0]       = null;
	doors[1]       = null;
	doors[2]       = maze[1][0];
	maze[0][0].setDoors(doors);
	doors    = new Room[3];
	doors[0] = null;
	doors[1] = maze[1][0];
	doors[2] = null;
	maze[0][1].setDoors(doors);
	doors    = new Room[3];
	doors[0] = maze[1][0];
	doors[1] = null;
	doors[2] = null;
	maze[0][2].setDoors(doors);
	maze[2]    = new Room[3];
	x         += (roomSize.width * 2);
	y          = (mazeSize.height / 2) - (roomSize.height / 2);
	maze[2][1] = new Room(x, y, MAZE_ROOM, 2, 1);
	maze[2][0] = new Room(x, y - (roomSize.height * 2), MAZE_ROOM, 2, 0);
	maze[2][2] = new Room(x, y + (roomSize.height * 2), MAZE_ROOM, 2, 2);
	doors      = new Room[3];
	doors[0]   = maze[2][0];
	doors[1]   = maze[2][1];
	doors[2]   = maze[2][2];
	maze[1][0].setDoors(doors);
	maze[3]    = new Room[5];
	x         += (roomSize.width * 2);
	y          = (mazeSize.height / 2) - (roomSize.height / 2);
	maze[3][2] = new Room(x, y, MAZE_ROOM, 3, 2);
	maze[3][1] = new Room(x, y - (roomSize.height * 2), MAZE_ROOM, 3, 1);
	maze[3][0] = new Room(x, y - (roomSize.height * 4), MAZE_ROOM, 3, 0);
	maze[3][3] = new Room(x, y + (roomSize.height * 2), MAZE_ROOM, 3, 3);
	maze[3][4] = new Room(x, y + (roomSize.height * 4), MAZE_ROOM, 3, 4);
	doors      = new Room[3];
	doors[0]   = maze[3][0];
	doors[1]   = maze[3][1];
	doors[2]   = maze[3][2];
	maze[2][0].setDoors(doors);
	doors    = new Room[3];
	doors[0] = maze[3][1];
	doors[1] = maze[3][2];
	doors[2] = maze[3][3];
	maze[2][1].setDoors(doors);
	doors    = new Room[3];
	doors[0] = maze[3][2];
	doors[1] = maze[3][3];
	doors[2] = maze[3][4];
	maze[2][2].setDoors(doors);
	maze[4]    = new Room[3];
	x         += (roomSize.width * 2);
	y          = (mazeSize.height / 2) - (roomSize.height / 2);
	maze[4][1] = new Room(x, y, MAZE_ROOM, 4, 1);
	maze[4][0] = new Room(x, y - (roomSize.height * 2), MAZE_ROOM, 4, 0);
	maze[4][2] = new Room(x, y + (roomSize.height * 2), MAZE_ROOM, 4, 2);
	doors      = new Room[3];
	doors[0]   = null;
	doors[1]   = null;
	doors[2]   = maze[4][0];
	maze[3][0].setDoors(doors);
	doors    = new Room[3];
	doors[0] = null;
	doors[1] = maze[4][0];
	doors[2] = maze[4][1];
	maze[3][1].setDoors(doors);
	doors    = new Room[3];
	doors[0] = maze[4][0];
	doors[1] = maze[4][1];
	doors[2] = maze[4][2];
	maze[3][2].setDoors(doors);
	doors    = new Room[3];
	doors[0] = maze[4][1];
	doors[1] = maze[4][2];
	doors[2] = null;
	maze[3][3].setDoors(doors);
	doors    = new Room[3];
	doors[0] = maze[4][2];
	doors[1] = null;
	doors[2] = null;
	maze[3][4].setDoors(doors);
	end_maze_index = 5;
	maze[5]      = new Room[1];
	x           += (roomSize.width * 2);
	y            = (mazeSize.height / 2) - (roomSize.height / 2);
	maze[5][0]   = new Room(x, y, END_MAZE, 5, 0);
	doors        = new Room[3];
	doors[0]     = null;
	doors[1]     = null;
	doors[2]     = maze[5][0];
	maze[4][0].setDoors(doors);
	doors    = new Room[3];
	doors[0] = null;
	doors[1] = maze[5][0];
	doors[2] = null;
	maze[4][1].setDoors(doors);
	doors    = new Room[3];
	doors[0] = maze[5][0];
	doors[1] = null;
	doors[2] = null;
	maze[4][2].setDoors(doors);
	maze[6]    = new Room[3];
	x         += (roomSize.width * 2);
	y          = (mazeSize.height / 2) - (roomSize.height / 2);
	maze[6][1] = new Room(x, y, GOAL_ROOM, 6, 1);
	maze[6][0] = new Room(x, y - (roomSize.height * 2), GOAL_ROOM, 6, 0);
	maze[6][2] = new Room(x, y + (roomSize.height * 2), GOAL_ROOM, 6, 2);
	doors      = new Room[3];
	doors[0]   = maze[6][0];
	doors[1]   = maze[6][1];
	doors[2]   = maze[6][2];
	maze[5][0].setDoors(doors);
	doors    = new Room[3];
	doors[0] = null;
	doors[1] = null;
	doors[2] = null;
	maze[6][0].setDoors(doors);
	maze[6][1].setDoors(doors);
	maze[6][2].setDoors(doors);
	mouseX = mouseY = 0;
	maze[0][0].has_mouse = true;
	maze[0][0].selected = true;
	maze[end_maze_index + 1][2].has_cheese = true;
	maze[end_maze_index + 1][2].selected  = true;
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
		showStatus("Cannot get image: " + MOUSE_IMAGE_FILE);

	imgURL = MazeMouse.class .getResource(CHEESE_IMAGE_FILE);

	if (imgURL != null)
		cheese = new ImageIcon(imgURL).getImage();

	if (cheese == null)
		showStatus("Cannot get image: " + CHEESE_IMAGE_FILE);

	if (mouse != null) {
		w1 = mouse.GetWidth(this);
		h1 = mouse.GetHeight(this);

		if ((w1 <= 0) || (h1 <= 0))
			showStatus("Invalid image: " + MOUSE_IMAGE_FILE);
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
			showStatus("Invalid image: " + CHEESE_IMAGE_FILE);
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
		showStatus("Cannot load sound file: " + SQUEAK_SOUND_FILE);
	else {
		// Playing and stopping ensures sound completely loaded.
		squeakSound.play();
		squeakSound.Stop();
	}
}

void MazeMouse::Start() {
	// Connect to Mona server, and initialize parameters.
	if (error == null) {
		try {
			socket = new Socket(serverAddress, serverPort);
			in     = new BufferedReader(new InputStreamReader(
											socket.getInputStream()));
			out = new PrintWriter(new BufferedWriter(
									  new OutputStreamWriter(socket.getOutputStream())),
								  true);
			// #sensors=5, #responses=HOP+1, #needs=1
			out.println("p 5 " + (HOP + 1) + " 1");
			out.flush();
			// Get MAX_MEDIATOR_LEVEL parameter string.
			String parmStr = in.readLine();

			if ((parmStr == null) ||
				!parmStr.startsWith("MAX_MEDIATOR_LEVEL="))
				throw new Exception("Invalid MAX_MEDIATOR_LEVEL");

			try {
				int maxmed = Integer.parseInt(parmStr.substring(parmStr.indexOf(
																	"=") + 1).trim());

				if (maxmed < 1)
					throw new Exception();
			}
			catch (Exception e) {
				throw new Exception("Invalid MAX_MEDIATOR_LEVEL");
			}

			// Set a long second effect interval
			// for a higher level mediator.
			out.println("s 1 2");
			out.flush();
			out.println("i 1 0 2 0.5");
			out.flush();
			out.println("i 1 1 10 0.5");
			out.flush();
			// Set need and goal for cheese.
			out.println("n 0 " + CHEESE_NEED);
			out.flush();
			out.println("g 0 0.0 0.0 0.0 " + (double)GOAL_ROOM +
						" 1.0 null " + CHEESE_GOAL);
			out.flush();
		}
		catch (Exception e) {
			error = "Cannot connect to server: " + serverHost + ":" +
					serverPort + " Error: " + e.toString();
		}
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
}

void MazeMouse::Stop() {
	if (displayThread != null) {
		displayThread.interrupt();

		try {
			displayThread.join();
		}
		catch (InterruptedException e) {
		}

		displayThread = null;
	}

	if (stepThread != null) {
		stepThread.interrupt();

		try {
			stepThread.join();
		}
		catch (InterruptedException e) {
		}

		stepThread = null;
	}

	if (socket != null) {
		out.println("quit");
		out.flush();

		try {
			socket.close();
		}
		catch (Exception e) {
		}

		socket = null;
	}
}

void MazeMouse::Run() {
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

void MazeMouse::Display() {
	// Get graphics.
	if (canvasGraphics == null) {
		if ((canvasGraphics = mazeCanvas.getGraphics()) == null)
			return;

		imageGraphics = mazeImage.getGraphics();
		canvasGraphics.setFont(font);
		imageGraphics.setFont(font);
		fontMetrics = canvasGraphics.getFontMetrics();
		fontAscent  = fontMetrics.getMaxAscent();
		fontWidth   = fontMetrics.getMaxAdvance();
		fontHeight  = fontMetrics.GetHeight();
	}

	// Clear display.
	imageGraphics.setColor(Color.white);
	imageGraphics.fillRect(0, 0, mazeSize.width, mazeSize.height);

	// Draw maze.
	for (int i = 0; i < maze.length; i++) {
		for (int j = 0; j < maze[i].length; j++)
			maze[i][j].draw(imageGraphics);
	}

	// Label maze.
	int h = fontHeight / 2;
	imageGraphics.drawString("Start", maze[0][0].x + 4, maze[0][0].y - h);
	imageGraphics.drawString("Entry", maze[begin_maze_index][0].x + 4,
							 maze[begin_maze_index][0].y - h);
	imageGraphics.drawString("Maze", maze[begin_maze_index + 1][0].x + 6,
							 maze[begin_maze_index + 1][0].y - h);
	imageGraphics.drawString("Maze", maze[begin_maze_index + 2][0].x + 6,
							 maze[begin_maze_index + 2][0].y - h);
	imageGraphics.drawString("Maze", maze[begin_maze_index + 3][0].x + 6,
							 maze[begin_maze_index + 3][0].y - h);
	imageGraphics.drawString("Exit", maze[end_maze_index][0].x + 6,
							 maze[end_maze_index][0].y - h);
	imageGraphics.drawString("Goal", maze[end_maze_index + 1][0].x + 6,
							 maze[end_maze_index + 1][0].y - h);

	// Show error message?
	if (error != null) {
		imageGraphics.setColor(Color.white);
		int w = fontMetrics.stringWidth(error);
		imageGraphics.fillRect(((mazeSize.width - w) / 2) - 2,
							   (mazeSize.height / 2) - (fontAscent + 2), w + 4, fontHeight +
							   4);
		imageGraphics.setColor(Color.black);
		imageGraphics.drawString(error, (mazeSize.width - w) / 2,
								 mazeSize.height / 2);
	}

	// Refresh display.
	canvasGraphics.drawImage(mazeImage, 0, 0, this);
}

void MazeMouse::Step() {
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
		out.println("r " + cr);
	else
		out.println("r null");

	out.flush();
	// Initiate sensory/response cycle.
	out.Print("c ");

	for (i = 0; i < maze[mouseX][mouseY].doors.length; i++) {
		if (maze[mouseX][mouseY].doors[i] != null)
			out.Print("1.0 ");
		else if (see_all_maze_doors && (mouseX > 1) && (mouseX < 5)) {
			// Maze room door is visible although possibly blocked.
			out.Print("1.0 ");
		}
		else
			out.Print("0.0 ");
	}

	out.Print((double)maze[mouseX][mouseY].type + " ");

	if (maze[mouseX][mouseY].has_cheese) {
		out.println("1.0");
		// Eat the cheese!
		maze[mouseX][mouseY].has_cheese = false;

		if ((squeakSound != null) && !mute.isSelected())
			squeakSound.play();
	}
	else
		out.println("0.0");

	out.flush();

	// Move mouse based on response.
	try {
		r = Integer.parseInt(in.readLine());
	}
	catch (Exception e) {
		error = "Error getting response from server: " + e.toString();
		return;
	}

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

void MazeMouse::ActionPerformed(ActionEvent evt) {
	if (error != null)
		return;

	if (evt.getSource() == reset) {
		// Erase mouse memory.
		out.println("e stm");
		out.flush();
		out.println("e ltm");
		out.flush();

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

void MazeMouse::ItemStateChanged(ItemEvent evt) {
	if (error != null)
		return;

	if (evt.getSource() == start) {
		if (start.isSelected()) {
			start.SetLabel("Stop Trial");
			// Reset need.
			out.println("n 0 " + CHEESE_NEED);
			out.flush();
			// Clear working memory.
			out.println("e stm");
			out.flush();
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


void MazeMouse::LeftDown(Point p, dword keyflags) {
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


bool MazeMouse::Key(dword key, int count) {
	int key = evt.getKeyCode();

	switch (key) {
	case 'l':
	case 'L':
		// Toggle logging.
		logging = !logging;

		if (logging) {
			showStatus("logging on");
			out.println("l on");
		}
		else {
			showStatus("logging off");
			out.println("l off");
		}

		out.flush();
		break;

	case 'd':
	case 'D':
		// Dump neural network.
		showStatus("dump network");
		out.println("d");
		out.flush();
		break;
	}
}
#endif
