/*==============================================================
  Test Sprite on Fly2

  (C)2005-2012 Chuan-Chang Wang, All Rights Reserved
  Created : 0323, 2005

  Last Updated : 0729, 2012, Kevin C. Wang
 ===============================================================*/
#include "FlyWin32.h"

int frame = 0;

int width = 1024;            // window width
int height = 768;            // window height
BOOL4 beFullScreen = FALSE;  // be full screen or not

VIEWPORTid vID;              // viewport

SCENEid sID2;                // the 2D scene
OBJECTid spID0 = FAILED_ID;  // the sprite
FnSprite sp;
FnScene scene;

TEXTid textID = FAILED_ID;

void Tester(BYTE, BOOL4);
void QuitGame(BYTE, BOOL4);
void RenderIt(int);
void PlayAnimation(int);

/*------------------
  the main program
  C.Wang 0728, 2012
 -------------------*/
void FyMain(int argc, char **argv)
{
   // create a new world
   float blockLen = 700.0f;
   FyBeginMemorySystem(1, &blockLen, 200.0f, 100.0f);
   FyStartFlyWin32("Sprite Test on Fly2", 0, 0, width, height, beFullScreen, TRUE);

   FySetTexturePath("Data\\Textures");

   // create a viewport
   vID = FyCreateViewport(0, 0, width, height);
   FnViewport vp;
   vp.Object(vID);
   vp.SetBackgroundColor(0.2f, 0.2f, 0.2f);

   // create a 2D scene for sprite rendering which will be rendered on the top of 3D
   sID2 = FyCreateScene(1);
   scene.Object(sID2);
   scene.SetSpriteWorldSize(width, height);         // 2D scene size in pixels

   spID0 = scene.CreateObject(SPRITE);
   sp.Object(spID0);
   //sp.SetSize(width, height);
   sp.SetSize(400, 400);
   //sp.SetImage("ChessBoard", 0, NULL, 0, NULL, NULL, MANAGED_MEMORY, FALSE, FALSE, 10, 10);
   sp.SetImage("ChessBoard", 0, NULL, 0, NULL, NULL, MANAGED_MEMORY, FALSE, FALSE);
   sp.SetPosition(100, 100, 0);
   //sp.SetPosition(0, 0, 0);
   //sp.SetRenderMode(WIREFRAME);

   textID = FyCreateText("Trebuchet MS", 18, FALSE, FALSE, TRUE);

   // set Hotkeys
   FyDefineHotKey(FY_F1, Tester, FALSE);      // press F1 to play animation
   FyDefineHotKey(FY_F2, Tester, FALSE);      // press F1 to play animation
   FyDefineHotKey(FY_ESCAPE, QuitGame, FALSE);       // escape for quiting the game

   // bind timers, frame rate = 30 fps
   FyBindTimer(0, 30.0f, RenderIt, FALSE);
   FyBindTimer(1, 30.0f, PlayAnimation, TRUE);

   // invoke the system
   FyInvokeFly(TRUE);
}


/*-------------------
  play the animation
  C.Wang 0308, 2004
 --------------------*/
void PlayAnimation(int skip)
{
}


/*------------------
  renderer
  C.Wang 0308, 2004
 -------------------*/
void RenderIt(int skip)
{
   // render the 2D scene for sprites on the top of the 3D
   FnViewport vp;
   vp.Object(vID);
   vp.RenderSprites(sID2, TRUE, TRUE);  // no clear the background but clear the z buffer

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

   char text[128];
   FnText shufa(textID);
   shufa.Begin(vID);
   shufa.Write(string, 20, 40, 255, 0, 0);
   sprintf(text, "Free Video Memory %d MB : %d Textures", FyQuerySystem(VIDEO_MEMORY)/1024/1024, FyQuerySystem(TEXTURE_NUMBER_IN_VIDEO_MEMORY));
   shufa.Write(text, 20, 60, 255, 0, 0);
   shufa.Write("±z¦n", 20, 80, 255, 0, 0);
   shufa.End();

   // swap buffer
   FySwapBuffers();
}


/*------------------
  quit the demo
  C.Wang 0304, 2004
 -------------------*/
void QuitGame(BYTE code, BOOL4 value)
{
   if (code == FY_ESCAPE) {
      if (value) {
         FyQuitFlyWin32();
      }
   }
}


/*-------------------
  play the animation
  C.Wang 1227, 2011
 --------------------*/
void Tester(BYTE code, BOOL4 value)
{
   if (code == FY_F1) {
      if (!value) {
         FnSprite sp(spID0);
         float pos[8], uv[8];
         pos[0] = 30.0f;
         pos[1] = 150.0f;
         pos[2] = width - 165.0f;
         pos[3] = 200.0f;
         pos[4] = width - 305.0f;
         pos[5] = height - 200.0f;
         pos[6] = 145.0f;
         pos[7] = height - 270.0f;
         uv[0] = 0.2f;
         uv[1] = 0.3f;
         uv[2] = 0.8f;
         uv[3] = 0.2f;
         uv[4] = 0.9f;
         uv[5] = 0.7f;
         uv[6] = 0.1f;
         uv[7] = 0.78f;
         sp.ChangeCornerPositionUV(pos, 0, uv);
      }
   }
   else if (code == FY_F2) {
      if (!value) {
         FnSprite sp(spID0);
         float pos[8], uv[8];
         pos[0] = 0.0f;
         pos[1] = 0.0f;
         pos[2] = width - 200.0f;
         pos[3] = 0.0f;
         pos[4] = width - 200.0f;
         pos[5] = height - 200.0f;
         pos[6] = 0.0f;
         pos[7] = height - 200.0f;
         uv[0] = 0.0f;
         uv[1] = 0.0f;
         uv[2] = 1.0f;
         uv[3] = 0.0f;
         uv[4] = 1.0f;
         uv[5] = 1.0f;
         uv[6] = 0.0f;
         uv[7] = 1.0f;
         sp.ChangeCornerPositionUV(pos, 0, uv);
      }
   }
}
