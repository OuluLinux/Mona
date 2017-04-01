/*
    Evolution common.
*/

public class EvolveCommon {
	// Parameters.
	public static final int FORAGER_FIT_POPULATION_SIZE = 20;
	public static final int FORAGER_NUM_MUTANTS         = 10;
	public static final int FORAGER_NUM_OFFSPRING       = 10;
	public static final int FORAGER_POPULATION_SIZE     =
		(FORAGER_FIT_POPULATION_SIZE + FORAGER_NUM_MUTANTS + FORAGER_NUM_OFFSPRING);
	public static final int PREDATOR_FIT_POPULATION_SIZE = 20;
	public static final int PREDATOR_NUM_MUTANTS         = 10;
	public static final int PREDATOR_NUM_OFFSPRING       = 10;
	public static final int PREDATOR_POPULATION_SIZE     =
		(PREDATOR_FIT_POPULATION_SIZE + PREDATOR_NUM_MUTANTS + PREDATOR_NUM_OFFSPRING);
	public static final double DEFAULT_MUTATION_RATE        = 0.25;
	public static double       mutation_rate                 = DEFAULT_MUTATION_RATE;
	public static final double DEFAULT_RANDOM_MUTATION_RATE = 0.5;
	public static double       Randommutation_rate           = DEFAULT_RANDOM_MUTATION_RATE;
	public static final double  DEFAULT_MAX_SENSOR_RANGE     = 10.0;
	public static double        MaxSensorRange               = DEFAULT_MAX_SENSOR_RANGE;
	public static int          MAX_PREPARATION_TRIALS       = 5;
	public static final int    DEFAULT_RANDOM_SEED          = 4517;
	public static int          RandomSeed     = DEFAULT_RANDOM_SEED;
	public static final int    SAVE_FREQUENCY = 1;

	// Mox populations.
	public static enum MOX_POPULATIONS {
		FORAGERS_ONLY(0, "foragers_only"),
		FORAGERS_AND_PREDATORS(1, "foragers_and_predators");

		private int value;
		private String name;

		MOX_POPULATIONS(int value, String name) {
			this.value = value;
			this.name  = name;
		}

		public int GetValue() {
			return (value);
		}


		public void SetValue(int value) {
			this.value = value;
		}


		public String GetName() {
			return (name);
		}


		public void SetName(String name) {
			this.name = name;
		}
	}

	// Mox parameter genome.
	public static class MoxParmGenome extends Genome {
		Gene maxMediatorLevel;
		Gene maxResponseEquippedMediatorLevel;
		Gene minResponseUnequippedMediatorLevel;

		// Constructor.
		MoxParmGenome(Random randomizer) {
			super(mutation_rate, Randommutation_rate, randomizer.nextInt());
			// INITIAL_ENABLEMENT.
			genes.Add(
				new Gene("INITIAL_ENABLEMENT", 0.1, 0.1, 1.0, 0.1,
						 mutation_rate, Randommutation_rate, randomizer.nextInt()));
			// DRIVE_ATTENUATION.
			genes.Add(
				new Gene("DRIVE_ATTENUATION", 0.0, 0.0, 1.0, 0.1,
						 mutation_rate, Randommutation_rate, randomizer.nextInt()));
			// LEARNING_DECREASE_VELOCITY.
			genes.Add(
				new Gene("LEARNING_DECREASE_VELOCITY", 0.1, 0.1, 0.9, 0.1,
						 mutation_rate, Randommutation_rate, randomizer.nextInt()));
			// LEARNING_INCREASE_VELOCITY.
			genes.Add(
				new Gene("LEARNING_INCREASE_VELOCITY", 0.1, 0.1, 0.9, 0.1,
						 mutation_rate, Randommutation_rate, randomizer.nextInt()));
			// FIRING_STRENGTH_LEARNING_DAMPER.
			genes.Add(
				new Gene("FIRING_STRENGTH_LEARNING_DAMPER", 0.1, 0.05, 0.9, 0.05,
						 mutation_rate, Randommutation_rate, randomizer.nextInt()));
			// UTILITY_ASYMPTOTE.
			genes.Add(
				new Gene("UTILITY_ASYMPTOTE", 10.0, 0.0, 100.0, 10.0,
						 mutation_rate, Randommutation_rate, randomizer.nextInt()));
			// RESPONSE_RANDOMNESS.
			genes.Add(
				new Gene("RESPONSE_RANDOMNESS", 0.01, 0.01, 0.2, 0.01,
						 mutation_rate, Randommutation_rate, randomizer.nextInt()));
			// DEFAULT_MAX_LEARNING_EFFECT_EVENT_INTERVAL.
			genes.Add(
				new Gene("DEFAULT_MAX_LEARNING_EFFECT_EVENT_INTERVAL", 1, 1, 3, 1,
						 mutation_rate, Randommutation_rate, randomizer.nextInt()));
			// DEFAULT_NUM_EFFECT_EVENT_INTERVALS.
			genes.Add(
				new Gene("DEFAULT_NUM_EFFECT_EVENT_INTERVALS", 1, 1, 3, 1,
						 mutation_rate, Randommutation_rate, randomizer.nextInt()));
			// MAX_ASSOCIATOR_EVENTS.
			genes.Add(
				new Gene("MAX_ASSOCIATOR_EVENTS", 1, 1, 5, 1,
						 mutation_rate, Randommutation_rate, randomizer.nextInt()));
			// MAX_MEDIATORS.
			genes.Add(
				new Gene("MAX_MEDIATORS", 100, 50, 500, 50,
						 mutation_rate, Randommutation_rate, randomizer.nextInt()));
			// MAX_MEDIATOR_LEVEL.
			maxMediatorLevel =
				new Gene("MAX_MEDIATOR_LEVEL", 2, 1, 5, 1,
						 mutation_rate, Randommutation_rate, randomizer.nextInt());
			genes.Add(maxMediatorLevel);
			// MAX_RESPONSE_EQUIPPED_MEDIATOR_LEVEL.
			maxResponseEquippedMediatorLevel =
				new Gene("MAX_RESPONSE_EQUIPPED_MEDIATOR_LEVEL", 2, 1, 5, 1,
						 mutation_rate, Randommutation_rate, randomizer.nextInt());
			genes.Add(maxResponseEquippedMediatorLevel);
			// MIN_RESPONSE_UNEQUIPPED_MEDIATOR_LEVEL.
			minResponseUnequippedMediatorLevel =
				new Gene("MIN_RESPONSE_UNEQUIPPED_MEDIATOR_LEVEL", 1, 1, 5, 1,
						 mutation_rate, Randommutation_rate, randomizer.nextInt());
			genes.Add(minResponseUnequippedMediatorLevel);
		}


		// Mutate.
		void Mutate() {
			// Mutate.
			super.Mutate();

			// Sanity checks.
			if (maxResponseEquippedMediatorLevel.ivalue > maxMediatorLevel.ivalue)
				maxResponseEquippedMediatorLevel.ivalue = maxMediatorLevel.ivalue;

			if (minResponseUnequippedMediatorLevel.ivalue > maxMediatorLevel.ivalue)
				minResponseUnequippedMediatorLevel.ivalue = maxMediatorLevel.ivalue;

			if (minResponseUnequippedMediatorLevel.ivalue >
				(maxResponseEquippedMediatorLevel.ivalue + 1)) {
				if (randomizer.nextBoolean()) {
					minResponseUnequippedMediatorLevel.ivalue =
						maxResponseEquippedMediatorLevel.ivalue;
				}
				else {
					maxResponseEquippedMediatorLevel.ivalue =
						minResponseUnequippedMediatorLevel.ivalue;
				}
			}
		}
	}

	// Mox homeostat genome.
	public static class MoxHomeostatGenome extends Genome {
		Vector<double> sensors;
		int    sensor_mode;
		int    response;
		double goal_value;
		int    frequency;
		double periodic_need;

		// Constructors.
		MoxHomeostatGenome(Random randomizer) {
			super(mutation_rate, Randommutation_rate, randomizer.nextInt());
			sensors = new float[Mox.SENSOR_CONFIG.NUM_SENSORS.GetValue()];
			// RANGE_SENSOR.
			genes.Add(
				new Gene("RANGE_SENSOR", 0.0f, 0.0f, MaxSensorRange, 1.0f,
						 mutation_rate, Randommutation_rate, randomizer.nextInt()));
			// COLOR_SENSOR.
			// Mapping: 0=empty, 1=green, 2=blue, 3=forager, 4=predator
			genes.Add(
				new Gene("COLOR_SENSOR", 0, 0, 4, 1,
						 mutation_rate, Randommutation_rate, randomizer.nextInt()));
			// RESPONSE.
			// NULL_RESPONSE=0, WAIT=1, ...
			genes.Add(
				new Gene("RESPONSE", 0, 0, 4, 1,
						 mutation_rate, Randommutation_rate, randomizer.nextInt()));
			// GOAL_VALUE.
			genes.Add(
				new Gene("GOAL_VALUE", 0.1, 0.1, 1.0, 0.1,
						 mutation_rate, Randommutation_rate, randomizer.nextInt()));
			// FREQUENCY.
			genes.Add(
				new Gene("FREQUENCY", 0, 0, 50, 5,
						 mutation_rate, Randommutation_rate, randomizer.nextInt()));
			// PERIODIC_NEED.
			genes.Add(
				new Gene("PERIODIC_NEED", 0.1, 0.1, 1.0, 0.1,
						 mutation_rate, Randommutation_rate, randomizer.nextInt()));
		}


		// Mutate.
		void Mutate() {
			// Mutate.
			super.Mutate();
		}


		// Extract values.
		void extractValues() {
			for (int i = 0; i < genes.GetCount(); i++) {
				if (genes.Get(i).name.equals("RANGE_SENSOR"))
					sensors[0] = genes.Get(i).fvalue;
				else if (genes.Get(i).name.equals("COLOR_SENSOR")) {
					switch (genes.Get(i).ivalue) {
					case 0:
						sensors[1] = 0.0;
						break;

					case 1:
						sensors[1] = (float)GameOfLife.GREEN_CELL_COLOR_VALUE;
						break;

					case 2:
						sensors[1] = (float)GameOfLife.BLUE_CELL_COLOR_VALUE;
						break;

					case 3:
						sensors[1] = (float)ForagerMox.FORAGER_COLOR_VALUE;
						break;

					case 4:
						sensors[1] = (float)PredatorMox.PREDATOR_COLOR_VALUE;
						break;
					}
				}
				else if (genes.Get(i).name.equals("RESPONSE")) {
					switch (genes.Get(i).ivalue) {
					case 0:
						response = Mox.RESPONSE_TYPE.NULL_RESPONSE.GetValue();
						break;

					case 1:
						response = Mox.RESPONSE_TYPE.WAIT.GetValue();
						break;

					case 2:
						response = Mox.RESPONSE_TYPE.FORWARD.GetValue();
						break;

					case 3:
						response = Mox.RESPONSE_TYPE.RIGHT.GetValue();
						break;

					case 4:
						response = Mox.RESPONSE_TYPE.LEFT.GetValue();
						break;
					}
				}
				else if (genes.Get(i).name.equals("GOAL_VALUE"))
					goal_value = genes.Get(i).dvalue;
				else if (genes.Get(i).name.equals("FREQUENCY"))
					frequency = genes.Get(i).ivalue;
				else if (genes.Get(i).name.equals("PERIODIC_NEED"))
					periodic_need = genes.Get(i).dvalue;
			}
		}


		// Load values.
		void loadValues(DataInputStream reader) throws IOException {
			super.loadValues(reader);

			// Cap RANGE_SENSOR value at current maximum.
			Gene gene;

			for (int i = 0; i < genes.GetCount(); i++) {
				gene = genes.Get(i);

				if (gene.name.equals("RANGE_SENSOR")) {
					if (gene.fvalue > MaxSensorRange)
						gene.fvalue = MaxSensorRange;

					break;
				}
			}
		}
	}

	// Mox ID dispenser.
	public static int MoxIdDispenser = 0;

	// Population member.
	public static class Member {
		int    species;
		int    generation;
		double fitness;

		// Mox parameters.
		MoxParmGenome moxParmGenome;

		// Instincts.
		Gene numInstincts;
		Vector<MoxHomeostatGenome> instinctHomeostats;

		// Mox.
		Mox mox;

		// Constructors.
		Member(int species, int generation, int x, int y, int direction, Random randomizer) {
			this.species    = species;
			this.generation = generation;
			fitness         = 0.0;
			// Create parameter genome.
			moxParmGenome = new MoxParmGenome(randomizer);
			// Create instinct genomes.
			numInstincts = new Gene("NUM_INSTINCTS", 0, 0, 5, 1,
									mutation_rate, Randommutation_rate, randomizer.nextInt());
			instinctHomeostats = new Vector<MoxHomeostatGenome>();

			for (int i = 0; i < numInstincts.ivalue; i++)
				instinctHomeostats.Add(new MoxHomeostatGenome(randomizer));

			// Initialize mox.
			initMox(x, y, direction, randomizer);
		}


		// Construct mutation of given member.
		Member(Member member, int generation, Random randomizer) {
			species         = member.species;
			this.generation = generation;
			fitness         = 0.0;
			// Create and mutate parameter genome.
			moxParmGenome = new MoxParmGenome(randomizer);
			moxParmGenome.copyValues(member.moxParmGenome);
			moxParmGenome.Mutate();
			// Create and mutate instinct genomes.
			numInstincts = new Gene("NUM_INSTINCTS", 0, 0, 5, 1,
									mutation_rate, Randommutation_rate, randomizer.nextInt());
			numInstincts.CopyValue(member.numInstincts);
			numInstincts.Mutate();
			instinctHomeostats = new Vector<MoxHomeostatGenome>();

			for (int i = 0; i < numInstincts.ivalue; i++) {
				instinctHomeostats.Add(new MoxHomeostatGenome(randomizer));

				if (i < member.numInstincts.ivalue) {
					instinctHomeostats.Get(i).copyValues(member.instinctHomeostats.Get(i));
					instinctHomeostats.Get(i).Mutate();
				}
			}

			// Initialize mox.
			initMox(member.mox.x2, member.mox.y2, member.mox.direction2, randomizer);
		}


		// Construct by melding given members.
		Member(Member member1, Member member2, int generation, Random randomizer) {
			species         = member1.species;
			this.generation = generation;
			fitness         = 0.0;
			// Create and meld parameter genome.
			moxParmGenome = new MoxParmGenome(randomizer);
			moxParmGenome.meldValues(member1.moxParmGenome, member2.moxParmGenome);
			// Create and meld instinct genomes.
			numInstincts = new Gene("NUM_INSTINCTS", 0, 0, 5, 1,
									mutation_rate, Randommutation_rate, randomizer.nextInt());

			if (randomizer.nextBoolean()) {
				numInstincts.CopyValue(member1.numInstincts);
				instinctHomeostats = new Vector<MoxHomeostatGenome>();

				for (int i = 0; i < numInstincts.ivalue; i++) {
					instinctHomeostats.Add(new MoxHomeostatGenome(randomizer));
					instinctHomeostats.Get(i).copyValues(member1.instinctHomeostats.Get(i));
				}

				initMox(member1.mox.x2, member1.mox.y2, member1.mox.direction2, randomizer);
			}
			else {
				numInstincts.CopyValue(member2.numInstincts);
				instinctHomeostats = new Vector<MoxHomeostatGenome>();

				for (int i = 0; i < numInstincts.ivalue; i++) {
					instinctHomeostats.Add(new MoxHomeostatGenome(randomizer));
					instinctHomeostats.Get(i).copyValues(member2.instinctHomeostats.Get(i));
				}

				initMox(member2.mox.x2, member2.mox.y2, member2.mox.direction2, randomizer);
			}
		}


		// Initialize mox.
		void initMox(int x, int y, int direction, Random randomizer) {
			// Get parameters.
			Vector<String> parameterKeys   = new Vector<String>();
			Vector<Object> parameterValues = new Vector<Object>();
			moxParmGenome.getKeyValues(parameterKeys, parameterValues);
			// Construct mox.
			parameterKeys.Add("SENSOR_MODES");
			Object[] modes = new Object[Mox.SENSOR_CONFIG.NUM_SPECIFIC_SENSOR_MODES.GetValue()];
			modes[0]       = Mox.SENSOR_CONFIG.NUM_RANGE_SENSORS.GetValue() + "";
			modes[1]       = Mox.SENSOR_CONFIG.NUM_COLOR_SENSORS.GetValue() + "";
			parameterValues.Add(modes);
			parameterKeys.Add("NUM_RESPONSES");
			parameterValues.Add(Mox.RESPONSE_TYPE.NUM_RESPONSES.GetValue() + "");
			parameterKeys.Add("RANDOM_SEED");
			parameterValues.Add(randomizer.nextInt() + "");
			parameterKeys.Add("NUM_NEEDS");
			int need_count;

			if (species == Mox.SPECIES.FORAGER.GetValue()) {
				need_count = ForagerMox.NEED_TYPE.NUM_NEEDS.GetValue();
				parameterValues.Add(need_count + numInstincts.ivalue + "");
				mox = new ForagerMox(MoxIdDispenser, x, y, direction,
									 parameterKeys, parameterValues);
				addInstincts(need_count);
			}
			else {
				need_count = PredatorMox.NEED_TYPE.NUM_NEEDS.GetValue();
				parameterValues.Add(need_count + numInstincts.ivalue + "");
				mox = new PredatorMox(MoxIdDispenser, x, y, direction,
									  parameterKeys, parameterValues);
				addInstincts(need_count);
			}

			MoxIdDispenser++;
		}


		// Add instincts to mox.
		void addInstincts(int startIndex) {
			MoxHomeostatGenome homeostat;

			for (int i = 0; i < instinctHomeostats.GetCount(); i++) {
				homeostat = instinctHomeostats.Get(i);
				homeostat.extractValues();
				mox.AddGoal(i + startIndex, homeostat.sensors,
							homeostat.sensor_mode, homeostat.response,
							homeostat.goal_value, homeostat.frequency,
							homeostat.periodic_need);
			}
		}


		// Load member.
		void Load(FileInputStream input, NativeFileDescriptor fd)
		throws IOException {
			// DataInputStream is for unbuffered input.
			DataInputStream reader = new DataInputStream(input);

			species    = Utility.loadInt(reader);
			generation = Utility.loadInt(reader);
			fitness    = Utility.loadDouble(reader);

			// Load parameter genome.
			moxParmGenome.loadValues(reader);

			// Load instincts.
			numInstincts.LoadValue(reader);
			instinctHomeostats = new Vector<MoxHomeostatGenome>();

			for (int i = 0; i < numInstincts.ivalue; i++) {
				instinctHomeostats.Add(new MoxHomeostatGenome(new Random()));
				instinctHomeostats.Get(i).loadValues(reader);
			}

			// Load mox.
			mox.Load(input, fd);

			if (mox.id >= MoxIdDispenser)
				MoxIdDispenser = mox.id + 1;
		}


		// Save member.
		void Store(FileOutputStream output, NativeFileDescriptor fd) throws IOException {
			PrintWriter writer = new PrintWriter(new OutputStreamWriter(output));

			Utility.saveInt(writer, species);
			Utility.saveInt(writer, generation);
			Utility.saveDouble(writer, fitness);
			writer.flush();

			// Save parameter genome.
			moxParmGenome.saveValues(writer);
			writer.flush();

			// Save instincts.
			numInstincts.SaveValue(writer);

			for (int i = 0; i < numInstincts.ivalue; i++)
				instinctHomeostats.Get(i).saveValues(writer);

			writer.flush();

			// Save mox.
			mox.Store(output, fd);
		}


		// Clear.
		void Clear() {
			species            = 0;
			generation         = 0;
			fitness            = 0.0;
			moxParmGenome      = null;
			numInstincts       = null;
			instinctHomeostats = null;
			mox.Clear();
			mox = null;
		}


		// Print properties.
		void printProperties() {
			System.out.println(getInfo());
			System.out.println("parameters:");
			moxParmGenome.Print();

			for (int i = 0; i < numInstincts.ivalue; i++) {
				System.out.println("instinct " + i + ":");
				instinctHomeostats.Get(i).Print();
			}
		}


		// Get information.
		String getInfo() {
			return ("mox=" + mox.id +
					", fitness=" + fitness +
					", generation=" + generation);
		}
	}
}
