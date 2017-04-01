#if 0
// Random numbers.

#ifndef __RANDOM__
#define __RANDOM__

#include "Common.h"

// Random value.
typedef unsigned long   RANDOM;

// Probability type.
typedef double          double;

// Period parameters.
#define RAND_N             624
#define RAND_M             397
/* constant vector a */
#define RAND_MATRIX_A      0x9908b0dfUL
/* most significant w-r bits */
#define RAND_UPPER_MASK    0x80000000UL
/* least significant r bits */
#define RAND_LOWER_MASK    0x7fffffffUL

// Random numbers.
class Random {
public:

	// The array for the state vector.
	// mti==RAND_N+1 means mt[RAND_N] is not initialized.
	int mt[RAND_N];
	int    mti;

	// Save/restore random state on stack.
	stack<int*> smt;
	stack<int>      smti;

	// Constructors.
	Random() {
		mti = RAND_N + 1;
	}


	Random(int seed) {
		mti = RAND_N + 1;
		SRAND(seed);
	}


	// Destructor.
	~Random() {
		RAND_CLEAR();
	}


	// Seed random numbers.
	void SRAND(int seed);

	// Get random number
	int RAND(); // genrand_int32

	// Random probability >= 0.0 && <= 1.0
	double RAND_PROB();

	// Random within interval >= min && <= max
	double RAND_INTERVAL(double min, double max);

	// Random chance for given probability
	bool RAND_CHANCE(double p);

	// Random choice of 0 to n-1.
	int RAND_CHOICE(int n);

	// Random boolean.
	bool RAND_BOOL();

	// Load random state
	void RAND_LOAD(Stream& fp);

	// Save random state
	void RAND_SAVE(Stream& fp);

	// Save random state
	void RAND_PUSH();

	// Restore random state
	void RAND_POP();

	// Clear random stack
	void RAND_CLEAR();

	// Clone random state
	void RAND_CLONE(Random& random);

private:

	/* initializes mt[N] with a seed */
	void init_genrand(int s);

	/* initialize by an array with array-length */
	/* init_key is the array for initializing keys */
	/* key_length is its length */
	/* slight change for C++, 2004/2/26 */
	void init_by_array(int init_key[], int key_length);

	/* generates a random number on [0,0xffffffff]-interval */
	int genrand_int32(void);

	/* generates a random number on [0,0x7fffffff]-interval */
	long genrand_int31(void);

	/* generates a random number on [0,1]-real-interval */
	double genrand_real1(void);

	/* generates a random number on [0,1)-real-interval */
	double genrand_real2(void);

	/* generates a random number on (0,1)-real-interval */
	double genrand_real3(void);

	/* generates a random number on [0,1) with 53-bit resolution*/
	double genrand_res53(void);

	/* load state from file pointer */
	void load_genrand(Stream& fp);

	/* save state to file pointer */
	void save_genrand(Stream& fp);

	// Save random state.
	void push_genrand();

	// Restore random state.
	void pop_genrand();
};

// Return random probability for given random object.
extern "C" double external_rand(void* random);
#endif
#endif
