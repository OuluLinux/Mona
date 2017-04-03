// For conditions of distribution and use, see copyright notice in minc.hpp

#include "Tmaze.h"

// Constructor.
Tmaze::Tmaze() {
}


// Destructor.
Tmaze::~Tmaze() {
	path.Clear();
}


// Is given maze a duplicate?
bool Tmaze::IsDuplicate(Tmaze& maze) {
	if ((int)path.GetCount() != (int)maze->path.GetCount())
		return false;

	for (int i = 0; i < path.GetCount(); i++) {
		if (path[i].mark != maze->path[i].mark)
			return false;

		if (path[i].direction != maze->path[i].direction)
			return false;
	}

	return true;
}
