#ifndef _MazeMouse_MazeMouseApplication_h_
#define _MazeMouse_MazeMouseApplication_h_

#include "Room.h"


class MazeMouseApplication : public TopWindow {
	
protected:
	
	// Sizes.
	int WIDTH, HEIGHT;
	
	// Cheese need and goal.
	double CHEESE_NEED, CHEESE_GOAL;

	// Modes
	int TRAIN, RUN;

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
	Vector<double> sensors;

	// Maze.
	Array<Array<Room> > maze;
	Size roomSize;
	int       begin_maze_index;
	int       end_maze_index;
	int       mouseX;
	int       mouseY;

	// Are maze room doors all visible?
	// Some of these doors are blocked.
	bool see_all_maze_doors;

	Size mazeSize;
	
	// Display.
	/*Size screen_size;
	Canvas    mazeCanvas;
	Graphics  canvasGraphics;
	Image     mazeImage;
	Graphics  imageGraphics;
	String    error;

	// Controls.
	JPanel    controls;
	JCheckBox start;
	Choice    mode;
	TextField response;
	JCheckBox mute;
	JButton   reset;
	JButton   help;*/
	bool   helpDisplay;

	// Font.
	Font        font;
	//FontMetrics fontMetrics;
	//int         fontAscent;
	//int         fontWidth;
	//int         fontHeight;

	// Images and sound.
	Image     mouse_image;
	Image     cheese_image;
	//AudioClip squeakSound;

	// Threads.
	//private Thread displayThread;
	//private Thread stepThread;
	//Toolkit        toolkit;
	
public:
	// Constructor.
	MazeMouseApplication();


	// Create Mona.
	void CreateMona(int random_seed);


	// Run.
	void Runner();


	// Display.
	
	/*synchronized void display() {
		// Clear display.
		imageGraphics.setColor(Color.white);
		imageGraphics.fillRect(0, 0, mazeSize.width, mazeSize.height);

		if (helpDisplay) {
			imageGraphics.setColor(Color.black);

			for (int i = 0; i < HELP_TEXT.length; i++)
				imageGraphics.drawString(HELP_TEXT[i], 1, (i * (fontHeight + 1)) + fontHeight);
		}
		else {
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
		}

		// Refresh display.
		canvasGraphics.drawImage(mazeImage, 0, 0, this);
	}*/


	// Step.
	void Step();


	// Button listener.
	void ButtonPressed();
	

	// CheckBox listener.
	void ItemStateChanged();

	// Mouse press.
	virtual void LeftDown(Point p, dword keyflags);

	// Key press.
	virtual bool Key(dword key, int count);

};



#endif
