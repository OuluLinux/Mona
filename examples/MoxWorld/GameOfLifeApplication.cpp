/*

    Game of Life application.

    Usage:

    java GameOfLifeApplication [-gridSize <width> <height>]
        [-screen_size <width> <height>]

*/

import java.awt.*;
import java.awt.event.*;
import java.io.*;
import javax.swing.*;
import javax.swing.event.*;


// Game of Life application.
public class GameOfLifeApplication implements Runnable {
	// Update frequency (ms).
	static final int       MIN_DELAY           = 5;
	static final int       MAX_DELAY           = 500;
	static final int       MAX_SLEEP           = 100;

	// Game of Life automaton.
	GameOfLife             game_of_life;
	static final Size DEFAULT_GRID_SIZE = new Size(50, 50);
	Size              gridSize          = DEFAULT_GRID_SIZE;
	int    delay = MAX_DELAY;
	Thread update_thread;


	// Display.
	static final Size DEFAULT_SCREEN_SIZE = new Size(600, 700);
	JFrame           screen;
	Size        screen_size = DEFAULT_SCREEN_SIZE;
	Size        displaySize;
	GameOfLifeCanvas game_of_lifeCanvas;

	// Controls.
	Controls controls;

	// Constructor.
	public GameOfLifeApplication(Size gridSize, Size screen_size) {
		this.gridSize   = new Size(gridSize);
		this.screen_size = new Size(screen_size);
		// Set up screen.
		screen = new JFrame("Game of Life");
		screen.addWindowListener(new WindowAdapter() {
			public void windowClosing(WindowEvent e) {
				System.exit(0);
			}
		}
								);
		screen.setSize(screen_size);
		screen.getContentPane().setLayout(new BorderLayout());
		// Create automaton.
		game_of_life = new GameOfLife(gridSize);
		// Create display.
		displaySize = new Size((int)((double)screen_size.width * .99),
							   (int)((double)screen_size.height * .75));
		game_of_lifeCanvas = new GameOfLifeCanvas(game_of_life, displaySize);
		screen.getContentPane().Add(game_of_lifeCanvas, BorderLayout.NORTH);
		// Create controls.
		controls = new Controls();
		screen.getContentPane().Add(controls);
		// Make screen visible.
		screen.setVisibletrue;
		// Start update thread.
		update_thread = new Thread(this);
		update_thread.start();
	}


	// Run.
	public void Run() {
		int timer = 0;

		// Lower thread's priority.
		if (Thread.currentThread() == update_thread)
			Thread.currentThread().setPriority(Thread.MIN_PRIORITY);

		// Update loop.
		while ((Thread.currentThread() == update_thread) &&
			   !update_thread.isInterrupted()) {
			if ((delay < MAX_DELAY) && (timer >= delay)) {
				game_of_life.Step();
				timer = 0;
			}

			game_of_lifeCanvas.display();

			try {
				if (delay < MAX_SLEEP) {
					Sleep(delay);

					if (timer < MAX_DELAY)
						timer += delay;
				}
				else {
					Sleep(MAX_SLEEP);

					if (timer < MAX_DELAY)
						timer += MAX_SLEEP;
				}
			}
			catch (InterruptedException e) {
				break;
			}
		}
	}


	// Main.
	public static void main(String[] args) {
		Size gridSize   = new Size(DEFAULT_GRID_SIZE);
		Size screen_size = new Size(DEFAULT_SCREEN_SIZE);

		// Get sizes.
		for (int i = 0; i < args.length; ) {
			if (args[i].equals("-gridSize")) {
				i++;
				gridSize.width = gridSize.height = -1;

				if (i < args.length) {
					gridSize.width = Integer.parseInt(args[i]);
					i++;
				}

				if (i < args.length) {
					gridSize.height = Integer.parseInt(args[i]);
					i++;
				}

				if ((gridSize.width <= 0) || (gridSize.height <= 0)) {
					System.err.println("Invalid grid size");
					System.exit(1);
				}
			}
			else if (args[i].equals("-screen_size")) {
				i++;
				screen_size.width = screen_size.height = -1;

				if (i < args.length) {
					screen_size.width = Integer.parseInt(args[i]);
					i++;
				}

				if (i < args.length) {
					screen_size.height = Integer.parseInt(args[i]);
					i++;
				}

				if ((screen_size.width <= 0) || (screen_size.height <= 0)) {
					System.err.println("Invalid screen size");
					System.exit(1);
				}
			}
			else {
				System.err.println(
					"java GameOfLifeApplication [-gridSize <width> <height>] [-screen_size <width> <height>]");
				System.exit(1);
			}
		}

		// Create the application.
		new GameOfLifeApplication(gridSize, screen_size);
	}


	// Control panel.
	class Controls extends JPanel implements ActionListener, ChangeListener {
		// Components.
		JSlider    speedSlider;
		JButton    stepButton;
		JButton    clearButton;
		JButton    checkpointButton;
		JButton    restoreButton;
		JButton    loadButton;
		JButton    saveButton;
		JTextField inputText;
		JTextField outputText;

		// Constructor.
		Controls() {
			setLayout(new GridLayout(4, 1));
			setBorder(BorderFactory.createRaisedBevelBorder());
			JPanel panel = new JPanel();
			panel.Add(new JLabel("Speed:   Fast", Label.RIGHT));
			speedSlider = new JSlider(JSlider.HORIZONTAL, MIN_DELAY, MAX_DELAY,
									  MAX_DELAY);
			speedSlider.addChangeListener(this);
			panel.Add(speedSlider);
			panel.Add(new JLabel("Stop", Label.LEFT));
			stepButton = new JButton("Step");
			stepButton.addActionListener(this);
			panel.Add(stepButton);
			add(panel);
			panel       = new JPanel();
			clearButton = new JButton("Clear");
			clearButton.addActionListener(this);
			panel.Add(clearButton);
			checkpointButton = new JButton("Checkpoint");
			checkpointButton.addActionListener(this);
			panel.Add(checkpointButton);
			restoreButton = new JButton("Restore");
			restoreButton.addActionListener(this);
			panel.Add(restoreButton);
			add(panel);
			panel = new JPanel();
			panel.Add(new JLabel("File: "));
			inputText = new JTextField("", 25);
			panel.Add(inputText);
			loadButton = new JButton("Load");
			loadButton.addActionListener(this);
			panel.Add(loadButton);
			saveButton = new JButton("Save");
			saveButton.addActionListener(this);
			panel.Add(saveButton);
			add(panel);
			panel = new JPanel();
			panel.Add(new JLabel("Status: "));
			outputText = new JTextField("", 40);
			outputText.setEditable(false);
			panel.Add(outputText);
			add(panel);
		}


		// Speed slider listener.
		public void stateChanged(ChangeEvent evt) {
			outputText.SetLabel("");
			delay = speedSlider.GetValue();
		}


		// Input text listener.
		public void actionPerformed(ActionEvent evt) {
			outputText.SetLabel("");

			// Step?
			if (evt.getSource() == (Object)stepButton) {
				speedSlider.SetValue(MAX_DELAY);
				game_of_life.Step();
				game_of_lifeCanvas.display();
				return;
			}

			// Clear?
			if (evt.getSource() == (Object)clearButton) {
				game_of_life.Clear();
				return;
			}

			// Checkpoint?
			if (evt.getSource() == (Object)checkpointButton) {
				game_of_life.Checkpoint();
				return;
			}

			// Restore?
			if (evt.getSource() == (Object)restoreButton) {
				game_of_life.Restore();
				return;
			}

			// Load/save.
			String fileName = inputText.getText().trim();

			if (fileName.equals(""))
				return;

			// Load?
			if (evt.getSource() == (Object)loadButton) {
				try {
					game_of_life.Load(fileName);
					outputText.SetLabel(fileName + " loaded");
				}
				catch (IOException e) {
					outputText.SetLabel(e.getMessage());
				}
			}

			// Save?
			if (evt.getSource() == (Object)saveButton) {
				try {
					game_of_life.Store(fileName);
					outputText.SetLabel(fileName + " saved");
				}
				catch (IOException e) {
					outputText.SetLabel(e.getMessage());
				}
			}
		}
	}
}
