// For conditions of distribution and use, see copyright notice in muzz.hpp

// A muzz is a simple robotic creature that uses a mona neural network
// for a brain.

#include "Muzz.h"

// Identifier dispenser.
int Muzz::id_dispenser = 0;

// Muzz needs.
Mona::NEED Muzz::      INIT_HUNGER      = 1.0;
Mona::NEED Muzz::      INIT_THIRST      = 1.0;
const Mona::NEED Muzz::EAT_GOAL_VALUE   = 1.0;
const Mona::NEED Muzz::DRINK_GOAL_VALUE = 1.0;

// Base size.
const double Muzz::base_size = 0.025f;

// Hover height.
const double Muzz::HOVER_height = 0.0f;

// Movement constraint parameters.
const double Muzz::max_height_change    = 0.75f;
const double Muzz::max_angle_adjustment = 30.0f;

// Constructor.
Muzz::Muzz(const Color& color, BlockTerrain* terrain, int placement_seed) : BaseObject() {
	double       d1, d2, d3, a;
	Vector3f        vertices[8];
	const double flatten = 0.5f;
	const double border  = base_size * 0.025f;
	id = id_dispenser;
	id_dispenser++;
	m_color   = color;
	m_terrain    = terrain;
	m_randomizer = randomizer;
	// Place muzz on the terrain.
	Place(placement_seed);
	// Build display.
	display = glGenLists(1);
	glNewList(display, GL_COMPILE);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glDisable(GL_TEXTURE_2D);
	// Draw the base.
	glTranslatef(base_size * -0.5f, 0.0f, base_size * -0.5f);
	glColor3f(1.0f, 1.0f, 1.0f);
	DrawBlock(0.0f, base_size, 0.0f, base_size * flatten, 0.0f, base_size);
	glColor3f(0.0f, 0.0f, 0.0f);
	// Draw the front 'v' pattern.
	DrawBlock(0.0f, base_size, 0.0f, border, base_size, base_size + border);
	DrawBlock(0.0f, base_size, (base_size * flatten) - border, base_size * flatten, base_size, base_size + border);
	DrawBlock(-border, 0.0f, 0.0f, base_size * flatten, base_size, base_size + border);
	DrawBlock(base_size, base_size + border, 0.0f, base_size * flatten, base_size, base_size + border);
	glPushMatrix();
	glTranslatef(0.0f, (base_size * flatten) - border, 0.0f);
	d1 = (base_size * flatten) - border;
	d2 = base_size * 0.5f;
	d3 = sqrt((d1 * d1) + (d2 * d2));
	a  = 90.0f - RadiansToDegrees(asin(d2 / d3));
	glRotatef(-a, 0.0f, 0.0f, 1.0f);
	DrawBlock(0.0f, d3, 0.0f, border, base_size, base_size + border);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(base_size, (base_size * flatten) - border, 0.0f);
	glRotatef(a, 0.0f, 0.0f, 1.0f);
	DrawBlock(-d3, 0.0f, 0.0f, border, base_size, base_size + border);
	glPopMatrix();
	// Draw the back inverted 'v' pattern.
	DrawBlock(0.0f, base_size, 0.0f, border, -border, 0.0f);
	DrawBlock(0.0f, base_size, (base_size * flatten) - border, base_size * flatten, -border, 0.0f);
	DrawBlock(-border, 0.0f, 0.0f, base_size * flatten, -border, 0.0f);
	DrawBlock(base_size, base_size + border, 0.0f, base_size * flatten, -border, 0.0f);
	glPushMatrix();
	glTranslatef(base_size * 0.5f, (base_size * flatten) - border, 0.0f);
	glRotatef(-a, 0.0f, 0.0f, 1.0f);
	DrawBlock(0.0f, d3, 0.0f, border, -border, 0.0f);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(base_size * 0.5f, (base_size * flatten) - border, 0.0f);
	glRotatef(a, 0.0f, 0.0f, 1.0f);
	DrawBlock(-d3, 0.0f, 0.0f, border, -border, 0.0f);
	glPopMatrix();
	// Draw the left diagnonal pattern.
	DrawBlock(-border, 0.0f, 0.0f, border, 0.0f, base_size);
	DrawBlock(-border, 0.0f, (base_size * flatten) - border, base_size * flatten, 0.0f, base_size);
	glPushMatrix();
	d1 = (base_size * flatten) - border;
	d2 = base_size;
	d3 = sqrt((d1 * d1) + (d2 * d2));
	a  = RadiansToDegrees(asin(d1 / d3));
	glRotatef(-a, 1.0f, 0.0f, 0.0f);
	DrawBlock(-border, 0.0f, 0.0f, border, 0.0f, d3);
	glPopMatrix();
	// Draw the right diagnonal pattern.
	DrawBlock(base_size, base_size + border, 0.0f, border, 0.0f, base_size);
	DrawBlock(base_size, base_size + border, (base_size * flatten) - border, base_size * flatten, 0.0f, base_size);
	glPushMatrix();
	d1 = (base_size * flatten) - border;
	d2 = base_size;
	d3 = sqrt((d1 * d1) + (d2 * d2));
	a  = RadiansToDegrees(asin(d1 / d3));
	glRotatef(-a, 1.0f, 0.0f, 0.0f);
	DrawBlock(base_size, base_size + border, 0.0f, border, 0.0f, d3);
	glPopMatrix();
	// Draw the "turret".
	glColor3f(m_color[0], m_color[1], m_color[2]);
	turretCylinder = gluNewQuadric();
	gluQuadricDrawStyle(turretCylinder, GLU_FILL);
	gluQuadricNormals(turretCylinder, GLU_SMOOTH);
	glPushMatrix();
	glTranslatef(base_size * 0.5f, base_size * 0.75f, base_size * 0.5f);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	gluCylinder(turretCylinder, base_size * 0.2f, base_size * 0.2f, base_size * 0.25f, 25, 5);
	glPopMatrix();
	turretTopInner = gluNewQuadric();
	gluQuadricDrawStyle(turretTopInner, GLU_FILL);
	gluQuadricNormals(turretTopInner, GLU_SMOOTH);
	glPushMatrix();
	glTranslatef(base_size * 0.5f, base_size * 0.75f, base_size * 0.5f);
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	gluDisk(turretTopInner, 0.0f, base_size * 0.15f, 25, 5);
	glPopMatrix();
	glColor3f(0.5f, 0.5f, 0.5f);
	turretTopOuter = gluNewQuadric();
	gluQuadricDrawStyle(turretTopOuter, GLU_FILL);
	gluQuadricNormals(turretTopOuter, GLU_SMOOTH);
	glPushMatrix();
	glTranslatef(base_size * 0.5f, base_size * 0.75f, base_size * 0.5f);
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	gluDisk(turretTopOuter, base_size * 0.15f, base_size * 0.2f, 25, 5);
	glPopMatrix();
	turretEye = gluNewQuadric();
	gluQuadricDrawStyle(turretEye, GLU_FILL);
	gluQuadricNormals(turretEye, GLU_SMOOTH);
	glPushMatrix();
	glTranslatef(base_size * 0.5f, base_size * 0.63f, base_size * 0.65f);
	gluSphere(turretEye, base_size * 0.1f, 10, 10);
	glPopMatrix();
	turretRightEar = gluNewQuadric();
	gluQuadricDrawStyle(turretRightEar, GLU_FILL);
	gluQuadricNormals(turretRightEar, GLU_SMOOTH);
	glPushMatrix();
	glTranslatef(base_size * 0.3f, base_size * 0.63f, base_size * 0.5f);
	gluSphere(turretRightEar, base_size * 0.05f, 10, 10);
	glPopMatrix();
	turretLeftEar = gluNewQuadric();
	gluQuadricDrawStyle(turretLeftEar, GLU_FILL);
	gluQuadricNormals(turretLeftEar, GLU_SMOOTH);
	glPushMatrix();
	glTranslatef(base_size * 0.7f, base_size * 0.63f, base_size * 0.5f);
	gluSphere(turretLeftEar, base_size * 0.05f, 10, 10);
	glPopMatrix();
	glColor3f(1.0f, 1.0f, 1.0f);
	glPopMatrix();
	glEndList();
	// Initialize brain.
	brain = new Mona();
	ASSERT(brain != NULL);
	InitBrain(brain);
	// Initialize needs.
	SetNeed(FOOD, INIT_HUNGER);
	SetNeed(WATER, INIT_THIRST);
	// Allocate brain sensors.
	brain_sensors.SetCount(NUM_BRAIN_SENSORS);
	// Initialize foraging status.
	has_food      = has_water = false;
	has_foodCycle = has_waterCycle = -1;
}


// Default constructor.
Muzz::Muzz(BlockTerrai& terrain) : BaseObject() {
	int i;
	id = id_dispenser;
	id_dispenser++;

	for (i = 0; i < 3; i++)
		m_color[i] = 0.0f;

	for (i = 0; i < 3; i++)
		m_place_position[i] = 0.0f;

	m_place_direction = 0.0f;
	m_terrain        = &terrain;
	m_randomizer     = NULL;
	display          = (GLuint)(-1);
	turretCylinder   = NULL;
	turretTopInner   = NULL;
	turretTopOuter   = NULL;
	turretEye        = NULL;
	turretRightEar   = NULL;
	turretLeftEar    = NULL;
	brain            = NULL;
	has_food          = has_water = false;
	has_foodCycle     = has_waterCycle = -1;
}


// Destructor.
Muzz::~Muzz() {
	if (display != (GLuint)(-1))
		glDeleteLists(display, 1);

	if (turretCylinder != NULL)
		gluDeleteQuadric(turretCylinder);

	if (turretTopInner != NULL)
		gluDeleteQuadric(turretTopInner);

	if (turretTopOuter != NULL)
		gluDeleteQuadric(turretTopOuter);

	if (turretEye != NULL)
		gluDeleteQuadric(turretEye);

	if (turretRightEar != NULL)
		gluDeleteQuadric(turretRightEar);

	if (turretLeftEar != NULL)
		gluDeleteQuadric(turretLeftEar);

	if (brain != NULL)
		delete brain;
}


// Initialize muzz brain.
void Muzz::InitBrain(Mona* brain) {
	int i;
	Vector<bool>         sensorMask;
	Vector<bool>         bits;
	Vector<Mona::SENSOR> goalSensors;
	// Initialize network.
	brain->InitNet(NUM_BRAIN_SENSORS, NUM_RESPONSES, NUM_NEEDS,
				   brain->random_seed);
	sensorMask.Clear();

	for (i = 0; i < NUM_BRAIN_SENSORS; i++)
		sensorMask.Add(true);

	brain->AddSensorMode(sensorMask);
	sensorMask.Clear();

	for (i = 0; i < NUM_BRAIN_SENSORS; i++) {
		if (i < NUM_BRAIN_SENSOR_MODE_1)
			sensorMask.Add(true);
		else
			sensorMask.Add(false);
	}

	brain->AddSensorMode(sensorMask);
	sensorMask.Clear();

	for (i = 0; i < NUM_BRAIN_SENSORS; i++) {
		if (i < NUM_BRAIN_SENSOR_MODE_1)
			sensorMask.Add(false);
		else
			sensorMask.Add(true);
	}

	brain->AddSensorMode(sensorMask);
	// Add sensory goals.
	goalSensors.SetCount(NUM_BRAIN_SENSORS);

	for (i = 0; i < NUM_BRAIN_SENSORS; i++)
		bits.Add(false);

	SetBoolBits(bits, TERRAIN_SENSOR_0, 3, PLATFORM);
	SetBoolBits(bits, OBJECT_SENSOR_0, 5, MUSHROOM +
				(MAX_OTHER - MIN_OTHER));

	for (i = 0; i < NUM_BRAIN_SENSORS; i++) {
		if (bits[i])
			goalSensors[i] = 1.0f;
		else
			goalSensors[i] = 0.0f;
	}

	brain->homeostats[FOOD]->AddGoal(goalSensors, SENSOR_MODE_2,
									 EAT, EAT_GOAL_VALUE);
	SetBoolBits(bits, OBJECT_SENSOR_0, 5, POOL +
				(MAX_OTHER - MIN_OTHER));

	for (i = 0; i < NUM_BRAIN_SENSORS; i++) {
		if (bits[i])
			goalSensors[i] = 1.0f;
		else
			goalSensors[i] = 0.0f;
	}

	brain->homeostats[WATER]->AddGoal(goalSensors, SENSOR_MODE_2,
									  DRINK, DRINK_GOAL_VALUE);
}


// Set binary value in bools.
void Muzz::SetBoolBits(Vector<bool>& bits, int begin,
					   int length, int value) {
	for (int i = 0; i < length; i++) {
		if ((value % 2) == 1)
			bits[begin + i] = true;
		else
			bits[begin + i] = false;

		value = (unsigned int)value >> 1;
	}
}


// Get color.
void Muzz::GetColor(Color& color) {
	color[0] = m_color[0];
	color[1] = m_color[1];
	color[2] = m_color[2];
}


// Set color.
void Muzz::SetColor(const Color& color) {
	m_color[0] = color[0];
	m_color[1] = color[1];
	m_color[2] = color[2];
}


// Get terrain.
BlockTerrain* Muzz::GetTerrain() {
	return (m_terrain);
}


// Get randomizer.
Random* Muzz::Getrandomizer() {
	return (m_randomizer);
}


// Random muzz placement on terrain.
void Muzz::Place(int placement_seed) {
	int x, y;
	BlockTerrain::Block::DIRECTION direction;
	m_randomizer->RAND_PUSH();
	m_randomizer->SRAND(placement_seed);
	// Pick a random block.
	x = m_randomizer->RAND_CHOICE(m_terrain->width);
	y = m_randomizer->RAND_CHOICE(m_terrain->height);
	// Pick a random direction.
	direction = (BlockTerrain::Block::DIRECTION)m_randomizer->RAND_CHOICE(4);
	// Place.
	Place(x, y, direction);
	m_randomizer->RAND_POP();
}


// Place in block terrain units.
void Muzz::Place(int x, int y, BlockTerrain::Block::DIRECTION direction) {
	ASSERT(x >= 0 && x < m_terrain->width);
	ASSERT(y >= 0 && y < m_terrain->height);
	double fx = ((double)x * m_terrain->block_size) + (m_terrain->block_size * 0.5f);
	double fy = ((double)y * m_terrain->block_size) + (m_terrain->block_size * 0.5f);

	switch (direction) {
	case BlockTerrain::Block::NORTH:
		Place(fx, fy, 0.0f);
		break;

	case BlockTerrain::Block::EAST:
		Place(fx, fy, 90.0f);
		break;

	case BlockTerrain::Block::SOUTH:
		Place(fx, fy, 180.0f);
		break;

	case BlockTerrain::Block::WEST:
		Place(fx, fy, 270.0f);
		break;

	default:
		ASSERT(false);
	}
}


// General placement.
void Muzz::Place(double x, double y, double direction) {
	double p[3];
	Vector  n;
	p[0] = x;
	p[2] = y;
	m_terrain->GetGeometry(p[0], p[2], p[1], n);
	m_place_position[0] = p[0];
	m_place_position[1] = p[1];
	m_place_position[2] = p[2];
	m_place_direction   = direction;
	Place();
}


// Default placement.
void Muzz::Place() {
	clearSpacial();
	SetPosition(m_place_position);
	setYaw(m_place_direction);
	Forward(0.0f);
}


// Move forward.
void Muzz::Forward(double step) {
	MoveOverTerrain(MOVE_FORWARD, step);
}


// Move backward.
void Muzz::Backward(double step) {
	MoveOverTerrain(MOVE_FORWARD, -step);
}


// Turn right.
void Muzz::Right(double angle) {
	MoveOverTerrain(TURN_RIGHT, angle);
}


// Turn left.
void Muzz::Left(double angle) {
	MoveOverTerrain(TURN_LEFT, angle);
}


// Conform movement to terrain topology.
// Return false if move is invalid.
bool Muzz::MoveOverTerrain(int type, double amount) {
	int     i;
	double r, c0[3], c1[3], c2[3], c3[3], nc0[3], nc1[3], nc2[3], nc3[3];
	double oldpos[3], pos[3], oldfwd[3], fwd[3], up[3], normal[3], rotation[4];
	Vector  v1, v2, n;
	bool    ok;
	// Clear transforms for coordinate conversions.
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	// Make sure maximum height change is not exceeded for all 4 corners.
	r     = base_size * 0.5f;
	c0[0] = -r;
	c0[1] = 0.0f;
	c0[2] = r;
	localToWorld(c0, c0);
	c1[0] = r;
	c1[1] = 0.0f;
	c1[2] = r;
	localToWorld(c1, c1);
	c2[0] = r;
	c2[1] = 0.0f;
	c2[2] = -r;
	localToWorld(c2, c2);
	c3[0] = -r;
	c3[1] = 0.0f;
	c3[2] = -r;
	localToWorld(c3, c3);

	switch (type) {
	case MOVE_FORWARD:
		GetPosition(oldpos);
		getForward(fwd);
		cSpacial::normalize(fwd);

		for (i = 0; i < 3; i++)
			pos[i] = oldpos[i] + (fwd[i] * amount);

		SetPosition(pos);
		break;

	case TURN_LEFT:
		amount = -amount;

	case TURN_RIGHT:
		addYaw(amount);
		break;
	}

	do {
		ok = true;
		double max = max_height_change * m_terrain->block_size;
		nc0[0] = -r;
		nc0[1] = 0.0f;
		nc0[2] = r;
		localToWorld(nc0, nc0);
		m_terrain->GetGeometry(c0[0], c0[2], c0[1], n);
		m_terrain->GetGeometry(nc0[0], nc0[2], nc0[1], n);

		if (fabs(c0[1] - nc0[1]) > max) {
			ok = false;
			break;
		}

		nc1[0] = r;
		nc1[1] = 0.0f;
		nc1[2] = r;
		localToWorld(nc1, nc1);
		m_terrain->GetGeometry(c1[0], c1[2], c1[1], n);
		m_terrain->GetGeometry(nc1[0], nc1[2], nc1[1], n);

		if (fabs(c1[1] - nc1[1]) > max) {
			ok = false;
			break;
		}

		nc2[0] = r;
		nc2[1] = 0.0f;
		nc2[2] = -r;
		localToWorld(nc2, nc2);
		m_terrain->GetGeometry(c2[0], c2[2], c2[1], n);
		m_terrain->GetGeometry(nc2[0], nc2[2], nc2[1], n);

		if (fabs(c2[1] - nc2[1]) > max) {
			ok = false;
			break;
		}

		nc3[0] = -r;
		nc3[1] = 0.0f;
		nc3[2] = -r;
		localToWorld(nc3, nc3);
		m_terrain->GetGeometry(c3[0], c3[2], c3[1], n);
		m_terrain->GetGeometry(nc3[0], nc3[2], nc3[1], n);

		if (fabs(c3[1] - nc3[1]) > max) {
			ok = false;
			break;
		}
	}
	while (false);

	if (!ok) {
		switch (type) {
		case MOVE_FORWARD:
			SetPosition(oldpos);
			break;

		case TURN_LEFT:
		case TURN_RIGHT:
			addYaw(-amount);
			break;
		}

		return false;
	}

	// Set height to average height of corners.
	GetPosition(pos);
	pos[1] = (nc0[1] + nc1[1] + nc2[1] + nc3[1]) / 4.0f;
	SetPosition(pos);
	// Save original heading for later.
	getForward(oldfwd);
	// New up vector is normal to base plane.
	v1.x = nc1[0] - nc0[0];
	v1.y = nc1[1] - nc0[1];
	v1.z = nc1[2] - nc0[2];
	v2.x = nc2[0] - nc0[0];
	v2.y = nc2[1] - nc0[1];
	v2.z = nc2[2] - nc0[2];
	n    = v1 ^ v2;
	n.Normalize();
	normal[0] = n.x;
	normal[1] = n.y;
	normal[2] = n.z;
	// Smooth out rotation.
	getUp(up);
	getBillboard(normal, up, rotation);

	if ((fabs(rotation[0]) > tol) || (fabs(rotation[1]) > tol) || (fabs(rotation[2]) > tol)) {
		if (rotation[3] > max_angle_adjustment)
			rotation[3] = max_angle_adjustment;
		else if (rotation[3] < -max_angle_adjustment)
			rotation[3] = -max_angle_adjustment;

		MergeRotation(rotation[3], rotation);
	}

	// Set new height to "hover" above terrain.
	pos[1] += HOVER_height;
	SetPosition(pos);
	return true;
}

// Draw the muzz.
void Muzz::Draw() {
	ASSERT(display != (GLuint)(-1));
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	// Transform.
	glTranslatef(m_spacial->getX(), m_spacial->getY(), m_spacial->getZ());
	glMultMatrixf(&m_spacial->m_rotmatrix[0][0]);
	double scale = m_spacial->getScale();
	glScalef(scale, scale, scale);
	// Draw.
	glDisable(GL_TEXTURE_2D);
	glCallList(display);
	glPopMatrix();
}


// Highlight the muzz.
void Muzz::Highlight() {
	ASSERT(display != (GLuint)(-1));
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	// Transform.
	glTranslatef(m_spacial->getX(), m_spacial->getY(), m_spacial->getZ());
	glMultMatrixf(&m_spacial->m_rotmatrix[0][0]);
	double scale = m_spacial->getScale();
	glScalef(scale, scale, scale);
	// Draw enclosing semi-tranparent sphere.
	glDisable(GL_TEXTURE_2D);
	glColor4f(m_color[0], m_color[1], m_color[2], 0.5f);
	glEnable(GL_BLEND);
	glutSolidSphere(base_size, 20, 20);
	glDisable(GL_BLEND);
	glPopMatrix();
}


// Reset muzz.
void Muzz::Reset() {
	ASSERT(brain != NULL);
	brain->ClearWorkingMemory();
	SetNeed(FOOD, INIT_HUNGER);
	SetNeed(WATER, INIT_THIRST);
	// Reset foraging results.
	has_food      = has_water = false;
	has_foodCycle = has_waterCycle = -1;
	// Re-place.
	Place();
}


// Reset muzz need.
void Muzz::ReSetNeed(int need) {
	ASSERT(brain != NULL);

	if (need == FOOD) {
		SetNeed(FOOD, INIT_HUNGER);
		has_food      = false;
		has_foodCycle = -1;
	}
	else {
		SetNeed(WATER, INIT_THIRST);
		has_water      = false;
		has_waterCycle = -1;
	}
}


// Clear muzz need.
void Muzz::ClearNeed(int need) {
	ASSERT(brain != NULL);

	if (need == FOOD) {
		SetNeed(FOOD, 0.0);
		has_food = true;
	}
	else {
		SetNeed(WATER, 0.0);
		has_water = true;
	}
}


// Sensory-response cycle.
int Muzz::Cycle(int* sensors, int cycleNum) {
	int i, response;
	Vector<bool> bits;
	ASSERT(brain != NULL);
	// Set up brain sensors.
	bits.SetCount(NUM_BRAIN_SENSORS);

	if (sensors[FORWARD_SENSOR] == OPEN)
		bits[FORWARD_SENSOR] = true;
	else
		bits[FORWARD_SENSOR] = false;

	if (sensors[RIGHT_SENSOR] == OPEN)
		bits[RIGHT_SENSOR] = true;
	else
		bits[RIGHT_SENSOR] = false;

	if (sensors[LEFT_SENSOR] == OPEN)
		bits[LEFT_SENSOR] = true;
	else
		bits[LEFT_SENSOR] = false;

	SetBoolBits(bits, TERRAIN_SENSOR_0, 3, sensors[TERRAIN_SENSOR]);

	if (sensors[OBJECT_SENSOR] >= MIN_OTHER) {
		SetBoolBits(bits, OBJECT_SENSOR_0, 5,
					sensors[OBJECT_SENSOR] - MIN_OTHER);
	}
	else {
		SetBoolBits(bits, OBJECT_SENSOR_0, 5,
					sensors[OBJECT_SENSOR] + (MAX_OTHER - MIN_OTHER));
	}

	for (i = 0; i < NUM_BRAIN_SENSORS; i++) {
		if (bits[i])
			brain_sensors[i] = 1.0f;
		else
			brain_sensors[i] = 0.0f;
	}

	// Get response.
	response = (int)brain->Cycle(brain_sensors);

	// Note food and water consumption.
	if ((sensors[OBJECT_SENSOR] == MUSHROOM) && (response == EAT)) {
		if (!has_food) {
			has_food      = true;
			has_foodCycle = cycleNum;
		}
	}

	if ((sensors[OBJECT_SENSOR] == POOL) && (response == DRINK)) {
		if (!has_water) {
			has_water      = true;
			has_waterCycle = cycleNum;
		}
	}

	return (response);
}


// Print muzz brain in XML format.
void Muzz::PrintBrain(FILE* out) {
	ASSERT(brain != NULL);
	fprintf(out, "Muzz brain:\n");
	#ifdef MONA_TRACKING
	brain->Print((Mona::TRACKING_FLAGS)Mona::TRACK_DRIVE, out);
	#else
	brain->Print(out);
	#endif
}


// Print muzz response potentials.
void Muzz::printResponsePotentials(FILE* out) {
	ASSERT(brain != NULL);
	fprintf(out, "Muzz response potentials:\n");

	for (int i = 0; i < NUM_RESPONSES; i++) {
		switch (i) {
		case WAIT:
			fprintf(out, "WAIT = %f\n", brain->response_potentials[i]);
			break;

		case FORWARD:
			fprintf(out, "FORWARD = %f\n", brain->response_potentials[i]);
			break;

		case RIGHT:
			fprintf(out, "RIGHT = %f\n", brain->response_potentials[i]);
			break;

		case LEFT:
			fprintf(out, "LEFT = %f\n", brain->response_potentials[i]);
			break;

		case EAT:
			fprintf(out, "EAT = %f\n", brain->response_potentials[i]);
			break;

		case DRINK:
			fprintf(out, "DRINK = %f\n", brain->response_potentials[i]);
			break;
		}
	}
}


// Load muzz.
void Muzz::Load(String filename) {
	FILE* fp;

	if ((fp = FOPEN_READ(filename)) == NULL) {
		fprintf(stderr, "Cannot load muzz from file %s\n", filename);
		exit(1);
	}

	Load(fp);
	FCLOSE(fp);
}


void Muzz::Load(FILE* fp) {
	ASSERT(brain != NULL);
	((BaseObject*)this)->Load(fp);
	FREAD_INT(&id, fp);
	FREAD_BOOL(&has_food, fp);
	FREAD_BOOL(&has_water, fp);
	FREAD_INT(&has_foodCycle, fp);
	FREAD_INT(&has_waterCycle, fp);
	brain->Load(fp);
	FREAD_FLOAT(&m_color[0], fp);
	FREAD_FLOAT(&m_color[1], fp);
	FREAD_FLOAT(&m_color[2], fp);
	FREAD_FLOAT(&m_place_position[0], fp);
	FREAD_FLOAT(&m_place_position[1], fp);
	FREAD_FLOAT(&m_place_position[2], fp);
	FREAD_FLOAT(&m_place_direction, fp);
}


// Save muzz.
void Muzz::Store(String filename) {
	FILE* fp;

	if ((fp = FOPEN_WRITE(filename)) == NULL) {
		fprintf(stderr, "Cannot save muzz to file %s\n", filename);
		exit(1);
	}

	Store(fp);
	FCLOSE(fp);
}


void Muzz::Store(FILE* fp) {
	ASSERT(brain != NULL);
	((BaseObject*)this)->Store(fp);
	FWRITE_INT(&id, fp);
	FWRITE_BOOL(&has_food, fp);
	FWRITE_BOOL(&has_water, fp);
	FWRITE_INT(&has_foodCycle, fp);
	FWRITE_INT(&has_waterCycle, fp);
	brain->Store(fp);
	FWRITE_FLOAT(&m_color[0], fp);
	FWRITE_FLOAT(&m_color[1], fp);
	FWRITE_FLOAT(&m_color[2], fp);
	FWRITE_FLOAT(&m_place_position[0], fp);
	FWRITE_FLOAT(&m_place_position[1], fp);
	FWRITE_FLOAT(&m_place_position[2], fp);
	FWRITE_FLOAT(&m_place_direction, fp);
}
