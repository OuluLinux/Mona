// For conditions of distribution and use, see copyright notice in muzz.hpp

/*

    The Muzz World.

    Muzz creatures interact with a block world.
    Each muzz has a mona neural network for a brain.

*/


#if 0



// Draw world.
void drawWorld(int skipMuzz) {
	int i;
	glColor3f(1.0f, 1.0f, 1.0f);

	if (wire_view)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glPushMatrix();
	// Draw the terrain.
	terrain.Draw();

	// Draw muzzes.
	for (i = 0; i < muzz_count; i++) {
		if (i != skipMuzz) {
			// Name the muzz for selection purposes.
			glPushMatrix();
			glPushName(i + 1);
			muzzes[i]->Draw();
			glPopName();
			glPopMatrix();
		}
	}

	// Draw mushrooms.
	for (i = 0; i < mushroom_count; i++) {
		if (mushrooms[i]->IsAlive())
			mushrooms[i]->Draw();
	}

	// Draw water pools.
	for (i = 0; i < pool_count; i++)
		pools[i]->Draw();

	glPopMatrix();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


// Display muzz view.
void displayMuzzView(void) {
	if (ViewSelection == TERRAIN_VIEW_ONLY)
		return;

	// Clear transform matrix.
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Place camera at current muzz view point.
	if (current_muzz != -1) {
		// Camera follows muzz.
		muzzes[current_muzz]->aimCamera(camera);
		// Set viewport and frustum.
		glViewport(viewports[MUZZ_VIEWPORT].x, viewports[MUZZ_VIEWPORT].y,
				   viewports[MUZZ_VIEWPORT].width, viewports[MUZZ_VIEWPORT].height);
		camera.setFrustum(MUZZ_FRUSTUM_ANGLE, viewports[MUZZ_VIEWPORT].aspect,
						  MUZZ_FRUSTUM_NEAR, MUZZ_FRUSTUM_FAR);
		// Draw the world.
		drawWorld(current_muzz);
	}
	else {
		// Set viewport.
		glViewport(viewports[MUZZ_VIEWPORT].x, viewports[MUZZ_VIEWPORT].y,
				   viewports[MUZZ_VIEWPORT].width, viewports[MUZZ_VIEWPORT].height);
	}

	// Label view.
	glLineWidth(2.0);
	enter2Dmode();
	draw2Dstring(5, 12, FONT, (char*)"Muzz");
	exit2Dmode();
}


// Display the terrain view.
void displayTerrainView(void) {
	if (ViewSelection == MUZZ_VIEW_ONLY)
		return;
	
	/*
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	// Set viewport and frustum.
	glViewport(viewports[TERRAIN_VIEWPORT].x, viewports[TERRAIN_VIEWPORT].y,
			   viewports[TERRAIN_VIEWPORT].width, viewports[TERRAIN_VIEWPORT].height);
	camera.setFrustum(TERRAIN_FRUSTUM_ANGLE, viewports[TERRAIN_VIEWPORT].aspect,
					  MUZZ_FRUSTUM_NEAR, MUZZ_FRUSTUM_FAR);
	// Position camera.
	camera.clearSpacial();
	camera.SetPosition(TerrainViewPosition);
	camera.setPitch(-90.0f);
	camera.setRoll(180.0f);
	camera.Place();

	// Rendering to select a muzz?
	if (renderMode == GL_SELECT)
		StartPicking();

	// Draw the world with a small perspective angle.
	glPushMatrix();
	glRotatef(20.0f, 1.0f, 0.0f, 1.0f);
	drawWorld(-1);

	if (renderMode == GL_SELECT)
		stopPicking();
	else {
		// Highlight current muzz.
		if (current_muzz != -1)
			muzzes[current_muzz]->Highlight();
	}

	glPopMatrix();
	// Label view.
	glLineWidth(2.0);
	enter2Dmode();
	draw2Dstring(5, 12, FONT, (char*)"Terrain");
	exit2Dmode();
	*/
	
}


// Display the controls view.
void displayControls(void) {
	int   vw, vh;
	GLint viewport[4];
	char  buf[100];
	// Set viewport.
	glViewport(viewports[CONTROLS_VIEWPORT].x, viewports[CONTROLS_VIEWPORT].y,
			   viewports[CONTROLS_VIEWPORT].width, viewports[CONTROLS_VIEWPORT].height);
	glGetIntegerv(GL_VIEWPORT, viewport);
	vw = viewport[2];
	vh = viewport[3];
	// Render the GUI.
	counter.markFrameStart();
	enter2DMode(guiFrame->GetWidth(), guiFrame->GetHeight());
	guiFrame->render(counter.getFrameInterval());
	counter.markFrameEnd();
	// Show frame rate.
	sprintf(buf, "FPS: %.2f", frame_rate.FPS);
	draw2Dstring(5, 15, FONT, buf);

	// Show cycle?
	if (cycles >= 0) {
		// Show training trial also?
		if ((training_trial_count >= 0) && (current_trial >= 0) &&
			(current_trial <= training_trial_count)) {
			sprintf(buf, "Cycle=%d/%d  Training trial=%d/%d",
					cycle_counter, cycles, current_trial, training_trial_count);
			draw2Dstring(5, vh - 10, FONT, buf);
		}
		else {
			sprintf(buf, "Cycle=%d/%d", cycle_counter, cycles);
			draw2Dstring(5, vh - 10, FONT, buf);
		}
	}
	else {
		// Show training trial?
		if ((training_trial_count >= 0) && (current_trial >= 0) &&
			(current_trial <= training_trial_count)) {
			sprintf(buf, "Training trial=%d/%d",
					current_trial, training_trial_count);
			draw2Dstring(5, vh - 10, FONT, buf);
		}
	}

	exit2DMode();
}


// Display the muzz status panel.
void displayMuzzStatus(void) {
	int   vw, vh, xoff, yoff;
	GLint viewport[4];
	char  buf[50];
	// Set viewport.
	glViewport(viewports[MUZZ_STATUS_VIEWPORT].x, viewports[MUZZ_STATUS_VIEWPORT].y,
			   viewports[MUZZ_STATUS_VIEWPORT].width, viewports[MUZZ_STATUS_VIEWPORT].height);
	enter2Dmode();

	if (current_muzz != -1) {
		sprintf(buf, "Muzz %d status", muzzes[current_muzz]->id);
		draw2Dstring(5, 12, FONT, buf);
	}
	else
		draw2Dstring(5, 12, FONT, (char*)"Muzz status");

	glGetIntegerv(GL_VIEWPORT, viewport);
	vw = viewport[2];
	vh = viewport[3];
	// Show sensors.
	xoff = 5;
	yoff = vh / 3;
	draw2Dstring(xoff, yoff, FONT, (char*)"Sensors:");
	xoff += 5;
	yoff += 15;

	if (current_muzz != -1) {
		if (muzz_states[current_muzz].sensors[Muzz::RIGHT_SENSOR] != Muzz::CLOSED) {
			if (muzz_states[current_muzz].sensors[Muzz::LEFT_SENSOR] != Muzz::CLOSED) {
				if (muzz_states[current_muzz].sensors[Muzz::FORWARD_SENSOR] != Muzz::CLOSED)
					draw2Dstring(xoff, yoff, FONT, (char*)" open<-open->open");
				else
					draw2Dstring(xoff, yoff, FONT, (char*)" open<-closed->open");
			}
			else {
				if (muzz_states[current_muzz].sensors[Muzz::FORWARD_SENSOR] != Muzz::CLOSED)
					draw2Dstring(xoff, yoff, FONT, (char*)" closed<-open->open");
				else
					draw2Dstring(xoff, yoff, FONT, (char*)" closed<-closed->open");
			}
		}
		else {
			if (muzz_states[current_muzz].sensors[Muzz::LEFT_SENSOR] != Muzz::CLOSED) {
				if (muzz_states[current_muzz].sensors[Muzz::FORWARD_SENSOR] != Muzz::CLOSED)
					draw2Dstring(xoff, yoff, FONT, (char*)" open<-open->closed");
				else
					draw2Dstring(xoff, yoff, FONT, (char*)" open<-closed->closed");
			}
			else {
				if (muzz_states[current_muzz].sensors[Muzz::FORWARD_SENSOR] != Muzz::CLOSED)
					draw2Dstring(xoff, yoff, FONT, (char*)" closed<-open->closed");
				else
					draw2Dstring(xoff, yoff, FONT, (char*)" closed<-closed->closed");
			}
		}

		yoff += 15;

		switch (muzz_states[current_muzz].sensors[Muzz::TERRAIN_SENSOR]) {
		case Muzz::PLATFORM:
			draw2Dstring(xoff, yoff, FONT, (char*)" Terrain=platform");
			break;

		case Muzz::WALL:
			draw2Dstring(xoff, yoff, FONT, (char*)" Terrain=wall");
			break;

		case Muzz::DROP:
			draw2Dstring(xoff, yoff, FONT, (char*)" Terrain=drop off");
			break;

		case Muzz::RAMP_UP:
			draw2Dstring(xoff, yoff, FONT, (char*)" Terrain=ramp up");
			break;

		case Muzz::RAMP_DOWN:
			draw2Dstring(xoff, yoff, FONT, (char*)" Terrain=ramp down");
			break;
		}

		yoff += 15;

		switch (muzz_states[current_muzz].sensors[Muzz::OBJECT_SENSOR]) {
		case Muzz::MUSHROOM:
			draw2Dstring(xoff, yoff, FONT, (char*)"  Object=mushroom");
			break;

		case Muzz::POOL:
			draw2Dstring(xoff, yoff, FONT, (char*)"  Object=pool");
			break;

		case Muzz::MUZZ:
			draw2Dstring(xoff, yoff, FONT, (char*)"  Object=muzz");
			break;

		case Muzz::EMPTY:
			draw2Dstring(xoff, yoff, FONT, (char*)"  Object=empty");
			break;

		default:
			#if (SENSE_BLOCK_ID == 1)
			if ((char)(muzz_states[current_muzz].sensors[Muzz::OBJECT_SENSOR] & 0xff) == '[')
				sprintf(buf, "  Object=blank(%d)", muzz_states[current_muzz].sensors[Muzz::OBJECT_SENSOR]);
			else {
				sprintf(buf, "  Object=%c(%d)", (char)(muzz_states[current_muzz].sensors[Muzz::OBJECT_SENSOR] & 0xff),
						muzz_states[current_muzz].sensors[Muzz::OBJECT_SENSOR]);
			}

			#else

			if ((char)muzz_states[current_muzz].sensors[Muzz::OBJECT_SENSOR] == '[')
				sprintf(buf, "  Object=blank");
			else
				sprintf(buf, "  Object=%c", muzz_states[current_muzz].sensors[Muzz::OBJECT_SENSOR]);

			#endif
			draw2Dstring(xoff, yoff, FONT, buf);
			break;
		}
	}
	else {
		draw2Dstring(xoff, yoff, FONT, (char*)" NA<-NA->NA");
		yoff += 15;
		draw2Dstring(xoff, yoff, FONT, (char*)" Terrain=NA");
		yoff += 15;
		draw2Dstring(xoff, yoff, FONT, (char*)"  Object=NA");
	}

	// Show response.
	xoff = (vw / 2) + 15;
	yoff = vh / 4;

	if (current_muzz != -1) {
		switch (muzz_states[current_muzz].response) {
		case Muzz::WAIT:
			draw2Dstring(xoff, yoff, FONT, (char*)"Response=wait");
			break;

		case Muzz::FORWARD:
			draw2Dstring(xoff, yoff, FONT, (char*)"Response=forward");
			break;

		case Muzz::RIGHT:
			draw2Dstring(xoff, yoff, FONT, (char*)"Response=right");
			break;

		case Muzz::LEFT:
			draw2Dstring(xoff, yoff, FONT, (char*)"Response=left");
			break;

		case Muzz::EAT:
			draw2Dstring(xoff, yoff, FONT, (char*)"Response=eat");
			break;

		case Muzz::DRINK:
			draw2Dstring(xoff, yoff, FONT, (char*)"Response=drink");
			break;
		}
	}
	else
		draw2Dstring(xoff, yoff, FONT, (char*)"Response=NA");

	// Show needs.
	yoff += 15;
	draw2Dstring(xoff, yoff, FONT, (char*)"Needs:");
	xoff += 5;
	yoff += 15;

	if (current_muzz != -1) {
		sprintf(buf, " Water=%.2f", muzzes[current_muzz]->GetNeed(Muzz::WATER));
		draw2Dstring(xoff, yoff, FONT, buf);
		yoff += 15;
		sprintf(buf, "  Food=%.2f", muzzes[current_muzz]->GetNeed(Muzz::FOOD));
		draw2Dstring(xoff, yoff, FONT, buf);
	}
	else {
		draw2Dstring(xoff, yoff, FONT, (char*)" Water=NA");
		yoff += 15;
		draw2Dstring(xoff, yoff, FONT, (char*)"  Food=NA");
	}

	exit2Dmode();
}


// Display muzz help view.
void displayMuzzHelpView(void) {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glViewport(viewports[MUZZ_HELP_VIEWPORT].x, viewports[MUZZ_HELP_VIEWPORT].y,
			   viewports[MUZZ_HELP_VIEWPORT].width, viewports[MUZZ_HELP_VIEWPORT].height);
	glLineWidth(2.0);
	enter2Dmode();
	helpInfo(MUZZ_HELP_VIEWPORT);
	draw2Dstring(5, 12, FONT, (char*)"Muzz");
	exit2Dmode();
}


// Display the terrain help view.
void displayTerrainHelpView(void) {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glViewport(viewports[TERRAIN_HELP_VIEWPORT].x, viewports[TERRAIN_HELP_VIEWPORT].y,
			   viewports[TERRAIN_HELP_VIEWPORT].width, viewports[TERRAIN_HELP_VIEWPORT].height);
	glLineWidth(2.0);
	enter2Dmode();
	helpInfo(TERRAIN_HELP_VIEWPORT);
	draw2Dstring(5, 12, FONT, (char*)"Terrain");
	exit2Dmode();
}


// Display function.
void display(void) {
	// Clear display.
	glutSetWindow(main_window);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Normal rendering?
	if (renderMode == GL_RENDER) {
		// Display viewports.
		if (Mode == HELP_MODE) {
			displayMuzzHelpView();
			displayTerrainHelpView();
		}
		else {
			displayMuzzView();
			displayTerrainView();
		}

		displayControls();
		displayMuzzStatus();
		// Partition window.
		drawPartitions();
		glutSwapBuffers();
		glFlush();
		// Update frame rate.
		frame_rate.Update();
	}
	else {
		// Rendering for muzz selection.
		displayTerrainView();
	}
}


// Configure viewport dimensions.
void configureviewports() {
	int i;

	switch (ViewSelection) {
	case VIEW_BOTH:
		i = TERRAIN_VIEWPORT;
		viewports[i].x      = 0;
		viewports[i].y      = GUIheight;
		viewports[i].width  = WindowWidth / 2;
		viewports[i].height = WindowHeight - GUIheight;

		if (viewports[i].height > 0)
			viewports[i].aspect = (double)viewports[i].width / (double)viewports[i].height;
		else
			viewports[i].aspect = 1.0;

		i = MUZZ_VIEWPORT;
		viewports[i].x      = WindowWidth / 2;
		viewports[i].y      = GUIheight;
		viewports[i].width  = WindowWidth / 2;
		viewports[i].height = WindowHeight - GUIheight;

		if (viewports[i].height > 0)
			viewports[i].aspect = (double)viewports[i].width / (double)viewports[i].height;
		else
			viewports[i].aspect = 1.0;

		break;

	case TERRAIN_VIEW_ONLY:
		i = TERRAIN_VIEWPORT;
		viewports[i].x      = 0;
		viewports[i].y      = GUIheight;
		viewports[i].width  = WindowWidth;
		viewports[i].height = WindowHeight - GUIheight;

		if (viewports[i].height > 0)
			viewports[i].aspect = (double)viewports[i].width / (double)viewports[i].height;
		else
			viewports[i].aspect = 1.0;

		break;

	case MUZZ_VIEW_ONLY:
		i = MUZZ_VIEWPORT;
		viewports[i].x      = 0;
		viewports[i].y      = GUIheight;
		viewports[i].width  = WindowWidth;
		viewports[i].height = WindowHeight - GUIheight;

		if (viewports[i].height > 0)
			viewports[i].aspect = (double)viewports[i].width / (double)viewports[i].height;
		else
			viewports[i].aspect = 1.0;

		break;
	}

	i = CONTROLS_VIEWPORT;
	viewports[i].x      = 0;
	viewports[i].y      = 0;
	viewports[i].width  = WindowWidth / 2;
	viewports[i].height = GUIheight;

	if (viewports[i].height > 0)
		viewports[i].aspect = (double)viewports[i].width / (double)viewports[i].height;
	else
		viewports[i].aspect = 1.0;

	i = MUZZ_STATUS_VIEWPORT;
	viewports[i].x      = WindowWidth / 2;
	viewports[i].y      = 0;
	viewports[i].width  = WindowWidth / 2;
	viewports[i].height = GUIheight;

	if (viewports[i].height > 0)
		viewports[i].aspect = (double)viewports[i].width / (double)viewports[i].height;
	else
		viewports[i].aspect = 1.0;

	i = TERRAIN_HELP_VIEWPORT;
	viewports[i].x      = 0;
	viewports[i].y      = GUIheight;
	viewports[i].width  = WindowWidth / 2;
	viewports[i].height = WindowHeight - GUIheight;

	if (viewports[i].height > 0)
		viewports[i].aspect = (double)viewports[i].width / (double)viewports[i].height;
	else
		viewports[i].aspect = 1.0;

	i = MUZZ_HELP_VIEWPORT;
	viewports[i].x      = WindowWidth / 2;
	viewports[i].y      = GUIheight;
	viewports[i].width  = WindowWidth / 2;
	viewports[i].height = WindowHeight - GUIheight;

	if (viewports[i].height > 0)
		viewports[i].aspect = (double)viewports[i].width / (double)viewports[i].height;
	else
		viewports[i].aspect = 1.0;

	// Set GUI frame dimensions.
	guiFrame->setSizes(viewports[CONTROLS_VIEWPORT].width,
							viewports[CONTROLS_VIEWPORT].height);
	guiFrame->forceUpdatetrue;
}


// Reshape.
void reshape(int width, int height) {
	// Hack to make sure window is what it is reported to be...
	static bool init = true;

	if (init) {
		init = false;
		glutReshapeWindow(width, height);
	}

	WindowWidth = width;

	if (height == 0)
		height = 1;

	WindowHeight = height;
	GUIheight    = WindowHeight / 8;
	configureviewports();
	glutPostRedisplay();
}


// Keyboard input.
#define DELETE_KEY       127
#define RETURN_KEY       13
#define BACKSPACE_KEY    8
void
keyboard(unsigned char key, int x, int y) {
	// Void manual response.
	manual_response = INVALID_RESPONSE;

	// Toggle help mode.
	if ((Mode == HELP_MODE) && (key == ' ')) {
		if (terrain_mode)
			Mode = TERRAIN_MODE;
		else
			Mode = MANUAL_MODE;

		frame_rate.Reset();
	}
	else {
		// Check GUI key event.
		guiFrame->checkKeyboardEvents(KeyEvent(key), KE_PRESSED);

		switch (key) {
		// Eat and drink responses.
		case 'e':
			manual_response = Muzz::EAT;
			break;

		case 'd':
			manual_response = Muzz::DRINK;
			break;

		// Dumps.
		case 'p':
			if (current_muzz != -1)
				muzzes[current_muzz]->PrintBrain();
			else
				printf("Please select a muzz\n");

			break;

		case 'r':
			if (current_muzz != -1)
				muzzes[current_muzz]->printResponsePotentials();
			else
				printf("Please select a muzz\n");

			break;

		case 's':
			saveMuzzWorld((char*)"muzz.world");
			break;

		// Toggle wireframe mode.
		case 'w':
			wire_view = !wire_view;
			break;

		// Full screen.
		case 'f':
			glutFullScreen();
			break;

		// User help.
		case 'h':
			Mode = HELP_MODE;
			break;

		// Quit.
		case 'q':
			termMuzzWorld();
			glutDestroyWindow(main_window);
			exit(0);
		}
	}

	// Re-display.
	glutPostRedisplay();
}


// Special keyboard input.
void
specialKeyboard(int key, int x, int y) {
	switch (Mode) {
	case MANUAL_MODE:
		if ((current_muzz == -1) ||
			(manual_response != INVALID_RESPONSE))
			break;

		switch (key) {
		case GLUT_KEY_UP:
			manual_response = Muzz::FORWARD;
			break;

		case GLUT_KEY_RIGHT:
			manual_response = Muzz::RIGHT;
			break;

		case GLUT_KEY_LEFT:
			manual_response = Muzz::LEFT;
			break;
		}

		break;

	case TERRAIN_MODE:
		switch (key) {
		case GLUT_KEY_UP:
			TerrainViewPosition[2] -= TERRAIN_MOVEMENT_DELTA;
			break;

		case GLUT_KEY_DOWN:
			TerrainViewPosition[2] += TERRAIN_MOVEMENT_DELTA;
			break;

		case GLUT_KEY_RIGHT:
			TerrainViewPosition[0] += TERRAIN_MOVEMENT_DELTA;
			break;

		case GLUT_KEY_LEFT:
			TerrainViewPosition[0] -= TERRAIN_MOVEMENT_DELTA;
			break;

		case GLUT_KEY_PAGE_DOWN:
			TerrainViewPosition[1] -= TERRAIN_MOVEMENT_DELTA;
			break;

		case GLUT_KEY_PAGE_UP:
			TerrainViewPosition[1] += TERRAIN_MOVEMENT_DELTA;
			break;
		}

		break;
	}

	// Re-display.
	glutPostRedisplay();
}



// Mouse callbacks.
void mouseClicked(int button, int state, int x, int y) {
	if (button != GLUT_LEFT_BUTTON)
		return;

	cursorX = x;
	cursorY = y;

	// Switch views and initiate muzz selection.
	if ((Mode != HELP_MODE) && (state == GLUT_DOWN)) {
		int vy = WindowHeight - cursorY;
		PendingViewSelection = (VIEW_SELECTION)(-1);

		switch (ViewSelection) {
		case MUZZ_VIEW_ONLY:
			if (((cursorX >= viewports[MUZZ_VIEWPORT].x) &&
				 (cursorX < (viewports[MUZZ_VIEWPORT].x + viewports[MUZZ_VIEWPORT].width))) &&
				((vy >= viewports[MUZZ_VIEWPORT].y) &&
				 (vy < (viewports[MUZZ_VIEWPORT].y + viewports[MUZZ_VIEWPORT].height)))) {
				ViewSelection = VIEW_BOTH;
				configureviewports();
				glutPostRedisplay();
			}

			break;

		case TERRAIN_VIEW_ONLY:
			if (((cursorX >= viewports[TERRAIN_VIEWPORT].x) &&
				 (cursorX < (viewports[TERRAIN_VIEWPORT].x + viewports[TERRAIN_VIEWPORT].width))) &&
				((vy >= viewports[TERRAIN_VIEWPORT].y) &&
				 (vy < (viewports[TERRAIN_VIEWPORT].y + viewports[TERRAIN_VIEWPORT].height)))) {
				// Reconfigure viewports after picking render.
				PendingViewSelection = VIEW_BOTH;
				// Setup render to pick a muzz.
				renderMode = GL_SELECT;
			}

			break;

		case VIEW_BOTH:
			if (((cursorX >= viewports[MUZZ_VIEWPORT].x) &&
				 (cursorX < (viewports[MUZZ_VIEWPORT].x + viewports[MUZZ_VIEWPORT].width))) &&
				((vy >= viewports[MUZZ_VIEWPORT].y) &&
				 (vy < (viewports[MUZZ_VIEWPORT].y + viewports[MUZZ_VIEWPORT].height)))) {
				ViewSelection = MUZZ_VIEW_ONLY;
				configureviewports();
				glutPostRedisplay();
			}

			if (((cursorX >= viewports[TERRAIN_VIEWPORT].x) &&
				 (cursorX < (viewports[TERRAIN_VIEWPORT].x + viewports[TERRAIN_VIEWPORT].width))) &&
				((vy >= viewports[TERRAIN_VIEWPORT].y) &&
				 (vy < (viewports[TERRAIN_VIEWPORT].y + viewports[TERRAIN_VIEWPORT].height)))) {
				// Reconfigure viewports after picking render.
				PendingViewSelection = TERRAIN_VIEW_ONLY;
				// Setup render to pick a muzz.
				renderMode = GL_SELECT;
			}

			break;
		}
	}

	// Adjust for GUI viewport.
	x -= viewports[CONTROLS_VIEWPORT].x;
	y -= (WindowHeight - viewports[CONTROLS_VIEWPORT].height);
	MouseEvent event = MouseEvent(MB_BUTTON1, x, y, guiFrame->GetHeight() - y);
	guiFrame->checkMouseEvents(event, (state == GLUT_DOWN) ? ME_CLICKED : ME_RELEASED);
}


void mouseDragged(int x, int y) {
	x -= viewports[CONTROLS_VIEWPORT].x;
	y -= (WindowHeight - viewports[CONTROLS_VIEWPORT].height);
	MouseEvent event = MouseEvent(MB_UNKNOWN_BUTTON, x, y, guiFrame->GetHeight() - y);
	guiFrame->checkMouseEvents(event, ME_DRAGGED);
}


void mouseMoved(int x, int y) {
	x -= viewports[CONTROLS_VIEWPORT].x;
	y -= (WindowHeight - viewports[CONTROLS_VIEWPORT].height);
	MouseEvent event = MouseEvent(MB_UNKNOWN_BUTTON, x, y, guiFrame->GetHeight() - y);
	guiFrame->checkMouseEvents(event, ME_MOVED);
}


// GUI event handler.
void EventsHandler::actionPerformed(GUIEvent& evt) {
	const String& callbackString   = evt.getCallbackString();
	GUIRectangle*       sourceRectangle = evt.getEventSource(),
						*parent        = sourceRectangle ? sourceRectangle->getParent() : NULL;
	int widgetType = sourceRectangle->getWidgetType();

	if (widgetType == WT_CHECK_BOX) {
		GUICheckBox* checkbox = (GUICheckBox*)sourceRectangle;
		// Void manual response.
		manual_response = INVALID_RESPONSE;

		// Muzz/terrain mode?
		if (callbackString == "manual") {
			terrain_mode = !checkbox->isChecked();

			if (!terrain_mode && (current_muzz == -1)) {
				terrain_mode = true;
				checkbox->setChecked(false);
			}

			if (Mode != HELP_MODE) {
				if (terrain_mode)
					Mode = TERRAIN_MODE;
				else
					Mode = MANUAL_MODE;
			}
		}

		if (callbackString == "pause")
			pause = checkbox->isChecked();
	}

	if (widgetType == WT_BUTTON) {
		GUIButton* button = (GUIButton*)sourceRectangle;
		// Void manual response.
		manual_response = INVALID_RESPONSE;

		// Reset world?
		if (callbackString == "reset") {
			if (button->isClicked()) {
				if (training_trial_count >= 0)
					resetTraining();

				resetMuzzWorld();
				cycle_counter = 0;
			}
		}

		if (callbackString == "step") {
			if (button->isClicked()) {
				if (Mode == TERRAIN_MODE)
					step = true;
			}
		}

		if (callbackString == "help") {
			if (button->isClicked()) {
				if (Mode != HELP_MODE)
					Mode = HELP_MODE;
				else {
					if (terrain_mode)
						Mode = TERRAIN_MODE;
					else
						Mode = MANUAL_MODE;

					frame_rate.Reset();
				}
			}
		}

		if (callbackString == "exit") {
			if (button->isClicked()) {
				termMuzzWorld();
				glutDestroyWindow(main_window);
				exit(0);
			}
		}
	}
}


// Picking.
void StartPicking() {
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glSelectBuffer(BUFSIZE, selectBuf);
	glRenderMode(GL_SELECT);
	glInitNames();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPickMatrix((GLdouble)cursorX, (GLdouble)(WindowHeight - cursorY), 5.0, 5.0, viewport);
	gluPerspective(TERRAIN_FRUSTUM_ANGLE, viewports[TERRAIN_VIEWPORT].aspect,
				   MUZZ_FRUSTUM_NEAR, MUZZ_FRUSTUM_FAR);
	glMatrixMode(GL_MODELVIEW);
}


void processHits(GLint hits, GLuint buffer[], int sw) {
	GLint  i, numberOfNames;
	GLuint names, *ptr, minZ, *ptrNames;
	numberOfNames = 0;
	ptr           = (GLuint*)buffer;
	minZ          = 0xffffffff;

	for (i = 0; i < hits; i++) {
		names = *ptr;
		ptr++;

		if (*ptr < minZ) {
			numberOfNames = names;
			minZ          = *ptr;
			ptrNames      = ptr + 2;
		}

		ptr += names + 2;
	}

	if (numberOfNames > 0) {
		ptr = ptrNames;
		i   = *ptr - 1;

		if (current_muzz != i) {
			current_muzz = i;
			muzz_states[i].moveAmount = 0.0;
		}
		else {
			muzz_states[i].moveAmount = 0.0;
			current_muzz = -1;
			manualCheck->setChecked(false);
			terrain_mode = true;

			if (Mode == MANUAL_MODE)
				Mode = TERRAIN_MODE;
		}

		manual_response = INVALID_RESPONSE;
		// Mouse click was used for picking so cancel pending view selection.
		PendingViewSelection = (VIEW_SELECTION)(-1);
	}
}


void stopPicking() {
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glFlush();
	pickHits = glRenderMode(GL_RENDER);

	if (pickHits != 0)
		processHits(pickHits, selectBuf, 0);

	renderMode = GL_RENDER;

	// Change to pending view if picking did not occur.
	if (PendingViewSelection != (VIEW_SELECTION)(-1)) {
		ViewSelection        = PendingViewSelection;
		PendingViewSelection = (VIEW_SELECTION)(-1);
		configureviewports();
		glutPostRedisplay();
	}
}


// Help for controls.
void helpInfo(int viewport) {
	int i, v;
	glColor3f(1.0f, 1.0f, 1.0f);
	v = 30;
	draw2Dstring(5, v, FONT, (char*)"Controls:");
	v += (2 * LINE_SPACE);

	if (viewport == MUZZ_HELP_VIEWPORT) {
		for (i = 0; MuzzControlHelp[i] != NULL; i++) {
			draw2Dstring(5, v, FONT, MuzzControlHelp[i]);
			v += LINE_SPACE;
		}
	}
	else if (viewport == TERRAIN_HELP_VIEWPORT) {
		for (i = 0; TerrainViewControlHelp[i] != NULL; i++) {
			draw2Dstring(5, v, FONT, TerrainViewControlHelp[i]);
			v += LINE_SPACE;
		}
	}

	v += LINE_SPACE;
	draw2Dstring(5, v, FONT, (char*)"Press space bar to continue...");
}


// Draw window partitions.
void drawPartitions() {
	glViewport(0, 0, WindowWidth, WindowHeight);
	glLineWidth(2.0);
	enter2Dmode();
	glBegin(GL_LINES);

	if ((Mode == HELP_MODE) || (ViewSelection == VIEW_BOTH)) {
		glVertex2f(viewports[MUZZ_HELP_VIEWPORT].width, 0);
		glVertex2f(viewports[MUZZ_HELP_VIEWPORT].width, viewports[MUZZ_HELP_VIEWPORT].height);
	}

	glVertex2f(0, viewports[MUZZ_HELP_VIEWPORT].height);
	glVertex2f(WindowWidth, viewports[MUZZ_HELP_VIEWPORT].height);
	glVertex2f(viewports[MUZZ_STATUS_VIEWPORT].x, viewports[MUZZ_HELP_VIEWPORT].height);
	glVertex2f(viewports[MUZZ_STATUS_VIEWPORT].x, WindowHeight);
	glEnd();
	exit2Dmode();
	glLineWidth(1.0);
}


void enter2Dmode() {
	GLint viewport[4];
	glColor3f(1.0, 1.0, 1.0);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glGetIntegerv(GL_VIEWPORT, viewport);
	gluOrtho2D(0, viewport[2], 0, viewport[3]);
	// Invert the y axis, down is positive.
	glScalef(1, -1, 1);
	// Move the origin from the bottom left corner to the upper left corner.
	glTranslatef(0, -viewport[3], 0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}


void exit2Dmode() {
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}


// Print string on screen at specified location.
void draw2Dstring(double x, double y, void* font, char* string) {
	char* c;
	glRasterPos2f(x, y);

	for (c = string; *c != '\0'; c++)
		glutBitmapCharacter(font, *c);
}


// GUI 2D mode.
void enter2DMode(GLint winWidth, GLint winHeight) {
	Tuple4i viewport;

	if ((winWidth <= 0) || (winHeight <= 0)) {
		glGetIntegerv(GL_VIEWPORT, viewport);
		winWidth  = viewport.z;
		winHeight = viewport.w;
	}

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, winWidth, winHeight, 0);
	glDisable(GL_DEPTH_TEST);
}


void exit2DMode() {
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
}




// Less-than comparison for sort.
bool ltcmpSearch(ResponseSearch* a, ResponseSearch* b) {
	if (a->goalDist < b->goalDist)
		return true;
	else
		return false;
}




#endif
