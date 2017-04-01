/*
    Gene.
*/


public class Gene {
	// Mutation rate.
	double mutation_rate;

	// Probability of random mutation.
	double random_mutation_rate;

	// Random numbers.
	int    random_seed;
	Random randomizer;

	// Value types.
	enum VALUE_TYPE {
		INTEGER_VALUE, FLOAT_VALUE, DOUBLE_VALUE
	};

	// Mutable value.
	VALUE_TYPE type;
	String     name;
	int        ivalue, imin, imax, idelta;
	double      fvalue, fmin, fmax, fdelta;
	double     dvalue, dmin, dmax, ddelta;

	// Constructors.
	Gene(double mutation_rate, double random_mutation_rate, int random_seed) {
		type                    = VALUE_TYPE.DOUBLE_VALUE;
		name                    = null;
		ivalue                  = imin = imax = idelta = 0;
		fvalue                  = fmin = fmax = fdelta = 0.0;
		dvalue                  = dmin = dmax = ddelta = 0.0;
		this.mutation_rate       = mutation_rate;
		this.random_mutation_rate = random_mutation_rate;
		this.random_seed         = random_seed;
		randomizer              = new Random(random_seed);
	}


	Gene(String name, int value, int min, int max, int delta,
		 double mutation_rate, double random_mutation_rate, int random_seed) {
		type                    = VALUE_TYPE.INTEGER_VALUE;
		this.name               = new String(name);
		ivalue                  = imin = imax = idelta = 0;
		fvalue                  = fmin = fmax = fdelta = 0.0;
		dvalue                  = dmin = dmax = ddelta = 0.0;
		ivalue                  = value;
		imin                    = min;
		imax                    = max;
		idelta                  = delta;
		this.mutation_rate       = mutation_rate;
		this.random_mutation_rate = random_mutation_rate;
		this.random_seed         = random_seed;
		randomizer              = new Random(random_seed);
	}


	Gene(String name, double value, double min, double max, double delta,
		 double mutation_rate, double random_mutation_rate, int random_seed) {
		type                    = VALUE_TYPE.FLOAT_VALUE;
		this.name               = new String(name);
		ivalue                  = imin = imax = idelta = 0;
		fvalue                  = fmin = fmax = fdelta = 0.0;
		dvalue                  = dmin = dmax = ddelta = 0.0;
		fvalue                  = value;
		fmin                    = min;
		fmax                    = max;
		fdelta                  = delta;
		this.mutation_rate       = mutation_rate;
		this.random_mutation_rate = random_mutation_rate;
		this.random_seed         = random_seed;
		randomizer              = new Random(random_seed);
	}


	Gene(String name, double value, double min, double max, double delta,
		 double mutation_rate, double random_mutation_rate, int random_seed) {
		type                    = VALUE_TYPE.DOUBLE_VALUE;
		this.name               = new String(name);
		ivalue                  = imin = imax = idelta = 0;
		fvalue                  = fmin = fmax = fdelta = 0.0;
		dvalue                  = dmin = dmax = ddelta = 0.0;
		dvalue                  = value;
		dmin                    = min;
		dmax                    = max;
		ddelta                  = delta;
		this.mutation_rate       = mutation_rate;
		this.random_mutation_rate = random_mutation_rate;
		this.random_seed         = random_seed;
		randomizer              = new Random(random_seed);
	}


	// Mutate gene.
	void Mutate() {
		int    i;
		double  f;
		double d;

		if (randomizer.nextDouble() > mutation_rate)
			return;

		switch (type) {
		case INTEGER_VALUE:
			if (randomizer.nextDouble() <= random_mutation_rate) {
				i = imax - imin;

				if (i > 0)
					ivalue = randomizer.nextInt(imax - imin) + imin;
				else
					ivalue = imin;
			}
			else {
				i = ivalue;

				if (randomizer.nextBoolean()) {
					i += idelta;

					if (i > imax)
						i = imax;
				}
				else {
					i -= idelta;

					if (i < imin)
						i = imin;
				}

				ivalue = i;
			}

			break;

		case FLOAT_VALUE:
			if (randomizer.nextDouble() <= random_mutation_rate)
				fvalue = (randomizer.nextFloat() * (fmax - fmin)) + fmin;
			else {
				f = fvalue;

				if (randomizer.nextBoolean()) {
					f += fdelta;

					if (f > fmax)
						f = fmax;
				}
				else {
					f -= fdelta;

					if (f < fmin)
						f = fmin;
				}

				fvalue = f;
			}

			break;

		case DOUBLE_VALUE:
			if (randomizer.nextDouble() <= random_mutation_rate)
				dvalue = (randomizer.nextDouble() * (dmax - dmin)) + dmin;
			else {
				d = dvalue;

				if (randomizer.nextBoolean()) {
					d += ddelta;

					if (d > dmax)
						d = dmax;
				}
				else {
					d -= ddelta;

					if (d < dmin)
						d = dmin;
				}

				dvalue = d;
			}

			break;
		}
	}


	// Copy gene value.
	void CopyValue(Gene from) {
		switch (type) {
		case INTEGER_VALUE:
			ivalue = from.ivalue;
			break;

		case FLOAT_VALUE:
			fvalue = from.fvalue;
			break;

		case DOUBLE_VALUE:
			dvalue = from.dvalue;
			break;
		}
	}


	// Load value.
	void LoadValue(DataInputStream reader) throws IOException {
		int itype = Utility.loadInt(reader);

		switch (itype) {
		case 0:
			ivalue = Utility.loadInt(reader);
			break;

		case 1:
			fvalue = Utility.loadFloat(reader);
			break;

		case 2:
			dvalue = Utility.loadDouble(reader);
			break;
		}
	}


	// Save value.
	void SaveValue(PrintWriter writer) throws IOException {
		switch (type) {
		case INTEGER_VALUE:
			Utility.saveInt(writer, 0);
			Utility.saveInt(writer, ivalue);
			break;

		case FLOAT_VALUE:
			Utility.saveInt(writer, 1);
			Utility.saveFloat(writer, fvalue);
			break;

		case DOUBLE_VALUE:
			Utility.saveInt(writer, 2);
			Utility.saveDouble(writer, dvalue);
			break;
		}

		writer.flush();
	}


	// Print gene.
	void Print() {
		switch (type) {
		case INTEGER_VALUE:
			System.out.println(name + "=" + ivalue);
			break;

		case FLOAT_VALUE:
			System.out.println(name + "=" + fvalue);
			break;

		case DOUBLE_VALUE:
			System.out.println(name + "=" + dvalue);
			break;
		}
	}
}
