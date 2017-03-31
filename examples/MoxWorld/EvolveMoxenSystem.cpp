/*
 * Evolve a system of mox foragers and predators
 * by mutating and recombining parameters.
 */

import java.util.*;
import java.io.*;
import mona.NativeFileDescriptor;

public class EvolveMoxenSystem
{
   // Usage.
   public static final String Usage =
      "Usage:\n" +
      "  New run:\n" +
      "    java EvolveMoxenSystem\n" +
      "      -generations <evolution generations>\n" +
      "      -steps <moxen steps>\n" +
      "     [-stepGameOfLife]\n" +
      "      -moxPopulations \"" + EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY.GetName() +
      "\" | \"" + EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS.GetName() + "\"\n" +
      "      -loadCells <file name>\n" +
      "      -output <evolution output file name>\n" +
      "     [-mutation_rate <mutation rate>]\n" +
      "     [-random_mutation_rate <random mutation rate>]\n" +
      "     [-maxSensorRange <maximum sensor range>]\n" +
      "     [-random_seed <random seed> (for new run)]\n" +
      "     [-logfile <log file name>]\n" +
      "     [-dashboard (run with mox world dashboard)]\n" +
      "  Resume run:\n" +
      "    java EvolveMoxenSystem\n" +
      "      -generations <evolution generations>\n" +
      "      -steps <moxen steps>\n" +
      "     [-stepGameOfLife]\n" +
      "      -input <evolution input file name>\n" +
      "      -output <evolution output file name>\n" +
      "     [-mutation_rate <mutation rate>]\n" +
      "     [-random_mutation_rate <random mutation rate>]\n" +
      "     [-maxSensorRange <maximum sensor range>]\n" +
      "     [-random_seed <random seed> (for new run)]\n" +
      "     [-logfile <log file name>]\n" +
      "     [-dashboard (run with mox world dashboard)]\n" +
      "  Extract into mox_world_forager|predator_<mox id>.mw files:\n" +
      "    java EvolveMoxenSystem\n" +
      "      -extract\n" +
      "      -input <evolution input file name>\n" +
      "  Print evolved property values:\n" +
      "    java EvolveMoxenSystem\n" +
      "      -print\n" +
      "      -input <evolution input file name>";

   // generation_count.
   int generation;
   int generation_count;

   // Steps.
   int Steps;

   // Step Game of Life?
   bool StepGameOfLife;

   // Mox populations.
   EvolveCommon.MOX_POPULATIONS MoxPopulations;

   // File names.
   String      cells_file_name;
   String      input_file_name;
   String      output_file_name;
   String      log_file_name;
   PrintWriter log_writer;

   // Random numbers.
   Random randomizer;

   // Run with dashboard.
   bool dashboard;

   // Extract moxen files.
   bool Extract;

   // Print moxen property values.
   bool PrintProperties;

   // Maximum mox cycle time.
   long MaxMoxCycleTime;

   // Mox world.
   MoxWorld moxWorld;

   // Populations.
   EvolveCommon.Member[] ForagerPopulation;
   EvolveCommon.Member[] PredatorPopulation;

   // Constructor.
   public EvolveMoxenSystem(String[] args)
   {
      int i;

      // Get options.
      generation     = 0;
      generation_count    = -1;
      Steps          = -1;
      StepGameOfLife = false;
      bool gotStepGameOfLife = false;
      MoxPopulations = EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY;
      bool gotMoxPopulations = false;
      cells_file_name = null;
      input_file_name = output_file_name = log_file_name = null;
      log_writer     = null;
      bool has_mutation_rate       = false;
      bool has_random_mutation_rate = false;
      bool has_max_sensor_range     = false;
      bool has_random_seed         = false;
      Extract         = false;
      PrintProperties = false;
      dashboard       = false;
      for (i = 0; i < args.length; i++)
      {
         if (args[i].equals("-generations"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println(Usage);
               System.exit(1);
            }
            generation_count = Integer.parseInt(args[i]);
            if (generation_count < 0)
            {
               System.err.println(Usage);
               System.exit(1);
            }
            continue;
         }

         if (args[i].equals("-steps"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println(Usage);
               System.exit(1);
            }
            Steps = Integer.parseInt(args[i]);
            if (Steps < 0)
            {
               System.err.println(Usage);
               System.exit(1);
            }
            continue;
         }

         if (args[i].equals("-stepGameOfLife"))
         {
            StepGameOfLife    = true;
            gotStepGameOfLife = true;
            continue;
         }

         if (args[i].equals("-moxPopulations"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println(Usage);
               System.exit(1);
            }
            if (args[i].equals(EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY.GetName()))
            {
               MoxPopulations = EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY;
            }
            else if (args[i].equals(EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS.GetName()))
            {
               MoxPopulations = EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS;
            }
            else
            {
               System.err.println(Usage);
               System.exit(1);
            }
            gotMoxPopulations = true;
            continue;
         }

         if (args[i].equals("-loadCells"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println(Usage);
               System.exit(1);
            }
            if (cells_file_name == null)
            {
               cells_file_name = new String(args[i]);
            }
            else
            {
               System.err.println(Usage);
               System.exit(1);
            }
            continue;
         }

         if (args[i].equals("-input"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println(Usage);
               System.exit(1);
            }
            input_file_name = new String(args[i]);
            continue;
         }

         if (args[i].equals("-output"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println(Usage);
               System.exit(1);
            }
            output_file_name = new String(args[i]);
            continue;
         }

         if (args[i].equals("-mutation_rate"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println(Usage);
               System.exit(1);
            }
            EvolveCommon.mutation_rate = Double.parseDouble(args[i]);
            if ((EvolveCommon.mutation_rate < 0.0) || (EvolveCommon.mutation_rate > 1.0))
            {
               System.err.println(Usage);
               System.exit(1);
            }
            has_mutation_rate = true;
            continue;
         }

         if (args[i].equals("-random_mutation_rate"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println(Usage);
               System.exit(1);
            }
            EvolveCommon.Randommutation_rate = Double.parseDouble(args[i]);
            if ((EvolveCommon.Randommutation_rate < 0.0) || (EvolveCommon.Randommutation_rate > 1.0))
            {
               System.err.println(Usage);
               System.exit(1);
            }
            has_random_mutation_rate = true;
            continue;
         }

         if (args[i].equals("-maxSensorRange"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println(Usage);
               System.exit(1);
            }
            EvolveCommon.MaxSensorRange = Float.parseFloat(args[i]);
            if (EvolveCommon.MaxSensorRange < 0.0f)
            {
               System.err.println(Usage);
               System.exit(1);
            }
            has_max_sensor_range = true;
            continue;
         }

         if (args[i].equals("-random_seed"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println(Usage);
               System.exit(1);
            }
            EvolveCommon.RandomSeed = Integer.parseInt(args[i]);
            has_random_seed           = true;
            continue;
         }

         if (args[i].equals("-logfile"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println(Usage);
               System.exit(1);
            }
            log_file_name = new String(args[i]);
            continue;
         }

         if (args[i].equals("-dashboard"))
         {
            dashboard = true;
            continue;
         }

         if (args[i].equals("-extract"))
         {
            Extract = true;
            continue;
         }

         if (args[i].equals("-print"))
         {
            PrintProperties = true;
            continue;
         }

         System.err.println(Usage);
         System.exit(1);
      }

      // Extract moxen files or print properties?
      if (Extract || PrintProperties)
      {
         if ((generation_count != -1) || (Steps != -1) || gotStepGameOfLife ||
             gotMoxPopulations || (input_file_name == null) ||
             (output_file_name != null) || (cells_file_name != null) ||
             (log_file_name != null) || has_mutation_rate || has_random_mutation_rate ||
             has_max_sensor_range || has_random_seed || dashboard)
         {
            System.err.println(Usage);
            System.exit(1);
         }
         if (Extract && PrintProperties)
         {
            System.err.println(Usage);
            System.exit(1);
         }
      }
      else
      {
         if (generation_count == -1)
         {
            System.err.println("generation_count option required");
            System.err.println(Usage);
            System.exit(1);
         }

         if (Steps == -1)
         {
            System.err.println("Steps option required");
            System.err.println(Usage);
            System.exit(1);
         }

         if (output_file_name == null)
         {
            System.err.println("Output file required");
            System.err.println(Usage);
            System.exit(1);
         }

         if (input_file_name != null)
         {
            if (gotMoxPopulations || (cells_file_name != null))
            {
               System.err.println(Usage);
               System.exit(1);
            }
         }
         else
         {
            if (!gotMoxPopulations || (cells_file_name == null))
            {
               System.err.println(Usage);
               System.exit(1);
            }
         }
      }

      // Set maximum sensor range.
      ForagerMox.MAX_SENSOR_RANGE  = EvolveCommon.MaxSensorRange;
      PredatorMox.MAX_SENSOR_RANGE = EvolveCommon.MaxSensorRange;

      // Seed random numbers.
      randomizer = new Random(EvolveCommon.RandomSeed);

      // Open log file?
      if (log_file_name != null)
      {
         try
         {
            log_writer = new PrintWriter(new FileOutputStream(new File(log_file_name)));
         }
         catch (Exception e) {
            System.err.println("Cannot open log file " + log_file_name +
                               ":" + e.getMessage());
            System.exit(1);
         }
      }
   }


   // Start evolve.
   public void start()
   {
      // Initialize populations?
      if (input_file_name == null)
      {
         Init();
      }
      else
      {
         // Load populations.
         Load();
      }

      // Log run.
      log("Initializing evolve:");
      log("  Options:");
      log("    generations=" + generation_count);
      log("    steps=" + Steps);
      if (StepGameOfLife)
      {
         log("    stepGameOfLife=true");
      }
      else
      {
         log("    stepGameOfLife=false");
      }
      if (input_file_name == null)
      {
         if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY)
         {
            log("    MoxPopulations=" + EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY.GetName());
         }
         else
         {
            log("    MoxPopulations=" + EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS.GetName());
         }
         log("    loadCells=" + cells_file_name);
      }
      else
      {
         log("    input=" + input_file_name);
      }
      log("    output=" + output_file_name);
      log("    mutation_rate=" + EvolveCommon.mutation_rate);
      log("    Randommutation_rate=" + EvolveCommon.Randommutation_rate);
      log("    MaxSensorRange=" + EvolveCommon.MaxSensorRange);
      log("    RandomSeed=" + EvolveCommon.RandomSeed);
      log("  Parameters:");
      log("    FORAGER_FIT_POPULATION_SIZE=" + EvolveCommon.FORAGER_FIT_POPULATION_SIZE);
      log("    FORAGER_NUM_MUTANTS=" + EvolveCommon.FORAGER_NUM_MUTANTS);
      log("    FORAGER_NUM_OFFSPRING=" + EvolveCommon.FORAGER_NUM_OFFSPRING);
      if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS)
      {
         log("    PREDATOR_FIT_POPULATION_SIZE=" + EvolveCommon.PREDATOR_FIT_POPULATION_SIZE);
         log("    PREDATOR_NUM_MUTANTS=" + EvolveCommon.PREDATOR_NUM_MUTANTS);
         log("    PREDATOR_NUM_OFFSPRING=" + EvolveCommon.PREDATOR_NUM_OFFSPRING);
      }

      // Extract moxen files?
      if (Extract)
      {
         extract();
         return;
      }

      // Print moxen properties?
      if (PrintProperties)
      {
         printProperties();
         return;
      }

      // Set maximum mox cycle time according to current running conditions.
      MaxMoxCycleTime = Mox.getMaxCycleTime();

      // Evolution loop.
      log("Begin evolve:");
      for (generation_count += generation; generation < generation_count; generation++)
      {
         log("generation=" + generation);

         evolve(generation);

         // Save populations?
         if ((generation % EvolveCommon.SAVE_FREQUENCY) == 0)
         {
            Store(generation);
         }
      }

      // Save populations.
      Store(generation - 1);

      log("End evolve");
   }


   // Initialize evolution.
   void Init()
   {
      int i;

      try
      {
         moxWorld = new MoxWorld();
         moxWorld.loadCells(cells_file_name);
      }
      catch (Exception e) {
         System.err.println("Cannot load cells file " +
                            cells_file_name +
                            ":" + e.getMessage());
         System.exit(1);
      }

      ForagerPopulation = new EvolveCommon.Member[EvolveCommon.FORAGER_POPULATION_SIZE];
      for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++)
      {
         if (i == 0)
         {
            forager_population[i] =
               new EvolveCommon.Member(Mox.SPECIES.FORAGER.GetValue(), 0,
                                       0, 0, Mox.DIRECTION.NORTH.GetValue(), randomizer);
         }
         else
         {
            // Mutate parameters.
            forager_population[i] =
               new EvolveCommon.Member(forager_population[0], 0, randomizer);
         }
      }
      if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS)
      {
         PredatorPopulation = new EvolveCommon.Member[EvolveCommon.PREDATOR_POPULATION_SIZE];
         for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++)
         {
            if (i == 0)
            {
               Predatorpopulation[i] =
                  new EvolveCommon.Member(Mox.SPECIES.PREDATOR.GetValue(), 0,
                                          moxWorld.GetWidth() - 1,
                                          moxWorld.GetHeight() - 1,
                                          Mox.DIRECTION.SOUTH.GetValue(), randomizer);
            }
            else
            {
               // Mutate parameters.
               Predatorpopulation[i] =
                  new EvolveCommon.Member(Predatorpopulation[0], 0, randomizer);
            }
         }
      }
   }


   // Load evolution.
   void Load()
   {
      int                  i;
      FileInputStream      input  = null;
      NativeFileDescriptor fd     = null;
      DataInputStream      reader = null;

      // Open the file.
      try
      {
         input  = new FileInputStream(new File(input_file_name));
         reader = new DataInputStream(input);
         fd     = new NativeFileDescriptor(input_file_name, "r");
         fd.open();
      }
      catch (Exception e) {
         System.err.println("Cannot open input file " + input_file_name +
                            ":" + e.getMessage());
      }

      try
      {
         generation = Utility.loadInt(reader);
         generation++;
      }
      catch (Exception e) {
         System.err.println("Cannot load from file " + input_file_name +
                            ":" + e.getMessage());
         System.exit(1);
      }

      // Load mox world.
      try
      {
         moxWorld = new MoxWorld();
         moxWorld.Load(input, fd);
      }
      catch (Exception e) {
         System.err.println("Cannot load mox world from file " + input_file_name +
                            ":" + e.getMessage());
         System.exit(1);
      }

      // Load populations.
      try
      {
         if (Utility.loadInt(reader) == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY.GetValue())
         {
            MoxPopulations = EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY;
         }
         else
         {
            MoxPopulations = EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS;
         }
         ForagerPopulation = new EvolveCommon.Member[EvolveCommon.FORAGER_POPULATION_SIZE];
         for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++)
         {
            forager_population[i] =
               new EvolveCommon.Member(Mox.SPECIES.FORAGER.GetValue(), 0, 0, 0, 0, randomizer);
            forager_population[i].Load(input, fd);
         }
         if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS)
         {
            PredatorPopulation = new EvolveCommon.Member[EvolveCommon.PREDATOR_POPULATION_SIZE];
            for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++)
            {
               Predatorpopulation[i] =
                  new EvolveCommon.Member(Mox.SPECIES.PREDATOR.GetValue(), 0, 0, 0, 0, randomizer);
               Predatorpopulation[i].Load(input, fd);
            }
         }

         input.close();
         fd.close();
      }
      catch (Exception e) {
         System.err.println("Cannot load populations from file " + input_file_name +
                            ":" + e.getMessage());
         System.exit(1);
      }
   }


   // Save evolution.
   void Store(int generation)
   {
      int                  i;
      FileOutputStream     output = null;
      NativeFileDescriptor fd     = null;
      PrintWriter          writer = null;

      try
      {
         output = new FileOutputStream(new File(output_file_name));
         writer = new PrintWriter(output);
         fd     = new NativeFileDescriptor(output_file_name, "w");
         fd.open();
      }
      catch (Exception e) {
         System.err.println("Cannot open output file " + output_file_name +
                            ":" + e.getMessage());
         System.exit(1);
      }

      try
      {
         Utility.saveInt(writer, generation);
         writer.flush();
      }
      catch (Exception e) {
         System.err.println("Cannot save to file " + output_file_name +
                            ":" + e.getMessage());
         System.exit(1);
      }

      // Save mox world.
      try
      {
         moxWorld.Store(output, fd);
      }
      catch (Exception e) {
         System.err.println("Cannot save mox world to file " + output_file_name +
                            ":" + e.getMessage());
         System.exit(1);
      }

      // Save populations.
      try
      {
         if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY)
         {
            Utility.saveInt(writer, EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY.GetValue());
         }
         else
         {
            Utility.saveInt(writer, EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS.GetValue());
         }
         writer.flush();
         for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++)
         {
            forager_population[i].Store(output, fd);
         }
         if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS)
         {
            for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++)
            {
               Predatorpopulation[i].Store(output, fd);
            }
         }

         output.close();
         fd.close();
      }
      catch (Exception e) {
         System.err.println("Cannot save populations to file " + output_file_name +
                            ":" + e.getMessage());
         System.exit(1);
      }
   }


   // Evolution generation.
   void evolve(int generation)
   {
      // Evaluate member fitness.
      Evaluate(generation);

      // Prune unfit members.
      Prune();

      // Create new members by mutation.
      Mutate();

      // Create new members by mating.
      DoMating();
   }


   // Evaluate member fitnesses.
   void Evaluate(int generation)
   {
      int i, step;
      int blue_food_need_idx, mox_food_need_idx;
      Mox mox;

      ArrayList<Mox> moxen;

      log("Evaluate:");

      // Prepare for evaluation.
      prepareEvaluation();

      // Set up mox world.
      moxen = new ArrayList<Mox>();
      populateMoxen(moxen);
      moxWorld.setMoxen(moxen);
      moxWorld.reset();

      for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++)
      {
         forager_population[i].fitness = 0.0;
      }
      if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS)
      {
         for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++)
         {
            Predatorpopulation[i].fitness = 0.0;
         }
      }
      blue_food_need_idx = ForagerMox.NEED_TYPE.BLUE_FOOD.GetValue();
      mox_food_need_idx  = PredatorMox.NEED_TYPE.MOX_FOOD.GetValue();

      // Step world.
      if (dashboard)
      {
         moxWorld.createdashboard();
         dashboard = moxWorld.updatedashboard(0, Steps);
      }
      for (step = 0; step < Steps; step++)
      {
         for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++)
         {
            mox = forager_population[i].mox;
            mox.startCycleTimeAccumulation();
         }
         if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS)
         {
            for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++)
            {
               mox = Predatorpopulation[i].mox;
               mox.startCycleTimeAccumulation();
            }
         }

         // Step the moxen.
         moxWorld.stepMoxen();

         // Update forager fitness.
         for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++)
         {
            mox = forager_population[i].mox;
            if (mox.isAlive)
            {
               // Increase fitness when food found.
               if (mox.GetNeed(blue_food_need_idx) == 0.0)
               {
                  forager_population[i].fitness += 1.0;
                  mox.SetNeed(blue_food_need_idx, ForagerMox.BLUE_FOOD_NEED_VALUE);
               }

               // Kill sluggish mox?
               if (mox.getCycleTimeAccumulator() > MaxMoxCycleTime)
               {
                  mox.isAlive = false;
               }
            }
         }

         // Update predator fitness.
         if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS)
         {
            for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++)
            {
               mox = Predatorpopulation[i].mox;
               if (mox.isAlive)
               {
                  // Increase fitness when food found.
                  if (mox.GetNeed(mox_food_need_idx) == 0.0)
                  {
                     Predatorpopulation[i].fitness += 1.0;
                     mox.SetNeed(mox_food_need_idx, PredatorMox.MOX_FOOD_NEED_VALUE);
                  }

                  // Kill sluggish mox?
                  if (mox.getCycleTimeAccumulator() > MaxMoxCycleTime)
                  {
                     mox.isAlive = false;
                  }
               }
            }
         }

         // Step the Game of Life.
         if (StepGameOfLife)
         {
            moxWorld.stepGameOfLife();
         }

         // Update dashboard?
         if (dashboard)
         {
            dashboard = moxWorld.updatedashboard(step + 1, Steps);
         }
      }
      moxWorld.destroydashboard();
      moxWorld.setMoxen(new ArrayList<Mox>());
      moxWorld.reset();

      log("  Foragers:");
      for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++)
      {
         log("    member=" + i + ", " + forager_population[i].getInfo());
      }
      if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS)
      {
         log("  Predators:");
         for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++)
         {
            log("    member=" + i + ", " + Predatorpopulation[i].getInfo());
         }
      }
   }


   // Populate moxen.
   void populateMoxen(ArrayList<Mox> moxen)
   {
      int i, x, y, direction;
      int blue_food_need_idx, mox_food_need_idx;
      Mox mox;

      blue_food_need_idx = ForagerMox.NEED_TYPE.BLUE_FOOD.GetValue();
      mox_food_need_idx  = PredatorMox.NEED_TYPE.MOX_FOOD.GetValue();

      for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++)
      {
         mox = forager_population[i].mox;
         mox.SetNeed(blue_food_need_idx, ForagerMox.BLUE_FOOD_NEED_VALUE);
         x         = randomizer.nextInt(moxWorld.GetWidth());
         y         = randomizer.nextInt(moxWorld.GetHeight());
         direction = randomizer.nextInt(4);
         mox.setSpacialProperties(x, y, direction);
         moxen.Add(mox);
      }

      if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS)
      {
         for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++)
         {
            mox = Predatorpopulation[i].mox;
            mox.SetNeed(mox_food_need_idx, PredatorMox.MOX_FOOD_NEED_VALUE);
            x         = randomizer.nextInt(moxWorld.GetWidth());
            y         = randomizer.nextInt(moxWorld.GetHeight());
            direction = randomizer.nextInt(4);
            mox.setSpacialProperties(x, y, direction);
            moxen.Add(mox);
         }
      }
   }


   // Prepare new moxen for evaluation by giving them
   // some experience before competing with existing moxen.
   void prepareEvaluation()
   {
      int i, step;
      int blue_food_need_idx, mox_food_need_idx;
      Mox mox;

      ArrayList<Mox> moxen;

      log("  Preparing new moxen");

      // Set up mox world.
      moxen = new ArrayList<Mox>();
      populateMoxen(moxen);
      moxWorld.setMoxen(moxen);
      moxWorld.reset();

      blue_food_need_idx = ForagerMox.NEED_TYPE.BLUE_FOOD.GetValue();
      mox_food_need_idx  = PredatorMox.NEED_TYPE.MOX_FOOD.GetValue();

      // Step world.
      for (step = 0; step < Steps; step++)
      {
         for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++)
         {
            mox = forager_population[i].mox;
            mox.startCycleTimeAccumulation();
         }
         if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS)
         {
            for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++)
            {
               mox = Predatorpopulation[i].mox;
               mox.startCycleTimeAccumulation();
            }
         }

         // Step the moxen.
         moxWorld.stepMoxen();

         for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++)
         {
            mox = forager_population[i].mox;
            if (mox.isAlive)
            {
               if (mox.getCycleTimeAccumulator() > MaxMoxCycleTime)
               {
                  mox.isAlive = false;
               }
               else
               {
                  if (mox.GetNeed(blue_food_need_idx) == 0.0)
                  {
                     mox.SetNeed(blue_food_need_idx, ForagerMox.BLUE_FOOD_NEED_VALUE);
                  }
               }
            }
         }

         if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS)
         {
            for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++)
            {
               mox = Predatorpopulation[i].mox;
               if (mox.isAlive)
               {
                  if (mox.getCycleTimeAccumulator() > MaxMoxCycleTime)
                  {
                     mox.isAlive = false;
                  }
                  else
                  {
                     if (mox.GetNeed(mox_food_need_idx) == 0.0)
                     {
                        mox.SetNeed(mox_food_need_idx, PredatorMox.MOX_FOOD_NEED_VALUE);
                     }
                  }
               }
            }
         }

         // Step the Game of Life.
         if (StepGameOfLife)
         {
            moxWorld.stepGameOfLife();
         }
      }
      moxWorld.setMoxen(new ArrayList<Mox>());
      moxWorld.reset();
      log("  Preparation completed");
   }


   // Prune unfit members.
   void Prune()
   {
      double max;
      int    i, j, m;

      EvolveCommon.Member member;

      log("Select:");
      log("  Foragers:");
      EvolveCommon.Member[] fitPopulation =
         new EvolveCommon.Member[EvolveCommon.FORAGER_FIT_POPULATION_SIZE];
      max = 0.0;
      for (i = 0; i < EvolveCommon.FORAGER_FIT_POPULATION_SIZE; i++)
      {
         m = -1;
         for (j = 0; j < EvolveCommon.FORAGER_POPULATION_SIZE; j++)
         {
            member = forager_population[j];
            if (member == null)
            {
               continue;
            }
            if ((m == -1) || (member.fitness > max))
            {
               m   = j;
               max = member.fitness;
            }
         }
         member = forager_population[m];
         forager_population[m] = null;
         fitpopulation[i]     = member;
         log("    " + member.getInfo());
      }
      for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++)
      {
         if (forager_population[i] != null)
         {
            forager_population[i].Clear();
            forager_population[i] = null;
         }
      }
      for (i = 0; i < EvolveCommon.FORAGER_FIT_POPULATION_SIZE; i++)
      {
         forager_population[i] = fitpopulation[i];
         fitpopulation[i]     = null;
      }

      if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY)
      {
         return;
      }

      log("  Predators:");
      fitPopulation =
         new EvolveCommon.Member[EvolveCommon.PREDATOR_FIT_POPULATION_SIZE];
      for (i = 0; i < EvolveCommon.PREDATOR_FIT_POPULATION_SIZE; i++)
      {
         m = -1;
         for (j = 0; j < EvolveCommon.PREDATOR_POPULATION_SIZE; j++)
         {
            member = Predatorpopulation[j];
            if (member == null)
            {
               continue;
            }
            if ((m == -1) || (member.fitness > max))
            {
               m   = j;
               max = member.fitness;
            }
         }
         member = Predatorpopulation[m];
         Predatorpopulation[m] = null;
         fitpopulation[i]      = member;
         log("    " + member.getInfo());
      }
      for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++)
      {
         if (Predatorpopulation[i] != null)
         {
            Predatorpopulation[i] = null;
         }
      }
      for (i = 0; i < EvolveCommon.PREDATOR_FIT_POPULATION_SIZE; i++)
      {
         Predatorpopulation[i] = fitpopulation[i];
      }
   }


   // Mutate members.
   void Mutate()
   {
      int i, j;

      EvolveCommon.Member member, mutant;

      log("Mutate:");
      log("  Foragers:");
      for (i = 0; i < EvolveCommon.FORAGER_NUM_MUTANTS; i++)
      {
         // Select a fit member to mutate.
         j      = randomizer.nextInt(EvolveCommon.FORAGER_FIT_POPULATION_SIZE);
         member = forager_population[j];

         // Create mutant member.
         mutant = new EvolveCommon.Member(member, member.generation + 1, randomizer);
         forager_population[EvolveCommon.FORAGER_FIT_POPULATION_SIZE + i] = mutant;
         log("    member=" + j + ", " + member.getInfo() +
             " -> member=" + (EvolveCommon.FORAGER_FIT_POPULATION_SIZE + i) +
             ", " + mutant.getInfo());
      }

      if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY)
      {
         return;
      }

      log("  Predators:");
      for (i = 0; i < EvolveCommon.PREDATOR_NUM_MUTANTS; i++)
      {
         // Select a fit member to mutate.
         j      = randomizer.nextInt(EvolveCommon.PREDATOR_FIT_POPULATION_SIZE);
         member = Predatorpopulation[j];

         // Create mutant member.
         mutant = new EvolveCommon.Member(member, member.generation + 1, randomizer);
         Predatorpopulation[EvolveCommon.PREDATOR_FIT_POPULATION_SIZE + i] = mutant;
         log("    member=" + j + ", " + member.getInfo() +
             " -> member=" + (EvolveCommon.PREDATOR_FIT_POPULATION_SIZE + i) +
             ", " + mutant.getInfo());
      }
   }


   // Produce offspring by melding parent parameters.
   void DoMating()
   {
      int i, j, k;

      EvolveCommon.Member member1, member2, offspring;

      log("Mate:");
      if (EvolveCommon.FORAGER_FIT_POPULATION_SIZE > 1)
      {
         log("  Foragers:");
         for (i = 0; i < EvolveCommon.FORAGER_NUM_OFFSPRING; i++)
         {
            // Select a pair of fit members to mate.
            j       = randomizer.nextInt(EvolveCommon.FORAGER_FIT_POPULATION_SIZE);
            member1 = forager_population[j];
            while ((k = randomizer.nextInt(EvolveCommon.FORAGER_FIT_POPULATION_SIZE)) == j) {}
            member2 = forager_population[k];

            // Create offspring.
            offspring = new EvolveCommon.Member(member1, member2,
                                                (member1.generation > member2.generation ?
                                                 member1.generation : member2.generation) + 1, randomizer);
            forager_population[EvolveCommon.FORAGER_FIT_POPULATION_SIZE +
                              EvolveCommon.FORAGER_NUM_MUTANTS + i] = offspring;
            log("    member=" + j + ", " + member1.getInfo() + " + member=" +
                k + ", " + member2.getInfo() +
                " -> member=" + (EvolveCommon.FORAGER_FIT_POPULATION_SIZE +
                                 EvolveCommon.FORAGER_NUM_MUTANTS + i) +
                ", " + offspring.getInfo());
         }
      }

      if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY)
      {
         return;
      }

      if (EvolveCommon.PREDATOR_FIT_POPULATION_SIZE > 1)
      {
         log("  Predators:");
         for (i = 0; i < EvolveCommon.PREDATOR_NUM_OFFSPRING; i++)
         {
            // Select a pair of fit members to mate.
            j       = randomizer.nextInt(EvolveCommon.PREDATOR_FIT_POPULATION_SIZE);
            member1 = Predatorpopulation[j];
            while ((k = randomizer.nextInt(EvolveCommon.PREDATOR_FIT_POPULATION_SIZE)) == j) {}
            member2 = Predatorpopulation[k];

            // Create offspring.
            offspring = new EvolveCommon.Member(member1, member2,
                                                (member1.generation > member2.generation ?
                                                 member1.generation : member2.generation) + 1, randomizer);
            Predatorpopulation[EvolveCommon.PREDATOR_FIT_POPULATION_SIZE +
                               EvolveCommon.PREDATOR_NUM_MUTANTS + i] = offspring;
            log("    member=" + j + ", " + member1.getInfo() + " + member=" +
                k + ", " + member2.getInfo() +
                " -> member=" + (EvolveCommon.PREDATOR_FIT_POPULATION_SIZE +
                                 EvolveCommon.PREDATOR_NUM_MUTANTS + i) +
                ", " + offspring.getInfo());
         }
      }
   }


   // Extract moxen files.
   void extract()
   {
      int    i;
      int    blue_food_need_idx, mox_food_need_idx;
      Mox    mox;
      String filename;

      // Extract foragers.
      blue_food_need_idx = ForagerMox.NEED_TYPE.BLUE_FOOD.GetValue();
      for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++)
      {
         // Set up mox world.
         mox = forager_population[i].mox;
         mox.SetNeed(blue_food_need_idx, ForagerMox.BLUE_FOOD_NEED_VALUE);
         ArrayList<Mox> moxen = new ArrayList<Mox>(1);
         moxen.Add(0, mox);

         // Save mox world.
         filename = "mox_world_forager_" + mox.id + ".mw";
         moxWorld.setMoxen(moxen);
         moxWorld.reset();
         try
         {
            moxWorld.Store(filename);
         }
         catch (Exception e) {
            System.err.println("Cannot save mox world to file " + filename +
                               ":" + e.getMessage());
            System.exit(1);
         }
      }

      if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY)
      {
         return;
      }

      // Extract predators.
      mox_food_need_idx = PredatorMox.NEED_TYPE.MOX_FOOD.GetValue();
      for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++)
      {
         // Set up mox world.
         mox = Predatorpopulation[i].mox;
         mox.SetNeed(mox_food_need_idx, PredatorMox.MOX_FOOD_NEED_VALUE);
         ArrayList<Mox> moxen = new ArrayList<Mox>(1);
         moxen.Add(0, mox);

         // Save mox world.
         filename = "mox_world_predator_" + mox.id + ".mw";
         moxWorld.setMoxen(moxen);
         moxWorld.reset();
         try
         {
            moxWorld.Store(filename);
         }
         catch (Exception e) {
            System.err.println("Cannot save mox world to file " + filename +
                               ":" + e.getMessage());
            System.exit(1);
         }
      }
   }


   // Print moxen properties.
   void printProperties()
   {
      int i;

      System.out.println("Moxen properties:");

      // Print foragers.
      System.out.println("=============================");
      System.out.println("Foragers:");
      for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++)
      {
         System.out.println("-----------------------------");
         forager_population[i].printProperties();
      }

      if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY)
      {
         return;
      }

      // Print predators.
      System.out.println("=============================");
      System.out.println("Predators:");
      for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++)
      {
         System.out.println("-----------------------------");
         Predatorpopulation[i].printProperties();
      }
   }


   // Logging.
   void log(String message)
   {
      if (log_writer != null)
      {
         log_writer.println(message);
         log_writer.flush();
      }
   }


   // Main.
   public static void main(String[] args)
   {
      EvolveMoxenSystem evolveMoxenSystem = new EvolveMoxenSystem(args);

      evolveMoxenSystem.start();
      System.exit(0);
   }
}
