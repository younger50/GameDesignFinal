/*==============================================================
  This is a demo program for TheFly System

  Hello !

  (C)2004 Chuan-Chang Wang, All Rights Reserved
  Created : 0303, 2004, C. Wang

  Last Updated : 1010, 2004, C.Wang
 ===============================================================*/


#include "TheFlyWin32.h"
#include "FyFX.h"
#include "FyMedia.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <cstring>
#include <string>
#include <time.h>

#define PI 3.14159265


using namespace std;

int oldX, oldY, oldXM, oldYM, oldXMM, oldYMM;

WORLDid gID = FAILED_ID;
VIEWPORTid vID;
SCENEid sID;
FnScene scene;
OBJECTid nID, cID, lID;
OBJECTid LbloodID, DbloodID, RbloodID;
OBJECTid LmpID, DmpID, RmpID;
OBJECTid mpID_arr[2];
OBJECTid bloodID_arr[100];
OBJECTid terrainID;
AUDIOid audioID;
FnActor actor, actorD, actorR;
ACTORid record[100];
FnCamera camera;
ACTIONid curPoseID, idleID, runID, CombatIdleID, NormalAttack1ID, DamageHID;
ACTIONid DcurPoseID, DidleID, DrunID, DCombatIdleID, DNormalAttack1ID, DDamageLID, DDieID;
//ACTIONid RcurPoseID, RidleID, RrunID, RCombatIdleID, RNormalAttack1ID, RDamageLID, RDieID;
ACTIONid RcurPoseID[100], RidleID[100], RrunID[100], RCombatIdleID[100], RNormalAttack1ID[100], RDamageLID[100], RDieID[100],RDamageHID[100];
FnTerrain terrain;
FnAudio audioRDie;
FnAudio audioDDie;
FnAudio audioLAtt;
//for heavy hit
FnAudio audioLHat;
//for ultimate at
FnAudio audioLUat1;
//FnAudio audioLUat2;
eF3DFX *fx00 = NULL;
eF3DFX *fx01 = NULL;

eF3DFX *fxR = NULL;
eF3DFX *fxH = NULL;
eF3DFX *fxU = NULL;
eF3DFX *fxY = NULL;

eF3DFX *fx1 = NULL;
eF3DFX *fx2 = NULL;
eF3DFX *fx3 = NULL;
eF3DFX *fx4 = NULL;
eF3DFX *fx5 = NULL;
eF3DFX *fx6 = NULL;
eF3DFX *fx7 = NULL;
eF3DFX *fx8 = NULL;

MEDIAid mediaID;
FnMedia media; 
float cameraheght=320;
float target[3];
float turnr[15];
int lockAttackNumber=0;
int lockDamageNumber=0;
int donzoHP=500;
int robberHP[100];
int lyubuHP=100;
int lockDonzoDamageNumber=0;
int lockRobberDamageNumber[100];
int lockDonzoAttackNumber=0;
int lockRobberAttackNumber[100];
int counter=0;
int actor_num=0, finalnum=0;
int last_key=3;
int level[5]={0, 0, 0, 0, 0};

void QuitGame(WORLDid, BYTE, BOOL);
void GameAI(int);

void InitPivot(WORLDid, int, int);
void PivotCam(WORLDid, int, int);
void InitMove(WORLDid, int, int);
void MoveCam(WORLDid, int, int);
void InitZoom(WORLDid, int, int);
void ZoomCam(WORLDid, int, int);

float distance2D(FnActor,FnActor);
float distance2D(float*,float*);
void vectorAtoB2D(float* , FnActor ,FnActor);
bool attackJudge(FnActor,FnActor);
bool attackJudgeU(FnActor,FnActor);
void bar_reloc(void);
bool hasCollision(void);

void newRobber(float pos[3]);
void freeRobber(int index);

/* Add by qcl */
FnShuFa shufa;
FnShuFa shufaBig;
SHUFAid suid;

SCENEid sID2;
VIEWPORTid vID2;
OBJECTid spID = FAILED_ID, spID1 = FAILED_ID, spID2 = FAILED_ID;

ACTIONid LDieID,HATKID,UATKID;
int poepleDie = 0;

bool Llive = TRUE;
bool Dlive = TRUE;
int Rnum = 0;
bool lighting = FALSE;

bool relive = FALSE;

MEDIAid mmID;
FnMedia mP;

bool gameOver = FALSE;

/*------------------
  the main program
  C.Wang 0308, 2004
 -------------------*/
void main(int argc, char **argv)
{
   // create a new world
   gID = FyWin32CreateWorld("Game Programming Final Project", 0, 0, 800, 600, 32, FALSE);

   FnWorld gw;
   gw.Object(gID);
   gw.SetObjectPath("Data\\NTU4\\Scenes");
   gw.SetTexturePath("Data\\NTU4\\Scenes\\Textures");
   gw.SetShaderPath("Data\\NTU4\\Shaders");
   gw.SetScenePath("Data\\NTU4\\Scenes");

   // create a viewport
   vID = gw.CreateViewport(0, 0, 800, 600);
   FnViewport vp;
   vp.Object(vID);
   vp.SetBackgroundColor(0.3f, 0.4f, 0.5f);

  

   

   // create a 3D scene & the 3D entities
   sID = gw.CreateScene(10);

   //scence
   //FnScene scene;
   scene.Object(sID);
   scene.Load("scene2");
   

   // load a terrain file
   terrainID = scene.CreateTerrain(ROOT);
   //FnTerrain terrain;
   terrain.Object(terrainID);
   terrain.Load("terrain");
   terrain.GenerateTerrainData();


   //set character path
   gw.Object(gID);
   gw.SetObjectPath("Data\\NTU4\\Characters");
   gw.SetTexturePath("Data\\NTU4\\Characters");
   gw.SetCharacterPath("Data\\NTU4\\Characters");

   //load actor
   ACTORid nID = scene.LoadActor("Lyubu");
   ACTORid nIDD = scene.LoadActor("Donzo");
   ACTORid nIDR = scene.LoadActor("Robber02");
   //FnActor actor;
   actor.Object(nID);
   actorD.Object(nIDD);
   actorR.Object(nIDR);
   nID = scene.CreateObject(ROOT);
   cID = scene.CreateCamera(ROOT);
   lID = scene.CreateLight(ROOT);

   //Lyubu position
   float pos[3];
   pos[0] = 3624.0;
   pos[1] = -4291.0;
   pos[2] = 1000.0f;
   actor.SetPosition(pos);
   actor.PutOnTerrain(terrainID,false,0.0f);
   //Donzo position
   float posD[3];
   posD[0] = 560.0;
   posD[1] = 2882.0;
   posD[2] = 1000.0f;
   actorD.SetPosition(posD);
   actorD.PutOnTerrain(terrainID,false,0.0f);
   //Robber02 position
   //float posR[3];
   //posR[0] = 3569.0;
   //posR[1] = -3108.0;
   //posR[2] = 1000.0f;
   //actorR.SetPosition(posR);
   //actorR.PutOnTerrain(terrainID,false,0.0f);

   // create a bloodbar of Lyubu
    LbloodID = scene.CreateObject(ROOT);
    FnObject blood;
    blood.Object(LbloodID);
    float mypos[3], size[2], color[3];
    mypos[0] = 0.0f;
    mypos[1] = 0.0f;
    mypos[2] = 80.0f;
    size[0] = 20.0f;
    size[1] = 2.0f;
    color[1] = 1.0f; color[0] = color[2] = 0.0f;
    blood.Billboard(mypos, size, NULL, 0, color);
    blood.SetParent(actor.GetBaseObject());
	//mp bar of Lyubu
	LmpID = scene.CreateObject(ROOT);
	blood.Object(LmpID);
    mypos[0] = 0.0f;
    mypos[1] = 0.0f;
    mypos[2] = 76.0f;
    size[0] = 0.0f;
    size[1] = 2.0f;
    color[2] = 1.0f; color[1] = color[0] = 0.0f;
    blood.Billboard(mypos, size, NULL, 0, color);
    blood.SetParent(actor.GetBaseObject());
	// create a bloodbar of Donzo
    DbloodID = scene.CreateObject(ROOT);
    blood.Object(DbloodID);
    mypos[0] = 0.0f;
    mypos[1] = 0.0f;
    mypos[2] = 100.0f;
    size[0] = 20.0f;
    size[1] = 2.0f;
    color[1] = 1.0f; color[0] = color[2] = 0.0f;
    blood.Billboard(mypos, size, NULL, 0, color);
    blood.SetParent(actorD.GetBaseObject());
	//mp bar of Donzo
	DmpID = scene.CreateObject(ROOT);
	blood.Object(DmpID);
    mypos[0] = 0.0f;
    mypos[1] = 0.0f;
    mypos[2] = 96.0f;
    size[0] = 0.0f;
    size[1] = 2.0f;
    color[2] = 1.0f; color[1] = color[0] = 0.0f;
    blood.Billboard(mypos, size, NULL, 0, color);
    blood.SetParent(actorD.GetBaseObject());
	// create a bloodbar of Robber
    //RbloodID = scene.CreateObject(ROOT);
    //blood.Object(RbloodID);
    //mypos[0] = 0.0f;
    //mypos[1] = 0.0f;
    //mypos[2] = 80.0f;
    //size[0] = 20.0f;
    //size[1] = 2.0f;
    //color[1] = 1.0f; color[0] = color[2] = 0.0f;
    //blood.Billboard(mypos, size, NULL, 0, color);
    //blood.SetParent(actorR.GetBaseObject());
	//mp bar of Robber
	//RmpID = scene.CreateObject(ROOT);
	//blood.Object(RmpID);
    //mypos[0] = 0.0f;
    //mypos[1] = 0.0f;
    //mypos[2] = 76.0f;
    //size[0] = 0.0f;
    //size[1] = 2.0f;
    //color[2] = 1.0f; color[1] = color[0] = 0.0f;
    //blood.Billboard(mypos, size, NULL, 0, color);
    //blood.SetParent(actorR.GetBaseObject());
	//recrod actor,mp and blood bar
	record[actor_num]=nID;
	bloodID_arr[actor_num]=LbloodID;
	mpID_arr[actor_num++]=LmpID;
	record[actor_num]=nIDD;
	bloodID_arr[actor_num]=DbloodID;
	mpID_arr[actor_num++]=DmpID;
	//record[actor_num]=actorR;
	//bloodID_arr[actor_num]=RbloodID;
	//mpID_arr[actor_num++]=RmpID;

	float posR[3];
   posR[0] = 3817.0, posR[1] = -3061.0, posR[2] = 1000.0f;
   newRobber(posR);
   posR[0] = 3555.0, posR[1] = -3073.0, posR[2] = 1000.0f;
   newRobber(posR);
   posR[0] = 3271.0, posR[1] = -3236.0, posR[2] = 1000.0f;
   newRobber(posR);


   // get Idle action
   idleID = actor.GetBodyAction(NULL, "Idle");
   // get Run action
   runID = actor.GetBodyAction(NULL, "Run");
   DrunID = actorD.GetBodyAction(NULL, "Run");
   //RrunID = actorR.GetBodyAction(NULL, "Run");
   // get CombatIdle action
   //RCombatIdleID = actorR.GetBodyAction(NULL, "CombatIdle");
   DCombatIdleID = actorD.GetBodyAction(NULL, "CombatIdle");
   CombatIdleID = actor.GetBodyAction(NULL, "CombatIdle");
   // get NormalAttack1 action
   NormalAttack1ID = actor.GetBodyAction(NULL, "NormalAttack1");
   DNormalAttack1ID = actorD.GetBodyAction(NULL, "AttackL1");
   //RNormalAttack1ID = actorR.GetBodyAction(NULL, "NormalAttack1");
   //get damage action
   DamageHID = actor.GetBodyAction(NULL, "HeavyDamaged");
   //RDamageLID = actorR.GetBodyAction(NULL, "Damage1");
   DDamageLID = actorD.GetBodyAction(NULL, "DamageL");
   //get die action
   DDieID = actorD.GetBodyAction(NULL, "Die");
   //RDieID = actorR.GetBodyAction(NULL, "Dead");

   HATKID = actor.GetBodyAction(NULL,"HeavyAttack3");
   UATKID = actor.GetBodyAction(NULL,"UltimateAttack");
   LDieID = actor.GetBodyAction(NULL,"Die");
   // make idle action as the current action
   actor.MakeCurrentAction(0, NULL, idleID);
   // play the 1st frame of idle action
   actor.Play(0, START, 0.0f, FALSE, TRUE);
   
   /*
   // load the teapot model
   FnObject model;
   model.Object(nID);
   model.Load("teapot");
   */
   
   // create an audio object
   gw.SetAudioPath("Data\\NTU4\\Audio");
   audioID = gw.CreateAudio();   
   audioLAtt.Object(audioID);
   BOOL beA = audioLAtt.Load("01_pose16");   // au_bullet.hit1.wav
   audioDDie.Object(gw.CreateAudio());
   audioDDie.Load("02_pose25");
   audioRDie.Object(gw.CreateAudio());
   audioRDie.Load("03_pose25");
   //for heavy hit
   audioLHat.Object(gw.CreateAudio());
   audioLHat.Load("bomb");
   //for ultimate at
   audioLUat1.Object(gw.CreateAudio());
   audioLUat1.Load("storm-fire");
   //audioLUat2.Object(gw.CreateAudio());
   //audioLUat2.Load("Fireball2");

   // play new.mp3
   FyBeginMedia("Data\\NTU4\\Media", 10);
   HWND hwnd = FyGetWindowHandle(gw.Object());
   mmID = FyCreateMediaPlayer(0, "36", 0, 0, 0, 0);
   mP.Object(mmID);
   mP.Play(LOOP);
   
  // FyBeginMedia("Data\\Media", 10);
  // mediaID = FyCreateMediaPlayer(0, "city.mp3", 0, 0, 0, 0);
  // media.Object(mediaID);
  // media.Play(ONCE);  

   // load a demo FX
   gw.SetTexturePath("Data\\NTU4\\FXs\\Textures");
   gw.SetObjectPath("Data\\NTU4\\FXs\\Models");
   fx00 = new eF3DFX(sID);
   fx00->SetWorkPath("Data\\NTU4\\FXs");
   BOOL beOK = fx00->Load("Attack01");

   fxR = new eF3DFX(sID);
   fxR->SetWorkPath("Data\\NTU4\\FXs");
   fxR->Load("NoPigeon1");
   fxH = new eF3DFX(sID);
   fxH->SetWorkPath("Data\\NTU4\\FXs");
   fxH->Load("Eat");
   fxU = new eF3DFX(sID);
   fxU->SetWorkPath("Data\\NTU4\\FXs");
   fxU->Load("Pose13");
  
   fxY = new eF3DFX(sID);
   fxY->SetWorkPath("Data\\NTU4\\FXs");
   fxY->Load("03_pose13");
  

   // translate the camera
   float dir[3];
   float dirup[3];
   dir[0]=-1;dir[1]=0;dir[2]=0;
   dirup[0]=0;dirup[1]=0;dirup[2]=300;
   //FnCamera camera;
   camera.Object(cID);
   dir[0]=3698.0f, dir[1]=-3644.0f, dir[2]=120;
   camera.SetPosition(dir);
   actor.GetPosition(dirup);
   dirup[0]-=dir[0], dirup[1]-=dir[1], dirup[2]=0;
   dir[0]=0.0f, dir[1]=0.0f, dir[2]=1.0f;
   camera.SetDirection(dirup, dir);

	/******** Start by qcl ********/

	//Setting 中文
	suid = gw.CreateShuFa("新細明體",14,FALSE,FALSE);
	shufa.Object(suid);
	suid = gw.CreateShuFa("標楷體",28,FALSE,FALSE);
	shufaBig.Object(suid);

	sID2 = gw.CreateScene(1);
	scene.Object(sID2);
	scene.SetSpriteWorldSize(800,600);

	spID = scene.CreateSprite();
	FnSprite sp;
	sp.Object(spID);
	sp.SetRectArea(NULL,120,120,NULL,"Data\\Image\\lyubu.jpg",0,FALSE,0,0,0);
	sp.SetRectPosition(10,470,0);

	scene.Object(sID);
	/******** End by qcl ********/

   // translate the light
   FnLight light;
   light.Object(lID);
   light.SetName("MainLight");
   light.Translate(-50.0f, -50.0f, 50.0f, GLOBAL);

   // set Hotkeys
   FyDefineHotKey(FY_ESCAPE, QuitGame, FALSE);

   // define some mouse functions
   FyBindMouseFunction(LEFT_MOUSE, InitPivot, PivotCam, NULL, NULL);
   FyBindMouseFunction(MIDDLE_MOUSE, InitZoom, ZoomCam, NULL, NULL);
   FyBindMouseFunction(RIGHT_MOUSE, InitMove, MoveCam, NULL, NULL);

   /* bind a timer, frame rate = 30 fps */
   FyBindTimer(0, 30.0f, GameAI, TRUE);
	srand ( time(NULL) );
   // invoke the system
   FyInvokeTheFly(TRUE);
}

bool fxEnd=true;
bool fxEnd2=true;

//------------------------------------------------------------------------------------
// timer callback function which will be invoked by TheFly3D System every 1/30 second
// C.Wang 0308, 2004
//------------------------------------------------------------------------------------
int cameramovinglock=0;
void GameAI(int skip)
{
	if(!mP.IsPlaying()){
		mP.Object(mmID);
		mP.Play(LOOP);
	}

	if(Rnum==0&&!Dlive)
		gameOver = TRUE;
	
	float actorcameradismax=660;
	float actorcameradismin=540;
	float movementspeed=20;
	cameraheght=120;
	float detectChaseDistance=650;
	float size[2], size2[2], size3[2];
	
	bool atk_n = FyCheckHotKeyStatus(FY_F);
	bool atk_h = FyCheckHotKeyStatus(FY_E);
	bool atk_u = FyCheckHotKeyStatus(FY_G);

	//calculate time
	counter++;
	//every 1 sec add some mp
	if(counter>=30){
		counter=0;
		FnBillBoard mp;
		//Lyubu
		mp.Object(LmpID, 0);
		mp.GetSize(size);
		if(lyubuHP>0){
			size[0]+=2.0f;
			if(size[0]>=20.0f)size[0]=20.0f;
		}else{
			size[0] = 0.0f;
		}
		mp.SetSize(size);
		//Donzo
		mp.Object(DmpID, 0);
		mp.GetSize(size2);
		size2[0]+=2.0f;
		if(size2[0]>=20.0f)size2[0]=20.0f;
		if(donzoHP<=0)size2[0]=0.0f;
		mp.SetSize(size2);
		if(donzoHP<=0)DmpID=NULL;
		//Robber
		//mp.Object(RmpID, 0);
		//mp.GetSize(size3);
		//size3[0]+=2.0f;
		//if(size3[0]>=20.0f)size3[0]=20.0f;
		//if(robberHP<=0)size3[0]=0.0f;
		//mp.SetSize(size3);
		//if(robberHP<=0)RmpID=NULL;
	}
    FnBillBoard Lblood;
    Lblood.Object(LbloodID, 0);
	Lblood.GetSize(size);
    FnBillBoard Dblood;
    Dblood.Object(DbloodID, 0);
    Dblood.GetSize(size2);
	FnBillBoard Rblood;
    //Rblood.Object(RbloodID, 0);
    //Rblood.GetSize(size3);

	FnBillBoard lmp;
	float Lsize[2];
	lmp.Object(LmpID, 0);
	lmp.GetSize(Lsize);

	//復活
	if(FyCheckHotKeyStatus(FY_R)&&!Llive){
		actor.MakeCurrentAction(0, NULL, idleID);
		lyubuHP = 100;
		Llive = TRUE;
		size[0] = 20;
		Lblood.SetSize(size);
		float lcolor[3];
		lcolor[1] = 1.0f;
		Lblood.SetColor(lcolor);

		relive = TRUE;

		if(fx01==NULL){
			fx01 = fxR->Clone();
		}else{
			delete fx01;
			fx01 = fxR->Clone();
		}

			eF3DBaseFX *fx;
			float xyzPos[3];
			actor.GetPosition(xyzPos);
			xyzPos[2]+=50;
			int numFX = fx01->NumberFXs();
			for ( int i = 0; i < numFX; i++) {
			   fx = fx01->GetFX(i);
			   fx->InitPosition(xyzPos);
			}

			fxEnd=false;
	}

	//stage open
	float src[3], target[3];
	actor.GetPosition(src);
	//level1
	target[0]=3271.0f, target[1]=-2708.0f, target[2]=0.0f;
	if(level[0]==0 && distance2D(src, target)<=300.0f){
		float rob_pos[3];
		rob_pos[0]=2524.0f, rob_pos[1]=-2305.0f, rob_pos[2]=10.0f;
		newRobber(rob_pos);
		rob_pos[0]=1857.0f, rob_pos[1]=-2844.0f, rob_pos[2]=10.0f;
		newRobber(rob_pos);
		rob_pos[0]=1574.0f, rob_pos[1]=-2722.0f, rob_pos[2]=10.0f;
		newRobber(rob_pos);
		level[0]=1;
	}
	//level2
	target[0]=-676.0f, target[1]=-2541.0f, target[2]=0.0f;
	if(level[1]==0 && distance2D(src, target)<=300.0f){
		float rob_pos[3];
		rob_pos[0]=-1698.0f, rob_pos[1]=-3243.0f, rob_pos[2]=0.0f;
		newRobber(rob_pos);
		rob_pos[0]=-2061.0f, rob_pos[1]=-2987.0f, rob_pos[2]=0.0f;
		newRobber(rob_pos);
		rob_pos[0]=-909.0f, rob_pos[1]=-2653.0f, rob_pos[2]=0.0f;
		newRobber(rob_pos);
		level[1]=1;
	}
	//level3
	target[0]=-2252.0f, target[1]=-2821.0f, target[2]=0.0f;
	if(level[2]==0 && distance2D(src, target)<=300.0f){
		float rob_pos[3];
		rob_pos[0]=-2743.0f, rob_pos[1]=-443.0f, rob_pos[2]=0.0f;
		newRobber(rob_pos);
		rob_pos[0]=-2706.0f, rob_pos[1]=-494.0f, rob_pos[2]=0.0f;
		newRobber(rob_pos);
		rob_pos[0]=-2800.0f, rob_pos[1]=-410.0f, rob_pos[2]=0.0f;
		newRobber(rob_pos);
		rob_pos[0]=-2650.0f, rob_pos[1]=-370.0f, rob_pos[2]=0.0f;
		newRobber(rob_pos);
		rob_pos[0]=-1732.0f, rob_pos[1]=-120.0f, rob_pos[2]=0.0f;
		newRobber(rob_pos);
		level[2]=1;
	}
	//level4
	target[0]=-1760.0f, target[1]=1500.0f, target[2]=0.0f;
	if(level[3]==0 && distance2D(src, target)<=300.0f){
		float rob_pos[3];
		rob_pos[0]=-1851.0f, rob_pos[1]=2152.0f, rob_pos[2]=0.0f;
		newRobber(rob_pos);
		rob_pos[0]=-1820.0f, rob_pos[1]=2102.0f, rob_pos[2]=0.0f;
		newRobber(rob_pos);

		rob_pos[0]=-1697.0f, rob_pos[1]=1507.0f, rob_pos[2]=0.0f;
		newRobber(rob_pos);
		rob_pos[0]=-1864.0f, rob_pos[1]=1117.0f, rob_pos[2]=0.0f;
		newRobber(rob_pos);
		rob_pos[0]=-2082.0f, rob_pos[1]=2745.0f, rob_pos[2]=0.0f;
		newRobber(rob_pos);
		rob_pos[0]=-1067.0f, rob_pos[1]=2189.0f, rob_pos[2]=0.0f;
		newRobber(rob_pos);
		rob_pos[0]=-1439.0f, rob_pos[1]=1711.0f, rob_pos[2]=0.0f;
		newRobber(rob_pos);

		level[3]=1;
	}
	//level5:when donzo comes

	//move camera
	 if(FyCheckHotKeyStatus(FY_Q) && cameramovinglock==0){
		 cameramovinglock=5;
		 float cameraDir[3];
		 float dummy[3];
		 camera.GetDirection(cameraDir,dummy);
		 float actorDir[3];
		 actor.GetDirection(actorDir,dummy);
		 float newCameraDir[3];
		 float tempcameraDir[3];
		 float tempactorDir[3];
		 tempcameraDir[0]=cameraDir[0]/sqrt(cameraDir[0]*cameraDir[0]+cameraDir[1]*cameraDir[1]);
		 tempcameraDir[1]=cameraDir[1]/sqrt(cameraDir[0]*cameraDir[0]+cameraDir[1]*cameraDir[1]);
		 tempactorDir[0]=actorDir[0]/sqrt(actorDir[0]*actorDir[0]+actorDir[1]*actorDir[1]);
		 tempactorDir[1]=actorDir[1]/sqrt(actorDir[0]*actorDir[0]+actorDir[1]*actorDir[1]);
		 newCameraDir[0]=tempcameraDir[0]+tempactorDir[0];
		 newCameraDir[1]=tempcameraDir[1]+tempactorDir[1];
		 float cameraPos[3];
		 float actorPos[3];
		 camera.GetPosition(cameraPos);
		 actor.GetPosition(actorPos);
		 float disAtoC;
		 disAtoC=distance2D(cameraPos,actorPos);
		 float scale;
		 if(sqrt(newCameraDir[0]*newCameraDir[0]+newCameraDir[1]*newCameraDir[1])==0){
			 newCameraDir[0]=-actorDir[1];
			 newCameraDir[1]=actorDir[0];
		 }
		 scale=disAtoC/sqrt(newCameraDir[0]*newCameraDir[0]+newCameraDir[1]*newCameraDir[1]);
		 cameraPos[0]=actorPos[0]-scale*newCameraDir[0];
		 cameraPos[1]=actorPos[1]-scale*newCameraDir[1];
		 
		 newCameraDir[2]=0;
		 dummy[0]=0;
		 dummy[1]=0;
		 dummy[2]=1;
		 camera.SetDirection(newCameraDir,dummy);
		 //move camera height
		 float buffheight=cameraPos[2]-actorPos[2]-cameraheght;
		 cameraPos[2]=actorPos[2]+cameraheght+buffheight;
   		 camera.SetPosition(cameraPos);
 
		 //rotate camera to look at head
		 camera.Rotate(X_AXIS, -atan((cameraPos[2]-actorPos[2]-100)/distance2D(actorPos,cameraPos))* 180 / PI, LOCAL);
		 cameramovinglock--;

	 }
	 else if(cameramovinglock != 0){
		 float cameraDir[3];
		 float dummy[3];
		 camera.GetDirection(cameraDir,dummy);
		 float actorDir[3];
		 actor.GetDirection(actorDir,dummy);
		 float newCameraDir[3];
		 float tempcameraDir[3];
		 float tempactorDir[3];
		 tempcameraDir[0]=cameraDir[0]/sqrt(cameraDir[0]*cameraDir[0]+cameraDir[1]*cameraDir[1]);
		 tempcameraDir[1]=cameraDir[1]/sqrt(cameraDir[0]*cameraDir[0]+cameraDir[1]*cameraDir[1]);
		 tempactorDir[0]=actorDir[0]/sqrt(actorDir[0]*actorDir[0]+actorDir[1]*actorDir[1]);
		 tempactorDir[1]=actorDir[1]/sqrt(actorDir[0]*actorDir[0]+actorDir[1]*actorDir[1]);
		 newCameraDir[0]=tempcameraDir[0]+tempactorDir[0];
		 newCameraDir[1]=tempcameraDir[1]+tempactorDir[1];
		 if(cameramovinglock==1){
			 newCameraDir[0]=actorDir[0];
			 newCameraDir[1]=actorDir[1];
		 }
		 float cameraPos[3];
		 float actorPos[3];
		 camera.GetPosition(cameraPos);
		 actor.GetPosition(actorPos);
		 float disAtoC;
		 disAtoC=distance2D(cameraPos,actorPos);
		 float scale;
		 if(sqrt(newCameraDir[0]*newCameraDir[0]+newCameraDir[1]*newCameraDir[1])==0){
			 newCameraDir[0]=-actorDir[1];
			 newCameraDir[1]=actorDir[0];
		 }
		 scale=disAtoC/sqrt(newCameraDir[0]*newCameraDir[0]+newCameraDir[1]*newCameraDir[1]);
		 cameraPos[0]=actorPos[0]-scale*newCameraDir[0];
		 cameraPos[1]=actorPos[1]-scale*newCameraDir[1];
		 
		 newCameraDir[2]=0;
		 dummy[0]=0;
		 dummy[1]=0;
		 dummy[2]=1;
		 camera.SetDirection(newCameraDir,dummy);
		 //move camera height
		 float buffheight=cameraPos[2]-actorPos[2]-cameraheght;
		 cameraPos[2]=actorPos[2]+cameraheght+buffheight;
   		 camera.SetPosition(cameraPos);
 
		 //rotate camera to look at head
		 camera.Rotate(X_AXIS, -atan((cameraPos[2]-actorPos[2]-100)/distance2D(actorPos,cameraPos))* 180 / PI, LOCAL);
		 cameramovinglock--;
	 }

	if(atk_u&&Llive&&Lsize[0]==20){
		lockDamageNumber = 0;
	 }

   if(lockAttackNumber>0)
   {
	   if(Llive){
		//curPoseID = NormalAttack1ID;
		//ACTIONid curPoseIDt = actor.GetCurrentAction(NULL, 0);
		//if(curPoseIDt!=NormalAttack1ID){
		//   actor.MakeCurrentAction(0, NULL, NormalAttack1ID);
		//   
		//}
	   
	    lockAttackNumber--;

		if(lockAttackNumber==0){
			if(fx1!=NULL){fxEnd2=true;delete fx1;fx1=NULL;}
			if(fx2!=NULL){fxEnd2=true;delete fx2;fx2=NULL;}
			if(fx3!=NULL){fxEnd2=true;delete fx3;fx3=NULL;}
			if(fx4!=NULL){fxEnd2=true;delete fx4;fx4=NULL;}
			if(fx5!=NULL){fxEnd2=true;delete fx5;fx5=NULL;}
			if(fx6!=NULL){fxEnd2=true;delete fx6;fx6=NULL;}
			if(fx7!=NULL){fxEnd2=true;delete fx7;fx7=NULL;}
			if(fx8!=NULL){fxEnd2=true;delete fx8;fx8=NULL;}
		}
	   }
   }
   else if(lockDamageNumber>0){
	   
   }
   else if(atk_n||atk_h||atk_u)
   {
	   if(atk_u&&Llive&&Lsize[0]==20){
		   Lsize[0] = 0;
		   lmp.SetSize(Lsize);
		   lockAttackNumber=60;
		   curPoseID = UATKID;
		   if(Llive){
			ACTIONid curPoseIDt = actor.GetCurrentAction(NULL, 0);
			if(curPoseIDt!=UATKID){
				   actor.MakeCurrentAction(0, NULL, UATKID);		  
			}
		   
			audioLUat1.Play(ONCE);
			//audioLUat2.Play(ONCE);
			if(fx01==NULL){
				fx01 = fxU->Clone();
			}else{
				delete fx01;
				fx01 = fxU->Clone();
			}
			eF3DBaseFX *fx;
			float xyzPos[3];			
			float xyz[3];
			actor.GetPosition(xyzPos);
			xyzPos[2]+=50;
			int numFX = fx01->NumberFXs();
			for ( int i = 0; i < numFX; i++) {
			   fx = fx01->GetFX(i);
			   fx->InitPosition(xyzPos);
			}
			if(fx1 == NULL){ fx1 = fxY->Clone(); }else{ delete fx1; fx1 = fxY->Clone(); }
			if(fx2 == NULL){ fx2 = fxY->Clone(); }else{ delete fx2; fx2 = fxY->Clone(); }
			if(fx3 == NULL){ fx3 = fxY->Clone(); }else{ delete fx3; fx3 = fxY->Clone(); }
			if(fx4 == NULL){ fx4 = fxY->Clone(); }else{ delete fx4; fx4 = fxY->Clone(); }
			if(fx5 == NULL){ fx5 = fxY->Clone(); }else{ delete fx5; fx5 = fxY->Clone(); }
			if(fx6 == NULL){ fx6 = fxY->Clone(); }else{ delete fx6; fx6 = fxY->Clone(); }
			if(fx7 == NULL){ fx7 = fxY->Clone(); }else{ delete fx7; fx7 = fxY->Clone(); }
			if(fx8 == NULL){ fx8 = fxY->Clone(); }else{ delete fx8; fx8 = fxY->Clone(); }
			numFX = fx1->NumberFXs();
			xyz[2]= xyzPos[2];
			for (int i =0;i<numFX;i++){
			
				fx = fx1->GetFX(i);
				xyz[0] = xyzPos[0] + 0;
				xyz[1] = xyzPos[1] + 400;
				fx->InitPosition(xyz);

				fx = fx2->GetFX(i);
				xyz[0] = xyzPos[0] + 0;
				xyz[1] = xyzPos[1] - 200;
				fx->InitPosition(xyz);

				fx = fx3->GetFX(i);
				xyz[0] = xyzPos[0] + 200;
				xyz[1] = xyzPos[1] + 0;
				fx->InitPosition(xyz);

				fx = fx4->GetFX(i);
				xyz[0] = xyzPos[0] - 200;
				xyz[1] = xyzPos[1] + 0;
				fx->InitPosition(xyz);

				fx = fx5->GetFX(i);
				xyz[0] = xyzPos[0] + 150;
				xyz[1] = xyzPos[1] + 150;
				fx->InitPosition(xyz);

				fx = fx6->GetFX(i);
				xyz[0] = xyzPos[0] + 150;
				xyz[1] = xyzPos[1] - 150;
				fx->InitPosition(xyz);

				fx = fx7->GetFX(i);
				xyz[0] = xyzPos[0] - 150;
				xyz[1] = xyzPos[1] + 150;
				fx->InitPosition(xyz);

				fx = fx8->GetFX(i);
				xyz[0] = xyzPos[0] - 150;
				xyz[1] = xyzPos[1] - 150;
				fx->InitPosition(xyz);
			
			}
			fxEnd2=false;
			fxEnd=false;
		   }
		   //處理if打到人(Donzor,R...)
		   if(attackJudgeU(actor,actorD))
		   {	   
			   if(actorD.GetCurrentAction(NULL, 0)!=DDamageLID && lockDonzoDamageNumber==0 && donzoHP>0)actorD.MakeCurrentAction(0, NULL, DDamageLID);		   
			   lockDonzoDamageNumber=60;
			   donzoHP=donzoHP-60;
			   size2[0]-=2.4f;
			   if(size2[0]<=0.0f)
				   size2[0]=0.0f;
			   Dblood.SetSize(size2);
			   //color of bloodbar
			   float coltmp[3];
			   if(size2[0]/20.0f<=0.6f && size2[0]/20.0f>0.2f){
				   coltmp[0]=1.0f, coltmp[1]=1.0f, coltmp[2]=0.0f;
				   Dblood.SetColor(coltmp);
			   }
			   else if(size2[0]/20.0f<=0.2f){
				   coltmp[0]=1.0f, coltmp[1]=0.0f, coltmp[2]=0.0f;
				   Dblood.SetColor(coltmp);
			   }
			   //float t[3];
			   //Dblood.GetPosition(t);
			   //t[1]=(20.0f-size2[0])/(-2.0f);
			   //Dblood.SetPosition(t);
		   }


		   for(int a=2;a<actor_num;a++){
		   actorR.Object(record[a]);
		   if(attackJudgeU(actor,actorR))
		   {
			   if(actorR.GetCurrentAction(NULL, 0)!=RDamageLID[a] && lockRobberDamageNumber[a]==0 && robberHP[a]>0)actorR.MakeCurrentAction(0, NULL, RDamageLID[a]);		   
			   lockRobberDamageNumber[a]=60;
			   robberHP[a]=robberHP[a]-60;
			   FnBillBoard bl;
			   bl.Object(bloodID_arr[a], 0);
			   bl.GetSize(size3);
			   size3[0]-=24.0f;
			   if(size3[0]<=0.0f)
				   size3[0]=0.0f;
			   bl.SetSize(size3);
			   //color of bloodbar
			   float coltmp[3];
			   if(size3[0]/20.0f<=0.6f && size3[0]/20.0f>0.2f){
				   coltmp[0]=1.0f, coltmp[1]=1.0f, coltmp[2]=0.0f;
				   bl.SetColor(coltmp);
			   }
			   else if(size3[0]/20.0f<=0.2f){
				   coltmp[0]=1.0f, coltmp[1]=0.0f, coltmp[2]=0.0f;
				   bl.SetColor(coltmp);
			   }
		   }
	   }

	   }else if(atk_h&&Lsize[0]>=10){
		   Lsize[0]-=10;
		   lmp.SetSize(Lsize);
		   
		   lockAttackNumber=30;
		   curPoseID = HATKID;
		   if(Llive){
			ACTIONid curPoseIDt = actor.GetCurrentAction(NULL, 0);
			if(curPoseIDt!=HATKID){
				   actor.MakeCurrentAction(0, NULL, HATKID);		  
			}
			audioLAtt.Play(ONCE);
			audioLHat.Play(ONCE);

			if(fx01==NULL){
				fx01 = fxH->Clone();
			}else{
				delete fx01;
				fx01 = fxH->Clone();
			}
			eF3DBaseFX *fx;
			float xyzPos[3];
			actor.GetPosition(xyzPos);
			xyzPos[2]+=50;
			int numFX = fx01->NumberFXs();
			for ( int i = 0; i < numFX; i++) {
			   fx = fx01->GetFX(i);
			   fx->InitPosition(xyzPos);
			}


			fxEnd=false;
		   }
		   //處理if打到人(Donzor,R...)
		   if(attackJudge(actor,actorD))
		   {	   
			   if(actorD.GetCurrentAction(NULL, 0)!=DDamageLID && lockDonzoDamageNumber==0 && donzoHP>0)actorD.MakeCurrentAction(0, NULL, DDamageLID);		   
			   lockDonzoDamageNumber=30;
			   donzoHP=donzoHP-40;
			   size2[0]-=1.6f;
			   if(size2[0]<=0.0f)
				   size2[0]=0.0f;
			   Dblood.SetSize(size2);
			   //color of bloodbar
			   float coltmp[3];
			   if(size2[0]/20.0f<=0.6f && size2[0]/20.0f>0.2f){
				   coltmp[0]=1.0f, coltmp[1]=1.0f, coltmp[2]=0.0f;
				   Dblood.SetColor(coltmp);
			   }
			   else if(size2[0]/20.0f<=0.2f){
				   coltmp[0]=1.0f, coltmp[1]=0.0f, coltmp[2]=0.0f;
				   Dblood.SetColor(coltmp);
			   }
			   //float t[3];
			   //Dblood.GetPosition(t);
			   //t[1]=(20.0f-size2[0])/(-2.0f);
			   //Dblood.SetPosition(t);
		   }
		   for(int a=2;a<actor_num;a++){
		   actorR.Object(record[a]);
		   if(attackJudge(actor,actorR))
		   {
			   if(actorR.GetCurrentAction(NULL, 0)!=RDamageLID[a] && lockRobberDamageNumber[a]==0 && robberHP[a]>0)actorR.MakeCurrentAction(0, NULL, RDamageLID[a]);		   
			   lockRobberDamageNumber[a]=30;
			   robberHP[a]=robberHP[a]-40;
			   FnBillBoard bl;
			   bl.Object(bloodID_arr[a], 0);
			   bl.GetSize(size3);
			   size3[0]-=16.0f;
			   if(size3[0]<=0.0f)
				   size3[0]=0.0f;
			   bl.SetSize(size3);
			   //color of bloodbar
			   float coltmp[3];
			   if(size3[0]/20.0f<=0.6f && size3[0]/20.0f>0.2f){
				   coltmp[0]=1.0f, coltmp[1]=1.0f, coltmp[2]=0.0f;
				   bl.SetColor(coltmp);
			   }
			   else if(size3[0]/20.0f<=0.2f){
				   coltmp[0]=1.0f, coltmp[1]=0.0f, coltmp[2]=0.0f;
				   bl.SetColor(coltmp);
			   }
		   }
	   }
	   }else if(atk_n){
			lockAttackNumber=20;
		   curPoseID = NormalAttack1ID;
		   if(Llive){
			ACTIONid curPoseIDt = actor.GetCurrentAction(NULL, 0);
			if(curPoseIDt!=NormalAttack1ID){
				   actor.MakeCurrentAction(0, NULL, NormalAttack1ID);		  
			}
		   
			audioLAtt.Play(ONCE);
			if(fx01==NULL){
				fx01 = fx00->Clone();
			}else{
				delete fx01;
				fx01 = fx00->Clone();
			}

			eF3DBaseFX *fx;
			float xyzPos[3];
			actor.GetPosition(xyzPos);
			xyzPos[2]+=50;
			int numFX = fx01->NumberFXs();
			for ( int i = 0; i < numFX; i++) {
			   fx = fx01->GetFX(i);
			   fx->InitPosition(xyzPos);
			}

			fxEnd=false;
		   }
		   //處理if打到人(Donzor,R...)
		   if(attackJudge(actor,actorD))
		   {	   
			   if(actorD.GetCurrentAction(NULL, 0)!=DDamageLID && lockDonzoDamageNumber==0 && donzoHP>0)actorD.MakeCurrentAction(0, NULL, DDamageLID);		   
			   lockDonzoDamageNumber=20;
			   donzoHP=donzoHP-20;
			   size2[0]-=0.8f;
			   if(size2[0]<=0.0f)
				   size2[0]=0.0f;
			   Dblood.SetSize(size2);
			   //color of bloodbar
			   float coltmp[3];
			   if(size2[0]/20.0f<=0.6f && size2[0]/20.0f>0.2f){
				   coltmp[0]=1.0f, coltmp[1]=1.0f, coltmp[2]=0.0f;
				   Dblood.SetColor(coltmp);
			   }
			   else if(size2[0]/20.0f<=0.2f){
				   coltmp[0]=1.0f, coltmp[1]=0.0f, coltmp[2]=0.0f;
				   Dblood.SetColor(coltmp);
			   }
			   //float t[3];
			   //Dblood.GetPosition(t);
			   //t[1]=(20.0f-size2[0])/(-2.0f);
			   //Dblood.SetPosition(t);
		   }
		   for(int a=2;a<actor_num;a++){
		   actorR.Object(record[a]);
		   if(attackJudge(actor,actorR))
		   {
			   if(actorR.GetCurrentAction(NULL, 0)!=RDamageLID[a] && lockRobberDamageNumber[a]==0 && robberHP[a]>0)actorR.MakeCurrentAction(0, NULL, RDamageLID[a]);		   
			   lockRobberDamageNumber[a]=20;
			   robberHP[a]=robberHP[a]-20;
			   FnBillBoard bl;
			   bl.Object(bloodID_arr[a], 0);
			   bl.GetSize(size3);
			   size3[0]-=8.0f;
			   if(size3[0]<=0.0f)
				   size3[0]=0.0f;
			   bl.SetSize(size3);
			   //color of bloodbar
			   float coltmp[3];
			   if(size3[0]/20.0f<=0.6f && size3[0]/20.0f>0.2f){
				   coltmp[0]=1.0f, coltmp[1]=1.0f, coltmp[2]=0.0f;
				   bl.SetColor(coltmp);
			   }
			   else if(size3[0]/20.0f<=0.2f){
				   coltmp[0]=1.0f, coltmp[1]=0.0f, coltmp[2]=0.0f;
				   bl.SetColor(coltmp);
			   }
		   }
	   }   
	   }

	   
   }
   else if (FyCheckHotKeyStatus(FY_UP)) { /*FnActor actor;
									 actor.Object(nID);*/
	   //initialize camera's direction to flat
	   
	   if(fx01!=NULL&&relive){
			delete fx01;
			fx01 = NULL;
			relive = FALSE;
	  }
	   
	   float fdirC[3];
	   float udirC[3];
	   float positionA[3];
	   float positionC[3];
	   udirC[0]=0;
	   udirC[1]=0;
	   udirC[2]=1;
	   actor.GetPosition(positionA);
	   camera.GetPosition(positionC);
	   fdirC[0]=positionA[0]-positionC[0];
	   fdirC[1]=positionA[1]-positionC[1];
	   fdirC[2]=0;
	   camera.SetDirection(fdirC,udirC);

	   //turn the actor
	   actor.SetDirection(fdirC,udirC);

	   //move actor
	   curPoseID = runID;
	   ACTIONid curPoseIDt = actor.GetCurrentAction(NULL, 0);
	   if(curPoseIDt!=runID){
		   actor.MakeCurrentAction(0, NULL, curPoseID);
		   
	   }
	   
	   int actormovereturn=actor.MoveForward(movementspeed, TRUE, FALSE,0.0f,TRUE);

	   //move camera?2D
	   float posA[3];
	   float posC[3];
	   actor.GetPosition(posA);
	   camera.GetPosition(posC);
	   if(distance2D(posA,posC)>actorcameradismax){
		   camera.MoveForward(movementspeed, TRUE, FALSE,0.0f,TRUE);
	   }
	   //move camera height
	   float tempposC[3];
	   actor.GetPosition(posA);
	   camera.GetPosition(posC);
	   tempposC[0]=posC[0];
	   tempposC[1]=posC[1];
	   float buffheight=posC[2]-posA[2]-cameraheght;
	   if(actormovereturn==WALK&&buffheight>0)buffheight-=movementspeed;
	   tempposC[2]=posA[2]+cameraheght+buffheight;
	   camera.SetPosition(tempposC);

	   //rotate camera to look at head
	   camera.GetPosition(posC);
	   camera.Rotate(X_AXIS, -atan((posC[2]-posA[2]-100)/distance2D(posA,posC))* 180 / PI, LOCAL);

	   last_key=1;
   }
   else if(FyCheckHotKeyStatus(FY_DOWN)) { /*FnActor actor;
									 actor.Object(nID);*/
	   //initialize camera's direction to flat
	if(fx01!=NULL&&relive){
			delete fx01;
			fx01 = NULL;
			relive = FALSE;
	  }
	   float fdirC[3];
	   float udirC[3];
	   float positionA[3];
	   float positionC[3];
	   udirC[0]=0;
	   udirC[1]=0;
	   udirC[2]=1;
	   actor.GetPosition(positionA);
	   camera.GetPosition(positionC);
	   fdirC[0]=positionA[0]-positionC[0];
	   fdirC[1]=positionA[1]-positionC[1];
	   fdirC[2]=0;
	   camera.SetDirection(fdirC,udirC);

	   //turn the actor
	   fdirC[0]=-fdirC[0];
	   fdirC[1]=-fdirC[1];
	   actor.SetDirection(fdirC,udirC);

	   //move actor
	   curPoseID = runID;
	   ACTIONid curPoseIDt = actor.GetCurrentAction(NULL, 0);
	   if(curPoseIDt!=runID){
		   actor.MakeCurrentAction(0, NULL, curPoseID);
		   
	   }
	   
	   int actormovereturn=actor.MoveForward(movementspeed, TRUE, FALSE,0.0f,TRUE);

	   //move camera?2D
	   float buffheight=0;
	   float posA[3];
	   float posC[3];
	   actor.GetPosition(posA);
	   camera.GetPosition(posC);
	   buffheight=posC[2]-posA[2]-cameraheght;
	   if(distance2D(posA,posC)<actorcameradismin){
		   camera.SetDirection(fdirC,udirC);
		   float tpposC[3];
		   tpposC[0]=posC[0];
		   tpposC[1]=posC[1];
		   tpposC[2]=30;
		   float hitpos[3];
		   terrain.HitTest(tpposC, fdirC, hitpos, TRUE);
		   if(distance2D(hitpos,tpposC)<20&&actormovereturn==WALK){
			   float tposC[3];			   
			   camera.GetPosition(tposC);			   
			   buffheight+=movementspeed;			   
		   }
		   else if(actormovereturn==WALK)
		   {
		       camera.MoveForward(movementspeed, TRUE, FALSE,0.0f,TRUE);
		   }
		   fdirC[0]=-fdirC[0];
	       fdirC[1]=-fdirC[1];
		   camera.SetDirection(fdirC,udirC);
	   }
	   //move camera height
	   float tempposC[3];
	   actor.GetPosition(posA);
	   camera.GetPosition(posC);
	   tempposC[0]=posC[0];
	   tempposC[1]=posC[1];
	   tempposC[2]=posA[2]+cameraheght+buffheight;
	   camera.SetPosition(tempposC);

	   //rotate camera to look at head
	   camera.GetPosition(posC);
	   camera.Rotate(X_AXIS, -atan((posC[2]-posA[2]-100)/distance2D(posA,posC))* 180 / PI, LOCAL);
	   last_key=2;
   }
   else if(FyCheckHotKeyStatus(FY_RIGHT)) { /*FnActor actor;
									 actor.Object(nID);*/
	   //initialize camera's direction to flat
	if(fx01!=NULL&&relive){
			delete fx01;
			fx01 = NULL;
			relive = FALSE;
	  }
	   float fdirC[3];
	   float udirC[3];
	   float positionA[3];
	   float positionC[3];
	   udirC[0]=0;
	   udirC[1]=0;
	   udirC[2]=1;
	   actor.GetPosition(positionA);
	   camera.GetPosition(positionC);
	   fdirC[0]=positionA[0]-positionC[0];
	   fdirC[1]=positionA[1]-positionC[1];
	   fdirC[2]=0;
	   camera.SetDirection(fdirC,udirC);

	   //get distance between camera and actor
	   float radius = distance2D(positionA,positionC);

	   //turn the actor
	   float temp=fdirC[0];
	   fdirC[0]=fdirC[1];
	   fdirC[1]=-temp;
	   actor.SetDirection(fdirC,udirC);

	   //move actor
	   curPoseID = runID;
	   ACTIONid curPoseIDt = actor.GetCurrentAction(NULL, 0);
	   if(curPoseIDt!=runID){
		   actor.MakeCurrentAction(0, NULL, curPoseID);
		   
	   }
	   
	   int walkreturn=actor.MoveForward(movementspeed, TRUE, FALSE,0.0f,TRUE);
	   if(walkreturn=BOUNDARY){
		   float rotatecamera[3];
		   rotatecamera[0]=-fdirC[0];
		   rotatecamera[1]=-fdirC[1];
		   rotatecamera[2]=fdirC[2];
		   camera.SetDirection(rotatecamera,udirC);
		   camera.MoveForward(movementspeed, TRUE, FALSE,0.0f,TRUE);
		   
	   }

	   //camera rotate right
	   float posA[3];
	   float posC[3];
	   actor.GetPosition(posA);
	   camera.GetPosition(posC);
	   float buffheight=posC[2]-posA[2]-cameraheght;
	   float newdirC[3];
	   newdirC[0]=posA[0]-posC[0];
	   newdirC[1]=posA[1]-posC[1];
	   newdirC[2]=0;
	   camera.SetDirection(newdirC,udirC);
	   
	   //actor go back to circle and face right
	   float shouldmove=sqrt(radius*radius+movementspeed*movementspeed)-radius+1;
	   float nnewdirC[3]={-newdirC[0],-newdirC[1],0};
	   actor.SetDirection(nnewdirC,udirC);
	   actor.MoveForward(shouldmove, TRUE, FALSE,0.0f,TRUE);
	   float rnewdirC[3]={-nnewdirC[1],nnewdirC[0],0};
	   actor.SetDirection(rnewdirC,udirC);

	   //move camera height
	   float tempposC[3];
	   actor.GetPosition(posA);
	   camera.GetPosition(posC);
	   tempposC[0]=posC[0];
	   tempposC[1]=posC[1];
	   tempposC[2]=posA[2]+cameraheght+buffheight;
	   camera.SetPosition(tempposC);

	   //rotate camera to look at head
	   camera.GetPosition(posC);
	   float debugdis=distance2D(posA,posC);
	   camera.Rotate(X_AXIS, -atan((posC[2]-posA[2]-100)/distance2D(posA,posC))* 180 / PI, LOCAL);
	   //camera.Rotate(X_AXIS, -15, LOCAL);
	   last_key=4;
   }
   else if(FyCheckHotKeyStatus(FY_LEFT)) { /*FnActor actor;
									 actor.Object(nID);*/
	   //initialize camera's direction to flat
	  if(fx01!=NULL&&relive){
			delete fx01;
			fx01 = NULL;
			relive = FALSE;
	  }
	   float fdirC[3];
	   float udirC[3];
	   float positionA[3];
	   float positionC[3];
	   udirC[0]=0;
	   udirC[1]=0;
	   udirC[2]=1;
	   actor.GetPosition(positionA);
	   camera.GetPosition(positionC);
	   fdirC[0]=positionA[0]-positionC[0];
	   fdirC[1]=positionA[1]-positionC[1];
	   fdirC[2]=0;
	   camera.SetDirection(fdirC,udirC);

	   
	   //get distance between camera and actor
	   float radius = distance2D(positionA,positionC);

	   //turn the actor
	   float temp=fdirC[0];
	   fdirC[0]=fdirC[1];
	   fdirC[1]=-temp;
	   fdirC[0]=-fdirC[0];
	   fdirC[1]=-fdirC[1];
	   actor.SetDirection(fdirC,udirC);

	   //move actor
	   curPoseID = runID;
	   ACTIONid curPoseIDt = actor.GetCurrentAction(NULL, 0);
	   if(curPoseIDt!=runID){
		   actor.MakeCurrentAction(0, NULL, curPoseID);
		   
	   }
	   
	   int walkreturn=actor.MoveForward(movementspeed, TRUE, FALSE,0.0f,TRUE);
	   if(walkreturn=BOUNDARY){
		   float rotatecamera[3];
		   rotatecamera[0]=-fdirC[0];
		   rotatecamera[1]=-fdirC[1];
		   rotatecamera[2]=fdirC[2];
		   camera.SetDirection(rotatecamera,udirC);
		   camera.MoveForward(movementspeed, TRUE, FALSE,0.0f,TRUE);
		   
	   }

	   //camera rotate right
	   float posA[3];
	   float posC[3];
	   actor.GetPosition(posA);
	   camera.GetPosition(posC);
	   float buffheight=posC[2]-posA[2]-cameraheght;
	   float newdirC[3];
	   newdirC[0]=posA[0]-posC[0];
	   newdirC[1]=posA[1]-posC[1];
	   newdirC[2]=0;
	   camera.SetDirection(newdirC,udirC);

	   //actor go back to circle and face left
	   float shouldmove=sqrt(radius*radius+movementspeed*movementspeed)-radius+1;//<--??+1
	   float nnewdirC[3]={-newdirC[0],-newdirC[1],0};
	   actor.SetDirection(nnewdirC,udirC);
	   actor.MoveForward(shouldmove, TRUE, FALSE,0.0f,TRUE);
	   float lnewdirC[3]={nnewdirC[1],-nnewdirC[0],0};
	   actor.SetDirection(lnewdirC,udirC);

	   //move camera height
	   float tempposC[3];
	   actor.GetPosition(posA);
	   camera.GetPosition(posC);
	   tempposC[0]=posC[0];
	   tempposC[1]=posC[1];
	   tempposC[2]=posA[2]+cameraheght+buffheight;
	   camera.SetPosition(tempposC);

	   //rotate camera to look at head
	   camera.GetPosition(posC);
	   float debugdis=distance2D(posA,posC);
	   camera.Rotate(X_AXIS, -atan((posC[2]-posA[2]-100)/distance2D(posA,posC))* 180 / PI, LOCAL);
	   //camera.Rotate(X_AXIS, -15, LOCAL);
	   last_key=3;
   }   
   else
   {
	   /**/
	   if(distance2D(actorD,actor)>detectChaseDistance && distance2D(actorR,actor)>detectChaseDistance){
		   curPoseID = idleID;
		   ACTIONid curPoseIDt = actor.GetCurrentAction(NULL, 0);
		   if(curPoseIDt!=idleID && lockDamageNumber==0){
			   actor.MakeCurrentAction(0, NULL, curPoseID);		   
		   }
	   }
	   else{
		   curPoseID = CombatIdleID;
		   ACTIONid curPoseIDt = actor.GetCurrentAction(NULL, 0);
		   if(curPoseIDt!=CombatIdleID && lockDamageNumber==0){
			   actor.MakeCurrentAction(0, NULL, curPoseID);		   
		   }
	   }
	   
	   /*
	   curPoseID = NormalAttack1ID;
	   ACTIONid curPoseIDt = actor.GetCurrentAction(NULL, 0);
	   if(curPoseIDt!=NormalAttack1ID){
		   actor.MakeCurrentAction(0, NULL, NormalAttack1ID);
		   
	   }
	   actor.Play(0, LOOP, 1.0f, FALSE, TRUE);
	   */
   }

   //Donzo action
   if(actorD.GetCurrentAction(NULL, 0)!=DCombatIdleID && lockDonzoAttackNumber==0 && lockDonzoDamageNumber==0 && donzoHP>0 && (distance2D(actorD,actor)>detectChaseDistance || distance2D(actorD,actor)<100)){
	   actorD.MakeCurrentAction(0, NULL, DCombatIdleID);		   
   }
    else if(actorD.GetCurrentAction(NULL, 0)!=DrunID && lockDonzoAttackNumber==0 && lockDonzoDamageNumber==0 && donzoHP>0 && distance2D(actorD,actor)<=detectChaseDistance && distance2D(actorD,actor)>=100)
   {	   
	   actorD.MakeCurrentAction(0, NULL, DrunID);
	   if(level[4]==0){
		   float rob_pos[3];
		   rob_pos[0]=250.0f, rob_pos[1]=2554.0f, rob_pos[2]=0.0f;
		   newRobber(rob_pos);
		   rob_pos[0]=580.0f, rob_pos[1]=2882.0f, rob_pos[2]=0.0f;
		   newRobber(rob_pos);
	   	   rob_pos[0]=540.0f, rob_pos[1]=2882.0f, rob_pos[2]=0.0f;
		   newRobber(rob_pos);
		   rob_pos[0]=560.0f, rob_pos[1]=2800.0f, rob_pos[2]=0.0f;
		   newRobber(rob_pos);
		   rob_pos[0]=520.0f, rob_pos[1]=2882.0f, rob_pos[2]=0.0f;
		   newRobber(rob_pos);

		   rob_pos[0]=520.0f, rob_pos[1]=2882.0f, rob_pos[2]=0.0f;
		   newRobber(rob_pos);
		   rob_pos[0]=500.0f, rob_pos[1]=2882.0f, rob_pos[2]=0.0f;
		   newRobber(rob_pos);
		   rob_pos[0]=480.0f, rob_pos[1]=2882.0f, rob_pos[2]=0.0f;
		   newRobber(rob_pos);
		   rob_pos[0]=460.0f, rob_pos[1]=2882.0f, rob_pos[2]=0.0f;
		   newRobber(rob_pos);
		   rob_pos[0]=440.0f, rob_pos[1]=2882.0f, rob_pos[2]=0.0f;
		   newRobber(rob_pos);



		   level[4]=1;
		   finalnum=5;
		   /* play thunder.mp3
		   FnWorld gw;
		   gw.Object(gID);
		   FyBeginMedia("Data\\NTU4\\Media", 1);
		   HWND hwnd = FyGetWindowHandle(gw.Object());
		   MEDIAid mmID = FyCreateMediaPlayer(0, "thunder", 0, 0, 0, 0);
		   FnMedia mP;
		   mP.Object(mmID);
		   mP.Play(ONCE);*/
	   }
	   while(finalnum<5){
		   float rob_pos[3];
		   float Donzo_dir[3];
		   actorD.GetPosition(rob_pos);
		   actorD.GetDirection(Donzo_dir, NULL);
		   rob_pos[0]=rob_pos[0]+Donzo_dir[0]*100.0f;
		   rob_pos[1]=rob_pos[1]+Donzo_dir[1]*100.0f;
		   newRobber(rob_pos);
	   }
   }
   if(actorD.GetCurrentAction(NULL, 0)==DrunID && lockDonzoAttackNumber==0 && lockDonzoDamageNumber==0 && donzoHP>0 && distance2D(actorD,actor)<=detectChaseDistance && distance2D(actorD,actor)>=100)
   {
	   float vectorDtoL[2];
	   vectorAtoB2D(vectorDtoL, actorD, actor);
	   float direction_d[3];
	   float udir_d[3];
	   actorD.GetDirection(direction_d,udir_d);
	   direction_d[0]=vectorDtoL[0];
	   direction_d[1]=vectorDtoL[1];
	   actorD.SetDirection(direction_d,udir_d);
	   //calculate the ray cross
	   float Dpos[3];
	   actorD.GetPosition(Dpos);
	   for(int i=1;i<actor_num;i++){
		   char actorname1[20]="\0";
		   char actorname2[20]="\0";
		   FnActor tmpactor;
		   tmpactor.Object(record[i]);
		   sprintf(actorname1, "%p", tmpactor);
		   sprintf(actorname2, "%p", actorD);
		   if(strcmp(actorname1, actorname2)==0)
			   continue;
		   float mypos[3],mydir[3];
		   float totaltmp[3],totaltmp2[3];
		   tmpactor.GetDirection(mydir, NULL);
		   tmpactor.GetPosition(mypos);
		   for(int j=1;j<100;j++){
			   totaltmp[0]=mypos[0]+mydir[0]*j;
			   totaltmp[1]=mypos[1]+mydir[1]*j;
			   totaltmp[2]=mypos[2];
			   totaltmp2[0]=Dpos[0]+direction_d[0]*j;
			   totaltmp2[1]=Dpos[1]+direction_d[1]*j;
			   totaltmp2[2]=Dpos[2];
			   if(distance2D(totaltmp, totaltmp2)<50.0f){
				   actorD.TurnRight(-25.0f);
				   actorD.GetDirection(direction_d, NULL);
				   break;
			   }
		   }
	   }
	   actorD.MoveForward(movementspeed-10, TRUE, FALSE,0.0f,TRUE);
   }
   if(lockDonzoAttackNumber==0 && lockDonzoDamageNumber==0 && donzoHP>0 && distance2D(actorD,actor)<=100)
   {
	 if(Llive){	 
	   //facing Lyubu
	   float vectorDtoL[2];
	   float direction_d[3];
	   vectorAtoB2D(vectorDtoL, actorD, actor);
	   actorD.GetDirection(direction_d,NULL);
	   direction_d[0]=vectorDtoL[0];
	   direction_d[1]=vectorDtoL[1];
	   actorD.SetDirection(direction_d,NULL);
	   //if still too close to others,move backward
	   
	   //for(int i=1;i<actor_num;i++){
	//	   float tmp;
	//	   char name1[20]="\0",name2[20]="\0";
	//	   sprintf(name1, "%p", actorD);
	//	   sprintf(name2, "%p", record[i]);
	//	   if((tmp=distance2D(actorD, record[i]))<50.0f && strcmp(name1, name2)!=0){
	//		   actorD.MoveForward(-50.0f, TRUE, FALSE,0.0f,TRUE);
	//		   return;
	//	   }
	  // }

	   //attack
	   actorD.MakeCurrentAction(0, NULL, DNormalAttack1ID);
	   lockDonzoAttackNumber = 30;
     }
   }
   if(lockDonzoDamageNumber>0)lockDonzoDamageNumber--;
   if(lockDonzoAttackNumber>0)lockDonzoAttackNumber--;
   if(lockDonzoAttackNumber == 15 && lockDonzoDamageNumber==0 && attackJudge(actorD, actor))
   {
	 if(Llive){
	   actor.MakeCurrentAction(0, NULL, DamageHID);
	   lockDamageNumber=15;
	   if(lyubuHP>=0){
		lyubuHP-=20;
		if(lyubuHP<=0)lyubuHP=0;
		size[0]-=4.0f;
	    if(size[0]<=0.0f)size[0]=0.0f;
	    Lblood.SetSize(size);
	    //color of bloodbar
	    float coltmp[3];
	    if(size[0]/20.0f<=0.6f && size[0]/20.0f>0.2f){
		    coltmp[0]=1.0f, coltmp[1]=1.0f, coltmp[2]=0.0f;
		    Lblood.SetColor(coltmp);
	    }
	    else if(size[0]/20.0f<=0.2f){
		    coltmp[0]=1.0f, coltmp[1]=0.0f, coltmp[2]=0.0f;
		    Lblood.SetColor(coltmp);
	    }
	   }
	 }
   }
   
   if(donzoHP<=0)
   {
	   if(actorD.GetCurrentAction(NULL, 0)!=DDieID){
		   actorD.MakeCurrentAction(0, NULL, DDieID);
		   Dlive = FALSE;
		   poepleDie++;
		   audioDDie.Play(ONCE);

	   }
   }
   
   if(donzoHP<=0){
	   bool over=actorD.Play(0, ONCE, 1.0f, FALSE, TRUE);
	   /*
	   //delete from actor list
	   if(over==false){
		   for(int i=1;i<actor_num;i++){
			   char tmp1[20]="\0";
			   char tmp2[20]="\0";
			   FnActor tmpact;
			   tmpact.Object(record[i]);
			   sprintf(tmp1, "%p", actorD);
			   sprintf(tmp2, "%p", tmpact);
			   if(strcmp(tmp1, tmp2)==0){
				   for(int j=i;j<actor_num-1;j++){
					   record[j]=record[j+1];
					   bloodID_arr[j]=bloodID_arr[j+1];
				   }
				   actor_num--;
				   break;
			   }
		   }
	   }*/
   }
   else actorD.Play(0, LOOP, 1.0f, FALSE, TRUE);

   //Robber action
   for(int a=2;a<actor_num;a++){
	   actorR.Object(record[a]);
	   if(actorR.GetCurrentAction(NULL, 0)!=RCombatIdleID[a] && lockRobberAttackNumber[a]==0 && lockRobberDamageNumber[a]==0 && robberHP[a]>0 && (distance2D(actorR,actor)>detectChaseDistance || distance2D(actorR,actor)<100)){
		   actorR.MakeCurrentAction(0, NULL, RCombatIdleID[a]);		   
	   }
	   else if(actorR.GetCurrentAction(NULL, 0)!=RrunID[a] && lockRobberAttackNumber[a]==0 && lockRobberDamageNumber[a]==0 && robberHP[a]>0 && distance2D(actorR,actor)<=detectChaseDistance && distance2D(actorR,actor)>=100)
	   {	   
		   actorR.MakeCurrentAction(0, NULL, RrunID[a]);
	   }
	   if(actorR.GetCurrentAction(NULL, 0)==RrunID[a] && lockRobberAttackNumber[a]==0 && lockRobberDamageNumber[a]==0 && robberHP[a]>0 && distance2D(actorR,actor)<=detectChaseDistance && distance2D(actorR,actor)>=100)
	   {
		   float vectorRtoL[2];
		   vectorAtoB2D(vectorRtoL, actorR, actor);
		   float direction_r[3];
		   float udir_r[3];
		   actorR.GetDirection(direction_r,udir_r);
		   direction_r[0]=vectorRtoL[0];
		   direction_r[1]=vectorRtoL[1];
		   actorR.SetDirection(direction_r,udir_r);
		   //calculate the ray cross
		   float Rpos[3];
		   actorR.GetPosition(Rpos);
		   for(int i=1;i<actor_num;i++){
			   char actorname1[20]="\0";
			   char actorname2[20]="\0";
			   sprintf(actorname1, "%p", record[i]);
			   sprintf(actorname2, "%p", record[a]);
			   if(strcmp(actorname1, actorname2)==0)
				   continue;
				FnActor tmpact;
			   tmpact.Object(record[i]);
			   float mypos[3],mydir[3];
			   float totaltmp[3],totaltmp2[3];
			   tmpact.GetDirection(mydir, NULL);
			   tmpact.GetPosition(mypos);
			   for(int j=1;j<100;j++){
				   totaltmp[0]=mypos[0]+mydir[0]*j;
				   totaltmp[1]=mypos[1]+mydir[1]*j;
				   totaltmp[2]=mypos[2];
				   totaltmp2[0]=Rpos[0]+direction_r[0]*j;
				   totaltmp2[1]=Rpos[1]+direction_r[1]*j;
				   totaltmp2[2]=Rpos[2];
				   if(distance2D(totaltmp, totaltmp2)<50.0f){
					   actorR.TurnRight(turnr[a]);
					   actorR.GetDirection(direction_r, NULL);
					   break;
				   }
			   }
		   }
		   actorR.MoveForward(movementspeed-10, TRUE, FALSE,0.0f,TRUE);
	   }
	   if(lockRobberAttackNumber[a]==0 && lockRobberDamageNumber[a]==0 && robberHP[a]>0 && distance2D(actorR,actor)<=100)
	   {
		 if(Llive){  
		   //facing Lyubu
		   float vectorRtoL[2];
		   float direction_r[3];
		   vectorAtoB2D(vectorRtoL, actorR, actor);
		   actorR.GetDirection(direction_r,NULL);
		   direction_r[0]=vectorRtoL[0];
		   direction_r[1]=vectorRtoL[1];
		   actorR.SetDirection(direction_r,NULL);
		   //if still too close to others,move backward
		   /*
		   for(int i=1;i<actor_num;i++){
			   float tmp;
			   char name1[20]="\0",name2[20]="\0";
			   sprintf(name1, "%p", actorR);
			   sprintf(name2, "%p", record[i]);
			   if((tmp=distance2D(actorR, record[i]))<50.0f && strcmp(name1, name2)!=0){
				   actorR.MoveForward(-50.0f, TRUE, FALSE,0.0f,TRUE);
				   //actorR.MakeCurrentAction(0, NULL, RrunID);
				   return;
			   }
		   }
		   */
		   //attack
		   actorR.MakeCurrentAction(0, NULL, RNormalAttack1ID[a]);
		   lockRobberAttackNumber[a] = 30;
		 }
	   }
	   if(lockRobberDamageNumber[a]>0)lockRobberDamageNumber[a]--;
	   if(lockRobberAttackNumber[a]>0)lockRobberAttackNumber[a]--;
	   if(lockRobberAttackNumber[a] == 15 && lockRobberDamageNumber[a]==0 && attackJudge(actorR, actor))
	   {
		 if(Llive){
		   actor.MakeCurrentAction(0, NULL, DamageHID);
		   lockDamageNumber=10;
		   if(lyubuHP>=0){
		   lyubuHP-=10;
		   if(lyubuHP<=0)lyubuHP=0;
		   size[0]-=2.0f;
		   if(size[0]<=0.0f)size[0]=0.0f;
		   Lblood.SetSize(size);
		   //color of bloodbar
		   float coltmp[3];
		   if(size[0]/20.0f<=0.6f && size[0]/20.0f>0.2f){
			   coltmp[0]=1.0f, coltmp[1]=1.0f, coltmp[2]=0.0f;
			   Lblood.SetColor(coltmp);
		   }
		   else if(size[0]/20.0f<=0.2f){
			   coltmp[0]=1.0f, coltmp[1]=0.0f, coltmp[2]=0.0f;
			   Lblood.SetColor(coltmp);
		   }
		   }

		 }
	   }
	   if(robberHP[a]<=0)
	   {
		   if(actorR.GetCurrentAction(NULL, 0)!=RDieID[a]){
			   actorR.MakeCurrentAction(0, NULL, RDieID[a]);	
			   audioRDie.Play(ONCE);
		   }
	   }
	   if(robberHP[a]<=0){
		   bool ing=actorR.Play(0, ONCE, 1.0f, FALSE, TRUE);
		   actorR.MoveForward(-2.0f, TRUE, FALSE,0.0f,TRUE);
		   //delete from actor list
		   if(ing==false){
			   for(int i=1;i<actor_num;i++){
				   char tmp1[20]="\0";
				   char tmp2[20]="\0";
				   sprintf(tmp1, "%p", record[a]);
				   sprintf(tmp2, "%p", record[i]);
				   if(strcmp(tmp1, tmp2)==0){
					   freeRobber(i);
					   break;
				   }
			   }
		   }
	   }
	   else actorR.Play(0, LOOP, 1.0f, FALSE, TRUE);
   }

   //Lyubu
   if(lyubuHP>0){
	if(lockDamageNumber>0)lockDamageNumber--;
	actor.Play(0, LOOP, (float)skip, FALSE, TRUE);
   }else{
	   if(Llive){
		actor.MakeCurrentAction(0, NULL, LDieID);
		Llive = FALSE;
	   }
	
		if(!actor.Play(0, ONCE, (float)skip ,FALSE, TRUE)){
			//after do once
		} 
   }

   if (fx01 != NULL && fxEnd==false) {
      BOOL beOK = fx01->Play((float) skip);
      if (!beOK) {
         fx01->Reset();  // make it from the starting position and play it again
		 fxEnd=true;
         // if you just play the FX once, here you need to delete the FX
         delete fx01;
         fx01 = NULL;
      }
   }

	if (fx1 != NULL && fxEnd2==false){ if (!fx1->Play((float) skip)) { fx1->Reset(); fxEnd2=true; delete fx1; fx1 = NULL; }  }
	if (fx2 != NULL && fxEnd2==false){ if (!fx2->Play((float) skip)) { fx2->Reset(); fxEnd2=true; delete fx2; fx2 = NULL; }  }
	if (fx3 != NULL && fxEnd2==false){ if (!fx3->Play((float) skip)) { fx3->Reset(); fxEnd2=true; delete fx3; fx3 = NULL; }  }
	if (fx4 != NULL && fxEnd2==false){ if (!fx4->Play((float) skip)) { fx4->Reset(); fxEnd2=true; delete fx4; fx4 = NULL; }  }
	if (fx5 != NULL && fxEnd2==false){ if (!fx5->Play((float) skip)) { fx5->Reset(); fxEnd2=true; delete fx5; fx5 = NULL; }  }
	if (fx6 != NULL && fxEnd2==false){ if (!fx6->Play((float) skip)) { fx6->Reset(); fxEnd2=true; delete fx6; fx6 = NULL; }  }
	if (fx7 != NULL && fxEnd2==false){ if (!fx7->Play((float) skip)) { fx7->Reset(); fxEnd2=true; delete fx7; fx7 = NULL; }  }
	if (fx8 != NULL && fxEnd2==false){ if (!fx8->Play((float) skip)) { fx8->Reset(); fxEnd2=true; delete fx8; fx8 = NULL; }  }

   //relocate blood&mp bars
   bar_reloc();
   // render the scene
   FnViewport vp;
   vp.Object(vID);
   vp.Render(cID, TRUE, TRUE);

	/* qcl */

	// render the 2D scene for sprites on the top of the 3D
	vp.Render2D(sID2, FALSE, TRUE);  // no clear the background but clear the z buffer

	float aPos[3];
	float aPov[3];
	char integer[8];
	actor.GetWorldPosition(aPos);
	aPos[2] += 100.0f;
	vp.ComputeScreenPosition(cID,aPov,aPos,PHYSICAL_SCREEN,FALSE);
	shufa.Begin(vID);
	if(Llive){
		shufa.Write("★㊣↖煞气a呂布↘㊣☆",(int)aPov[0]-75,(int)aPov[1],255,255,0);
	}else{
		shufa.Write("★㊣↖陣亡a呂布↘㊣☆",(int)aPov[0]-75,(int)aPov[1],255,155,0);
	}

	itoa(Rnum,integer,10);
	shufa.Write("仍有             位雜魚尚未料理",600,40,200,100,200);
	shufa.Write(integer,635,40,255,0,0);

	if(Dlive){
		shufa.Write("尚未消滅敵方主將董卓",600,60,255,255,0);
	}else{
		shufa.Write("已經消滅敵方主將董卓",600,60,0,255,0);
	}

	shufa.End();

	shufaBig.Begin(vID);
	if(Llive){
		shufaBig.Write("呂布",140,10,255,255,255);
	}else{
		shufaBig.Write("呂布 (陣亡)",140,10,100,100,100);
	}
	shufaBig.Write("斬敵        人",600,10,255,115,0);

	
	itoa(poepleDie,integer,10);
	shufaBig.Write(integer,660,10,255,0,0);

	if(!Llive){
		if(counter>=29){
			lighting = !lighting;
		}
		if(lighting){
			shufaBig.Write("請按 r 鍵 Reboot 呂布",250,400,50,255,50);
		}
	}else if(gameOver){
		if(counter>=29){
			lighting = !lighting;
		}
		if(lighting){
			shufaBig.Write("戰勝董卓，遊戲獲勝！",250,400,50,255,50);
		}
	
	}



	shufaBig.End();
	
	ostringstream buf;


	FnWorld gw;
	gw.Object(gID);
	gw.StartMessageDisplay();
	
	buf << "HP = " << lyubuHP << endl;
	gw.MessageOnScreen(140,50,(char *)buf.str().c_str(),255,10,100,255);

	buf.str("");
	for(int i=0;i<lyubuHP;i++){
		buf << "|";
	}
	if( lyubuHP > 60){
		gw.MessageOnScreen(140,60,(char *)buf.str().c_str(),0,255,0,255);
	}else if(lyubuHP > 20){
		gw.MessageOnScreen(140,60,(char *)buf.str().c_str(),255,255,0,255);
	}else{
		gw.MessageOnScreen(140,60,(char *)buf.str().c_str(),255,0,0,255);
	}

	buf.str("");
	buf << "MP = " << Lsize[0] << endl;
	gw.MessageOnScreen(140,80,(char *)buf.str().c_str(),255,10,100,255);

	buf.str("");
	for(int i=0;i<(int)Lsize[0]*5;i++){
		buf << "|";
	}
	gw.MessageOnScreen(140,90,(char *)buf.str().c_str(),0,0,255,255);

	gw.FinishMessageDisplay();
	/* eoq */

   
   gw.SwapBuffers();
}

float distance2D(float* a,float* b)
{
	return sqrt((a[0]-b[0])*(a[0]-b[0])+(a[1]-b[1])*(a[1]-b[1]));
}

float distance2D(FnActor actorA,FnActor actorB)
{
	float a[3];
	float b[3];
	actorA.GetPosition(a);
	actorB.GetPosition(b);
	return sqrt((a[0]-b[0])*(a[0]-b[0])+(a[1]-b[1])*(a[1]-b[1]));
}

void vectorAtoB2D(float* ansVector, FnActor actorA,FnActor actorB)
{
	float position_a[3];
	float position_b[3];
	actorA.GetPosition(position_a);
	actorB.GetPosition(position_b);
	ansVector[0]=position_b[0]-position_a[0];
	ansVector[1]=position_b[1]-position_a[1];
}

bool attackJudge(FnActor attacker, FnActor victim)
{
	float position_a[3];
	float position_v[3];
	attacker.GetPosition(position_a);
	victim.GetPosition(position_v);
	if(distance2D(position_a,position_v)<=200)
	{
		float direction_a[3];
		float udir_a[3];
		attacker.GetDirection(direction_a,udir_a);
		float vector_atov[2];
		vector_atov[0]=position_v[0]-position_a[0];
		vector_atov[1]=position_v[1]-position_a[1];
		if(direction_a[0]*vector_atov[0]+direction_a[1]*vector_atov[1]>0)
		{
			return true;
		}
	}
	return false;
}

bool attackJudgeU(FnActor attacker, FnActor victim)
{
	float position_a[3];
	float position_v[3];
	attacker.GetPosition(position_a);
	victim.GetPosition(position_v);
	if(distance2D(position_a,position_v)<=600)
	{
		return true;
	}
	return false;
}


//relocate bar
void bar_reloc()
{
	float size[3];
	float postmp[3];
	FnBillBoard Lmp;
	//mp bar
	Lmp.Object(LmpID, 0);
	Lmp.GetSize(size);
	Lmp.GetPosition(postmp);
	switch(last_key){
		case 3:
			postmp[0]=0.0f;
			postmp[1]=size[0]/2-10.0f;
			break;
		case 4:
			postmp[0]=0.0f;
			postmp[1]=10.0f-size[0]/2;
			break;
		case 2:
			postmp[1]=0.0f;
			postmp[0]=size[0]/2-10.0f;
			break;
		case 1:
			postmp[1]=0.0f;
			postmp[0]=10.0f-size[0]/2;
			break;
	}
	Lmp.SetPosition(postmp);
	//blood bar
	Lmp.Object(LbloodID, 0);
	Lmp.GetSize(size);
	Lmp.GetPosition(postmp);
	switch(last_key){
		case 3:
			postmp[0]=0.0f;
			postmp[1]=size[0]/2-10.0f;
			break;
		case 4:
			postmp[0]=0.0f;
			postmp[1]=10.0f-size[0]/2;
			break;
		case 2:
			postmp[1]=0.0f;
			postmp[0]=size[0]/2-10.0f;
			break;
		case 1:
			postmp[1]=0.0f;
			postmp[0]=10.0f-size[0]/2;
			break;
	}
	Lmp.SetPosition(postmp);
	//other actors
	float dir1[3], dir2[3];
	camera.GetDirection(dir1, NULL);
	dir1[2]=0.0f;
	for(int i=1;i<actor_num;i++){
		FnActor tmpact;
		tmpact.Object(record[i]);
		tmpact.GetDirection(dir2, NULL);
		float cosi=(dir1[0]*dir2[0]+dir1[1]*dir2[1]+dir1[2]*dir2[2])/(float)sqrt(dir1[0]*dir1[0]+dir1[1]*dir1[1]+dir1[2]*dir1[2])*(float)sqrt(dir2[0]*dir2[0]+dir2[1]*dir2[1]+dir2[2]*dir2[2]);
		float sine=(float)acos(cosi);
		sine=(float)sin(sine);
		if(dir1[0]*dir2[1]-dir1[1]*dir2[0]<0)
			sine*=(-1);
		//mp bar(only donzo)
		if(i==1){
			Lmp.Object(mpID_arr[i], 0);
			Lmp.GetSize(size);
			Lmp.GetPosition(postmp);
			postmp[0]=cosi*(10.0f-size[0]/2);
			postmp[1]=sine*(size[0]/2-10.0f);
			Lmp.SetPosition(postmp);
		}
		//blood bar
		Lmp.Object(bloodID_arr[i], 0);
		Lmp.GetSize(size);
		Lmp.GetPosition(postmp);
		postmp[0]=cosi*(10.0f-size[0]/2);
		postmp[1]=sine*(size[0]/2-10.0f);
		Lmp.SetPosition(postmp);
	}
}
//new a Robber
void newRobber(float pos[3]){
	ACTORid nIDR = scene.LoadActor("Robber02");
    actorR.Object(nIDR);
	actorR.SetPosition(pos);
	actorR.PutOnTerrain(terrainID,false,0.0f);
	//blood bar
	FnObject bld;
	float mypos[3], size[2], color[3];
	OBJECTid RbloodID = scene.CreateObject(ROOT);
    bld.Object(RbloodID);
    mypos[0] = 0.0f;
    mypos[1] = 0.0f;
    mypos[2] = 80.0f;
    size[0] = 20.0f;
    size[1] = 2.0f;
    color[1] = 1.0f; color[0] = color[2] = 0.0f;
    bld.Billboard(mypos, size, NULL, 0, color);
    bld.SetParent(actorR.GetBaseObject());
	//generate action
	RrunID[actor_num] = actorR.GetBodyAction(NULL, "Run");
	RCombatIdleID[actor_num] = actorR.GetBodyAction(NULL, "CombatIdle");
	RNormalAttack1ID[actor_num] = actorR.GetBodyAction(NULL, "NormalAttack1");
	RDamageLID[actor_num] = actorR.GetBodyAction(NULL, "Damage1");
	RDamageHID[actor_num] = actorR.GetBodyAction(NULL, "Damage2");
	RDieID[actor_num] = actorR.GetBodyAction(NULL, "Dead");
	//RDamageLID[actor_num] = actorR.GetBodyAction(NULL, "LeftDamaged");
	//RDamageHID[actor_num] = actorR.GetBodyAction(NULL, "RightDamaged");
	//RDieID[actor_num] = actorR.GetBodyAction(NULL, "Die");
	//add to record
	record[actor_num]=nIDR;
	robberHP[actor_num]=50;
	lockRobberDamageNumber[actor_num]=0;
	lockRobberAttackNumber[actor_num]=0;
	int r=rand()%80;
	if(r%2==1)r*=(-1);
	turnr[actor_num]=r;
	bloodID_arr[actor_num++]=RbloodID;
	if(finalnum<5 && level[4]==1)finalnum++;

	Rnum++;
}

void freeRobber(int index){
	poepleDie++;
	for(int i=index;i<actor_num-1;i++){
		//delete action
		RrunID[i] = RrunID[i+1];
		RCombatIdleID[i] = RCombatIdleID[i+1];
		RNormalAttack1ID[i] = RNormalAttack1ID[i+1];
		RDamageLID[i] = RDamageLID[i+1];
		RDamageHID[i] = RDamageHID[i+1];
		RDieID[i] = RDieID[i+1];
		//delete from record
		record[i]=record[i+1];
		robberHP[i]=robberHP[i+1];
		lockRobberDamageNumber[i]=lockRobberDamageNumber[i+1];
		lockRobberAttackNumber[i]=lockRobberAttackNumber[i+1];
		bloodID_arr[i]=bloodID_arr[i+1];
	}
	actor_num--;
	if(finalnum>0 && level[4]==1)finalnum--;

	Rnum--;
}


//-------------------
// quit the demo
// C.Wang 0304, 2004
//-------------------
void QuitGame(WORLDid gID, BYTE code, BOOL value)
{
   if (code == FY_ESCAPE) {
      if (value) {
         FyWin32EndWorld(gID);
      }
   }
}


/*-----------------------------------
  initialize the pivot of the camera
  C.Wang 0329, 2004
 ------------------------------------*/
void InitPivot(WORLDid g, int x, int y)
{
   oldX = x;
   oldY = y;
}


/*------------------
  pivot the camera
  C.Wang 0329, 2004
 -------------------*/
void PivotCam(WORLDid g, int x, int y)
{
   FnObject model;

   if (x != oldX) {
      model.Object(cID);
      model.Rotate(Z_AXIS, (float) (x - oldX), GLOBAL);
      oldX = x;
   }

   if (y != oldY) {
      model.Object(cID);
      model.Rotate(X_AXIS, (float) (y - oldY), GLOBAL);
      oldY = y;
   }
}


/*----------------------------------
  initialize the move of the camera
  C.Wang 0329, 2004
 -----------------------------------*/
void InitMove(WORLDid g, int x, int y)
{
   oldXM = x;
   oldYM = y;
}


/*------------------
  move the camera
  C.Wang 0329, 2004
 -------------------*/
void MoveCam(WORLDid g, int x, int y)
{
   if (x != oldXM) {
      FnObject model;

      model.Object(cID);
      model.Translate((float)(x - oldXM)*0.1f, 0.0f, 0.0f, LOCAL);
      oldXM = x;
   }
   if (y != oldYM) {
      FnObject model;

      model.Object(cID);
      model.Translate(0.0f, (float)(oldYM - y)*0.1f, 0.0f, LOCAL);
      oldYM = y;
   }
}


/*----------------------------------
  initialize the zoom of the camera
  C.Wang 0329, 2004
 -----------------------------------*/
void InitZoom(WORLDid g, int x, int y)
{
   oldXMM = x;
   oldYMM = y;
}


/*------------------
  zoom the camera
  C.Wang 0329, 2004
 -------------------*/
void ZoomCam(WORLDid g, int x, int y)
{
   if (x != oldXMM || y != oldYMM) {
      FnObject model;

      model.Object(cID);
      model.Translate(0.0f, 0.0f, (float)(x - oldXMM), LOCAL);
      oldXMM = x;
      oldYMM = y;
   }
}
