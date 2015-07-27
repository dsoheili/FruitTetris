/*
CMPT 361 Assignment 1 - FruitTetris implementation Sample Skeleton Code

- This is ONLY a skeleton code showing:
How to use multiple buffers to store different objects
An efficient scheme to represent the grids and blocks

- Compile and Run:
Type make in terminal, then type ./FruitTetris

This code is extracted from Connor MacLeod's (crmacleo@sfu.ca) assignment submission
by Rui Ma (ruim@sfu.ca) on 2014-03-04. 

Modified in Sep 2014 by Honghua Li (honghual@sfu.ca).

Modified by Daniel Soheili for the purpose of assignment 1 for CMPT 361
301163609
dsoheili@sfu.ca
Feb. 6th 2015


*/

#include "include/Angel.h"
#include <cstdlib>
#include <iostream>


using namespace std;


// xsize and ysize represent the window size - updated if window is reshaped to prevent stretching of the game
int xsize = 400; 
int ysize = 720;

// current tile
vec2 tile[4]; // An array of 4 2d vectors representing displacement from a 'center' piece of the tile, on the grid
vec2 tilepos = vec2(5, 19); // The position of the current tile using grid coordinates ((0,0) is the bottom left corner)

// An array storing all possible orientations of all possible tiles
// The 'tile' array will always be some element [i][j] of this array (an array of vec2)
vec2 allRotationsIshape[4][4] = 
	{{vec2(-2, 0), vec2(-1, 0), vec2(0, 0), vec2(1, 0)},
	{vec2(0, 1), vec2(0, 0), vec2(0,-1), vec2(0, -2)},
	{vec2(-2, 0), vec2(-1, 0), vec2(0, 0), vec2(1, 0)},
	{vec2(0, 1), vec2(0, 0), vec2(0,-1), vec2(0, -2)}};

vec2 allRotationsSshape[4][4] = 
	{{vec2(-1, -1), vec2(0,-1), vec2(0, 0), vec2(1,0)},
	{vec2(0, 1), vec2(0, 0), vec2(1, 0), vec2(1, -1)},
	{vec2(-1, -1), vec2(0,-1), vec2(0, 0), vec2(1,0)},
	{vec2(0, 1), vec2(0, 0), vec2(1, 0), vec2(1, -1)}};

vec2 allRotationsLshape[4][4] = 
	{{vec2(0, 0), vec2(-1,0), vec2(1, 0), vec2(-1,-1)},
	{vec2(0, 1), vec2(0, 0), vec2(0,-1), vec2(1, -1)},     
	{vec2(1, 1), vec2(-1,0), vec2(0, 0), vec2(1,  0)},  
	{vec2(-1,1), vec2(0, 1), vec2(0, 0), vec2(0, -1)}};

int allShapes[3] = {0,  //I
					1,  //S
					2}; //L


// colors
vec4 purple = vec4(1.0, 0.0, 1.0, 1.0);
vec4 red 	= vec4(1.0, 0.0, 0.0, 1.0); 
vec4 yellow = vec4(1.0, 1.0, 0.0, 1.0); 
vec4 green  = vec4(0.0, 1.0, 0.0, 1.0); 
vec4 orange = vec4(1.0, 0.5, 0.0, 1.0);  
vec4 white  = vec4(1.0, 1.0, 1.0, 1.0);
vec4 black  = vec4(0.0, 0.0, 0.0, 1.0);

vec4 AllColours[5] = {purple, red, yellow, green, orange};

// Global variables
int randomColour; //Index of a randomly generated colour from AllColours array
int newShape;     // Keeps track of the shape of the currently falling block
int rotOrient;	  // Keeps track of the rotation orientation of the currently falling block
vec4 currentColours[4]; // Keeps track of the colours of the currently falling block for shuffling
int xPosition[4]; // Keeps track of the current X position of the falling block's pieces on the board
int yPosition[4]; // Keeps track of the current Y position of the falling block's pieces on the board
int timeInterval = 361;
bool askedForTimeInterval = false;

 
//board[x][y] represents whether the cell (x,y) is occupied
bool board[10][20]; 

//An array containing the colour of each of the 10*20*2*3 vertices that make up the board
//Initially, all will be set to black. As tiles are placed, sets of 6 vertices (2 triangles; 1 square)
//will be set to the appropriate colour in this array before updating the corresponding VBO
vec4 boardcolours[1200];

// location of vertex attributes in the shader program
GLuint vPosition;
GLuint vColor;

// locations of uniform variables in shader program
GLuint locxsize;
GLuint locysize;

// VAO and VBO
GLuint vaoIDs[3]; // One VAO for each object: the grid, the board, the current piece
GLuint vboIDs[6]; // Two Vertex Buffer Objects for each VAO (specifying vertex positions and colours, respectively)

//-------------------------------------------------------------------------------------------------------------------

// When the current tile is moved or rotated (or created), update the VBO containing its vertex position data
void updatetile()
{
	// Bind the VBO containing current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]); 

	// For each of the 4 'cells' of the tile,
	for (int i = 0; i < 4; i++) 
	{
		// Calculate the grid coordinates of the cell
		GLfloat x = tilepos.x + tile[i].x; 
		GLfloat y = tilepos.y + tile[i].y;

		// Create the 4 corners of the square - these vertices are using location in pixels
		// These vertices are later converted by the vertex shader
		vec4 p1 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1); 
		vec4 p2 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);
		vec4 p3 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1);
		vec4 p4 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);

		// Two points are used by two triangles eac
		vec4 newpoints[6] = {p1, p2, p3, p2, p3, p4}; 

		xPosition[i] = (int)x; // Maintain information on the whereabouts of the tile
		yPosition[i] = (int)y; // Have to include "(int)" because the file was previously stored as a float

		// Put new data in the VBO
		glBufferSubData(GL_ARRAY_BUFFER, i*6*sizeof(vec4), 6*sizeof(vec4), newpoints); 
	}

	glBindVertexArray(0);
}

//-------------------------------------------------------------------------------------------------------------------

// Called at the start of play and every time a tile is placed
void newtile()
{
	tilepos = vec2(rand() % 8+1, 19); // Put the tile at the top of the board

	newShape = rand() % 3; // randomly select a number from 0 to 2
							   // the value is tied in to a block
	rotOrient = rand() % 4; //random rotation orientation

	if (newShape == allShapes[0]){ // shape I
		// Update the geometry VBO of current tile
		for (int i = 0; i < 4; i++)
			tile[i] = allRotationsIshape[rotOrient][i]; // Get the 4 pieces of the new tile
		updatetile(); 
	}

	else if (newShape == allShapes[1]){ // shape S
		// Update the geometry VBO of current tile
		for (int i = 0; i < 4; i++)
			tile[i] = allRotationsSshape[rotOrient][i]; // Get the 4 pieces of the new tile
		updatetile(); 
	}

	else { // shape L
		// Update the geometry VBO of current tile
		for (int i = 0; i < 4; i++)
			tile[i] = allRotationsLshape[rotOrient][i]; // Get the 4 pieces of the new tile
		updatetile(); 
	}



	// Update the color VBO of current tile

	randomColour = rand() % 5;
	int tracker = 0;
	int colourNum = 0;
	vec4 newcolours[24];
	for (int i = 0; i < 24; i++){
		newcolours[i] = AllColours[randomColour]; // You should randomlize the color
		currentColours[colourNum] = randomColour;
		tracker++; //A new clour every 6 elements, and this helps up keep track
		if (tracker > 5){
			currentColours[colourNum] = AllColours[randomColour];
			randomColour = rand() % 5;
			tracker = 0;
			colourNum++;
		}
	}
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

}

//-------------------------------------------------------------------------------------------------------------------

void initGrid()
{
	// ***Generate geometry data
	vec4 gridpoints[64]; // Array containing the 64 points of the 32 total lines to be later put in the VBO
	vec4 gridcolours[64]; // One colour per vertex
	// Vertical lines 
	for (int i = 0; i < 11; i++){
		gridpoints[2*i] = vec4((33.0 + (33.0 * i)), 33.0, 0, 1);
		gridpoints[2*i + 1] = vec4((33.0 + (33.0 * i)), 693.0, 0, 1);
		
	}
	// Horizontal lines
	for (int i = 0; i < 21; i++){
		gridpoints[22 + 2*i] = vec4(33.0, (33.0 + (33.0 * i)), 0, 1);
		gridpoints[22 + 2*i + 1] = vec4(363.0, (33.0 + (33.0 * i)), 0, 1);
	}
	// Make all grid lines white
	for (int i = 0; i < 64; i++)
		gridcolours[i] = white;


	// *** set up buffer objects
	// Set up first VAO (representing grid lines)
	glBindVertexArray(vaoIDs[0]); // Bind the first VAO
	glGenBuffers(2, vboIDs); // Create two Vertex Buffer Objects for this VAO (positions, colours)

	// Grid vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]); // Bind the first grid VBO (vertex positions)
	glBufferData(GL_ARRAY_BUFFER, 64*sizeof(vec4), gridpoints, GL_STATIC_DRAW); // Put the grid points in the VBO
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0); 
	glEnableVertexAttribArray(vPosition); // Enable the attribute
	
	// Grid vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]); // Bind the second grid VBO (vertex colours)
	glBufferData(GL_ARRAY_BUFFER, 64*sizeof(vec4), gridcolours, GL_STATIC_DRAW); // Put the grid colours in the VBO
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor); // Enable the attribute
}


void initBoard()
{
	// *** Generate the geometric data
	vec4 boardpoints[1200];
	for (int i = 0; i < 1200; i++)
		boardcolours[i] = black; // Let the empty cells on the board be black
	// Each cell is a square (2 triangles with 6 vertices)
	for (int i = 0; i < 20; i++){
		for (int j = 0; j < 10; j++)
		{		
			vec4 p1 = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), .5, 1);
			vec4 p2 = vec4(33.0 + (j * 33.0), 66.0 + (i * 33.0), .5, 1);
			vec4 p3 = vec4(66.0 + (j * 33.0), 33.0 + (i * 33.0), .5, 1);
			vec4 p4 = vec4(66.0 + (j * 33.0), 66.0 + (i * 33.0), .5, 1);
			
			// Two points are reused
			boardpoints[6*(10*i + j)    ] = p1;
			boardpoints[6*(10*i + j) + 1] = p2;
			boardpoints[6*(10*i + j) + 2] = p3;
			boardpoints[6*(10*i + j) + 3] = p2;
			boardpoints[6*(10*i + j) + 4] = p3;
			boardpoints[6*(10*i + j) + 5] = p4;
		}
	}

	// Initially no cell is occupied
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 20; j++)
			board[i][j] = false; 


	// *** set up buffer objects
	glBindVertexArray(vaoIDs[1]);
	glGenBuffers(2, &vboIDs[2]);

	// Grid cell vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[2]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardpoints, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Grid cell vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

// No geometry for current tile initially
void initCurrentTile()
{
	glBindVertexArray(vaoIDs[2]);
	glGenBuffers(2, &vboIDs[4]);

	// Current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Current tile vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

void init()
{
	// Load shaders and use the shader program
	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	// Get the location of the attributes (for glVertexAttribPointer() calls)
	vPosition = glGetAttribLocation(program, "vPosition");
	vColor = glGetAttribLocation(program, "vColor");

	// Create 3 Vertex Array Objects, each representing one 'object'. Store the names in array vaoIDs
	glGenVertexArrays(3, &vaoIDs[0]);

	// Initialize the grid, the board, and the current tile
	initGrid();
	initBoard();
	initCurrentTile();

	// The location of the uniform variables in the shader program
	locxsize = glGetUniformLocation(program, "xsize"); 
	locysize = glGetUniformLocation(program, "ysize");

	// Game initialization
	newtile(); // create new next tile

	// set to default
	glBindVertexArray(0);
	glClearColor(0, 0, 0, 0);
}

//-------------------------------------------------------------------------------------------------------------------

// Rotates the current tile, if there is room
void rotate()
{      
	//update orientation, limit it to 0-3
	rotOrient ++;
	if (rotOrient >= 4){
		rotOrient = 0;
	}

	//redraw block to rotate

	if (newShape == allShapes[0]){ // shape I
		// Update the geometry VBO of current tile
		for (int i = 0; i < 4; i++)
			tile[i] = allRotationsIshape[rotOrient][i]; // Get the 4 pieces of the new tile
		updatetile(); 
	}

	else if (newShape == allShapes[1]){ // shape S
		// Update the geometry VBO of current tile
		for (int i = 0; i < 4; i++)
			tile[i] = allRotationsSshape[rotOrient][i]; // Get the 4 pieces of the new tile
		updatetile(); 
	}

	else { // shape L
		// Update the geometry VBO of current tile
		for (int i = 0; i < 4; i++)
			tile[i] = allRotationsLshape[rotOrient][i]; // Get the 4 pieces of the new tile
		updatetile(); 
	}

}

//-------------------------------------------------------------------------------------------------------------------

// Checks if the specified row (0 is the bottom 19 the top) is full
// If every cell in the row is occupied, it will clear that cell and everything above it will shift down one row
void checkfullrow(int row)
{
	bool fullRow = true;
	for (int i = 0; i < 10; i++){
		if (board[i][row] == false) // Check if each individual tile of the row is occupied or not
			fullRow = false;
	}
	if (fullRow == true){ // If a row is full then shift everything down by one

		glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);

		for (int i = row; i < 19; i++){ // Shift down all the bool values for occupied spaces
			for (int j = 0; j < 10; j++){
				board[j][i] = board[j][i+1];
			}
		}
		for (int i = row; i < 19; i++){ // Shift down the colours
			for (int j = 0; j < 10; j++){
				boardcolours[6*((10*i) + j) + 0] = boardcolours[6*((10*(i+1)) + j) + 0];
				boardcolours[6*((10*i) + j) + 1] = boardcolours[6*((10*(i+1)) + j) + 1];
				boardcolours[6*((10*i) + j) + 2] = boardcolours[6*((10*(i+1)) + j) + 2];
				boardcolours[6*((10*i) + j) + 3] = boardcolours[6*((10*(i+1)) + j) + 3];
				boardcolours[6*((10*i) + j) + 4] = boardcolours[6*((10*(i+1)) + j) + 4];
				boardcolours[6*((10*i) + j) + 5] = boardcolours[6*((10*(i+1)) + j) + 5];
			}
		}
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(boardcolours), boardcolours);
		glBindVertexArray(0);	
	}
}

//-------------------------------------------------------------------------------------------------------------------

// Each square consists of 6 nodes. (2 triangles with 3 nodes each)
// Since colours are type vec4, this function checks if two nodes have the same vec4 values
// vec4 has x, y, z, w values

bool sameNodeColour(vec4 colour1, vec4 colour2){
	
	bool matches = true;

	if ( colour1.x != colour2.x ) 
		matches = false;

	else if ( colour1.y != colour2.y )
		matches = false;

	else if ( colour1.z != colour2.z )
		matches = false;

	else if ( colour1.w != colour2.w )
		matches = false;

	return matches;
}


//-------------------------------------------------------------------------------------------------------------------

// Now we can use the function defined previously to check for three matching colours

bool threeMatchingBlocks(vec4 colour1, vec4 colour2, vec4 colour3){

	bool matches = false;

	if ( sameNodeColour(colour1, colour2) && sameNodeColour(colour1, colour3) )
		matches = true;

	return matches;
}

//-------------------------------------------------------------------------------------------------------------------

// For each block that is set, check for each piece of it that the spaces next to it are full or not
// If true, then check if they have the same colour
// If they do, then do the same thing that was done on checkfullrow
// ie. shift everything above those three blocks down, and update the colours
void checkFruit(){
/*
	bool rightNotEmpty = false;
	bool leftNotEmpty = false;
	//bool downNotEmpty = false;
	//bool colourMatch = false;

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);

	for (int k = 0; k < 4; k++){


		int xneg2 = tilepos.x-2 + tile[k].x;
		int xneg1 = tilepos.x-1 + tile[k].x; 
		int x0 = tilepos.x+0 + tile[k].x;
		int x1 = tilepos.x+1 + tile[k].x;
		int x2 = tilepos.x+2 + tile[k].x;  
		int y = tilepos.y + tile[k].y;

		// Check the right side of each piece of the set tile and see if it is empty or not
		// Limit it to position 7, because if it is on position 8 (of 9) then it can't have
		// two blocks next to it.

		if ( (board[x1][y] == true) && (board[x2][y] == true) && (tilepos.x <8) )
			rightNotEmpty = true;

		// Check the colours of the tiles on the right side of the placed piece
		// First check if the two tiles next to it are empty
		// If they are occupied, compare the colours
		if (rightNotEmpty == true){
			int colour1 = 6*((10*x0) + y);
			int colour2 = 6*((10*x1) + y);
			int colour3 = 6*((10*x2) + y);


			// Since the colours are vec4 type, we need to compare each element of the vec4 using our previous function
			if (threeMatchingBlocks(boardcolours[colour1], boardcolours[colour2], boardcolours[colour3])){
				
				// Empty the blocks that contained the squares with matching colours

				board[x0][y] = false;
				board[x1][y] = false;
				board[x2][y] = false;

				// Shift every occupation down by one

				for (int h = y; h < 19; h++){
					board[x0][h] = board[x0][h+1];
					board[x1][h] = board[x1][h+1];
					board[x2][h] = board[x2][h+1];
				}

				// Update the colours by shifting everything above them down by one
				for (int i = y; i < 19; i++){
					for (int j = tilepos.x; j < tilepos.x+3; j++){
						boardcolours[6*((10*i) + j) + 0] = boardcolours[6*((10*(i+1)) + j) + 0];
						boardcolours[6*((10*i) + j) + 1] = boardcolours[6*((10*(i+1)) + j) + 1];
						boardcolours[6*((10*i) + j) + 2] = boardcolours[6*((10*(i+1)) + j) + 2];
						boardcolours[6*((10*i) + j) + 3] = boardcolours[6*((10*(i+1)) + j) + 3];
						boardcolours[6*((10*i) + j) + 4] = boardcolours[6*((10*(i+1)) + j) + 4];
						boardcolours[6*((10*i) + j) + 5] = boardcolours[6*((10*(i+1)) + j) + 5];
					}
				}


			}
			rightNotEmpty = false; // Set it to false for the next iteration
			leftNotEmpty = false; // Set this to false as wel in case there is a match on the left side too
			
		}


	}
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(boardcolours), boardcolours);
	glBindVertexArray(0);
*/
}


//-------------------------------------------------------------------------------------------------------------------

//
void checkFruitSet(){
	/*for (int i = 0; i < 10; i++ ){
		for (int j = 0; j < 20; j++){
			if ( (board[i][j] == true) && (board[i+1][j] == true) && (board[i+2][j] == true) ){
				vec4 colour1 = boardcolours[6*((10*j) + i)];
				vec4 colour2 = boardcolours[6*((10*j) + (i+1))];
				vec4 colour3 = boardcolours[6*((10*j) + (i+2))];
				if (threeMatchingBlocks(colour1, colour2, colour3))
					
			}
		}
	}*/
}

//-------------------------------------------------------------------------------------------------------------------

// Places the current tile - update the board vertex colour VBO and the array maintaining occupied cells
void settile(int data)
{

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]); // Bind the VBO containing current tile vertex colours

	// Colour the board similar to the way it was done on initBoard() but with the tile colours
	for (int i = 0; i < 4; i++){

		// (From updateTile)
		// Calculate the grid coordinates of the cell
		int x = tilepos.x + tile[i].x; 
		int y = tilepos.y + tile[i].y;

		// (From initBoard)
		// Paint the board with eac of the block's tiles/squares
		boardcolours[6*(10*y + x) + 0] = currentColours[i];
		boardcolours[6*(10*y + x) + 1] = currentColours[i];
		boardcolours[6*(10*y + x) + 2] = currentColours[i];
		boardcolours[6*(10*y + x) + 3] = currentColours[i];
		boardcolours[6*(10*y + x) + 4] = currentColours[i];
		boardcolours[6*(10*y + x) + 5] = currentColours[i];

		// Now set the values on board[][] to true to indicatee the blocks are occupied
		board[x][y] = true;

	}
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(boardcolours), boardcolours);
	glBindVertexArray(0);
	for (int i = 0; i < 20; i++){
		checkfullrow(i);
		
	}
	checkFruit();

}

//-------------------------------------------------------------------------------------------------------------------

// Given (x,y), tries to move the tile x squares to the right and y squares down
// Returns true if the tile was successfully moved, or false if there was some issue
bool movetile(vec2 direction)
{

	bool canMove = true;

	for ( int i = 0; i < 4; i++){
		int futureX = (tilepos.x)+(direction.x)+(tile[i].x); // Check how it would react to the tile
		int futureY = (tilepos.y)+(direction.y)+(tile[i].y); // in the requested position

		// set boundaries, ie. the three sides of the board and check for other existing blocks on the board
		if ((board[futureX][futureY] == true) || (futureX<0) || (futureX>9) || (futureY<0))
			canMove = false;
	}
	if (canMove == true){
		tilepos.x = tilepos.x+direction.x;
		tilepos.y = tilepos.y+direction.y;
	}
	updatetile();
	return canMove;
}

//-------------------------------------------------------------------------------------------------------------------

//moves the colours of the tile by one block
void shuffle()
{
			vec4 tempcolor = currentColours[0]; // store thefirst colour before the moving process to assign it to the last one
			for (int i = 0; i < 3; i++){		//a‐>b, b‐>c, c‐>d, d‐>a colour swap
				currentColours[i] = currentColours[i+1];
			}
			currentColours[3] = tempcolor;

			if (newShape == allShapes[0]){ // shape I
				// Update the geometry VBO of current tile
				for (int i = 0; i < 4; i++)
					tile[i] = allRotationsIshape[rotOrient][i]; // Get the 4 pieces of the new tile
				updatetile(); 
			}

			else if (newShape == allShapes[1]){ // shape S
				// Update the geometry VBO of current tile
				for (int i = 0; i < 4; i++)
					tile[i] = allRotationsSshape[rotOrient][i]; // Get the 4 pieces of the new tile
				updatetile(); 
			}

			else { // shape L
				// Update the geometry VBO of current tile
				for (int i = 0; i < 4; i++)
					tile[i] = allRotationsLshape[rotOrient][i]; // Get the 4 pieces of the new tile
				updatetile(); 
			}


			// Update the color VBO of current tile
			// Place the stored colurs back into the tiles after shifting them by one
			vec4 newcolours[24];

			for (int i = 0; i < 6; i++){
			newcolours[i] = currentColours[0];
			}
			for (int i = 6; i < 12; i++){
			newcolours[i] = currentColours[1];
			}
			for (int i = 12; i < 18; i++){
			newcolours[i] = currentColours[2];
			}
			for (int i = 18; i < 24; i++){
			newcolours[i] = currentColours[3];
			}

			glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glBindVertexArray(0);


}

//-------------------------------------------------------------------------------------------------------------------

void gravity(int timeInterval)
{
		tilepos.y = tilepos.y-1; // Lower the current block's position on the y-axis by one
		updatetile();            // Update to show the changes
		glutTimerFunc(timeInterval,gravity, timeInterval);  // Use timer to repeat it on a timer
		glutPostRedisplay();			// Show the changes to the player
	
}
//-------------------------------------------------------------------------------------------------------------------

// Starts the game over - empties the board, creates new tiles, resets line counters
void restart()
{
	initBoard(); // re-initialize the game board
	newtile();   // drop a new tile for the player

}
//-------------------------------------------------------------------------------------------------------------------

// Draws the game
void display()
{

	glClear(GL_COLOR_BUFFER_BIT);

	glUniform1i(locxsize, xsize); // x and y sizes are passed to the shader program to maintain shape of the vertices on screen
	glUniform1i(locysize, ysize);

	glBindVertexArray(vaoIDs[1]); // Bind the VAO representing the grid cells (to be drawn first)
	glDrawArrays(GL_TRIANGLES, 0, 1200); // Draw the board (10*20*2 = 400 triangles)

	glBindVertexArray(vaoIDs[2]); // Bind the VAO representing the current tile (to be drawn on top of the board)
	glDrawArrays(GL_TRIANGLES, 0, 24); // Draw the current tile (8 triangles)

	glBindVertexArray(vaoIDs[0]); // Bind the VAO representing the grid lines (to be drawn on top of everything else)
	glDrawArrays(GL_LINES, 0, 64); // Draw the grid lines (21+11 = 32 lines)


	glutSwapBuffers();
}

//-------------------------------------------------------------------------------------------------------------------

// Reshape callback will simply change xsize and ysize variables, which are passed to the vertex shader
// to keep the game the same from stretching if the window is stretched
void reshape(GLsizei w, GLsizei h)
{
	xsize = w;
	ysize = h;
	glViewport(0, 0, w, h);
}

//-------------------------------------------------------------------------------------------------------------------

// Handle arrow key keypresses
void special(int key, int x, int y)
{
	switch(key) 
	{
		case GLUT_KEY_UP: //rotate
		    rotate();
		    break;
		case GLUT_KEY_RIGHT: //move right
			movetile(vec2(1,0));
			break;
		case GLUT_KEY_LEFT: //move left
			movetile(vec2(-1,0));
			break;
		case GLUT_KEY_DOWN:
			movetile(vec2(0,-1)); //descend block
			break;
	}
	updatetile();
	glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

// Handles standard keypresses
void keyboard(unsigned char key, int x, int y)
{
	switch(key) 
	{
		case 033: // Both escape key and 'q' cause the game to exit
		    exit(EXIT_SUCCESS);
		    break;
		case 'q':
			exit (EXIT_SUCCESS);
			break;
		case 'r': // 'r' key restarts the game
			restart();
			break;
		case ' ': //shuffle
			shuffle();
			break;


	}
	glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

// On Idle we check if the space where the block is headed is occupied or not
// if it is, then we call the settile function to set it
void idle(void)
{
	bool endGame = false;

	for (int j = 0; j < 10; j++){
		if (board[j][19] == true)
			endGame = true;
	}


	if ( endGame == false ){
		for (int i = 0; i < 4; i++){
			if ((yPosition[i]) == 0){ // If the block is at the bottom of the board, place it and call newtile
				settile(0);
				newtile();
				break;
			}
			if (board[xPosition[i]][yPosition[i]-1] == true){ // Check if the square below on the board is occupied
				settile(0);	// if it is, then set the block on top of it and call newtile
				newtile();
				break;
			}
		}
	}
	glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	if (askedForTimeInterval == false){
		cout << "The higher the value of the interval of descent, the slower the block falls.\n";
		cout << "Please enter the interval of descent of blocks (from 1 - 1000): ";
		cin >> timeInterval;
		if (timeInterval < 1)
			cout << "please enter a value between 1-1000.";
		else if (timeInterval >= 1)
			askedForTimeInterval == true;
	}


	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(xsize, ysize);
	glutInitWindowPosition(680, 178); // Center the game window (well, on a 1920x1080 display)
	glutCreateWindow("Fruit Tetris");
	glewInit();
	init();
	srand(time(NULL));
	glutTimerFunc(timeInterval,gravity, timeInterval);
	checkFruit();

	// Callback functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(special);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);

	glutMainLoop(); // Start main loop
	return 0;
}
