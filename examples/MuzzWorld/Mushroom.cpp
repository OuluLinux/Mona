// For conditions of distribution and use, see copyright notice in muzz.hpp

// Mushroom.

#include "Mushroom.h"

// Sizes.
double Mushroom::width  = 0.01f;
double Mushroom::height = 0.01f;

// Constructor.
Mushroom::Mushroom(const Color& color, BlockTerrain* terrain) : BaseObject() {
	int       i;
	const int maxtries = 1000;
	m_alive      = true;
	m_color   = color;

	// Place the mushroom on the terrain.
	for (i = 0; i < maxtries; i++) {
		Place(Random());

		if (m_terrain->blocks[(int)(m_place_position[0] / m_terrain->block_size)]
			[(int)(m_place_position[2] / m_terrain->block_size)].type == BlockTerrain::Block::PLATFORM)
			break;
	}

	if (i == maxtries) {
		fprintf(stderr, "Cannot place mushroom on terrain\n");
		exit(1);
	}

	// Build drawing display.
	/*display = glGenLists(1);
	glNewList(display, GL_COMPILE);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glDisable(GL_TEXTURE_2D);
	glColor3f(m_color[0], m_color[1], m_color[2]);
	stem = gluNewQuadric();
	gluQuadricDrawStyle(stem, GLU_FILL);
	gluQuadricNormals(stem, GLU_SMOOTH);
	glPushMatrix();
	glTranslatef(0.0f, height * 0.75f, 0.0f);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	gluCylinder(stem, width * 0.5f, width * 0.5f, height * 0.75f, 20, 10);
	glPopMatrix();
	cap = gluNewQuadric();
	gluQuadricDrawStyle(cap, GLU_FILL);
	gluQuadricNormals(cap, GLU_SMOOTH);
	glPushMatrix();
	glTranslatef(0.0f, height * 0.9f, 0.0f);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	gluCylinder(cap, 0.0f, width, height * 0.25f, 20, 5);
	glPopMatrix();
	glColor3f(0.5f, 0.5f, 0.5f);
	capBorder = gluNewQuadric();
	gluQuadricDrawStyle(capBorder, GLU_FILL);
	gluQuadricNormals(capBorder, GLU_SMOOTH);
	glPushMatrix();
	glTranslatef(0.0f, height * 0.7f, 0.0f);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	gluCylinder(capBorder, width, width, height * 0.1f, 20, 2);
	glPopMatrix();
	glColor3f(1.0f, 1.0f, 1.0f);
	glPopMatrix();
	glEndList();*/
}


// Destructor.
Mushroom::~Mushroom() {
	/*gluDeleteQuadric(stem);
	gluDeleteQuadric(cap);
	gluDeleteQuadric(capBorder);
	glDeleteLists(display, 1);*/
}


// Get color.
void Mushroom::GetColor(Color& color) {
	color = m_color;
}


// Set color.
void Mushroom::SetColor(const Color& color) {
	m_color = color;
}


// Random mushroom placement on terrain.
void Mushroom::Place(int placement_seed) {
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
void Mushroom::Place(int x, int y) {
	ASSERT(x >= 0 && x < m_terrain->width);
	ASSERT(y >= 0 && y < m_terrain->height);
	double fx = ((double)x * m_terrain->block_size) + (m_terrain->block_size * 0.5f);
	double fy = ((double)y * m_terrain->block_size) + (m_terrain->block_size * 0.5f);
	Place(fx, fy);
}


// General placement.
void Mushroom::Place(double x, double y) {
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
void Mushroom::Place() {
	clearSpacial();
	SetPosition(m_place_position);
}


// Draw the mushroom.
void Mushroom::Draw() {
	/*glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	// Transform.
	glTranslatef(m_spacial->getX(), m_spacial->getY(), m_spacial->getZ());
	glMultMatrixf(&m_spacial->m_rotmatrix[0][0]);
	double scale = m_spacial->getScale();
	glScalef(scale, scale, scale);
	// Draw.
	glDisable(GL_TEXTURE_2D);
	glCallList(display);
	glPopMatrix();*/
}


// Load mushroom.
void Mushroom::Load(String filename) {
	Stream& fp;

	if ((fp = FOPEN_READ(filename)) == NULL) {
		fprintf(stderr, "Cannot load mushroom from file %s\n", filename);
		exit(1);
	}

	Load(fp);
	FCLOSE(fp);
}


void Mushroom::Serialize(Stream& fp) {
	fp % m_alive;
	BaseObject::Serialize(fp);
}


// Save mushroom.
void Mushroom::Store(String filename) {
	Stream& fp;

	if ((fp = FOPEN_WRITE(filename)) == NULL) {
		fprintf(stderr, "Cannot save mushroom to file %s\n", filename);
		exit(1);
	}

	Store(fp);
	FCLOSE(fp);
}


void Mushroom::Store(Stream& fp) {
	FWRITE_BOOL(&m_alive, fp);
	((BaseObject*)this)->Store(fp);
}
