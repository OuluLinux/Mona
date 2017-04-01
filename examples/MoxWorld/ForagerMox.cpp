/*
 * Forager mox.
 */

import java.util.*;
import java.awt.Color;
import mona.Mona;

public class ForagerMox extends Mox
{
   // Color.
   public static final Color FORAGER_COLOR       = Color.BLACK;
   public static final int   FORAGER_COLOR_VALUE = 9;

   // Maximum sensor range (-1 == infinite).
   public static double MAX_SENSOR_RANGE = -1.0;

   // Need types.
   public enum NEED_TYPE
   {
      BLUE_FOOD(0),
      NUM_NEEDS(1);

      private int value;

      NEED_TYPE(int value)
      {
         this.value = value;
      }

      public int GetValue()
      {
         return (value);
      }
   }

   // Need and goal values.
   public static              final double BLUE_FOOD_NEED_VALUE = 1.0;
   public static final double BLUE_FOOD_GOAL_VALUE = 1.0;
   public static final String BLUE_FOOD_NEED_NAME  = "blue food";

   // Constructors.
   public ForagerMox()
   {
      super();
      setProperties(-1, SPECIES.FORAGER.GetValue(),
                    0, 0, DIRECTION.NORTH.GetValue(), true);

      // Create mona.
      createMona(DEFAULT_RANDOM_SEED);

      // Configure forager-specific needs and goals.
      configureNeedsAndGoals();
   }


   public ForagerMox(int id, int x, int y, int direction, int random_seed)
   {
      mutex = new Object();
      setProperties(id, SPECIES.FORAGER.GetValue(),
                    x, y, direction, true);

      // Create mona.
      createMona(random_seed);

      // Configure forager-specific needs and goals.
      configureNeedsAndGoals();
   }


   // Construct with mona parameters.
   public ForagerMox(int id, int x, int y, int direction,
                     Vector<String> monaParmKeys, Vector<Object> monaParmVals)
   {
      mutex = new Object();
      setProperties(id, SPECIES.FORAGER.GetValue(),
                    x, y, direction, true);

      // Create mona.
      createMona(monaParmKeys, monaParmVals);

      // Configure forager-specific needs and goals.
      configureNeedsAndGoals();
   }


   // Create mona.
   void createMona(int random_seed)
   {
      synchronized (mutex)
      {
         mona = new Mona(SENSOR_CONFIG.NUM_SENSORS.GetValue(),
                         RESPONSE_TYPE.NUM_RESPONSES.GetValue(),
                         NEED_TYPE.NUM_NEEDS.GetValue(), random_seed);
         boolean[] sensorMask = new boolean[SENSOR_CONFIG.NUM_SENSORS.GetValue()];
         for (int i = 0; i < SENSOR_CONFIG.NUM_SENSORS.GetValue(); i++)
         {
            sensorMask[i] = true;
         }
         mona.AddSensorMode(sensorMask, SENSOR_RESOLUTION);
         for (int i = 0; i < SENSOR_CONFIG.NUM_SENSORS.GetValue(); i++)
         {
            if (i < SENSOR_CONFIG.NUM_RANGE_SENSORS.GetValue())
            {
               sensorMask[i] = true;
            }
            else
            {
               sensorMask[i] = false;
            }
         }
         mona.AddSensorMode(sensorMask, SENSOR_RESOLUTION);
         for (int i = 0; i < SENSOR_CONFIG.NUM_SENSORS.GetValue(); i++)
         {
            if (i < SENSOR_CONFIG.NUM_RANGE_SENSORS.GetValue())
            {
               sensorMask[i] = false;
            }
            else
            {
               sensorMask[i] = true;
            }
         }
         mona.AddSensorMode(sensorMask, SENSOR_RESOLUTION);
      }
   }


   // Configure forager-specific needs and goals.
   void configureNeedsAndGoals()
   {
      synchronized (mutex)
      {
         needValues = new double[NEED_TYPE.NUM_NEEDS.GetValue()];
         needValues[NEED_TYPE.BLUE_FOOD.GetValue()] = BLUE_FOOD_NEED_VALUE;
         goal_values = new double[NEED_TYPE.NUM_NEEDS.GetValue()];
         goal_values[NEED_TYPE.BLUE_FOOD.GetValue()] = BLUE_FOOD_GOAL_VALUE;
         needNames = new String[NEED_TYPE.NUM_NEEDS.GetValue()];
         needNames[NEED_TYPE.BLUE_FOOD.GetValue()] = BLUE_FOOD_NEED_NAME;
         mona.SetNeed(NEED_TYPE.BLUE_FOOD.GetValue(), BLUE_FOOD_NEED_VALUE);
         Vector<double> sensors = new float[SENSOR_CONFIG.NUM_SENSORS.GetValue()];
         sensors[SENSOR_CONFIG.RANGE_SENSOR_INDEX.GetValue()]     = 0;
         sensors[SENSOR_CONFIG.COLOR_HUE_SENSOR_INDEX.GetValue()] =
            GameOfLife.BLUE_CELL_COLOR_VALUE;
         sensors[SENSOR_CONFIG.COLOR_INTENSITY_SENSOR_INDEX.GetValue()] = 1.0;
         mona.AddGoal(NEED_TYPE.BLUE_FOOD.GetValue(), sensors,
                      SENSOR_CONFIG.BASIC_SENSOR_MODE.GetValue(),
                      RESPONSE_TYPE.NULL_RESPONSE.GetValue(),
                      BLUE_FOOD_GOAL_VALUE);
      }
   }


   // Reset state.
   void reset()
   {
      super.reset();
      synchronized (mutex)
      {
         mona.SetNeed(NEED_TYPE.BLUE_FOOD.GetValue(), BLUE_FOOD_NEED_VALUE);
      }
   }
}
