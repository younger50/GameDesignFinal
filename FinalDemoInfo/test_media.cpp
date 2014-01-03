/*==============================================================
  This is a demo program for TheFly3D System

  Hello !

  (C)2004 Chuan-Chang Wang, All Rights Reserved
  Created : 0303, 2004, C. Wang

  Last Updated : 1010, 2004, C.Wang
 ===============================================================*/
#include "FlyWin32.h"
#include "FyMedia.h"

int frame = 0;
int oldX, oldY, oldXM, oldYM, oldXMM, oldYMM;

VIEWPORTid vID;
SCENEid sID;
OBJECTid nID, cID, lID;
MEDIAid mmID;

BOOL4 beFullScreen = FALSE;

void QuitGame(BYTE, BOOL4);
void SaveCW3(BYTE, BOOL4);
void GameAI(int);

void InitPivot(int, int);
void PivotCam(int, int);
void InitMove(int, int);
void MoveCam(int, int);
void InitZoom(int, int);
void ZoomCam(int, int);


/*------------------
  the main program
  C.Wang 0308, 2004
 -------------------*/
void FyMain(int argc, char **argv)
{
   // create a new world
   FyStartFlyWin32("Hello TheFly3D !", 0, 0, 800, 600, beFullScreen);

   FySetModelPath("Data\\Models");
   FySetTexturePath("Data\\textures");
   FySetShaderPath("Data\\Shaders");

   FyBeginMedia("data\\media", 2);
   //mmID = FyCreateMediaPlayer("dawn.mpg", 0, 0, 800, 600);
   //mmID = FyCreateMediaPlayer("opening.avi", 0, 0, 800, 600);
   mmID = FyCreateMediaPlayer("MUSIC_fogforest.mp3", 0, 0, 800, 600);
   FnMedia mP;
   mP.Object(mmID);
   mP.Play(ONCE);
   //mP.SetVolume(0.1f);
   
   // create a viewport
   vID = FyCreateViewport(0, 0, 800, 600);
   FnViewport vp;
   vp.Object(vID);
   vp.SetBackgroundColor(0.3f, 0.4f, 0.5f);

   // create a 3D scene & the 3D entities
   sID = FyCreateScene(1);

   FnScene scene;
   scene.Object(sID);
   nID = scene.CreateObject(OBJECT);
   cID = scene.CreateObject(CAMERA);
   lID = scene.CreateObject(LIGHT);

   // load the teapot
   FnObject model;
   model.ID(nID);
   model.Load("teapot.cw3");
   model.Translate(20.0f, 0.0f, 0.0f, GLOBAL);
   
   // translate the camera
   FnCamera camera;
   camera.ID(cID);
   camera.Rotate(X_AXIS, 90.0f, LOCAL);
   camera.SetAspectRatio(800.0f/600.0f);
   camera.Translate(0.0f, 10.0f, 200.0f, LOCAL);

   // translate the light
   FnLight light;
   light.ID(lID);
   light.Translate(-30.0f, -30.0f, 0.0f, GLOBAL);
   light.SetName("MainLight");
   light.SetColor(0.9f, 0.9f, 0.7f);

   // set Hotkeys
   FyDefineHotKey(FY_ESCAPE, QuitGame, FALSE);

   // define some mouse functions
   FyBindMouseFunction(LEFT_MOUSE, InitPivot, PivotCam, NULL, NULL);
   FyBindMouseFunction(MIDDLE_MOUSE, InitZoom, ZoomCam, NULL, NULL);
   FyBindMouseFunction(RIGHT_MOUSE, InitMove, MoveCam, NULL, NULL);

   /* bind a timer, frame rate = 30 fps */
   FyBindTimer(0, 30.0f, GameAI, TRUE);

   // invoke the system
   FyInvokeFly(TRUE);
}


//------------------------------------------------------------------------------------
// timer callback function which will be invoked by TheFly3D System every 1/30 second
// C.Wang 0308, 2004
//------------------------------------------------------------------------------------
void GameAI(int skip)
{
   if (mmID != FAILED_ID) {
      FnMedia md;
      md.Object(mmID);
      if (md.GetState() == MEDIA_STOPPED) {
         // after playing, delete the media object
         FyDeleteMediaPlayer(mmID);
         mmID = FAILED_ID;
      }
      else {
         return;
      }
   }

   FnLight lgt;
   lgt.Object(lID);
   lgt.Rotate(Z_AXIS, 0.5f, GLOBAL);

   FnObject model;
   model.Object(nID);
   model.Rotate(Z_AXIS, 3.0f, GLOBAL);

   // render the scene
   FnViewport vp;
   vp.Object(vID);
   vp.Render3D(cID, TRUE, TRUE);

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

   FySwapBuffers();
}


//-------------------
// quit the demo
// C.Wang 0304, 2004
//-------------------
void QuitGame(BYTE code, BOOL4 value)
{
   if (code == FY_ESCAPE) {
      if (value) {
         FyEndMedia();
         FyQuitFlyWin32();
      }
   }
}


/*-----------------------------------
  initialize the pivot of the camera
  C.Wang 0329, 2004
 ------------------------------------*/
void InitPivot(int x, int y)
{
   oldX = x;
   oldY = y;
   frame = 0;
}


/*------------------
  pivot the camera
  C.Wang 0329, 2004
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
  C.Wang 0329, 2004
 -----------------------------------*/
void InitMove(int x, int y)
{
   oldXM = x;
   oldYM = y;
   frame = 0;
}


/*------------------
  move the camera
  C.Wang 0329, 2004
 -------------------*/
void MoveCam(int x, int y)
{
   if (x != oldXM) {
      FnObject model;

      model.ID(cID);
      model.Translate((float)(x - oldXM)*0.1f, 0.0f, 0.0f, LOCAL);
      oldXM = x;
   }
   if (y != oldYM) {
      FnObject model;

      model.ID(cID);
      model.Translate(0.0f, (float)(oldYM - y)*0.1f, 0.0f, LOCAL);
      oldYM = y;
   }
}


/*----------------------------------
  initialize the zoom of the camera
  C.Wang 0329, 2004
 -----------------------------------*/
void InitZoom(int x, int y)
{
   oldXMM = x;
   oldYMM = y;
   frame = 0;
}


/*------------------
  zoom the camera
  C.Wang 0329, 2004
 -------------------*/
void ZoomCam(int x, int y)
{
   if (x != oldXMM || y != oldYMM) {
      FnObject model;

      model.ID(cID);
      model.Translate(0.0f, 0.0f, (float)(x - oldXMM), LOCAL);
      oldXMM = x;
      oldYMM = y;
   }
}