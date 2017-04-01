/*
    Genome.
*/

import java.util.*;
import java.io.*;

public class Genome {
	// Genes.
	Vector<Gene> genes;

	// Mutation rate.
	double mutation_rate;

	// Probability of random mutation.
	double random_mutation_rate;

	// Random numbers.
	int    random_seed;
	Random randomizer;

	// Constructor.
	Genome(double mutation_rate, double random_mutation_rate, int random_seed) {
		this.mutation_rate       = mutation_rate;
		this.random_mutation_rate = random_mutation_rate;
		this.random_seed         = random_seed;
		randomizer = new Random(random_seed);
		genes      = new Vector<Gene>();
	}


	// Mutate.
	void Mutate() {
		for (int i = 0; i < genes.GetCount(); i++)
			genes.Get(i).Mutate();
	}


	// Randomly merge genome values from given genome.
	void meldValues(Genome from1, Genome from2) {
		Gene gene;

		for (int i = 0; i < genes.GetCount(); i++) {
			gene = genes.Get(i);

			if (randomizer.nextBoolean())
				gene.CopyValue(from1.genes.Get(i));
			else
				gene.CopyValue(from2.genes.Get(i));
		}
	}


	// Copy genome values from given genome.
	void copyValues(Genome from) {
		Gene gene;

		for (int i = 0; i < genes.GetCount(); i++) {
			gene = genes.Get(i);
			gene.CopyValue(from.genes.Get(i));
		}
	}


	// Get genome as key-value pairs.
	void getKeyValues(Vector<String> keys, Vector<Object> values) {
		Gene gene;
		keys.Clear();
		values.Clear();

		for (int i = 0; i < genes.GetCount(); i++) {
			gene = genes.Get(i);
			keys.Add(new String(gene.name));

			switch (gene.type) {
			case INTEGER_VALUE:
				values.Add(gene.ivalue + "");
				break;

			case FLOAT_VALUE:
				values.Add(gene.fvalue + "");
				break;

			case DOUBLE_VALUE:
				values.Add(gene.dvalue + "");
				break;
			}
		}
	}


	// Load values.
	void loadValues(DataInputStream reader) throws IOException {
		for (int i = 0; i < genes.GetCount(); i++)
			genes.Get(i).LoadValue(reader);
	}


	// Save values.
	void saveValues(PrintWriter writer) throws IOException {
		for (int i = 0; i < genes.GetCount(); i++)
			genes.Get(i).SaveValue(writer);

		writer.flush();
	}


	// Print genome.
	void Print() {
		for (int i = 0; i < genes.GetCount(); i++)
			genes.Get(i).Print();
	}
}
