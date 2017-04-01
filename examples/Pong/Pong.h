/*
    Copyright (c) 2015 Tom Portegys (portegys@gmail.com). All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are
    permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY TOM PORTEGYS "AS IS" AND ANY EXPRESS OR IMPLIED
    WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
    FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> OR
    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
    ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
    ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// The game of Pong.

#ifndef PONG_HPP
#define PONG_HPP

#ifdef _WIN32
	#ifndef WIN32
		#define WIN32
	#endif
#endif

#include "../common/common.h"

/*
    Game area size: 1x1.
    Paddle on right side.
*/
class Pong {
public:

	// Step outcome.
	enum STEP_OUTCOME {
		CLEAR  = 0,
		SCORE  = 1,
		STRIKE = 2
	};

	// Paddle.
	class Paddle {
	public:
		static const double DEFAULT_WIDTH;
		static const double DEFAULT_LENGTH;
		double              width;
		double              length;
		double              position;

		// Constructor.
		Paddle();

		// Set properties.
		void setWidth(double width);
		void setLength(double length);
		void setPosition(double position);

		// Move paddle.
		void move(double position);

		// Load.
		void Load(FILE*);

		// Save.
		void Store(FILE*);
	};
	Paddle paddle;

	// Ball.
	class Ball {
	public:
		static const double DEFAULT_RADIUS;
		double              radius;
		Vector             position;
		Vector             velocity;

		// Constructor.
		Ball();

		// Set properties.
		void setRadius(double radius);
		void setPosition(double x, double y);
		void setSpeed(double speed);
		void setVelocity(double dx, double dy);

		// Step ball.
		STEP_OUTCOME Step(Paddle& paddle);

		// Load.
		void Load(FILE*);

		// Save.
		void Store(FILE*);
	};
	Ball ball;

	// Constructor.
	Pong(double paddleLength = Paddle::DEFAULT_LENGTH, double ballRadius = Ball::DEFAULT_RADIUS);

	// Set paddle position.
	void setPaddlePosition(double position);

	// Set ball position and velocity.
	void setBallPosition(double x, double y);
	void setBallSpeed(double speed);
	void setBallVelocity(double dx, double dy);

	// Step.
	STEP_OUTCOME Step();

	// Load.
	void Load(String filename);
	void Load(FILE*);

	// Save.
	void Store(String filename);
	void Store(FILE*);
};
#endif
