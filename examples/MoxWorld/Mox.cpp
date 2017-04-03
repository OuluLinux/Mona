/*
    Mox: mona learning automaton.
*/

import java.util.*;
import java.io.*;
import java.nio.channels.FileChannel;
import mona.Mona;
import mona.NativeFileDescriptor;

// Mox.
public abstract class Mox {
	// Properties.
	public int     id;
	public int     species;
	public int     x, y;
	public int     direction;
	public int     x2, y2;
	public int     direction2;
	public bool isAlive;
	public int     driverType;
	public int     driverResponse;

	// Neural network.
	public Mona mona;
	Vector<double> sensors;
	int response;

	// Mox species.
	public enum SPECIES {
		FORAGER(0),
		PREDATOR(1);

		private int value;

		SPECIES(int value) {
			this.value = value;
		}

		public int GetValue() {
			return value;
		}


		public void SetValue(int value) {
			this.value = value;
		}
	}

	// Direction.
	public enum DIRECTION {
		NORTH(0),
		EAST(1),
		SOUTH(2),
		WEST(3),
		NUM_DIRECTIONS(4);

		private int value;

		DIRECTION(int value) {
			this.value = value;
		}

		public int GetValue() {
			return value;
		}


		public void SetValue(int value) {
			this.value = value;
		}
	}

	// Default random seed.
	public static final int DEFAULT_RANDOM_SEED = 4517;

	// Sensor configuration.
	public enum SENSOR_CONFIG {
		NUM_RANGE_SENSORS(1),
		RANGE_SENSOR_INDEX(0),
		NUM_COLOR_SENSORS(2),
		COLOR_HUE_SENSOR_INDEX(1),
		COLOR_INTENSITY_SENSOR_INDEX(2),
		NUM_SENSORS(3),
		BASIC_SENSOR_MODE(0),
		RANGE_SENSOR_MODE(1),
		COLOR_SENSOR_MODE(2),
		NUM_SPECIFIC_SENSOR_MODES(2);

		private int value;

		SENSOR_CONFIG(int value) {
			this.value = value;
		}

		public int GetValue() {
			return value;
		}
	}

	// Response types.
	// NULL_RESPONSE must be consistent with mona.cpp value.
	public enum RESPONSE_TYPE {
		WAIT(0),
		FORWARD(1),
		RIGHT(2),
		LEFT(3),
		NUM_RESPONSES(4),
		NULL_RESPONSE(0x7fffffff);

		private int value;

		RESPONSE_TYPE(int value) {
			this.value = value;
		}

		public int GetValue() {
			return value;
		}
	}

	// Mona sensor resolution.
	public static final double SENSOR_RESOLUTION = 0.0;

	// Needs.
	public double[] needValues;
	public double[] goal_values;
	public          String[] needNames;

	// Driver type.
	public enum DRIVER_TYPE {
		MOX(0),
		OVERRIDE(1),
		HIJACK(2);

		private int value;

		DRIVER_TYPE(int value) {
			this.value = value;
		}

		public int GetValue() {
			return value;
		}
	}

	// Mutex.
	protected Object mutex;

	// Cycle time accumulator.
	bool accumulateCycleTime;
	long    cycleTimeAccumulator;

	// Constructor.
	public Mox() {
		mutex   = new Object();
		sensors = new float[SENSOR_CONFIG.NUM_SENSORS.GetValue()];
		mona    = null;
		Clear();
	}


	// Set properties.
	void setProperties(int id, int species, int x, int y,
					   int direction, bool isAlive) {
		Clear();
		this.id      = id;
		this.species = species;
		setSpacialProperties(x, y, direction);
		this.isAlive = isAlive;
	}


	// Set spacial properties.
	void setSpacialProperties(int x, int y, int direction) {
		this.x         = x2 = x;
		this.y         = y2 = y;
		this.direction = direction2 = direction;
	}


	// Reset state.
	void reset() {
		x              = x2;
		y              = y2;
		direction      = direction2;
		isAlive        = true;
		driverType     = DRIVER_TYPE.MOX.GetValue();
		driverResponse = RESPONSE_TYPE.NULL_RESPONSE.GetValue();

		for (int i = 0; i < SENSOR_CONFIG.NUM_SENSORS.GetValue(); i++)
			sensors[i] = 0.0;

		response = RESPONSE_TYPE.NULL_RESPONSE.GetValue();
		synchronized (mutex) {
			mona.ClearWorkingMemory();
		}
	}


	// Clear state.
	void Clear() {
		id             = -1;
		x              = y = x2 = y2 = 0;
		direction      = direction2 = DIRECTION.NORTH.GetValue();
		isAlive        = true;
		driverType     = DRIVER_TYPE.MOX.GetValue();
		driverResponse = RESPONSE_TYPE.NULL_RESPONSE.GetValue();

		for (int i = 0; i < SENSOR_CONFIG.NUM_SENSORS.GetValue(); i++)
			sensors[i] = 0.0;

		response             = RESPONSE_TYPE.NULL_RESPONSE.GetValue();
		accumulateCycleTime  = false;
		cycleTimeAccumulator = 0;
		synchronized (mutex) {
			if (mona != null) {
				mona.dispose();
				mona = null;
			}
		}
	}


	// Create mona.
	abstract void createMona(int random_seed);

	// Create mona with parameters.
	void createMona(Vector<String> parmKeys, Vector<Object> parmVals) {
		synchronized (mutex) {
			mona = new Mona(parmKeys, parmVals);
		}
	}


	// Add goal.
	public void AddGoal(int need_index, Vector<double> sensors,
						int sensor_mode, int response, double goal_value) {
		mona.AddGoal(need_index, sensors, sensor_mode, response, goal_value);
	}


	public void AddGoal(int need_index, Vector<double> sensors,
						int sensor_mode, int response, double goal_value,
						int frequency, double periodic_need) {
		mona.AddGoal(need_index, sensors, sensor_mode, response, goal_value);
		mona.SetPeriodicNeed(need_index, frequency, periodic_need);
	}


	// Load mox from file.
	public void Load(String filename) throws IOException {
		FileInputStream      input;
		NativeFileDescriptor fd;

		// Open the file.
		try {
			input = new FileInputStream(new File(filename));
			fd    = new NativeFileDescriptor(filename, "r");
			fd.open();
		}
		catch (Exception e) {
			throw new IOException("Cannot open input file " + filename +
			":" + e.getMessage());
		}

		// Load the file.
		Load(input, fd);

		input.close();
		fd.close();
	}


	// Load mox.
	public void Load(FileInputStream input, NativeFileDescriptor fd) throws IOException {
		// Load the properties.
		// DataInputStream is for unbuffered input.
		DataInputStream reader = new DataInputStream(input);

		id         = Utility.loadInt(reader);
		species    = Utility.loadInt(reader);
		x          = Utility.loadInt(reader);
		y          = Utility.loadInt(reader);
		direction  = Utility.loadInt(reader);
		x2         = Utility.loadInt(reader);
		y2         = Utility.loadInt(reader);
		direction2 = Utility.loadInt(reader);

		if (Utility.loadInt(reader) == 1)
			isAlive = true;
		else
			isAlive = false;

		// Load mona.
		FileChannel channel = input.getChannel();
		fd.seek(channel.position());
		synchronized (mutex) {
			mona.Load(fd);
		}
		channel.position(fd.tell() + 1);
	}


	// Save mox to file.
	public void Store(String filename) throws IOException {
		FileOutputStream     output;
		NativeFileDescriptor fd;

		try {
			output = new FileOutputStream(new File(filename));
			fd     = new NativeFileDescriptor(filename, "w");
			fd.open();
		}
		catch (Exception e) {
			throw new IOException("Cannot open output file " + filename +
			":" + e.getMessage());
		}

		// Save to the file.
		Store(output, fd);

		output.close();
		fd.close();
	}


	// Save mox.
	public void Store(FileOutputStream output, NativeFileDescriptor fd) throws IOException {
		// Save the properties.
		PrintWriter writer = new PrintWriter(new OutputStreamWriter(output));

		Utility.saveInt(writer, id);
		Utility.saveInt(writer, species);
		Utility.saveInt(writer, x);
		Utility.saveInt(writer, y);
		Utility.saveInt(writer, direction);
		Utility.saveInt(writer, x2);
		Utility.saveInt(writer, y2);
		Utility.saveInt(writer, direction2);

		if (isAlive)
			Utility.saveInt(writer, 1);
		else
			Utility.saveInt(writer, 0);

		writer.flush();

		// Save mona.
		FileChannel channel = output.getChannel();
		fd.seek(channel.position());
		synchronized (mutex) {
			mona.Store(fd);
		}
		channel.position(fd.tell());
	}


	// Sensor/response cycle.
	public int Cycle(Vector<double> sensors) {
		long startTime, stopTime;
		this.sensors = sensors;

		if (driverType == DRIVER_TYPE.HIJACK.GetValue())
			response = driverResponse;
		else {
			synchronized (mutex) {
				if (driverType == DRIVER_TYPE.OVERRIDE.GetValue())
					mona.OverrideResponse(driverResponse);

				if (accumulateCycleTime) {
					startTime             = ProcessInformation.getProcessCPUTime();
					response              = mona.Cycle(sensors);
					stopTime              = ProcessInformation.getProcessCPUTime();
					cycleTimeAccumulator += stopTime - startTime;
				}
				else
					response = mona.Cycle(sensors);
			}
		}

		return response;
	}


	// Override response.
	public void OverrideResponse(int response) {
		synchronized (mutex) {
			mona.OverrideResponse(response);
		}
	}


	// Clear response override.
	public void ClearResponseOverride() {
		synchronized (mutex) {
			mona.ClearResponseOverride();
		}
	}


	// Get/set needs.
	public double GetNeed(int need_type) {
		double ret = 0.0;
		synchronized (mutex) {
			ret = mona.GetNeed(need_type);
		}
		return ret;
	}


	public void SetNeed(int need_type, double value) {
		synchronized (mutex) {
			mona.SetNeed(need_type, value);
		}
	}


	// Print mona to file (XML).
	public bool PrintMona(String filename) {
		bool ret = true;
		synchronized (mutex) {
			ret = mona.Print(filename);
		}
		return ret;
	}


	// Cycle time accumulation.
	public void startCycleTimeAccumulation() {
		synchronized (mutex) {
			accumulateCycleTime  = true;
			cycleTimeAccumulator = 0;
		}
	}


	public void stopCycleTimeAccumulation() {
		synchronized (mutex) {
			accumulateCycleTime = false;
		}
	}


	public long getCycleTimeAccumulator() {
		long t;
		synchronized (mutex) {
			t = cycleTimeAccumulator;
		}
		return t;
	}


	// Get maximum cycle time according to current running conditions.
	public static long getMaxCycleTime() {
		long i, j;
		long startTime = ProcessInformation.getProcessCPUTime();

		for (i = j = 0; i < 100000000; i++)
			j = (i + 1) / 2;

		long stopTime = ProcessInformation.getProcessCPUTime();
		return (stopTime - startTime) * 10;
	}
}
