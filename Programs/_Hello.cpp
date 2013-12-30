/*==============================================================
  character movement testing using Fly2

  - Load a scene
  - Generate a terrain object
  - Load a character
  - Control a character to move
  - Change poses

  (C)2012 Chuan-Chang Wang, All Rights Reserved
  Created : 0802, 2012

  Last Updated : 1007, 2013, Kevin C. Wang
 ===============================================================*/
#include "FlyWin32.h"

#define PI 3.141592658


VIEWPORTid vID;                 // the major viewport
SCENEid sID;                    // the 3D scene
OBJECTid cID, tID;              // the main camera and the terrain for terrain following
CHARACTERid actorID;            // the major character
ACTIONid idleID, runID, walkID, curPoseID ;

ACTIONid walkToRunID, walkToIdleID; // actions
ACTIONid idleToRunID, idleToWalkID; 
ACTIONid runToIdleID, runToWalkID; 

ROOMid terrainRoomID = FAILED_ID;
TEXTid textID = FAILED_ID;
OBJECTid lineTestID = FAILED_ID;

// some globals
int frame = 0;
int oldX, oldY, oldXM, oldYM, oldXMM, oldYMM;

bool movingBackward;
bool movingForward;
bool turingR;
bool turingL;
bool changing;
float beforeMovingCouter;

// hotkey callbacks
void QuitGame(BYTE, BOOL4);
void Movement(BYTE, BOOL4);

// timer callbacks
void GameAI(int);
void RenderIt(int);

// mouse callbacks
void InitPivot(int, int);
void PivotCam(int, int);
void InitMove(int, int);
void MoveCam(int, int);
void InitZoom(int, int);
void ZoomCam(int, int);

float focusArea[3];

float ptDist(float a[3], float b[3]){
	float res[3] = {
		a[0] - b[0],
		a[1] - b[1],
		a[2] - b[2]
	};
	return sqrt(res[0]*res[0]+res[1]*res[1]+res[2]*res[2]);
}

float minDst(const float a, const float b){
	if( a < b )
		return a;

	return b;
}

// Initial Parameter
#define _XY_DST 600.0f
#define _Z_DST 1900.0f
#define _VIEW_H_PAD 65
const float rotateStep = 0.3f;
const float moveStep   = 8.0f;

/*------------------
  the main program
  C.Wang 0720, 2006
 -------------------*/
void FyMain(int argc, char **argv){
   // create a new world
   BOOL4 beOK = FyStartFlyWin32("Homework #02 - Camera", 50, 20, 1280, 800, FALSE);
 
   // setup the data searching paths
   FySetModelPath("Data\\NTU\\\\Scenes");
   FySetTexturePath("Data\\NTU\\\\Scenes\\Textures");
   FySetScenePath("Data\\NTU\\\\Scenes");
   FySetShaderPath("Data\\NTU\\\\Shaders");

   // create a viewport
   vID = FyCreateViewport(0, 0, 1280, 800);
   FnViewport vp;
   vp.ID(vID);

   // create a 3D scene
   sID = FyCreateScene(10);
   FnScene scene;
   scene.ID(sID);

   // load the scene
   scene.Load("scene2");
   scene.SetAmbientLights(1.0f, 1.0f, 1.0f, 0.6f, 0.6f, 0.6f);

   // load the terrain
   tID = scene.CreateObject(OBJECT);
   FnObject terrain;
   terrain.ID(tID);
   BOOL beOK1 = terrain.Load("terrain");
   terrain.Show(FALSE);

   // set terrain environment
   terrainRoomID = scene.CreateRoom(SIMPLE_ROOM, 10);
   FnRoom room;
   room.ID(terrainRoomID);
   room.AddObject(tID);

   // load the character
   FySetModelPath("Data\\NTU\\\\Characters");
   FySetTexturePath("Data\\NTU\\\\Characters");
   FySetCharacterPath("Data\\NTU\\\\Characters");
   actorID = scene.LoadCharacter("Lyubu");

   // put the character on terrain
   float pos[3], fDir[3], uDir[3];
   float cpos[3], cfDir[3], cuDir[3];
   FnCharacter actor;
   actor.ID(actorID);
   // pos[0] = 3569.0f; pos[1] = -3208.0f; pos[2] = 1000.0f;
   pos[0] = 3941.0f; pos[1] = -3517.0f; pos[2] = 285.0f;
   fDir[0] = 1.0f; fDir[1] = 1.0f; fDir[2] = 0.0f;
   uDir[0] = 0.0f; uDir[1] = 0.0f; uDir[2] = 1.0f;
   actor.SetDirection(fDir, uDir);
   
   actor.SetTerrainRoom(terrainRoomID, 10.0f);
   beOK = actor.PutOnTerrain(pos);

   // Get character actions
   idleID = actor.GetBodyAction(NULL, "Idle");
   runID = actor.GetBodyAction(NULL, "Run");
   walkID = actor.GetBodyAction(NULL, "Walk");

   runToIdleID = actor.CreateCrossBlendAction(NULL,"runToIdle","Run","Idle",10.0f);
   runToWalkID = actor.CreateCrossBlendAction(NULL,"runToWalk","Run","Walk",10.0f);

   idleToWalkID = actor.CreateCrossBlendAction(NULL,"idleToWalk","Idle","Walk",10.0f);
   idleToRunID = actor.CreateCrossBlendAction(NULL,"idleToRun","Idle","Run",10.0f);

   walkToIdleID = actor.CreateCrossBlendAction(NULL,"walkToIdle","Walk","Idle",10.0f);
   walkToRunID = actor.CreateCrossBlendAction(NULL,"walkToRun","Walk","Run",10.0f);

   // set the character to idle action
   curPoseID = idleID;
   actor.SetCurrentAction(NULL, 0, curPoseID);
   actor.Play(START, 0.0f, FALSE, TRUE);
   //actor.TurnRight(90.0f);

   // translate the camera
   cID = scene.CreateObject(CAMERA);
   FnCamera camera;
   camera.ID(cID);
   camera.SetNearPlane(5.0f);
   camera.SetFarPlane(100000.0f);
   
   actor.GetPosition(pos);
   actor.GetDirection(fDir, uDir);

   for( int i=0 ; i<3 ; i++ )
	   focusArea[i] = pos[i];

   // set camera initial position and orientation
   float hitResult[3];
   cpos[0] = pos [0] - _XY_DST*fDir[0]; 
   cpos[1] = pos [1] - _XY_DST*fDir[1]; 
   cpos[2] = pos [2] + _Z_DST*sin(15.0f/180.0f*PI);

   cfDir[0]=  pos[0] - cpos[0];
   cfDir[1]=  pos[1] - cpos[1];
   cfDir[2]=  pos[2] - cpos[2] + _VIEW_H_PAD;

   double mx = sqrt(cfDir[0]*cfDir[0]+cfDir[1]*cfDir[1]+cfDir[2]*cfDir[2]);
   cfDir[0] /= mx;
   cfDir[1] /= mx;
   cfDir[2] /= mx;

   float kdir[3] = {fDir[1],-fDir[0],0};

   cuDir[0]=-cfDir[2] * fDir[0];
   cuDir[1]=-cfDir[2] * fDir[1];
   cuDir[2]=cfDir[0] * fDir[0]+cfDir[1] * fDir[1];

   camera.SetPosition(cpos);
   camera.SetDirection(cfDir, cuDir);

   FnLight lgt;
   lgt.ID(scene.CreateObject(LIGHT));
   lgt.Translate(70.0f, -70.0f, 70.0f, REPLACE);
   lgt.SetColor(1.0f, 1.0f, 1.0f);
   lgt.SetIntensity(1.0f);

   // create a text object
   textID = FyCreateText("Trebuchet MS", 18, FALSE, FALSE);

   // set Hotkeys
   FyDefineHotKey(FY_ESCAPE, QuitGame, FALSE);  // escape for quiting the game
   FyDefineHotKey(FY_UP, Movement, FALSE);      // Up for moving forward
   FyDefineHotKey(FY_DOWN, Movement, FALSE);      // Up for moving backward
   FyDefineHotKey(FY_RIGHT, Movement, FALSE);   // Right for turning right
   FyDefineHotKey(FY_LEFT, Movement, FALSE);    // Left for turning left
   FyDefineHotKey(FY_F1, Movement, FALSE);      // show/hide terrain boundary
   FyDefineHotKey(FY_W, Movement, FALSE);
   FyDefineHotKey(FY_A, Movement, FALSE);
   FyDefineHotKey(FY_S, Movement, FALSE);
   FyDefineHotKey(FY_D, Movement, FALSE);

   // define some mouse functions
   FyBindMouseFunction(LEFT_MOUSE, InitPivot, PivotCam, NULL, NULL);
   FyBindMouseFunction(MIDDLE_MOUSE, InitZoom, ZoomCam, NULL, NULL);
   FyBindMouseFunction(RIGHT_MOUSE, InitMove, MoveCam, NULL, NULL);

   // bind timers, frame rate = 30 fps
   FyBindTimer(0, 30.0f, GameAI, TRUE);
   FyBindTimer(1, 30.0f, RenderIt, TRUE);

   // invoke the system
   FyInvokeFly(TRUE);

   movingBackward=false;
   movingForward=false;
   turingR=false;
   turingL=false;
      
}

/*-------------------------------------------------------------
  30fps timer callback in fixed frame rate for major game loop
  C.Wang 1103, 2007
 --------------------------------------------------------------*/
typedef enum _MOVE_DIR_TAG{
	_MV_FW,
	_MV_BK,
	_MV_L,
	_MV_R,
	_MV_IDL
}_MOVE_DIR_TAG;

_MOVE_DIR_TAG lastMVTags = _MV_IDL;
_MOVE_DIR_TAG nowMVTags = _MV_IDL;

#define _FREE_MOVE_DST 80

float CHK_DST = 0.0f;
float lastPos[3] = {0.0f, 0.0f, 0.0f};

void GameAI(int skip){
	FnCharacter actor;
	FnCamera camera;
	FnObject terrain;
	float pos[3], fDir[3], uDir[3];
	float cpos[3], cfDir[3], cuDir[3];
	float hitResult[3];

	camera.ID(cID);
	camera.SetNearPlane(5.0f);
	camera.SetFarPlane(100000.0f);
	// play character pose
	actor.ID(actorID);
	actor.Play(LOOP, (float) skip, FALSE, TRUE);
	terrain.ID(tID); 

	// Homework #01 part 1
	if(changing){
	   if(movingForward){
		   if(curPoseID==idleID){
				actor.Play(ONCE, (float) skip, FALSE, TRUE);
				actor.SetCurrentAction(0, NULL, idleToRunID,20.0);	   
	
				actor.Play(LOOP, (float) skip, FALSE, TRUE);
				actor.SetCurrentAction(0, NULL, runID,20.0);
				beforeMovingCouter =10;   
		   }
		   else if(curPoseID == walkID){
				 actor.Play(ONCE, (float) skip, FALSE, TRUE);
				 actor.SetCurrentAction(0, NULL, walkToRunID,20.0);	   
	
				actor.Play(LOOP, (float) skip, FALSE, TRUE);
				actor.SetCurrentAction(0, NULL, runID,20.0);
				beforeMovingCouter =5;   
		   }		  		   
	
		   curPoseID = runID;	   
	   }
	   else if( turingL || turingR ){
		   if(curPoseID==idleID){
			   actor.Play(ONCE, (float) skip, FALSE, TRUE);
			   actor.SetCurrentAction(0, NULL, idleToWalkID,20.0);	   
	
			   actor.Play(LOOP, (float) skip, FALSE, TRUE);
			   actor.SetCurrentAction(0, NULL, walkID,20.0);
			   beforeMovingCouter =10;
		   }
		   else if(curPoseID == runID){
			   actor.Play(ONCE, (float) skip, FALSE, TRUE);
			   actor.SetCurrentAction(0, NULL, runToWalkID,20.0);	   
	
			   actor.Play(LOOP, (float) skip, FALSE, TRUE);
			   actor.SetCurrentAction(0, NULL, walkID,20.0);
			   beforeMovingCouter =5;   
		   }
		   curPoseID = walkID;	   
	
	   }
	   else if(movingBackward){
		   actor.TurnRight(180.0f);
		   if(curPoseID==idleID){
			   actor.Play(ONCE, (float) skip, FALSE, TRUE);
			   actor.SetCurrentAction(0, NULL, idleToRunID,20.0);	   
	
			   actor.Play(LOOP, (float) skip, FALSE, TRUE);
			   actor.SetCurrentAction(0, NULL, runID,20.0);
			   beforeMovingCouter =10;   
		   }
		   else if(curPoseID == walkID){
			   actor.Play(ONCE, (float) skip, FALSE, TRUE);
			   actor.SetCurrentAction(0, NULL, walkToRunID,20.0);	   
	
			   actor.Play(LOOP, (float) skip, FALSE, TRUE);
			   actor.SetCurrentAction(0, NULL, runID,20.0);
			   beforeMovingCouter =5;   
		   }
		   curPoseID = runID;	   
		   
		}
		changing=false;
	}

	
	if( movingForward ){
		if(beforeMovingCouter>0) {
			beforeMovingCouter-=1.0;
		}
		else{
			actor.GetPosition(pos);
			actor.GetDirection(fDir, uDir);

			if( terrain.HitTest(pos, fDir, hitResult)!=FAILED_ID ){
				if( ptDist(pos, hitResult) > moveStep )
					actor.MoveForward(moveStep, TRUE, TRUE, 0, TRUE);
			}
			actor.GetPosition(pos);
			actor.GetDirection(fDir, uDir);

			// set camera initial position and orientation
			fDir[0] = -fDir[0];
			fDir[1] = -fDir[1];
			camera.GetPosition(cpos);
			float cameaHT[3];
			if( terrain.HitTest(cpos, fDir, cameaHT) != FAILED_ID ){
				float getMin = minDst(fabs(cameaHT[0]-pos[0]), fabs(cameaHT[1]-pos[1]));
				float tmpVec[3] = {fabs(cameaHT[0]-pos[0]), fabs(cameaHT[1]-pos[1]), 0.0}, zero[3] = {0, 0, 0};
				float xydst = ptDist(tmpVec, zero);
				CHK_DST = xydst;
				if( _XY_DST*sqrt(2.0) > xydst ){
					cpos[0] = pos [0] + xydst*fDir[0]; 
					cpos[1] = pos [1] + xydst*fDir[1];
					cpos[2] = pos [2] + (_Z_DST + (_XY_DST*sqrt(2.0) - xydst))*sin(15.0f/180.0f*PI);
				}
				else{
					cpos[0] = pos [0] + _XY_DST*fDir[0]; 
					cpos[1] = pos [1] + _XY_DST*fDir[1]; 
					cpos[2] = pos [2] + _Z_DST*sin(15.0f/180.0f*PI);
				}

			}
			else{
				cpos[0] = pos [0] + _XY_DST*fDir[0]; 
				cpos[1] = pos [1] + _XY_DST*fDir[1]; 
				cpos[2] = pos [2] + _Z_DST*sin(15.0f/180.0f*PI);
			}
			fDir[0] = -fDir[0];
			fDir[1] = -fDir[1];

			cfDir[0]=  pos[0] - cpos[0];
			cfDir[1]=  pos[1] - cpos[1];
			cfDir[2]=  pos[2] - cpos[2] + _VIEW_H_PAD;

			double mx = sqrt(cfDir[0]*cfDir[0]+cfDir[1]*cfDir[1]+cfDir[2]*cfDir[2]);
			cfDir[0] /= mx;
			cfDir[1] /= mx;
			cfDir[2] /= mx;

			cuDir[0]=-cfDir[2] * fDir[0];
			cuDir[1]=-cfDir[2] * fDir[1];
			cuDir[2]=cfDir[0] * fDir[0]+cfDir[1] * fDir[1];

			if( ptDist(focusArea, pos) > _FREE_MOVE_DST ){
				camera.SetPosition(cpos);
				camera.SetDirection(cfDir, cuDir);
			}
			// End of If
		}
	}

	if(movingBackward ){
		if(beforeMovingCouter>0){
			beforeMovingCouter-=1.0;
		}
		else{
			// actor.MoveForward(moveStep);
			actor.GetPosition(pos);
			actor.GetDirection(fDir, uDir);

			if( terrain.HitTest(pos, fDir, hitResult)!=FAILED_ID ){
				if( ptDist(pos, hitResult) > moveStep )
					actor.MoveForward(moveStep, TRUE, TRUE, 0, TRUE);
			}
			// actor.GetPosition(pos);
			// actor.GetDirection(fDir, uDir);

			// fDir[0] = -fDir[0];
			// fDir[1] = -fDir[1];
			// set camera initial position and orientation

			camera.GetPosition(cpos);
			float cameaHT[3];

			if( terrain.HitTest(cpos, fDir, cameaHT) != FAILED_ID ){
				float getMin = minDst(fabs(cameaHT[0]-pos[0]), fabs(cameaHT[1]-pos[1]));
				float tmpVec[3] = {fabs(cameaHT[0]-pos[0]), fabs(cameaHT[1]-pos[1]), 0.0}, zero[3] = {0, 0, 0};
				float xydst = ptDist(tmpVec, zero);
				CHK_DST = xydst;
				if( _XY_DST*sqrt(2.0) > xydst ){
					cpos[0] = pos [0] + xydst*fDir[0]; 
					cpos[1] = pos [1] + xydst*fDir[1];
					cpos[2] = pos [2] + (_Z_DST + (_XY_DST*sqrt(2.0) - xydst))*sin(15.0f/180.0f*PI);
				}
				else{
					cpos[0] = pos [0] + _XY_DST*fDir[0]; 
					cpos[1] = pos [1] + _XY_DST*fDir[1]; 
					cpos[2] = pos [2] + _Z_DST*sin(15.0f/180.0f*PI);
				}
				// cpos[2] = pos[2];
				// 
				// if( terrain.HitTest(cpos, fDir, hitResult)!=FAILED_ID ){
				// 	// Hit the wall-like plane
				// 	float hitDist = ptDist(cpos, hitResult);
				// 	float charToHit = ptDist(pos, hitResult);
				// 
				// 	float getMin = minDst(fabs(hitResult[0]-pos[0]), fabs(hitResult[1]-pos[1]));
				// 	if( _XY_DST > getMin ){
				// 		cpos[0] = pos [0] + getMin*fDir[0]; 
				// 		cpos[1] = pos [1] + getMin*fDir[1];
				// 		cpos[2] = pos [2] + (800.0f + (_XY_DST-getMin))*sin(15.0f/180.0f*PI);
				// 	}
				// 	else{
				// 		cpos[0] = pos [0] + _XY_DST*fDir[0]; 
				// 		cpos[1] = pos [1] + _XY_DST*fDir[1]; 
				// 		cpos[2] = pos [2] + 800.0f*sin(15.0f/180.0f*PI);
				// 	}
				// }
				// else{
				// 	cpos[0] = pos [0] + _XY_DST*fDir[0]; 
				// 	cpos[1] = pos [1] + _XY_DST*fDir[1]; 
				// 	cpos[2] = pos [2] + 800.0f*sin(15.0f/180.0f*PI);
				// }
			}
			else{
				cpos[0] = pos [0] + _XY_DST*fDir[0]; 
				cpos[1] = pos [1] + _XY_DST*fDir[1]; 
				cpos[2] = pos [2] + _Z_DST*sin(15.0f/180.0f*PI);
			}

			fDir[0] = -fDir[0];
			fDir[1] = -fDir[1];

			cfDir[0]=  pos[0] - cpos[0] ;
			cfDir[1]=  pos[1] - cpos[1] ;
			cfDir[2]=  pos[2] - cpos[2] + _VIEW_H_PAD;

			double mx = sqrt(cfDir[0]*cfDir[0]+cfDir[1]*cfDir[1]+cfDir[2]*cfDir[2]);
			cfDir[0] /= mx;
			cfDir[1] /= mx;
			cfDir[2] /= mx;

			cuDir[0]=-cfDir[2] * fDir[0];
			cuDir[1]=-cfDir[2] * fDir[1];
			cuDir[2]=cfDir[0] * fDir[0]+cfDir[1] * fDir[1];

			if( ptDist(focusArea, pos) > _FREE_MOVE_DST ){
				camera.SetPosition(cpos);
				camera.SetDirection(cfDir, cuDir);
			}

			// camera.SetPosition(cpos);
			// camera.SetDirection(cfDir, cuDir);
			// end of If
		}
	}

	// Moving Position
	if( !(movingForward || movingBackward) ){
		if(turingL){   
			if(beforeMovingCouter>0) {
				beforeMovingCouter-=1.0;
			}
			else {
				camera.GetDirection(cfDir,cuDir);
				camera.GetPosition(cpos);
				float tpos[3];

				actor.GetPosition(tpos);
				pos[0] = cpos[0] + (tpos[0]-cpos[0])*cos(-rotateStep/180.0f*PI) - (tpos[1]-cpos[1])*sin(-rotateStep/180.0f*PI);
				pos[1] = cpos[1] + (tpos[0]-cpos[0])*sin(-rotateStep/180.0f*PI) + (tpos[1]-cpos[1])*cos(-rotateStep/180.0f*PI);
				pos[2] = tpos[2];
				fDir[0] = pos[1] - cpos[1];
				fDir[1] = -pos[0] + cpos[0];
				fDir[2] = 0.0f;
				uDir[0] = 0.0f;
				uDir[1] = 0.0f;
				uDir[2] = 1.0f;
				actor.PutOnTerrain(pos);
				actor.SetDirection(fDir,uDir);
				
				cfDir[0] = pos[0] - cpos[0];
				cfDir[1] = pos[1] - cpos[1];
				cfDir[2] = pos[2] - cpos[2] + _VIEW_H_PAD;
				double mx = sqrt(cfDir[0]*cfDir[0]+cfDir[1]*cfDir[1]+cfDir[2]*cfDir[2]);
				cfDir[0] /= mx;
				cfDir[1] /= mx;
				cfDir[2] /= mx;

				float kdir[3] = {fDir[0],fDir[1],fDir[2]};
				cuDir[0]=cfDir[2] * kdir[1];
				cuDir[1]=-cfDir[2] * kdir[0];
				cuDir[2]=cfDir[1] * kdir[0]-cfDir[0] * kdir[1];

				actor.GetPosition(pos);
				actor.GetDirection(fDir, uDir);
				// if( terrain.HitTest(pos, fDir, hitResult)!=NONE ){
					
					if( ptDist(pos, lastPos) < 1 ){
						cpos[0] = pos[0] + (cpos[0]-pos[0])*cos(-rotateStep*2.0/180.0f*PI) - (cpos[1]-pos[1])*sin(-rotateStep*2.0/180.0f*PI);
						cpos[1] = pos[1] + (cpos[0]-pos[0])*sin(-rotateStep*2.0/180.0f*PI) + (cpos[1]-pos[1])*cos(-rotateStep*2.0/180.0f*PI);
						camera.SetPosition(cpos);
						camera.SetDirection(cfDir, cuDir);
					}
					else{
						if( ptDist(focusArea, pos) > _FREE_MOVE_DST ){
							camera.SetDirection(cfDir, cuDir);
						}
					}
				// }
								
				// End of If
				// camera.SetDirection(cfDir,cuDir);
			}
		}

		if(turingR){
			if(beforeMovingCouter>0){
				beforeMovingCouter-=1.0;
			}
			else{
				camera.GetDirection(cfDir,cuDir);
				camera.GetPosition(cpos);
				float tpos[3];

				actor.GetPosition(tpos);
				pos[0] = cpos[0] + (tpos[0]-cpos[0])*cos(rotateStep/180.0f*PI) - (tpos[1]-cpos[1])*sin(rotateStep/180.0f*PI);
				pos[1] = cpos[1] + (tpos[0]-cpos[0])*sin(rotateStep/180.0f*PI) + (tpos[1]-cpos[1])*cos(rotateStep/180.0f*PI);
				pos[2] = tpos[2];

				fDir[0] = -pos[1] + cpos[1];
				fDir[1] = pos[0] - cpos[0];
				fDir[2] = 0.0f;
				uDir[0] = 0.0f;
				uDir[1] = 0.0f;
				uDir[2] = 1.0f;
				actor.PutOnTerrain(pos);
				actor.SetDirection(fDir,uDir);

				cfDir[0] = pos[0] - cpos[0];
				cfDir[1] = pos[1] - cpos[1];
				cfDir[2] = pos[2] - cpos[2] + _VIEW_H_PAD;
				double mx = sqrt(cfDir[0]*cfDir[0]+cfDir[1]*cfDir[1]+cfDir[2]*cfDir[2]);
				cfDir[0] /= mx;
				cfDir[1] /= mx;
				cfDir[2] /= mx;

				float kdir[3] = {-fDir[0],-fDir[1],-fDir[2]};
				cuDir[0]=cfDir[2] * kdir[1];
				cuDir[1]=-cfDir[2] * kdir[0];
				cuDir[2]=cfDir[1] * kdir[0]-cfDir[0] * kdir[1];
				
				actor.GetPosition(pos);
				actor.GetDirection(fDir, uDir);
				// if( terrain.HitTest(pos, fDir, hitResult)!=NONE ){
				CHK_DST = ptDist(pos, lastPos);
				if( ptDist(pos, lastPos) < 1 ){
					cpos[0] = pos[0] + (cpos[0]-pos[0])*cos(rotateStep*2.0/180.0f*PI) - (cpos[1]-pos[1])*sin(rotateStep*2.0/180.0f*PI);
					cpos[1] = pos[1] + (cpos[0]-pos[0])*sin(rotateStep*2.0/180.0f*PI) + (cpos[1]-pos[1])*cos(rotateStep*2.0/180.0f*PI);
					camera.SetPosition(cpos);
					camera.SetDirection(cfDir, cuDir);
				}
				else{
					if( ptDist(focusArea, pos) > _FREE_MOVE_DST ){
						camera.SetDirection(cfDir, cuDir);
					}
				}
				// }
				// End of If
				// camera.SetDirection(cfDir,cuDir);
			}
		}
	}
   
	actor.GetPosition(pos);
	for( int i=0 ; i<3 ; i++ )
		lastPos[i] = pos[i];

   // Stop Moving
   if(!(turingL || turingR || movingForward || movingBackward) && curPoseID!=idleID){
		if(curPoseID==runID){
			actor.Play(ONCE, (float) skip, FALSE, TRUE);
			actor.SetCurrentAction(0, NULL, runToIdleID,20.0);
			actor.Play(LOOP, (float) skip, FALSE, TRUE);
			actor.SetCurrentAction(0, NULL, idleID,20.0);
		}
		else if(curPoseID == walkID){
			actor.Play(ONCE, (float) skip, FALSE, TRUE);
			actor.SetCurrentAction(0, NULL, walkToIdleID,20.0);
			actor.Play(LOOP, (float) skip, FALSE, TRUE);
			actor.SetCurrentAction(0, NULL, idleID,20.0);
		}
		curPoseID = idleID;
		if( nowMVTags == _MV_BK )
			actor.TurnRight(180);
		else if( nowMVTags == _MV_L )
			actor.TurnRight(-90);
		else if( nowMVTags == _MV_R )
			actor.TurnRight(90);
		actor.GetPosition(pos);
		actor.GetDirection(fDir, uDir);
		
		// set camera initial position and orientation
		cpos[0] = pos [0] - 600.0f*cos(15.0f/180.0f*PI)*fDir[0]; 
		cpos[1] = pos [1] - 600.0f*cos(15.0f/180.0f*PI)*fDir[1]; 
		cpos[2] = pos [2] + 600.0f*sin(15.0f/180.0f*PI);
		
		cfDir[0]=cos(15.0f/180.0f*PI)*fDir[0]/(sqrt(cos(15.0f/180.0f*PI)*cos(15.0f/180.0f*PI)*fDir[0]*fDir[0] + cos(15.0f/180.0f*PI)*cos(15.0f/180.0f*PI)*fDir[1]*fDir[1]+sin(15.0f/180.0f*PI)*sin(15.0f/180.0f*PI) ));
		cfDir[1]=cos(15.0f/180.0f*PI)*fDir[1]/(sqrt(cos(15.0f/180.0f*PI)*cos(15.0f/180.0f*PI)*fDir[0]*fDir[0] + cos(15.0f/180.0f*PI)*cos(15.0f/180.0f*PI)*fDir[1]*fDir[1]+sin(15.0f/180.0f*PI)*sin(15.0f/180.0f*PI) ));
		cfDir[2]=-sin(15.0f/180.0f*PI)/(sqrt(cos(15.0f/180.0f*PI)*cos(15.0f/180.0f*PI)*fDir[0]*fDir[0] + cos(15.0f/180.0f*PI)*cos(15.0f/180.0f*PI)*fDir[1]*fDir[1]+sin(15.0f/180.0f*PI)*sin(15.0f/180.0f*PI) ));
		
		float kdir[3] = {fDir[1],-fDir[0],0};
		
		cuDir[0]=-cfDir[2] * fDir[0];
		cuDir[1]=-cfDir[2] * fDir[1];
		cuDir[2]=cfDir[0] * fDir[0]+cfDir[1] * fDir[1];
		
		// camera.SetPosition(cpos);
		// camera.SetDirection(cfDir, cuDir);
		lastMVTags = nowMVTags;
		nowMVTags  = _MV_IDL;
		if( ptDist(focusArea, pos) > _FREE_MOVE_DST ){
			for( int i=0 ; i<3 ; i++ )
				focusArea[i] = pos[i];
		}
		// End of if
   }
}

/*----------------------
  perform the rendering
  C.Wang 0720, 2006
 -----------------------*/
void RenderIt(int skip){
   FnViewport vp;

   // render the whole scene
   vp.ID(vID);
   vp.Render3D(cID, TRUE, TRUE);

   // get camera's data
   FnCamera camera;
   camera.ID(cID);

   float pos[3], fDir[3], uDir[3];
   camera.GetPosition(pos);
   camera.GetDirection(fDir, uDir);

   // show frame rate
   static char string[128];
   if (frame == 0) {
      FyTimerReset(0);
   }

   if (frame/10*10 == frame) {
      float curTime;

      curTime = FyTimerCheckTime(0);
      sprintf(string, "Fps: %6.2f", frame/curTime);
   }

   frame += skip;
   if (frame >= 1000) {
      frame = 0;
   }

   FnText text;
   text.ID(textID);

   text.Begin(vID);
   text.Write(string, 20, 20, 255, 0, 0);

   char posS[256], fDirS[256], uDirS[256], temp[256];
   char charPos[256], charDir[256];
   sprintf(posS, "pos: %8.3f %8.3f %8.3f", pos[0], pos[1], pos[2]);
   sprintf(fDirS, "facing: %8.3f %8.3f %8.3f", fDir[0], fDir[1], fDir[2]);
   sprintf(uDirS, "up: %8.3f %8.3f %8.3f", uDir[0], uDir[1], uDir[2]);

   FnCharacter actor;
   actor.ID(actorID);
   actor.GetPosition(pos);
   actor.GetDirection(fDir, uDir);
   sprintf(temp, "Hit Dist: %f\n", CHK_DST);
   sprintf(charPos, "Char Pos: %f %f %f\n", pos[0], pos[1], pos[2]);
   sprintf(charDir, "Char fDir: %f %f %f\n", fDir[0], fDir[1], fDir[2]);

   text.Write(posS, 20, 35, 255, 255, 0);
   text.Write(fDirS, 20, 50, 255, 255, 0);
   text.Write(uDirS, 20, 65, 255, 255, 0);
   text.Write(temp, 20, 80, 255, 255, 0);
   text.Write(charPos, 20, 95, 255, 255, 0);
   text.Write(charDir, 20, 110, 255, 255, 0);

   text.End();

   // swap buffer
   FySwapBuffers();
}
/*------------------
  movement control
  C.Wang 1103, 2006
 -------------------*/
void Movement(BYTE code, BOOL4 value)
{
	 FnCharacter actor;
	 actor.ID(actorID);

	if(code == 38 || code == FY_W ){
		if(value){
			movingForward=true;
			changing=true;
			nowMVTags = _MV_FW;
		}
		else{
			movingForward=false;
		}
	}

	if(code ==40 || code == FY_S ){
		if(value){
			movingBackward=true;
			changing=true;
			nowMVTags = _MV_BK;
		}
		else{
			movingBackward=false;
		}
	}

	if(code == 39 || code == FY_D ){
		if(value){
			turingL=true;
			nowMVTags = _MV_L;
		}
		else{
			turingL=false;
		}
	}

	if(code == 37 || code == FY_A ){
		if(value){
			turingR=true;
			nowMVTags = _MV_R;
		}
		else{
			turingR=false;
		}
	}

	changing=true;
   // Homework #01 part 2
}


/*------------------
  quit the demo
  C.Wang 0327, 2005
 -------------------*/
void QuitGame(BYTE code, BOOL4 value)
{
   if (code == FY_ESCAPE) {
      if (value) {
         FyQuitFlyWin32();
      }
   }
}

/*-----------------------------------
  initialize the pivot of the camera
  C.Wang 0329, 2005
 ------------------------------------*/
void InitPivot(int x, int y)
{
   oldX = x;
   oldY = y;
   frame = 0;
}


/*------------------
  pivot the camera
  C.Wang 0329, 2005
 -------------------*/
void PivotCam(int x, int y)
{
   FnObject model;

   if (x != oldX) {
      model.ID(cID);
      model.Rotate(Z_AXIS, (float) (x - oldX), GLOBAL);
      oldX = x;
   }

   if (y != oldY) {
      model.ID(cID);
      model.Rotate(X_AXIS, (float) (y - oldY), GLOBAL);
      oldY = y;
   }
}


/*----------------------------------
  initialize the move of the camera
  C.Wang 0329, 2005
 -----------------------------------*/
void InitMove(int x, int y){
   oldXM = x;
   oldYM = y;
   frame = 0;
}


/*------------------
  move the camera
  C.Wang 0329, 2005
 -------------------*/
void MoveCam(int x, int y){
   if (x != oldXM) {
      FnObject model;

      model.ID(cID);
      model.Translate((float)(x - oldXM)*2.0f, 0.0f, 0.0f, LOCAL);
      oldXM = x;
   }
   if (y != oldYM) {
      FnObject model;

      model.ID(cID);
      model.Translate(0.0f, (float)(oldYM - y)*2.0f, 0.0f, LOCAL);
      oldYM = y;
   }
}


/*----------------------------------
  initialize the zoom of the camera
  C.Wang 0329, 2005
 -----------------------------------*/
void InitZoom(int x, int y)
{
   oldXMM = x;
   oldYMM = y;
   frame = 0;
}


/*------------------
  zoom the camera
  C.Wang 0329, 2005
 -------------------*/
void ZoomCam(int x, int y){
   if (x != oldXMM || y != oldYMM) {
      FnObject model;

      model.ID(cID);
      model.Translate(0.0f, 0.0f, (float)(x - oldXMM)*10.0f, LOCAL);
      oldXMM = x;
      oldYMM = y;
   }
}