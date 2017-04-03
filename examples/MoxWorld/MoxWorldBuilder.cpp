/*
    Build a Game of Life mox world using given parameters.
*/

import java.util.*;
import java.awt.*;

public class MoxWorldBuilder {
	// Usage.
	public static final String Usage =
		"Usage:\n" +
		"    java MoxWorldBuilder\n" +
		"      -steps <steps>\n" +
		"      -gridSize <width> <height>\n" +
		"      -live_cell_probability <0.0-1.0>\n" +
		"     [-random_seed <random number seed>]\n" +
		"     [-blue_cell_count <quantity>]\n" +
		"     [-max_blue_cell_count <quantity>]\n" +
		"     [-min_green_cell_count <quantity>]\n" +
		"     [-max_green_cell_count <quantity>]\n" +
		"     [-max_build_attempts <quantity>]\n" +
		"      -save <file name>\n" +
		"     [-dashboard]";

	// Default random seed.
	public static final int DEFAULT_RANDOM_SEED = 4517;

	// Default maximum build attempts.
	public static final int DEFAULT_MAX_BUILD_ATTEMPTS = 1000;

	// Game of Life mox world.
	public GameOfLife game_of_life;

	// Random numbers.
	Random randomizer;

	// dashboard display.
	MoxWorldDashboard dashboard;

	// Constructor.
	public MoxWorldBuilder(int width, int height) {
		// Create Game of Life.
		game_of_life = new GameOfLife(new Size(width, height));
	}


	// Build.
	public bool Build(int steps, double live_cell_prob, int random_seed,
					  int blue_cell_count, int max_blue_cell_count,
					  int min_green_cell_count, int max_green_cell_count,
					  int max_build_attempts) {
		// Random numbers.
		randomizer = new Random(random_seed);

		// Try to build world.
		for (int i = 0; i < max_build_attempts; i++) {
			// Initialize.
			Init(live_cell_prob);
			// Run steps.
			Run(steps);

			// Check properties.
			if (check(blue_cell_count, max_blue_cell_count, min_green_cell_count, max_green_cell_count))
				return true;
		}

		return false;
	}


	// Initialize.
	public void Init(double live_cell_prob) {
		int x, y;
		int width  = game_of_life.GetWidth();
		int height = game_of_life.GetHeight();
		synchronized (game_of_life.lock) {
			for (x = 0; x < width; x++) {
				for (y = 0; y < height; y++) {
					if (randomizer.nextFloat() < live_cell_prob)
						game_of_life.cells[x][y] = 1;
				}
			}

			game_of_life.Step();
			game_of_life.Checkpoint();
		}
	}


	// Run steps.
	public void Run(int steps) {
		for (int i = 0; i < steps; i++) {
			// Update dashboard.
			updatedashboard(i + 1, steps);
			// Step Game of Life.
			stepGameOfLife();
		}

		synchronized (game_of_life.lock) {
			game_of_life.Checkpoint();
		}
	}


	// Step Game Of Life.
	public void stepGameOfLife() {
		synchronized (game_of_life.lock) {
			game_of_life.Step();
		}
	}


	// Check.
	public bool check(int blue_cell_count, int max_blue_cell_count,
					  int min_green_cell_count, int max_green_cell_count) {
		int x, y;
		int width      = game_of_life.GetWidth();
		int height     = game_of_life.GetHeight();
		int blueCount  = 0;
		int greenCount = 0;

		for (x = 0; x < width; x++) {
			for (y = 0; y < height; y++) {
				if (game_of_life.cells[x][y] == GameOfLife.BLUE_CELL_COLOR_VALUE)
					blueCount++;

				if (game_of_life.cells[x][y] == GameOfLife.GREEN_CELL_COLOR_VALUE)
					greenCount++;
			}
		}

		if ((blueCount >= blue_cell_count) && (blueCount <= max_blue_cell_count) &&
			(greenCount >= min_green_cell_count) && (greenCount <= max_green_cell_count))
			return true;
		else
			return false;
	}


	// Create dashboard.
	public void createdashboard() {
		if (dashboard == null)
			dashboard = new MoxWorldDashboard(game_of_life);
	}


	// Destroy dashboard.
	public void destroydashboard() {
		if (dashboard != null) {
			dashboard.reset();
			dashboard.setVisible(false);
			dashboard = null;
		}
	}


	// Update dashboard.
	// Return true if dashboard operational.
	public bool updatedashboard(int step, int steps, String message) {
		if (dashboard != null) {
			dashboard.setMessage(message);
			dashboard.Update(step, steps);

			if (dashboard.quit) {
				dashboard = null;
				return false;
			}
			else
				return true;
		}
		else
			return false;
	}


	public bool updatedashboard(int step, int steps) {
		return updatedashboard(step, steps, "");
	}


	public bool updatedashboard() {
		if (dashboard != null) {
			dashboard.Update();

			if (dashboard.quit) {
				dashboard = null;
				return false;
			}
			else
				return true;
		}
		else
			return false;
	}


	// Main.
	public static void main(String[] args) {
		// Get options.
		int     steps            = -1;
		int     width            = -1;
		int     height           = -1;
		double   live_cell_prob     = -1.0;
		int     random_seed       = DEFAULT_RANDOM_SEED;
		int     blue_cell_count     = 0;
		int     max_blue_cell_count     = -1;
		int     min_green_cell_count    = 0;
		int     max_green_cell_count    = -1;
		int     max_build_attempts = DEFAULT_MAX_BUILD_ATTEMPTS;
		String  savefile         = null;
		bool dashboard        = false;

		for (int i = 0; i < args.length; i++) {
			if (args[i].equals("-steps")) {
				i++;

				if (i >= args.length) {
					System.err.println("Invalid steps option");
					System.err.println(MoxWorldBuilder.Usage);
					System.exit(1);
				}

				try {
					steps = Integer.parseInt(args[i]);
				}
				catch (NumberFormatException e) {
					System.err.println("Invalid steps option");
					System.err.println(MoxWorldBuilder.Usage);
					System.exit(1);
				}

				if (steps < 0) {
					System.err.println("Invalid steps option");
					System.err.println(MoxWorldBuilder.Usage);
					System.exit(1);
				}

				continue;
			}

			if (args[i].equals("-gridSize")) {
				i++;

				if (i >= args.length) {
					System.err.println("Invalid gridSize option");
					System.err.println(MoxWorldBuilder.Usage);
					System.exit(1);
				}

				try {
					width = Integer.parseInt(args[i]);
				}
				catch (NumberFormatException e) {
					System.err.println("Invalid gridSize width option");
					System.err.println(MoxWorldBuilder.Usage);
					System.exit(1);
				}

				if (width < 2) {
					System.err.println("Invalid gridSize width option");
					System.err.println(MoxWorldBuilder.Usage);
					System.exit(1);
				}

				i++;

				if (i >= args.length) {
					System.err.println("Invalid gridSize option");
					System.err.println(MoxWorldBuilder.Usage);
					System.exit(1);
				}

				try {
					height = Integer.parseInt(args[i]);
				}
				catch (NumberFormatException e) {
					System.err.println("Invalid gridSize height option");
					System.err.println(MoxWorldBuilder.Usage);
					System.exit(1);
				}

				if (height < 2) {
					System.err.println("Invalid gridSize height option");
					System.err.println(MoxWorldBuilder.Usage);
					System.exit(1);
				}

				continue;
			}

			if (args[i].equals("-live_cell_probability")) {
				i++;

				if (i >= args.length) {
					System.err.println("Invalid live_cell_probability option");
					System.err.println(MoxWorldBuilder.Usage);
					System.exit(1);
				}

				try {
					live_cell_prob = Float.parseFloat(args[i]);
				}
				catch (NumberFormatException e) {
					System.err.println("Invalid live_cell_probability option");
					System.err.println(MoxWorldBuilder.Usage);
					System.exit(1);
				}

				if ((live_cell_prob < 0.0f) || (live_cell_prob > 1.0f)) {
					System.err.println("Invalid live_cell_probability option");
					System.err.println(MoxWorldBuilder.Usage);
					System.exit(1);
				}

				continue;
			}

			if (args[i].equals("-random_seed")) {
				i++;

				if (i >= args.length) {
					System.err.println("Invalid random_seed option");
					System.err.println(MoxWorldBuilder.Usage);
					System.exit(1);
				}

				try {
					random_seed = Integer.parseInt(args[i]);
				}
				catch (NumberFormatException e) {
					System.err.println("Invalid random_seed option");
					System.err.println(MoxWorldBuilder.Usage);
					System.exit(1);
				}

				continue;
			}

			if (args[i].equals("-blue_cell_count")) {
				i++;

				if (i >= args.length) {
					System.err.println("Invalid blue_cell_count option");
					System.err.println(MoxWorldBuilder.Usage);
					System.exit(1);
				}

				try {
					blue_cell_count = Integer.parseInt(args[i]);
				}
				catch (NumberFormatException e) {
					System.err.println("Invalid blue_cell_count option");
					System.err.println(MoxWorldBuilder.Usage);
					System.exit(1);
				}

				if (blue_cell_count < 0) {
					System.err.println("Invalid blue_cell_count option");
					System.err.println(MoxWorldBuilder.Usage);
					System.exit(1);
				}

				continue;
			}

			if (args[i].equals("-max_blue_cell_count")) {
				i++;

				if (i >= args.length) {
					System.err.println("Invalid max_blue_cell_count option");
					System.err.println(MoxWorldBuilder.Usage);
					System.exit(1);
				}

				try {
					max_blue_cell_count = Integer.parseInt(args[i]);
				}
				catch (NumberFormatException e) {
					System.err.println("Invalid max_blue_cell_count option");
					System.err.println(MoxWorldBuilder.Usage);
					System.exit(1);
				}

				if (max_blue_cell_count < 0) {
					System.err.println("Invalid max_blue_cell_count option");
					System.err.println(MoxWorldBuilder.Usage);
					System.exit(1);
				}

				continue;
			}

			if (args[i].equals("-min_green_cell_count")) {
				i++;

				if (i >= args.length) {
					System.err.println("Invalid min_green_cell_count option");
					System.err.println(MoxWorldBuilder.Usage);
					System.exit(1);
				}

				try {
					min_green_cell_count = Integer.parseInt(args[i]);
				}
				catch (NumberFormatException e) {
					System.err.println("Invalid min_green_cell_count option");
					System.err.println(MoxWorldBuilder.Usage);
					System.exit(1);
				}

				if (min_green_cell_count < 0) {
					System.err.println("Invalid min_green_cell_count option");
					System.err.println(MoxWorldBuilder.Usage);
					System.exit(1);
				}

				continue;
			}

			if (args[i].equals("-max_green_cell_count")) {
				i++;

				if (i >= args.length) {
					System.err.println("Invalid max_green_cell_count option");
					System.err.println(MoxWorldBuilder.Usage);
					System.exit(1);
				}

				try {
					max_green_cell_count = Integer.parseInt(args[i]);
				}
				catch (NumberFormatException e) {
					System.err.println("Invalid max_green_cell_count option");
					System.err.println(MoxWorldBuilder.Usage);
					System.exit(1);
				}

				if (max_green_cell_count < 0) {
					System.err.println("Invalid max_green_cell_count option");
					System.err.println(MoxWorldBuilder.Usage);
					System.exit(1);
				}

				continue;
			}

			if (args[i].equals("-max_build_attempts")) {
				i++;

				if (i >= args.length) {
					System.err.println("Invalid max_build_attempts option");
					System.err.println(MoxWorldBuilder.Usage);
					System.exit(1);
				}

				try {
					max_build_attempts = Integer.parseInt(args[i]);
				}
				catch (NumberFormatException e) {
					System.err.println("Invalid max_build_attempts option");
					System.err.println(MoxWorldBuilder.Usage);
					System.exit(1);
				}

				if (max_build_attempts < 1) {
					System.err.println("Invalid max_build_attempts option");
					System.err.println(MoxWorldBuilder.Usage);
					System.exit(1);
				}

				continue;
			}

			if (args[i].equals("-save")) {
				i++;

				if (i >= args.length) {
					System.err.println("Invalid save option");
					System.err.println(MoxWorldBuilder.Usage);
					System.exit(1);
				}

				if (savefile == null)
					savefile = args[i];
				else {
					System.err.println("Duplicate save option");
					System.err.println(MoxWorldBuilder.Usage);
					System.exit(1);
				}

				continue;
			}

			if (args[i].equals("-dashboard")) {
				dashboard = true;
				continue;
			}

			System.err.println(MoxWorldBuilder.Usage);
			System.exit(1);
		}

		// Check options.
		if ((steps < 0) || (width == -1) ||
			(height == -1) || (live_cell_prob < 0.0f) ||
			(savefile == null)) {
			System.err.println(MoxWorldBuilder.Usage);
			System.exit(1);
		}

		if (max_blue_cell_count == -1)
			max_blue_cell_count = width * height;

		if (max_green_cell_count == -1)
			max_green_cell_count = width * height;

		if (blue_cell_count > max_blue_cell_count) {
			System.err.println(MoxWorldBuilder.Usage);
			System.exit(1);
		}

		if (min_green_cell_count > max_green_cell_count) {
			System.err.println(MoxWorldBuilder.Usage);
			System.exit(1);
		}

		if ((blue_cell_count + min_green_cell_count) > (width * height)) {
			System.err.println(MoxWorldBuilder.Usage);
			System.exit(1);
		}

		// Create.
		MoxWorldBuilder mox_world_builder = new MoxWorldBuilder(width, height);

		// Create dashboard?
		if (dashboard)
			mox_world_builder.createdashboard();

		// Build mox world.
		if (!mox_world_builder.Build(steps, live_cell_prob, random_seed,
									 blue_cell_count, max_blue_cell_count,
									 min_green_cell_count, max_green_cell_count,
									 max_build_attempts)) {
			System.err.println("Cannot build mox world");
			System.exit(1);
		}

		// Save.
		try {
			mox_world_builder.game_of_life.Store(savefile);
		}
		catch (Exception e) {
			System.err.println("Cannot save to file " + savefile + ": " + e.getMessage());
			System.exit(1);
		}

		System.exit(0);
	}
}
