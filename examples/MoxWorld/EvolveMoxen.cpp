/*
    Evolve individual mox foragers and predator-forager pairs
    by mutating and recombining parameters.
*/

import java.util.*;
import java.io.*;
import mona.NativeFileDescriptor;

public class EvolveMoxen {
	// Usage.
	public static final String Usage =
		"Usage:\n" +
		"  New run:\n" +
		"    java EvolveMoxen\n" +
		"      -generations <evolution generations>\n" +
		"      -steps <moxen steps>\n" +
		"     [-stepGameOfLife]\n" +
		"      -moxPopulations \"" + EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY.GetName() +
		"\" | \"" + EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS.GetName() + "\"\n" +
		"     [-trainForagers (train foragers before evaluating them)]\n" +
		"      -loadCells <file name> (repeatable option)\n" +
		"      -output <evolution output file name>\n" +
		"     [-mutation_rate <mutation rate>]\n" +
		"     [-random_mutation_rate <random mutation rate>]\n" +
		"     [-maxSensorRange <maximum sensor range>]\n" +
		"     [-random_seed <random seed> (for new run)]\n" +
		"     [-logfile <log file name>]\n" +
		"     [-dashboard (run with mox world dashboard)]\n" +
		"  Resume run:\n" +
		"    java EvolveMoxen\n" +
		"      -generations <evolution generations>\n" +
		"      -steps <moxen steps>\n" +
		"     [-stepGameOfLife]\n" +
		"     [-trainForagers (train foragers before evaluating them)]\n" +
		"      -input <evolution input file name>\n" +
		"      -output <evolution output file name>\n" +
		"     [-mutation_rate <mutation rate>]\n" +
		"     [-random_mutation_rate <random mutation rate>]\n" +
		"     [-maxSensorRange <maximum sensor range>]\n" +
		"     [-random_seed <random seed> (for new run)]\n" +
		"     [-logfile <log file name>]\n" +
		"     [-dashboard (run with mox world dashboard)]\n" +
		"  Extract into mox_world_forager|predator_<mox id>_world_<number>.mw files:\n" +
		"    java EvolveMoxen\n" +
		"      -extract\n" +
		"      -input <evolution input file name>\n" +
		"  Print population properties:\n" +
		"    java EvolveMoxen\n" +
		"      -properties\n" +
		"      -input <evolution input file name>\n" +
		"  Print evolution statistics:\n" +
		"    java EvolveMoxen\n" +
		"      -statistics\n" +
		"      -input <evolution input file name>";

	// generation_count.
	int generation;
	int generation_count;

	// Steps.
	int Steps;

	// Step Game of Life?
	bool StepGameOfLife;

	// Mox populations.
	EvolveCommon.MOX_POPULATIONS MoxPopulations;

	// Train foragers?
	bool TrainForagers;

	// File names.
	ArrayList<String> cells_file_names;
	String            input_file_name;
	String            output_file_name;
	String            log_file_name;
	PrintWriter       log_writer;

	// Random numbers.
	Random randomizer;

	// Run with dashboard.
	bool dashboard;

	// Extract moxen files.
	bool Extract;

	// Print population properties.
	bool PrintProperties;

	// Print evolution statistics.
	bool PrintStatistics;

	// Evolution statistics.
	double[] ForagerFittest;
	double[] ForagerAverage;
	double[] PredatorFittest;
	double[] PredatorAverage;

	// Maximum mox cycle time.
	long MaxMoxCycleTime;

	// Mox worlds.
	ArrayList<MoxWorld> moxWorlds;

	// Populations.
	EvolveCommon.Member[] ForagerPopulation;
	EvolveCommon.Member[] PredatorPopulation;

	// Constructor.
	public EvolveMoxen(String[] args) {
		int i;
		// Get options.
		generation     = 0;
		generation_count    = -1;
		Steps          = -1;
		StepGameOfLife = false;
		bool gotStepGameOfLife = false;
		MoxPopulations = EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY;
		TrainForagers  = false;
		bool gotMoxPopulations = false;
		cells_file_names = new ArrayList<String>();
		input_file_name  = output_file_name = log_file_name = null;
		log_writer      = null;
		bool has_mutation_rate       = false;
		bool has_random_mutation_rate = false;
		bool has_max_sensor_range     = false;
		bool has_random_seed         = false;
		Extract         = false;
		PrintProperties = false;
		PrintStatistics = false;
		dashboard       = false;

		for (i = 0; i < args.length; i++) {
			if (args[i].equals("-generations")) {
				i++;

				if (i >= args.length) {
					System.err.println(Usage);
					System.exit(1);
				}

				generation_count = Integer.parseInt(args[i]);

				if (generation_count < 0) {
					System.err.println(Usage);
					System.exit(1);
				}

				continue;
			}

			if (args[i].equals("-steps")) {
				i++;

				if (i >= args.length) {
					System.err.println(Usage);
					System.exit(1);
				}

				Steps = Integer.parseInt(args[i]);

				if (Steps < 0) {
					System.err.println(Usage);
					System.exit(1);
				}

				continue;
			}

			if (args[i].equals("-stepGameOfLife")) {
				StepGameOfLife    = true;
				gotStepGameOfLife = true;
				continue;
			}

			if (args[i].equals("-moxPopulations")) {
				i++;

				if (i >= args.length) {
					System.err.println(Usage);
					System.exit(1);
				}

				if (args[i].equals(EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY.GetName()))
					MoxPopulations = EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY;
				else if (args[i].equals(EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS.GetName()))
					MoxPopulations = EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS;
				else {
					System.err.println(Usage);
					System.exit(1);
				}

				gotMoxPopulations = true;
				continue;
			}

			if (args[i].equals("-trainForagers")) {
				TrainForagers = true;
				continue;
			}

			if (args[i].equals("-loadCells")) {
				i++;

				if (i >= args.length) {
					System.err.println(Usage);
					System.exit(1);
				}

				cells_file_names.Add(new String(args[i]));
				continue;
			}

			if (args[i].equals("-input")) {
				i++;

				if (i >= args.length) {
					System.err.println(Usage);
					System.exit(1);
				}

				input_file_name = new String(args[i]);
				continue;
			}

			if (args[i].equals("-output")) {
				i++;

				if (i >= args.length) {
					System.err.println(Usage);
					System.exit(1);
				}

				output_file_name = new String(args[i]);
				continue;
			}

			if (args[i].equals("-mutation_rate")) {
				i++;

				if (i >= args.length) {
					System.err.println(Usage);
					System.exit(1);
				}

				EvolveCommon.mutation_rate = Double.parseDouble(args[i]);

				if ((EvolveCommon.mutation_rate < 0.0) || (EvolveCommon.mutation_rate > 1.0)) {
					System.err.println(Usage);
					System.exit(1);
				}

				has_mutation_rate = true;
				continue;
			}

			if (args[i].equals("-random_mutation_rate")) {
				i++;

				if (i >= args.length) {
					System.err.println(Usage);
					System.exit(1);
				}

				EvolveCommon.Randommutation_rate = Double.parseDouble(args[i]);

				if ((EvolveCommon.Randommutation_rate < 0.0) || (EvolveCommon.Randommutation_rate > 1.0)) {
					System.err.println(Usage);
					System.exit(1);
				}

				has_random_mutation_rate = true;
				continue;
			}

			if (args[i].equals("-maxSensorRange")) {
				i++;

				if (i >= args.length) {
					System.err.println(Usage);
					System.exit(1);
				}

				EvolveCommon.MaxSensorRange = Float.parseFloat(args[i]);

				if (EvolveCommon.MaxSensorRange < 0.0f) {
					System.err.println(Usage);
					System.exit(1);
				}

				has_max_sensor_range = true;
				continue;
			}

			if (args[i].equals("-random_seed")) {
				i++;

				if (i >= args.length) {
					System.err.println(Usage);
					System.exit(1);
				}

				EvolveCommon.RandomSeed = Integer.parseInt(args[i]);
				has_random_seed           = true;
				continue;
			}

			if (args[i].equals("-logfile")) {
				i++;

				if (i >= args.length) {
					System.err.println(Usage);
					System.exit(1);
				}

				log_file_name = new String(args[i]);
				continue;
			}

			if (args[i].equals("-dashboard")) {
				dashboard = true;
				continue;
			}

			if (args[i].equals("-extract")) {
				Extract = true;
				continue;
			}

			if (args[i].equals("-properties")) {
				PrintProperties = true;
				continue;
			}

			if (args[i].equals("-statistics")) {
				PrintStatistics = true;
				continue;
			}

			System.err.println(Usage);
			System.exit(1);
		}

		// Extract moxen files or print properties?
		if (Extract || PrintProperties || PrintStatistics) {
			if ((generation_count != -1) || (Steps != -1) || gotStepGameOfLife ||
				gotMoxPopulations || TrainForagers || (input_file_name == null) ||
				(output_file_name != null) || (cells_file_names.GetCount() > 0) ||
				(log_file_name != null) || has_mutation_rate || has_random_mutation_rate ||
				has_max_sensor_range || has_random_seed || dashboard) {
				System.err.println(Usage);
				System.exit(1);
			}

			if (Extract && (PrintProperties || PrintStatistics)) {
				System.err.println(Usage);
				System.exit(1);
			}

			if (PrintProperties && (Extract || PrintStatistics)) {
				System.err.println(Usage);
				System.exit(1);
			}

			if (PrintStatistics && (Extract || PrintProperties)) {
				System.err.println(Usage);
				System.exit(1);
			}
		}
		else {
			if (generation_count == -1) {
				System.err.println("generation_count option required");
				System.err.println(Usage);
				System.exit(1);
			}

			if (Steps == -1) {
				System.err.println("Steps option required");
				System.err.println(Usage);
				System.exit(1);
			}

			if (output_file_name == null) {
				System.err.println("Output file required");
				System.err.println(Usage);
				System.exit(1);
			}

			if (input_file_name != null) {
				if (gotMoxPopulations || (cells_file_names.GetCount() > 0)) {
					System.err.println(Usage);
					System.exit(1);
				}
			}
			else {
				if (!gotMoxPopulations || (cells_file_names.GetCount() == 0)) {
					System.err.println(Usage);
					System.exit(1);
				}
			}
		}

		// Set maximum sensor range.
		ForagerMox.MAX_SENSOR_RANGE  = EvolveCommon.MaxSensorRange;
		PredatorMox.MAX_SENSOR_RANGE = EvolveCommon.MaxSensorRange;
		// Seed random numbers.
		randomizer = new Random(EvolveCommon.RandomSeed);

		// Open log file?
		if (log_file_name != null) {
			try {
				log_writer = new PrintWriter(new FileOutputStream(new File(log_file_name)));
			}
			catch (Exception e) {
				System.err.println("Cannot open log file " + log_file_name +
								   ":" + e.getMessage());
				System.exit(1);
			}
		}
	}


	// Start evolve.
	public void start() {
		// Initialize populations?
		if (input_file_name == null)
			Init();
		else {
			// Load populations.
			Load();
		}

		// Log run.
		log("Initializing evolve:");
		log("  Options:");
		log("    generations=" + generation_count);
		log("    steps=" + Steps);

		if (StepGameOfLife)
			log("    stepGameOfLife=true");
		else
			log("    stepGameOfLife=false");

		if (input_file_name == null) {
			if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY)
				log("    MoxPopulations=" + EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY.GetName());
			else
				log("    MoxPopulations=" + EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS.GetName());

			for (int i = 0; i < cells_file_names.GetCount(); i++)
				log("    loadCells=" + cells_file_names.Get(i));
		}
		else
			log("    input=" + input_file_name);

		log("    output=" + output_file_name);
		log("    mutation_rate=" + EvolveCommon.mutation_rate);
		log("    Randommutation_rate=" + EvolveCommon.Randommutation_rate);
		log("    MaxSensorRange=" + EvolveCommon.MaxSensorRange);
		log("    RandomSeed=" + EvolveCommon.RandomSeed);
		log("  Parameters:");
		log("    FORAGER_FIT_POPULATION_SIZE=" + EvolveCommon.FORAGER_FIT_POPULATION_SIZE);
		log("    FORAGER_NUM_MUTANTS=" + EvolveCommon.FORAGER_NUM_MUTANTS);
		log("    FORAGER_NUM_OFFSPRING=" + EvolveCommon.FORAGER_NUM_OFFSPRING);

		if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS) {
			log("    PREDATOR_FIT_POPULATION_SIZE=" + EvolveCommon.PREDATOR_FIT_POPULATION_SIZE);
			log("    PREDATOR_NUM_MUTANTS=" + EvolveCommon.PREDATOR_NUM_MUTANTS);
			log("    PREDATOR_NUM_OFFSPRING=" + EvolveCommon.PREDATOR_NUM_OFFSPRING);
		}

		// Extract moxen files?
		if (Extract) {
			extract();
			return;
		}

		// Print population properties?
		if (PrintProperties) {
			printProperties();
			return;
		}

		// Print evolution statistics?
		if (PrintStatistics) {
			printStatistics();
			return;
		}

		// Set maximum mox cycle time according to current running conditions.
		MaxMoxCycleTime = Mox.getMaxCycleTime();
		// Evolution loop.
		log("Begin evolve:");

		for (generation_count += generation; generation < generation_count; generation++) {
			log("generation=" + generation);
			evolve(generation);

			// Save populations?
			if ((generation % EvolveCommon.SAVE_FREQUENCY) == 0)
				Store(generation);
		}

		// Save populations.
		Store(generation - 1);
		log("End evolve");
	}


	// Initialize evolution.
	void Init() {
		int      i;
		MoxWorld moxWorld;
		moxWorlds = new ArrayList<MoxWorld>();

		for (i = 0; i < cells_file_names.GetCount(); i++) {
			try {
				moxWorld = new MoxWorld();
				moxWorld.loadCells(cells_file_names.Get(i));
				moxWorlds.Add(moxWorld);

				if (i > 0) {
					if ((moxWorlds.Get(0).GetWidth() != moxWorld.GetWidth()) ||
						(moxWorlds.Get(0).GetHeight() != moxWorld.GetHeight())) {
						System.err.println("Sizes in cells file " +
										   cells_file_names.Get(i) +
										   "must equal those in other cells files");
						System.exit(1);
					}
				}
			}
			catch (Exception e) {
				System.err.println("Cannot load cells file " +
								   cells_file_names.Get(i) +
								   ":" + e.getMessage());
				System.exit(1);
			}
		}

		ForagerPopulation = new EvolveCommon.Member[EvolveCommon.FORAGER_POPULATION_SIZE];

		for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++) {
			if (i == 0) {
				forager_population[i] =
					new EvolveCommon.Member(Mox.SPECIES.FORAGER.GetValue(), 0,
											0, 0, Mox.DIRECTION.NORTH.GetValue(), randomizer);
			}
			else {
				// Mutate parameters.
				forager_population[i] =
					new EvolveCommon.Member(forager_population[0], 0, randomizer);
			}
		}

		ForagerFittest = new double[generation_count + 1];
		ForagerAverage = new double[generation_count + 1];

		if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS) {
			PredatorPopulation = new EvolveCommon.Member[EvolveCommon.PREDATOR_POPULATION_SIZE];

			for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++) {
				if (i == 0) {
					Predatorpopulation[i] =
						new EvolveCommon.Member(Mox.SPECIES.PREDATOR.GetValue(), 0,
												moxWorlds.Get(0).GetWidth() - 1,
												moxWorlds.Get(0).GetHeight() - 1,
												Mox.DIRECTION.SOUTH.GetValue(), randomizer);
				}
				else {
					// Mutate parameters.
					Predatorpopulation[i] =
						new EvolveCommon.Member(Predatorpopulation[0], 0, randomizer);
				}
			}

			PredatorFittest = new double[generation_count + 1];
			PredatorAverage = new double[generation_count + 1];
		}
	}


	// Load evolution.
	void Load() {
		int                  i, n;
		FileInputStream      input  = null;
		NativeFileDescriptor fd     = null;
		DataInputStream      reader = null;
		MoxWorld             moxWorld;

		// Open the file.
		try {
			input  = new FileInputStream(new File(input_file_name));
			reader = new DataInputStream(input);
			fd     = new NativeFileDescriptor(input_file_name, "r");
			fd.open();
		}
		catch (Exception e) {
			System.err.println("Cannot open input file " + input_file_name +
							   ":" + e.getMessage());
		}

		try {
			generation = Utility.loadInt(reader);
			generation++;
		}
		catch (Exception e) {
			System.err.println("Cannot load from file " + input_file_name +
							   ":" + e.getMessage());
			System.exit(1);
		}

		// Load mox world.
		moxWorlds = new ArrayList<MoxWorld>();

		try {
			n = Utility.loadInt(reader);

			for (i = 0; i < n; i++) {
				moxWorld = new MoxWorld();
				moxWorld.Load(input, fd);
				moxWorlds.Add(moxWorld);
			}
		}
		catch (Exception e) {
			System.err.println("Cannot load mox world from file " + input_file_name +
							   ":" + e.getMessage());
			System.exit(1);
		}

		// Load populations.
		try {
			if (Utility.loadInt(reader) == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY.GetValue())
				MoxPopulations = EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY;
			else
				MoxPopulations = EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS;

			ForagerPopulation = new EvolveCommon.Member[EvolveCommon.FORAGER_POPULATION_SIZE];

			for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++) {
				forager_population[i] =
					new EvolveCommon.Member(Mox.SPECIES.FORAGER.GetValue(), 0, 0, 0, 0, randomizer);
				forager_population[i].Load(input, fd);
			}

			ForagerFittest = new double[generation + generation_count + 1];
			ForagerAverage = new double[generation + generation_count + 1];

			for (i = 0; i < generation; i++) {
				ForagerFittest[i] = Utility.loadDouble(reader);
				ForagerAverage[i] = Utility.loadDouble(reader);
			}

			if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS) {
				PredatorPopulation = new EvolveCommon.Member[EvolveCommon.PREDATOR_POPULATION_SIZE];

				for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++) {
					Predatorpopulation[i] =
						new EvolveCommon.Member(Mox.SPECIES.PREDATOR.GetValue(), 0, 0, 0, 0, randomizer);
					Predatorpopulation[i].Load(input, fd);
				}

				PredatorFittest = new double[generation + generation_count + 1];
				PredatorAverage = new double[generation + generation_count + 1];

				for (i = 0; i < generation; i++) {
					PredatorFittest[i] = Utility.loadDouble(reader);
					PredatorAverage[i] = Utility.loadDouble(reader);
				}
			}

			input.close();
			fd.close();
		}
		catch (Exception e) {
			System.err.println("Cannot load populations from file " + input_file_name +
							   ":" + e.getMessage());
			System.exit(1);
		}
	}


	// Save evolution.
	void Store(int generation) {
		int                  i, n;
		FileOutputStream     output = null;
		NativeFileDescriptor fd     = null;
		PrintWriter          writer = null;

		try {
			output = new FileOutputStream(new File(output_file_name));
			writer = new PrintWriter(output);
			fd     = new NativeFileDescriptor(output_file_name, "w");
			fd.open();
		}
		catch (Exception e) {
			System.err.println("Cannot open output file " + output_file_name +
							   ":" + e.getMessage());
			System.exit(1);
		}

		try {
			Utility.saveInt(writer, generation);
			writer.flush();
		}
		catch (Exception e) {
			System.err.println("Cannot save to file " + output_file_name +
							   ":" + e.getMessage());
			System.exit(1);
		}

		// Save mox world.
		try {
			n = moxWorlds.GetCount();
			Utility.saveInt(writer, n);
			writer.flush();

			for (i = 0; i < n; i++)
				moxWorlds.Get(i).Store(output, fd);
		}
		catch (Exception e) {
			System.err.println("Cannot save mox world to file " + output_file_name +
							   ":" + e.getMessage());
			System.exit(1);
		}

		// Save populations.
		try {
			if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY)
				Utility.saveInt(writer, EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY.GetValue());
			else
				Utility.saveInt(writer, EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS.GetValue());

			writer.flush();

			for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++)
				forager_population[i].Store(output, fd);

			writer.flush();

			for (i = 0, n = generation + 1; i < n; i++) {
				Utility.saveDouble(writer, ForagerFittest[i]);
				Utility.saveDouble(writer, ForagerAverage[i]);
			}

			writer.flush();

			if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS) {
				for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++)
					Predatorpopulation[i].Store(output, fd);

				writer.flush();

				for (i = 0, n = generation + 1; i < n; i++) {
					Utility.saveDouble(writer, PredatorFittest[i]);
					Utility.saveDouble(writer, PredatorAverage[i]);
				}
			}

			writer.flush();
			output.close();
			fd.close();
		}
		catch (Exception e) {
			System.err.println("Cannot save populations to file " + output_file_name +
							   ":" + e.getMessage());
			System.exit(1);
		}
	}


	// Evolution generation.
	void evolve(int generation) {
		// Train foragers?
		if (TrainForagers)
			trainForagers();

		// Evaluate member fitness.
		Evaluate(generation);
		// Prune unfit members.
		Prune();
		// Create new members by mutation.
		Mutate();
		// Create new members by mating.
		DoMating();
	}


	// Train foragers.
	void trainForagers() {
		int i, j, step;
		int blue_food_need_idx;
		Mox mox;
		ArrayList<Mox> moxen;
		MoxWorld       moxWorld;
		blue_food_need_idx = ForagerMox.NEED_TYPE.BLUE_FOOD.GetValue();

		for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++) {
			mox = forager_population[i].mox;
			mox.SetNeed(blue_food_need_idx, ForagerMox.BLUE_FOOD_NEED_VALUE);
			moxen = new ArrayList<Mox>(1);
			moxen.Add(0, mox);
			// Train mox in all mox worlds.
			forager_population[i].fitness = 0.0;

			for (j = 0; j < moxWorlds.GetCount(); j++) {
				moxWorld = moxWorlds.Get(j);
				moxWorld.setMoxen(moxen);
				moxWorld.reset();

				// Step world.
				if (dashboard) {
					moxWorld.createdashboard();
					dashboard = moxWorld.updatedashboard(0, Steps,
														 "forager training" +
														 ", member=" + i + ", mox=" + mox.id +
														 ", world=" + j +
														 ", blue food need=" +
														 mox.GetNeed(blue_food_need_idx));
				}

				for (step = 0; step < Steps; step++) {
					if (mox.GetNeed(blue_food_need_idx) == 0.0)
						break;

					if (!mox.isAlive)
						break;

					setTrainingResponse(mox, moxWorld);
					moxWorld.stepMoxen();

					if (StepGameOfLife)
						moxWorld.stepGameOfLife();

					if (dashboard) {
						dashboard = moxWorld.updatedashboard(step + 1, Steps,
															 "forager training" +
															 ", member=" + i + ", mox=" + mox.id +
															 ", world=" + j +
															 ", blue food need=" +
															 mox.GetNeed(blue_food_need_idx));
					}
				}

				moxWorld.destroydashboard();
				moxWorld.setMoxen(new ArrayList<Mox>());
				moxWorld.reset();
			}
		}
	}


	// Set mox training response.
	void setTrainingResponse(Mox mox, MoxWorld moxWorld) {
		int     x, y, nx, ny, ex, ey, sx, sy, wx, wy, w, h;
		int     d, dn, de, ds, dw;
		int     blue_food_need_idx;
		bool needBlueFood;
		// Locate adjacent cells.
		w  = moxWorld.GetWidth();
		h  = moxWorld.GetHeight();
		nx = mox.x;
		ny = ((mox.y + 1) % h);
		ex = (mox.x + 1) % w;
		ey = mox.y;
		sx = mox.x;
		sy = mox.y - 1;

		if (sy < 0)
			sy += h;

		wx = mox.x - 1;

		if (wx < 0)
			wx += w;

		wy = mox.y;
		// Get distance from goal to nearest adjacent cell.
		blue_food_need_idx = ForagerMox.NEED_TYPE.BLUE_FOOD.GetValue();
		needBlueFood    = false;

		if (mox.GetNeed(blue_food_need_idx) > 0.0)
			needBlueFood = true;

		dn = de = ds = dw = -1;

		for (x = 0; x < w; x++) {
			for (y = 0; y < h; y++) {
				if ((moxWorld.game_of_life.cells[x][y] == GameOfLife.BLUE_CELL_COLOR_VALUE) &&
					needBlueFood) {
					d = dist(x, y, nx, ny, w, h);

					if ((dn == -1) || (d < dn))
						dn = d;

					d = dist(x, y, ex, ey, w, h);

					if ((de == -1) || (d < de))
						de = d;

					d = dist(x, y, sx, sy, w, h);

					if ((ds == -1) || (d < ds))
						ds = d;

					d = dist(x, y, wx, wy, w, h);

					if ((dw == -1) || (d < dw))
						dw = d;
				}
			}
		}

		if (dn == -1)
			return;

		if (mox.direction == Mox.DIRECTION.NORTH.GetValue()) {
			if ((dn <= de) && (dn <= dw) && (dn <= ds))
				mox.OverrideResponse(Mox.RESPONSE_TYPE.FORWARD.GetValue());
			else if ((de <= dw) && (de <= ds))
				mox.OverrideResponse(Mox.RESPONSE_TYPE.RIGHT.GetValue());
			else
				mox.OverrideResponse(Mox.RESPONSE_TYPE.LEFT.GetValue());

			return;
		}
		else if (mox.direction == Mox.DIRECTION.EAST.GetValue()) {
			if ((de <= dn) && (de <= ds) && (de <= dw))
				mox.OverrideResponse(Mox.RESPONSE_TYPE.FORWARD.GetValue());
			else if ((ds <= dn) && (ds <= dw))
				mox.OverrideResponse(Mox.RESPONSE_TYPE.RIGHT.GetValue());
			else
				mox.OverrideResponse(Mox.RESPONSE_TYPE.LEFT.GetValue());

			return;
		}
		else if (mox.direction == Mox.DIRECTION.SOUTH.GetValue()) {
			if ((ds <= de) && (ds <= dw) && (ds <= dn))
				mox.OverrideResponse(Mox.RESPONSE_TYPE.FORWARD.GetValue());
			else if ((dw <= de) && (dw <= dn))
				mox.OverrideResponse(Mox.RESPONSE_TYPE.RIGHT.GetValue());
			else
				mox.OverrideResponse(Mox.RESPONSE_TYPE.LEFT.GetValue());

			return;
		}
		else {
			if ((dw <= dn) && (dw <= ds) && (dw <= de))
				mox.OverrideResponse(Mox.RESPONSE_TYPE.FORWARD.GetValue());
			else if ((dn <= ds) && (dn <= de))
				mox.OverrideResponse(Mox.RESPONSE_TYPE.RIGHT.GetValue());
			else
				mox.OverrideResponse(Mox.RESPONSE_TYPE.LEFT.GetValue());

			return;
		}
	}


	// City-block distance.
	int dist(int x1, int y1, int x2, int y2, int w, int h) {
		int d, dx, dy;
		dx = x2 - x1;

		if (dx < 0)
			dx = -dx;

		d = w - dx;

		if (d < dx)
			dx = d;

		dy = y2 - y1;

		if (dy < 0)
			dy = -dy;

		d = h - dy;

		if (d < dy)
			dy = d;

		d = dx + dy;
		return d;
	}


	// Evaluate member fitnesses.
	void Evaluate(int generation) {
		int    i, j, step;
		int    blue_food_need_idx, mox_food_need_idx;
		int    moxFoodStep;
		Mox    mox;
		long   excessCycleTime;
		String logEntry;
		ArrayList<Mox> moxen;
		MoxWorld       moxWorld;
		log("Evaluate:");
		// Prepare for evaluation.
		prepareEvaluation(generation);
		// Evaluate foragers.
		log("  Foragers:");
		blue_food_need_idx = ForagerMox.NEED_TYPE.BLUE_FOOD.GetValue();
		mox_food_need_idx  = PredatorMox.NEED_TYPE.MOX_FOOD.GetValue();

		for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++) {
			// Set up mox world.
			if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY) {
				mox = forager_population[i].mox;
				mox.SetNeed(blue_food_need_idx, ForagerMox.BLUE_FOOD_NEED_VALUE);
				moxen = new ArrayList<Mox>(1);
				moxen.Add(0, mox);
			}
			else {
				// Set up mox world with random fit predator.
				mox = Predatorpopulation[randomizer.nextInt(EvolveCommon.PREDATOR_FIT_POPULATION_SIZE)].mox;
				mox.SetNeed(mox_food_need_idx, PredatorMox.MOX_FOOD_NEED_VALUE);
				moxen = new ArrayList<Mox>(2);
				moxen.Add(0, mox);
				mox = forager_population[i].mox;
				mox.SetNeed(blue_food_need_idx, ForagerMox.BLUE_FOOD_NEED_VALUE);
				moxen.Add(1, mox);
			}

			// Evaluate mox in all mox worlds.
			excessCycleTime = 0;
			forager_population[i].fitness = 0.0;

			for (j = 0; j < moxWorlds.GetCount(); j++) {
				moxWorld = moxWorlds.Get(j);
				moxWorld.setMoxen(moxen);
				moxWorld.reset();

				// Step world.
				if (dashboard) {
					moxWorld.createdashboard();
					dashboard = moxWorld.updatedashboard(0, Steps,
														 "generation=" + generation +
														 ", member=" + i + ", mox=" + mox.id +
														 ", world=" + j +
														 ", blue food need=" +
														 mox.GetNeed(blue_food_need_idx));
				}

				for (step = 0; step < Steps; step++) {
					mox.startCycleTimeAccumulation();
					moxWorld.stepMoxen();

					if (!mox.isAlive)
						break;

					if (mox.GetNeed(blue_food_need_idx) == 0.0) {
						forager_population[i].fitness += 1.0 + (1.0 / (double)(step + 1));
						mox.SetNeed(blue_food_need_idx, ForagerMox.BLUE_FOOD_NEED_VALUE);
					}

					if (mox.getCycleTimeAccumulator() > MaxMoxCycleTime) {
						excessCycleTime = mox.getCycleTimeAccumulator();
						break;
					}

					if (StepGameOfLife)
						moxWorld.stepGameOfLife();

					if (dashboard) {
						dashboard = moxWorld.updatedashboard(step + 1, Steps,
															 "generation=" + generation +
															 ", member=" + i + ", mox=" + mox.id +
															 ", world=" + j +
															 ", blue food need=" +
															 mox.GetNeed(blue_food_need_idx));
					}
				}

				moxWorld.destroydashboard();
				moxWorld.setMoxen(new ArrayList<Mox>());
				moxWorld.reset();
			}

			logEntry = "    member=" + i + ", " + forager_population[i].getInfo();

			if (!mox.isAlive)
				logEntry = logEntry + ", eaten";

			if (excessCycleTime > 0)
				logEntry = logEntry + ", excess cycle time=" + excessCycleTime;

			log(logEntry);
		}

		if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY)
			return;

		// Evaluate predators.
		log("  Predators:");

		for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++) {
			// Set up mox world with random fit prey.
			mox = forager_population[randomizer.nextInt(EvolveCommon.FORAGER_FIT_POPULATION_SIZE)].mox;
			mox.SetNeed(blue_food_need_idx, ForagerMox.BLUE_FOOD_NEED_VALUE);
			moxen = new ArrayList<Mox>(2);
			moxen.Add(0, mox);
			mox = Predatorpopulation[i].mox;
			mox.SetNeed(mox_food_need_idx, PredatorMox.MOX_FOOD_NEED_VALUE);
			moxen.Add(1, mox);
			excessCycleTime = 0;
			Predatorpopulation[i].fitness = 0.0;

			for (j = 0; j < moxWorlds.GetCount(); j++) {
				moxWorld = moxWorlds.Get(j);
				moxWorld.setMoxen(moxen);
				moxWorld.reset();

				// Step world.
				if (dashboard) {
					moxWorld.createdashboard();
					dashboard = moxWorld.updatedashboard(0, Steps,
														 "generation=" + generation +
														 ", member=" + i + ", mox=" + mox.id +
														 ", world=" + j +
														 ", mox food need=" +
														 mox.GetNeed(mox_food_need_idx));
				}

				moxFoodStep = Steps;

				for (step = 0; step < Steps; step++) {
					mox.startCycleTimeAccumulation();
					moxWorld.stepMoxen();

					if (mox.GetNeed(mox_food_need_idx) == 0.0) {
						moxFoodStep = step;
						break;
					}

					if (mox.getCycleTimeAccumulator() > MaxMoxCycleTime) {
						excessCycleTime = mox.getCycleTimeAccumulator();
						break;
					}

					if (StepGameOfLife)
						moxWorld.stepGameOfLife();

					if (dashboard) {
						dashboard = moxWorld.updatedashboard(step + 1, Steps,
															 "generation=" + generation +
															 ", member=" + i + ", mox=" + mox.id +
															 ", world=" + j +
															 ", mox food need=" +
															 mox.GetNeed(mox_food_need_idx));
					}
				}

				Predatorpopulation[i].fitness += (double)moxFoodStep;
				moxWorld.destroydashboard();
				moxWorld.setMoxen(new ArrayList<Mox>());
				moxWorld.reset();
			}

			logEntry = "    member=" + i + ", " + Predatorpopulation[i].getInfo();

			if (excessCycleTime > 0)
				logEntry = logEntry + ", excess cycle time=" + excessCycleTime;

			log(logEntry);
		}
	}


	// Prepare new moxen for evaluation by giving them
	// experience equivalent to existing moxen.
	void prepareEvaluation(int generation) {
		int i, j, n, runs, step;
		int blue_food_need_idx, mox_food_need_idx;
		Mox mox;
		ArrayList<Mox> moxen;
		MoxWorld       moxWorld;
		blue_food_need_idx = ForagerMox.NEED_TYPE.BLUE_FOOD.GetValue();
		mox_food_need_idx  = PredatorMox.NEED_TYPE.MOX_FOOD.GetValue();
		log("  Preparing new moxen:");

		// Catch up to current generation.
		if (generation < EvolveCommon.MAX_PREPARATION_TRIALS)
			runs = generation;
		else
			runs = EvolveCommon.MAX_PREPARATION_TRIALS;

		for (n = 0; n < runs; n++) {
			log("    Run " + (n + 1) + " of " + runs);

			// Run new foragers.
			for (i = EvolveCommon.FORAGER_FIT_POPULATION_SIZE;
				 i < EvolveCommon.FORAGER_POPULATION_SIZE; i++) {
				// Set up mox world.
				if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY) {
					mox = forager_population[i].mox;
					mox.SetNeed(blue_food_need_idx, ForagerMox.BLUE_FOOD_NEED_VALUE);
					moxen = new ArrayList<Mox>(1);
					moxen.Add(0, mox);
				}
				else {
					// Set up mox world with random fit predator.
					mox = Predatorpopulation[randomizer.nextInt(EvolveCommon.PREDATOR_FIT_POPULATION_SIZE)].mox;
					mox.SetNeed(mox_food_need_idx, PredatorMox.MOX_FOOD_NEED_VALUE);
					moxen = new ArrayList<Mox>(2);
					moxen.Add(0, mox);
					mox = forager_population[i].mox;
					mox.SetNeed(blue_food_need_idx, ForagerMox.BLUE_FOOD_NEED_VALUE);
					moxen.Add(1, mox);
				}

				// Run mox in all mox worlds.
				forager_population[i].fitness = 0.0;

				for (j = 0; j < moxWorlds.GetCount(); j++) {
					moxWorld = moxWorlds.Get(j);
					moxWorld.setMoxen(moxen);
					moxWorld.reset();

					// Step world.
					for (step = 0; step < Steps; step++) {
						mox.startCycleTimeAccumulation();
						moxWorld.stepMoxen();

						if (!mox.isAlive)
							break;

						if (mox.GetNeed(blue_food_need_idx) == 0.0)
							mox.SetNeed(blue_food_need_idx, ForagerMox.BLUE_FOOD_NEED_VALUE);

						if (mox.getCycleTimeAccumulator() > MaxMoxCycleTime)
							break;

						if (StepGameOfLife)
							moxWorld.stepGameOfLife();
					}

					moxWorld.destroydashboard();
					moxWorld.setMoxen(new ArrayList<Mox>());
					moxWorld.reset();
				}
			}

			if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY)
				continue;

			// Run new predators.
			for (i = EvolveCommon.PREDATOR_FIT_POPULATION_SIZE;
				 i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++) {
				// Set up mox world with random fit prey.
				mox = forager_population[randomizer.nextInt(EvolveCommon.FORAGER_FIT_POPULATION_SIZE)].mox;
				mox.SetNeed(blue_food_need_idx, ForagerMox.BLUE_FOOD_NEED_VALUE);
				moxen = new ArrayList<Mox>(2);
				moxen.Add(0, mox);
				mox = Predatorpopulation[i].mox;
				mox.SetNeed(mox_food_need_idx, PredatorMox.MOX_FOOD_NEED_VALUE);
				moxen.Add(1, mox);
				Predatorpopulation[i].fitness = 0.0;

				for (j = 0; j < moxWorlds.GetCount(); j++) {
					moxWorld = moxWorlds.Get(j);
					moxWorld.setMoxen(moxen);
					moxWorld.reset();

					// Step world.
					for (step = 0; step < Steps; step++) {
						mox.startCycleTimeAccumulation();
						moxWorld.stepMoxen();

						if (mox.GetNeed(mox_food_need_idx) == 0.0)
							break;

						if (mox.getCycleTimeAccumulator() > MaxMoxCycleTime)
							break;

						if (StepGameOfLife)
							moxWorld.stepGameOfLife();
					}

					moxWorld.destroydashboard();
					moxWorld.setMoxen(new ArrayList<Mox>());
					moxWorld.reset();
				}
			}
		}

		log("  Preparation completed");
	}


	// Prune unfit members.
	void Prune() {
		double min, max, d;
		int    i, j, m;
		EvolveCommon.Member member;
		log("Select:");
		log("  Foragers:");
		EvolveCommon.Member[] fitPopulation =
			new EvolveCommon.Member[EvolveCommon.FORAGER_FIT_POPULATION_SIZE];
		max = 0.0;

		for (i = 0; i < EvolveCommon.FORAGER_FIT_POPULATION_SIZE; i++) {
			m = -1;

			for (j = 0; j < EvolveCommon.FORAGER_POPULATION_SIZE; j++) {
				member = forager_population[j];

				if (member == null)
					continue;

				if ((m == -1) || (member.fitness > max)) {
					m   = j;
					max = member.fitness;
				}
			}

			member = forager_population[m];
			forager_population[m] = null;
			fitpopulation[i]     = member;
			log("    " + member.getInfo());
		}

		for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++) {
			if (forager_population[i] != null) {
				forager_population[i].Clear();
				forager_population[i] = null;
			}
		}

		d = 0.0;

		for (i = 0; i < EvolveCommon.FORAGER_FIT_POPULATION_SIZE; i++) {
			forager_population[i] = fitpopulation[i];
			fitpopulation[i]     = null;
			d += forager_population[i].fitness;
		}

		ForagerFittest[generation] = forager_population[0].fitness;
		ForagerAverage[generation] = d / (double)EvolveCommon.FORAGER_FIT_POPULATION_SIZE;

		if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY)
			return;

		log("  Predators:");
		fitPopulation =
			new EvolveCommon.Member[EvolveCommon.PREDATOR_FIT_POPULATION_SIZE];
		min = 0.0;

		for (i = 0; i < EvolveCommon.PREDATOR_FIT_POPULATION_SIZE; i++) {
			m = -1;

			for (j = 0; j < EvolveCommon.PREDATOR_POPULATION_SIZE; j++) {
				member = Predatorpopulation[j];

				if (member == null)
					continue;

				if ((m == -1) || (member.fitness < min)) {
					m   = j;
					min = member.fitness;
				}
			}

			member = Predatorpopulation[m];
			Predatorpopulation[m] = null;
			fitpopulation[i]      = member;
			log("    " + member.getInfo());
		}

		for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++) {
			if (Predatorpopulation[i] != null)
				Predatorpopulation[i] = null;
		}

		d = 0.0;

		for (i = 0; i < EvolveCommon.PREDATOR_FIT_POPULATION_SIZE; i++) {
			Predatorpopulation[i] = fitpopulation[i];
			d += Predatorpopulation[i].fitness;
		}

		PredatorFittest[generation] = Predatorpopulation[0].fitness;
		PredatorAverage[generation] = d / (double)EvolveCommon.PREDATOR_FIT_POPULATION_SIZE;
	}


	// Mutate members.
	void Mutate() {
		int i, j;
		EvolveCommon.Member member, mutant;
		log("Mutate:");
		log("  Foragers:");

		for (i = 0; i < EvolveCommon.FORAGER_NUM_MUTANTS; i++) {
			// Select a fit member to mutate.
			j      = randomizer.nextInt(EvolveCommon.FORAGER_FIT_POPULATION_SIZE);
			member = forager_population[j];
			// Create mutant member.
			mutant = new EvolveCommon.Member(member, member.generation + 1, randomizer);
			forager_population[EvolveCommon.FORAGER_FIT_POPULATION_SIZE + i] = mutant;
			log("    member=" + j + ", " + member.getInfo() +
				" -> member=" + (EvolveCommon.FORAGER_FIT_POPULATION_SIZE + i) +
				", " + mutant.getInfo());
		}

		if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY)
			return;

		log("  Predators:");

		for (i = 0; i < EvolveCommon.PREDATOR_NUM_MUTANTS; i++) {
			// Select a fit member to mutate.
			j      = randomizer.nextInt(EvolveCommon.PREDATOR_FIT_POPULATION_SIZE);
			member = Predatorpopulation[j];
			// Create mutant member.
			mutant = new EvolveCommon.Member(member, member.generation + 1, randomizer);
			Predatorpopulation[EvolveCommon.PREDATOR_FIT_POPULATION_SIZE + i] = mutant;
			log("    member=" + j + ", " + member.getInfo() +
				" -> member=" + (EvolveCommon.PREDATOR_FIT_POPULATION_SIZE + i) +
				", " + mutant.getInfo());
		}
	}


	// Produce offspring by melding parent parameters.
	void DoMating() {
		int i, j, k;
		EvolveCommon.Member member1, member2, offspring;
		log("Mate:");

		if (EvolveCommon.FORAGER_FIT_POPULATION_SIZE > 1) {
			log("  Foragers:");

			for (i = 0; i < EvolveCommon.FORAGER_NUM_OFFSPRING; i++) {
				// Select a pair of fit members to mate.
				j       = randomizer.nextInt(EvolveCommon.FORAGER_FIT_POPULATION_SIZE);
				member1 = forager_population[j];

				while ((k = randomizer.nextInt(EvolveCommon.FORAGER_FIT_POPULATION_SIZE)) == j) {}

				member2 = forager_population[k];
				// Create offspring.
				offspring = new EvolveCommon.Member(member1, member2,
													(member1.generation > member2.generation ?
													 member1.generation : member2.generation) + 1, randomizer);
				forager_population[EvolveCommon.FORAGER_FIT_POPULATION_SIZE +
																			EvolveCommon.FORAGER_NUM_MUTANTS + i] = offspring;
				log("    member=" + j + ", " + member1.getInfo() + " + member=" +
					k + ", " + member2.getInfo() +
					" -> member=" + (EvolveCommon.FORAGER_FIT_POPULATION_SIZE +
									 EvolveCommon.FORAGER_NUM_MUTANTS + i) +
					", " + offspring.getInfo());
			}
		}

		if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY)
			return;

		if (EvolveCommon.PREDATOR_FIT_POPULATION_SIZE > 1) {
			log("  Predators:");

			for (i = 0; i < EvolveCommon.PREDATOR_NUM_OFFSPRING; i++) {
				// Select a pair of fit members to mate.
				j       = randomizer.nextInt(EvolveCommon.PREDATOR_FIT_POPULATION_SIZE);
				member1 = Predatorpopulation[j];

				while ((k = randomizer.nextInt(EvolveCommon.PREDATOR_FIT_POPULATION_SIZE)) == j) {}

				member2 = Predatorpopulation[k];
				// Create offspring.
				offspring = new EvolveCommon.Member(member1, member2,
													(member1.generation > member2.generation ?
													 member1.generation : member2.generation) + 1, randomizer);
				Predatorpopulation[EvolveCommon.PREDATOR_FIT_POPULATION_SIZE +
																			 EvolveCommon.PREDATOR_NUM_MUTANTS + i] = offspring;
				log("    member=" + j + ", " + member1.getInfo() + " + member=" +
					k + ", " + member2.getInfo() +
					" -> member=" + (EvolveCommon.PREDATOR_FIT_POPULATION_SIZE +
									 EvolveCommon.PREDATOR_NUM_MUTANTS + i) +
					", " + offspring.getInfo());
			}
		}
	}


	// Extract moxen files.
	void extract() {
		int      i, j;
		int      blue_food_need_idx, mox_food_need_idx;
		Mox      mox;
		String   filename;
		MoxWorld moxWorld;
		// Extract foragers.
		blue_food_need_idx = ForagerMox.NEED_TYPE.BLUE_FOOD.GetValue();

		for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++) {
			// Set up mox world.
			mox = forager_population[i].mox;
			mox.SetNeed(blue_food_need_idx, ForagerMox.BLUE_FOOD_NEED_VALUE);
			ArrayList<Mox> moxen = new ArrayList<Mox>(1);
			moxen.Add(0, mox);

			// Save mox world.
			for (j = 0; j < moxWorlds.GetCount(); j++) {
				filename = "mox_world_forager_" + mox.id + "_world_" + j + ".mw";
				moxWorld = moxWorlds.Get(j);
				moxWorld.setMoxen(moxen);
				moxWorld.reset();

				try {
					moxWorld.Store(filename);
				}
				catch (Exception e) {
					System.err.println("Cannot save mox world to file " + filename +
									   ":" + e.getMessage());
					System.exit(1);
				}
			}
		}

		if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY)
			return;

		// Extract predators.
		mox_food_need_idx = PredatorMox.NEED_TYPE.MOX_FOOD.GetValue();

		for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++) {
			// Set up mox world.
			mox = Predatorpopulation[i].mox;
			mox.SetNeed(mox_food_need_idx, PredatorMox.MOX_FOOD_NEED_VALUE);
			ArrayList<Mox> moxen = new ArrayList<Mox>(1);
			moxen.Add(0, mox);

			// Save mox world.
			for (j = 0; j < moxWorlds.GetCount(); j++) {
				filename = "mox_world_predator_" + mox.id + "_world_" + j + ".mw";
				moxWorld = moxWorlds.Get(j);
				moxWorld.setMoxen(moxen);
				moxWorld.reset();

				try {
					moxWorld.Store(filename);
				}
				catch (Exception e) {
					System.err.println("Cannot save mox world to file " + filename +
									   ":" + e.getMessage());
					System.exit(1);
				}
			}
		}
	}


	// Print population properties.
	void printProperties() {
		int i;
		System.out.println("Population properties:");
		// Print foragers.
		System.out.println("=============================");
		System.out.println("Foragers:");

		for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++) {
			System.out.println("-----------------------------");
			forager_population[i].printProperties();
		}

		if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY)
			return;

		// Print predators.
		System.out.println("=============================");
		System.out.println("Predators:");

		for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++) {
			System.out.println("-----------------------------");
			Predatorpopulation[i].printProperties();
		}
	}


	// Print evolution statistics.
	void printStatistics() {
		int i;
		System.out.println("Evolution statistics:");
		// Print forager statistics.
		System.out.println("Foragers:");
		System.out.println("generation\tFittest");

		for (i = 0; i < generation; i++)
			System.out.println(i + "\t\t" + ForagerFittest[i]);

		System.out.println("generation\tAverage");

		for (i = 0; i < generation; i++)
			System.out.println(i + "\t\t" + ForagerAverage[i]);

		if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY)
			return;

		// Print predator statistics.
		System.out.println("Predators:");
		System.out.println("generation\tFittest");

		for (i = 0; i < generation; i++)
			System.out.println(i + "\t\t" + PredatorFittest[i]);

		System.out.println("generation\tAverage");

		for (i = 0; i < generation; i++)
			System.out.println(i + "\t\t" + PredatorAverage[i]);
	}


	// Logging.
	void log(String message) {
		if (log_writer != null) {
			log_writer.println(message);
			log_writer.flush();
		}
	}


	// Main.
	public static void main(String[] args) {
		EvolveMoxen evolveMoxen = new EvolveMoxen(args);
		evolveMoxen.start();
		System.exit(0);
	}
}
