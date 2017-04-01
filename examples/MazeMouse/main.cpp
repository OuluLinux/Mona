#include "MazeMouseApplication.h"


GUI_APP_MAIN {
	String usage      = "Usage: java mona.MazeMouseApplication [-random_seed <random seed>]";
	int    random_seed = GetSysTime().Get();

	/*for (int i = 0; i < args.length; i++) {
		if (args[i].equals("-random_seed")) {
			i++;

			if (i >= args.length) {
				System.err.println("Invalid random seed");
				System.err.println(usage);
				System.exit(1);
			}

			random_seed = Integer.parseInt(args[i]);
		}
		else {
			System.err.println(usage);
			System.exit(1);
		}
	}*/

	MazeMouseApplication mazeMouse;
	mazeMouse.Run();
	
}
