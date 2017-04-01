// For conditions of distribution and use, see copyright notice in pong.hpp

#include "Pong.h"

const double Pong::Paddle::DEFAULT_WIDTH  = 0.05f;
const double Pong::Paddle::DEFAULT_LENGTH = 0.2f;

// Paddle constructor.
Pong::Paddle::Paddle()
{
   width    = DEFAULT_WIDTH;
   length   = DEFAULT_LENGTH;
   position = 0.5f;
}


// Set paddle width.
void Pong::Paddle::setWidth(double w)
{
   width = w;
}


// Set paddle length.
void Pong::Paddle::setLength(double l)
{
   length = l;
}


// Set paddle position.
void Pong::Paddle::setPosition(double y)
{
   position = y;
}


void Pong::Paddle::Serialize(Stream& fp)
{
	fp % width % length % position;
}

const double Pong::Ball::DEFAULT_RADIUS = 0.05f;

// Ball constructor.
Pong::Ball::Ball()
{
   radius   = DEFAULT_RADIUS;
   position = Vector(0.5f, 0.5f, 0.0f);
   velocity = Vector(0.0f, 0.0f, 0.0f);
}


// Set ball radius.
void Pong::Ball::setRadius(double r)
{
   radius = r;
}


// Set ball position.
void Pong::Ball::setPosition(double x, double y)
{
   position = Vector(x, y, 0.0f);
}


// Set ball speed.
void Pong::Ball::setSpeed(double speed)
{
   velocity.Normalize(speed);
}


// Set ball velocity.
void Pong::Ball::setVelocity(double dx, double dy)
{
   velocity = Vector(dx, dy, 0.0f);
}


// Step ball.
Pong::STEP_OUTCOME Pong::Ball::Step(Paddle& paddle)
{
   Pong::STEP_OUTCOME outcome = Pong::CLEAR;

   if ((position.x - radius + velocity.x) <= 0.0f)
   {
      velocity.x = -velocity.x;
   }
   if ((position.x + radius + velocity.x) >= 1.0f)
   {
      outcome = Pong::SCORE;
   }
   if ((position.y - radius + velocity.y) <= 0.0f)
   {
      velocity.y = -velocity.y;
   }
   if ((position.y + radius + velocity.y) >= 1.0f)
   {
      velocity.y = -velocity.y;
   }
   double l2 = paddle.length / 2.0;
   if (((position.x + radius + velocity.x) >= (1.0f - paddle.width)) &&
       (position.y <= (paddle.position + l2)) && (position.y >= (paddle.position - l2)))
   {
      velocity.x = -velocity.x;
      outcome    = Pong::STRIKE;
   }
   position.x += velocity.x;
   position.y += velocity.y;
   return (outcome);
}


void Pong::Ball::Serialize(Stream& fp)
{
	fp % radius % position.x % position.y % velocity.x % velocity.y;
}

// Pong constructor.
Pong::Pong(double paddleLength, double ballRadius)
{
   paddle.setLength(paddleLength);
   ball.setRadius(ballRadius);
}


// Set paddle position.
void Pong::setPaddlePosition(double y)
{
   paddle.setPosition(y);
}


// Set ball position.
void Pong::setBallPosition(double x, double y)
{
   ball.setPosition(x, y);
}


// Set ball speed.
void Pong::setBallSpeed(double speed)
{
   ball.setSpeed(speed);
}


// Set ball velocity.
void Pong::setBallVelocity(double dx, double dy)
{
   ball.setVelocity(dx, dy);
}


// Step: returns true if ball scores.
Pong::STEP_OUTCOME Pong::Step()
{
   return (ball.Step(paddle));
}


// Load pong.
void Pong::Load(String filename)
{
   Stream& fp;

   if ((fp = FOPEN_READ(filename)) == NULL)
   {
      fprintf(stderr, "Cannot load pong from file %s\n", filename);
      exit(1);
   }
   Load(fp);
   FCLOSE(fp);
}


void Pong::Serialize(Stream& fp)
{
   paddle.Load(fp);
   ball.Load(fp);
}


// Save pong.
void Pong::Store(String filename)
{
   Stream& fp;

   if ((fp = FOPEN_WRITE(filename)) == NULL)
   {
      fprintf(stderr, "Cannot save pong to file %s\n", filename);
      exit(1);
   }
   Store(fp);
   FCLOSE(fp);
}


void Pong::Store(Stream& fp)
{
   paddle.Store(fp);
   ball.Store(fp);
}
