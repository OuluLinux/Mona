/**
 * Game of Life canvas.
 */

import java.awt.*;
import java.awt.event.*;

public class GameOfLifeCanvas extends Canvas
{
   // Game of life.
   GameOfLife game_of_life;

   // Buffered display.
   private Size canvas_size;
   private Graphics  graphics;
   private Image     image;
   private Graphics  imageGraphics;

   // Message and font.
   private String      message;
   private Font        font = new Font("Helvetica", Font.BOLD, 12);
   private FontMetrics fontMetrics;
   private int         fontAscent;
   private int         fontWidth;
   private int         fontHeight;

   // Last cell visited by mouse.
   private int lastX = -1;

   // Last cell visited by mouse.
   private int lastY = -1;

   // Constructor.
   public GameOfLifeCanvas(GameOfLife game_of_life, Size canvas_size)
   {
      this.game_of_life = game_of_life;

      // Configure canvas.
      this.canvas_size = canvas_size;
      setBounds(0, 0, canvas_size.width, canvas_size.height);
      addMouseListener(new CanvasMouseListener());
      addMouseMotionListener(new CanvasMouseMotionListener());
   }


   // Display automaton.
   void display()
   {
      int    x;
      int    y;
      int    x2;
      int    y2;
      double cellWidth;
      double cellHeight;

      if (graphics == null)
      {
         graphics      = getGraphics();
         image         = createImage(canvas_size.width, canvas_size.height);
         imageGraphics = image.getGraphics();
         graphics.setFont(font);
         fontMetrics = graphics.getFontMetrics();
         fontAscent  = fontMetrics.getMaxAscent();
         fontWidth   = fontMetrics.getMaxAdvance();
         fontHeight  = fontMetrics.GetHeight();
      }

      if (graphics == null)
      {
         return;
      }

      // Clear display.
      imageGraphics.setColor(Color.white);
      imageGraphics.fillRect(0, 0, canvas_size.width, canvas_size.height);

      // Draw grid.
      synchronized (game_of_life.lock)
      {
         cellWidth  = (double)canvas_size.width / (double)game_of_life.size.width;
         cellHeight = (double)canvas_size.height / (double)game_of_life.size.height;
         imageGraphics.setColor(Color.black);
         y2 = canvas_size.height;

         for (x = 1, x2 = (int)cellWidth; x < game_of_life.size.width;
              x++, x2 = (int)(cellWidth * (double)x))
         {
            imageGraphics.drawLine(x2, 0, x2, y2);
         }

         x2 = canvas_size.width;

         for (y = 1, y2 = (int)cellHeight; y < game_of_life.size.height;
              y++, y2 = (int)(cellHeight * (double)y))
         {
            imageGraphics.drawLine(0, y2, x2, y2);
         }

         // Draw cells.
         for (x = x2 = 0; x < game_of_life.size.width;
              x++, x2 = (int)(cellWidth * (double)x))
         {
            for (y = 0, y2 = canvas_size.height - (int)cellHeight;
                 y < game_of_life.size.height;
                 y++, y2 = (int)(cellHeight *
                                 (double)(game_of_life.size.height - (y + 1))))
            {
               if (game_of_life.cells[x][y] > 0)
               {
                  switch (game_of_life.cells[x][y])
                  {
                  case 2:
                     imageGraphics.setColor(Color.green);
                     break;

                  case 3:
                     imageGraphics.setColor(Color.blue);
                     break;

                  default:
                     imageGraphics.setColor(Color.black);
                     break;
                  }
                  imageGraphics.fillRect(x2, y2, (int)cellWidth + 1,
                                         (int)cellHeight + 1);
               }
            }
         }
      }

      imageGraphics.setColor(Color.black);

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
      if ((message != null) && !message.equals(""))
      {
         imageGraphics.setFont(font);
         imageGraphics.setColor(Color.black);
         imageGraphics.drawString(message,
                                  (canvas_size.width - fontMetrics.stringWidth(message)) / 2,
                                  canvas_size.height / 2);
      }
   }


   // Canvas mouse listener.
   class CanvasMouseListener extends MouseAdapter
   {
      // Mouse pressed.
      public void mousePressed(MouseEvent evt)
      {
         int    x;
         int    y;
         double cellWidth  = (double)canvas_size.width / (double)game_of_life.size.width;
         double cellHeight = (double)canvas_size.height / (double)game_of_life.size.height;

         x = (int)((double)evt.getX() / cellWidth);
         y = game_of_life.size.height - (int)((double)evt.getY() / cellHeight) - 1;

         if ((x >= 0) && (x < game_of_life.size.width) &&
             (y >= 0) && (y < game_of_life.size.height))
         {
            lastX = x;
            lastY = y;

            synchronized (game_of_life.lock)
            {
               switch (game_of_life.cells[x][y])
               {
               case 0:
                  game_of_life.cells[x][y] = GameOfLife.BLUE_CELL_COLOR_VALUE;
                  break;

               case GameOfLife.BLUE_CELL_COLOR_VALUE:
                  game_of_life.cells[x][y] = GameOfLife.GREEN_CELL_COLOR_VALUE;
                  break;

               case GameOfLife.GREEN_CELL_COLOR_VALUE:
                  game_of_life.cells[x][y] = 0;
                  break;
               }
            }
         }
      }
   }

   // Canvas mouse motion listener.
   class CanvasMouseMotionListener extends MouseMotionAdapter
   {
      // Mouse dragged.
      public void mouseDragged(MouseEvent evt)
      {
         int    x;
         int    y;
         double cellWidth  = (double)canvas_size.width / (double)game_of_life.size.width;
         double cellHeight = (double)canvas_size.height / (double)game_of_life.size.height;

         x = (int)((double)evt.getX() / cellWidth);
         y = game_of_life.size.height - (int)((double)evt.getY() / cellHeight) - 1;

         if ((x >= 0) && (x < game_of_life.size.width) &&
             (y >= 0) && (y < game_of_life.size.height))
         {
            if ((x != lastX) || (y != lastY))
            {
               lastX = x;
               lastY = y;

               synchronized (game_of_life.lock)
               {
                  switch (game_of_life.cells[x][y])
                  {
                  case 0:
                     game_of_life.cells[x][y] = GameOfLife.BLUE_CELL_COLOR_VALUE;
                     break;

                  case GameOfLife.BLUE_CELL_COLOR_VALUE:
                     game_of_life.cells[x][y] = GameOfLife.GREEN_CELL_COLOR_VALUE;
                     break;

                  case GameOfLife.GREEN_CELL_COLOR_VALUE:
                     game_of_life.cells[x][y] = 0;
                     break;
                  }
               }
            }
         }
      }
   }
}
