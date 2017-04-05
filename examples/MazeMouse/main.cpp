#include "MazeMouseApplication.h"

#define IMAGECLASS Images
#define IMAGEFILE <MazeMouse/images.iml>
#include <Draw/iml_source.h>


GUI_APP_MAIN {
	MazeMouseApplication mazeMouse;
	mazeMouse.Run();
}
