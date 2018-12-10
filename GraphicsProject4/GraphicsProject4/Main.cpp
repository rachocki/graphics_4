#include <GL/glut.h>
#include <vector>
#include <time.h>
#include <string>

// Basic Game Code
/*
The Idea: the player (bat) collects tokens (bugs) by mousing over them
	-The bugs move.
	-The bugs are generated at random times, locations, and movement patterns.
	-The bat catches a bug when the mouse is in the area of the token's quad. The bug
		disappears and the player's score is incremented.
	-There is a time limit, displayed in the upper left hand corner.
	-The final score is displayed after time is up.
*/
// To Do
/*
	-Fix screen flicker! Consider double buffering.
	-Fix the color errors- the timer keeps turning yellow.

	Stretch Goals:
	-Add sprite of bat to cursor position: At every mouse movement, translate the center of the sprite to the
		cursor position.
	-Animate the player sprite by swapping out the sprite for the next frame of the animation at intervals.
	-Display the score as text on the screen.
	-Edit the bug spawn code to only generate bugs at the edges of the screen.
	-Import a nice background image.
	-Add some enemies.
	-Add variation in size and point value of bugs.
*/



//Global Vars
bool gameOver = false; //terminates game loop when true
int score = 0; //defaults to 0
int currentTime = 120; //default 120 seconds at start of game, decrements.
std::vector<std::vector<int>> activeBugs; //tracks existing bugs
unsigned char timestring[] = "TIME: 120"; //string to print time
unsigned char scorestring[] = "SCORE: 000"; //string to print score 
int startTime = 120; //time remaining at start of new game
int bugCountDown = 0; //seconds until next bug spawn
int lastBugGenerated = 120; //time at which last bug was generated.
void generateBug();
void updateTimer(int newTime);
void updateScore();
int getIntersect(int x, int y);
std::vector<int> tempVec; //temporary storage to simplify operations on the bug vector.
int currentX = -1;
int currentY = -1;

int main(int argc, char **argv)
{
	//Variables, Functions, and Callbacks
	void canvasDisplay();
	void mouseMove(int x, int y);
	void countDown(int count);
	void redraw();
	void updateTimer(int newTime);

	//Initialize Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(700, 480);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Hungry Bat");
	glClearColor(0.0, 0.0, 0.0, 0.0); //BG Color. TODO: Change this to an image.

	//Initialize Coordinate System + Viewport, Enable Textures
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, 700.0, 0, 480.0);
	glEnable(GL_TEXTURE_2D);

	//Register callbacks
	glutDisplayFunc(canvasDisplay);
	glutPassiveMotionFunc(mouseMove);
	glutTimerFunc(1000, countDown, 0);
	glutDisplayFunc(redraw); 

	glutMainLoop();

	return 0;
}

//Functions

//Initial setup
void canvasDisplay(void) {

	glClear(GL_COLOR_BUFFER_BIT); //Clear screen
	updateTimer(currentTime); //Print timer

}

void mouseMove(int x, int y) {

	//TODO Display the bat at current place with current animation frame.
	//For now, it's a square.
	currentX = x;
	currentY = y;
}

//Called every 1000 ms as available, so once per second.
void countDown(int count) {

	//updateTimer(currentTime - 1); //Print timer
	currentTime--; //Decrement time variable
	if (currentTime != 0) {
		glutTimerFunc(1000, countDown, 0);
	}

}

void redraw() { 
	
		glClear(GL_COLOR_BUFFER_BIT); //Clear screen
		updateTimer(currentTime); //Print timer
		//updateScore();

		glBegin(GL_QUADS);
		glColor3f(1, 1, 1);
		glVertex2d(currentX - 5, 480 - currentY - 5);
		glVertex2d(currentX - 5, 480 - currentY + 5);
		glVertex2d(currentX + 5, 480 - currentY + 5);
		glVertex2d(currentX + 5, 480 - currentY - 5);
		glEnd();
		glFlush();

		//Check for game-over condition
		if (currentTime == 0) {
			gameOver = true;
			updateScore();
		}
		//If not game over, and if there are bugs, redraw the bugs.
		else if (!activeBugs.empty()) {
			//Only redraws currently existing bugs.
			for (int i = 0; i < activeBugs.size(); i++) {

				tempVec = activeBugs[i];
				tempVec[2] += tempVec[0];
				tempVec[3] += tempVec[1];

				//Move and redraw bug at new location
				glPushMatrix();
				glTranslated(tempVec[0], tempVec[1], 0);

				glBegin(GL_QUADS);
				glColor3f(1, 0.890, 0.321);
				glVertex2d(tempVec[2], tempVec[3]);
				glVertex2d(tempVec[2] + 5, tempVec[3]);
				glVertex2d(tempVec[2] + 5, tempVec[3] + 5);
				glVertex2d(tempVec[2], tempVec[3] + 5);
				glEnd();
				glFlush();

				glPopMatrix();

				activeBugs[i] = tempVec;

				int bug = getIntersect(currentX, currentY);
				//Check for collision with bug
				if (bug != -1) {
					score++;
					if (i == 0) {
						//The math in else case causes an iterator to the -1st index, which breaks it.
						//So this handles the case where the bug to be removed is at index 0.
						activeBugs.erase(activeBugs.begin());
					}
					else {
						activeBugs.erase(activeBugs.begin() + (bug - 1));
					}
				}

				//remove off-screen bugs
				if (tempVec[2] <= 0 || tempVec[3] <= 0 || tempVec[2] >= 700 || tempVec[3] >= 480) {
					if (i == 0) {
						activeBugs.erase(activeBugs.begin());
					}
					else {
						activeBugs.erase((activeBugs.begin() + (i - 1)));
					}
				}
			}
		}

		//If it's time to generate a new bug, then do that and set a new time to generate a bug.
		if (lastBugGenerated - currentTime == bugCountDown) {
			generateBug();
			bugCountDown = rand() % 4;
			lastBugGenerated = currentTime;
		}
		glutPostRedisplay(); //Prompt screen to redraw based on what just happened.

}

void generateBug() {

	//generates a new bug.
	std::vector<int> bugVector; //0: x dir, 1: y dir, 2: start x, 3: start y
	bugVector.push_back(rand() % 9 + (-4)); // random x dir from 0 to 4
	bugVector.push_back(rand() % 9 + (-4)); // same for y
	bugVector.push_back(rand() % 701); //random start position. TODO: if there's time, fix it so they only come from an edge.
	bugVector.push_back(rand() % 481);
	
	activeBugs.push_back(bugVector); //Adds new bug to the bug vector.

	//Draw bug at its start location.
	glBegin(GL_QUADS);
	glColor3f(1, 0.890, 0.321); //Yellow
		glVertex2d(bugVector[2], bugVector[3]);
		glVertex2d(bugVector[2] + 5, bugVector[3]);
		glVertex2d(bugVector[2] + 5, bugVector[3] + 5);
		glVertex2d(bugVector[2], bugVector[3] + 5);
	glEnd();
	glFlush();
}

int getIntersect(int x, int y) {

	//Checks for collision between player and bugs, and returns the position of the bug in the bug array. 
	//Returns -1 if no collision is found.
	if (!activeBugs.empty()) {
		for (int i = 0; i < activeBugs.size(); i++) {
			tempVec = activeBugs[i];
			if ((x - 10) <= tempVec[2] && tempVec[2] <= (x + 5)) { // The +s are for current sizes of bat and bug
				if ((480-y-10)<= tempVec[3] && tempVec[3]<=(480-y+5)) {
					return i; //Returns bug.
				}
			}
		}
	}
	return -1; //i.e. no bug collisions.

}

//Sets up and updates the timer character array.
void updateTimer(int newTime) {

	if (newTime < 100) {
		//set a leading 0
		timestring[6] = '0';
		timestring[7] = '0' + (newTime % 100) / 10;
		timestring[8] = '0' + newTime % 10; 
	}
	else if (newTime < 10) {
		//set 2 leading 0s
		timestring[6] = '0';
		timestring[7] = '0';
		timestring[8] = '0' + newTime;
	}
	else {
		//no leading 0s **
		timestring[6] = '0' + newTime / 100;
		timestring[7] = '0' + (newTime % 100) / 10;
		timestring[8] = '0' + newTime % 10;
	}

	glRasterPos2f(10, 460); //Position of text
	glColor3f(0, 0, 0); //Color of text - white. TODO For some reason this doesn't always work.
	for (int i = 0; i < 9; i++) {
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, timestring[i]); //writes the current char
	}
	glEnd();
	glFlush();

}

//Sets up and updates the score character array.
void updateScore() {

	if (score < 10) {
		//set 2 leading 0s
		scorestring[7] = '0';
		scorestring[8] = '0';
		scorestring[9] = '0' + score;
	}
	else if (score < 100) {
		//set a leading 0
		scorestring[7] = '0';
		scorestring[8] = '0' + (score % 100) / 10;
		scorestring[9] = '0' + score % 10;
	}
	else {
		//no leading 0s **
		scorestring[7] = '0' + score / 100;
		scorestring[8] = '0' + (score % 100) / 10;
		scorestring[9] = '0' + score % 10;
	}

	glRasterPos2f(330, 235); //Position of text
	glColor3f(0, 0, 0); //Color of text - white. TODO For some reason this doesn't always work.
	for (int i = 0; i < 10; i++) {
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, scorestring[i]); //writes the current char
	}
	glEnd();
	glFlush();

}