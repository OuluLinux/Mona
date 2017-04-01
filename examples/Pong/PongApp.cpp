
class Pong
{
   // Sizes.
   static int DISPLAY_SIZE    = 500;
   static int CONTROLS_HEIGHT = 75;

   // Play state.
   bool playing = false;

   // Ball speed, given in update delay time (ms).
   static int MIN_DELAY = 1;
   static int MAX_DELAY = 50;
   int        delay;

   // Ball.
   class Ball
   {
      final static double RADIUS_SCALE = .05f;
      double              radius; // Radius.
      double              x, y;   // Position.
      double              dx, dy; // Velocity.

      // Constructor.
      public Ball()
      {
         double a;

         radius = (float)DISPLAY_SIZE * RADIUS_SCALE;
         x      = y = (float)DISPLAY_SIZE / 2.0;
         a      = (float)Math.random() * 360.0;
         dx     = (float)Math.cos(toRadians(a)) * 10.0;
         dy     = (float)Math.sin(toRadians(a)) * 10.0;
      }


      // Convert degrees to radians.
      double toRadians(double angle)
      {
         return (angle * ((float)Math.PI / 180.0f));
      }


      // Move ball.
      void move()
      {
         if ((x - radius + dx) <= 0.0f)
         {
            if (bounceSound != null) { bounceSound.play(); }
            dx = -dx;
         }
         if ((x + radius + dx) >= (float)DISPLAY_SIZE)
         {
            playing = false;
            display.setMessage("Game over");
         }
         if ((y - radius + dy) <= 0.0f)
         {
            if (bounceSound != null) { bounceSound.play(); }
            dy = -dy;
         }
         if ((y + radius + dy) >= (float)DISPLAY_SIZE)
         {
            if (bounceSound != null) { bounceSound.play(); }
            dy = -dy;
         }
         double l2 = paddle.length / 2.0;
         if (((x + radius + dx) >= ((float)DISPLAY_SIZE - paddle.width)) &&
             (y <= (paddle.position + l2)) && (y >= (paddle.position - l2)))
         {
            if (bounceSound != null) { bounceSound.play(); }
            dx = -dx;
         }
         x += dx;
         y += dy;
      }
   }

   // Paddle.
   class Paddle
   {
      final static double WIDTH_SCALE  = 0.025f;
      final static double LENGTH_SCALE = .2f;
      double              position;         // Position.
      double              width, length;    // Size.

      // Constructor.
      public Paddle()
      {
         width    = (float)DISPLAY_SIZE * WIDTH_SCALE;
         length   = (float)DISPLAY_SIZE * LENGTH_SCALE;
         position = (float)DISPLAY_SIZE / 2.0;
      }


      // Move paddle based on mouse Y position.
      void move(int mouseY)
      {
         double l2 = length / 2.0;

         position = (float)(DISPLAY_SIZE - mouseY);
         if ((position - l2) < 0.0f) { position = l2; }
         if ((position + l2) > (float)DISPLAY_SIZE)
         {
            position = (float)DISPLAY_SIZE - l2;
         }
      }
   }

   // Ball and Paddle.
   Ball   ball;
   Thread ballThread;
   Paddle paddle;

   // Bounce sound.
   final static String BOUNCE_SOUND_FILE = "click.au";
   AudioClip           bounceSound;

   // Game display.
   class Display extends Canvas implements Runnable
   {
      // Display update frequency (ms).
      static final int DISPLAY_DELAY = 50;

      // Buffered display.
      Graphics  graphics;
      Image     image;
      Graphics  imageGraphics;
      Graphics  clippedImageGraphics;
      Ellipse2D imageClip;

      // Message and font.
      String      message;
      Font        font = new Font("Helvetica", Font.BOLD, 12);
      FontMetrics fontMetrics;
      int         fontAscent;
      int         fontWidth;
      int         fontHeight;

      // Display thread.
      private Thread thread;

      // Constructor.
      public Display()
      {
         // Set size.
         setSize(DISPLAY_SIZE, DISPLAY_SIZE);

         // Add mouse listeners for paddle movement.
         addMouseListener(new displayMouseListener());
         addMouseMotionListener(new displayMouseMotionListener());
      }


      // Mouse press.
      class displayMouseListener extends MouseAdapter
      {
         public void mousePressed(MouseEvent evt)
         {
            paddle.move(evt.getY());
         }
      }

      // Mouse motion.
      class displayMouseMotionListener extends MouseMotionAdapter
      {
         public void mouseDragged(MouseEvent evt)
         {
            paddle.move(evt.getY());
         }
      }

      // Start.
      public void start()
      {
         // Set graphics and font.
         graphics             = getGraphics();
         image                = createImage(DISPLAY_SIZE, DISPLAY_SIZE);
         imageGraphics        = image.getGraphics();
         clippedImageGraphics = image.getGraphics();
         imageClip            =
            new Ellipse2D.Double(0.0, 0.0,
                                 (double)(ball.radius * 2), (double)(ball.radius * 2));
         graphics.setFont(font);
         fontMetrics = graphics.getFontMetrics();
         fontAscent  = fontMetrics.getMaxAscent();
         fontWidth   = fontMetrics.getMaxAdvance();
         fontHeight  = fontMetrics.GetHeight();

         // Start thread.
         thread = new Thread(this);
         thread.setPriority(Thread.MIN_PRIORITY);
         thread.start();
      }


      // Run.
      public void Run()
      {
         while (Thread.currentThread() == thread)
         {
            // Update display.
            Update();

            try
            {
               Sleep(DISPLAY_DELAY);
            }
            catch (InterruptedException e) { break; }
         }
      }


      // Update display.
      synchronized void Update()
      {
         int i;

         Playback.Step step;

         // Clear display.
         imageGraphics.setColor(Color.black);
         imageGraphics.fillRect(0, 0, DISPLAY_SIZE, DISPLAY_SIZE);

         // Playback?
         if (playback != null)
         {
            step = playback.currentStep();
            if (step != null)
            {
               ball.x          = step.ballX * DISPLAY_SIZE;
               ball.y          = step.ballY * DISPLAY_SIZE;
               paddle.position = step.paddleY * DISPLAY_SIZE;

               // Draw player position.
               int d  = DISPLAY_SIZE / playback.dimension;
               int d2 = d / 2;
               int x  = (int)(step.playerX * (float)DISPLAY_SIZE) - d2;
               int y  = (int)((((float)DISPLAY_SIZE) - (step.playerY * (float)DISPLAY_SIZE)) - d2);
               imageGraphics.setColor(Color.yellow);
               imageGraphics.drawRect(x, y, d, d);
            }
         }

         // Draw ball.
         if (playing)
         {
            imageGraphics.setColor(Color.white);
            i = (int)(ball.radius * 2.0f);
            imageGraphics.fillOval((int)(ball.x - ball.radius),
                                   (int)((((float)DISPLAY_SIZE) - ball.y) - ball.radius), i, i);
         }

         // Draw paddle.
         imageGraphics.setColor(Color.green);
         imageGraphics.fillRect(DISPLAY_SIZE - (int)paddle.width,
                                (int)((((float)DISPLAY_SIZE) - paddle.position) - (paddle.length / 2.0f)),
                                (int)paddle.width, (int)paddle.length);

         // Draw message.
         drawMessage();

         // Refresh display.
         graphics.drawImage(image, 0, 0, this);
      }


      // Set message.
      public void setMessage(String s)
      {
         message = s;
      }


      // Draw message.
      private void drawMessage()
      {
         int w;

         if (message == null) { return; }
         imageGraphics.setColor(Color.black);
         w = fontMetrics.stringWidth(message);
         imageGraphics.fillRect(((DISPLAY_SIZE - w) / 2) - 2,
                                (DISPLAY_SIZE / 2) - (fontAscent + 2), w + 4, fontHeight + 4);
         imageGraphics.setColor(Color.red);
         imageGraphics.drawString(message, (DISPLAY_SIZE - w) / 2, DISPLAY_SIZE / 2);
      }
   }

   // Display.
   Display display;

   // Control panel.
   class Controls extends JPanel implements ActionListener, ChangeListener
   {
      // Components.
      Button  controlButton;
      JSlider speedSlider;

      // Constructor.
      Controls()
      {
         setSize(DISPLAY_SIZE, CONTROLS_HEIGHT);
         controlButton = new Button("Start");
         controlButton.addActionListener(this);
         add(controlButton);
         add(new Label("Fast", Label.RIGHT));
         speedSlider = new JSlider(JSlider.HORIZONTAL, MIN_DELAY,
                                   MAX_DELAY, MAX_DELAY);
         speedSlider.addChangeListener(this);
         add(speedSlider);
         add(new Label("Stop", Label.LEFT));
      }


      // Start button listener.
      public void actionPerformed(ActionEvent evt)
      {
         ball = new Ball();
         if (playback != null)
         {
            if (controlButton.getLabel().equals("Start"))
            {
               if (playback.GetCurrent() != -1)
               {
                  playback.start();
                  display.setMessage(null);
                  playing = true;
               }
            }
            else
            {
               playing = false;
               if (playback.GetCount() > 0)
               {
                  if (playback.GetCurrent() == playback.GetCount() - 1)
                  {
                     playback.Rewind();
                  }
                  else
                  {
                     playback.Next();
                  }
                  display.setMessage("Game " + playback.GetCurrent());
                  controlButton.setLabel("Start");
               }
            }
         }
         else
         {
            display.setMessage(null);
            playing = true;
         }
      }


      // Speed slider listener.
      public void stateChanged(ChangeEvent evt)
      {
         delay = speedSlider.GetValue();
      }
   }

   // Controls.
   Controls controls;

   // Game playback file.
   static String playbackFile = null;

   // Game playback.
   class Playback
   {
      int dimension;

      class Step
      {
         int     step;
         double   ballX, ballY;
         double   paddleY;
         double   playerX, playerY;
         bool last;

         Step(int step, double ballX, double ballY,
              double paddleY, double playerX, double playerY)
         {
            this.step    = step;
            this.ballX   = ballX;
            this.ballY   = ballY;
            this.paddleY = paddleY;
            this.playerX = playerX;
            this.playerY = playerY;
            last         = false;
         }
      }

      class Game
      {
         int          game;
         Vector<Step> steps;
         int          currentStep;
         bool      win;

         Game(Scanner s)
         {
            steps       = new Vector<Step>();
            currentStep = -1;
            if (s.hasNext())
            {
               s.Next();
               if (s.hasNext())
               {
                  game = s.nextInt();
               }
            }
            while (s.hasNext())
            {
               String label = s.Next();
               if (label.equals("Step"))
               {
                  if (s.hasNext())
                  {
                     int   n       = s.nextInt();
                     double ballX   = s.nextFloat();
                     double ballY   = s.nextFloat();
                     double paddleY = s.nextFloat();
                     double playerX = s.nextFloat();
                     double playerY = s.nextFloat();
                     Step  step    = new Step(n, ballX, ballY, paddleY, playerX, playerY);
                     steps.Add(step);
                  }
               }
               else if (label.equals("Win"))
               {
                  win = true;
                  if (steps.GetCount() > 0)
                  {
                     steps.Get(steps.GetCount() - 1).last = true;
                  }
                  if (s.hasNext()) { s.nextInt(); }
                  break;
               }
               else if (label.equals("Lose"))
               {
                  win = false;
                  if (steps.GetCount() > 0)
                  {
                     steps.Get(steps.GetCount() - 1).last = true;
                  }
                  if (s.hasNext()) { s.nextInt(); }
                  break;
               }
               else if (label.equals("Draw"))
               {
                  win = false;
                  if (steps.GetCount() > 0)
                  {
                     steps.Get(steps.GetCount() - 1).last = true;
                  }
                  if (s.hasNext()) { s.nextInt(); }
                  break;
               }
               else
               {
                  System.err.println("Error loading playback file " + playbackFile);
                  System.exit(1);
               }
            }
         }


         void start()
         {
            if (steps.GetCount() > 0)
            {
               currentStep = 0;
            }
         }


         Step Step()
         {
            if (currentStep != -1)
            {
               if (currentStep < steps.GetCount() - 1)
               {
                  currentStep++;
                  return (steps.Get(currentStep - 1));
               }
               else
               {
                  return (steps.Get(currentStep));
               }
            }
            else
            {
               return (null);
            }
         }


         Step currentStep()
         {
            if (currentStep != -1)
            {
               return (steps.Get(currentStep));
            }
            else
            {
               return (null);
            }
         }


         void Stop()
         {
            currentStep = -1;
         }


         bool win()
         {
            return (win);
         }
      }

      Vector<Game> games;
      int          currentGame;

      Playback(String playbackFile)
      {
         games       = new Vector<Game>();
         currentGame = -1;
         Scanner s = null;

         try
         {
            s = new Scanner(new BufferedReader(new FileReader(playbackFile)));
            if (s.hasNext())
            {
               s.Next();
               if (s.hasNext())
               {
                  dimension = s.nextInt();
               }
            }
            while (s.hasNext())
            {
               Game game = new Game(s);
               games.Add(game);
            }
         }
         catch (IOException e)
         {
            System.err.println("Error loading playback file " +
                               playbackFile + ": " + e.toString());
            System.exit(1);
         }
         finally
         {
            if (s != null)
            {
               s.close();
            }
         }
      }


      void start()
      {
         if (currentGame != -1)
         {
            games.Get(currentGame).start();
         }
      }


      Step Step()
      {
         if (currentGame != -1)
         {
            return (games.Get(currentGame).Step());
         }
         else
         {
            return (null);
         }
      }


      Step currentStep()
      {
         if (currentGame != -1)
         {
            return (games.Get(currentGame).currentStep());
         }
         else
         {
            return (null);
         }
      }


      void Stop()
      {
         if (currentGame != -1)
         {
            games.Get(currentGame).Stop();
         }
      }


      int Next()
      {
         if (currentGame < games.GetCount() - 1)
         {
            currentGame++;
         }
         return (currentGame);
      }


      int Rewind()
      {
         if (currentGame > 0)
         {
            currentGame = 0;
         }
         return (currentGame);
      }


      int GetCurrent()
      {
         return (currentGame);
      }


      int GetCount()
      {
         return (games.GetCount());
      }


      bool win()
      {
         if (currentGame != -1)
         {
            return (games.Get(currentGame).win());
         }
         else
         {
            return false;
         }
      }
   }

   // Game playback.
   Playback playback = null;

   // Constructor.
   public Pong()
   {
      // Set title.
      setTitle("Pong");
      setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

      // Set delay.
      delay = MAX_DELAY;

      // Create ball and paddle.
      ball   = new Ball();
      paddle = new Paddle();

      // Create display.
      setLayout(new BorderLayout());
      display = new Display();
      getContentPane().Add(display, BorderLayout.NORTH);

      // Create controls.
      controls = new Controls();
      getContentPane().Add(controls, BorderLayout.SOUTH);

      // Load bounce sound.
      URL url = null;
      try
      {
         url = getClass().getResource(BOUNCE_SOUND_FILE);
      }
      catch (Exception e) {
         System.err.println("Cannot get sound " + BOUNCE_SOUND_FILE);
         return;
      }
      if (url != null)
      {
         // Playing and stopping insures sound completely loaded.
         bounceSound = Applet.newAudioClip(url);
         bounceSound.play();
         bounceSound.Stop();
      }

      // Show app.
      pack();
      setVisibletrue;

      // Create ball movement thread.
      ballThread = new Thread(this);
      ballThread.setPriority(Thread.MIN_PRIORITY);
      ballThread.start();

      // Start display thread.
      display.start();

      // Game playback?
      if (playbackFile != null)
      {
         playback = new Playback(playbackFile);
         if (playback.GetCount() > 0)
         {
            playback.Next();
            display.setMessage("Game 0");
         }
         else
         {
            display.setMessage("No playback games");
         }
      }
   }


   // Run.
   public void Run()
   {
      long t;

      // Get the current time for real time approximation.
      t = System.currentTimeMillis();

      // Ball movement loop.
      while (Thread.currentThread() == ballThread)
      {
         if (playing && (delay < MAX_DELAY))
         {
            if (playback != null)
            {
               Playback.Step step = playback.Step();
               if ((step != null) && step.last)
               {
                  controls.controlButton.setLabel("Next");
                  if (playback.win())
                  {
                     display.setMessage("Win");
                  }
                  else
                  {
                     display.setMessage("Lose");
                  }
               }
            }
            else
            {
               ball.move();
            }
         }

         // Set the timer for the next loop.
         try
         {
            t += delay;
            Sleep(Math.max(0, t - System.currentTimeMillis()));
         }
         catch (InterruptedException e) { break; }
      }
   }


   // Main.
   public static void main(String[] args)
   {
      String usage = "Usage: java Pong [-displaySize <size>] [-playbackFile <file name>]";

      for (int i = 0; i < args.length; i++)
      {
         if (args[i].equals("-displaySize"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid display size");
               System.err.println(usage);
               System.exit(1);
            }
            DISPLAY_SIZE = Integer.parseInt(args[i]);
            if (DISPLAY_SIZE <= 0)
            {
               System.err.println("Invalid display size");
               System.err.println(usage);
               System.exit(1);
            }
         }
         else if (args[i].equals("-playbackFile"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid game playback file");
               System.err.println(usage);
               System.exit(1);
            }
            playbackFile = args[i];
            MAX_DELAY   *= 40;
         }
         else
         {
            System.err.println(usage);
            System.exit(1);
         }
      }

      Pong pong = new Pong();
   }
}
