
public class Tmaze {
	// Maze junction.
	class Junction {
		int            mark;
		int            direction;
		double         probability;
		bool        highlight;
		int            choice;
		Vector<String> annotations;

		Junction(int mark, int direction, double probability) {
			this.mark        = mark;
			this.direction   = direction;
			this.probability = probability;
			highlight        = false;
			choice           = -1;
			annotations      = null;
		}


		Junction() {
			mark        = direction = -1;
			highlight   = false;
			probability = 0.0;
			choice      = -1;
			annotations = null;
		}
	}

	Vector<Junction> path;

	Tmaze() {
		path = new Vector<Junction>();
	}


	void addJunction(int mark, int direction, double probability) {
		path.Add(new Junction(mark, direction, probability));
	}


	void addJunction(Junction junction) {
		path.Add(junction);
	}


	// Is given maze a duplicate?
	bool IsDuplicate(Tmaze maze) {
		if (path.GetCount() != maze.path.GetCount())
			return false;

		for (int i = 0; i < path.GetCount(); i++) {
			if (path.Get(i).mark != maze.path.Get(i).mark)
				return false;

			if (path.Get(i).direction != maze.path.Get(i).direction)
				return false;
		}

		return true;
	}


	void clearAnnotations() {
		for (int i = 0; i < path.GetCount(); i++) {
			path.Get(i).highlight   = false;
			path.Get(i).choice      = -1;
			path.Get(i).annotations = null;
		}
	}


	void Print() {
		Junction junction;
		System.out.println("Mark\tDirection\tProbabilities");

		for (int i = 0; i < path.GetCount(); i++) {
			junction = path.Get(i);

			if (junction.direction == 0) {
				System.out.println(junction.mark + "\tleft\t" +
								   junction.probability + " " + (1.0 - junction.probability));
			}
			else {
				System.out.println(junction.mark + "\right\t" +
								   junction.probability + " " + (1.0 - junction.probability));
			}
		}
	}
};
