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
#include "FyMedia.h"
#include "myMath.cpp"
#include <iostream>
#include <vector>

using namespace std;

#define PI 3.141592658

int _WIN_W = 1024;				// window width
int _WIN_H = 768;				// window height



VIEWPORTid vID;                 // the major viewport
SCENEid sID;                    // the 3D scene
SCENEid sID_2D;                 // the 2D scene
OBJECTid cID, tID;              // the main camera and the terrain for terrain following
CHARACTERid actorID;            // the major character
FnScene scene, scene_2D;

ROOMid terrainRoomID	= FAILED_ID;
TEXTid textID			= FAILED_ID;
OBJECTid lineTestID		= FAILED_ID;
OBJECTid uiBoard		= FAILED_ID;  // the sprite

FnSprite ui_keyHint;

VECTOR3D castleDirection;
double globalTime;
float test;
int score;

MEDIAid mmID;


//------audio--------
FnAudio audio_Atk_main ;
FnAudio audio_beAtkedMain_main;
FnAudio audio_die_main;
FnAudio audio_beAtkID0_main;
FnAudio audio_Atk_NPC_R ;
FnAudio audio_beAtkedMain_NPC_R;
FnAudio audio_die_NPC_R;
FnAudio audio_beAtkID0_NPC_R;
FnAudio audio_Atk_DONZO ;
FnAudio audio_beAtkedMain_DONZO;
FnAudio audio_die_DONZO;
FnAudio audio_beAtkID0_DONZO;

//--------------------

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

void cursorCtr(BYTE, BOOL4);
void trackCursorPos(void);

void Attack(BYTE, BOOL4);

// timer callbacks
void GameAI(int);
void RenderIt(int);
void GlobalTimer(int);

// mouse callbacks
void InitPivot(int, int);
void PivotCam(int, int);
void InitMove(int, int);
void MoveCam(int, int);
void InitZoom(int, int);
void ZoomCam(int, int);

void Attack_mouse_L(int x, int y);
void Attack_mouse_R(int x, int y);

float focusArea[3];

void NewWave(int fishNum,int bossNum);
void CallNewWave(BYTE code,BOOL4 value);

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
const float rotateStep = 0.3f;
const float moveStep   = 8.0f;
enum _MOVE_KEYS{
	_MV_FW = 0,
	_MV_BK = 1,
	_MV_L  = 2,
	_MV_R  = 3
};

typedef class ATTRIBUTE{
public:
	double HP;
	double ATK, ATK_H, DEF;
	double SPEED;
	double Org_HP;

	ATTRIBUTE(double getHP=100, double getATK=50, double getATK_H=70, double getDEF=30, double getSPD = 1){
		Org_HP= getHP;
		HP    = getHP;
		ATK   = getATK;
		ATK_H = getATK_H;
		DEF   = getDEF;
		SPEED = getSPD;
	}
	~ATTRIBUTE(){;}

	inline double getHit(ATTRIBUTE &attacker){
		// Decrease HP, return Attack Number
		double tmp = attacker.ATK - DEF;
		tmp = (tmp<0) ? 0 : tmp;
		HP -= tmp;
		return tmp;
	}
	inline double getHit_H(ATTRIBUTE &attacker){
		// Decrease HP, return Attack Number
		double tmp = attacker.ATK_H - DEF;
		tmp = (tmp<0) ? 0 : tmp;
		HP -= tmp;
		return tmp;
	}
	inline bool isDie(){
		if( HP <= 0 ){
			HP = 0;
			return true;
		}
		else
			return false;
	}
	void operator()(double getHP=1000, double getATK=50, double getDEF=30, double getSPD = 1){
		Org_HP= getHP;
		HP  = getHP;
		ATK = getATK;
		DEF = getDEF;
		SPEED = getSPD;
	}

}ATTRIBUTE;

float rot[]={0.0, 0.0};
POINT3D preMousePos;

typedef class PLAYER{
public:
	POINT3D pos;
	VECTOR3D fDir, uDir;
	float nowAngle;

	int blockCnt;
	// For Moving Animation & Control
	bool moveKeys[4];
	bool changeMove, cameraFollow;
	bool isGameOver;

	bool doAtk;
	int AtkKey;

	int actorType;

public:
	ATTRIBUTE Attr;

	FnCharacter actor;
	

	CHARACTERid ID;
	ACTIONid idleID, runID, walkID, curPoseID;
	// ACTIONid idle2RunID[2], idle2WalkID[2], walk2RunID[2];

	ACTIONid dieID, tmpAtkID, beAtkedMainID;
	ACTIONid beAtkID0;	// for heavy attaak
	
	GEOMETRYid bloodBarID;
	//GEOMETRYid garyBarID;
	OBJECTid baseID;                // the base object ID of the main character

	//audio
	//FnAudio audio_Atk;
	//FnAudio audio_beAtkedMain;
	//FnAudio audio_die;
	//FnAudio audio_beAtkID0;
	//FnAudio audio_walk;

	PLAYER(float posx = 0.0, float posy = 0.0, float posz = 0.0){
		// pos(posx, posy, posz);
		initial_pos(posx, posy, posz);
		fDir(1.0, 1.0, 0.0);
		uDir(0.0, 0.0, 1.0);

		for( int i=0 ; i<4 ; i++ )
			moveKeys[i] = false;
		changeMove   = false;
		cameraFollow = false;
		doAtk        = false;
		isGameOver   = false;
		blockCnt = -1;
		nowAngle = 0;
		AtkKey   = 0;
		
		bloodBarID = FAILED_ID;
		//garyBarID  = FAILED_ID;

	}
	void initial_pos(float posx = 0.0, float posy = 0.0, float posz = 0.0){
		pos(posx, posy, posz);
	}
	~PLAYER(){;}

	bool loadPlayer(FnScene &scene, char *fileName){
		float _fDir[3], _uDir[3], _pos[3];
		pos.getArr(_pos);
		fDir.getArr(_fDir);
		uDir.getArr(_uDir);

		ID = scene.LoadCharacter(fileName);
		actor.ID(ID);
		actor.SetDirection(_fDir, _uDir);
		actor.SetTerrainRoom(terrainRoomID, 10.0f);
		
		 // get base object of the character
		baseID = actor.GetBaseObject();
		
		BOOL4 beOK = actor.PutOnTerrain(_pos);

		loadPlayerAction();
		if( beOK )
			return true;
		else
			return false;
	}
	virtual void loadPlayerAction()=0;

	virtual bool Play_preIdf(int skip)=0;
	void Play(int skip){

		actor.Play(LOOP, (float) skip, FALSE, TRUE);
		if( Attr.isDie() ){
			actor.Play(ONCE, (float) skip, FALSE, TRUE);
			blockCnt = -1;
			return;
		}

		ACTIONid getSysAction = actor.GetCurrentAction(NULL);
		if( getSysAction==beAtkedMainID ){
			switch(actorType)
			{
				case 0:
					audio_beAtkedMain_main.Play(ONCE);
					break;
				case 2:
					audio_beAtkedMain_DONZO.Play(ONCE);
					break;
				default:
					audio_beAtkedMain_NPC_R.Play(ONCE);
					break;
			}

			actor.Play(ONCE, (float) skip, FALSE, TRUE);
			if( blockCnt==0 ){
				curPoseID = idleID;
				blockCnt  = -1;
				actor.SetCurrentAction(NULL, 0, idleID, 0.0);
			}
		}
		else if( getSysAction==beAtkID0 ){
			switch(actorType)
			{
				case 0:
					audio_beAtkID0_main.Play(ONCE);
					break;
				case 2:
					audio_beAtkID0_DONZO.Play(ONCE);
					break;
				default:
					audio_beAtkID0_NPC_R.Play(ONCE);
					break;
			}
			actor.Play(ONCE, (float) skip, FALSE, TRUE);
			if( blockCnt==0 ){
				curPoseID = idleID;
				blockCnt  = -1;
				actor.SetCurrentAction(NULL, 0, idleID, 0.0);
			}
		}
		if( Play_preIdf(skip) )
			return;

		// if( doAtk ){
		// 	if( curPoseID != tmpAtkID ){
		// 		actor.SetCurrentAction(NULL, 0, tmpAtkID, 0.0);
		// 		blockCnt = 10;
		// 		curPoseID = tmpAtkID;
		// 	}
		// 	return;
		// }

		if( changeMove ){
			actor.TurnRight((360-int(nowAngle))%360);

			// Idf curPoseID
			if(curPoseID==idleID){
				if( moveKeys[_MV_FW] || moveKeys[_MV_BK] )
					actor.SetCurrentAction(0, NULL, runID, 10.0);
				else
					actor.SetCurrentAction(0, NULL, runID, 10.0);	  
				blockCnt =10;
			}
			else if(curPoseID == walkID){
				actor.SetCurrentAction(0, NULL, runID, 10.0);
				blockCnt = 5;
			}
			else if(curPoseID == runID && !(moveKeys[_MV_FW] || moveKeys[_MV_BK])){
				actor.SetCurrentAction(0, NULL, runID, 10.0);
				blockCnt =5;
			}

			if( moveKeys[_MV_FW] ){
				if( moveKeys[_MV_R] )
					nowAngle = -45;
				else if( moveKeys[_MV_L] )
					nowAngle = 45;
				else
					nowAngle = 0;
				curPoseID = runID;
			}
			else if( moveKeys[_MV_BK] ){
				if( moveKeys[_MV_R] )
					nowAngle = 180+45;
				else if( moveKeys[_MV_L] )
					nowAngle = 180-45;
				else
					nowAngle = 180;
				curPoseID = runID;
			}
			else if( moveKeys[_MV_L] || moveKeys[_MV_R] ){
				if( moveKeys[_MV_R] )
					nowAngle = -90;
				else if( moveKeys[_MV_L] )
					nowAngle = 90;

				curPoseID = runID;
			}

			actor.TurnRight(nowAngle);

			changeMove = false;
		}
		// End of Change Move

		// Stop Moving
		if(!(moveKeys[_MV_FW]||moveKeys[_MV_BK]||moveKeys[_MV_L]||moveKeys[_MV_R]) && 
			(curPoseID==walkID || curPoseID==runID)){
			if(curPoseID==runID){
				actor.SetCurrentAction(0, NULL, idleID, 10.0);
			}
			else if(curPoseID == walkID){
				actor.SetCurrentAction(0, NULL, idleID, 10.0);
			}
			actor.TurnRight((360-int(nowAngle))%360);
			nowAngle  = 0;
			curPoseID = idleID;
			blockCnt  = -1;
			cameraFollow = false;
		}
	}
	void MoveChar(int skip, POINT3D fCamPos){
		if( blockCnt == -1 )		// IDLE
			return;
		if( blockCnt > 0 ){
			cameraFollow = false;
			blockCnt--;
			return;
		}
		// Declaration
		POINT3D afterMovePos;
		float arr_pos[3], arr_fDir[3], arr_uDir[3];

		actor.MoveForward(moveStep, TRUE, TRUE, 0, TRUE);

		actor.GetPosition(arr_pos);
		afterMovePos.putArr(arr_pos);

		if( pos == afterMovePos )
			cameraFollow = false;
		else
			cameraFollow = true;

		// update the data
		actor.GetPosition(arr_pos);
		actor.GetDirection(arr_fDir, arr_uDir);
		pos.putArr(arr_pos);
		fDir.putArr(arr_fDir);
		uDir.putArr(arr_uDir);
	}
	void GetHurt(int skip){
		if(bloodBarID == FAILED_ID)
			load_bloodbar(scene);
		if( !isGameOver ){
			if(bloodBarID == FAILED_ID)
			load_bloodbar(scene);

			if( Attr.isDie() ){
				FnBillboard bb(bloodBarID);
				float newSize[2];
				bb.GetSize(newSize);
				newSize[0] = 0.0f;
				bb.SetPositionSize(NULL, newSize);
				bb.Show(FALSE);
				bb.~FnBillboard();
				//FnBillboard bb1(garyBarID);
				//bb1.Show(FALSE);
				//bb1.~FnBillboard();

				switch(actorType)
				{
					case 0:
						audio_die_main.Play(ONCE);
						break;
					case 2:
						audio_die_DONZO.Play(ONCE);
						break;
					default:
						audio_die_NPC_R.Play(ONCE);
						break;
				}
				actor.SetCurrentAction(0, NULL, dieID, 20.0);
				isGameOver = true;
			}
			else{
				FnBillboard bb_blood(bloodBarID);
				bb_blood.Show(FALSE);
				bb_blood.~FnBillboard();
				//FnBillboard bb_gary(garyBarID);
				//bb_gary.Show(FALSE);
				//bb_gary.~FnBillboard();
				
				bloodBarID=FAILED_ID;
				//garyBarID=FAILED_ID;
				load_bloodbar(scene);

				actor.SetCurrentAction(0, NULL, beAtkedMainID, 20.0);
				blockCnt = 10;
				curPoseID = beAtkedMainID;
			}
			 
		}
	}
	void GetHurt_H(int skip){
		if( !isGameOver ){
			if(bloodBarID == FAILED_ID)
			load_bloodbar(scene);

			if( Attr.isDie() ){
				FnBillboard bb(bloodBarID);
				float newSize[2];
				bb.GetSize(newSize);
				newSize[0] = 0.0f;
				bb.SetPositionSize(NULL, newSize);
				bb.Show(FALSE);
				bb.~FnBillboard();
				//FnBillboard bb1(garyBarID);
				//bb1.Show(FALSE);
				//bb1.~FnBillboard();

				switch(actorType)
				{
					case 0:
						audio_die_main.Play(ONCE);
						break;
					case 2:
						audio_die_DONZO.Play(ONCE);
						break;
					default:
						audio_die_NPC_R.Play(ONCE);
						break;
				}
				actor.SetCurrentAction(0, NULL, dieID, 20.0);
				isGameOver = true;

			}
			else{
				FnBillboard bb_blood(bloodBarID);
				bb_blood.Show(FALSE);
				bb_blood.~FnBillboard();
				//FnBillboard bb_gary(garyBarID);
				//bb_gary.Show(FALSE);
				//bb_gary.~FnBillboard();
				
				bloodBarID=FAILED_ID;
				//garyBarID=FAILED_ID;
				load_bloodbar(scene);

				actor.SetCurrentAction(0, NULL, beAtkID0, 20.0);
				blockCnt = 15;
				curPoseID = beAtkID0;
			}
		}

	}
	virtual bool AttackGoal(PLAYER &goal, int &type)=0;

	void  load_bloodbar(FnScene &scene)
	{
		float size[2], color[4];

		// put  blood Bar
		OBJECTid bbID = scene.CreateObject(OBJECT);
		FnObject bb(bbID);
		bb.SetParent(baseID);
		float xTrans = -30.0f *(1-Attr.HP/Attr.Org_HP)/2.0f;
		bb.Translate(xTrans, 0.0f, 95.0f, REPLACE);

		size[0] = Attr.HP/Attr.Org_HP * 25.0f;
		size[1] = 1.2f;
		color[0] = color[3] = 1.0f; color[1] = color[2] = 0.0f;
		bloodBarID = bb.Billboard(NULL, size, NULL, 0, color);

		// put  gary Bar
		/*OBJECTid bbID1 = scene.CreateObject(OBJECT);
		FnObject bb1(bbID1);
		bb1.SetParent(baseID);
		bb1.Translate(0.0f, 0.0f, 95.0f, REPLACE);

		size[0] = 25.60f;
		size[1] = 1.8f;
		color[0] = 0.5f;
		color[1] = color[2] = 0.5f;
		color[3] = 0.5f; 
		
		garyBarID = bb1.Billboard(NULL, size, NULL, 0, color);*/
	}
}PLAYER;

typedef class MAINCHAR : public PLAYER{
public:
	ACTIONid beAtkID[2];
	ACTIONid atkID[3];

public:
	MAINCHAR(float posx = 0.0, float posy = 0.0, float posz = 0.0) : PLAYER(posx, posy, posz){
		actorType = 0; //MAINCHAR
	}
	~MAINCHAR(){;}

	void loadPlayerAction(){
		actorType = 0;
		//load audio
		/*audio_Atk.ID(FyCreateAudio());
		audio_Atk.Load("01_pose07");*/
		// audio_die.ID(FyCreateAudio());
		// audio_die.Load("02_pose25");
		// audio_beAtkedMain.ID(FyCreateAudio());
		// audio_beAtkedMain.Load("02_pose10");
		// audio_beAtkID0.ID(FyCreateAudio());
		// audio_beAtkID0.Load("01_pose12");
		
		// Get character actions
		idleID = actor.GetBodyAction(NULL, "CombatIdle");
		runID  = actor.GetBodyAction(NULL, "Run");
		walkID = actor.GetBodyAction(NULL, "Walk");
		dieID  = actor.GetBodyAction(NULL, "Die");

		beAtkedMainID = actor.GetBodyAction(NULL, "HeavyDamaged");
		beAtkID[0] = actor.GetBodyAction(NULL, "RightDamaged");
		beAtkID[1] = actor.GetBodyAction(NULL, "LeftDamaged");

		tmpAtkID = actor.GetBodyAction(NULL, "NormalAttack1");
		atkID[0] = actor.GetBodyAction(NULL, "NormalAttack2");
		atkID[1] = actor.GetBodyAction(NULL, "NormalAttack4");
		atkID[2] = actor.GetBodyAction(NULL, "HeavyAttack1");

		// set the character to idle action
		curPoseID = idleID;
		actor.SetCurrentAction(NULL, 0, curPoseID);
		actor.Play(START, 0.0f, FALSE, TRUE);
	}
	bool AttackGoal(PLAYER &goal, int &type){
		if( !doAtk )
			return false;

		double _IDF_DST = 200, _IDF_ANG[2] = {-20, 20};
		switch(AtkKey){
		case 1:
			_IDF_DST = 200;
			_IDF_ANG[0] = -10;
			_IDF_ANG[1] = 10;
			type = 0;
			break;
		case 2:
			_IDF_DST = 220;
			_IDF_ANG[0] = -360;
			_IDF_ANG[1] = 360;
			type = 1;
			break;
		case 3:
			_IDF_DST = 250;
			_IDF_ANG[0] = -20;
			_IDF_ANG[1] = 20;
			type = 0;
			break;
		case 4:
			_IDF_DST = 250;
			_IDF_ANG[0] = -20;
			_IDF_ANG[1] = 20;
			type = 1;
			break;
		default:
			_IDF_DST = 200;
			_IDF_ANG[0] = -20;
			_IDF_ANG[1] = 20;
			type = 0;
			break;
		}

		VECTOR3D goalDst = goal.pos - pos;
		goalDst.z = 0;

		double getDst = fabs(goalDst.magnitude());

		if( getDst < _IDF_DST ){
			goalDst.normalize();
			double getAngle = acos(goalDst.dot(fDir))*180.0/PI;
			double theSign = (goalDst.dot(fDir) > 0) ? 1.0 : -1.0;

			if( getAngle*theSign > _IDF_ANG[0] && getAngle*theSign < _IDF_ANG[1] )
				return true;
		}
		return false;
	}
	bool Play_preIdf(int skip){
		ACTIONid tmp;
		int tmpCnt = 10;

		switch(AtkKey){
		case 1:
			tmp = tmpAtkID;
			break;
		case 2:
			tmp = atkID[0];
			tmpCnt = 20;
			break;
		case 3:
			tmp = atkID[1];
			tmpCnt = 40;
			break;
		case 4:
			tmp = atkID[2];
			tmpCnt = 40;
			break;
		default:
			tmp = tmpAtkID;
			break;
		}

		ACTIONid getSysAction = actor.GetCurrentAction(NULL);
		if( getSysAction==tmp ){
			switch(actorType)
			{
			case 0:
				audio_Atk_main.Play(ONCE);
				break;
			case 2:
				audio_Atk_DONZO.Play(ONCE);
				break;
			default:
				audio_Atk_NPC_R.Play(ONCE);
				break;
			}
			
			actor.Play(ONCE, (float) skip, FALSE, TRUE);
			if( blockCnt==0 ){
				curPoseID = idleID;
				blockCnt  = -1;
				actor.SetCurrentAction(NULL, 0, idleID, 0.0);
			}
		}

		if( doAtk ){
			if( curPoseID != tmp ){
				actor.SetCurrentAction(NULL, 0, tmp, 0.0);
				blockCnt = tmpCnt;
				curPoseID = tmp;
			}
			return true;
		}
		return false;
	}
	void rotCharDir(){
		float R2PI = 1.0/180.0f*PI;
		POINT3D nowCharPos = pos;
		POINT3D tmpPt = fDir.findVertex(pos, 100.0);
		POINT3D resPt;

		resPt.x = nowCharPos.x + (tmpPt.x-nowCharPos.x)*cos(rot[0]*R2PI) - (tmpPt.y-nowCharPos.y)*sin(rot[0]*R2PI);
		resPt.y = nowCharPos.y + (tmpPt.x-nowCharPos.x)*sin(rot[0]*R2PI) + (tmpPt.y-nowCharPos.y)*cos(rot[0]*R2PI);
		resPt.z = pos.z;

		fDir = resPt - pos;
		fDir.normalize();

		float arr_fDir[3], arr_uDir[3];
		fDir.getArr(arr_fDir);
		uDir.getArr(arr_uDir);
		actor.SetDirection(arr_fDir, arr_uDir);
	}
}MAINCHAR;

//enemy AI Parameters

#define enemy_idle 0
#define enemy_turnToCasle 1
#define enemy_attkCasle 2

#define enemy_surrondPlayer 3
#define enemy_turnToPlayer 4
#define enemy_goForPlayer 5
#define enemy_attkPlayer 6
#define enemy_dead 7
#define enemy_DoAtk 8


typedef class ENEMY : public PLAYER
{
public:
	float moveStep;
	int status;
	float statusTime;
	float randomIdleTime;
	bool isArrive;
	bool isMoving;
	

private:
	bool statusCheck; //是否第一次進入這個status ture為第一次進入 false為不是


public:
	ENEMY(float posx=0.0,float posy =0.0, float posz=0.0):PLAYER(posx,posy,posz)
	{
		actorType =4;
		moveStep=5.0f;
		status = enemy_idle;
		statusTime = 0.0;
		statusCheck = true;		
		isArrive =false;
		isMoving =false;
		;}

	~ENEMY(){;}		

	void arriveCheck()
	{

		if(isArrive==false && pos.y >-3000)
		{
			isArrive = true;
			score +=1;
		}
	}

	void deadCheck()
	{
		if(isGameOver)
		{
			status=enemy_dead;
		}
	}

	void Play(int skip){

		actor.Play(LOOP, (float) skip, FALSE, TRUE);

		if( Attr.isDie() ){
			actor.Play(ONCE, (float) skip, FALSE, TRUE);
			blockCnt = -1;
			return;
		}

		ACTIONid getSysAction = actor.GetCurrentAction(NULL);

		if( getSysAction==beAtkedMainID ){
			switch(actorType)
			{
				case 0:
					audio_beAtkedMain_main.Play(ONCE);
					break;
				case 2:
					audio_beAtkedMain_DONZO.Play(ONCE);
					break;
				default:
					audio_beAtkedMain_NPC_R.Play(ONCE);
					break;
			}
			
			actor.Play(ONCE, (float) skip, FALSE, TRUE);
			if( blockCnt==0 ){
				curPoseID = idleID;
				blockCnt  = -1;
				actor.SetCurrentAction(NULL, 0, idleID, 0.0);
			}
		}
		else if( getSysAction==beAtkID0 ){
			switch(actorType)
			{
				case 0:
					audio_beAtkID0_main.Play(ONCE);
					break;
				case 2:
					audio_beAtkID0_DONZO.Play(ONCE);
					break;
				default:
					audio_beAtkID0_NPC_R.Play(ONCE);
					break;
			}
			actor.Play(ONCE, (float) skip, FALSE, TRUE);
			if( blockCnt==0 ){
				curPoseID = idleID;
				blockCnt  = -1;
				actor.SetCurrentAction(NULL, 0, idleID, 0.0);
			}
		}

		if( Play_preIdf(skip) )
			return;

		 if( doAtk ){
		 	if( curPoseID != tmpAtkID ){
		 		actor.SetCurrentAction(NULL, 0, tmpAtkID, 0.0);
		 		blockCnt = 5;
		 		curPoseID = tmpAtkID;
		 	}
		 	return;
		 }
	}

	void MoveChar(int skip, POINT3D fCamPos){
		if( !isMoving)
		{
			return;
		}

		if( blockCnt == -1 )		// IDLE
			return;

		if( blockCnt > 0 ){
			cameraFollow = false;
			blockCnt--;
			return;
		}
		// Declaration
		POINT3D afterMovePos;
		float arr_pos[3], arr_fDir[3], arr_uDir[3];

		actor.MoveForward(moveStep, TRUE, TRUE, 0, TRUE);

		actor.GetPosition(arr_pos);
		afterMovePos.putArr(arr_pos);

		if( pos == afterMovePos )
			cameraFollow = false;
		else
			cameraFollow = true;

		// update the data
		actor.GetPosition(arr_pos);
		actor.GetDirection(arr_fDir, arr_uDir);
		pos.putArr(arr_pos);
		fDir.putArr(arr_fDir);
		uDir.putArr(arr_uDir);
	}



	void chageStatus(int statusToChange)	//改變狀態並記錄當下時間
	{
		statusCheck = true;
		status=statusToChange;
		statusTime = globalTime;
		changeMove= true;
	}


	void Idle(PLAYER &mainChar)
	{
		if(statusCheck)
		{
			isMoving=false;
			randomIdleTime= 2+(rand()%30)*0.1;
			blockCnt=-1;
		    actor.SetCurrentAction(NULL,0,idleID);	
			statusCheck = false;			
			//actor.SetCurrentAction(NULL,0,idleID);
		}		
		
		if((mainChar.pos-pos).magnitude()<300)//若距離300有敵人 進入追蹤模式
		{
			chageStatus(enemy_surrondPlayer);
			return;
		}//進入包圍

		if((globalTime - statusTime)>randomIdleTime)//idle 2~5秒則進攻城堡
		{
				chageStatus(enemy_attkCasle);							
				return;
		
		}
	}

	

	void GoForCastle(PLAYER &mainChar)
	{			
		if(statusCheck)
		{
			isMoving=true;
			statusCheck = false;					
			actor.SetCurrentAction(NULL,0,runID);					
			blockCnt=20;
		}
			fDir=castleDirection;
			fDir.normalize();
			float _fDir[3], _uDir[3], _pos[3];
			pos.getArr(_pos);
			fDir.getArr(_fDir);
			uDir.getArr(_uDir);			
			actor.SetDirection(_fDir, _uDir);
			

		if((mainChar.pos-pos).magnitude()<300)//若距離300有敵人 進入追蹤模式
		{
			chageStatus(enemy_surrondPlayer);
			return;
		}//進入包圍
		
		if((globalTime - statusTime)>2) //前進2秒進入idle
		{							
			chageStatus(enemy_idle);
		}
	}


	void SurroundPlayer(PLAYER &mainChar)
	{
		if(statusCheck)
		{			
			statusCheck = false;
			isMoving=false;
			actor.SetCurrentAction(NULL,0,idleID);
			blockCnt=-1;
		}			
			VECTOR3D vec = (mainChar.pos - pos);

			vec.normalize();
			fDir.normalize();
			
			fDir=vec; 
			float _fDir[3], _uDir[3], _pos[3];
			pos.getArr(_pos);
			fDir.getArr(_fDir);
			uDir.getArr(_uDir);			
			actor.SetDirection(_fDir, _uDir);

		if((mainChar.pos-pos).magnitude() > 600)
		{
			chageStatus(enemy_attkCasle);
			return;			
		}	

		if((mainChar.pos-pos).magnitude() > 200)
		{
			chageStatus(enemy_goForPlayer);
			return;			
		}

	
	}

	void GoForPlayer(PLAYER &mainChar)
	{
		if(statusCheck)
		{
			isMoving=true;
			blockCnt=20;
			statusCheck = false;
			actor.SetCurrentAction(NULL,0,runID);			
		}		

			VECTOR3D vec = (mainChar.pos - pos);
			vec.normalize();
			fDir.normalize();
			
			fDir=vec; 
			float _fDir[3], _uDir[3], _pos[3];
			pos.getArr(_pos);
			fDir.getArr(_fDir);
			uDir.getArr(_uDir);			
			actor.SetDirection(_fDir, _uDir);


		if((mainChar.pos-pos).magnitude() <= 200)	//距離抵達350 準備攻擊
		{	
			isMoving=false;			
			actor.SetCurrentAction(NULL,0,idleID);
			chageStatus(enemy_attkPlayer);
			return;
		}

		
		if((mainChar.pos-pos).magnitude() > 600) //距離太遠 ->直接攻城
		{		
			chageStatus(enemy_attkCasle);
			return;
		}
			

	}


	void AttkPlayer(PLAYER &mainChar)
	{

		if(statusCheck)
		{
			isMoving=false;
			blockCnt=-1;
			statusCheck = false;
			//actor.SetCurrentAction(NULL,0,idleID);		
		}		
			VECTOR3D vec = (mainChar.pos - pos);
			vec.normalize();
			fDir.normalize();
			
			fDir=vec; 
			float _fDir[3], _uDir[3], _pos[3];
			pos.getArr(_pos);
			fDir.getArr(_fDir);
			uDir.getArr(_uDir);			
			actor.SetDirection(_fDir, _uDir);


		if(fabs((mainChar.pos-pos).magnitude()) > 600) //距離太遠 ->直接攻城
		{		
			chageStatus(enemy_attkCasle);
			return;
		}

		if((mainChar.pos-pos).magnitude()>200) 
		{			
			chageStatus(enemy_surrondPlayer);
			return;
		}


		if((globalTime - statusTime)>1) //停滯兩秒 20%機率發動攻擊
		{			
			if(rand()%10>5)
			{
				chageStatus(enemy_DoAtk);
			}
		}
	
	}

	void Attk()
	{
		if(statusCheck)
		{			
			isMoving=false;
			blockCnt=-1;
			statusCheck = false;
			actor.SetCurrentAction(NULL,0,tmpAtkID);		
			AtkKey=rand()%2;
			doAtk=true;
		}	
		

		chageStatus(enemy_idle);
	}

	


}ENEMY;

typedef class NPC_R : public ENEMY{
public:
	// ACTIONid beAtkID0;
public:
	NPC_R(float posx = 0.0, float posy = 0.0, float posz = 0.0) : ENEMY(posx, posy, posz){
		actorType = 1; //NPC_R
		;
	}
	~NPC_R(){;}

	void loadPlayerAction(){
		actorType = 1; //NPC_R
		//load audio
		// audio_Atk.ID(FyCreateAudio());
		// audio_Atk.Load("02_pose22");
		// audio_die.ID(FyCreateAudio());
		// audio_die.Load("03_pose25");
		// audio_beAtkedMain.ID(FyCreateAudio());
		// audio_beAtkedMain.Load("03_pose22");
		// audio_beAtkID0.ID(FyCreateAudio());
		// audio_beAtkID0.Load("03_pose22");

		// Get character actions
		idleID = actor.GetBodyAction(NULL, "CombatIdle");
		runID  = actor.GetBodyAction(NULL, "Run");
		dieID  = actor.GetBodyAction(NULL, "Die");

		beAtkedMainID = actor.GetBodyAction(NULL, "Damage1");
		beAtkID0 = actor.GetBodyAction(NULL, "Damage2");

		tmpAtkID = actor.GetBodyAction(NULL, "NormalAttack1");

		// set the character to idle action
		curPoseID = idleID;
		actor.SetCurrentAction(NULL, 0, curPoseID);
		actor.Play(START, 0.0f, FALSE, TRUE);
	}
	bool Play_preIdf(int skip){
		return false;
	}
	bool AttackGoal(PLAYER &goal, int &type){
		if( !doAtk )
			return false;

		double _IDF_DST = 200, _IDF_ANG[2] = {-20, 20};

		switch(type){
		case 1:
			_IDF_DST = 200;
			_IDF_ANG[0] = -20;
			_IDF_ANG[1] = 20;
			type = 0;
			break;
		case 2:
			_IDF_DST = 220;
			_IDF_ANG[0] = -20;
			_IDF_ANG[1] = 20;
			type = 1;
			break;
		
		default:
			_IDF_DST = 200;
			_IDF_ANG[0] = -20;
			_IDF_ANG[1] = 20;
			type = 0;
			break;
		}

		VECTOR3D goalDst = goal.pos - pos;
		goalDst.z = 0;

		double getDst = fabs(goalDst.magnitude());

		if( getDst < _IDF_DST ){
			goalDst.normalize();
			double getAngle = acos(goalDst.dot(fDir))*180.0/PI;
			double theSign = (goalDst.dot(fDir) > 0) ? 1.0 : -1.0;

			if( getAngle*theSign > _IDF_ANG[0] && getAngle*theSign < _IDF_ANG[1] )
				return true;
		}
		return false;
	}
}NPC_R;
typedef class DONZO : public PLAYER{
public:
	// ACTIONid beAtkID0;
public:
	DONZO(float posx = 0.0, float posy = 0.0, float posz = 0.0) : PLAYER(posx, posy, posz){
		actorType = 2; //DONZO
		;
	}
	~DONZO(){;}

	void loadPlayerAction(){
		actorType = 2; //DONZO
		//load audio
		// audio_Atk.ID(FyCreateAudio());
		// audio_Atk.Load("02_pose07");
		// audio_die.ID(FyCreateAudio());
		// audio_die.Load("02_pose25");
		// audio_beAtkedMain.ID(FyCreateAudio());
		// audio_beAtkedMain.Load("02_pose10");
		// audio_beAtkID0.ID(FyCreateAudio());
		// audio_beAtkID0.Load("02_pose10");


		// Get character actions
		idleID = actor.GetBodyAction(NULL, "Idle");
		runID  = actor.GetBodyAction(NULL, "Run");
		dieID  = actor.GetBodyAction(NULL, "Die");

		beAtkedMainID = actor.GetBodyAction(NULL, "DamageL");
		beAtkID0 = actor.GetBodyAction(NULL, "DamageH");

		tmpAtkID = actor.GetBodyAction(NULL, "NormalAttack1");

		// set the character to idle action
		curPoseID = idleID;
		actor.SetCurrentAction(NULL, 0, curPoseID);
		actor.Play(START, 0.0f, FALSE, TRUE);
	}
	bool Play_preIdf(int skip){
		return false;
	}
	bool AttackGoal(PLAYER &goal, int &type){
		return false;
	}
}DONZO;

// Camera Parameters
#define _XY_DST 600.0f
#define _Z_DST 1900.0f
#define _VIEW_H_PAD 65

typedef class sCAMERA{
public:
	POINT3D pos, focusPt;
	VECTOR3D fDir, uDir;

public:
	FnCamera camera;
	OBJECTid ID;

	sCAMERA(){;}
	~sCAMERA(){;}
	void computeCameraPos(float getPos[3], float getFDir[3], float getUDir[3]){
		VECTOR3D tmpFDir;
		tmpFDir.putArr(getFDir);

		pos.putArr(getPos);
		focusPt.putArr(getPos);
		fDir.putArr(getFDir);
		uDir.putArr(getUDir);

		pos.x = focusPt.x - _XY_DST*fDir.x;
		pos.y = focusPt.y - _XY_DST*fDir.y;
		pos.z = focusPt.z + _Z_DST*sin(5.0f/180.0f*PI);

		focusPt.z += _VIEW_H_PAD;
		
		fDir = focusPt - pos;
		fDir.normalize();

		uDir.x = -fDir.z * tmpFDir.x;
		uDir.y = -fDir.z * tmpFDir.y;
		uDir.z = fDir.x*tmpFDir.x + fDir.y*tmpFDir.y;
		setCameraPos();
	}
	void moveCamera_follow(PLAYER &refChar){
		float cpos[3], cfDir[3], cuDir[3];
		POINT3D nextCPos;

		nextCPos = refChar.fDir.findVertex(pos, moveStep);
		pos = nextCPos;

		VECTOR3D tmpFDir;
		tmpFDir = refChar.fDir;

		fDir = refChar.pos - nextCPos;
		fDir.z += _VIEW_H_PAD;

		VECTOR3D refUDir = refChar.uDir;
		refUDir.inverse();
		VECTOR3D tmpVec = fDir.cross(refUDir);
		tmpVec.normalize();
		uDir = fDir.cross(tmpVec);
		uDir.normalize();

		setCameraPos();
	}
	void updateCamera_rot(PLAYER &refChar){
		float R2PI = 1.0/180.0f*PI;
		POINT3D nowCamPos = pos;

		pos.x = refChar.pos.x + (nowCamPos.x-refChar.pos.x)*cos(rot[0]*R2PI) - (nowCamPos.y-refChar.pos.y)*sin(rot[0]*R2PI);
		pos.y = refChar.pos.y + (nowCamPos.x-refChar.pos.x)*sin(rot[0]*R2PI) + (nowCamPos.y-refChar.pos.y)*cos(rot[0]*R2PI);
		pos.z = refChar.pos.z + _Z_DST*sin(5.0f/180.0f*PI);

		fDir = refChar.pos - pos;
		fDir.z += _VIEW_H_PAD;
		fDir.normalize();

		VECTOR3D refUDir = refChar.uDir;
		refUDir.inverse();
		VECTOR3D tmpVec = fDir.cross(refUDir);
		tmpVec.normalize();
		uDir = fDir.cross(tmpVec);
		uDir.normalize();

		setCameraPos();
	}
	void setCameraPos(){
		float cpos[3], cfDir[3], cuDir[3];

		pos.getArr(cpos);
		fDir.getArr(cfDir);
		uDir.getArr(cuDir);

		camera.SetPosition(cpos);
		camera.SetDirection(cfDir, cuDir);
	}
	bool loadCamera(FnScene &scene){
		ID = scene.CreateObject(CAMERA);

		camera.ID(ID);
		camera.SetNearPlane(5.0f);
		camera.SetFarPlane(100000.0f);

		setCameraPos();
		return true;
	}
	
}sCAMERA;

MAINCHAR  mainChar;
DONZO npc01;
vector<NPC_R> robbot(0);
sCAMERA followCam;
BOOL isShowCursor = FALSE;
BOOL isShowHelpUI = TRUE;


/*------------------
  the main program
  C.Wang 0720, 2006
 -------------------*/
void FyMain(int argc, char **argv){
	// create a new world
	BOOL4 beOK = FyStartFlyWin32("Homewrok03", 50, 30, _WIN_W, _WIN_H, FALSE);


	castleDirection = VECTOR3D(0,1,0);
	castleDirection.normalize();
	globalTime = 0.0f;
	test = 0.0f;
	score =0;

	// setup the data searching paths
	FySetModelPath("Data\\NTU\\\\Scenes");
	FySetTexturePath("Data\\NTU\\\\Scenes\\Textures");
	// FySetTexturePath("Data\\Textures");
	FySetScenePath("Data\\NTU\\\\Scenes");
	FySetShaderPath("Data\\NTU\\\\Shaders");
	FySetAudioPath("Data\\NTU\\\\Audio");
	
	FyBeginMedia("Data\\NTU\\\\Media", 2);

	// create a viewport
	vID = FyCreateViewport(0, 0, _WIN_W, _WIN_H);
	FnViewport vp;
	vp.ID(vID);
	vp.SetBackgroundColor(0.2f, 0.2f, 0.2f);

	// Create Key Hint Panel
	sID_2D = FyCreateScene(1);
	scene_2D.Object(sID_2D);
	scene_2D.SetSpriteWorldSize(_WIN_W, _WIN_H);         // 2D scene size in pixels

	uiBoard = scene_2D.CreateObject(SPRITE);
	ui_keyHint.Object(uiBoard);
	ui_keyHint.SetSize(1024, 350);
	ui_keyHint.SetImage("keyHint01.png", 0, NULL, 0, NULL, NULL, MANAGED_MEMORY, FALSE, FALSE);
	ui_keyHint.SetPosition(0, 20, 0);

	// create a 3D scene
	sID = FyCreateScene(10);
	scene.ID(sID);

	// load the scene
	scene.Load("scene2");
	scene.SetAmbientLights(1.0f, 1.0f, 1.0f, 0.6f, 0.6f, 0.6f);

	//load background  music
	mmID = FyCreateMediaPlayer("MUSIC_village.mp3", 0, 0, _WIN_W, _WIN_H);
	FnMedia mP;
	mP.Object(mmID);
	mP.Play(LOOP);
	
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

	// load audio
	// audio_Atk_main.ID(FyCreateAudio());
	// audio_Atk_main.Load("01_pose07");
	// audio_die_main.ID(FyCreateAudio());
	// audio_die_main.Load("02_pose25");
	// audio_beAtkedMain_main.ID(FyCreateAudio());
	// audio_beAtkedMain_main.Load("02_pose10");
	// audio_beAtkID0_main.ID(FyCreateAudio());
	// audio_beAtkID0_main.Load("01_pose12");
	// 
	// audio_Atk_NPC_R.ID(FyCreateAudio());
	// audio_Atk_NPC_R.Load("02_pose22");
	// audio_die_NPC_R.ID(FyCreateAudio());
	// audio_die_NPC_R.Load("03_pose25");
	// audio_beAtkedMain_NPC_R.ID(FyCreateAudio());
	// audio_beAtkedMain_NPC_R.Load("03_pose22");
	// audio_beAtkID0_NPC_R.ID(FyCreateAudio());
	// audio_beAtkID0_NPC_R.Load("03_pose22");
	// 
	// audio_Atk_DONZO.ID(FyCreateAudio());
	// audio_Atk_DONZO.Load("02_pose07");
	// audio_die_DONZO.ID(FyCreateAudio());
	// audio_die_DONZO.Load("02_pose25");
	// audio_beAtkedMain_DONZO.ID(FyCreateAudio());
	// audio_beAtkedMain_DONZO.Load("02_pose10");
	// audio_beAtkID0_DONZO.ID(FyCreateAudio());
	// audio_beAtkID0_DONZO.Load("02_pose10");

	// load the character
	FySetModelPath("Data\\NTU\\\\Characters");
	FySetTexturePath("Data\\NTU\\\\Characters");
	FySetCharacterPath("Data\\NTU\\\\Characters");

	mainChar.initial_pos(3500.0f, -3000.0f, 285.0f);
	mainChar.loadPlayer(scene, "Lyubu");
	mainChar.load_bloodbar(scene);

	npc01.initial_pos(4050.0f, -3470.0f, 285.0f);
	npc01.loadPlayer(scene, "Donzo");
	npc01.actor.TurnRight(180);

	// for( int i=0 ; i<(int)robbot.size() ; i++ ){
	// 	robbot[i].initial_pos(4050.0f+rand()%400-200, -3670.0f+rand()%400-200, 285.0f);
	// 	robbot[i].loadPlayer(scene, "Robber02");
	// 	robbot[i].actor.TurnRight(180);
	// }

	NewWave(1,0);

	// put the character on terrain
	float pos[3], fDir[3], uDir[3];
	mainChar.actor.GetPosition(pos);
	mainChar.actor.GetDirection(fDir, uDir);

	followCam.computeCameraPos(pos, fDir, uDir);
	followCam.loadCamera(scene);
	// cID = followCam.ID;

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
	FyDefineHotKey(FY_1, Attack, FALSE);
	FyDefineHotKey(FY_2, Attack, FALSE);
	FyDefineHotKey(FY_3, Attack, FALSE);
	FyDefineHotKey(FY_4, Attack, FALSE);

	FyDefineHotKey(FY_F2, cursorCtr, FALSE);
	FyDefineHotKey(FY_P, CallNewWave, FALSE);

	// define some mouse functions
	FyBindMouseFunction(LEFT_MOUSE, Attack_mouse_L, NULL, NULL, NULL);
	FyBindMouseFunction(RIGHT_MOUSE, Attack_mouse_R, NULL, NULL, NULL);
	// FyBindMouseFunction(MIDDLE_MOUSE, InitZoom, ZoomCam, NULL, NULL);
	// FyBindMouseFunction(RIGHT_MOUSE, InitMove, MoveCam, NULL, NULL);

	// bind timers, frame rate = 30 fps
	FyBindTimer(0, 30.0f, GameAI, TRUE);
	FyBindTimer(1, 30.0f, RenderIt, TRUE);
	FyBindTimer(2, 10.0f, GlobalTimer, TRUE);

	// invoke the system
	FyInvokeFly(TRUE);

	movingBackward=false;
	movingForward=false;
	turingR=false;
	turingL=false;

	POINT mousePos;
	GetCursorPos(&mousePos);
	preMousePos.x = mousePos.x;
	preMousePos.y = mousePos.y;
}

/*-------------------------------------------------------------
  30fps timer callback in fixed frame rate for major game loop
  C.Wang 1103, 2007
 --------------------------------------------------------------*/

#define _FREE_MOVE_DST 80

float CHK_DST = 0.0f;
float lastPos[3] = {0.0f, 0.0f, 0.0f};

void trackCursorPos(void){
	POINT mousePos;
	GetCursorPos(&mousePos);

	rot[0] = (preMousePos.x - mousePos.x);
	// rot[1] = (preMousePos.y - mousePos.y);

	if( rot[0] == 0 )
		return;
	
	mainChar.rotCharDir();
	followCam.updateCamera_rot(mainChar);
	

	preMousePos.x = mousePos.x;
	preMousePos.y = mousePos.y;
}

void GameAI(int skip){
	// Hide the Cursor
	ShowCursor(isShowCursor);
	
	if (mmID != FAILED_ID) {
      FnMedia md;
      md.Object(mmID);
      if (md.GetState() == MEDIA_STOPPED) {
         // after playing, delete the media object
         FyDeleteMediaPlayer(mmID);
         mmID = FAILED_ID;
      }
      else {
         //return;
      }
	}

	FnObject terrain;
	
	terrain.ID(tID);
	
	// General Play Action Control
	mainChar.Play(skip);
	npc01.Play(skip);
	for( int i=0 ; i<(int)robbot.size() ; i++ ){
		robbot[i].Play(skip);
	}

	// Moving Posing Control
	mainChar.MoveChar(skip, followCam.pos);
	npc01.MoveChar(skip, followCam.pos);
	for( int i=0 ; i<(int)robbot.size() ; i++ ){
		//robbot[i].DO();
		robbot[i].MoveChar(skip, followCam.pos);
		switch(robbot[i].status)
		{
		case enemy_idle :
			robbot[i].Idle(mainChar);
			break;
		case enemy_attkCasle:
			robbot[i].GoForCastle(mainChar);
			break;
		case enemy_attkPlayer :
			robbot[i].AttkPlayer(mainChar);
			break;
		case enemy_goForPlayer :
			robbot[i].GoForPlayer(mainChar);
			break;
		case enemy_surrondPlayer:
			robbot[i].SurroundPlayer(mainChar);
			break;
		case enemy_DoAtk:
			robbot[i].Attk();
			//robbot[i].TurnToCastle(mainChar);
			break;
		case enemy_turnToPlayer:
			//robbot[i].TurnToPlayer(mainChar);
			break;

		}
		robbot[i].deadCheck();
		robbot[i].arriveCheck();

		
	}
	
	if(score>10)
	{
		mainChar.Attr.HP=0;		
		mainChar.GetHurt_H(skip);
	}
	


	// Attack People
	int getType;
	if( mainChar.AttackGoal(npc01, getType) ){
		if( getType == 0 ){			// Normal Attack
			npc01.Attr.getHit(mainChar.Attr);
			npc01.GetHurt(skip);
		}
		else if( getType == 1 ){	// Heavy Attack
			npc01.Attr.getHit_H(mainChar.Attr);
			npc01.GetHurt_H(skip);
		}
	}

	for( int i=0 ; i<(int)robbot.size() ; i++ )
	{
		if( mainChar.AttackGoal(robbot[i], getType) ){
			if( getType == 0 ){			// Normal Attack
				robbot[i].Attr.getHit(mainChar.Attr);
				robbot[i].GetHurt(skip);
			}
			else if( getType == 1 ){	// Heavy Attack
				robbot[i].Attr.getHit_H(mainChar.Attr);
				robbot[i].GetHurt_H(skip);
			}
		}

		if(robbot[i].doAtk)
		{
			int atkType =rand()%2;
			if( robbot[i].AttackGoal(mainChar,atkType))
			{
				if( atkType == 0 ){			// Normal Attack
					mainChar.Attr.getHit(robbot[i].Attr);
					mainChar.GetHurt(skip);
				}
				else if( atkType == 1 ){	// Heavy Attack
					mainChar.Attr.getHit_H(robbot[i].Attr);
					mainChar.GetHurt_H(skip);
				}
			}
	
			robbot[i].doAtk=false;
		}
	}



	if( mainChar.cameraFollow )
		followCam.moveCamera_follow(mainChar);

	trackCursorPos();

	if(mainChar.doAtk)
		mainChar.doAtk = false;
}

void RenderIt(int skip){
	FnViewport vp;
	
	// render the whole scene
	vp.ID(vID);
	
	vp.Render3D(followCam.ID, TRUE, TRUE);
	if( isShowHelpUI )
		vp.RenderSprites(sID_2D, FALSE, TRUE);
	
	// get camera's data
	float pos[3], fDir[3], uDir[3];
	followCam.camera.GetPosition(pos);
	followCam.camera.GetDirection(fDir, uDir);
	
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
	
	//
	char HP[256], bkcnt[256];
	sprintf(HP, "HP: %8.3f %d TIME[%lf]", npc01.Attr.HP, robbot[0].status, globalTime);
	sprintf(bkcnt, "bkcnt: %d ", mainChar.blockCnt);
	//
	
	char posS[256], fDirS[256], uDirS[256], temp[256];
	char charPos[256], charDir[256];
	char testS[256];
	followCam.camera.GetPosition(pos);
	followCam.camera.GetDirection(fDir, uDir);
	sprintf(posS, "pos: %8.3f %8.3f %8.3f", pos[0], pos[1], pos[2]);
	sprintf(fDirS, "facing: %8.3f %8.3f %8.3f", fDir[0], fDir[1], fDir[2]);
	sprintf(uDirS, "up: %8.3f %8.3f %8.3f", uDir[0], uDir[1], uDir[2]);
	
	mainChar.actor.GetPosition(pos);
	mainChar.actor.GetDirection(fDir, uDir);
	sprintf(temp, "rot[0 1]: %f %f\n", rot[0], rot[1]);
	sprintf(charPos, "Char Pos: %f %f %f\n", pos[0], pos[1], pos[2]);
	sprintf(charDir, "Char fDir: %f %f %f\n", fDir[0], fDir[1], fDir[2]);
	sprintf(testS,"test Value: %d\n", score);
	
	text.Write(posS, 20, 35, 255, 255, 0);
	text.Write(fDirS, 20, 50, 255, 255, 0);
	text.Write(uDirS, 20, 65, 255, 255, 0);
	text.Write(temp, 20, 80, 255, 255, 0);
	text.Write(charPos, 20, 95, 255, 255, 0);
	text.Write(charDir, 20, 110, 255, 255, 0);
	
	text.Write(HP, 20, 130, 255, 255, 0);
	text.Write(testS, 20, 150, 255, 255, 0);
	
	text.End();
	
	// swap buffer
	FySwapBuffers();
}


void GlobalTimer(int skip)
{
	globalTime +=0.1;
}


void cursorCtr(BYTE code, BOOL4 value){
	if( code == FY_F2 && value )
		isShowCursor = !isShowCursor;
}
void Attack_mouse_L(int x, int y){
	mainChar.doAtk  = true;
	mainChar.AtkKey = 1;
}
void Attack_mouse_R(int x, int y){
	mainChar.doAtk  = true;
	mainChar.AtkKey = 2;
}
void Attack(BYTE code, BOOL4 value){
	return;
	if(code == FY_1 && value ){
		mainChar.doAtk  = true;
		mainChar.AtkKey = 1;
	}
	if(code == FY_2 && value ){
		mainChar.doAtk  = true;
		mainChar.AtkKey = 2;
	}
	if(code == FY_3 && value ){
		mainChar.doAtk  = true;
		mainChar.AtkKey = 3;
	}
	if(code == FY_4 && value ){
		mainChar.doAtk  = true;
		mainChar.AtkKey = 4;
	}
}
void Movement(BYTE code, BOOL4 value){
	if( code == FY_F1 ){
		if( value )
			isShowHelpUI = !isShowHelpUI;
		return;
	}
	if(code == 38 || code == FY_W ){
		mainChar.moveKeys[_MV_FW] = (value) ? true : false;
	}

	if(code ==40 || code == FY_S ){
		mainChar.moveKeys[_MV_BK] = (value) ? true : false;
	}

	if(code == 39 || code == FY_D ){
		mainChar.moveKeys[_MV_L] = (value) ? true : false;
	}

	if(code == 37 || code == FY_A ){
		mainChar.moveKeys[_MV_R] = (value) ? true : false;
	}

	mainChar.changeMove = true;
   // Homework #01 part 2
}


void CallNewWave(BYTE code,BOOL4 value)
{
	if(code == FY_P  ||  code == 37)
	{
		NewWave(5,1);
	}
}


void NewWave(int fishNum,int bossNum)
{

	for(int i =0;i<fishNum;i++)
	{
		NPC_R robot = NPC_R(3320.0f+rand()%460,-4290.0f+rand()%140,285.0f);		
		robot.loadPlayer(scene,"Robber02");			
		robbot.insert(robbot.end(),robot);
	}		
	//robbot[i].actor.TurnRight(180);
}



void QuitGame(BYTE code, BOOL4 value)
{
   if (code == FY_ESCAPE) {
      if (value) {
         FyQuitFlyWin32();
      }
   }
}

void InitPivot(int x, int y){
   oldX = x;
   oldY = y;
   frame = 0;
}
void PivotCam(int x, int y){
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

void InitMove(int x, int y){
   oldXM = x;
   oldYM = y;
   frame = 0;
}
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

void InitZoom(int x, int y){
   oldXMM = x;
   oldYMM = y;
   frame = 0;
}
void ZoomCam(int x, int y){
   if (x != oldXMM || y != oldYMM) {
      FnObject model;

      model.ID(cID);
      model.Translate(0.0f, 0.0f, (float)(x - oldXMM)*10.0f, LOCAL);
      oldXMM = x;
      oldYMM = y;
   }
}