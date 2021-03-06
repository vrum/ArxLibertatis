/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */
/* Based on:
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code').

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/
// Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#include "core/Core.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <iomanip>
#include <sstream>
#include <vector>

#include <boost/version.hpp>
#include <boost/foreach.hpp>

#include "Configure.h"

#include "ai/Paths.h"
#include "ai/PathFinderManager.h"

#include "animation/Animation.h"
#include "animation/AnimationRender.h"
#include "animation/Intro.h"

#include "cinematic/Cinematic.h"
#include "cinematic/CinematicKeyframer.h"

#include "core/Application.h"
#include "core/ArxGame.h"
#include "core/Config.h"
#include "core/Localisation.h"
#include "core/GameTime.h"
#include "core/Version.h"

#include "game/Damage.h"
#include "game/EntityManager.h"
#include "game/Equipment.h"
#include "game/Inventory.h"
#include "game/Levels.h"
#include "game/Missile.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "game/Spells.h"
#include "game/npc/Dismemberment.h"
#include "game/spell/FlyingEye.h"
#include "game/spell/Cheat.h"
#include "game/effect/Quake.h"

#include "gui/Hud.h"
#include "gui/LoadLevelScreen.h"
#include "gui/Menu.h"
#include "gui/MenuWidgets.h"
#include "gui/Speech.h"
#include "gui/MiniMap.h"
#include "gui/TextManager.h"

#include "graphics/BaseGraphicsTypes.h"
#include "graphics/Draw.h"
#include "graphics/DrawLine.h"
#include "graphics/DrawDebug.h"
#include "graphics/font/Font.h"
#include "graphics/GraphicsModes.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/Math.h"
#include "graphics/Renderer.h"
#include "graphics/Vertex.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/Fade.h"
#include "graphics/effects/Fog.h"
#include "graphics/image/Image.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/ParticleManager.h"
#include "graphics/particle/MagicFlare.h"
#include "graphics/texture/TextureStage.h"

#include "gui/Cursor.h"
#include "gui/Interface.h"
#include "gui/Text.h"

#include "input/Input.h"
#include "input/Keyboard.h"
#include "input/Mouse.h"

#include "io/fs/SystemPaths.h"
#include "io/resource/ResourcePath.h"
#include "io/resource/PakReader.h"
#include "cinematic/CinematicLoad.h"
#include "io/Screenshot.h"
#include "io/log/Logger.h"

#include "math/Angle.h"
#include "math/Rectangle.h"
#include "math/Vector.h"
 
#include "physics/Collisions.h"
#include "physics/Projectile.h"

#include "platform/CrashHandler.h"
#include "platform/Flags.h"
#include "platform/Platform.h"
#include "platform/ProgramOptions.h"

#include "scene/LinkedObject.h"
#include "cinematic/CinematicSound.h"
#include "scene/ChangeLevel.h"
#include "scene/Scene.h"
#include "scene/GameSound.h"
#include "scene/LoadLevel.h"
#include "scene/Interactive.h"
#include "scene/Light.h"
#include "scene/Object.h"

#include "script/Script.h"

#include "window/RenderWindow.h"

class TextManager;

Image savegame_thumbnail;

extern TextManager	*pTextManage;
extern float FORCE_TIME_RESTORE;

extern long		DONT_WANT_PLAYER_INZONE;
extern long		TOTPDL;
extern long		COLLIDED_CLIMB_POLY;

//-----------------------------------------------------------------------------

ParticleManager	*pParticleManager = NULL;

TextureContainer *	GoldCoinsTC[MAX_GOLD_COINS_VISUALS]; // Gold Coins Icons

#if BUILD_EDIT_LOADSAVE
EERIE_MULTI3DSCENE * mse = NULL;
long ADDED_IO_NOT_SAVED = 0;
#endif

Vec2s DANAEMouse;
Vec3f moveto;
Vec3f Mscenepos;
Vec3f lastteleport;
EERIE_3DOBJ * GoldCoinsObj[MAX_GOLD_COINS_VISUALS];// 3D Objects For Gold Coins
EERIE_3DOBJ	* arrowobj=NULL;			// 3D Object for arrows
EERIE_3DOBJ * cameraobj=NULL;			// Camera 3D Object		// NEEDTO: Remove for Final
EERIE_3DOBJ * markerobj=NULL;			// Marker 3D Object		// NEEDTO: Remove for Final
EERIE_3DOBJ * cabal=NULL;				// Cabalistic 3D Object // NEEDTO: Load dynamically

std::string WILLADDSPEECH;

Vec2s STARTDRAG;
Entity * COMBINE=NULL;

// START - Information for Player Teleport between/in Levels-------------------------------------
std::string TELEPORT_TO_LEVEL;
std::string TELEPORT_TO_POSITION;
long TELEPORT_TO_ANGLE;
// END -   Information for Player Teleport between/in Levels---------------------------------------
res::path LastLoadedScene;

float BASE_FOCAL=350.f;
float STRIKE_AIMTIME=0.f;

float framedelay=0.f;

long LOAD_N_DONT_ERASE=0;
long NO_TIME_INIT=0;

Rect g_size(640, 480);
Vec2f g_sizeRatio(1.f, 1.f);

long CurrFightPos=0;
long NO_PLAYER_POSITION_RESET=0;

float BOW_FOCAL=0;
long PlayerWeaponBlocked=-1;

long REQUEST_SPEECH_SKIP= 0;
long CURRENTLEVEL		= -1;
long DONT_ERASE_PLAYER	= 0;
long FASTmse			= 0;

//-----------------------------------------------------------------------------
// EDITOR FLAGS/Vars
//-----------------------------------------------------------------------------
// Flag used to Launch Moulinex
long LOADEDD = 0; // Is a Level Loaded ?

long CHANGE_LEVEL_ICON=-1;

bool g_cursorOverBook = false;
//-----------------------------------------------------------------------------
// DEBUG FLAGS/Vars
//-----------------------------------------------------------------------------
bool FirstFrame=true;
unsigned long WILLADDSPEECHTIME=0;
unsigned long AimTime;
//-----------------------------------------------------------------------------

long START_NEW_QUEST=0;
static long LAST_WEAPON_TYPE = -1;

float Original_framedelay=0.f;

float PULSATE;

extern EERIE_CAMERA * ACTIVECAM;


bool g_debugToggles[10];

// Sends ON GAME_READY msg to all IOs
void SendGameReadyMsg()
{
	LogDebug("SendGameReadyMsg");
	SendMsgToAllIO(SM_GAME_READY);
}

bool AdjustUI() {
	
	// Sets Danae Screen size depending on windowed/full-screen state
	g_size = Rect(mainApp->getWindow()->getSize().x, mainApp->getWindow()->getSize().y);
		
	// Computes X & Y screen ratios compared to a standard 640x480 screen
	
	g_sizeRatio.x = g_size.width() * ( 1.0f / 640 );
	g_sizeRatio.y = g_size.height() * ( 1.0f / 480 );
	
	if(!ARX_Text_Init()) {
		return false;
	}
	
	MenuReInitAll();
	
	return true;
}

void DanaeRestoreFullScreen() {
	
	MenuReInitAll();
	
	AdjustUI();

	LoadScreen();
}

void runGame() {
	
	// TODO Time will be re-initialized later, but if we don't initialize it now casts to int might overflow.
	arxtime.init();
	
	mainApp = new ArxGame();
	if(mainApp->initialize()) {
		// Init all done, start the main loop
		mainApp->run();
	} else {
		// Fallback to a generic critical error in case none was set yet...
		LogCritical << "Application failed to initialize properly.";
	}
	
	if(mainApp) {
		mainApp->shutdown();
		delete mainApp;
		mainApp = NULL;
	}
}


//*************************************************************************************
// Entity * FlyingOverObject(EERIE_S2D * pos)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Returns IO under cursor, be it in inventories or in scene
//   Returns NULL if no IO under cursor
//*************************************************************************************
Entity * FlyingOverObject(const Vec2s & pos)
{
	Entity* io = NULL;

	if((io = GetFromInventory(pos)) != NULL)
		return io;

	if(InInventoryPos(pos))
		return NULL;

	if((io = InterClick(pos)) != NULL)
		return io;

	return NULL;
}

static void PlayerLaunchArrow_Test(float aimratio, float poisonous, const Vec3f & pos, const Anglef & angle) {
	
	Vec3f vect = angleToVecForArrow(angle);
	Vec3f position = pos;
	float velocity = aimratio + 0.3f;

	if(velocity < 0.9f)
		velocity = 0.9f;
	
	glm::quat quat = angleToQuatForArrow(angle);

	float wd = getEquipmentBaseModifier(IO_EQUIPITEM_ELEMENT_Damages);
	// TODO Why ignore relative modifiers? Why not just use player.Full_damages?
	
	float damages = wd * (1.f + (player.m_skillFull.projectile + player.m_attributeFull.dexterity) * (1.f/50));

	ARX_THROWN_OBJECT_Throw(PlayerEntityHandle, position, vect, quat, velocity, damages, poisonous);
}

extern unsigned long LAST_JUMP_ENDTIME;

//*************************************************************************************
// Switches from/to Game Mode/Editor Mode
//*************************************************************************************
void SetEditMode(long ed, const bool stop_sound) {
	
	LAST_JUMP_ENDTIME = 0;
	
	if(!DONT_ERASE_PLAYER) {
		player.lifePool.current = 0.1f;
	}
	
	RestoreAllLightsInitialStatus();

	if (stop_sound) ARX_SOUND_MixerStop(ARX_SOUND_MixerGame);

	RestoreInitialIOStatus();

	if (ed)
	{
		ARX_PATH_ComputeAllBoundingBoxes();

		arxtime.pause();
	}
	else
	{
		ARX_SCRIPT_ResetAll(true);
		EERIE_ANIMMANAGER_PurgeUnused();
	}
	
	if(!DONT_ERASE_PLAYER) {
		ARX_PLAYER_MakeFreshHero();
	}
	
}



long NO_GMOD_RESET=0;

static void FirstFrameProc() {
	
	if(!pParticleManager)
		pParticleManager = new ParticleManager();

	if(!NO_GMOD_RESET)
		ARX_GLOBALMODS_Reset();

	NO_GMOD_RESET = 0;
	
	STARTDRAG = Vec2s_ZERO;
	DANAEMouse = Vec2s_ZERO;
	bookclick = false;
	
	if(!LOAD_N_DONT_ERASE)
		arxtime.init();

	ARX_BOOMS_ClearAllPolyBooms();
	ARX_DAMAGES_Reset();
	ARX_MISSILES_ClearAll();
	spells.clearAll();
	ARX_SPELLS_ClearAllSymbolDraw();
	ARX_PARTICLES_ClearAll();

	if(!LOAD_N_DONT_ERASE) {
		CleanScriptLoadedIO();
		RestoreInitialIOStatus();
		DRAGINTER=NULL;
	}

	ARX_SPELLS_ResetRecognition();
	
	eyeball.exist=0;
	WILLADDSPEECHTIME=0;
	WILLADDSPEECH.clear();
	
	for(size_t i = 0; i < MAX_DYNLIGHTS; i++) {
		lightHandleGet((LightHandle)i)->exist = 0;
	}
	
	arxtime.update_last_frame_time();
	
	if(!LOAD_N_DONT_ERASE) {
		CleanInventory();
		ARX_SCRIPT_Timer_ClearAll();
		UnlinkAllLinkedObjects();
		ARX_SCRIPT_ResetAll(false);
	}

	SecondaryInventory=NULL;
	TSecondaryInventory=NULL;
	ARX_FOGS_Render();

	if(!LOAD_N_DONT_ERASE) {
		arxtime.init();

		if(!DONT_ERASE_PLAYER)
			ARX_PLAYER_InitPlayer();

		playerInterfaceFaderResetSlid();

		player.lifePool.current = player.lifePool.max;
		player.manaPool.current = player.manaPool.max;
		if(!DONT_ERASE_PLAYER) {
			ARX_PLAYER_MakeFreshHero();
		}
	}
	
	InitSnapShot(fs::paths.user / "snapshot");
}
Vec3f LastValidPlayerPos;
Vec3f	WILL_RESTORE_PLAYER_POSITION;
long WILL_RESTORE_PLAYER_POSITION_FLAG=0;

void FirstFrameHandling() {
	
	arx_assert(entities.player());
	
	LogDebug("FirstFrameHandling");
	Vec3f trans;
	FirstFrame = true;

	ARX_PARTICLES_FirstInit();
	RenderBatcher::getInstance().reset();
	
	progressBarAdvance(2.f);
	LoadLevelScreen();
	
	FirstFrameProc();
	
	if(FASTmse) {
		FASTmse = 0;
		if(LOADEDD) {
			trans = Mscenepos;
			player.pos = loddpos + trans;
		} else {
			player.pos.y += player.baseHeight();
		}
		progressBarAdvance(4.f);
		LoadLevelScreen();
	}
#if BUILD_EDIT_LOADSAVE
	else if(mse) {
		Mscenepos.x=-mse->cub.xmin-(mse->cub.xmax-mse->cub.xmin)*.5f+((float)ACTIVEBKG->Xsize*(float)ACTIVEBKG->Xdiv)*.5f;
		Mscenepos.z=-mse->cub.zmin-(mse->cub.zmax-mse->cub.zmin)*.5f+((float)ACTIVEBKG->Zsize*(float)ACTIVEBKG->Zdiv)*.5f;
		float t1=(float)(long)(mse->point0.x/BKG_SIZX);
		float t2=(float)(long)(mse->point0.z/BKG_SIZZ);
		t1=mse->point0.x-t1*BKG_SIZX;
		t2=mse->point0.z-t2*BKG_SIZZ;
		Mscenepos.x=(float)((long)(Mscenepos.x/BKG_SIZX))*BKG_SIZX+(float)BKG_SIZX*.5f;
		Mscenepos.z=(float)((long)(Mscenepos.z/BKG_SIZZ))*BKG_SIZZ+(float)BKG_SIZZ*.5f;
		mse->pos.x=Mscenepos.x=Mscenepos.x+BKG_SIZX-t1;
		mse->pos.z=Mscenepos.z=Mscenepos.z+BKG_SIZZ-t2;
		Mscenepos.y=mse->pos.y=-mse->cub.ymin-100.f-mse->point0.y;

		if (!NO_PLAYER_POSITION_RESET)
		{
			player.pos.x = mse->pos.x+mse->point0.x;
			player.pos.z = mse->pos.z+mse->point0.z;
			player.pos.y = mse->pos.y+mse->point0.y;
		}

		EERIERemovePrecalcLights();

		progressBarAdvance();
		LoadLevelScreen();

		SceneAddMultiScnToBackground(mse);

		progressBarAdvance(2.f);
		LoadLevelScreen();

		trans = mse->pos;

		ReleaseMultiScene(mse);
		mse=NULL;

		if(!NO_PLAYER_POSITION_RESET) {
			if(LOADEDD) {
				player.pos = loddpos + trans;
			} else {
				player.pos.y += player.baseHeight();
			}
		}

		NO_PLAYER_POSITION_RESET=0;

		progressBarAdvance();
		LoadLevelScreen();
	}
#endif // BUILD_EDIT_LOADSAVE
	else
	{
		progressBarAdvance(4.f);
		LoadLevelScreen();
	}

	if(player.torch) {
		ARX_SOUND_PlaySFX(SND_TORCH_LOOP, NULL, 1.0F, ARX_SOUND_PLAY_LOOPED);
	}
	
	MagicFlareSetCamera(&subj);
	
	lastteleport = player.basePosition();
	subj.orgTrans.pos = moveto = player.pos;

	subj.angle = player.angle;
	
	RestoreLastLoadedLightning(*ACTIVEBKG);

	progressBarAdvance();
	LoadLevelScreen();

	if(!LOAD_N_DONT_ERASE)
		SetEditMode(0);

	progressBarAdvance();
	LoadLevelScreen();

	LOAD_N_DONT_ERASE=0;
	DONT_ERASE_PLAYER=0;

	progressBarAdvance();
	LoadLevelScreen();

	FirstFrame=false;
	PrepareIOTreatZone(1);
	CURRENTLEVEL=GetLevelNumByName(LastLoadedScene.string());
	
	if(!NO_TIME_INIT)
		arxtime.init();
	
	arxtime.update_last_frame_time();
	
	progressBarAdvance();
	LoadLevelScreen();
	
	if(DONT_WANT_PLAYER_INZONE) {
		player.inzone = NULL;
		DONT_WANT_PLAYER_INZONE = 0;
	}
	
	progressBarAdvance();
	LoadLevelScreen();

	player.desiredangle.setYaw(0.f);
	player.angle.setYaw(0.f);
	ARX_PLAYER_RectifyPosition();

	entities.player()->_npcdata->vvpos = -99999;

	SendGameReadyMsg();
	PLAYER_MOUSELOOK_ON = false;
	player.Interface &= ~INTER_NOTE;

	if(NO_TIME_INIT) {
		arxtime.force_time_restore(FORCE_TIME_RESTORE);
		arxtime.force_frame_time_restore(FORCE_TIME_RESTORE);
	} else {
		arxtime.resume();
	}

	EntityHandle t = entities.getById("seat_stool1_0012");
	if(ValidIONum(t)) {
		entities[t]->ioflags |= IO_FORCEDRAW;
	}
	
	if(WILL_RESTORE_PLAYER_POSITION_FLAG) {
		Entity * io = entities.player();
		player.pos = WILL_RESTORE_PLAYER_POSITION;
		io->pos = player.basePosition();
		for(size_t i = 0; i < io->obj->vertexlist.size(); i++) {
			io->obj->vertexlist3[i].v = io->obj->vertexlist[i].v + io->pos;
		}
		WILL_RESTORE_PLAYER_POSITION_FLAG = 0;
	}
	
	ARX_NPC_RestoreCuts();
	
	ResetVVPos(entities.player());
	
	progressBarAdvance();
	LoadLevelScreen();
	LoadLevelScreen(-2);
	
	if (	(!CheckInPoly(player.pos))
		&&	(LastValidPlayerPos.x!=0.f)
		&&	(LastValidPlayerPos.y!=0.f)
		&&	(LastValidPlayerPos.z!=0.f)) {
		player.pos = LastValidPlayerPos;
	}

	LastValidPlayerPos = player.pos;
}

//*************************************************************************************

void ManageNONCombatModeAnimations() {
	arx_assert(entities.player());
	
	Entity *io = entities.player();

	ANIM_USE * useanim3=&io->animlayer[3];
	ANIM_HANDLE ** alist=io->anims;

	if(player.Current_Movement & (PLAYER_LEAN_LEFT | PLAYER_LEAN_RIGHT))
		return;

	if(player.equiped[EQUIP_SLOT_SHIELD] != PlayerEntityHandle && !BLOCK_PLAYER_CONTROLS) {
		if ( (useanim3->cur_anim==NULL)  ||
			( (useanim3->cur_anim!=alist[ANIM_SHIELD_CYCLE])
			&& (useanim3->cur_anim!=alist[ANIM_SHIELD_HIT])
			&& (useanim3->cur_anim!=alist[ANIM_SHIELD_START]) ) )
		{
			changeAnimation(io, 3, alist[ANIM_SHIELD_START]);
		} else if(useanim3->cur_anim==alist[ANIM_SHIELD_START] && (useanim3->flags & EA_ANIMEND)) {
			changeAnimation(io, 3, alist[ANIM_SHIELD_CYCLE], EA_LOOP);
		}
	} else {
		if(useanim3->cur_anim==alist[ANIM_SHIELD_CYCLE]) {
			changeAnimation(io, 3, alist[ANIM_SHIELD_END]);
		} else if(useanim3->cur_anim == alist[ANIM_SHIELD_END] && (useanim3->flags & EA_ANIMEND)) {
			useanim3->cur_anim=NULL;
		}
	}
}

static long Player_Arrow_Count() {
	
	long count = 0;
	
	arx_assert(player.bag >= 0);
	arx_assert(player.bag <= 3);
	
	for(size_t bag = 0; bag < size_t(player.bag); bag++)
	for(size_t y = 0; y < INVENTORY_Y; y++)
	for(size_t x = 0; x < INVENTORY_X; x++) {
		INVENTORY_SLOT & slot = inventory[bag][x][y];
		
		if(slot.io && slot.io->className() == "arrows" && slot.io->durability >= 1.f) {
			count += checked_range_cast<long>(slot.io->durability);
		}
	}
	
	return count;
}

static Entity * Player_Arrow_Count_Decrease() {
	
	Entity * io = NULL;
	
	for(size_t bag = 0; bag < size_t(player.bag); bag++)
	for(size_t y = 0; y < INVENTORY_Y; y++)
	for(size_t x = 0; x < INVENTORY_X; x++) {
		INVENTORY_SLOT & slot = inventory[bag][x][y];
		
		if(slot.io && slot.io->className() == "arrows" && slot.io->durability >= 1.f) {
			if(!io || io->durability > slot.io->durability)
				io = slot.io;
		}
	}
	
	return io;
}
float GLOBAL_SLOWDOWN=1.f;

static bool StrikeAimtime() {
	
	ARX_PLAYER_Remove_Invisibility();
	STRIKE_AIMTIME = float(arxtime) - (float)AimTime;
	STRIKE_AIMTIME = STRIKE_AIMTIME * (1.f+(1.f-GLOBAL_SLOWDOWN));

	if(STRIKE_AIMTIME > player.Full_AimTime)
		STRIKE_AIMTIME=1.f;
	else
		STRIKE_AIMTIME = (float)STRIKE_AIMTIME / (float)player.Full_AimTime;

	if(STRIKE_AIMTIME < 0.1f)
		STRIKE_AIMTIME = 0.1f;

	if(STRIKE_AIMTIME > 0.8f)
		return true;

	return false;
}

static void strikeSpeak(Entity * io) {
	
	if(!StrikeAimtime()) {
		return;
	}
	
	const std::string * str;
	EntityHandle equiped = player.equiped[EQUIP_SLOT_WEAPON];
	if(equiped != PlayerEntityHandle && !entities[equiped]->strikespeech.empty()) {
		str = &entities[equiped]->strikespeech;
	} else if(!io->strikespeech.empty()) {
		str = &io->strikespeech;
	} else {
		return;
	}
	
	ARX_SPEECH_AddSpeech(io, *str, ANIM_TALK_NEUTRAL, ARX_SPEECH_FLAG_NOTEXT);
}

void ManageCombatModeAnimations() {
	arx_assert(entities.player());
		
	Entity * const io = entities.player();
	
	ANIM_USE * useanim=&io->animlayer[1];

	ANIM_HANDLE ** alist=io->anims;
	WeaponType weapontype = ARX_EQUIPMENT_GetPlayerWeaponType();
	
	if(weapontype == WEAPON_BARE && LAST_WEAPON_TYPE != weapontype) {
		if(useanim->cur_anim != alist[ANIM_BARE_WAIT]) {
			changeAnimation(io, 1, alist[ANIM_BARE_WAIT]);
			AimTime = 0;
		}
	}
	
	switch(weapontype) {
		case WEAPON_BARE:	// BARE HANDS PLAYER MANAGEMENT

			if(useanim->cur_anim == alist[ANIM_BARE_WAIT]) {
				AimTime = 0;
				if(eeMousePressed1()) {
					changeAnimation(io, 1, alist[ANIM_BARE_STRIKE_LEFT_START + CurrFightPos * 3]);
					io->isHit = false;
				}
			}

			// Now go for strike cycle...
			for(long j = 0; j < 4; j++) {
				if(useanim->cur_anim == alist[ANIM_BARE_STRIKE_LEFT_START+j*3] && (useanim->flags & EA_ANIMEND)) {
					changeAnimation(io, 1, alist[ANIM_BARE_STRIKE_LEFT_CYCLE + j * 3], EA_LOOP);
					AimTime = (unsigned long)(arxtime);
				} else if(useanim->cur_anim == alist[ANIM_BARE_STRIKE_LEFT_CYCLE+j*3] && !eeMousePressed1()) {
					changeAnimation(io, 1, alist[ANIM_BARE_STRIKE_LEFT + j * 3]);
					strikeSpeak(io);
					SendIOScriptEvent(io, SM_STRIKE, "bare");
					PlayerWeaponBlocked = -1;
					CurrFightPos = 0;
					AimTime = 0;
				} else if(useanim->cur_anim == alist[ANIM_BARE_STRIKE_LEFT+j*3]) {
					if(useanim->flags & EA_ANIMEND) {
						changeAnimation(io, 1, alist[ANIM_BARE_WAIT], EA_LOOP);
						CurrFightPos = 0;
						AimTime = (unsigned long)(arxtime);
						PlayerWeaponBlocked = -1;
					} else if(useanim->ctime > useanim->cur_anim->anims[useanim->altidx_cur]->anim_time * 0.2f
							&& useanim->ctime < useanim->cur_anim->anims[useanim->altidx_cur]->anim_time * 0.8f
							&& PlayerWeaponBlocked == -1
					) {
						long id = -1;
						
						if(useanim->cur_anim == alist[ANIM_BARE_STRIKE_LEFT]) {
							id = io->obj->fastaccess.left_attach;
						} else { // Strike Right
							id = io->obj->fastaccess.primary_attach;
						}
						
						if(id != -1) {
							Sphere sphere;
							sphere.origin = io->obj->vertexlist3[id].v;
							sphere.radius = 25.f;
							
							EntityHandle num;
							
							if(CheckAnythingInSphere(sphere, PlayerEntityHandle, 0, &num)) {
								float dmgs = (player.m_miscFull.damages + 1) * STRIKE_AIMTIME;
								
								if(ARX_DAMAGES_TryToDoDamage(io->obj->vertexlist3[id].v, dmgs, 40, PlayerEntityHandle)) {
									PlayerWeaponBlocked = useanim->ctime;
								}
								
								ARX_PARTICLES_Spawn_Spark(sphere.origin, dmgs, 2);
								
								if(ValidIONum(num)) {
									ARX_SOUND_PlayCollision(entities[num]->material, MATERIAL_FLESH, 1.f, 1.f, sphere.origin, NULL);
								}
							}
						}
					}
				}
			}

		break;
		case WEAPON_DAGGER: // DAGGER PLAYER MANAGEMENT
			
			// Waiting and receiving Strike Impulse
			if(useanim->cur_anim == alist[ANIM_DAGGER_WAIT]) {
				AimTime = 0;
				if(eeMousePressed1()) {
					changeAnimation(io, 1, alist[ANIM_DAGGER_STRIKE_LEFT_START + CurrFightPos * 3]);
					io->isHit = false;
				}
			}

			// Now go for strike cycle...
			for(long j = 0; j < 4; j++) {
				if(useanim->cur_anim == alist[ANIM_DAGGER_STRIKE_LEFT_START+j*3] && (useanim->flags & EA_ANIMEND)) {
					changeAnimation(io, 1, alist[ANIM_DAGGER_STRIKE_LEFT_CYCLE + j * 3], EA_LOOP);
					AimTime = (unsigned long)(arxtime);
				} else if(useanim->cur_anim == alist[ANIM_DAGGER_STRIKE_LEFT_CYCLE+j*3] && !eeMousePressed1()) {
					changeAnimation(io, 1, alist[ANIM_DAGGER_STRIKE_LEFT + j * 3]);
					strikeSpeak(io);
					SendIOScriptEvent(io, SM_STRIKE, "dagger");
					CurrFightPos = 0;
					AimTime = 0;
				} else if(useanim->cur_anim == alist[ANIM_DAGGER_STRIKE_LEFT+j*3]) {
					if(useanim->ctime > useanim->cur_anim->anims[useanim->altidx_cur]->anim_time * 0.3f
						&& useanim->ctime < useanim->cur_anim->anims[useanim->altidx_cur]->anim_time * 0.7f)
					{
						Entity * weapon = entities[player.equiped[EQUIP_SLOT_WEAPON]];
						
						if(PlayerWeaponBlocked == -1
							&& ARX_EQUIPMENT_Strike_Check(io, weapon, STRIKE_AIMTIME, 0))
						{
							PlayerWeaponBlocked = useanim->ctime;
						}
					}

					if(useanim->flags & EA_ANIMEND) {
						changeAnimation(io, 1, alist[ANIM_DAGGER_WAIT], EA_LOOP);
						useanim->flags &= ~(EA_PAUSED | EA_REVERSE);
						CurrFightPos = 0;
						AimTime = (unsigned long)(arxtime);
						PlayerWeaponBlocked = -1;
					}

					if(PlayerWeaponBlocked != -1 && useanim->ctime < useanim->cur_anim->anims[useanim->altidx_cur]->anim_time * 0.9f) {
						Entity * weapon = entities[player.equiped[EQUIP_SLOT_WEAPON]];
						ARX_EQUIPMENT_Strike_Check(io, weapon, STRIKE_AIMTIME, 1);
					}
				}
			}

		break;
		case WEAPON_1H: // 1HANDED PLAYER MANAGEMENT
			
			// Waiting and Received Strike Impulse
			if(useanim->cur_anim == alist[ANIM_1H_WAIT]) {
				AimTime = 0;
				if(eeMousePressed1()) {
					changeAnimation(io, 1, alist[ANIM_1H_STRIKE_LEFT_START + CurrFightPos * 3]);
					io->isHit = false;
				}
			}

			// Now go for strike cycle...
			for(long j = 0; j < 4; j++) {
				if(useanim->cur_anim == alist[ANIM_1H_STRIKE_LEFT_START+j*3] && (useanim->flags & EA_ANIMEND)) {
					changeAnimation(io, 1, alist[ANIM_1H_STRIKE_LEFT_CYCLE + j * 3], EA_LOOP);
					AimTime = (unsigned long)(arxtime);
				} else if(useanim->cur_anim == alist[ANIM_1H_STRIKE_LEFT_CYCLE+j*3] && !eeMousePressed1()) {
					changeAnimation(io, 1, alist[ANIM_1H_STRIKE_LEFT + j * 3]);
					strikeSpeak(io);
					SendIOScriptEvent(io, SM_STRIKE, "1h");
					CurrFightPos = 0;
					AimTime = 0;
				} else if(useanim->cur_anim == alist[ANIM_1H_STRIKE_LEFT+j*3]) {
					if(useanim->ctime > useanim->cur_anim->anims[useanim->altidx_cur]->anim_time * 0.3f
						&& useanim->ctime < useanim->cur_anim->anims[useanim->altidx_cur]->anim_time * 0.7f)
					{
						Entity * weapon = entities[player.equiped[EQUIP_SLOT_WEAPON]];
						
						if(PlayerWeaponBlocked == -1
							&& ARX_EQUIPMENT_Strike_Check(io, weapon, STRIKE_AIMTIME, 0))
						{
							PlayerWeaponBlocked = useanim->ctime;
						}
					}

					if(useanim->flags & EA_ANIMEND) {
						changeAnimation(io, 1, alist[ANIM_1H_WAIT], EA_LOOP);
						useanim->flags &= ~(EA_PAUSED | EA_REVERSE);
						CurrFightPos = 0;
						AimTime = 0;
						PlayerWeaponBlocked = -1;
					}

					if(PlayerWeaponBlocked != -1 && useanim->ctime < useanim->cur_anim->anims[useanim->altidx_cur]->anim_time * 0.9f) {
						Entity * weapon = entities[player.equiped[EQUIP_SLOT_WEAPON]];
						ARX_EQUIPMENT_Strike_Check(io, weapon, STRIKE_AIMTIME, 1);
					}
				}
			}
		break;
		case WEAPON_2H: // 2HANDED PLAYER MANAGEMENT
			
			// Waiting and Receiving Strike Impulse
			if(useanim->cur_anim == alist[ANIM_2H_WAIT]) {
				AimTime = 0;
				if(eeMousePressed1()) {
					changeAnimation(io, 1, alist[ANIM_2H_STRIKE_LEFT_START + CurrFightPos * 3]);
					io->isHit = false;
				}
			}

			// Now go for strike cycle...
			for(long j = 0; j < 4; j++) {
				if(useanim->cur_anim == alist[ANIM_2H_STRIKE_LEFT_START+j*3] && (useanim->flags & EA_ANIMEND)) {
					changeAnimation(io, 1, alist[ANIM_2H_STRIKE_LEFT_CYCLE + j * 3], EA_LOOP);
					AimTime = (unsigned long)(arxtime);
				} else if(useanim->cur_anim == alist[ANIM_2H_STRIKE_LEFT_CYCLE+j*3] && !eeMousePressed1()) {
					changeAnimation(io, 1, alist[ANIM_2H_STRIKE_LEFT + j * 3]);
					strikeSpeak(io);
					SendIOScriptEvent(io, SM_STRIKE, "2h");
					CurrFightPos = 0;
					AimTime = 0;
				} else if(useanim->cur_anim == alist[ANIM_2H_STRIKE_LEFT+j*3]) {
					if(useanim->ctime > useanim->cur_anim->anims[useanim->altidx_cur]->anim_time * 0.3f
						&& useanim->ctime < useanim->cur_anim->anims[useanim->altidx_cur]->anim_time * 0.7f)
					{
						Entity * weapon = entities[player.equiped[EQUIP_SLOT_WEAPON]];
						
						if(PlayerWeaponBlocked == -1
							&& ARX_EQUIPMENT_Strike_Check(io, weapon, STRIKE_AIMTIME, 0))
						{
							PlayerWeaponBlocked = useanim->ctime;
						}
					}

					if(useanim->flags & EA_ANIMEND) {
						changeAnimation(io, 1, alist[ANIM_2H_WAIT], EA_LOOP);
						useanim->flags &= ~(EA_PAUSED | EA_REVERSE);
						CurrFightPos = 0;
						AimTime = 0;
						PlayerWeaponBlocked = -1;
					}

					if(PlayerWeaponBlocked != -1 && useanim->ctime < useanim->cur_anim->anims[useanim->altidx_cur]->anim_time * 0.9f) {
						Entity * weapon = entities[player.equiped[EQUIP_SLOT_WEAPON]];
						ARX_EQUIPMENT_Strike_Check(io, weapon, STRIKE_AIMTIME, 1);
					}
				}
			}
		break;
		case WEAPON_BOW: // MISSILE PLAYER MANAGEMENT
			if(useanim->cur_anim == alist[ANIM_MISSILE_STRIKE_CYCLE]) {
				if(GLOBAL_SLOWDOWN != 1.f)
					BOW_FOCAL += Original_framedelay;
				else
					BOW_FOCAL += framedelay;

				if(BOW_FOCAL > 710)
					BOW_FOCAL = 710;
			}

			// Waiting and Receiving Strike Impulse
			if(useanim->cur_anim == alist[ANIM_MISSILE_WAIT]) {
				AimTime = (unsigned long)(arxtime);

				if(eeMousePressed1() && Player_Arrow_Count() > 0) {
					changeAnimation(io, 1, alist[ANIM_MISSILE_STRIKE_PART_1]);
					io->isHit = false;
				}
			}

			if(useanim->cur_anim == alist[ANIM_MISSILE_STRIKE_PART_1] && (useanim->flags & EA_ANIMEND)) {
				AimTime = 0;
				changeAnimation(io, 1, alist[ANIM_MISSILE_STRIKE_PART_2]);
				EERIE_LINKEDOBJ_LinkObjectToObject(io->obj, arrowobj, "left_attach", "attach", NULL);
			}

			// Now go for strike cycle...
			if(useanim->cur_anim == alist[ANIM_MISSILE_STRIKE_PART_2] && (useanim->flags & EA_ANIMEND)) {
				changeAnimation(io, 1, alist[ANIM_MISSILE_STRIKE_CYCLE], EA_LOOP);
				AimTime = (unsigned long)(arxtime);
			} else if(useanim->cur_anim == alist[ANIM_MISSILE_STRIKE_CYCLE] && !eeMousePressed1()) {
				EERIE_LINKEDOBJ_UnLinkObjectFromObject(io->obj, arrowobj);
				changeAnimation(io, 1, alist[ANIM_MISSILE_STRIKE]);
				SendIOScriptEvent(io, SM_STRIKE, "bow");
				StrikeAimtime();
				STRIKE_AIMTIME = (float)(BOW_FOCAL)/710.f;
				Entity * quiver = Player_Arrow_Count_Decrease();
				float poisonous = 0.f;

				if(quiver) {
					poisonous = quiver->poisonous;
					if(quiver->poisonous_count > 0) {
						quiver->poisonous_count--;

						if(quiver->poisonous_count <= 0)
							quiver->poisonous = 0;
					}

					ARX_DAMAGES_DurabilityLoss(quiver, 1.f);

					// TODO is this needed ?, quivers seem to self destruct via script, but maybe not all
					if(ValidIOAddress(quiver) && quiver->durability <= 0.f) {
						ARX_INTERACTIVE_DestroyIOdelayed(quiver);
					}
				}

				float aimratio = STRIKE_AIMTIME;

				if(sp_max && poisonous < 3.f)
					poisonous = 3.f;

				Vec3f orgPos = player.pos + Vec3f(0.f, 40.f, 0.f);

				if(io->obj->fastaccess.left_attach >= 0) {
					orgPos = io->obj->vertexlist3[io->obj->fastaccess.left_attach].v;
				}

				Anglef orgAngle = player.angle;

				PlayerLaunchArrow_Test(aimratio, poisonous, orgPos, orgAngle);

				if(sp_max) {
					Anglef angle;
					Vec3f pos = player.pos + Vec3f(0.f, 40.f, 0.f);
					
					angle.setYaw(player.angle.getYaw());
					angle.setPitch(player.angle.getPitch() + 8);
					angle.setRoll(player.angle.getRoll());
					PlayerLaunchArrow_Test(aimratio, poisonous, pos, angle);
					angle.setYaw(player.angle.getYaw());
					angle.setPitch(player.angle.getPitch() - 8);
					PlayerLaunchArrow_Test(aimratio, poisonous, pos, angle);
					angle.setYaw(player.angle.getYaw());
					angle.setPitch(player.angle.getPitch() + 4.f);
					PlayerLaunchArrow_Test(aimratio, poisonous, pos, angle);
					angle.setYaw(player.angle.getYaw());
					angle.setPitch(player.angle.getPitch() - 4.f);
					PlayerLaunchArrow_Test(aimratio, poisonous, pos, angle);
				}

				AimTime = 0;
			} else if(useanim->cur_anim == alist[ANIM_MISSILE_STRIKE]) {
				BOW_FOCAL -= Original_framedelay;

				if(BOW_FOCAL < 0)
					BOW_FOCAL = 0;

				if(useanim->flags & EA_ANIMEND) {
					BOW_FOCAL = 0;
					changeAnimation(io, 1, alist[ANIM_MISSILE_WAIT], EA_LOOP);
					AimTime = 0;
					PlayerWeaponBlocked = -1;
					EERIE_LINKEDOBJ_UnLinkObjectFromObject(io->obj, arrowobj);
				}
			}
		break;
	}

	LAST_WEAPON_TYPE = weapontype;
}

void ManageCombatModeAnimationsEND() {
	
	Entity * io = entities.player();
	
	ANIM_USE * useanim = &io->animlayer[1];
	ANIM_USE * useanim3 = &io->animlayer[3];
	
	ANIM_HANDLE ** alist = io->anims;

	if(useanim->cur_anim
		&&(		(useanim->cur_anim == alist[ANIM_BARE_READY])
			||	(useanim->cur_anim == alist[ANIM_DAGGER_READY_PART_2])
			||	(useanim->cur_anim == alist[ANIM_DAGGER_READY_PART_1])
			||	(useanim->cur_anim == alist[ANIM_1H_READY_PART_2])
			||	(useanim->cur_anim == alist[ANIM_1H_READY_PART_1])
			||	(useanim->cur_anim == alist[ANIM_2H_READY_PART_2])
			||	(useanim->cur_anim == alist[ANIM_2H_READY_PART_1])
			||	(useanim->cur_anim == alist[ANIM_MISSILE_READY_PART_1])
			||	(useanim->cur_anim == alist[ANIM_MISSILE_READY_PART_2])	)
	) {
		AimTime = (unsigned long)(arxtime);
	}

	if(useanim->flags & EA_ANIMEND) {
		WeaponType weapontype = ARX_EQUIPMENT_GetPlayerWeaponType();

		if(useanim->cur_anim &&
			(	(useanim->cur_anim == io->anims[ANIM_BARE_UNREADY])
			||	(useanim->cur_anim == io->anims[ANIM_DAGGER_UNREADY_PART_2])
			||	(useanim->cur_anim == io->anims[ANIM_1H_UNREADY_PART_2])
			||	(useanim->cur_anim == io->anims[ANIM_2H_UNREADY_PART_2])
			||	(useanim->cur_anim == io->anims[ANIM_MISSILE_UNREADY_PART_2])	)
		) {
			io->_npcdata->ex_rotate->flags |= EXTRA_ROTATE_REALISTIC;
			AcquireLastAnim(io);
			useanim->cur_anim = NULL;
		}

		switch(weapontype) {
			case WEAPON_BARE: {
				// Is Weapon Ready ? In this case go to Fight Wait anim
				if(useanim->cur_anim == alist[ANIM_BARE_READY]) {
					if(player.Interface & INTER_NO_STRIKE) {
						player.Interface &= ~INTER_NO_STRIKE;
						changeAnimation(io, 1, alist[ANIM_BARE_WAIT], EA_LOOP);
					} else {
						changeAnimation(io, 1, alist[ANIM_BARE_STRIKE_LEFT_START + CurrFightPos * 3]);
					}
					AimTime = (unsigned long)(arxtime);
					io->isHit = false;
				}
				break;
			}
			case WEAPON_DAGGER: // DAGGER ANIMS end

				if(alist[ANIM_DAGGER_READY_PART_1]) {
					if(useanim->cur_anim == alist[ANIM_DAGGER_READY_PART_1]) {
						ARX_EQUIPMENT_AttachPlayerWeaponToHand();
						changeAnimation(io, 1, alist[ANIM_DAGGER_READY_PART_2]);
					} else if(useanim->cur_anim == alist[ANIM_DAGGER_READY_PART_2]) {
						if(player.Interface & INTER_NO_STRIKE) {
							player.Interface &= ~INTER_NO_STRIKE;
							changeAnimation(io, 1, alist[ANIM_DAGGER_WAIT], EA_LOOP);
						} else {
							changeAnimation(io, 1, alist[ANIM_DAGGER_STRIKE_LEFT_START + CurrFightPos * 3]);
						}
						AimTime = (unsigned long)(arxtime);
						io->isHit = false;
					} else if(useanim->cur_anim == alist[ANIM_DAGGER_UNREADY_PART_1]) {
						ARX_EQUIPMENT_AttachPlayerWeaponToBack();
						changeAnimation(io, 1, alist[ANIM_DAGGER_UNREADY_PART_2]);
					}
				}

			break;
			case WEAPON_1H:	// 1H ANIMS end

				if(alist[ANIM_1H_READY_PART_1]) {
					if(useanim->cur_anim == alist[ANIM_1H_READY_PART_1]) {
						ARX_EQUIPMENT_AttachPlayerWeaponToHand();
						changeAnimation(io, 1, alist[ANIM_1H_READY_PART_2]);
					} else if(useanim->cur_anim == alist[ANIM_1H_READY_PART_2]) {
						if(player.Interface & INTER_NO_STRIKE) {
							player.Interface &= ~INTER_NO_STRIKE;
							changeAnimation(io, 1, alist[ANIM_1H_WAIT], EA_LOOP);
						} else {
							changeAnimation(io, 1, alist[ANIM_1H_STRIKE_LEFT_START + CurrFightPos * 3]);
						}
						AimTime = (unsigned long)(arxtime);
						io->isHit = false;
					} else if (useanim->cur_anim == alist[ANIM_1H_UNREADY_PART_1]) {
						ARX_EQUIPMENT_AttachPlayerWeaponToBack();
						changeAnimation(io, 1, alist[ANIM_1H_UNREADY_PART_2]);
					}
				}

			break;
			case WEAPON_2H:	// 2H ANIMS end

				if(alist[ANIM_2H_READY_PART_1]) {
					if(useanim->cur_anim == alist[ANIM_2H_READY_PART_1]) {
						ARX_EQUIPMENT_AttachPlayerWeaponToHand();
						changeAnimation(io, 1, alist[ANIM_2H_READY_PART_2]);
					} else if(useanim->cur_anim == alist[ANIM_2H_READY_PART_2]) {
						if(player.Interface & INTER_NO_STRIKE) {
							player.Interface &= ~INTER_NO_STRIKE;
							changeAnimation(io, 1, alist[ANIM_2H_WAIT], EA_LOOP);
						} else {
							changeAnimation(io, 1, alist[ANIM_2H_STRIKE_LEFT_START + CurrFightPos * 3]);
						}
						AimTime = (unsigned long)(arxtime);
						io->isHit = false;
					} else if(useanim->cur_anim == alist[ANIM_2H_UNREADY_PART_1]) {
						ARX_EQUIPMENT_AttachPlayerWeaponToBack();
						changeAnimation(io, 1, alist[ANIM_2H_UNREADY_PART_2]);
					}
				}

			break;
			case WEAPON_BOW:// MISSILE Weapon ANIMS end

				if(alist[ANIM_MISSILE_READY_PART_1]) {
					if(useanim->cur_anim == alist[ANIM_MISSILE_READY_PART_1]) {
						ARX_EQUIPMENT_AttachPlayerWeaponToHand();
						changeAnimation(io, 1, alist[ANIM_MISSILE_READY_PART_2]);
					} else if(useanim->cur_anim == alist[ANIM_MISSILE_READY_PART_2]) {
						if(Player_Arrow_Count() > 0) {
							if(player.Interface & INTER_NO_STRIKE) {
								player.Interface &= ~INTER_NO_STRIKE;
								changeAnimation(io, 1, alist[ANIM_MISSILE_WAIT], EA_LOOP);
							} else {
								changeAnimation(io, 1, alist[ANIM_MISSILE_STRIKE_PART_1]);
							}
							io->isHit = false;
						} else {
							changeAnimation(io, 1, alist[ANIM_MISSILE_WAIT]);
						}
						EERIE_LINKEDOBJ_UnLinkObjectFromObject(io->obj, arrowobj);
					} else if(useanim->cur_anim == alist[ANIM_MISSILE_STRIKE_PART_1]) {
						// TODO why no AcquireLastAnim()?
						ANIM_Set(useanim, alist[ANIM_MISSILE_STRIKE_PART_2]);
					} else if(useanim->cur_anim == alist[ANIM_MISSILE_STRIKE_PART_2]) {
						// TODO why no AcquireLastAnim()?
						ANIM_Set(useanim, alist[ANIM_MISSILE_STRIKE_CYCLE]);
					} else if(useanim->cur_anim == alist[ANIM_MISSILE_UNREADY_PART_1]) {
						ARX_EQUIPMENT_AttachPlayerWeaponToBack();
						changeAnimation(io, 1, alist[ANIM_MISSILE_UNREADY_PART_2]);
					}
				}

			break;
		}

		// Spell casting anims
		if(alist[ANIM_CAST] && useanim->cur_anim == alist[ANIM_CAST]) {
			if(alist[ANIM_CAST_END]) {
				changeAnimation(io, 1, alist[ANIM_CAST_END]);
			}
		} else if(alist[ANIM_CAST_END] && useanim->cur_anim == alist[ANIM_CAST_END]) {
			AcquireLastAnim(io);
			useanim->cur_anim = NULL;
			player.doingmagic = 0;

			if(WILLRETURNTOCOMBATMODE) {
				player.Interface |= INTER_COMBATMODE;
				player.Interface |= INTER_NO_STRIKE;

				ARX_EQUIPMENT_LaunchPlayerReadyWeapon();
				WILLRETURNTOCOMBATMODE = false;
			}
		}
	}

	// Is the shield off ?
	if(useanim3->flags & EA_ANIMEND) {
		if(io->anims[ANIM_SHIELD_END] && useanim3->cur_anim == io->anims[ANIM_SHIELD_END]) {
			AcquireLastAnim(io);
			useanim3->cur_anim = NULL;
		}
	}
}

extern TextureContainer * ombrignon;

void DrawImproveVisionInterface() {

	if(ombrignon) {
		float mod = 0.6f + PULSATE * 0.35f;
		EERIEDrawBitmap(Rectf(g_size), 0.0001f, ombrignon, Color3f((0.5f+PULSATE*( 1.0f / 10 ))*mod,0.f,0.f).to<u8>());
	}
}

void DANAE_StartNewQuest()
{
	player.Interface = INTER_LIFE_MANA | INTER_MINIBACK | INTER_MINIBOOK;
	progressBarSetTotal(108);
	progressBarReset();
	LoadLevelScreen(1);
	DONT_ERASE_PLAYER=1;
	DanaeClearLevel();
	progressBarAdvance(2.f);
	LoadLevelScreen();
	DanaeLoadLevel("graph/levels/level1/level1.dlf");
	FirstFrame=true;
	START_NEW_QUEST=0;
	BLOCK_PLAYER_CONTROLS = false;
	fadeReset();
	player.Interface = INTER_LIFE_MANA | INTER_MINIBACK | INTER_MINIBOOK;
}

void ARX_SetAntiAliasing() {
	bool enabled = config.video.antialiasing && mainApp->getWindow()->getMSAALevel() > 0;
	GRenderer->SetAntialiasing(enabled);
}
