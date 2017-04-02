#ifndef _Mona_ValueSet_h_
#define _Mona_ValueSet_h_

#include "Common.h"

class ValueSet {
public:

	// Constructors.
	ValueSet() {}

	ValueSet(int size) {
		values.SetCount(size);
	}


	// Destructor.
	~ValueSet() {
		Clear();
	}


	// Clear
	inline void Clear() {
		values.Clear();
	}


	// Get size.
	inline int GetCount() {
		return ((int)values.GetCount());
	}


	// Allocate
	inline void Reserve(int size) {
		Clear();
		values.SetCount(size);
	}


	// Zero
	inline void Zero() {
		for (int i = 0; i < (int)values.GetCount(); i++)
			values[i] = 0.0;
	}


	// Get a specified value.
	inline double Get(int index) {
		ASSERT(index >= 0 && index < (int)values.GetCount());
		return (values[index]);
	}


	// Get sum of values.
	inline double GetSum() {
		double d;
		d = 0.0;

		for (int i = 0; i < (int)values.GetCount(); i++)
			d += values[i];

		return d;
	}


	// Set a scalar value.
	inline void Set(int index, double value) {
		ASSERT(index >= 0 && index < (int)values.GetCount());
		values[index] = value;
	}


	// Add a scalar value.
	inline void Add(int index, double value) {
		ASSERT(index >= 0 && index < (int)values.GetCount());
		values[index] += value;
	}


	// Subtract a scalar value.
	inline void Subtract(int index, double value) {
		ASSERT(index >= 0 && index < (int)values.GetCount());
		values[index] -= value;
	}


	// Multiply by a scalar value.
	inline void Multiply(int index, double value) {
		ASSERT(index >= 0 && index < (int)values.GetCount());
		values[index] *= value;
	}


	// Divide by a scalar value.
	inline void Divide(int index, double value) {
		ASSERT(index >= 0 && index < (int)values.GetCount());
		ASSERT(value != 0.0);
		values[index] /= value;
	}


	// Add a scalar value to all.
	inline void Add(double value) {
		for (int i = 0; i < (int)values.GetCount(); i++)
			values[i] += value;
	}


	// Subtract a scalar value from all.
	inline void Subtract(double value) {
		for (int i = 0; i < (int)values.GetCount(); i++)
			values[i] -= value;
	}


	// Multiply all by a scalar value.
	inline void Multiply(double value) {
		for (int i = 0; i < (int)values.GetCount(); i++)
			values[i] *= value;
	}


	// Divide all by a scalar value.
	inline void Divide(double value) {
		ASSERT(value != 0.0);

		for (int i = 0; i < (int)values.GetCount(); i++)
			values[i] /= value;
	}


	// Load given set of values into this.
	inline void Load(class ValueSet& loadSet) {
		if (loadSet.GetCount() != (int)values.GetCount())
			values.SetCount(loadSet.GetCount());

		for (int i = 0; i < (int)values.GetCount(); i++)
			values[i] = loadSet.values[i];
	}


	// Vector3f addition.
	inline void Add(class ValueSet& addSet) {
		ASSERT(GetCount() == addSet.GetCount());

		for (int i = 0; i < (int)values.GetCount(); i++)
			values[i] += addSet.values[i];
	}


	// Vector3f subtraction.
	inline void Subtract(class ValueSet& subSet) {
		ASSERT(GetCount() == subSet.GetCount());

		for (int i = 0; i < (int)values.GetCount(); i++)
			values[i] -= subSet.values[i];
	}


	// Load.
	inline void Serialize(Stream& fp) {
		fp % values;
	}


	// Print.
	/*  inline void Print(FILE *out = stdout)
	    {
	    for (int i = 0; i < (int)values.GetCount(); i++)
	    {
	      fprintf(out, "%f ", values[i]);
	    }
	    fprintf(out, "\n");
	    }*/


	Vector<double> values;                         // Value set
};

typedef class ValueSet   VALUE_SET;

#endif
