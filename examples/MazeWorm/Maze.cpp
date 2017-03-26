#include "Maze.h"

Maze::Maze() {
	
}

void Maze::ClearGrid() {
	grid.Clear();
	grid.SetCount(w * h, 0);
}

void Maze::CarveRecursiveBacktrack(int x, int y) {
	Vector<int> dirs;
	dirs.Add(0);
	dirs.Add(1);
	dirs.Add(2);
	dirs.Add(3);
	for(int i = 0; i < 10; i++) {
		int a = Random(4);
		int b = Random(4);
		if (a == b) {i--; continue;}
		dirs.Swap(a, b);
	}
	for(int i = 0; i < 4; i++) {
		int k = dirs[i];
		int j = k + 2;
		if (j >= 4) j -= 4;
		int dir = 1 << k;
		int opposite_dir = 1 << j;
		int nx, ny;
		switch (k) {
			case 0: nx = x; ny = y - 1; break;
			case 1: nx = x + 1; ny = y; break;
			case 2: nx = x; ny = y + 1; break;
			case 3: nx = x - 1; ny = y; break;
		}
		if (nx < 0 || ny < 0 || nx >= w || ny >= h) {
			continue;
		}
		byte& new_pos  = grid[nx + ny * w];
		byte& prev_pos = grid[x  + y  * w];
		if (new_pos != 0) {
			continue;
		}
		new_pos  |= opposite_dir;
		prev_pos |= dir;
		CarveRecursiveBacktrack(nx, ny);
	}
}

void Maze::GenerateRecursiveBacktrack() {
	ClearGrid();
	CarveRecursiveBacktrack(0, 0);
	RefreshDistanceMap();
}

void Maze::RefreshDistanceMap() {
	int x = exit.x;
	int y = exit.y;
	distance_map.Clear();
	distance_map.SetCount(w * h, -1);
	
	Vector<int> pos;
	for(int i = 0; i < grid.GetCount(); i++) {
		if (grid[i] != 0)
			pos.Add(i);
	}
	
	int level = 0;
	distance_map[x + y * w] = level;
	while (1) {
		int level_edges = 0;
		for(int p = 0; p < pos.GetCount(); p++) {
			int i = pos[p];
			int x = i % w;
			int y = i / w;
			byte b = grid[i];
			int edges = 0;
			for(int j = 0; j < 4; j++) {
				byte dir = 1 << j;
				if ((b & dir) == 0)
					continue;
				int x2, y2;
				switch (j) {
					case 0: x2 = x; y2 = y - 1; break;
					case 1: x2 = x+1; y2 = y; break;
					case 2: x2 = x; y2 = y + 1; break;
					case 3: x2 = x-1; y2 = y; break;
				}
				if (x2 < 0 || y2 < 0 || x2 >= w || y2 >= h)
					continue;
				int z = x2 + y2 * w;
				if (z < 0 || z >= distance_map.GetCount()) continue;
				if (distance_map[z] == level) edges++;
			}
			if (edges) {
				distance_map[i] = level + 1;
				pos.Remove(p);
				p--;
				level_edges++;
			}
		}
		if (!level_edges)
			break;
		level++;
	}
	
}

Vector<Point> Maze::GetDistanceMapPath(int x, int y) {
	Vector<Point> out;
	int distance = distance_map[x + y * w];
	
	if (distance == -1)
		return out;
	
	while (distance > 0) {
		bool moved = false;
		for(int k = 0; k < 4; k++) {
			int dir = 1 << k;
			byte b = grid[x + w * y];
			if ((b & dir) == 0)
				continue;
			int x2, y2;
			switch (k) {
				case 0: x2 = x; y2 = y - 1; break;
				case 1: x2 = x+1; y2 = y; break;
				case 2: x2 = x; y2 = y + 1; break;
				case 3: x2 = x-1; y2 = y; break;
			}
			if (x2 < 0 || y2 < 0 || x2 >= w || y2 >= h)
				continue;
			double d = distance_map[x2 + w * y2];
			if (d != -1 && d < distance) {
				distance = d;
				out.Add(Point(x, y));
				x = x2;
				y = y2;
				moved = true;
				break;
			}
		}
		if (!moved) {
			out.Add(Point(x, y));
			break;
		}
	}
	
	return out;
}

Image Maze::GetImage(int gw, int gh, int bw) {
	Size sz(w * gw, h * gh);
	ImageBuffer ib(sz);
	
	int max_dist = 0;
	for(int i = 0; i < distance_map.GetCount(); i++) max_dist = max(max_dist, distance_map[i]);
	
	RGBA* begin = ib.Begin();
	RGBA* it;
	for(int y = 0; y < h; y++) {
		for(int x = 0; x < w; x++) {
			int x0 = x*gw;
			int y0 = y*gh * w*gw;
			int z = x + y*w;
			int dist = distance_map[z];
			byte gray = dist == -1 ? 0 : 255 - dist * 255 / max_dist / 2;
			for(int i = 0; i < gh; i++) {
				it = begin + (x0 + y0 + i * w*gw);
				for(int j = 0; j < gw; j++) {
					it->r = gray;
					it->g = gray;
					it->b = gray;
					it->a = 255;
					it++;
				}
			}
			
			byte b = grid[z];
			bool N = b & 1;
			bool E = b & 2;
			bool S = b & 4;
			bool W = b & 8;
			if (!N) {
				for(int i = 0; i < bw; i++) {
					it = begin + (x0 + y0 + i * w*gw);
					for(int j = 0; j < gw; j++) {
						it->r = 0;
						it->g = 0;
						it->b = 0;
						it++;
					}
				}
			}
			if (!S) {
				for(int i = 0; i < bw; i++) {
					it = begin + (x0 + y0 + (gh - bw + i) * w*gw);
					for(int j = 0; j < gw; j++) {
						it->r = 0;
						it->g = 0;
						it->b = 0;
						it++;
					}
				}
			}
			if (!E) {
				for(int i = 0; i < gh; i++) {
					it = begin + (x0 + (gw - bw) + y0 + i * w*gw);
					for(int j = 0; j < bw; j++) {
						it->r = 0;
						it->g = 0;
						it->b = 0;
						it++;
					}
				}
			}
			if (!W) {
				for(int i = 0; i < gh; i++) {
					it = begin + (x0 + y0 + i * w*gw);
					for(int j = 0; j < bw; j++) {
						it->r = 0;
						it->g = 0;
						it->b = 0;
						it++;
					}
				}
			}
		}
	}
	
	return ib;
}

