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
#include "myMath.cpp"
#include <iostream>
#include <vector>

using namespace std;

#define PI 3.141592658

VIEWPORTid vID;                 // the major viewport
SCENEid sID;                    // the 3D scene
OBJECTid cID, tID;              // the main camera and the terrain for terrain following
CHARACTERid actorID;            // the major character

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

void cursorCtr(BYTE, BOOL4);
void trackCursorPos(void);

void Attack(BYTE, BOOL4);

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

	ATTRIBUTE(double getHP=100, double getATK=50, double getATK_H=70, double getDEF=30, double getSPD = 1){
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

public:
	ATTRIBUTE Attr;

	FnCharacter actor;
	CHARACTERid ID;
	ACTIONid idleID, runID, walkID, curPoseID;
	// ACTIONid idle2RunID[2], idle2WalkID[2], walk2RunID[2];

	ACTIONid dieID, tmpAtkID, beAtkedMainID;
	ACTIONid beAtkID0;	// for heavy attaak

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
			actor.Play(ONCE, (float) skip, FALSE, TRUE);
			if( blockCnt==0 ){
				curPoseID = idleID;
				blockCnt  = -1;
				actor.SetCurrentAction(NULL, 0, idleID, 0.0);
			}
		}
		else if( getSysAction==beAtkID0 ){
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
					actor.SetCurrentAction(0, NULL, walkID, 10.0);	  
				blockCnt =10;
			}
			else if(curPoseID == walkID){
				actor.SetCurrentAction(0, NULL, runID, 10.0);
				blockCnt = 5;
			}
			else if(curPoseID == runID && !(moveKeys[_MV_FW] || moveKeys[_MV_BK])){
				actor.SetCurrentAction(0, NULL, walkID, 10.0);
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

				curPoseID = walkID;
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
		if( !isGameOver ){
			if( Attr.isDie() ){
				actor.SetCurrentAction(0, NULL, dieID, 20.0);
				isGameOver = true;
			}
			else{
				actor.SetCurrentAction(0, NULL, beAtkedMainID, 20.0);
				blockCnt = 10;
				curPoseID = beAtkedMainID;
			}
		}
	}
	void GetHurt_H(int skip){
		if( !isGameOver ){
			if( Attr.isDie() ){
				actor.SetCurrentAction(0, NULL, dieID, 20.0);
				isGameOver = true;
			}
			else{
				actor.SetCurrentAction(0, NULL, beAtkID0, 20.0);
				blockCnt = 15;
				curPoseID = beAtkID0;
			}
		}

	}
	virtual bool AttackGoal(PLAYER &goal, int &type)=0;
}PLAYER;

typedef class MAINCHAR : public PLAYER{
public:
	ACTIONid beAtkID[2];
	ACTIONid atkID[3];

public:
	MAINCHAR(float posx = 0.0, float posy = 0.0, float posz = 0.0) : PLAYER(posx, posy, posz){
		;
	}
	~MAINCHAR(){;}

	void loadPlayerAction(){
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

typedef class NPC_R : public PLAYER{
public:
	// ACTIONid beAtkID0;
public:
	NPC_R(float posx = 0.0, float posy = 0.0, float posz = 0.0) : PLAYER(posx, posy, posz){
		;
	}
	~NPC_R(){;}

	void loadPlayerAction(){
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
		return false;
	}
}NPC_R;
typedef class DONZO : public PLAYER{
public:
	// ACTIONid beAtkID0;
public:
	DONZO(float posx = 0.0, float posy = 0.0, float posz = 0.0) : PLAYER(posx, posy, posz){
		;
	}
	~DONZO(){;}

	void loadPlayerAction(){
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
vector<NPC_R> robbot(8);
sCAMERA followCam;
BOOL isShowCursor = FALSE;

/*------------------
  the main program
  C.Wang 0720, 2006
 -------------------*/
void FyMain(int argc, char **argv){
	// create a new world
	BOOL4 beOK = FyStartFlyWin32("Homewrok03", 50, 20, 1024, 768, FALSE);

	// setup the data searching paths
	FySetModelPath("Data\\NTU\\\\Scenes");
	FySetTexturePath("Data\\NTU\\\\Scenes\\Textures");
	FySetScenePath("Data\\NTU\\\\Scenes");
	FySetShaderPath("Data\\NTU\\\\Shaders");

	// create a viewport
	vID = FyCreateViewport(0, 0, 1024, 768);
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

	mainChar.initial_pos(3941.0f, -3517.0f, 5.0f);
	mainChar.loadPlayer(scene, "Lyubu");

	npc01.initial_pos(4050.0f, -3470.0f, 285.0f);
	npc01.loadPlayer(scene, "Donzo");
	npc01.actor.TurnRight(180);

	for( int i=0 ; i<(int)robbot.size() ; i++ ){
		robbot[i].initial_pos(4050.0f+rand()%400-200, -3670.0f+rand()%400-200, 285.0f);
		robbot[i].loadPlayer(scene, "Robber02");
		robbot[i].actor.TurnRight(180);
	}

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

	// define some mouse functions
	// FyBindMouseFunction(LEFT_MOUSE, InitPivot, PivotCam, NULL, NULL);
	// FyBindMouseFunction(MIDDLE_MOUSE, InitZoom, ZoomCam, NULL, NULL);
	// FyBindMouseFunction(RIGHT_MOUSE, InitMove, MoveCam, NULL, NULL);

	// bind timers, frame rate = 30 fps
	FyBindTimer(0, 30.0f, GameAI, TRUE);
	FyBindTimer(1, 30.0f, RenderIt, TRUE);

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
		robbot[i].MoveChar(skip, followCam.pos);
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
	for( int i=0 ; i<(int)robbot.size() ; i++ ){
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
   sprintf(HP, "HP: %8.3f ", npc01.Attr.HP);
   sprintf(bkcnt, "bkcnt: %d ", mainChar.blockCnt);
   //

   char posS[256], fDirS[256], uDirS[256], temp[256];
   char charPos[256], charDir[256];
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

   text.Write(posS, 20, 35, 255, 255, 0);
   text.Write(fDirS, 20, 50, 255, 255, 0);
   text.Write(uDirS, 20, 65, 255, 255, 0);
   text.Write(temp, 20, 80, 255, 255, 0);
   text.Write(charPos, 20, 95, 255, 255, 0);
   text.Write(charDir, 20, 110, 255, 255, 0);

   text.Write(HP, 20, 130, 255, 255, 0);
   text.Write(bkcnt, 20, 150, 255, 255, 0);
   
   text.End();

   // swap buffer
   FySwapBuffers();
}

void cursorCtr(BYTE code, BOOL4 value){
	if( code == FY_F2 && value )
		isShowCursor = !isShowCursor;
}
void Attack(BYTE code, BOOL4 value){
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