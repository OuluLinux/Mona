/**
    Mox world dashboard.
*/

import java.util.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;


// Mox dashboard.
public class MoxWorldDashboard extends JFrame {
	// Moxen.
	ArrayList<Mox> moxen;

	// Moxen dashboards.
	ArrayList<Moxdashboard> moxendashboards;
	int currentMox;

	// Game of Life.
	GameOfLife game_of_life;

	// Title.
	static final String TITLE = "Mox world";

	// Sizes.
	static final Size DASHBOARD_SIZE = new Size(600, 700);

	// Mox display.
	MoxDisplay display;

	// Controls.
	MoxControls controls;

	// Step frequency (ms).
	static final int MIN_STEP_DELAY = 0;
	static final int MAX_STEP_DELAY = 1000;
	int              stepDelay      = MAX_STEP_DELAY;

	// Quit.
	bool quit;

	// Constructors.
	public MoxWorldDashboard(GameOfLife game_of_life, ArrayList<Mox> moxen) {
		this.game_of_life = game_of_life;
		setMoxen(moxen);
		currentMox = -1;
		Init();
	}


	public MoxWorldDashboard(GameOfLife game_of_life) {
		this.game_of_life = game_of_life;
		moxen           = new ArrayList<Mox>();
		setMoxen(moxen);
		currentMox = -1;
		Init();
	}


	// Initialize.
	void Init() {
		// Set up dashboard.
		setTitle(TITLE);
		addWindowListener(new WindowAdapter() {
			public void windowClosing(WindowEvent e) {
				reset();
				quit = true;
			}
		}
						 );
		setBounds(0, 0, DASHBOARD_SIZE.width, DASHBOARD_SIZE.height);
		JPanel basePanel = (JPanel)getContentPane();
		basePanel.setLayout(new BorderLayout());
		// Create display.
		Size displaySize = new Size((int)((double)DASHBOARD_SIZE.width * .99),
									(int)((double)DASHBOARD_SIZE.height * .85));
		display = new MoxDisplay(displaySize);
		basePanel.Add(display, BorderLayout.NORTH);
		// Create controls.
		controls = new MoxControls();
		basePanel.Add(controls, BorderLayout.SOUTH);
		// Make dashboard visible.
		setVisible(true);
	}


	// Set moxen.
	void setMoxen(ArrayList<Mox> moxen) {
		// Reset previous dashboards.
		reset();
		// Create moxen dashboards.
		this.moxen      = moxen;
		moxendashboards = new ArrayList<Moxdashboard>();
		Moxdashboard moxdashboard;

		for (int i = 0; i < moxen.GetCount(); i++) {
			moxdashboard = new Moxdashboard(moxen.Get(i), this);
			moxendashboards.Add(i, moxdashboard);
		}
	}


	// Reset.
	void reset() {
		currentMox = -1;

		if (moxendashboards != null) {
			for (int i = 0; i < moxendashboards.GetCount(); i++)
				moxendashboards.Get(i).setVisible(false);
		}
	}


	// Update dashboard.
	public void Update(int step, int steps) {
		controls.updateStepCounter(step, steps);
		Update();
	}


	private int timer = 0;
	public void Update() {
		if (quit)
			return;

		// Update moxen dashboards.
		for (int i = 0; i < moxendashboards.GetCount(); i++)
			moxendashboards.Get(i).Update();

		// Update display.
		display.Update();

		// Timer loop: count down delay by 1ms.
		for (timer = stepDelay; timer > 0 && !quit; ) {
			try {
				Sleep(1);
			}
			catch (InterruptedException e) {
				break;
			}

			display.Update();

			if (stepDelay < MAX_STEP_DELAY)
				timer--;
		}
	}


	// Set step delay.
	void SetStepDelay(int delay) {
		stepDelay = timer = delay;
	}


	// Step.
	void Step() {
		timer = 0;
	}


	// Close current mox dashboard.
	public void closeMoxdashboard() {
		if (currentMox != -1)
			closeMoxdashboard(currentMox);
	}


	// Close given mox dashboard.
	public void closeMoxdashboard(int moxIndex) {
		if (moxIndex == currentMox) {
			moxendashboards.Get(currentMox).setVisible(false);
			currentMox = -1;
		}
	}


	// Set message
	void setMessage(String message) {
		if (message == null)
			controls.messageText.SetLabel("");
		else
			controls.messageText.SetLabel(message);
	}


	// Mox display.
	public class MoxDisplay extends Canvas {
		// Buffered display.
		private Size canvas_size;
		private Graphics  graphics;
		private Image     image;
		private Graphics  imageGraphics;

		// Last cell visited by mouse.
		private int lastX = -1;

		// Last cell visited by mouse.
		private int lastY = -1;

		// Constructor.
		public MoxDisplay(Size canvas_size) {
			// Configure canvas.
			this.canvas_size = canvas_size;
			setBounds(0, 0, canvas_size.width, canvas_size.height);
			addMouseListener(new CanvasMouseListener());
			addMouseMotionListener(new CanvasMouseMotionListener());
		}


		// Update display.
		void Update() {
			int   x;
			int   y;
			int   x2;
			int   y2;
			double cellWidth;
			double cellHeight;

			if (quit)
				return;

			if (graphics == null) {
				graphics      = getGraphics();
				image         = createImage(canvas_size.width, canvas_size.height);
				imageGraphics = image.getGraphics();
			}

			if (graphics == null)
				return;

			// Clear display.
			imageGraphics.setColor(Color.white);
			imageGraphics.fillRect(0, 0, canvas_size.width, canvas_size.height);
			// Draw grid.
			synchronized (game_of_life.lock) {
				cellWidth  = (float)canvas_size.width / (float)game_of_life.size.width;
				cellHeight = (float)canvas_size.height / (float)game_of_life.size.height;
				imageGraphics.setColor(Color.black);
				y2 = canvas_size.height;

				for (x = 1, x2 = (int)cellWidth; x < game_of_life.size.width;
					 x++, x2 = (int)(cellWidth * (double)x))
					imageGraphics.drawLine(x2, 0, x2, y2);

				x2 = canvas_size.width;

				for (y = 1, y2 = (int)cellHeight; y < game_of_life.size.height;
					 y++, y2 = (int)(cellHeight * (double)y))
					imageGraphics.drawLine(0, y2, x2, y2);

				// Draw cells.
				for (x = x2 = 0; x < game_of_life.size.width;
					 x++, x2 = (int)(cellWidth * (double)x)) {
					for (y = 0, y2 = canvas_size.height - (int)cellHeight;
						 y < game_of_life.size.height;
						 y++, y2 = (int)(cellHeight *
										 (double)(game_of_life.size.height - (y + 1)))) {
						if (game_of_life.cells[x][y] > 0) {
							switch (game_of_life.cells[x][y]) {
							case GameOfLife.GREEN_CELL_COLOR_VALUE:
								imageGraphics.setColor(Color.green);
								break;

							case GameOfLife.BLUE_CELL_COLOR_VALUE:
								imageGraphics.setColor(Color.blue);
								break;

							default:
								imageGraphics.setColor(Color.black);
								break;
							}

							imageGraphics.fillRect(x2, y2, (int)cellWidth + 1,
												   (int)cellHeight + 1);
						}
					}
				}
			}
			imageGraphics.setColor(Color.black);
			// Draw moxen.
			Mox mox;
			int[] vx = new int[3];
			int[] vy = new int[3];

			for (int i = 0; i < moxen.GetCount(); i++) {
				mox = (Mox)moxen.Get(i);

				if (!mox.isAlive)
					continue;

				x2 = (int)(cellWidth * (double)mox.x);
				y2 = (int)(cellHeight *
						   (double)(game_of_life.size.height - (mox.y + 1)));

				// Highlight selected mox?
				if (i == currentMox)
					imageGraphics.setColor(Color.lightGray);
				else
					imageGraphics.setColor(Color.white);

				imageGraphics.fillRect(x2 + 1, y2 + 1, (int)cellWidth - 1,
									   (int)cellHeight - 1);

				if (mox.species == Mox.SPECIES.FORAGER.GetValue())
					imageGraphics.setColor(ForagerMox.FORAGER_COLOR);
				else if (mox.species == Mox.SPECIES.PREDATOR.GetValue())
					imageGraphics.setColor(PredatorMox.PREDATOR_COLOR);
				else
					imageGraphics.setColor(Color.black);

				if (mox.direction == Mox.DIRECTION.NORTH.GetValue()) {
					vx[0] = x2 + (int)(cellWidth * 0.5f);
					vy[0] = y2;
					vx[1] = x2;
					vy[1] = y2 + (int)cellHeight;
					vx[2] = x2 + (int)cellWidth;
					vy[2] = y2 + (int)cellHeight;
				}
				else if (mox.direction == Mox.DIRECTION.EAST.GetValue()) {
					vx[0] = x2 + (int)(cellWidth);
					vy[0] = y2 + (int)(cellHeight * 0.5f);
					vx[1] = x2;
					vy[1] = y2;
					vx[2] = x2;
					vy[2] = y2 + (int)cellHeight;
				}
				else if (mox.direction == Mox.DIRECTION.SOUTH.GetValue()) {
					vx[0] = x2 + (int)(cellWidth * 0.5f);
					vy[0] = y2 + (int)cellHeight;
					vx[1] = x2;
					vy[1] = y2;
					vx[2] = x2 + (int)cellWidth;
					vy[2] = y2;
				}
				else {
					vx[0] = x2;
					vy[0] = y2 + (int)(cellHeight * 0.5f);
					vx[1] = x2 + (int)cellWidth;
					vy[1] = y2;
					vx[2] = x2 + (int)cellWidth;
					vy[2] = y2 + (int)cellHeight;
				}

				imageGraphics.fillPolygon(vx, vy, 3);
			}

			// Refresh display.
			graphics.drawImage(image, 0, 0, this);
		}


		// Canvas mouse listener.
		class CanvasMouseListener extends MouseAdapter {
			// Mouse pressed.
			public void mousePressed(MouseEvent evt) {
				int     i;
				int     x;
				int     y;
				double  cellWidth  = (double)canvas_size.width / (double)game_of_life.size.width;
				double  cellHeight = (double)canvas_size.height / (double)game_of_life.size.height;
				Mox     mox;
				bool moxSelected;
				x = (int)((double)evt.getX() / cellWidth);
				y = game_of_life.size.height - (int)((double)evt.getY() / cellHeight) - 1;

				if ((x >= 0) && (x < game_of_life.size.width) &&
					(y >= 0) && (y < game_of_life.size.height)) {
					lastX = x;
					lastY = y;
					// Selecting mox?
					moxSelected = false;

					for (i = 0; i < moxen.GetCount(); i++) {
						mox = moxen.Get(i);

						if (mox.isAlive && (mox.x == x) && (mox.y == y)) {
							moxSelected = true;

							if (currentMox == -1) {
								currentMox = i;
								moxendashboards.Get(currentMox).setVisible(true);
							}
							else {
								if (i == currentMox) {
									moxendashboards.Get(currentMox).setVisible(false);
									currentMox = -1;
								}
								else {
									moxendashboards.Get(currentMox).setVisible(false);
									currentMox = i;
									moxendashboards.Get(currentMox).setVisible(true);
								}
							}

							break;
						}
					}

					if (!moxSelected && (currentMox != -1)) {
						moxendashboards.Get(currentMox).setVisible(false);
						currentMox = -1;
					}

					if (!moxSelected) {
						synchronized (game_of_life.lock) {
							if (game_of_life.cells[x][y] > 0)
								game_of_life.cells[x][y] = 0;
							else
								game_of_life.cells[x][y] = 1;
						}
					}

					// Refresh display.
					Update();
				}
			}
		}

		// Canvas mouse motion listener.
		class CanvasMouseMotionListener extends MouseMotionAdapter {
			// Mouse dragged.
			public void mouseDragged(MouseEvent evt) {
				int    x;
				int    y;
				double cellWidth  = (double)canvas_size.width / (double)game_of_life.size.width;
				double cellHeight = (double)canvas_size.height / (double)game_of_life.size.height;
				x = (int)((double)evt.getX() / cellWidth);
				y = game_of_life.size.height - (int)((double)evt.getY() / cellHeight) - 1;

				if ((x >= 0) && (x < game_of_life.size.width) &&
					(y >= 0) && (y < game_of_life.size.height)) {
					if ((x != lastX) || (y != lastY)) {
						lastX = x;
						lastY = y;
						synchronized (game_of_life.lock) {
							if (game_of_life.cells[x][y] > 0)
								game_of_life.cells[x][y] = 0;
							else
								game_of_life.cells[x][y] = 1;
						}
						// Refresh display.
						Update();
					}
				}
			}
		}
	}

	// Control panel.
	class MoxControls extends JPanel implements ActionListener, ChangeListener {
		// Components.
		JButton    resetButton;
		JLabel     stepCounter;
		JSlider    speedSlider;
		JButton    stepButton;
		JTextField messageText;

		// Constructor.
		MoxControls() {
			setLayout(new BorderLayout());
			setBorder(BorderFactory.createRaisedBevelBorder());
			JPanel panel = new JPanel();
			resetButton = new JButton("Reset");
			resetButton.addActionListener(this);
			panel.Add(resetButton);
			panel.Add(new JLabel("Speed:   Fast", Label.RIGHT));
			speedSlider = new JSlider(JSlider.HORIZONTAL, MIN_STEP_DELAY,
									  MAX_STEP_DELAY, MAX_STEP_DELAY);
			speedSlider.addChangeListener(this);
			panel.Add(speedSlider);
			panel.Add(new JLabel("Stop", Label.LEFT));
			stepButton = new JButton("Step");
			stepButton.addActionListener(this);
			panel.Add(stepButton);
			stepCounter = new JLabel("");
			panel.Add(stepCounter);
			add(panel, BorderLayout.NORTH);
			panel       = new JPanel();
			messageText = new JTextField("", 50);
			messageText.setEditable(false);
			panel.Add(messageText);
			add(panel, BorderLayout.SOUTH);
		}


		// Update step counter display
		void updateStepCounter(int step, int steps) {
			stepCounter.SetLabel("Step: " + step + "/" + steps);
		}


		// Speed slider listener.
		public void stateChanged(ChangeEvent evt) {
			SetStepDelay(speedSlider.GetValue());
		}


		// Step button listener.
		public void actionPerformed(ActionEvent evt) {
			// Reset?
			if (evt.getSource() == (Object)resetButton) {
				game_of_life.Restore();
				int numMox = moxen.GetCount();

				for (int i = 0; i < numMox; i++) {
					moxen.Get(i).reset();
					moxendashboards.Get(i).Update();
				}

				return;
			}

			// Step?
			if (evt.getSource() == (Object)stepButton) {
				speedSlider.SetValue(MAX_STEP_DELAY);
				Step();
				return;
			}
		}
	}
}
