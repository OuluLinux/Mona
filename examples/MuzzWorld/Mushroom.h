#ifndef MUSHROOM_HPP
#define MUSHROOM_HPP


#include <MonaCtrl/MonaCtrl.h>
#include "BlockTerrain.h"

class Mushroom : public BaseObject {
public:

	static double width, height;

	Mushroom(Color& color, BlockTerrain& terrain);
	~Mushroom();

	bool IsAlive() {return m_alive;}
	void SetAlive(bool alive) {m_alive = alive;}

	void GetColor(Color& color);
	void SetColor(const Color& color);

	void Place(int placement_seed);
	void Place(int x, int y);
	void Place(double x, double y);
	void Place();

	inline double GetPlaceX() {return (m_place_position[0]);}
	inline double GetPlaceY() {return (m_place_position[2]);}

	void Draw();
	void Load(String filename);
	void Load(Stream& s);
	void Store(String filename);
	void Store(Stream& s);

private:

	bool          m_alive;
	Color         m_color;
	double        m_place_position[3];
	BlockTerrain* m_terrain;
	//Random*       m_randomizer;

	// OpenGL display for drawing.
	/*GLuint display;

	// Components.
	GLUquadricObj* stem, *cap, *capBorder;*/
};
#endif
