/*
    Triangle mesh heightmap.
*/

#include "MeshHeightmap.h"
#include <assert.h>

// Parameters.
const int QuadTree::MAX_POLYGONS = 8;             // Maximum polygons per leaf node.
const int QuadTree::MAX_DEPTH    = 14;            // Maximum tree depth.

// Polygon constructor.
Poly::Poly(Vector<Vector3f>& vertices) {
	int    i;
	Vector3f v1, v2, normal;
	double  d;
	// Store vertices.
	ASSERT(vertices.GetCount() > 2);

	for (i = 0; i < vertices.GetCount(); i++)
		this->vertices.Add(vertices[i]);

	// Generate 3D plane.
	v1     = vertices[1] - vertices[0];
	v2     = vertices[2] - vertices[0];
	normal = v1 ^ v2;
	normal.Normalize();
	d = (-normal.x * vertices[0].x) + (-normal.y * vertices[0].y) +
		(-normal.z * vertices[0].z);
	plane = Plane(normal, d);
	// Determine 2D bounds in the XZ plane.
	bounds.min.x = bounds.max.x = vertices[0].x;
	bounds.min.z = bounds.max.z = vertices[0].z;

	for (i = 1; i < vertices.GetCount(); i++) {
		if (vertices[i].x < bounds.min.x)
			bounds.min.x = vertices[i].x;

		if (vertices[i].x > bounds.max.x)
			bounds.max.x = vertices[i].x;

		if (vertices[i].z < bounds.min.z)
			bounds.min.z = vertices[i].z;

		if (vertices[i].z > bounds.max.z)
			bounds.max.z = vertices[i].z;
	}
}


// Is point inside polygon?
// Adapted from Rourke's Computational Geometry FAQ.
bool Poly::contains(double x, double z) {
	int  i, j, size;
	bool ret = false;

	for (i = 0, size = (int)vertices.GetCount(), j = size - 1; i < size; j = i++) {
		if ((((vertices[i].z <= z) && (z < vertices[j].z)) ||
			 ((vertices[j].z <= z) && (z < vertices[i].z))) &&
			(x < (vertices[j].x - vertices[i].x) * (z - vertices[i].z) /
			 (vertices[j].z - vertices[i].z) + vertices[i].x))
			ret = !ret;
	}

	return (ret);
}


// Print polygon.
void Poly::Print() {
	printf("Polygon:\n");
	printf("\tvertices: ");

	for (int i = 0; i < vertices.GetCount(); i++)
		printf("(%f,%f,%f) ", vertices[i].x, vertices[i].y, vertices[i].z);

	printf("\n");
	printf("\tplane: normal=(%f,%f,%f),d=%f\n", plane.normal.x, plane.normal.y, plane.normal.z, plane.d);
	printf("\tbounds: xmin=%f,xmax=%f,zmin=%f,zmax=%f\n",
		   bounds.min.x, bounds.max.x, bounds.min.z, bounds.max.z);
}


// Quadtree constructor.,
QuadTree::QuadTree(Bounds& bounds) {
	root         = NULL;
	this->bounds = bounds;
}


// Destructor.
QuadTree::~QuadTree() {
	if (root != NULL)
		delete root;

	for (int i = 0; i < polygons.GetCount(); i++)
		delete polygons[i];
}


// Insert polygon.
bool QuadTree::Insert(Poly* polygon) {
	if (!bounds.contains(polygon->bounds))
		return false;

	// Insert into tree.
	if (root == NULL) {
		root = new QuadNode(0, bounds);
		ASSERT(root != NULL);
	}

	root->Insert(polygon);
	// Add to global list.
	polygons.Add(polygon);
	return true;
}


// Search for polygons containing coordinate.
void QuadTree::Search(double x, double z, Vector<Poly*>& found) {
	if (root != NULL)
		root->Search(x, z, found);
}


// Print polygons in tree.
void QuadTree::Print() {
	printf("Polygons in tree:\n");

	if (root != NULL)
		root->Print();
}


// Constructors.
QuadNode::QuadNode(int depth, Bounds& bounds) {
	this->depth = depth;

	for (int i = 0; i < 4; i++)
		outerren[i] = NULL;

	this->bounds = bounds;
}


// Destructor.
QuadNode::~QuadNode() {
	for (int i = 0; i < 4; i++) {
		if (outerren[i] != NULL)
			delete outerren[i];
	}
}


// Insert polygon.
void QuadNode::Insert(Poly* polygon) {
	int    i, j;
	Bounds outerBounds;
	double  rx, rz;

	// OK at this depth?
	if ((depth >= QuadTree::MAX_DEPTH) ||
		((polygons.GetCount() < QuadTree::MAX_POLYGONS) && (outerren[0] == NULL))) {
		polygons.Add(polygon);
		return;
	}

	// Create outerren?
	if (outerren[0] == NULL) {
		rx = (bounds.max.x - bounds.min.x) / 2.0;
		rz = (bounds.max.z - bounds.min.z) / 2.0;
		outerBounds.min.x = bounds.min.x;
		outerBounds.max.x = bounds.max.x - rx;
		outerBounds.min.z = bounds.min.z;
		outerBounds.max.z = bounds.max.z - rz;
		outerren[0]       = new QuadNode(depth + 1, outerBounds);
		ASSERT(outerren[0] != NULL);
		outerBounds.min.x = bounds.min.x;
		outerBounds.max.x = bounds.max.x - rx;
		outerBounds.min.z = bounds.min.z + rz;
		outerBounds.max.z = bounds.max.z;
		outerren[1]       = new QuadNode(depth + 1, outerBounds);
		ASSERT(outerren[1] != NULL);
		outerBounds.min.x = bounds.min.x + rx;
		outerBounds.max.x = bounds.max.x;
		outerBounds.min.z = bounds.min.z + rz;
		outerBounds.max.z = bounds.max.z;
		outerren[2]       = new QuadNode(depth + 1, outerBounds);
		ASSERT(outerren[2] != NULL);
		outerBounds.min.x = bounds.min.x + rx;
		outerBounds.max.x = bounds.max.x;
		outerBounds.min.z = bounds.min.z;
		outerBounds.max.z = bounds.max.z - rz;
		outerren[3]       = new QuadNode(depth + 1, outerBounds);
		ASSERT(outerren[3] != NULL);
	}

	// Insert polygons into outerren.
	polygons.Add(polygon);

	for (i = 0; i < polygons.GetCount(); i++) {
		for (j = 0; j < 4; j++) {
			if (outerren[j]->bounds.intersects(polygons[i]->bounds))
				outerren[j]->Insert(polygons[i]);
		}
	}

	polygons.Clear();
}


// Search for polygons containing coordinate.
void QuadNode::Search(double x, double z, Vector<Poly*>& found) {
	int i;

	for (i = 0; i < polygons.GetCount(); i++) {
		if (polygons[i]->contains(x, z))
			found.Add(polygons[i]);
	}

	if (outerren[0] == NULL)
		return;

	for (i = 0; i < 4; i++) {
		if (outerren[i]->bounds.contains(x, z))
			outerren[i]->Search(x, z, found);
	}
}


// Recursively print polygons in node.
void QuadNode::Print() {
	int i;
	printf("Polygons in node at depth=%d, bounds: xmin=%f,xmax=%f,zmin=%f,zmax=%f\n",
		   depth, bounds.min.x, bounds.max.x, bounds.min.z, bounds.max.z);

	for (i = 0; i < polygons.GetCount(); i++)
		polygons[i]->Print();

	if (outerren[0] == NULL)
		return;

	for (i = 0; i < 4; i++)
		outerren[i]->Print();
}


// Given point contained in this?
bool Bounds::contains(double x, double z) {
	if (x < min.x)
		return false;

	if (x > max.x)
		return false;

	if (z < min.z)
		return false;

	if (z > max.z)
		return false;

	return true;
}


// Is given bounds contained within this?
bool Bounds::contains(Bounds& bounds) {
	if (!contains(bounds.min.x, bounds.min.z))
		return false;

	if (!contains(bounds.max.x, bounds.min.z))
		return false;

	if (!contains(bounds.min.x, bounds.max.z))
		return false;

	if (!contains(bounds.max.x, bounds.max.z))
		return false;

	return true;
}


// Does given rectangle intersect this?
bool Bounds::intersects(Bounds& bounds) {
	if (contains(bounds))
		return true;

	if (bounds.contains(*this))
		return true;

	if ((((min.x >= bounds.min.x) && (min.x <= bounds.max.x)) ||
		 ((max.x >= bounds.min.x) && (max.x <= bounds.max.x))) &&
		(((bounds.min.z >= min.z) && (bounds.min.z <= max.z)) ||
		 ((bounds.max.z >= min.z) && (bounds.max.z <= max.z))))
		return true;

	if ((((bounds.min.x >= min.x) && (bounds.min.x <= max.x)) ||
		 ((bounds.max.x >= min.x) && (bounds.max.x <= max.x))) &&
		(((min.z >= bounds.min.z) && (min.z <= bounds.max.z)) ||
		 ((max.z >= bounds.min.z) && (max.z <= bounds.max.z))))
		return true;

	return false;
}
