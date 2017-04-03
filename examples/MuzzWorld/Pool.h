// For conditions of distribution and use, see copyright notice in muzz.hpp

// Water pool.

#ifndef POOL_HPP
#define POOL_HPP

#include <MonaCtrl/MonaCtrl.h>
#include "BlockTerrain.h"

class Pool : public BaseObject {
public:

	static double width, height;


	Pool(const Color& color, BlockTerrain& terrain);
	~Pool();
	void GetColor(Color& color);
	void SetColor(const Color& color);
	void Place(int placement_seed);
	void Place(int x, int y);
	void Place(double x, double y);
	void Place();

	inline double GetPlaceX() {
		return m_place_position[0];
	}
	inline double GetPlaceY() {
		return m_place_position[2];
	}

	void Draw();
	void Load(String filename);
	void Load(Stream& s);
	void Store(String filename);
	void Store(Stream& s);

private:

	Color         m_color;
	double        m_place_position[3];
	BlockTerrain  m_terrain;
	//Random        m_randomizer;

	// OpenGL display for drawing.
	/*  GLuint display;

	    // Components.
	    GLUquadricObj* poolBase, *poolSurface;*/

};
#endif
