// Assignment 2 Release Version.cpp: A program using the TL-Engine
//JStone3 Fun in the Garden Assignment.



#include <TL-Engine.h>	// TL-Engine include file and namespace
using namespace tle;
#include <math.h>
#include <iostream>
#include <sstream>

//Function used to reset the bullet and the dummy model in this case to the starting position after a collision has occured or the bullet moves past the back boundary.
void BulletStartPosition(IModel* bullet, float xPosition, float yPosition, float zPosition);
//Function used to reset the camera to its starting position and orientation.
void CameraStartPosition(ICamera* camera, float xPosition, float yPosition, float zPosition, float rotation);
//Function used for collision detection (Sphere-Sphere) between the marble the mushrooms and the worm. 
float CollisionDetection(IModel* mushrooms, IModel* bullet);
//Function used to create mushrooms after a worm segment has been destroyed. 
//The mushroom will be created using the worms current position.
//The mushroom model is passed using reference otherwise a new mushroom cannot be created.
void mushroomCreation(IModel* worm, IModel*& mushroom, IMesh* mushroomMesh, float mushroomYPosition);
//Struct to hold information for barriers.
struct Barriers
{
	IModel* barrierModel;
	bool skinHatched;
};


void main()
{
	// Create a 3D engine (using TLX engine here) and open a window for it
	I3DEngine* myEngine = New3DEngine(kTLX);
	myEngine->StartWindowed();

	// Add default folder for meshes and other media
	myEngine->AddMediaFolder(".\\Media");

	/**** Set up your scene here ****/
	//Create a font model to display font to the user when the game is over.
	IFont* gameOverFont = myEngine->LoadFont("Arial", 82);
	IFont* shotsFiredFont = myEngine->LoadFont("Arial", 36);
	IFont* mushroomsDestroyedFont = myEngine->LoadFont("Arial", 36);
	IFont* wormsDestroyedFont = myEngine->LoadFont("Arial", 36);
	stringstream shotsFired;
	stringstream mushroomsDestroyed;
	stringstream wormsDestroyed;
	int firedShots = 0;
	int destroyedMushrooms = 0;
	int destroyedWorms = 0;
	//Game state variables and the starting game state is set to ready
	enum gameStates { ready, firing, contact, over };
	gameStates gameState = ready;
	//Mushroom and worm contact states to record how many times each object has been hit and determine when to remove the object or create a new one.
	enum contactState { notHit, oneHit, twoHit, threeHit };
	//2nd mushroom state variable used to control the rising and falling of the mushrooms.
	enum mushroomAppearance { rising, falling, stationary, deleted };
	const int kNumMushrooms = 20;//Total number of mushrooms.
	const int kNumBarriers = 22;//Total number of barriers, used to indicate boundary of playing area.
	const int kNumInitialMushrooms = 9;//Number of mushrooms that are present from the start of the game.
	const int kNumWormParts = 10;//Number of worm parts.
	const int mushroomWormOffSet = 10;//This off set will be used when creating mushrooms after a worm part has been destroyed. 
	contactState mushroomState[kNumMushrooms];//Mushroom state variables are created for each mushroom. Controls how many times each mushroom has been hit.
	contactState wormState[kNumWormParts];//Worm state variables are created for each worm segment. Controls how many times each worm segment has been hit.
	mushroomAppearance mushroomPosition[kNumMushrooms];//Second set of mushroom state variables which will be used to control the rising and falling of each individual mushroom 
	//All speed values.
	float kGameSpeed = 500.f;//Every value of movement is calculated in relation to kGameSpeed.
	const float mushroomAscendSpeed = 100;//Variable used to store the speed that the mushrooms ascend at.
	const float mushroomDescendSpeed = -100;//Variable used to store the speed that the mushrooms ascend at.
	const float marbleRotationSpeed = 20.f;//Variable to store speed at which marble is rotated.
	const float marbleStrafeSpeed = 20.f;//Variable to store the speed that the marble moves when strafing.
	const float xMaxLimit = 80.f;//Upper bound of X axis movement.
	const float xMinLimit = -80.f;//Lower bound of X axis movement.
	const float mushroomRadiusAddMarbleRadius = 6.1f;//This is the total of the mushroom and marble radii added together. This value is used to determine whether a collision between marble and mushroom has occured.
	const float wormSegmentAddMarbleRadius = 7.f;//This is the total of the mushroom and worm segment radii added together.This value is used to determine whether a collision between marble and worm has occured.
	const float zMaxLimit = 210;//Upper bound of Z axis movement. 
	const float zMinLimit = 0;//Lower bound of Z axis movement.
	float collisionDist;//Variable that will be used to store the value returned from the collision detection function. This variable will be used to determine if a collision has occured. 
	const int wormGameOverCounter = 10;
	//Variables to store mesh and skin values.
	const string marbleGreenSkin = "glass_green.jpg";
	const string marbleRedSkin = "glass_red.jpg";
	const string barrierSkin1 = "barrier1.bmp";
	const string barrierSkin2 = "barrier1a.bmp";
	const string bushModel = "Bush.x";
	const string mushroomModel = "Mushroom.x";
	const string wormModel = "Segment.x";
	const string barrierModel = "Barrier.x";
	const string mushroomSkin = "mushroom.png";
	const string mushroomSkinHot = "mushroom_hot.png";
	const string mushroomSkinVeryHot = "mushroom_very_hot.png";
	const string mushroomSkinPurple = "purple_mushroom.png";
	const string mushroomSkinPurpleHot = "purple_mushroom_hot.png";
	const string wormSkin = "wormskin.jpg";
	const string wormSkinHot = "wormskin_hot.jpg";
	const string wormSkinVeryHot = "wormskin_very_hot.jpg";
	const float yMaxRotateLimit = 50.f;//Upper bound of marble rotation.
	const float yMinRotateLimit = -50.f;//Lower bound of marble rotation.
	const float yMaxCameraLimit = 80.f;//Upper bound of camera Y axis movement.
	const float yMinCameraLimit = 70.f;//Lower bound of camera Y axis movement.
	const float mushroomStationaryPoint = 0.f;//Y axis position where the mushrooms are in the correct starting position after rising upwards.
	const float mushroomDeletionPoint = -8.f;//Y axis position where the mushrooms move down to and are deleted after being hit three times. 
	const float rotateCWLimit = 50.f;//Upper limit of marble rotation.
	const float rotateCCWLimit = -50.f;//Lower limit of marble rotation.
	float rotateValue = 0.f;//Variable used to store how far the marble has been rotated.
	const float rightBarrierLimit = 82.f;
	const float leftBarrierLimit = -82.f;
	//Aray storing the starting position of the bullet.
	const float bulletPosition[3] = { 0, 2, 0 };
	//Arry storing the starting position of the camera.
	const float cameraPosition[4] = { 0, 75, -75, 25 };
	const float cameraMovement = 50;
	//Camera
	ICamera* camera = myEngine->CreateCamera(kManual, cameraPosition[0], cameraPosition[1], cameraPosition[2]);
	camera->RotateX(cameraPosition[3]);
	//Floor
	IMesh* floorMesh = myEngine->LoadMesh("Ground.x");
	IModel* floor = floorMesh->CreateModel();
	//Skybox
	IMesh* skyboxMesh = myEngine->LoadMesh("Skybox_SciFi.x");
	IModel* skybox = skyboxMesh->CreateModel(0, -1000, 0);
	//Marbles
	IMesh* marbleMesh = myEngine->LoadMesh("Marble.x");
	IModel* marble0 = marbleMesh->CreateModel(0, 2, 0);
	//Dummy model.
	IMesh* dummyMesh = myEngine->LoadMesh("Dummy.x");
	IModel* dummy = dummyMesh->CreateModel(0, 2, 0);
	//Arrow
	IMesh* arrowMesh = myEngine->LoadMesh("Arrow.x");
	IModel* arrow = arrowMesh->CreateModel(0, 0, -10);
	arrow->AttachToParent(dummy);

	//Creation of two 2 dimensional arrays to hold the co-ordinates that represent the grid that is the playing area.
	//One matrix is used to hold to X co-ordinates.
	int xGridMatrix[20][16];
	//The array is filled with all the x values on the playing grid. The array is filled 1 column at a time.
	//The array starts from the left hand side of the grid, this x value is -75.
	int xCoordinate = -75;
	//This outer loop iteratres through every column in the array.
	for (int xCol = 0; xCol < 16; xCol++)
	{
		//This inner loop interates through every row in the array.
		for (int xRow = 0; xRow < 20; xRow++)
		{
			//This code fills each cell in the array with the corresponding X axis value.
			xGridMatrix[xRow][xCol] = xCoordinate;
		}
		//After every cell in a column has been filled the x co-ordinate increase by 10 as we move across the playing grid.
		xCoordinate += 10;
	}
	//One matrix is used to hold to Z co-ordinates.
	int zGridMatrix[20][16];
	//The array starts from the top of the grid, this z value is 200. This is where the worm starts.
	int zCoordinate = 200;
	//This outer loop iteratres through every column in the array.
	for (int zRow = 0; zRow < 20; zRow++)
	{
		//This inner loop interates through every row in the array.
		for (int zCol = 0; zCol < 16; zCol++)
		{
			//This code fills each cell in the array with the corresponding X axis value.
			zGridMatrix[zRow][zCol] = zCoordinate;
		}
		//After every cell in a column has been filled the x co-ordinate increase by 10 as we move across the playing grid.
		zCoordinate -= 10;
	}
	int wormMovementCounter = 1000;//This counter will determine when the worm needs to move. It will decrease each frame by -1 and when it reaches 0 the worm will move one square.
	const int lHandSideGrid = 0;
	const int rHandSideGrid = 16;
	//Each worm segment has a position that corresponds to its X position on the game grid. 
	int worm0Position = 9;
	int worm1Position = 8;
	int worm2Position = 7;
	int worm3Position = 6;
	int worm4Position = 5;
	int worm5Position = 4;
	int worm6Position = 3;
	int worm7Position = 2;
	int worm8Position = 1;
	int worm9Position = 0;
	//Each worm segment has a row that corresponds to its Z position on the game grid. 
	int worm0CurrentRow = 0;
	int worm1CurrentRow = 0;
	int worm2CurrentRow = 0;
	int worm3CurrentRow = 0;
	int worm4CurrentRow = 0;
	int worm5CurrentRow = 0;
	int worm6CurrentRow = 0;
	int worm7CurrentRow = 0;
	int worm8CurrentRow = 0;
	int worm9CurrentRow = 0;
	//Enum to store the directions that the worm can move.
	enum wormDirection { left, right };
	//All of the worm segments start out the game travelling right. 
	wormDirection worm0Direction = right;
	wormDirection worm1Direction = right;
	wormDirection worm2Direction = right;
	wormDirection worm3Direction = right;
	wormDirection worm4Direction = right;
	wormDirection worm5Direction = right;
	wormDirection worm6Direction = right;
	wormDirection worm7Direction = right;
	wormDirection worm8Direction = right;
	wormDirection worm9Direction = right;

	//Worm
	IMesh* wormMesh = myEngine->LoadMesh(wormModel);
	IModel* worms[kNumWormParts];
	//Worm part 0 (Head)
	worms[0] = wormMesh->CreateModel(15, 10, 200);
	worms[0]->SetSkin(wormSkin);
	wormState[0] = notHit;
	//Worm part 1
	worms[1] = wormMesh->CreateModel(5, 10, 200);
	worms[1]->SetSkin(wormSkin);
	wormState[1] = notHit;
	//Worm part 2
	worms[2] = wormMesh->CreateModel(-5, 10, 200);
	worms[2]->SetSkin(wormSkin);
	wormState[2] = notHit;
	//Worm part 3
	worms[3] = wormMesh->CreateModel(-15, 10, 200);
	worms[3]->SetSkin(wormSkin);
	wormState[3] = notHit;
	//Worm part 4
	worms[4] = wormMesh->CreateModel(-25, 10, 200);
	worms[4]->SetSkin(wormSkin);
	wormState[4] = notHit;
	//Worm part 5
	worms[5] = wormMesh->CreateModel(-35, 10, 200);
	worms[5]->SetSkin(wormSkin);
	wormState[5] = notHit;
	//Worm part 6
	worms[6] = wormMesh->CreateModel(-45, 10, 200);
	worms[6]->SetSkin(wormSkin);
	wormState[6] = notHit;
	//Worm part 7
	worms[7] = wormMesh->CreateModel(-55, 10, 200);
	worms[7]->SetSkin(wormSkin);
	wormState[7] = notHit;
	//Worm part 8
	worms[8] = wormMesh->CreateModel(-65, 10, 200);
	worms[8]->SetSkin(wormSkin);
	wormState[8] = notHit;
	//Worm part 9
	worms[9] = wormMesh->CreateModel(-75, 10, 200);
	worms[9]->SetSkin(wormSkin);
	wormState[9] = notHit;

	//Bushes
	//Bush0
	IMesh* bushMesh = myEngine->LoadMesh(bushModel);
	IModel* bush0 = bushMesh->CreateModel(-20, 0, 210);
	bush0->Scale(30.f);
	bush0->RotateY(25.f);
	//Bush1
	IModel* bush1 = bushMesh->CreateModel(0, 0, 210);
	bush1->Scale(35.f);
	bush1->RotateY(-120.f);
	//Bush2
	IModel* bush2 = bushMesh->CreateModel(20, 0, 210);
	bush2->Scale(40.f);
	bush2->RotateY(50.f);
	//Bush3
	IModel* bush3 = bushMesh->CreateModel(40, 0, 210);
	bush3->Scale(45.f);
	bush3->RotateY(60.f);
	//Bush4
	IModel* bush4 = bushMesh->CreateModel(60, 0, 210);
	bush4->Scale(50.f);
	bush4->RotateY(190.f);
	//Bush5
	IModel* bush5 = bushMesh->CreateModel(-40, 0, 210);
	bush5->Scale(47.f);
	bush5->RotateY(250.f);

	//Mushrooms
	IMesh* mushroomMesh = myEngine->LoadMesh(mushroomModel);
	IModel* mushrooms[kNumMushrooms];
	//Mushroom0
	mushrooms[0] = mushroomMesh->CreateModel(44.1, mushroomDeletionPoint, 19.1);
	mushrooms[0]->SetSkin(mushroomSkin);
	mushroomState[0] = notHit;//Each initial mushroom has its state set to notHit at the start of the game.
	mushroomPosition[0] = rising;//Each initial mushroom second state is set to rising to allow the mushrooms to grow out of the floor.
	//Mushroom1
	mushrooms[1] = mushroomMesh->CreateModel(-44.1, mushroomDeletionPoint, 49.1);
	mushrooms[1]->SetSkin(mushroomSkin);
	mushroomState[1] = notHit;
	mushroomPosition[1] = rising;
	//Mushroom2
	mushrooms[2] = mushroomMesh->CreateModel(-14.1, mushroomDeletionPoint, 59.1);
	mushrooms[2]->SetSkin(mushroomSkin);
	mushroomState[2] = notHit;
	mushroomPosition[2] = rising;
	//Mushroom3
	mushrooms[3] = mushroomMesh->CreateModel(74.1, mushroomDeletionPoint, 69.1);
	mushrooms[3]->SetSkin(mushroomSkin);
	mushroomState[3] = notHit;
	mushroomPosition[3] = rising;
	//Mushroom4
	mushrooms[4] = mushroomMesh->CreateModel(34.1, mushroomDeletionPoint, 89.1);
	mushrooms[4]->SetSkin(mushroomSkin);
	mushroomState[4] = notHit;
	mushroomPosition[4] = rising;
	//Mushroom5
	mushrooms[5] = mushroomMesh->CreateModel(-64.1, mushroomDeletionPoint, 89.1);
	mushrooms[5]->SetSkin(mushroomSkin);
	mushroomState[5] = notHit;
	mushroomPosition[5] = rising;
	//Mushroom6
	mushrooms[6] = mushroomMesh->CreateModel(-24.1, mushroomDeletionPoint, 139.1);
	mushrooms[6]->SetSkin(mushroomSkin);
	mushroomState[6] = notHit;
	mushroomPosition[6] = rising;
	//Mushroom7
	mushrooms[7] = mushroomMesh->CreateModel(14.1, mushroomDeletionPoint, 159.1);
	mushrooms[7]->SetSkin(mushroomSkin);
	mushroomState[7] = notHit;
	mushroomPosition[7] = rising;
	//Mushroom8
	mushrooms[8] = mushroomMesh->CreateModel(54.1, mushroomDeletionPoint, 189.1);
	mushrooms[8]->SetSkin(mushroomSkin);
	mushroomState[8] = notHit;
	mushroomPosition[8] = rising;
	//Mushroom9
	mushrooms[9] = mushroomMesh->CreateModel(-54.1, mushroomDeletionPoint, 189.1);
	mushrooms[9]->SetSkin(mushroomSkin);
	mushroomState[9] = notHit;
	mushroomPosition[9] = rising;
	//Barriers
	IMesh* barrierMesh = myEngine->LoadMesh(barrierModel);
	Barriers barriers[kNumBarriers];

	barriers[0].barrierModel = barrierMesh->CreateModel(84, 0, 0);
	barriers[0].skinHatched;

	barriers[1].barrierModel = barrierMesh->CreateModel(84, 0, 18);
	barriers[1].skinHatched;

	barriers[2].barrierModel = barrierMesh->CreateModel(84, 0, 36);
	barriers[2].skinHatched;

	barriers[3].barrierModel = barrierMesh->CreateModel(84, 0, 54);
	barriers[3].skinHatched;

	barriers[4].barrierModel = barrierMesh->CreateModel(84, 0, 72);
	barriers[4].skinHatched = false;

	barriers[5].barrierModel = barrierMesh->CreateModel(84, 0, 90);
	barriers[5].skinHatched = false;

	barriers[6].barrierModel = barrierMesh->CreateModel(84, 0, 108);
	barriers[6].skinHatched = false;

	barriers[7].barrierModel = barrierMesh->CreateModel(84, 0, 126);
	barriers[7].skinHatched = false;

	barriers[8].barrierModel = barrierMesh->CreateModel(84, 0, 144);
	barriers[8].skinHatched = false;

	barriers[9].barrierModel = barrierMesh->CreateModel(84, 0, 162);
	barriers[9].skinHatched = false;

	barriers[10].barrierModel = barrierMesh->CreateModel(84, 0, 180);
	barriers[10].skinHatched = false;

	barriers[11].barrierModel = barrierMesh->CreateModel(-84, 0, 0);
	barriers[11].skinHatched;

	barriers[12].barrierModel = barrierMesh->CreateModel(-84, 0, 18);
	barriers[12].skinHatched;

	barriers[13].barrierModel = barrierMesh->CreateModel(-84, 0, 36);
	barriers[13].skinHatched;

	barriers[14].barrierModel = barrierMesh->CreateModel(-84, 0, 54);
	barriers[14].skinHatched;

	barriers[15].barrierModel = barrierMesh->CreateModel(-84, 0, 72);
	barriers[15].skinHatched = false;

	barriers[16].barrierModel = barrierMesh->CreateModel(-84, 0, 90);
	barriers[16].skinHatched = false;

	barriers[17].barrierModel = barrierMesh->CreateModel(-84, 0, 108);
	barriers[17].skinHatched = false;

	barriers[18].barrierModel = barrierMesh->CreateModel(-84, 0, 126);
	barriers[18].skinHatched = false;

	barriers[19].barrierModel = barrierMesh->CreateModel(-84, 0, 144);
	barriers[19].skinHatched = false;

	barriers[20].barrierModel = barrierMesh->CreateModel(-84, 0, 162);
	barriers[20].skinHatched = false;

	barriers[21].barrierModel = barrierMesh->CreateModel(-84, 0, 180);
	barriers[21].skinHatched = false;
	//Array used to set skins for each barrier.
	//First 4 barriers on each side have a hatched skin.
	for (int i = 0; i < kNumBarriers; i++)
	{
		if (barriers[i].skinHatched)
		{
			barriers[i].barrierModel->SetSkin(barrierSkin1);
		}
		else if (!barriers[i].skinHatched)
		{
			barriers[i].barrierModel->SetSkin(barrierSkin2);
		}
	}

	//Allow game to check if keys are pressed/held.
	bool marbleLeftMoveKeyPressed = false; //Z Key, moves the marble to the left.
	bool marbleRightMoveKeyPressed = false; //X key, moves the marble to the right.
	bool fireKeyPressed = false; //Space key, fires the marble down the playing area.
	bool rotateClockWiseKeyPressed = false; //, key, rotates marble left around the Y axis.
	bool rotateCounterClockWiseKeyPressed = false; //. key, rotates marble right around the Y axis.
	bool moveCameraUpKeyPressed = false;//Up arrow, moves camera up.
	bool moveCameraDownKeyPressed = false;//Down arrow, moves camera down.
	bool cameraResetKeyPressed = false;//C Key, resets the camera to it's starting position.
	bool pauseKeyPressed = false;//P Key, used to pause/unpause the game.
	bool pause = false;//Variable used to check whether game is paused/unpaused.
	// The main game loop, repeat until engine is stopped
	while (myEngine->IsRunning())
	{
		//This will count the time taken for frames to be displayed. By multiplying this value by kGameSpeed this means that every type of movement will occur at the same speed, regardless of a change in the frame rate.
		float frameTime = myEngine->Timer() * kGameSpeed;
		// Draw the scene
		myEngine->DrawScene();

		/**** Update your scene each frame here ****/
		//Every frame the value of the worm counter will decrease by 1, this will be used to time the movement of the worm.
		wormMovementCounter -= frameTime * 2;
		//Allow game to be quit without using alt+f4.
		if (myEngine->KeyHit(Key_Escape))
		{
			myEngine->Stop();

		}
		/*Variables used to monitor when keys are hit.
		Change bool values to enable/disable function of the keys.*/
		marbleLeftMoveKeyPressed = (myEngine->KeyHeld(Key_Z));
		marbleRightMoveKeyPressed = (myEngine->KeyHeld(Key_X));
		fireKeyPressed = (myEngine->KeyHit(Key_Space));
		rotateClockWiseKeyPressed = (myEngine->KeyHeld(Key_Comma));
		rotateCounterClockWiseKeyPressed = (myEngine->KeyHeld(Key_Period));
		moveCameraUpKeyPressed = (myEngine->KeyHeld(Key_Up));
		moveCameraDownKeyPressed = (myEngine->KeyHeld(Key_Down));
		cameraResetKeyPressed = (myEngine->KeyHeld(Key_C));
		pauseKeyPressed = (myEngine->KeyHit(Key_P));
		if (pauseKeyPressed)
		{
			pause = !pause;
		}
		//Variables to store Text and variables that will be used as a HUD to display info to the user. 
		stringstream shotsFired;
		shotsFired << "Shots Fired: " << firedShots;
		stringstream mushroomsDestroyed;
		mushroomsDestroyed << "Mushrooms Destroyed: " << destroyedMushrooms;
		stringstream wormsDestroyed;
		wormsDestroyed << "Worm Segments Destroyed: " << destroyedWorms;
		//This creates the text and draws it on the screen.
		shotsFiredFont->Draw(shotsFired.str(), 800, 50, kWhite, kLeft, kVCentre);
		mushroomsDestroyedFont->Draw(mushroomsDestroyed.str(), 800, 80, kWhite, kLeft, kVCentre);
		wormsDestroyedFont->Draw(wormsDestroyed.str(), 800, 110, kWhite, kLeft, kVCentre);
		//Camera Movement
		if (moveCameraUpKeyPressed)
		{
			//Camera upwards movement only occurs when camera is below max limit.
			if (camera->GetY() <= yMaxCameraLimit)
			{
				camera->MoveY(frameTime / cameraMovement);
				camera->RotateX(frameTime / cameraMovement);
			}
		}
		if (moveCameraDownKeyPressed)
		{
			//Camera downwards movement only occurs when camera is above min limit.
			if (camera->GetY() >= yMinCameraLimit)
			{
				camera->MoveY(-frameTime / cameraMovement);
				camera->RotateX(-frameTime / cameraMovement);
			}
		}
		//Allow user to reset camera to its starting position at any time.
		if (cameraResetKeyPressed)
		{
			CameraStartPosition(camera, cameraPosition[0], cameraPosition[1], cameraPosition[2], cameraPosition[3]);
		}
		//START OF GAME.
		if (!pause)
		{
			//Block of code used to move the worm down the grid towards the starting position of the marble. 
			if (gameState != over)
			{
				//First check if the counter has reached zero. Only then will the worm move. 
				if (wormMovementCounter <= 0)
				{
					//We also have to check that the worm segment hasn't been destroyed. 
					//Worm 0.
					if (wormState[0] != threeHit)
					{
						//Check whether the worm is at the left hand side of the grid = column 0.
						if (worm0Position == lHandSideGrid && worm0Direction == left)
						{
							//We reverse the direction of the worm.
							worm0Direction = right;
							//Then move the worm down one row.
							worm0CurrentRow++;
						}
						//Check whether the worm is at the right hand side of the grid = column 16.
						if (worm0Direction == right && worm0Position == rHandSideGrid)
						{
							//We move the worm down one row.
							worm0CurrentRow++;
							//Then change the direction of the worm.
							worm0Direction = left;
						}
						if (worm0Direction == right && worm0Position < rHandSideGrid)
						{
							//This moves the worm along the grid every time the counter reaches 0.
							worms[0]->SetX(xGridMatrix[worm0CurrentRow][worm0Position]);
							worms[0]->SetZ(zGridMatrix[worm0CurrentRow][worm0Position]);
							//Once the worm has moved we update the worms position so that the worm will continue to move forwards.
							worm0Position++;

						}
						if (worm0Direction == left && worm0Position > lHandSideGrid)
						{
							//Whe moving left we need to decrease the value of the worms position to enable it to move left.
							worm0Position--;
							//This moves the worm along the grid every time the counter reaches 0.
							worms[0]->SetX(xGridMatrix[worm0CurrentRow][worm0Position]);
							worms[0]->SetZ(zGridMatrix[worm0CurrentRow][worm0Position]);
						}
					}
					//Worm 1.
					if (wormState[1] != threeHit)
					{
						if (worm1Position == lHandSideGrid && worm1Direction == left)
						{
							worm1Direction = right;
							worm1CurrentRow++;
						}
						if (worm1Direction == right && worm1Position == rHandSideGrid)
						{
							worm1CurrentRow++;
							worm1Direction = left;
						}
						if (worm1Direction == right && worm1Position < rHandSideGrid)
						{
							worms[1]->SetX(xGridMatrix[worm1CurrentRow][worm1Position]);
							worms[1]->SetZ(zGridMatrix[worm1CurrentRow][worm1Position]);
							worm1Position++;
						}
						if (worm1Direction == left && worm1Position > lHandSideGrid)
						{
							worm1Position--;
							worms[1]->SetX(xGridMatrix[worm1CurrentRow][worm1Position]);
							worms[1]->SetZ(zGridMatrix[worm1CurrentRow][worm1Position]);
						}
					}
					//Worm 2.
					if (wormState[2] != threeHit)
					{
						if (worm2Position == lHandSideGrid && worm2Direction == left)
						{
							worm2Direction = right;
							worm2CurrentRow++;
						}
						if (worm2Direction == right && worm2Position == rHandSideGrid)
						{
							worm2CurrentRow++;
							worm2Direction = left;
						}
						if (worm2Direction == right && worm2Position < rHandSideGrid)
						{
							worms[2]->SetX(xGridMatrix[worm2CurrentRow][worm2Position]);
							worms[2]->SetZ(zGridMatrix[worm2CurrentRow][worm2Position]);
							worm2Position++;
						}
						if (worm2Direction == left && worm2Position > lHandSideGrid)
						{
							worm2Position--;
							worms[2]->SetX(xGridMatrix[worm2CurrentRow][worm2Position]);
							worms[2]->SetZ(zGridMatrix[worm2CurrentRow][worm1Position]);
						}
					}
					//Worm 3.
					if (wormState[3] != threeHit)
					{
						if (worm3Position == lHandSideGrid && worm3Direction == left)
						{
							worm3Direction = right;
							worm3CurrentRow++;
						}
						if (worm3Direction == right && worm3Position == rHandSideGrid)
						{
							worm3CurrentRow++;
							worm3Direction = left;
						}
						if (worm3Direction == right && worm3Position < rHandSideGrid)
						{
							worms[3]->SetX(xGridMatrix[worm3CurrentRow][worm3Position]);
							worms[3]->SetZ(zGridMatrix[worm3CurrentRow][worm3Position]);
							worm3Position++;
						}
						if (worm3Direction == left && worm3Position > lHandSideGrid)
						{
							worm3Position--;
							worms[3]->SetX(xGridMatrix[worm3CurrentRow][worm3Position]);
							worms[3]->SetZ(zGridMatrix[worm3CurrentRow][worm1Position]);
						}
					}
					//Worm 4.
					if (wormState[4] != threeHit)
					{
						if (worm4Position == lHandSideGrid && worm4Direction == left)
						{
							worm4Direction = right;
							worm4CurrentRow++;
						}
						if (worm4Direction == right && worm4Position == rHandSideGrid)
						{
							worm4CurrentRow++;
							worm4Direction = left;
						}
						if (worm4Direction == right && worm4Position < rHandSideGrid)
						{
							worms[4]->SetX(xGridMatrix[worm4CurrentRow][worm4Position]);
							worms[4]->SetZ(zGridMatrix[worm4CurrentRow][worm4Position]);
							worm4Position++;
						}
						if (worm4Direction == left && worm4Position > lHandSideGrid)
						{
							worm4Position--;
							worms[4]->SetX(xGridMatrix[worm4CurrentRow][worm4Position]);
							worms[4]->SetZ(zGridMatrix[worm4CurrentRow][worm1Position]);
						}
					}
					//Worm 5.
					if (wormState[5] != threeHit)
					{
						if (worm5Position == lHandSideGrid && worm5Direction == left)
						{
							worm5Direction = right;
							worm5CurrentRow++;
						}
						if (worm5Direction == right && worm5Position == rHandSideGrid)
						{
							worm5CurrentRow++;
							worm5Direction = left;
						}
						if (worm5Direction == right && worm5Position < rHandSideGrid)
						{
							worms[5]->SetX(xGridMatrix[worm5CurrentRow][worm5Position]);
							worms[5]->SetZ(zGridMatrix[worm5CurrentRow][worm5Position]);
							worm5Position++;
						}
						if (worm5Direction == left && worm5Position > lHandSideGrid)
						{
							worm5Position--;
							worms[5]->SetX(xGridMatrix[worm5CurrentRow][worm5Position]);
							worms[5]->SetZ(zGridMatrix[worm5CurrentRow][worm1Position]);
						}
					}
					//Worm 6.
					if (wormState[6] != threeHit)
					{
						if (worm6Position == lHandSideGrid && worm6Direction == left)
						{
							worm6Direction = right;
							worm6CurrentRow++;
						}
						if (worm6Direction == right && worm6Position == rHandSideGrid)
						{
							worm6CurrentRow++;
							worm6Direction = left;
						}
						if (worm6Direction == right && worm6Position < rHandSideGrid)
						{
							worms[6]->SetX(xGridMatrix[worm6CurrentRow][worm6Position]);
							worms[6]->SetZ(zGridMatrix[worm6CurrentRow][worm6Position]);
							worm6Position++;
						}
						if (worm6Direction == left && worm6Position > lHandSideGrid)
						{
							worm6Position--;
							worms[6]->SetX(xGridMatrix[worm6CurrentRow][worm6Position]);
							worms[6]->SetZ(zGridMatrix[worm6CurrentRow][worm6Position]);
						}
					}
					//Worm 7.
					if (wormState[7] != threeHit)
					{
						if (worm7Position == lHandSideGrid && worm7Direction == left)
						{
							worm7Direction = right;
							worm7CurrentRow++;
						}
						if (worm7Direction == right && worm7Position == rHandSideGrid)
						{
							worm7CurrentRow++;
							worm7Direction = left;
						}
						if (worm7Direction == right && worm7Position < rHandSideGrid)
						{
							worms[7]->SetX(xGridMatrix[worm7CurrentRow][worm7Position]);
							worms[7]->SetZ(zGridMatrix[worm7CurrentRow][worm7Position]);
							worm7Position++;
						}
						if (worm7Direction == left && worm7Position > lHandSideGrid)
						{
							worm7Position--;
							worms[7]->SetX(xGridMatrix[worm7CurrentRow][worm7Position]);
							worms[7]->SetZ(zGridMatrix[worm7CurrentRow][worm7Position]);
						}
					}
					//Worm 8.
					if (wormState[8] != threeHit)
					{
						if (worm8Position == lHandSideGrid && worm8Direction == left)
						{
							worm8Direction = right;
							worm8CurrentRow++;
						}
						if (worm8Direction == right && worm8Position == rHandSideGrid)
						{
							worm8CurrentRow++;
							worm8Direction = left;
						}
						if (worm8Direction == right && worm8Position < rHandSideGrid)
						{
							worms[8]->SetX(xGridMatrix[worm8CurrentRow][worm8Position]);
							worms[8]->SetZ(zGridMatrix[worm8CurrentRow][worm8Position]);
							worm8Position++;
						}
						if (worm8Direction == left && worm8Position > lHandSideGrid)
						{
							worm8Position--;
							worms[8]->SetX(xGridMatrix[worm8CurrentRow][worm8Position]);
							worms[8]->SetZ(zGridMatrix[worm8CurrentRow][worm8Position]);
						}
					}
					//Worm 9.
					if (wormState[9] != threeHit)
					{
						if (worm9Position == lHandSideGrid && worm9Direction == left)
						{
							worm9Direction = right;
							worm9CurrentRow++;
						}
						if (worm9Direction == right && worm9Position == rHandSideGrid)
						{
							worm9CurrentRow++;
							worm9Direction = left;
						}
						if (worm9Direction == right && worm9Position < rHandSideGrid)
						{
							worms[9]->SetX(xGridMatrix[worm9CurrentRow][worm9Position]);
							worms[9]->SetZ(zGridMatrix[worm9CurrentRow][worm9Position]);
							worm9Position++;

						}
						if (worm9Direction == left && worm9Position > lHandSideGrid)
						{
							worm9Position--;
							worms[9]->SetX(xGridMatrix[worm9CurrentRow][worm9Position]);
							worms[9]->SetZ(zGridMatrix[worm9CurrentRow][worm9Position]);
						}
					}
					//This checks if any of the worms segments have reached the bottom left hand corner of the grid.
					//If this is true then the game is over and the player has lost.
					if ((worm0CurrentRow == 19 && worm0Position == 0) || (worm1CurrentRow == 19 && worm1Position == 0) || (worm2CurrentRow == 19 && worm2Position == 0) || (worm3CurrentRow == 19 && worm3Position == 0) || (worm4CurrentRow == 19 && worm4Position == 0) || (worm5CurrentRow == 19 && worm5Position == 0) || (worm6CurrentRow == 19 && worm6Position == 0) || (worm7CurrentRow == 19 && worm7Position == 0) || (worm8CurrentRow == 19 && worm8Position == 0) || (worm9CurrentRow == 19 && worm9Position == 0))
					{
						gameState = over;
					}
					//We have to reset the counter to allow the worm to continue to move.
					wormMovementCounter = 1000;
				}
			}

			//Game state = ready.
			if (gameState == ready)
			{
				if (marbleLeftMoveKeyPressed)
				{
					//Marble movement limited by current position of marble. If above min limit then it moves left.
					//Dummy model also moves so that the arrow moves behind the marble.
					if (marble0->GetX() > xMinLimit)
					{
						dummy->MoveX(-frameTime / marbleStrafeSpeed);
						marble0->MoveX(-frameTime / marbleStrafeSpeed);
					}
				}
				if (marbleRightMoveKeyPressed)
				{
					//Marble movement limited by current position of marble. If below max limit then it moves right.
					//Dummy model also moves so that the arrow moves behind the marble.
					if (marble0->GetX() < xMaxLimit)
					{
						dummy->MoveX(frameTime / marbleStrafeSpeed);
						marble0->MoveX(frameTime / marbleStrafeSpeed);
					}
				}
				if (rotateClockWiseKeyPressed)
				{
					if (rotateValue <= rotateCWLimit)
					{
						//rotateValue stores how far the marble has been rotated. This value will be used to limit how far the marble is allowed to rotate.
						//This value will also be used to bounce the marble off the side barriers.
						rotateValue += (frameTime / marbleRotationSpeed);
						marble0->RotateY(frameTime / marbleRotationSpeed);
						dummy->RotateY(frameTime / marbleRotationSpeed);
					}
				}
				if (rotateCounterClockWiseKeyPressed)
				{
					if (rotateValue >= rotateCCWLimit)
					{
						//rotateValue stores how far the marble has been rotated. This value will be used to limit how far the marble is allowed to rotate.
						//This value will also be used to bounce the marble off the side barriers.
						rotateValue -= frameTime / marbleRotationSpeed;
						marble0->RotateY(-frameTime / marbleRotationSpeed);
						dummy->RotateY(-frameTime / marbleRotationSpeed);
					}
				}
				//Change game state to firing when space bar hit.
				if (fireKeyPressed)
				{
					//We increment the value of firedShots so that the game tracks how many times the user has fired the marble and this is displayed back to the user via the HUD.
					firedShots++;
					gameState = firing;
				}
			}

			//Game state = firing.
			if (gameState == firing)
			{
				//This launches the marble down the playing area.
				marble0->MoveLocalZ(frameTime);

				//The dummy can still be moved whilst the marble is travelling. Full rotation and +/- X axis movement.
				if (marbleLeftMoveKeyPressed)
				{
					dummy->MoveX(-frameTime / marbleStrafeSpeed);
				}
				if (marbleRightMoveKeyPressed)
				{
					dummy->MoveX(frameTime / marbleStrafeSpeed);
				}

				if (rotateClockWiseKeyPressed)
				{
					if (rotateValue <= rotateCWLimit)
					{
						rotateValue += (frameTime / marbleRotationSpeed);
						dummy->RotateY(frameTime / marbleRotationSpeed);
					}
				}
				if (rotateCounterClockWiseKeyPressed)
				{
					if (rotateValue >= rotateCCWLimit)
					{
						rotateValue -= frameTime / marbleRotationSpeed;
						dummy->RotateY(-frameTime / marbleRotationSpeed);
					}
				}
				//MARBLE/BARRIER COLLISION.
				//Since all the barriers are aligned we only have to check the marble X position to determine whether a collision has occured.
				//The collision detection used here is line detection. This is much simpler than sphere-box collision and also less intensive, meaning higher FPS.
				if (marble0->GetX() >= rightBarrierLimit || marble0->GetX() <= leftBarrierLimit)
				{
					//If a collision does occur then the marble is rotated by the negative of its current rotation value and this causes the marble to bounce off the barrier.
					marble0->RotateY(-rotateValue);
				}
				//For loop used to control mushroom marble collision detection. 
				for (int i = 0; i < kNumMushrooms; i++)
				{
					//Need to seperate collision detection between the first 10 starting mushrooms. Different conditions are needed to determine whether the mushrooms have been created or not.
					//If we check for a collision before we check if a mushroom has been created this will result in a crash.
					if (i <= kNumInitialMushrooms)
					{
						//We must first check that the mushroom hasn't been destroyed.
						if (mushroomState[i] != threeHit)
						{
							//Calculate the collision distance between the marble and each mushroom as the loop iterates.
							collisionDist = CollisionDetection(mushrooms[i], marble0);
							if (collisionDist < mushroomRadiusAddMarbleRadius)
							{
								if (mushroomState[i] == notHit)
								{
									//Mushroom skins change as the mushrooms get hit by the marble. 
									mushrooms[i]->SetSkin(mushroomSkinHot);
									//The state of the mushrooms change to indicate what will happen if the mushroom gets hit again.
									mushroomState[i] = oneHit;
									//Game state also changes to determine if the user has destroyed 5 mushrooms or not.
									gameState = contact;
								}
								if (mushroomState[i] == oneHit && gameState == firing)
								{
									mushrooms[i]->SetSkin(mushroomSkinVeryHot);
									mushroomState[i] = twoHit;
									gameState = contact;
								}
								if (mushroomState[i] == twoHit && gameState == firing)
								{
									//We increment this variable so that the game tracks how many mushrooms have been destroyed so that it can be displayed back via the HUD.
									destroyedMushrooms++;
									//When the mushroom state is change to three hits then the game will stop checking for collision between the marble and the corresponding mushroom because the mushroom will be deleted.
									mushroomState[i] = threeHit;
									gameState = contact;
								}
							}
						}
					}
					//This section focuses on collision detection between the marble and the mushrooms that will be created when the worm segments are destroyed.
					//Therefore we need to check if the mushrooms have first been created before we check for a collision between the marble and the mushroom.
					//Must specifiy the different conditions, mushrooms created from the worm don't have a state at the start of the game.
					if (i > kNumInitialMushrooms)
					{
						if (mushroomState[i] == notHit || mushroomState[i] == oneHit || mushroomState[i] == twoHit)
						{
							//Calculate the collision distance between the marble and each mushroom as the loop iterates.
							collisionDist = CollisionDetection(mushrooms[i], marble0);
							if (collisionDist < mushroomRadiusAddMarbleRadius)
							{
								if (mushroomState[i] == notHit)
								{
									mushrooms[i]->SetSkin(mushroomSkinHot);
									mushroomState[i] = oneHit;
									gameState = contact;
								}
								if (mushroomState[i] == oneHit && gameState == firing)
								{
									mushrooms[i]->SetSkin(mushroomSkinVeryHot);
									mushroomState[i] = twoHit;
									gameState = contact;
								}
								if (mushroomState[i] == twoHit && gameState == firing)
								{
									destroyedMushrooms++;
									mushroomState[i] = threeHit;
									gameState = contact;
								}
							}
						}
					}
				}
				//Worm/Marble collision detection.
				//There are a total of 10 worm segments in the game, this loop will iterate through each worm segment and check for a collision detection.
				for (int i = 0; i < kNumWormParts; i++)
				{
					//We must first check that the worm part hasn't been destroyed. 
					if (wormState[i] != threeHit)
					{
						//Calculate the collision distance between the marble and each worm segment as the loop iterates.
						collisionDist = CollisionDetection(worms[i], marble0);
						if (collisionDist < wormSegmentAddMarbleRadius)
						{
							if (wormState[i] == notHit)
							{
								//Worm skins also change as the worm gets hit.
								worms[i]->SetSkin(wormSkinHot);
								wormState[i] = oneHit;
								gameState = contact;
							}
							if (wormState[i] == oneHit && gameState == firing)
							{
								worms[i]->SetSkin(wormSkinVeryHot);
								wormState[i] = twoHit;
								gameState = contact;
							}
							if (wormState[i] == twoHit && gameState == firing)
							{
								//After a worm segment has been hit three times the worm segment will be deleted and a mushroom will be created at the worm segment's current position. 
								mushroomCreation(worms[i], mushrooms[i + mushroomWormOffSet], mushroomMesh, mushroomDeletionPoint);
								//Each new mushroom state is also set to notHit to indicate that the mushroom is now created and can be hit with the marlbe.
								mushroomState[i + mushroomWormOffSet] = notHit;
								//Mushrooms rise up from the ground when they are created.
								mushroomPosition[i + mushroomWormOffSet] = rising;
								//Worm segment is destroyed and removed.
								wormMesh->RemoveModel(worms[i]);
								//The value of destroyedWorms is increased by one every time a worm segment is destroyed. 
								//This will be used to display information to the user via the HUD and to determine whether the whole worm has been destroyed and the user has won the game. 
								destroyedWorms++;
								wormState[i] = threeHit;
								gameState = contact;
							}
						}
					}
				}
				//Checks if the marble goes beyond the back or end limit of the playing area.
				if (marble0->GetZ() >= zMaxLimit || marble0->GetZ() <= zMinLimit)
				{
					gameState = contact;
				}
			}
			//Game state = contact.
			if (gameState == contact)
			{
				//Checks how many mushrooms/worms have been destroyed and determines whether the game is over.
				if (destroyedWorms == wormGameOverCounter)
				{
					gameState = over;
				}
				else
				{
					//If the game isn't over then the marble and dummy's positions are reste to allow the user to fire again.
					BulletStartPosition(marble0, bulletPosition[0], bulletPosition[1], bulletPosition[2]);
					BulletStartPosition(dummy, bulletPosition[0], bulletPosition[1], bulletPosition[2]);
					//Rotate value is set to 0 so that the user can fully rotate the marble. If it weren't then the amount that the user could rotate the marble would be reduced.
					rotateValue = 0;
					gameState = ready;
				}

			}


			if (gameState == over)
			{
				//This determines if the user has destroyed all of the worm segments and they have win.
				if (destroyedWorms == wormGameOverCounter)
				{
					//Display a message to the user indicating they have won.
					gameOverFont->Draw("You won!", 650, 50, kCyan, kCentre);
					//Change colour of marble to indicate victory. 
					marble0->SetSkin(marbleGreenSkin);
				}
				//This will only occur when the worm has reached the bottom of the playing grid and the user hasn't destroyed every part of the worm.
				if (destroyedWorms < wormGameOverCounter)
				{
					//Display a message to the user indicating they have lost.
					gameOverFont->Draw("You lost!", 650, 50, kRed, kCentre);
					//Change colour of marble to indicate defeat.. 
					marble0->SetSkin(marbleRedSkin);
					//Every mushroom is then checked to see whether it has been hit or not. 
					//The msuhrooms will also change colour when the user loses.
					//They will change colour depending on how many times they have been hit.
					for (int i = 0; i < kNumMushrooms; i++)
					{
						if (mushroomState[i] == notHit)
						{
							mushrooms[i]->SetSkin(mushroomSkinPurple);
						}
						if (mushroomState[i] == oneHit || mushroomState[i] == twoHit)
						{
							mushrooms[i]->SetSkin(mushroomSkinPurpleHot);
						}
					}
				}
			}
			//This section is used to allow the mushrooms to rise and fall.
			for (int i = 0; i < kNumMushrooms; i++)
			{
				//Need to check for the mushroom state first, because this will determine whether the mushroom has been created or not.
				//The mushrooms will be created when their state value is notHit.
				//If we were to try and check for the Y co-ordinate then if the mushroom hasnt been created this will lead to an error.
				if (mushroomPosition[i] == rising && mushrooms[i]->GetY() <= mushroomStationaryPoint)
				{
					mushrooms[i]->MoveY(frameTime / mushroomAscendSpeed);
				}
				//Once the mushroom is in the correct starting position it stops moving.
				if (mushroomState[i] == notHit && mushrooms[i]->GetY() >= mushroomStationaryPoint)
				{
					mushroomPosition[i] = stationary;
				}
				//Once the mushroom has been hit three times it begins to sink down into the ground.
				if (mushroomPosition[i] != deleted && mushroomState[i] == threeHit && mushrooms[i]->GetY() >= mushroomDeletionPoint)
				{
					mushroomPosition[i] = falling;
					mushrooms[i]->MoveY(frameTime / mushroomDescendSpeed);
				}
				//Once the mushroom has sunk under the ground it will be deleted and removed from the game. 
				if (mushroomPosition[i] == falling && mushroomState[i] == threeHit && mushrooms[i]->GetY() <= mushroomDeletionPoint)
				{
					mushroomPosition[i] = deleted;
					mushroomMesh->RemoveModel(mushrooms[i]);
				}
			}
		}
	}
	// Delete the 3D engine now we are finished with it
	myEngine->Delete();
}
float CollisionDetection(IModel* mushrooms, IModel* bullet)
{
	float x;
	float z;
	//Calculate the vector between the bullet and the mushrooms.
	x = bullet->GetX() - mushrooms->GetX();
	z = bullet->GetZ() - mushrooms->GetZ();
	//Then calculate the length of the vector/distance between the two objects.
	float collisionDist = sqrt(x*x + z*z);
	return collisionDist;
}
void BulletStartPosition(IModel* bullet, float xPosition, float yPosition, float zPosition)
{
	bullet->SetPosition(xPosition, yPosition, zPosition);
	bullet->ResetOrientation();

}
void CameraStartPosition(ICamera* camera, float xPosition, float yPosition, float zPosition, float rotation)
{
	camera->SetPosition(xPosition, yPosition, zPosition);
	camera->ResetOrientation();
	camera->RotateX(rotation);
}
void mushroomCreation(IModel* worm, IModel*& mushroom, IMesh* mushroomMesh, float mushroomYPosition)
{
	float xPosition = worm->GetX();
	float zPosition = worm->GetZ();
	mushroom = mushroomMesh->CreateModel(xPosition, mushroomYPosition, zPosition);
}