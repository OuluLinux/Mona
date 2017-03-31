// For conditions of distribution and use, see copyright notice in muzz.hpp

// Water pool.

#include "Pool.h"

// Sizes.
double Pool::width  = 0.025f;
double Pool::height = 0.001f;

// Constructor.
Pool::Pool(const Color& color, BlockTerrain& terrain) : BaseObject() {
	int       i;
	const int maxtries = 1000;
	m_color      = color;
	m_terrain    = terrain;

	// Place the pool on the terrain.
	for (i = 0; i < maxtries; i++) {
		Place(m_randomizer->RAND());

		if (m_terrain->blocks[(int)(m_place_position[0] / m_terrain->block_size)]
			[(int)(m_place_position[2] / m_terrain->block_size)].type == BlockTerrain::Block::PLATFORM)
			break;
	}

	if (i == maxtries) {
		fprintf(stderr, "Cannot place pool on terrain\n");
		exit(1);
	}

	// Build drawing display.
	display = glGenLists(1);
	glNewList(display, GL_COMPILE);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glDisable(GL_TEXTURE_2D);
	glColor3f(m_color[0], m_color[1], m_color[2]);
	poolBase = gluNewQuadric();
	gluQuadricDrawStyle(poolBase, GLU_FILL);
	gluQuadricNormals(poolBase, GLU_SMOOTH);
	glPushMatrix();
	glTranslatef(0.0f, height, 0.0f);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	gluCylinder(poolBase, width, width, height, 20, 2);
	glPopMatrix();
	poolSurface = gluNewQuadric();
	gluQuadricDrawStyle(poolSurface, GLU_FILL);
	gluQuadricNormals(poolSurface, GLU_SMOOTH);
	glPushMatrix();
	glTranslatef(0.0f, height, 0.0f);
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	gluDisk(poolSurface, 0.0f, width, 20, 5);
	glPopMatrix();
	glColor3f(1.0f, 1.0f, 1.0f);
	glPopMatrix();
	glEndList();
}


// Destructor.
Pool::~Pool() {
	gluDeleteQuadric(poolBase);
	gluDeleteQuadric(poolSurface);
	glDeleteLists(display, 1);
}


// Get color.
void Pool::GetColor(Color& color) {
	color[0] = m_color[0];
	color[1] = m_color[1];
	color[2] = m_color[2];
}


// Set color.
void Pool::SetColor(const Color& color) {
	m_color[0] = color[0];
	m_color[1] = color[1];
	m_color[2] = color[2];
}


// Random pool placement on terrain.
void Pool::Place(int placement_seed) {
	int x, y;
	m_randomizer->RAND_PUSH();
	m_randomizer->SRAND(placement_seed);
	// Pick a random block.
	x = m_randomizer->RAND_CHOICE(m_terrain->width);
	y = m_randomizer->RAND_CHOICE(m_terrain->height);
	// Place.
	Place(x, y);
	m_randomizer->RAND_POP();
}


// Place in block terrain units.
void Pool::Place(int x, int y) {
	ASSERT(x >= 0 && x < m_terrain->width);
	ASSERT(y >= 0 && y < m_terrain->height);
	double fx = ((double)x * m_terrain->block_size) + (m_terrain->block_size * 0.5f);
	double fy = ((double)y * m_terrain->block_size) + (m_terrain->block_size * 0.5f);
	Place(fx, fy);
}


// General placement.
void Pool::Place(double x, double y) {
	double p[3];
	Vector  n;
	p[0] = x;
	p[2] = y;
	m_terrain->GetGeometry(p[0], p[2], p[1], n);
	m_place_position[0] = p[0];
	m_place_position[1] = p[1];
	m_place_position[2] = p[2];
	Place();
}


// Default placement.
void Pool::Place() {
	clearSpacial();
	SetPosition(m_place_position);
}


// Draw the pool.
void Pool::Draw() {
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


// Load pool.
void Pool::Load(String filename) {
	FILE* fp;

	if ((fp = FOPEN_READ(filename)) == NULL) {
		fprintf(stderr, "Cannot load pool from file %s\n", filename);
		exit(1);
	}

	Load(fp);
	FCLOSE(fp);
}


void Pool::Load(FILE* fp) {
	((BaseObject*)this)->Load(fp);
}


// Save pool.
void Pool::Store(String filename) {
	FILE* fp;

	if ((fp = FOPEN_WRITE(filename)) == NULL) {
		fprintf(stderr, "Cannot save pool to file %s\n", filename);
		exit(1);
	}

	Store(fp);
	FCLOSE(fp);
}


void Pool::Store(FILE* fp) {
	((BaseObject*)this)->Store(fp);
}
