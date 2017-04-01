#include "TopWindow.h"



ResponseSearch::ResponseSearch(int response, ResponseSearch* parent, Muzz* parentMuzz, int depth) {
	experience.response = response;
	this->parent        = parent;
	x          = z = dir = -1;
	forward    = backward = BlockTerrain::Block::NORTH;
	moveType   = Muzz::WAIT;
	moveAmount = 0.0;
	muzz       = new Muzz(*world->terrain);
	ASSERT(muzz != NULL);
	muzz->has_food       = parentMuzz->has_food;
	muzz->has_water      = parentMuzz->has_water;
	muzz->has_foodCycle  = parentMuzz->has_foodCycle;
	muzz->has_waterCycle = parentMuzz->has_waterCycle;
	delete muzz->m_spacial;
	muzz->m_spacial = parentMuzz->m_spacial->Clone();
	ASSERT(muzz->m_spacial != NULL);
	muzz->m_place_position[0] = parentMuzz->m_place_position[0];
	muzz->m_place_position[1] = parentMuzz->m_place_position[1];
	muzz->m_place_position[2] = parentMuzz->m_place_position[2];
	muzz->m_place_direction   = parentMuzz->m_place_direction;

	for (int i = 0; i < world->mushroom_count; i++)
		mushrooms.Add(false);

	this->depth = depth;
	goalDist    = -1;
}


// Destructor.
ResponseSearch::~ResponseSearch() {
	delete muzz;
}


// Set city-block distance to closest needed goal.
// Distance is zero when goals are achieved.
// Return true if needed goals are available.
bool ResponseSearch::SetGoalDist() {
	int       i, x, z, x2, z2, d;
	double   p[3], p2[3];
	bool      blocked;
	const int maxDist = 10000;
	goalDist = -1;

	if (muzz->has_food && muzz->has_water) {
		goalDist = 0;
		return true;
	}

	muzz->GetPosition(p);
	x = (int)(p[0] / world->terrain.block_size);
	z = (int)(p[2] / world->terrain.block_size);

	if (!muzz->has_food) {
		blocked = true;

		for (i = 0; i < world->mushroom_count; i++) {
			if (mushrooms[i]) {
				blocked = false;
				world->mushrooms[i].GetPosition(p2);
				x2 = (int)(p2[0] / world->terrain.block_size);
				z2 = (int)(p2[2] / world->terrain.block_size);
				d  = 0;

				if (x > x2)
					d += x - x2;
				else
					d += x2 - x;

				if (z > z2)
					d += z - z2;
				else
					d += z2 - z;

				if ((goalDist < 0) || (d < goalDist))
					goalDist = d;
			}
		}

		if (blocked)
			return false;
	}

	if (!muzz->has_water) {
		if (world->pool_count == 0)
			return false;

		for (i = 0; i < world->pool_count; i++) {
			world->pools[i].GetPosition(p2);
			x2 = (int)(p2[0] / world->terrain.block_size);
			z2 = (int)(p2[2] / world->terrain.block_size);
			d  = 0;

			if (x > x2)
				d += x - x2;
			else
				d += x2 - x;

			if (z > z2)
				d += z - z2;
			else
				d += z2 - z;

			if ((goalDist < 0) || (d < goalDist))
				goalDist = d;
		}
	}

	// Add distance representing unattained goals.
	if (!muzz->has_food)
		goalDist += maxDist;

	if (!muzz->has_water)
		goalDist += maxDist;

	return true;
}


// State equal?
bool ResponseSearch::Equals(ResponseSearch* r) {
	if ((x == r->x) && (z == r->z) && (dir == r->dir) &&
		(moveType == r->moveType) &&
		(moveAmount == r->moveAmount) &&
		(muzz->has_food == r->muzz->has_food) &&
		(muzz->has_water == r->muzz->has_water))
		return true;
	else
		return false;
}
