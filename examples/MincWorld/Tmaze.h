#ifndef __TMAZE__
#define __TMAZE__

#include <Mona/Common.h>

class Tmaze
{
public:

   // Special maze symbols.
   enum { START=0, GOAL=1, LEFT=0, RIGHT=1 };

   // Junction.
   class Junction
   {
public:
      int    mark;
      int    direction;
      double probability;
   };

   // Maze path.
   Vector<Junction> path;

   // Constructor.
   Tmaze();

   // Destructor.
   ~Tmaze();

   // Is given maze a duplicate?
   bool IsDuplicate(Tmaze *maze);
};
#endif
