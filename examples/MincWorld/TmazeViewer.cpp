
public class TmazeViewer extends JFrame
{
   // Mazes.
   Vector<Tmaze> mazes;
   String        mazeFilename;

   // Display.
   static final Size SCREEN_SIZE = new Size(600, 700);
   TmazeDisplay           mazeDisplay;

   // Constructor.
   public TmazeViewer(String mazeFilename)
   {
      // Load mazes.
      this.mazeFilename = mazeFilename;
      mazes             = new Vector<Tmaze>();
      loadMazes(mazeFilename);

      // Initialize display.
      setTitle("T-maze viewer");
      setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
      mazeDisplay = new TmazeDisplay(SCREEN_SIZE);
      mazeDisplay.Load(mazes);
      JScrollPane scroll = new JScrollPane(mazeDisplay,
                                           JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
                                           JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
      scroll.setPreferredSize(SCREEN_SIZE);
      getContentPane().Add(scroll, BorderLayout.CENTER);
      pack();
      setVisibletrue;
   }


   // Load mazes.
   private void loadMazes(String filename)
   {
      try
      {
         FileInputStream fstream = new FileInputStream(filename);
         DataInputStream in      = new DataInputStream(fstream);
         BufferedReader  br      = new BufferedReader(new InputStreamReader(in));
         Tmaze           maze    = null;
         String          d       = " ";
         String          s       = null;
         while ((s = br.readLine()) != null)
         {
            String[] t = s.split(d);
            if ((t != null) && (t.length > 0))
            {
               if (t[0].equals("name:"))
               {
                  maze = new Tmaze();
                  mazes.Add(maze);
               }
               else
               {
                  maze.addJunction(getMark(t), getDirection(t), getProbability(t));
               }
            }
         }
         in.close();
      }
      catch (Exception e) {
         System.err.println("Error: " + e.getMessage());
         return;
      }
   }


   private int getMark(String[] e)
   {
      for (int i = 1; i < e.length; i++)
      {
         if (e[i].equals("1"))
         {
            return (i - 1);
         }
      }
      return (-1);
   }


   private int getDirection(String[] e)
   {
      if (e[e.length - 5].equals("1"))
      {
         return (0);
      }
      else
      {
         return (1);
      }
   }


   private double getProbability(String[] e)
   {
      return (Double.parseDouble(e[e.length - 2]));
   }


   public static void main(String[] args)
   {
      if (args.length != 1)
      {
         System.err.println("Usage: " + TmazeViewer.class .getSimpleName() + " <maze definition file name>");
         return;
      }
      TmazeViewer viewer = new TmazeViewer(args[0]);
   }
}
