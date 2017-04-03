
// A muzz is a simple robotic creature controlled by a mona neural network.

#ifndef MUZZ_HPP
#define MUZZ_HPP

#include <MonaCtrl/MonaCtrl.h>
#include "BlockTerrain.h"
#include "Mushroom.h"
#include "Pool.h"
#include <Mona/Mona.h>

class Muzz : public BaseObject {
public:

	// Identifier.
	int id;

	// Identifier dispenser.
	static int id_dispenser;

	// Muzz world sensors.
	enum {
		FORWARD_SENSOR = 0,
		RIGHT_SENSOR  = 1,
		LEFT_SENSOR   = 2,
		TERRAIN_SENSOR = 3,
		OBJECT_SENSOR = 4,
		NUM_SENSORS   = 5
	};
	enum {
		OPEN  = 0,
		CLOSED = 1
	};
	enum {
		PLATFORM = 0,
		WALL     = 1,
		DROP     = 2,
		RAMP_UP  = 3,
		RAMP_DOWN = 4
	};
	enum {
		MUSHROOM = 0,
		POOL     = 1,
		MUZZ     = 2,
		EMPTY    = 3,
		MIN_OTHER = (int)'A',
		MAX_OTHER = (int)'['
	};

	// Brain sensors.
	enum {
		NUM_BRAIN_SENSORS      = 11,
		NUM_BRAIN_SENSOR_MODE_1 = 3,
		NUM_BRAIN_SENSOR_MODE_2 = 8
	};
	enum {
		// Binary encoding of terrain types:
		TERRAIN_SENSOR_0 = 3,
		TERRAIN_SENSOR_1 = 4,
		TERRAIN_SENSOR_2 = 5,

		// Binary encoding of object values:
		OBJECT_SENSOR_0 = 6,
		OBJECT_SENSOR_1 = 7,
		OBJECT_SENSOR_2 = 8,
		OBJECT_SENSOR_3 = 9,
		OBJECT_SENSOR_4 = 10
	};

	// Sensor modes.
	enum {
		SENSOR_MODE_0 = 0,
		SENSOR_MODE_1 = 1,
		SENSOR_MODE_2 = 2
	};

	// Muzz responses.
	enum {
		WAIT          = 0,
		FORWARD       = 1,
		RIGHT         = 2,
		LEFT          = 3,
		EAT           = 4,
		DRINK         = 5,
		NUM_RESPONSES = 6
	};

	// Muzz needs.
	enum {
		FOOD     = 0,
		WATER    = 1,
		NUM_NEEDS = 2
	};

	static NEED       INIT_HUNGER;
	static NEED       INIT_THIRST;
	static const NEED EAT_GOAL_VALUE;
	static const NEED DRINK_GOAL_VALUE;
	static const double base_size;
	static const double HOVER_height;
	static const double max_height_change;
	static const double max_angle_adjustment;

	Muzz(const Color& color, BlockTerrain& terrain, int placement_seed);
	Muzz(BlockTerrain& terrain);
	~Muzz();
	static void InitBrain(Mona& brain);
	static void SetBoolBits(Vector<bool>& bits, int begin, int length, int value);
	void GetColor(Color& color);
	void SetColor(const Color& color);
	BlockTerrain& GetTerrain();
	void Place(int placement_seed);
	void Place(int x, int y, BlockTerrain::Block::DIRECTION direction);
	void Place(double x, double y, double direction);
	void Place();

	inline double GetPlaceX() {
		return m_place_position[0];
	}
	inline double GetPlaceY() {
		return m_place_position[2];
	}
	inline double GetPlaceDirection() {
		return m_place_direction;
	}

	void Forward(double step);
	void Backward(double step);
	void Right(double angle);
	void Left(double angle);

	enum {MOVE_FORWARD, TURN_RIGHT, TURN_LEFT};

	bool MoveOverTerrain(int type, double amount);
	void Draw();
	void Highlight();

	double GetNeed(int need) {
		return brain.GetNeed(need);
	}
	void SetNeed(int need, double value) {
		brain.SetNeed(need, value);
	}
	void Reset();
	void ReSetNeed(int need);
	void ClearNeed(int need);
	int Cycle(int* sensors, int cycleNum = -1);

	void Load(String filename);
	void Load(Stream& s);
	void Store(String filename);
	void Store(Stream& s);
	void PrintBrain(Stream& s);
	void PrintResponsePotentials(Stream& s);


	bool has_food;
	bool has_water;
	int  has_foodCycle;
	int  has_waterCycle;

	Mona brain;

	Vector<SENSOR> brain_sensors;

	// Properties.
	Color         m_color;
	double        m_place_position[3];
	double        m_place_direction;
	BlockTerrain* m_terrain;

	// OpenGL display for drawing.
	/*	 GLuint display;

		// "Turret" components.
		GLUquadricObj* turretCylinder, *turretTopInner, *turretTopOuter,
					   turretEye, *turretRightEar, *turretLeftEar;*/
};
#endif
