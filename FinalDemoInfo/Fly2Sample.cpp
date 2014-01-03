/*==============================================================
  Test Program 01

  (C)2012, BlackSmith Technology, All Rights Reserved
  Created : 0706, 2012

  Last Updated : 1217, 2013, Chuan-Chang Wang
 ===============================================================*/
#include "FlyWin32.h"


VIEWPORTid vID;                 // the major viewport
SCENEid sID;                    // the 3D scene
OBJECTid cID, tID;              // the main camera and the terrain for terrain following
CHARACTERid actorID;            // the major character
ACTIONid idleID, runID, curPoseID; // two actions
OBJECTid baseID;                // the base object ID of the main character


// for rendering targets
MATERIALid matRenderTargetID = FAILED_ID;
TEXTUREid texRenderTargetID = FAILED_ID;
SHADERid postShaderID = FAILED_ID;
TEXTUREid depthMapID = FAILED_ID;

GEOMETRYid bloodBarID = FAILED_ID;

BOOL4 beRenderToDepthmap = FALSE;
BOOL4 beRenderTotexture = FALSE;

TEXTid textID = FAILED_ID;


// some globals
int frame = 0;
int oldX, oldY, oldXM, oldYM, oldXMM, oldYMM;

// hotkey callbacks
void QuitGame(BYTE, BOOL4);
void Movement(BYTE, BOOL4);
void SnapShot(BYTE, BOOL4);
void Demo(BYTE, BOOL4);

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

/*------------------
  the main program
  C.Wang 0706, 2012
 -------------------*/
void FyMain(int argc, char **argv)
{
   // create a new world
   FyStartFlyWin32("Fly2 Demo", 0, 0, 1024, 768, FALSE);

   // create a viewport
   vID = FyCreateViewport(0, 0, 1024, 768);
   FnViewport vp;
   vp.ID(vID);

   // create a 3D scene
   sID = FyCreateScene(10);
   FnScene scene;
   scene.ID(sID);

   // set terrain environment
   FySetModelPath("Data\\NTU\\Scenes");
   FySetTexturePath("Data\\NTU\\Scenes\\Textures");
   FySetScenePath("Data\\NTU\\Scenes");
   FySetShaderPath("Data\\NTU\\Shaders");

   // load the scene
   scene.Load("scene2");
   scene.SetAmbientLights(1.0f, 1.0f, 1.0f, 0.6f, 0.6f, 0.6f);

   // load the character
   FySetModelPath("Data\\NTU\\Characters");
   FySetTexturePath("Data\\NTU\\Characters");
   FySetCharacterPath("Data\\NTU\\Characters");
   actorID = scene.LoadCharacter("Lyubu");

   // put the character on terrain
   float pos[3], fDir[3], uDir[3];
   FnCharacter actor;
   actor.ID(actorID);
   pos[0] = 3569.0f; pos[1] = -3208.0f; pos[2] = 0.89664f;
   fDir[0] = 1.0f; fDir[1] = 1.0f; fDir[2] = 0.0f;
   uDir[0] = 0.0f; uDir[1] = 0.0f; uDir[2] = 1.0f;
   actor.SetPosition(pos);
   actor.SetDirection(fDir, uDir);

   // set character pose
   idleID = actor.GetBodyAction(NULL, "Idle");
   runID = actor.GetBodyAction(NULL, "Run");

   // get base object of the character
   baseID = actor.GetBaseObject();

   // set the character to waiting
   curPoseID = idleID;
   actor.SetCurrentAction(NULL, 0, curPoseID);
   actor.Play(START, 0.0f, FALSE, TRUE);
   actor.TurnRight(90.0f);

   // translate the camera
   cID = scene.CreateObject(CAMERA);
   FnCamera camera;
   camera.ID(cID);
   camera.SetNearPlane(5.0f);
   camera.SetFarPlane(3000.0f);

   // set camera initial position and orientation
   pos[0] = 4315.783f; pos[1] = -3199.686f; pos[2] = 93.046f;
   fDir[0] = -0.983f; fDir[1] = -0.143f; fDir[2] = -0.119f;
   uDir[0] = -0.116f; uDir[1] = -0.031f; uDir[2] = 0.993f;
   camera.SetPosition(pos);
   camera.SetDirection(fDir, uDir);

   // create a billboard and set parent to base object of the character to demo the simple bloodbar
   OBJECTid bbID = scene.CreateObject(OBJECT);
   FnObject bb(bbID);
   bb.SetParent(baseID);
   bb.Translate(0.0f, 0.0f, 95.0f, REPLACE);

   float size[2], color[4];

   size[0] = 25.0f;
   size[1] = 1.2f;
   color[0] = color[3] = 1.0f; color[1] = color[2] = 0.0f;
   bloodBarID = bb.Billboard(NULL, size, NULL, 0, color);

   FnLight lgt;
   lgt.ID(scene.CreateObject(LIGHT));
   lgt.Translate(70.0f, -70.0f, 70000.0f, REPLACE);
   lgt.SetColor(1.0f, 1.0f, 1.0f);
   lgt.SetIntensity(1.0f);
   lgt.SetName("mainLight");

   // create a material for rendering tagets
   matRenderTargetID = FyCreateMaterial(NULL, NULL, NULL, 10.0f, NULL);
   FnMaterial mat(matRenderTargetID);
   texRenderTargetID = mat.AddRenderTarget(0, 0, "BatchRendering", TEXTURE_32, 1024, 768);

   FySetShaderPath("Data\\Shaders");
   postShaderID = mat.AddShaderEffect("ScaleBuffer", "ScaleBuffer");

   // create a depth map
   depthMapID = FyCreateRenderTargetTexture("RenderTarget00", TEXTURE_1F32, 1024, 768);

   textID = FyCreateText("Trebuchet MS", 18, FALSE, FALSE);

   // set Hotkeys
   FyDefineHotKey(FY_ESCAPE, QuitGame);  // escape for quiting the game
   FyDefineHotKey(FY_UP, Movement);      // Up for moving forward
   FyDefineHotKey(FY_RIGHT, Movement);   // Right for turning right
   FyDefineHotKey(FY_LEFT, Movement);    // Left for turning left
   FyDefineHotKey(FY_F1, SnapShot);
   FyDefineHotKey(FY_F2, SnapShot);
   FyDefineHotKey(FY_F3, Demo);
   FyDefineHotKey(FY_F4, Demo);

   // define some mouse functions
   FyBindMouseFunction(LEFT_MOUSE, InitPivot, PivotCam, NULL, NULL);
   FyBindMouseFunction(MIDDLE_MOUSE, InitZoom, ZoomCam, NULL, NULL);
   FyBindMouseFunction(RIGHT_MOUSE, InitMove, MoveCam, NULL, NULL);

   // bind timers, frame rate = 30 fps
   FyBindTimer(0, 30.0f, GameAI, TRUE);
   FyBindTimer(1, 30.0f, RenderIt, TRUE);

   // invoke the system
   FyInvokeFly();
}


/*-------------------------------------------------------------
  30fps timer callback in fixed frame rate for major game loop
  C.Wang 1103, 2007
 --------------------------------------------------------------*/
void GameAI(int skip)
{
   FnCharacter actor;

   // play character pose
   actor.ID(actorID);
   actor.Play(LOOP, (float) skip, FALSE, TRUE);

   // The Homework #1 part 1
   float dist = 6.0f;
   if (FyCheckHotKeyStatus(FY_UP)) {
      actor.MoveForward(dist, TRUE, FALSE, 0.0f, TRUE);
   }
   
   if (FyCheckHotKeyStatus(FY_LEFT)) {
      actor.TurnRight(-2.0f);
   }
   
   if (FyCheckHotKeyStatus(FY_RIGHT)) {
      actor.TurnRight(2.0f);
   }
}


/*----------------------
  perform the rendering
  C.Wang 0720, 2006
 -----------------------*/
void RenderIt(int skip)
{
   FnViewport vp;
   vp.ID(vID);

   if (beRenderToDepthmap) {
      // render the depth map
      vp.SetRenderTarget(0, depthMapID);
      vp.RenderDepth(cID, UNIT_DEPTH_MAP, TRUE);
      vp.SetRenderTarget(0, BACK_BUFFER);
      beRenderToDepthmap = FALSE;
   }

   // set rendering targets if necessary
   if (beRenderTotexture) {
      vp.SetRenderTarget(0, texRenderTargetID);
   }

   // render the whole scene
   vp.Render3D(cID, TRUE, TRUE);

   // recover the rendering target to back buffer if necessary
   if (beRenderTotexture) {
      // set render target back to back buffer
      vp.SetRenderTarget(0, BACK_BUFFER);

      FnShader shader(postShaderID);
      shader.Render(vID, matRenderTargetID);
      beRenderTotexture = FALSE;
   }

   FnText text(textID);
   text.Begin(vID);
   text.Write("F3 to reduce the blood", 10, 10, 255, 0, 0, 255);
   text.Write("F4 to add the blood", 10, 25, 255, 0, 0, 255);
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
   // The Homework #1 part 2
   FnCharacter actor;
   actor.ID(actorID);

   if (!value) {
      if (code == FY_UP || code == FY_LEFT || code == FY_RIGHT) {
         if (actor.GetCurrentAction(NULL, 0) == runID) {
            if (!FyCheckHotKeyStatus(FY_UP) && !FyCheckHotKeyStatus(FY_LEFT) && !FyCheckHotKeyStatus(FY_RIGHT)) {
               curPoseID = idleID;
               actor.SetCurrentAction(NULL, 0, curPoseID);
               actor.Play(START, 0.0f, FALSE, TRUE);
            }
         }
      }
   }
   else {
      if (code == FY_UP || code == FY_LEFT || code == FY_RIGHT) {
         if (actor.GetCurrentAction(NULL, 0) == idleID) {
            curPoseID = runID;
            actor.SetCurrentAction(NULL, 0, curPoseID);
            actor.Play(START, 0.0f, FALSE, TRUE);
         }
      }
   }
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


/*------------------
  snapshot
  C.Wang 0706, 2012
 -------------------*/
void SnapShot(BYTE code, BOOL4 value)
{
   if (code == FY_F1) {
      if (value) {
         beRenderToDepthmap = TRUE;
         beRenderTotexture = TRUE;
      }
   }
   else if (code == FY_F2) {
      if (value) {
         // save the files
         FnTexture tex;
         tex.ID(texRenderTargetID);
         tex.Save("colorMap", FILE_DDS);
         tex.ID(depthMapID);
         tex.Save("depthMap", FILE_DDS);
      }
   }
}


/*------------------
  demo
  C.Wang 1217, 2013
 -------------------*/
void Demo(BYTE code, BOOL4 value)
{
   if (code == FY_F3) {
      if (value) {
         FnBillboard bb(bloodBarID);
         float newSize[2];
         bb.GetSize(newSize);
         newSize[0] -= 2.0f;
         if (newSize[0] < 0.0f) newSize[0] = 0.0f;
         bb.SetPositionSize(NULL, newSize);
      }
   }
   else if (code == FY_F4) {
      if (value) {
         FnBillboard bb(bloodBarID);
         float newSize[2];
         bb.GetSize(newSize);
         newSize[0] += 2.0f;
         if (newSize[0] > 25.0f) newSize[0] = 25.0f;
         bb.SetPositionSize(NULL, newSize);
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
void InitMove(int x, int y)
{
   oldXM = x;
   oldYM = y;
   frame = 0;
}


/*------------------
  move the camera
  C.Wang 0329, 2005
 -------------------*/
void MoveCam(int x, int y)
{
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
void ZoomCam(int x, int y)
{
   if (x != oldXMM || y != oldYMM) {
      FnObject model;

      model.ID(cID);
      model.Translate(0.0f, 0.0f, (float)(x - oldXMM)*10.0f, LOCAL);
      oldXMM = x;
      oldYMM = y;
   }
}