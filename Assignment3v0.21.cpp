// Jonathan Walsh
#include <TL-Engine.h>	// TL-Engine include file and namespace
#include <sstream> //Allows strings to be displayed on screen
#include <iomanip> //Allows floats with decimal places to be converted to whole numbers
using namespace tle;

struct vector2D
{
	//X and Z coordinates for momentumm, thrust and drag.
	float x;
	float z;
};
enum boxSide { LeftSide, RightSide, FrontSide, BackSide, NoSide }; //Shows side that box is collided with during collision.
enum gameStates { Start, Check1, Check2, Check3, Finish }; //Checkpoints.  Each time you pass a checkpoint the game state changes.
gameStates currentState = Start; //Initial state
enum WayPoints { WP1, WP2, WP3, WP4, WP5, WP6, WP7 }; //Waypoints the AI travels to.
WayPoints currentWP = WP1;//The initial waypoint the AI car is heading to.

vector2D Scalar(float s, vector2D v); //Scalar to created when a 2D vector is multiplied by a multiplier.
vector2D Sum3(vector2D v1, vector2D v2, vector2D v3); //Adds the momentum, thrust and drag together.
void CountDown(string &gettingReady, float &frameTime, float &countDown, bool &gameStarted); //Game starts after a 3 second countdown.
bool CheckpointPassed(float carPointX, float carPointZ, float checkpointX, float checkpointZ, float checkpointW, float checkpointD); //Point-sphere collision detection with checkpoints.
boxSide car2Box(float carXPos, float carZPos, float oldCarXPos, float oldCarZPos, float carRad,
	float boxXPos, float boxZPos, float boxWidth, float boxDepth); //Sphere-box collision detection.
bool car2Sphere(float carXPos, float carZPos, float carRad, float strutXPos, float strutZPos, float strutRad); //Sphre-sphere collision detection.
int CarDamage(float speed, int health); //Damage model for when car collides with objects.

										//Constants that store the amount of each model.
const int checkpointQuantity = 4;
const int waypointQuantity = 7;
const int isleQuantity = 14;
const int wallQuantity = 7;
const int tankQuantity = 6;
const int strutQuantity = 8;

//Holds each checkpoint number.
const int firstCheckpoint = 0;
const int secondCheckpoint = 1;
const int thirdCheckpoint = 2;
const int fourthCheckpoint = 3;

//Checkpoint dimensions.
const float checkpointWidth = 20.0f;
const float checkpointDepth = 3.0f;

//Width and depth of each wall.
float wallWidth = 2.0f;
float wallDepth = 10.0f;

//Y coordinate of skybox.
const float skyYPos = -960.0f;

//Radii for each model.
const float carRad = 4.0f;
const float strutRad = 0.1f;
const float tankRad = 0.5f;

const float maxDistance = 1000.0f;


void main()
{
	// Create a 3D engine (using TLX engine here) and open a window for it
	I3DEngine* myEngine = New3DEngine(kTLX);
	myEngine->StartWindowed();

	// Add default folder for meshes and other media
	myEngine->AddMediaFolder(".\\media");

	//Checkpoint X and Y coordinates.
	float checkpointXCoordinates[checkpointQuantity] = { 0.0f, 0.0f, 30.0f, 60.0f };
	float checkpointZCoordinates[checkpointQuantity] = { 0.0f, 100.0f, 155.0f, 100.0f };
	float rightAngle = 90.0f;

	//The little stumps on each side of the checkpoints.
	float checkPointStrutsX[strutQuantity] = { -8.0f, 9.0f, -8.0f, 9.0f, 30.0f, 30.0f, 52.0f, 69.0f };
	float checkPointStrutsZ[strutQuantity] = { 0.0f, 0.0f, 100.0f, 100.0f, 146.0f, 164.0f, 100.0f, 100.0f };

	//The X and Z coordinates for each of the isle models.
	float isleXCoordinate[isleQuantity] = { -10.0f, -10.0f, 10.0f, 10.0f, -10.0f, -10.0f, 50.0f, 50.0f, 65.0f, 65.0f, 50.0f, 50.0f, 65.0f, 65.0f };
	float isleZCoordinates[isleQuantity] = { 40.0f, 53.0f, 40.0f, 53.0f, 130.0f, 143.0f, 114.0f, 127.0f, 114.0f, 127.0f, 74.0f, 87.0f, 74.0f, 87.0f };

	//The X and Z coordinates of each of the wall models.

	float wallXCoordinates[wallQuantity] = { -10.5, 9.5f, -10.5f, 50.0f, 65.0f, 50.0f, 65.0f };
	float wallZCoordinates[wallQuantity] = { 46.0f, 46.0f, 136.0f, 120.0f, 120.0f, 80.0f, 80.0f };

	//The X, Y and Z coordinates of each of the tank models.
	float tankXCoordinates[tankQuantity] = { -5.0f, 10.0f, 9.5f, 25.0f, 0.0f, 45.0f };
	float tankZCoordinates[tankQuantity] = { 175.0f, 175.0f, 136.0f, 175.0f, 70.0f, 145.0f };
	float tankYCoordinates[tankQuantity] = { 0.0f, 0.0f, 0.0f, 0.0f, -5.0f, 0.0f };
	float Degrees25 = 25.0f;

	//The X and Z coordinates of each of the waypoints(dummies)
	float waypointXCoordinates[waypointQuantity] = { 0.0f, -5.0f, 0.0f, 0.0f, 60.0f, 70.0f, 57.0f };
	float waypointZCoordinates[waypointQuantity] = { 30.0f, 70.0f, 100.0f, 145.0f, 155.0f, 110.0f, 10.0f };

	float defaultBoost = 5.0f;//Initial boost duration.
	float defaultThrust = 0.1f;//Initial thrust.
	float defaultDrag = -0.0005f; //Initial drag.

								  //The thrust and drag initial multipliers.
	float thrustMultiplier = defaultThrust;
	float dragCoeff = defaultDrag;

	int health = 100; //The initial amount of health the car has before it takes any damage.
	int limitX = 0; //The initial limit before mouse speed has been added on.  Same for below.
	int limitY = 0;

	//The initial X positions of both cars.
	float initialCarZPos = -30.0f;
	float initialAiXPos = 10.0f;
	float resetYPos = 10.0f;
	float fPYPos = 5.0f;
	float fPZPos = 5.0f;

	//string variables
	string gettingReady = "Hit Space to Start. . ."; //User prompt
	string speedText = "Speed: "; //Speed text
	string backDropImage = "ui_backdrop.jpg";
	string aISkin = "sp01.jpg";

	float countDown = 3.0f; //3 second countdown until the game starts when you press spacebar.
	float boostDuration = 5.0f; //The amount of time you can use the boost for.
	float overheatDuration = 5.0f; //The amount of time it takes for the car to recover when it overheats.
	bool countingDown = false; //Counting down becomes true when spacebar has been pressed in order to start the countdown.

	float steeringFactor = 100.0f; //Multiplied by frametime to get the steering speed.

	float realisticSpeed = 1000.0f; //Gives a more realistic, but still proportional on screen value for speed.


									//Readout positions
	int speedReadOutYPos = 20;
	int xBoostDisplayPos = 500;

	float changeMomentumDirection = -1.5f;//Chnage the direction the momentum is going.

	bool gameStarted = false; //Becomes true when the count down has finished.
	bool checkPointPassed[checkpointQuantity];

	for (int i = 0; i < checkpointQuantity; i++)
	{
		checkPointPassed[i] = false; //Becomes true when you pass a checkpoint.
	}

	//Meshes
	IMesh*checkPointMesh = myEngine->LoadMesh("Checkpoint.x");
	IMesh*isleMesh = myEngine->LoadMesh("IsleStraight.x");
	IMesh*wallMesh = myEngine->LoadMesh("Wall.x");
	IMesh*carMesh = myEngine->LoadMesh("race2.x");
	IMesh*floorMesh = myEngine->LoadMesh("ground.x");
	IMesh*skyMesh = myEngine->LoadMesh("Skybox 07.x");
	IMesh*dummyMesh = myEngine->LoadMesh("dummy.x");
	IMesh*tankMesh = myEngine->LoadMesh("TankSmall1.x");

	//Models
	IModel*checkpoint[checkpointQuantity];
	IModel*isle[isleQuantity];
	IModel*wall[wallQuantity];
	IModel*tank[tankQuantity];
	IModel*hoverCar = carMesh->CreateModel(0.0f, 0.0f, initialCarZPos);
	IModel*aICar = carMesh->CreateModel(initialAiXPos, 0.0f, initialCarZPos);
	IModel*floor = floorMesh->CreateModel();
	IModel*sky = skyMesh->CreateModel(0.0f, skyYPos, 0.0f);
	IModel*dummyCar = dummyMesh->CreateModel();
	IModel*resetCam = dummyMesh->CreateModel();
	IModel*fPCam = dummyMesh->CreateModel();
	IModel*waypoint[wallQuantity];
	ISprite*backdrop = myEngine->CreateSprite(backDropImage);
	IFont*myFont = myEngine->LoadFont("Verdana", 24);

	//Camera Setup
	ICamera*myCamera = myEngine->CreateCamera(kManual);
	myCamera->AttachToParent(dummyCar); //Camera attached to dummy car so then it follows the car and can rotate around the car.

	dummyCar->AttachToParent(hoverCar);//Dummy is attached the the hover car.
	resetCam->AttachToParent(dummyCar);//""
	fPCam->AttachToParent(dummyCar);

	resetCam->SetLocalZ(initialCarZPos);//Camera is set to original position.
	resetCam->SetLocalY(resetYPos); //Reset camera is elevated slightly off the ground.
	fPCam->SetLocalY(fPYPos); //First person position relative to hover car.
	fPCam->SetLocalZ(fPZPos);

	aICar->SetSkin(aISkin); //Image being used on AI car,

							//Initial momentum, thrust and drag positions in the X and Z.
	vector2D momentum{ 0.0f, 0.0f };
	vector2D thrust{ 0.0f, 0.0f };
	vector2D drag{ 0.0f, 0.0f };
	float matrix[4][4]; //Matrix to get the facing vector/local Z

	for (int i = 0; i < checkpointQuantity; i++)
	{
		//Checkpoint models created and positioned.  Same with the next 4 for loops.
		checkpoint[i] = checkPointMesh->CreateModel(checkpointXCoordinates[i], 0.0f, checkpointZCoordinates[i]);
	}
	for (int i = 0; i < isleQuantity; i++)
	{
		isle[i] = isleMesh->CreateModel(isleXCoordinate[i], 0.0f, isleZCoordinates[i]);
	}
	for (int i = 0; i < wallQuantity; i++)
	{
		wall[i] = wallMesh->CreateModel(wallXCoordinates[i], 0.0f, wallZCoordinates[i]);
	}
	for (int i = 0; i < tankQuantity; i++)
	{
		tank[i] = tankMesh->CreateModel(tankXCoordinates[i], tankYCoordinates[i], tankZCoordinates[i]);
	}
	for (int i = 0; i < waypointQuantity; i++)
	{
		waypoint[i] = dummyMesh->CreateModel(waypointXCoordinates[i], 0.0f, waypointZCoordinates[i]);
	}

	//Rotating models
	checkpoint[2]->RotateY(rightAngle); //Checkpoint is rotated 90 degrees, so then it is facing to the right rather than forwards.

	tank[4]->RotateX(Degrees25); //Tank rotated 25 degrees in X.

								 //Keyboard key mappings.
	EKeyCode quit = Key_Escape;
	EKeyCode accelForward = Key_W;
	EKeyCode decelBackward = Key_S;
	EKeyCode steerRight = Key_D;
	EKeyCode steerLeft = Key_A;
	EKeyCode cameraForward = Key_Up;
	EKeyCode cameraBackward = Key_Down;
	EKeyCode cameraRight = Key_Right;
	EKeyCode cameraLeft = Key_Left;
	EKeyCode chaseCamera = Key_1;
	EKeyCode fPCamera = Key_2;
	EKeyCode startOrBoost = Key_Space;


	float frameTime = myEngine->Timer(); // Timer initialised.
										 // The main game loop, repeat until engine is stopped
	while (myEngine->IsRunning())
	{
		frameTime = myEngine->Timer(); //Timer started in loop.

									   // Draw the scene
		myEngine->DrawScene();

		float oldX = hoverCar->GetX(); //Reset position for hover car for when collides with objects.
		float oldZ = hoverCar->GetZ();

		if ((sqrt(oldX*oldX + oldZ * oldZ) > maxDistance))
		{
			myEngine->Stop();//Game closes if you leave the course.
		}

		//get the facing vector - local z of car
		hoverCar->GetMatrix(&matrix[0][0]);
		vector2D facingVector = { matrix[2][0]/*[Global Z][Local X]*/, matrix[2][2] /*[GlobalZ][LocalZ]*/ };

		//THRUST AND STEERING ONLY WORK WHEN GAMESTARTED IS TRUE.
		//calculate thrust(based on keyboard input)  
		if (myEngine->KeyHeld(accelForward) && gameStarted)
		{
			thrust = Scalar(thrustMultiplier, facingVector);
		}
		else if (myEngine->KeyHeld(decelBackward) && gameStarted)
		{
			thrust = Scalar(-thrustMultiplier, facingVector);
		}
		else
		{
			thrust = { 0.0f, 0.0f }; //No thrust when no keys are pressed, but still momentum.
		}

		//Steering
		if (myEngine->KeyHeld(steerRight) && gameStarted)
		{
			hoverCar->RotateY(steeringFactor * frameTime);
		}
		if (myEngine->KeyHeld(steerLeft) && gameStarted)
		{
			hoverCar->RotateY(-steeringFactor * frameTime);
		}


		//calculate drag(based on previous momentum)
		drag = Scalar(dragCoeff, momentum);

		//caluclate momentum(based on thrust, drag and previous momentum)
		momentum = Sum3(momentum, thrust, drag);

		//move the hover car (according to new momentum)
		hoverCar->Move(momentum.x * frameTime, 0.0f, momentum.z * frameTime);

		//Converts vector to scalar.
		float speed = sqrt((momentum.x*frameTime)*(momentum.x*frameTime) + 0.0f*0.0f + (momentum.z*frameTime)*(momentum.z*frameTime));
		speed *= realisticSpeed; //Gives a more realistic value for the speed.

		stringstream speedReadOut;
		if (speed < 0.0f)
		{
			//Speed is a scalar.  Velocity shows direction as well as speed.  
			//We just need speed, so negative speed isn't necessary.
			float makePositve = -1.0f;
			speed *= makePositve; //Make speed positive if speed is less to 0.
		}
		speedReadOut << speedText << fixed; /*Forces the program to read as a whole number*/
		speedReadOut << setprecision(0) /*Just whole number(0 decimal places)*/ << speed;
		myFont->Draw(speedReadOut.str(), 0, speedReadOutYPos);
		//Convert momentum into scalar.
		float scalarMomentum = sqrt(momentum.x*momentum.x + momentum.z*momentum.z);

		//Count Down text
		stringstream directionsText;
		directionsText << gettingReady;
		myFont->Draw(directionsText.str(), 0, 0);
		if (!countingDown)
		{
			if (myEngine->KeyHit(startOrBoost))
			{
				countingDown = true; //When space is pressed, count down starts.
			}

		}
		else if (countingDown)
		{
			countDown -= frameTime; //The frametime is taken away from the countdown.
			CountDown(gettingReady, frameTime, countDown, gameStarted); //Display is changed based on the value of countdown.
		}

		//Check for collision for checkpoints.  Replaces countdown text with state changes.
		if (CheckpointPassed(hoverCar->GetX(), hoverCar->GetZ(), checkpoint[firstCheckpoint]->GetX(), checkpoint[firstCheckpoint]->GetZ(), checkpointWidth, checkpointDepth)
			&& currentState == Start)
		{
			gettingReady = "Stage 1 Complete"; //First checkpoint passed.
			currentState = Check1;
		}

		if (CheckpointPassed(hoverCar->GetX(), hoverCar->GetZ(), checkpoint[secondCheckpoint]->GetX(), checkpoint[secondCheckpoint]->GetZ(), checkpointWidth, checkpointDepth)
			&& currentState == Check1)
		{
			gettingReady = "Stage 2 Complete"; //Second checkpoint passed.
			currentState = Check2;
		}

		if (CheckpointPassed(hoverCar->GetX(), hoverCar->GetZ(), checkpoint[thirdCheckpoint]->GetX(), checkpoint[thirdCheckpoint]->GetZ(), checkpointWidth, checkpointDepth)
			&& currentState == Check2)
		{
			gettingReady = "Stage 3 Complete"; //Third
			currentState = Check3;
		}

		if (CheckpointPassed(hoverCar->GetX(), hoverCar->GetZ(), checkpoint[fourthCheckpoint]->GetX(), checkpoint[fourthCheckpoint]->GetZ(), checkpointWidth, checkpointDepth)
			&& currentState == Check3)
		{
			gettingReady = "Race Finished!"; //Fourth/finish line passed.
			currentState = Finish;
		}

		//Check for collisions with walls/isles		
		for (int i = 0; i < wallQuantity; i++)
		{
			boxSide collision = car2Box(hoverCar->GetX(), hoverCar->GetZ(), oldX, oldZ, carRad,
				wallXCoordinates[i], wallZCoordinates[i], wallWidth, wallDepth);

			//Work out collisions
			if (collision == FrontSide || collision == BackSide)
			{
				hoverCar->SetZ(oldZ);
				momentum.x /= changeMomentumDirection; //Car bounces back when hits the object
				momentum.z /= changeMomentumDirection; //Which changes direction of momentum.  Same for future lines.
			}
			else if (collision == LeftSide || collision == RightSide)
			{
				hoverCar->SetX(oldX);
				momentum.x /= changeMomentumDirection;
				momentum.z /= changeMomentumDirection;

			}
			if (collision != NoSide)
			{
				health = CarDamage(scalarMomentum, health);//Car gets damage when it hits the object.

			}



		}


		//Check for collision with checkpoint struts
		//More brief comments below.  Read above comments if you require more detail.
		for (int i = 0; i < strutQuantity; i++)
		{
			bool collision = car2Sphere(hoverCar->GetX(), hoverCar->GetZ(), carRad, checkPointStrutsX[i], checkPointStrutsZ[i], strutRad);

			if (collision)
			{
				hoverCar->SetPosition(oldX, 0.0f, oldZ);
				momentum.x /= changeMomentumDirection;
				momentum.z /= changeMomentumDirection;
				health = CarDamage(scalarMomentum, health);
			}
		}

		//Check for collision with water tanks
		for (int i = 0; i < tankQuantity; i++)
		{
			bool collision = car2Sphere(hoverCar->GetX(), hoverCar->GetZ(), carRad, tankXCoordinates[i], tankZCoordinates[i], tankRad);

			if (collision)
			{
				hoverCar->SetPosition(oldX, 0.0f, oldZ);
				momentum.x /= changeMomentumDirection;
				momentum.z /= changeMomentumDirection;
				health = CarDamage(scalarMomentum, health);
			}
		}

		//Check for collision with AI car
		bool collision = car2Sphere(hoverCar->GetX(), hoverCar->GetZ(), carRad, aICar->GetX(), aICar->GetZ(), carRad);

		if (collision)
		{
			hoverCar->SetPosition(oldX, 0.0f, oldZ);
			momentum.x /= changeMomentumDirection;
			momentum.z /= changeMomentumDirection;
			health = CarDamage(scalarMomentum, health);
		}



		//Chase cam
		//Keyboard input (cam move forward, backward, left, right)
		float cameraSpeed = 10.0f;

		if (myEngine->KeyHeld(cameraForward))
		{
			//Arrow up = move forward
			myCamera->MoveZ(cameraSpeed*frameTime);
		}
		if (myEngine->KeyHeld(cameraBackward))
		{
			//Arrow down = move backward
			myCamera->MoveZ(-cameraSpeed * frameTime);
		}
		if (myEngine->KeyHeld(cameraLeft))
		{
			//Arrow left = left movement
			myCamera->MoveX(-cameraSpeed * frameTime);
		}
		if (myEngine->KeyHeld(cameraRight))
		{
			//Arrow right = right movement
			myCamera->MoveX(cameraSpeed*frameTime);
		}

		//Mouse Input (cam rotate upwards, downwards, left and right relative to car)

		myEngine->StartMouseCapture();
		int mouseMoveX = myEngine->GetMouseMovementX();
		int mouseMoveY = myEngine->GetMouseMovementY();
		int yCamLimits = 250;
		int xCamLimits = 2000;

		if (mouseMoveX > myEngine->GetMouseMovementX() && limitX > -xCamLimits)
		{
			//Move mouse to the right. Camera rotates around hover car to the right.
			dummyCar->RotateY(-cameraSpeed * frameTime*mouseMoveX);
			limitX -= mouseMoveX;
		}

		if (mouseMoveX < myEngine->GetMouseMovementX() && limitX < xCamLimits)
		{
			//Move mouse to the left.  Camera rotates around hover car to the left.
			if (mouseMoveX < 0)
			{
				mouseMoveX *= -1;
			}
			dummyCar->RotateY(cameraSpeed*frameTime *mouseMoveX);
			limitX += mouseMoveX;
		}

		if (mouseMoveY > myEngine->GetMouseMovementY() && limitY < yCamLimits)
		{
			//Mouse moves downwards.
			dummyCar->RotateLocalX(-cameraSpeed * frameTime*mouseMoveY);
			limitY += mouseMoveY;
		}
		if (mouseMoveY < myEngine->GetMouseMovementY() && limitY > -yCamLimits)
		{
			//Mouse moves upwards.
			if (mouseMoveY < 0)
			{
				int changeSign = -1;
				mouseMoveY *= changeSign;
			}
			dummyCar->RotateLocalX(cameraSpeed*frameTime*mouseMoveY);
			limitY -= mouseMoveY;
		}
		//Camera reset to third person view.
		if (myEngine->KeyHit(chaseCamera))
		{
			myCamera->SetZ(resetCam->GetZ());
			myCamera->SetX(resetCam->GetX());
			myCamera->SetY(resetCam->GetY());
			dummyCar->ResetOrientation();

		}
		//Camera reset to first person view.
		if (myEngine->KeyHit(fPCamera))
		{
			myCamera->SetZ(fPCam->GetZ());
			myCamera->SetX(fPCam->GetX());
			myCamera->SetY(fPCam->GetY());
			dummyCar->ResetOrientation();
		}

		//Boost Mode
		if (myEngine->KeyHeld(startOrBoost) && countDown <= 0.0f) //Only executes when count down has ended.  Spacebar held
		{
			float thrustChange = 1.0001f;
			boostDuration -= frameTime; // Amount of time for boost
			if (boostDuration > 0.0f)
			{
				thrustMultiplier *= thrustChange; //The amount the thrust is multiplied by.
			}
		}
		else
		{
			if (boostDuration > 0.0f)
			{
				boostDuration = defaultBoost; //Boost set back to default when enter key is no longer held.
				thrustMultiplier = defaultThrust;//Boost no longer takes place.
			}
		}
		if (boostDuration <= 0.0f)
		{
			overheatDuration -= frameTime; //Amount of time to recover from overheating.
			if (overheatDuration <= 0.0f)
			{

				//When engine has recovered, all values get set back to defaults
				overheatDuration = defaultBoost; //Default duration for overheating
				boostDuration = defaultBoost; //Default duration for boost. 
				dragCoeff = defaultDrag;
				thrustMultiplier = defaultThrust;
			}
			else
			{
				float changeDrag = 1.001;
				dragCoeff *= changeDrag; //The amount drag is multiplied by.
			}
		}

		stringstream boostDetails;
		if (boostDuration > 0.0f)
		{
			//Shows boost duration on ui.
			boostDetails << fixed << setprecision(0) /*Just whole number(0 decimal places)*/ << "Boost time: " << boostDuration;
		}
		else
		{
			//Shows revover time on ui.
			boostDetails << fixed << setprecision(0) /*Just whole number(0 decimal places)*/ << "Recovery time: " << overheatDuration;
		}
		myFont->Draw(boostDetails.str(), xBoostDisplayPos, 0);

		int alertXPos = 250;//Position of text
		int alertYPos = 20;
		if (boostDuration < 1.0f && boostDuration > 0.0f)
		{
			//Shows alert message when there is risk of overheating.
			stringstream alert;
			alert << "***ALERT!  OVERHEAT IMMINENT!***";
			myFont->Draw(alert.str(), alertXPos, alertYPos);
		}
		if (overheatDuration < 5.0f && overheatDuration > 0.0f)
		{
			//Engine overheated alert.
			stringstream alert;
			alert << "***ENGINE OVERHEATED!!***";
			myFont->Draw(alert.str(), alertXPos, alertYPos);
		}

		//health display
		stringstream healthDisplay;
		healthDisplay << "Car Health: " << health;
		int healthXPos = 250;//Position of text
		myFont->Draw(healthDisplay.str(), healthXPos, 0);
		//When health runs out a small amount of health is given
		//back after drag has increased for a period of time.
		//Refer to if statement where boost <= 0 for more details.^^^
		if (health <= 0.0f)
		{
			int damageHealth = 10;
			boostDuration = 1.0f;
			health = damageHealth;
		}

		//Non-player car
		float differenceFromWay = 0.5f; //Changes waypoint, just before it reaches the previous waypoint, so the car doesn't flip around.
		float nonPlayerCarSpeed = 30.0f; //The speed non-player car travels at multiplied by frameTime.

		if (currentWP == WP1)
		{
			if ((aICar->GetX() >= waypoint[0]->GetX()) - differenceFromWay && (aICar->GetZ() >= waypoint[0]->GetZ() - differenceFromWay))
			{
				currentWP = WP2;
			}
			else
			{
				aICar->LookAt(waypoint[0]); //Car goes to first waypoint if it hasn't reached its values yet.
			}
		}
		else if (currentWP == WP2)
		{

			if ((aICar->GetX() >= waypoint[1]->GetX()) - differenceFromWay && (aICar->GetZ() >= waypoint[1]->GetZ() - differenceFromWay))
			{
				currentWP = WP3;
			}
			else
			{
				aICar->LookAt(waypoint[1]);//Car goes to second waypoint if it hasn't reached its values yet.
			}
		}
		else if (currentWP == WP3)
		{
			if ((aICar->GetX() >= waypoint[2]->GetX()) - differenceFromWay && (aICar->GetZ() >= waypoint[2]->GetZ() - differenceFromWay))
			{
				currentWP = WP4;
			}
			else
			{
				aICar->LookAt(waypoint[2]);//Car goes to third waypoint if it hasn't reached its values yet.
			}
		}
		else if (currentWP == WP4)
		{
			if ((aICar->GetX() >= waypoint[3]->GetX()) - differenceFromWay && (aICar->GetZ() >= waypoint[3]->GetZ() - differenceFromWay))
			{
				currentWP = WP5;
			}
			else
			{
				aICar->LookAt(waypoint[3]);//Car goes to fourth waypoint if it hasn't reached its values yet.
			}
		}
		else if (currentWP == WP5)
		{
			if ((aICar->GetX() >= waypoint[4]->GetX()) - differenceFromWay && (aICar->GetZ() >= waypoint[4]->GetZ() - differenceFromWay))
			{
				currentWP = WP6;
			}
			else
			{
				aICar->LookAt(waypoint[4]);//Car goes to fifth waypoint if it hasn't reached its values yet.
			}
		}
		else if (currentWP == WP6)
		{
			if ((aICar->GetX() >= waypoint[5]->GetX()) - differenceFromWay && (aICar->GetZ() >= waypoint[5]->GetZ() - differenceFromWay))
			{
				currentWP = WP7;
			}
			else
			{

				aICar->LookAt(waypoint[5]);//Car goes to sixth waypoint if it hasn't reached its values yet.
			}
		}
		else if (currentWP == WP7)
		{
			if ((aICar->GetX() >= waypoint[6]->GetX()) - differenceFromWay && (aICar->GetZ() >= waypoint[6]->GetZ() - differenceFromWay))
			{
				
				aICar->LookAt(waypoint[6]);//Car goes to seventh waypoint if it hasn't reached its values yet.
			}	
		}

		if (gameStarted)
		{
			aICar->MoveLocalZ(nonPlayerCarSpeed*frameTime); //Car moves along the local Z.  It changes direction when the lookat function is used.
		}

		//Quit the game.
		if (myEngine->KeyHit(quit))
		{
			myEngine->Stop();
		}
	}
	// Delete the 3D engine now we are finished with it
	myEngine->Delete();
}

vector2D Scalar(float s, vector2D v)
{
	return { s* v.x, s* v.z }; //Scales vectors up.
}

vector2D Sum3(vector2D v1, vector2D v2, vector2D v3)
{
	return { v1.x + v2.x + v3.x, v1.z + v2.z + v3.z }; //Adds all three force vectors up.
}

void CountDown(string &gettingReady, float &frameTime, float &countDown, bool &gameStarted)
{
	//The game counts down depending on the values of countdown when frametime is taken
	//off countdown each frame.  When countdown is below 3.0, 3 is played and so on.
	//When it reaches 0, the screen displays "Go".  Game started then becomes true
	//and you can move the hover car.
	if (currentState == Start)
	{
		if (countDown < 3.0f)
		{
			gettingReady = "3";
			if (countDown <= 2.0f)
			{
				gettingReady = "2";
				if (countDown <= 1.0f)
				{
					gettingReady = "1";
					if (countDown <= 0.0f)
					{
						gettingReady = "Go!";
						gameStarted = true;
					}
				}
			}
		}
	}
}

bool CheckpointPassed(float carPointX, float carPointZ, float checkpointX, float checkpointZ, float checkpointW, float checkpointD)
{
	float minX = checkpointX - checkpointW / 2;
	float maxX = checkpointX + checkpointW / 2;
	float minZ = checkpointZ - checkpointD / 2;
	float maxZ = checkpointZ + checkpointD / 2;

	return(carPointX > minX && carPointX < maxX && carPointZ > minZ && carPointZ < maxZ); //Returns true when all statements are true
																						  //When the point(hovercar) is inside the sphere(checkpoint) the statement becomes true.
}

boxSide car2Box(float carXPos, float carZPos, float oldCarXPos, float oldCarZPos, float carRad,
	float boxXPos, float boxZPos, float boxWidth, float boxDepth)
{
	float minX = boxXPos - boxWidth / 2 - carRad;
	float maxX = boxXPos + boxWidth / 2 + carRad;
	float minZ = boxZPos - boxDepth / 2 - carRad;
	float maxZ = boxZPos + boxDepth / 2 + carRad;

	boxSide result = NoSide; //If no blocks have been hit, then no side is the result.

	if (carXPos > minX && carXPos < maxX && carZPos > minZ && carZPos < maxZ) //A block has been hit.
	{
		//Works out which side has been hit.
		if (oldCarXPos < minX)
		{
			result = LeftSide;
		}
		else if (oldCarXPos > maxX)
		{
			result = RightSide;
		}
		else if (oldCarZPos < minZ)
		{
			result = FrontSide;
		}
		else if (oldCarZPos > maxZ)
		{
			result = BackSide;
		}
	}
	return(result); //Returns the side which has been hit to the main code, when method has been called.
}
bool car2Sphere(float carXPos, float carZPos, float carRad, float strutXPos, float strutZPos, float strutRad)
{
	float distX = carXPos - strutXPos;
	float distZ = carZPos - strutZPos;
	float dist = sqrt(distX*distX + distZ * distZ); //The current distance between the car and the strut.

													//Returns true if both the radii added together are greater than the 
													//current distance between the car and the strut, hence a collision is detected.
	return (dist < (carRad + strutRad));
}

int CarDamage(float speed, int health)
{
	float minimumDamSpeed = 10.0f;
	float firstTierLimit = 50.0f;
	float secondTierLimit = 120.0f;
	int firstTierDeduct = 1;
	int secondTierDeduct = 2;
	int thirdTierDeduct = 5;

	if (speed > minimumDamSpeed && speed < firstTierLimit)
	{
		return health - firstTierDeduct; //Low impact.
	}
	else if (speed > firstTierDeduct && speed < secondTierLimit)
	{
		return health - secondTierDeduct; //Medium impact.
	}
	else if (speed > secondTierLimit)
	{
		return health - thirdTierDeduct; //High impact.
	}
	else
	{
		return health - 0; //No or very minor impact.
	}
}
