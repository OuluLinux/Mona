/*
 * Mox world: mona learning automata in a Game of Life world.
 */

import java.util.*;
import java.io.*;
import java.awt.*;
import javax.swing.*;
import mona.NativeFileDescriptor;

// Mox world.
public class MoxWorld
{
   // Usage.
   public static final String Usage =
      "Usage:\n" +
      "  New run:\n" +
      "    java MoxWorld\n" +
      "      -steps <steps>\n" +
      "     [-stepGameOfLife]\n" +
      "        -gridSize <width> <height>\n" +
      "        -live_cell_probability <0.0-1.0>\n" +
      "      |\n" +
      "        -loadCells <file name>\n" +
      "     [-foragerMoxen <quantity>]\n" +
      "     [-predatorMoxen <quantity>]\n" +
      "     [-random_seed <random number seed>]\n" +
      "     [-save <file name>]\n" +
      "     [-dashboard]\n" +
      "  Resume run:\n" +
      "    java MoxWorld\n" +
      "      -steps <steps>\n" +
      "     [-stepGameOfLife]\n" +
      "      -load <file name>\n" +
      "     [-save <file name>]\n" +
      "     [-dashboard]";

   // Default random seed.
   public static final int DEFAULT_RANDOM_SEED = 4517;

   // Moxen.
   ArrayList<Mox> moxen;

   // Game of Life.
   GameOfLife game_of_life;

   // dashboard display.
   MoxWorldDashboard dashboard;

   // Constructor.
   public MoxWorld()
   {
   }


   // Initialize cells.
   public void InitCells(int width, int height, double live_cell_prob, int random_seed)
   {
      int x, y;

      // Random numbers.
      Random random = new Random(random_seed);

      // Create Game of Life.
      game_of_life = new GameOfLife(new Size(width, height));
      for (x = 0; x < width; x++)
      {
         for (y = 0; y < height; y++)
         {
            if (random.nextFloat() < live_cell_prob)
            {
               game_of_life.cells[x][y] = 1;
            }
         }
      }
      game_of_life.Step();
      game_of_life.Checkpoint();
   }


   // Get width.
   int GetWidth()
   {
      if (game_of_life != null)
      {
         return (game_of_life.size.width);
      }
      else
      {
         return (0);
      }
   }


   // Get height.
   int GetHeight()
   {
      if (game_of_life != null)
      {
         return (game_of_life.size.height);
      }
      else
      {
         return (0);
      }
   }


   // Load cells from file.
   public void loadCells(String cellsFilename) throws IOException
   {
      game_of_life = new GameOfLife();
      try {
         game_of_life.Load(cellsFilename);
      }
      catch (Exception e) {
         throw new IOException("Cannot open cells file " + cellsFilename +
                               ":" + e.getMessage());
      }
   }


   // Create moxen.
   public void createMoxen(int numForagers, int numPredators, int random_seed)
   {
      int i, x, y, w, h;

      // Random numbers.
      Random random = new Random(random_seed);

      // Create moxen.
      w     = game_of_life.size.width;
      h     = game_of_life.size.height;
      moxen = new ArrayList<Mox>(numForagers + numPredators);
      for (i = 0; i < numForagers; i++)
      {
         x = random.nextInt(w);
         y = random.nextInt(h);
         moxen.Add(i, new ForagerMox(i, x, y,
                                     random.nextInt(Mox.DIRECTION.NUM_DIRECTIONS.GetValue()),
                                     random.nextInt()));
      }
      for (i = 0; i < numPredators; i++)
      {
         x = random.nextInt(w);
         y = random.nextInt(h);
         moxen.Add(numForagers + i, new PredatorMox(numForagers + i, x, y,
                                                    random.nextInt(Mox.DIRECTION.NUM_DIRECTIONS.GetValue()),
                                                    random.nextInt()));
      }
   }


   // Set moxen.
   public void setMoxen(ArrayList<Mox> moxen)
   {
      this.moxen = moxen;
      if (dashboard != null)
      {
         dashboard.setMoxen(moxen);
      }
   }


   // Reset.
   public void reset()
   {
      if (game_of_life != null)
      {
         game_of_life.Restore();
      }
      if (moxen != null)
      {
         int numMox = moxen.GetCount();
         for (int i = 0; i < numMox; i++)
         {
            moxen.Get(i).reset();
         }
      }
      if (dashboard != null)
      {
         dashboard.reset();
      }
   }


   // Clear.
   public void Clear()
   {
      if (dashboard != null)
      {
         dashboard.setVisible(false);
         dashboard = null;
      }
      game_of_life = null;
      if (moxen != null)
      {
         int numMox = moxen.GetCount();
         for (int i = 0; i < numMox; i++)
         {
            moxen.Get(i).Clear();
         }
         moxen = null;
      }
   }


   // Load from file.
   public void Load(String filename) throws IOException
   {
      FileInputStream      input;
      NativeFileDescriptor fd;

      // Open the file.
      try {
         input = new FileInputStream(new File(filename));
         fd    = new NativeFileDescriptor(filename, "r");
         fd.open();
      }
      catch (Exception e) {
         throw new IOException("Cannot open input file " + filename +
                               ":" + e.getMessage());
      }

      // Load world.
      Load(input, fd);

      input.close();
      fd.close();
   }


   // Load.
   public void Load(FileInputStream input, NativeFileDescriptor fd) throws IOException
   {
      // DataInputStream is for unbuffered input.
      DataInputStream reader = new DataInputStream(input);

      // Load Game of Life.
      game_of_life = new GameOfLife();
      game_of_life.Load(reader);

      // Load moxen.
      int numMox = Utility.loadInt(reader);
      moxen = new ArrayList<Mox>(numMox);
      ForagerMox  forager;
      PredatorMox predator;
      for (int i = 0; i < numMox; i++)
      {
         if (Utility.loadInt(reader) == Mox.SPECIES.FORAGER.GetValue())
         {
            forager = new ForagerMox();
            forager.Load(input, fd);
            moxen.Add(i, forager);
         }
         else
         {
            predator = new PredatorMox();
            predator.Load(input, fd);
            moxen.Add(i, predator);
         }
      }
   }


   // Save to file.
   public void Store(String filename) throws IOException
   {
      FileOutputStream     output;
      NativeFileDescriptor fd;

      try
      {
         output = new FileOutputStream(new File(filename));
         fd     = new NativeFileDescriptor(filename, "w");
         fd.open();
      }
      catch (Exception e) {
         throw new IOException("Cannot open output file " + filename +
                               ":" + e.getMessage());
      }

      // Save world.
      Store(output, fd);

      output.close();
      fd.close();
   }


   // Save.
   public void Store(FileOutputStream output, NativeFileDescriptor fd) throws IOException
   {
      PrintWriter writer = new PrintWriter(output);

      // Save Game of Life.
      game_of_life.Store(writer);

      // Save moxen.
      int numMox = moxen.GetCount();
      Utility.saveInt(writer, numMox);
      writer.flush();
      Mox mox;
      for (int i = 0; i < numMox; i++)
      {
         mox = moxen.Get(i);
         Utility.saveInt(writer, mox.species);
         writer.flush();
         mox.Store(output, fd);
      }
   }


   // Run.
   public void Run(int steps, bool stepGameOfLife)
   {
      for (int i = 0; i < steps; i++)
      {
         // Update dashboard.
         updatedashboard(i + 1, steps);

         // Step moxen.
         stepMoxen();

         // Step Game of Life.
         if (stepGameOfLife)
         {
            stepGameOfLife();
         }
      }
   }


   // Step moxen.
   public void stepMoxen()
   {
      int i, x, y, numMox;
      Mox mox;

      int width  = game_of_life.size.width;
      int height = game_of_life.size.height;

      int[][] moveTo      = new int[width][height];
      boolean[][] eatCell = new boolean[width][height];

      numMox = moxen.GetCount();

      synchronized (game_of_life.lock)
      {
         // Load moxen into cells.
         for (i = 0; i < numMox; i++)
         {
            mox = moxen.Get(i);
            if (mox.isAlive)
            {
               if (mox.species == Mox.SPECIES.FORAGER.GetValue())
               {
                  game_of_life.cells[mox.x][mox.y] = ForagerMox.FORAGER_COLOR_VALUE;
               }
               else
               {
                  game_of_life.cells[mox.x][mox.y] = PredatorMox.PREDATOR_COLOR_VALUE;
               }
            }
         }

         // Clear action maps.
         for (x = 0; x < width; x++)
         {
            for (y = 0; y < height; y++)
            {
               moveTo[x][y]  = -1;
               eatCell[x][y] = false;
            }
         }

         // Step forager moxen.
         for (i = 0; i < numMox; i++)
         {
            mox = moxen.Get(i);
            if (mox.isAlive && (mox.species == Mox.SPECIES.FORAGER.GetValue()))
            {
               stepMox(i, moveTo, eatCell);
            }
         }

         // Step predator moxen.
         // This is done after foragers since predators may eat foragers.
         for (i = 0; i < numMox; i++)
         {
            mox = moxen.Get(i);
            if (mox.isAlive && (mox.species == Mox.SPECIES.PREDATOR.GetValue()))
            {
               stepMox(i, moveTo, eatCell);
            }
         }

         // Perform actions.
         for (x = 0; x < width; x++)
         {
            for (y = 0; y < height; y++)
            {
               if (moveTo[x][y] >= 0)
               {
                  mox = moxen.Get(moveTo[x][y]);
                  game_of_life.cells[mox.x][mox.y] = 0;
                  mox.x = x;
                  mox.y = y;
                  if (mox.isAlive)
                  {
                     if (mox.species == Mox.SPECIES.FORAGER.GetValue())
                     {
                        game_of_life.cells[mox.x][mox.y] = ForagerMox.FORAGER_COLOR_VALUE;
                     }
                     else
                     {
                        game_of_life.cells[mox.x][mox.y] = PredatorMox.PREDATOR_COLOR_VALUE;
                     }
                  }
               }
               if (eatCell[x][y] == true)
               {
                  game_of_life.cells[x][y] = 0;
               }
            }
         }
      }
   }


   // Step mox.
   void stepMox(int moxIndex, int[][] moveTo, boolean[][] eatCell)
   {
      int     x, y, mx, my, x1, y1, x2, y2, x3, y3;
      int     range, rangeIndex, colorHueIndex, colorIntensityIndex, response, numMox;
      double   distance;
      Mox     mox, preyMox;
      bool gotGoal;

      Vector<double> sensors     = new float[Mox.SENSOR_CONFIG.NUM_SENSORS.GetValue()];
      rangeIndex          = Mox.SENSOR_CONFIG.RANGE_SENSOR_INDEX.GetValue();
      colorHueIndex       = Mox.SENSOR_CONFIG.COLOR_HUE_SENSOR_INDEX.GetValue();
      colorIntensityIndex = Mox.SENSOR_CONFIG.COLOR_INTENSITY_SENSOR_INDEX.GetValue();

      // Detect color ahead.
      int width  = game_of_life.size.width;
      int height = game_of_life.size.height;

      mox = moxen.Get(moxIndex);
      mx  = mox.x;
      my  = mox.y;
      if (mox.direction == Mox.DIRECTION.NORTH.GetValue())
      {
         for (range = 0, x = mox.x, y = ((mox.y + 1) % height);
              range < height - 2; range++, y = ((y + 1) % height))
         {
            if (range == 0)
            {
               mx = x;
               my = y;
            }
            if (game_of_life.cells[x][y] > 0) { break; }
         }
      }
      else if (mox.direction == Mox.DIRECTION.EAST.GetValue())
      {
         for (range = 0, x = (mox.x + 1) % width, y = mox.y;
              range < width - 2; range++, x = ((x + 1) % width))
         {
            if (range == 0)
            {
               mx = x;
               my = y;
            }
            if (game_of_life.cells[x][y] > 0) { break; }
         }
      }
      else if (mox.direction == Mox.DIRECTION.SOUTH.GetValue())
      {
         x = mox.x;
         y = mox.y - 1;
         if (y < 0) { y += height; }
         for (range = 0; range < height - 2; range++)
         {
            if (range == 0)
            {
               mx = x;
               my = y;
            }
            if (game_of_life.cells[x][y] > 0) { break; }
            y -= 1;
            if (y < 0) { y += height; }
         }
      }
      else
      {
         x = mox.x - 1;
         if (x < 0) { x += width; }
         y = mox.y;
         for (range = 0; range < width - 2; range++)
         {
            if (range == 0)
            {
               mx = x;
               my = y;
            }
            if (game_of_life.cells[x][y] > 0) { break; }
            x -= 1;
            if (x < 0) { x += width; }
         }
      }
      if (mox.species == Mox.SPECIES.FORAGER.GetValue())
      {
         if ((ForagerMox.MAX_SENSOR_RANGE >= 0.0f) &&
             ((float)range > ForagerMox.MAX_SENSOR_RANGE))
         {
            sensors[colorHueIndex]       = 0.0;
            sensors[colorIntensityIndex] = 0.0;
         }
         else
         {
            sensors[colorHueIndex]       = (float)game_of_life.cells[x][y];
            sensors[colorIntensityIndex] = 1.0f / (float)(range + 1);
         }
      }
      else
      {
         if ((PredatorMox.MAX_SENSOR_RANGE >= 0.0f) &&
             ((float)range > PredatorMox.MAX_SENSOR_RANGE))
         {
            sensors[colorHueIndex]       = 0.0;
            sensors[colorIntensityIndex] = 0.0;
         }
         else
         {
            sensors[colorHueIndex]       = (float)game_of_life.cells[x][y];
            sensors[colorIntensityIndex] = 1.0f / (float)(range + 1);
         }
      }

      // Get distance to nearest goal.
      if (mox.species == Mox.SPECIES.FORAGER.GetValue())
      {
         if (ForagerMox.MAX_SENSOR_RANGE >= 0.0f)
         {
            sensors[rangeIndex] = ForagerMox.MAX_SENSOR_RANGE *
                                  ForagerMox.MAX_SENSOR_RANGE;
            if ((width / 2) < (int)ForagerMox.MAX_SENSOR_RANGE)
            {
               x1 = mx - (width / 2);
               x2 = mx + (width / 2);
            }
            else
            {
               x1 = mx - (int)ForagerMox.MAX_SENSOR_RANGE;
               x2 = mx + (int)ForagerMox.MAX_SENSOR_RANGE;
            }
            if ((height / 2) < (int)ForagerMox.MAX_SENSOR_RANGE)
            {
               y1 = my - (height / 2);
               y2 = my + (height / 2);
            }
            else
            {
               y1 = my - (int)ForagerMox.MAX_SENSOR_RANGE;
               y2 = my + (int)ForagerMox.MAX_SENSOR_RANGE;
            }
            for (x = x1; x <= x2; x++)
            {
               x3 = x;
               if (x < 0) { x3 += width; }
               if (x >= width) { x3 = x % width; }
               for (y = y1; y <= y2; y++)
               {
                  y3 = y;
                  if (y < 0) { y3 += height; }
                  if (y >= height) { y3 = y % height; }
                  if (game_of_life.cells[x3][y3] == GameOfLife.BLUE_CELL_COLOR_VALUE)
                  {
                     distance  = (float)((mx - x) * (mx - x));
                     distance += (float)((my - y) * (my - y));
                     if (distance < sensors[rangeIndex])
                     {
                        sensors[rangeIndex] = distance;
                     }
                  }
               }
            }
            sensors[rangeIndex] = (float)Math.sqrt(sensors[rangeIndex]);
            if (sensors[rangeIndex] > ForagerMox.MAX_SENSOR_RANGE)
            {
               sensors[rangeIndex] = ForagerMox.MAX_SENSOR_RANGE;
            }
         }
         else
         {
            sensors[rangeIndex] = (float)((width + height) * (width + height));
            x1 = mx - (width / 2);
            x2 = mx + (width / 2);
            y1 = my - (height / 2);
            y2 = my + (height / 2);
            for (x = x1; x <= x2; x++)
            {
               x3 = x;
               if (x < 0) { x3 += width; }
               if (x >= width) { x3 = x % width; }
               for (y = y1; y <= y2; y++)
               {
                  y3 = y;
                  if (y < 0) { y3 += height; }
                  if (y >= height) { y3 = y % height; }
                  if (game_of_life.cells[x3][y3] == GameOfLife.BLUE_CELL_COLOR_VALUE)
                  {
                     distance  = (float)((mx - x) * (mx - x));
                     distance += (float)((my - y) * (my - y));
                     if (distance < sensors[rangeIndex])
                     {
                        sensors[rangeIndex] = distance;
                     }
                  }
               }
            }
            sensors[rangeIndex] = (float)Math.sqrt(sensors[rangeIndex]);
         }
      }
      else if (mox.species == Mox.SPECIES.PREDATOR.GetValue())
      {
         if (PredatorMox.MAX_SENSOR_RANGE >= 0.0f)
         {
            sensors[rangeIndex] = PredatorMox.MAX_SENSOR_RANGE *
                                  PredatorMox.MAX_SENSOR_RANGE;
            if ((width / 2) < (int)PredatorMox.MAX_SENSOR_RANGE)
            {
               x1 = mx - (width / 2);
               x2 = mx + (width / 2);
            }
            else
            {
               x1 = mx - (int)PredatorMox.MAX_SENSOR_RANGE;
               x2 = mx + (int)PredatorMox.MAX_SENSOR_RANGE;
            }
            if ((height / 2) < (int)PredatorMox.MAX_SENSOR_RANGE)
            {
               y1 = my - (height / 2);
               y2 = my + (height / 2);
            }
            else
            {
               y1 = my - (int)PredatorMox.MAX_SENSOR_RANGE;
               y2 = my + (int)PredatorMox.MAX_SENSOR_RANGE;
            }
            for (x = x1; x <= x2; x++)
            {
               x3 = x;
               if (x < 0) { x3 += width; }
               if (x >= width) { x3 = x % width; }
               for (y = y1; y <= y2; y++)
               {
                  y3 = y;
                  if (y < 0) { y3 += height; }
                  if (y >= height) { y3 = y % height; }
                  if (game_of_life.cells[x3][y3] == ForagerMox.FORAGER_COLOR_VALUE)
                  {
                     distance  = (float)((mx - x) * (mx - x));
                     distance += (float)((my - y) * (my - y));
                     if (distance < sensors[rangeIndex])
                     {
                        sensors[rangeIndex] = distance;
                     }
                  }
               }
            }
            sensors[rangeIndex] = (float)Math.sqrt(sensors[rangeIndex]);
            if (sensors[rangeIndex] > PredatorMox.MAX_SENSOR_RANGE)
            {
               sensors[rangeIndex] = PredatorMox.MAX_SENSOR_RANGE;
            }
         }
         else
         {
            sensors[rangeIndex] = (float)((width + height) * (width + height));
            x1 = mx - (width / 2);
            x2 = mx + (width / 2);
            y1 = my - (height / 2);
            y2 = my + (height / 2);
            for (x = x1; x <= x2; x++)
            {
               x3 = x;
               if (x < 0) { x3 += width; }
               if (x >= width) { x3 = x % width; }
               for (y = y1; y <= y2; y++)
               {
                  y3 = y;
                  if (y < 0) { y3 += height; }
                  if (y >= height) { y3 = y % height; }
                  if (game_of_life.cells[x3][y3] == ForagerMox.FORAGER_COLOR_VALUE)
                  {
                     distance  = (float)((mx - x) * (mx - x));
                     distance += (float)((my - y) * (my - y));
                     if (distance < sensors[rangeIndex])
                     {
                        sensors[rangeIndex] = distance;
                     }
                  }
               }
            }
            sensors[rangeIndex] = (float)Math.sqrt(sensors[rangeIndex]);
         }
      }

      // Check for goal.
      gotGoal = false;
      if (mox.species == Mox.SPECIES.FORAGER.GetValue())
      {
         if ((range == 0) &&
             (sensors[colorHueIndex] == GameOfLife.BLUE_CELL_COLOR_VALUE))
         {
            gotGoal = true;
         }
      }
      else if (mox.species == Mox.SPECIES.PREDATOR.GetValue())
      {
         if ((range == 0) &&
             (sensors[colorHueIndex] == ForagerMox.FORAGER_COLOR_VALUE))
         {
            gotGoal = true;
         }
      }

      // Cycle mox.
      response = mox.Cycle(sensors);

      // Process response.
      if (response == Mox.RESPONSE_TYPE.FORWARD.GetValue())
      {
         if (range > 0)
         {
            if (moveTo[mx][my] == -1)
            {
               moveTo[mx][my] = moxIndex;
            }
            else
            {
               moveTo[mx][my] = -2;
            }
         }
      }
      else if (response == Mox.RESPONSE_TYPE.RIGHT.GetValue())
      {
         mox.direction = (mox.direction + 1) % Mox.DIRECTION.NUM_DIRECTIONS.GetValue();
      }
      else if (response == Mox.RESPONSE_TYPE.LEFT.GetValue())
      {
         mox.direction -= 1;
         if (mox.direction < 0)
         {
            mox.direction += Mox.DIRECTION.NUM_DIRECTIONS.GetValue();
         }
      }

      if (gotGoal)
      {
         if (mox.species == Mox.SPECIES.FORAGER.GetValue())
         {
            if (game_of_life.cells[mx][my] == GameOfLife.BLUE_CELL_COLOR_VALUE)
            {
               eatCell[mx][my] = true;
            }
         }
         else
         {
            // Predator eats forager.
            numMox = moxen.GetCount();
            for (int i = 0; i < numMox; i++)
            {
               preyMox = moxen.Get(i);
               if ((preyMox.x == mx) && (preyMox.y == my))
               {
                  preyMox.isAlive = false;
                  if (dashboard != null)
                  {
                     dashboard.closeMoxdashboard(i);
                  }
                  eatCell[mx][my] = true;
                  break;
               }
            }
         }
      }
   }


   // Step Game Of Life.
   public void stepGameOfLife()
   {
      synchronized (game_of_life.lock)
      {
         game_of_life.Step();
      }
   }


   // Create dashboard.
   public void createdashboard()
   {
      if (dashboard == null)
      {
         if (moxen == null)
         {
            dashboard = new MoxWorldDashboard(game_of_life);
         }
         else
         {
            dashboard = new MoxWorldDashboard(game_of_life, moxen);
         }
      }
   }


   // Destroy dashboard.
   public void destroydashboard()
   {
      if (dashboard != null)
      {
         dashboard.reset();
         dashboard.setVisible(false);
         dashboard = null;
      }
   }


   // Update dashboard.
   // Return true if dashboard operational.
   public bool updatedashboard(int step, int steps, String message)
   {
      if (dashboard != null)
      {
         dashboard.setMessage(message);
         dashboard.Update(step, steps);
         if (dashboard.quit)
         {
            dashboard = null;
            return false;
         }
         else
         {
            return true;
         }
      }
      else
      {
         return false;
      }
   }


   public bool updatedashboard(int step, int steps)
   {
      return (updatedashboard(step, steps, ""));
   }


   public bool updatedashboard()
   {
      if (dashboard != null)
      {
         dashboard.Update();
         if (dashboard.quit)
         {
            dashboard = null;
            return false;
         }
         else
         {
            return true;
         }
      }
      else
      {
         return false;
      }
   }


   // Main.
   public static void main(String[] args)
   {
      // Get options.
      int     steps          = -1;
      bool stepGameOfLife = false;
      int     width          = -1;
      int     height         = -1;
      double   live_cell_prob   = -1.0;
      int     numForagers    = 0;
      int     numPredators   = 0;
      int     random_seed     = DEFAULT_RANDOM_SEED;
      String  cellsFilename  = null;
      String  loadfile       = null;
      String  savefile       = null;
      bool dashboard      = false;

      for (int i = 0; i < args.length; i++)
      {
         if (args[i].equals("-steps"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid steps option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            try
            {
               steps = Integer.parseInt(args[i]);
            }
            catch (NumberFormatException e) {
               System.err.println("Invalid steps option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            if (steps < 0)
            {
               System.err.println("Invalid steps option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            continue;
         }
         if (args[i].equals("-stepGameOfLife"))
         {
            stepGameOfLife = true;
            continue;
         }
         if (args[i].equals("-gridSize"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid gridSize option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            try
            {
               width = Integer.parseInt(args[i]);
            }
            catch (NumberFormatException e) {
               System.err.println("Invalid gridSize width option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            if (width < 2)
            {
               System.err.println("Invalid gridSize width option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid gridSize option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            try
            {
               height = Integer.parseInt(args[i]);
            }
            catch (NumberFormatException e) {
               System.err.println("Invalid gridSize height option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            if (height < 2)
            {
               System.err.println("Invalid gridSize height option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            continue;
         }
         if (args[i].equals("-live_cell_probability"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid live_cell_probability option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            try
            {
               live_cell_prob = Float.parseFloat(args[i]);
            }
            catch (NumberFormatException e) {
               System.err.println("Invalid live_cell_probability option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            if ((live_cell_prob < 0.0f) || (live_cell_prob > 1.0f))
            {
               System.err.println("Invalid live_cell_probability option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            continue;
         }
         if (args[i].equals("-loadCells"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid loadCells option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            if (cellsFilename == null)
            {
               cellsFilename = args[i];
            }
            else
            {
               System.err.println("Duplicate loadCells option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            continue;
         }
         if (args[i].equals("-foragerMoxen"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid foragerMoxen option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            try
            {
               numForagers = Integer.parseInt(args[i]);
            }
            catch (NumberFormatException e) {
               System.err.println("Invalid foragerMoxen option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            if (numForagers <= 0)
            {
               System.err.println("Invalid foragerMoxen option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            continue;
         }
         if (args[i].equals("-predatorMoxen"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid predatorMoxen option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            try
            {
               numPredators = Integer.parseInt(args[i]);
            }
            catch (NumberFormatException e) {
               System.err.println("Invalid predatorMoxen option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            if (numPredators <= 0)
            {
               System.err.println("Invalid predatorMoxen option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            continue;
         }
         if (args[i].equals("-random_seed"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid random_seed option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            try
            {
               random_seed = Integer.parseInt(args[i]);
            }
            catch (NumberFormatException e) {
               System.err.println("Invalid random_seed option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            continue;
         }
         if (args[i].equals("-load"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid load option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            if (loadfile == null)
            {
               loadfile = args[i];
            }
            else
            {
               System.err.println("Duplicate load option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            continue;
         }
         if (args[i].equals("-save"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid save option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            if (savefile == null)
            {
               savefile = args[i];
            }
            else
            {
               System.err.println("Duplicate save option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            continue;
         }
         if (args[i].equals("-dashboard"))
         {
            dashboard = true;
            continue;
         }
         System.err.println(MoxWorld.Usage);
         System.exit(1);
      }

      // Check options.
      if (steps < 0)
      {
         System.err.println(MoxWorld.Usage);
         System.exit(1);
      }
      if (loadfile == null)
      {
         if (cellsFilename == null)
         {
            if ((width == -1) || (height == -1) || (live_cell_prob < 0.0f))
            {
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
         }
         else
         {
            if ((width != -1) || (height != -1) || (live_cell_prob >= 0.0f))
            {
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
         }
      }
      else
      {
         if ((numForagers != 0) || (numPredators != 0) ||
             (width != -1) || (height != -1) ||
             (live_cell_prob >= 0.0f) || (cellsFilename != null))
         {
            System.err.println(MoxWorld.Usage);
            System.exit(1);
         }
      }

      // Set look and feel.
      try {
         UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
      }
      catch (Exception e)
      {
         System.err.println("Warning: cannot set look and feel");
      }

      // Create world.
      MoxWorld moxWorld = new MoxWorld();
      if (loadfile != null)
      {
         try
         {
            moxWorld.Load(loadfile);
         }
         catch (Exception e)
         {
            System.err.println("Cannot load from file " + loadfile + ": " + e.getMessage());
            System.exit(1);
         }
      }
      else
      {
         try
         {
            if (cellsFilename == null)
            {
               moxWorld.InitCells(width, height, live_cell_prob, random_seed);
            }
            else
            {
               moxWorld.loadCells(cellsFilename);
            }
            moxWorld.createMoxen(numForagers, numPredators, random_seed);
         }
         catch (Exception e)
         {
            System.err.println("Cannot initialize: " + e.getMessage());
            System.exit(1);
         }
      }

      // Create dashboard?
      if (dashboard)
      {
         moxWorld.createdashboard();
      }

      // Run.
      moxWorld.Run(steps, stepGameOfLife);

      // Save?
      if (savefile != null)
      {
         try
         {
            moxWorld.Store(savefile);
         }
         catch (Exception e)
         {
            System.err.println("Cannot save to file " + savefile + ": " + e.getMessage());
         }
      }
      System.exit(0);
   }
}
