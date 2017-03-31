#include "TmazeGrammar.h"

const double TmazeGrammar::NEAR_ZERO = 0.0001;

// Constructor.
TmazeGrammar::TmazeGrammar(RANDOM random_seed, int numMarks)
{
   ASSERT(numMarks > 1);

   this->random_seed = random_seed;
   this->numMarks   = numMarks;
   mazerandomizer   = new Random(random_seed);
   ASSERT(mazerandomizer != NULL);
}


// Destructor.
TmazeGrammar::~TmazeGrammar()
{
   if (mazerandomizer != NULL)
   {
      delete mazerandomizer;
   }
}


// Generate maze from grammar.
// Output: vector of location mark/direction pairs.
Tmaze *TmazeGrammar::generateMaze()
{
   Tmaze  *maze;
   Random *randomizer;

   maze = new Tmaze();
   ASSERT(maze != NULL);

   randomizer = new Random(random_seed);
   ASSERT(randomizer != NULL);

   while (extendMazePath(randomizer, maze) != Tmaze::GOAL)
   {
   }
   return (maze);
}


// Extend maze path with mark and door constant pair.
// Return mark constant.
int TmazeGrammar::extendMazePath(Random *randomizer, Tmaze *maze)
{
   Tmaze::Junction junction;

   // Select mark.
   if (maze->path.GetCount() == 0)
   {
      junction.mark = Tmaze::START;
   }
   else
   {
      junction.mark = randomizer->RAND_CHOICE(numMarks - 1) + 1;
   }

   // Select direction.
   if (junction.mark == Tmaze::GOAL)
   {
      junction.direction   = Tmaze::LEFT;
      junction.probability = 1.0;
   }
   else
   {
      junction.probability = randomizer->RAND_PROB();
      if (mazerandomizer->RAND_PROB() <= junction.probability)
      {
         junction.direction = Tmaze::LEFT;
      }
      else
      {
         junction.direction = Tmaze::RIGHT;
      }
   }
   maze->path.Add(junction);

   // Incorporate direction choice into context.
   randomizer->SRAND(randomizer->RAND() ^ (RANDOM)(junction.direction + 1));
   return (junction.mark);
}
