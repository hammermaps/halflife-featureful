/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/

//=========================================================
// Generic Monster - purely for scripted sequence work.
//=========================================================
#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"schedule.h"
#include	"player.h"
#include	"effects.h"
#include	"mod_features.h"
#include	"game.h"

#if FEATURE_GENEWORM

#define GENEWORM_ATTN 0.1f

#define GENEWORM_LEVEL0					0
#define GENEWORM_LEVEL1					1

#define GENEWORM_LEVEL0_HEIGHT			244
#define GENEWORM_LEVEL1_HEIGHT			304


#define GENEWORM_SKIN_EYE_OPEN			0
#define GENEWORM_SKIN_EYE_LEFT			1
#define GENEWORM_SKIN_EYE_RIGHT			2
#define GENEWORM_SKIN_EYE_CLOSED		3

#define GENEWORM_ATTACHMENT_MOUTH 0
#define GENEWORM_ATTACHMENT_SPAWN 1
#define GENEWORM_ATTACHMENT_RIGHTEYE 1
#define GENEWORM_ATTACHMENT_LEFTEYE 1

//=========================================================
// Monster's Anim Events Go Here
//=========================================================

#define GENEWORM_AE_BEAM			( 0 )		// Toxic beam attack (sprite trail)
#define GENEWORM_AE_PORTAL			( 2 )		// Create a portal that spawns an enemy.

#define GENEWORM_AE_MELEE_LEFT1		( 3 )		// Play hit sound
#define GENEWORM_AE_MELEE_LEFT2		( 4 )		// Activates the trigger_geneworm_hit

#define GENEWORM_AE_MELEE_RIGHT1	( 5 )		// Play hit sound
#define GENEWORM_AE_MELEE_RIGHT2	( 6 )		// Activates the trigger_geneworm_hit

#define GENEWORM_AE_MELEE_FORWARD1  ( 7 )		// Play hit sound
#define GENEWORM_AE_MELEE_FORWARD2  ( 8 )		// Activates the trigger_geneworm_hit

#define GENEWORM_AE_MAD				( 9 )		// Room starts shaking!

#define GENEWORM_AE_EYEPAIN			( 1012 )	// Still put here (In case we need to toggle eye pain status)


//=========================================================
// CGeneWormCloud
//=========================================================

class CGeneWormCloud : public CBaseEntity
{
public:
	void Spawn();
	void Precache();
	void TurnOn();
	void RunGeneWormCloud(float frames);
	static CGeneWormCloud *LaunchCloud(const Vector origin, const Vector aim, float velocity, edict_t *pOwner, float fadeTime, EntityOverrides entityOverrides);
	void CreateCloud(const Vector origin);

	virtual int Save(CSave &save);
	virtual int Restore(CRestore &restore);
	static TYPEDESCRIPTION m_SaveData[];

	void EXPORT GeneWormCloudThink();
	void EXPORT CloudTouch( CBaseEntity *pOther );

	int m_maxFrame;

	float m_fadeScale;
	float m_fadeRender;
	float m_lastTime;
	float m_birthtime;

	bool m_bLaunched;
};


LINK_ENTITY_TO_CLASS(env_genewormcloud, CGeneWormCloud)

TYPEDESCRIPTION CGeneWormCloud::m_SaveData[] =
{
	DEFINE_FIELD(CGeneWormCloud, m_maxFrame, FIELD_INTEGER),
	DEFINE_FIELD(CGeneWormCloud, m_fadeScale, FIELD_FLOAT),
	DEFINE_FIELD(CGeneWormCloud, m_fadeRender, FIELD_FLOAT),
	DEFINE_FIELD(CGeneWormCloud, m_birthtime, FIELD_TIME),
	DEFINE_FIELD(CGeneWormCloud, m_lastTime, FIELD_TIME),
	DEFINE_FIELD(CGeneWormCloud, m_bLaunched, FIELD_BOOLEAN),
};

IMPLEMENT_SAVERESTORE(CGeneWormCloud, CBaseEntity)

void CGeneWormCloud::Precache()
{
	PRECACHE_MODEL("sprites/ballsmoke.spr");
}

void CGeneWormCloud::Spawn()
{
	Precache();

	pev->solid = SOLID_BBOX;
	pev->effects = 0;
	pev->frame = 0;

	UTIL_SetSize(pev, Vector(0,0,0), Vector(0,0,0));
	SET_MODEL(ENT(pev), "sprites/ballsmoke.spr");

	m_maxFrame = MODEL_FRAMES(pev->modelindex)-1;
	pev->scale = 0.01;
	m_lastTime = gpGlobals->time;
	m_birthtime = gpGlobals->time;

	m_fadeScale = 0;
	m_bLaunched = false;

	SetTouch(&CGeneWormCloud::CloudTouch);
	SetThink(&CGeneWormCloud::GeneWormCloudThink);

	pev->nextthink = gpGlobals->time;
}

void CGeneWormCloud::CloudTouch(CBaseEntity *pOther)
{
	if (ENT(pOther->pev) == pev->owner || (ENT(pOther->pev) == VARS(pev->owner)->owner))
		return;

	if(pOther->pev->takedamage)
		pOther->TakeDamage(pev, pev, 5, DMG_ACID);

	pev->nextthink = gpGlobals->time;
	SetThink(NULL);
	UTIL_Remove(this);
}


void CGeneWormCloud::TurnOn()
{
	pev->effects = 0;

	if(pev->framerate != 0 && m_maxFrame > 1 && pev->spawnflags & FL_SWIM)
	{
		m_lastTime = gpGlobals->time;
		pev->nextthink = gpGlobals->time;
		pev->frame = 0;
	}
	else
		pev->frame = 0;
}

void CGeneWormCloud::RunGeneWormCloud(float frames)
{
	if(m_bLaunched)
	{
		pev->renderamt -= m_fadeRender;

		if(pev->scale >= 4.5)
		{
			pev->scale = 0;
			UTIL_Remove(this);
			return;
		}
		else
			pev->scale += 0.1;

		pev->frame += frames;

		if(pev->frame > m_maxFrame && m_maxFrame > 0)
		{
			pev->frame = m_maxFrame;
		}
	}
}

void CGeneWormCloud::GeneWormCloudThink()
{
	TraceResult tr;

	float frames = (gpGlobals->time - m_lastTime) * pev->framerate;

	UTIL_TraceHull(pev->origin, pev->origin, ignore_monsters, point_hull, ENT(pev), &tr);

	if(tr.fAllSolid && tr.fStartSolid && gpGlobals->time - m_birthtime >= 0.75)
	{
		UTIL_Remove(this);
	}

	RunGeneWormCloud(frames);

	pev->nextthink = gpGlobals->time + 0.1;
	m_lastTime = gpGlobals->time;
}

CGeneWormCloud *CGeneWormCloud::LaunchCloud(const Vector origin, const Vector aim, float velocity, edict_t *pOwner, float fadeTime, EntityOverrides entityOverrides)
{
	CGeneWormCloud *pCloud = GetClassPtr((CGeneWormCloud*)NULL);
	UTIL_SetOrigin(pCloud->pev, origin);
	pCloud->AssignEntityOverrides(entityOverrides);
	pCloud->Spawn();

	UTIL_MakeVectors(aim);

	pCloud->pev->owner = pOwner;

	pCloud->pev->skin = 0;
	pCloud->pev->body = 0;
	pCloud->pev->aiment = 0;
	pCloud->pev->movetype = MOVETYPE_NOCLIP;

	pCloud->pev->velocity = aim * velocity;
	pCloud->m_fadeScale = 2.5 / fadeTime;
	pCloud->m_fadeRender = (pCloud->pev->renderamt - 7) / fadeTime;

	pCloud->m_bLaunched = true;

	return pCloud;
}

void CGeneWormCloud::CreateCloud(const Vector origin)
{
	UTIL_SetOrigin(pev, origin);

	SetThink(&CGeneWormCloud::GeneWormCloudThink);
	SetTouch(&CGeneWormCloud::CloudTouch);
}

//========================================================
// CGeneWormSpawn
//========================================================

class CGeneWormSpawn : public CBaseEntity
{
public:

	void Spawn();
	void Precache();
	virtual int Save(CSave &save);
	virtual int Restore(CRestore &restore);
	static CGeneWormSpawn *LaunchSpawn(Vector origin, Vector aim, float speed, edict_t *pOwner, EntityOverrides entityOverrides);

	void EXPORT SpawnTouch(CBaseEntity *pOther) { }
	void EXPORT SpawnThink();

	static TYPEDESCRIPTION m_SaveData[];
	float m_flBirthTime;
	int m_maxFrame;
	bool m_bTrooperDropped;

	static const NamedSoundScript spawnSoundScript;
};

LINK_ENTITY_TO_CLASS(env_genewormspawn, CGeneWormSpawn)

TYPEDESCRIPTION CGeneWormSpawn::m_SaveData[] =
{
	DEFINE_FIELD(CGeneWormSpawn, m_flBirthTime, FIELD_TIME),
	DEFINE_FIELD(CGeneWormSpawn, m_bTrooperDropped, FIELD_BOOLEAN),
	DEFINE_FIELD(CGeneWormSpawn, m_maxFrame, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CGeneWormSpawn, CBaseEntity)

const NamedSoundScript CGeneWormSpawn::spawnSoundScript = {
	CHAN_WEAPON,
	{"debris/beamstart2.wav"},
	VOL_NORM,
	ATTN_NORM,
	"GeneWorm.Spawn"
};

void CGeneWormSpawn::Precache()
{
	PRECACHE_MODEL("sprites/tele1.spr");
	PRECACHE_MODEL("sprites/boss_glow.spr");

	RegisterAndPrecacheSoundScript(spawnSoundScript);
}

void CGeneWormSpawn::Spawn()
{
	Precache();
	pev->classname = MAKE_STRING("env_genewormspawn");

	pev->solid = SOLID_BBOX;
	pev->movetype = MOVETYPE_NOCLIP;
	pev->effects = 0;
	pev->frame = 0;
	pev->scale = 1.5;
	pev->rendermode = kRenderTransAdd;
	pev->renderamt = 255;

	SET_MODEL(ENT(pev), "sprites/tele1.spr");
	UTIL_SetSize(pev, g_vecZero, g_vecZero);

	m_maxFrame = g_engfuncs.pfnModelFrames(pev->modelindex) - 1;

	if(pev->angles.y != 0 && pev->angles.z == 0)
	{
		pev->angles.z = pev->angles.y;
		pev->angles.y = 0;
	}

	m_bTrooperDropped = false;
	m_flBirthTime = gpGlobals->time;

	SetTouch(&CGeneWormSpawn::SpawnTouch);
	SetThink(&CGeneWormSpawn::SpawnThink);

	pev->nextthink = gpGlobals->time + 0.1;
}

void CGeneWormSpawn::SpawnThink()
{
	CBaseEntity *player;

	pev->effects = 0;
	pev->framerate = 10;

	if((pev->origin - pev->owner->v.origin).Length() > 1050)
	{
		pev->velocity = g_vecZero;

		if(gpGlobals->time - m_flBirthTime >= 2.5)
		{
			if(!m_bTrooperDropped)
			{
				EmitSoundScript(spawnSoundScript);
				CBaseEntity *pEntity = CreateNoSpawn("monster_shocktrooper", pev->origin, pev->angles, ENT(pev));
				if (pEntity)
				{
					CBaseEntity* pOwner = CBaseEntity::Instance(pev->owner);
					if (pev->owner != 0 && pOwner)
					{
						CBaseMonster* geneworm = pOwner->MyMonsterPointer();
						if (geneworm)
						{
							CBaseMonster* strooper = pEntity->MyMonsterPointer();
							if (strooper)
							{
								strooper->m_iClass = geneworm->m_iClass;
								strooper->m_reverseRelationship = geneworm->m_reverseRelationship;
							}
						}
					}
					DispatchSpawn(pEntity->edict());
				}
				m_bTrooperDropped = true;
			}
			else
				pev->scale -= 0.25;
		}
	}

	if(pev->scale <= 0)
		UTIL_Remove(this);

	if(pev->frame >= m_maxFrame)
		pev->frame = 0;
	else
		pev->frame++;

	if(pev->scale < 3.2)
		pev->scale += 0.1;

	if((player = UTIL_FindEntityByClassname(0, "player")))
	{
		if((player->pev->origin - pev->origin).Length() <= 96)
			player->TakeDamage(VARS(player->pev), VARS(player->pev), 5, DMG_SHOCK);
	}

	pev->nextthink = gpGlobals->time + 0.1;
}


CGeneWormSpawn *CGeneWormSpawn::LaunchSpawn(Vector origin, Vector aim, float speed, edict_t *pOwner, EntityOverrides entityOverrides)
{
	CGeneWormSpawn *WormSpawn = GetClassPtr((CGeneWormSpawn*)NULL);

	UTIL_MakeVectors(aim);
	UTIL_SetOrigin(WormSpawn->pev, origin);

	WormSpawn->Spawn();
	WormSpawn->pev->velocity = aim * speed;
	WormSpawn->pev->owner = pOwner;


	return WormSpawn;
}

//=========================================================
// CGeneWorm
//=========================================================
class CGeneWorm : public CBaseMonster
{
public:

	int		Save(CSave &save);
	int		Restore(CRestore &restore);
	static	TYPEDESCRIPTION m_SaveData[];

	void Spawn(void);
	void Precache(void);
	bool IsEnabledInMod() { return g_modFeatures.IsMonsterEnabled("geneworm"); }
	int  DefaultClassify(void) { return CLASS_RACEX_SHOCK; }
	void Killed(entvars_t *pevInflictor, entvars_t *pevAttacker, int iGib);
	void TraceAttack(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);
	void FireHurtTargets(const char *targetName, CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value = 0.0f);

	void SetObjectCollisionBox(void)
	{
		pev->absmin = pev->origin + Vector( -437, -720, -332 );
		pev->absmax = pev->origin + Vector( 425, 164, 355 );
	}

	void HandleAnimEvent(MonsterEvent_t *pEvent);

	void EXPORT StartThink(void);
	void EXPORT HuntThink(void);
	void EXPORT CrashTouch(CBaseEntity *pOther);
	void EXPORT DyingThink(void);
	void EXPORT NullThink(void);
	void EXPORT CommandUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void EXPORT HitTouch( CBaseEntity *pOther );

	void NextActivity(void);

	void TrackHead();

	int  TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);

	void IdleSound(void);

	bool ClawAttack();

	Vector m_posTarget;

	CGeneWormCloud *m_pCloud;
	CGeneWormSpawn *m_orificeGlow;
	CSprite *m_pBall;

	float m_flLastSeen;
	float m_flOrificeOpenTime;
	float m_flSpitStartTime;
	float m_flMadDelayTime;
	float m_flTakeHitTime;
	float m_flHitTime;
	float m_flNextMeleeTime;
	float m_flNextRangeTime;
	float m_flDeathStart;

	bool m_fActivated;
	bool m_fRightEyeHit;
	bool m_fLeftEyeHit;
	bool m_fGetMad;
	bool m_fOrificeHit;
	bool m_fSpiting;
	bool m_fSpawningTrooper;
	bool m_fHasEntered;

	int m_iHitTimes;
	int m_iMaxHitTimes;
	int m_iWasHit;

	static const NamedSoundScript idleSoundScript;
	static const NamedSoundScript dieSoundScript;

	static const NamedSoundScript attack1SoundScript;
	static const NamedSoundScript attack2SoundScript;
	static const NamedSoundScript attack3SoundScript;
	static const NamedSoundScript beamAttackSoundScript;

	static const NamedSoundScript entrySoundScript;
	static const NamedSoundScript bigPain1SoundScript;
	static const NamedSoundScript bigPain2SoundScript;
	static const NamedSoundScript bigPain3SoundScript;
	static const NamedSoundScript bigPain4SoundScript;
	static const NamedSoundScript eyePainSoundScript;

	static const NamedSoundScript launchSpawnSoundScript;
};

LINK_ENTITY_TO_CLASS(monster_geneworm, CGeneWorm)

TYPEDESCRIPTION CGeneWorm::m_SaveData[] =
{
	DEFINE_FIELD(CGeneWorm, m_posTarget, FIELD_POSITION_VECTOR),
	DEFINE_FIELD(CGeneWorm, m_flLastSeen, FIELD_TIME),
	DEFINE_FIELD(CGeneWorm, m_pCloud, FIELD_CLASSPTR),
	DEFINE_FIELD(CGeneWorm, m_orificeGlow, FIELD_CLASSPTR),
	DEFINE_FIELD(CGeneWorm, m_pBall, FIELD_CLASSPTR),
	DEFINE_FIELD(CGeneWorm, m_iHitTimes, FIELD_INTEGER),
	DEFINE_FIELD(CGeneWorm, m_iMaxHitTimes, FIELD_INTEGER),
	DEFINE_FIELD(CGeneWorm, m_iWasHit, FIELD_INTEGER),
	DEFINE_FIELD(CGeneWorm, m_flNextMeleeTime, FIELD_TIME),
	DEFINE_FIELD(CGeneWorm, m_flNextRangeTime, FIELD_TIME),
	DEFINE_FIELD(CGeneWorm, m_fRightEyeHit, FIELD_BOOLEAN),
	DEFINE_FIELD(CGeneWorm, m_fLeftEyeHit, FIELD_BOOLEAN),
	DEFINE_FIELD(CGeneWorm, m_fOrificeHit, FIELD_BOOLEAN),
	DEFINE_FIELD(CGeneWorm, m_flOrificeOpenTime, FIELD_TIME),
	DEFINE_FIELD(CGeneWorm, m_fSpawningTrooper, FIELD_BOOLEAN),
	DEFINE_FIELD(CGeneWorm, m_fActivated, FIELD_BOOLEAN),
	DEFINE_FIELD(CGeneWorm, m_flDeathStart, FIELD_TIME),
	DEFINE_FIELD(CGeneWorm, m_fHasEntered, FIELD_BOOLEAN),
	DEFINE_FIELD(CGeneWorm, m_flMadDelayTime, FIELD_TIME),
	DEFINE_FIELD(CGeneWorm, m_fGetMad, FIELD_BOOLEAN),
	DEFINE_FIELD(CGeneWorm, m_fSpiting, FIELD_BOOLEAN),
	DEFINE_FIELD(CGeneWorm, m_flSpitStartTime, FIELD_TIME),
};
IMPLEMENT_SAVERESTORE(CGeneWorm, CBaseMonster)

//=========================================================
//=========================================================

const NamedSoundScript CGeneWorm::idleSoundScript = {
	CHAN_BODY,
	{"geneworm/geneworm_idle1.wav", "geneworm/geneworm_idle2.wav", "geneworm/geneworm_idle3.wav", "geneworm/geneworm_idle4.wav"},
	VOL_NORM,
	GENEWORM_ATTN,
	IntRange(95, 105),
	"GeneWorm.Idle"
};

const NamedSoundScript CGeneWorm::dieSoundScript = {
	CHAN_VOICE,
	{"geneworm/geneworm_death.wav"},
	VOL_NORM,
	GENEWORM_ATTN,
	"GeneWorm.Die"
};

const NamedSoundScript CGeneWorm::attack1SoundScript = {
	CHAN_VOICE,
	{"geneworm/geneworm_attack_mounted_rocket.wav"},
	VOL_NORM,
	GENEWORM_ATTN,
	"GeneWorm.Attack1"
};

const NamedSoundScript CGeneWorm::attack2SoundScript = {
	CHAN_VOICE,
	{"geneworm/geneworm_attack_mounted_gun.wav"},
	VOL_NORM,
	GENEWORM_ATTN,
	"GeneWorm.Attack2"
};

const NamedSoundScript CGeneWorm::attack3SoundScript = {
	CHAN_VOICE,
	{"geneworm/geneworm_big_attack_forward.wav"},
	VOL_NORM,
	GENEWORM_ATTN,
	"GeneWorm.Attack3"
};

const NamedSoundScript CGeneWorm::beamAttackSoundScript = {
	CHAN_VOICE,
	{"geneworm/geneworm_beam_attack.wav"},
	VOL_NORM,
	GENEWORM_ATTN,
	"GeneWorm.BeamAttack"
};

const NamedSoundScript CGeneWorm::entrySoundScript = {
	CHAN_VOICE,
	{"geneworm/geneworm_entry.wav"},
	VOL_NORM,
	GENEWORM_ATTN,
	"GeneWorm.Entry"
};

const NamedSoundScript CGeneWorm::bigPain1SoundScript = {
	CHAN_VOICE,
	{"geneworm/geneworm_final_pain1.wav"},
	VOL_NORM,
	GENEWORM_ATTN,
	"GeneWorm.BigPain1"
};

const NamedSoundScript CGeneWorm::bigPain2SoundScript = {
	CHAN_VOICE,
	{"geneworm/geneworm_final_pain2.wav"},
	VOL_NORM,
	GENEWORM_ATTN,
	"GeneWorm.BigPain2"
};

const NamedSoundScript CGeneWorm::bigPain3SoundScript = {
	CHAN_VOICE,
	{"geneworm/geneworm_final_pain3.wav"},
	VOL_NORM,
	GENEWORM_ATTN,
	"GeneWorm.BigPain1"
};

const NamedSoundScript CGeneWorm::bigPain4SoundScript = {
	CHAN_VOICE,
	{"geneworm/geneworm_final_pain4.wav"},
	VOL_NORM,
	GENEWORM_ATTN,
	"GeneWorm.BigPain4"
};

const NamedSoundScript CGeneWorm::eyePainSoundScript = {
	CHAN_VOICE,
	{"geneworm/geneworm_shot_in_eye.wav"},
	VOL_NORM,
	GENEWORM_ATTN,
	"GeneWorm.EyePain"
};

const NamedSoundScript CGeneWorm::launchSpawnSoundScript = {
	CHAN_WEAPON,
	{"debris/beamstart7.wav"},
	VOL_NORM,
	GENEWORM_ATTN,
	IntRange(95, 105),
	"GeneWorm.LaunchSpawn"
};

//=========================================================
// Spawn
//=========================================================
void CGeneWorm::Spawn()
{
	Precache();
	// motor
	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_NOT;


	SET_MODEL(ENT(pev), "models/geneworm.mdl");
	UTIL_SetSize(pev, Vector( -437, -720, -332 ), Vector( 425, 164, 355 ));
	UTIL_SetOrigin(pev, pev->origin);

	pev->flags |= FL_MONSTER;
	pev->takedamage = DAMAGE_AIM;
	SetMyHealth( gSkillData.gwormHealth );
	pev->view_ofs = Vector(0, 0, 192);

	pev->sequence = 0;
	ResetSequenceInfo();

	InitBoneControllers();

	pev->deadflag = DEAD_NO;
	pev->renderamt = 0;
	pev->rendermode = kRenderTransTexture;
	pev->effects = 0;

	SetMyBloodColor( BLOOD_COLOR_GREEN );

	m_iHitTimes = 0;
	m_iMaxHitTimes = 5;
	m_MonsterState = MONSTERSTATE_IDLE;

	m_flSpitStartTime = gpGlobals->time;
	m_flOrificeOpenTime = gpGlobals->time;
	m_flMadDelayTime = gpGlobals->time;
	m_flTakeHitTime = 0;
	m_flHitTime = 0;

	m_iWasHit = 0;
	m_fRightEyeHit = false;
	m_fLeftEyeHit = false;
	m_fGetMad = false;
	m_fOrificeHit = false;
	m_fActivated = false;
	m_fHasEntered = false;
	m_pCloud = NULL;

	UTIL_SetOrigin(pev, pev->origin);

	SetThink(&CGeneWorm::StartThink);

	pev->nextthink = gpGlobals->time + 0.1;
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CGeneWorm::Precache()
{
	PRECACHE_MODEL("models/geneworm.mdl");

	RegisterAndPrecacheSoundScript(idleSoundScript);
	RegisterAndPrecacheSoundScript(dieSoundScript);

	RegisterAndPrecacheSoundScript(attack1SoundScript);
	RegisterAndPrecacheSoundScript(attack2SoundScript);
	RegisterAndPrecacheSoundScript(attack3SoundScript);
	RegisterAndPrecacheSoundScript(beamAttackSoundScript);

	RegisterAndPrecacheSoundScript(entrySoundScript);
	RegisterAndPrecacheSoundScript(bigPain1SoundScript);
	RegisterAndPrecacheSoundScript(bigPain2SoundScript);
	RegisterAndPrecacheSoundScript(bigPain3SoundScript);
	RegisterAndPrecacheSoundScript(bigPain4SoundScript);
	RegisterAndPrecacheSoundScript(eyePainSoundScript);

	RegisterAndPrecacheSoundScript(launchSpawnSoundScript);

	PRECACHE_MODEL("sprites/tele1.spr");
	PRECACHE_MODEL("sprites/boss_glow.spr");
	UTIL_PrecacheOther("monster_shocktrooper");
	UTIL_PrecacheOther("env_genewormcloud", GetProjectileOverrides());
	UTIL_PrecacheOther("env_genewormspawn", GetProjectileOverrides());
}

//=========================================================
//
//=========================================================
void CGeneWorm::StartThink(void)
{
	Vector vecEyePos, vecEyeAng;

	GetAttachment(0, vecEyePos, vecEyeAng);

	pev->view_ofs = vecEyePos - pev->origin;

	pev->frame = 0;
	pev->sequence = LookupSequence("entry");
	ResetSequenceInfo();
	m_flNextMeleeTime = gpGlobals->time;
	m_flNextRangeTime = gpGlobals->time;

	SetUse(&CGeneWorm::CommandUse);
	SetTouch(&CGeneWorm::HitTouch);
	SetThink(&CGeneWorm::HuntThink);

	pev->nextthink = gpGlobals->time + 0.1;
}


void CGeneWorm::HitTouch( CBaseEntity *pOther )
{
	TraceResult tr = UTIL_GetGlobalTrace();

	if( pOther->pev->modelindex == pev->modelindex )
		return;

	if( m_flHitTime > gpGlobals->time )
		return;

	// only look at the ones where the player hit me
	if( tr.pHit == NULL || tr.pHit->v.modelindex != pev->modelindex )
		return;

	if( tr.iHitgroup == 0 )
	{
		pOther->TakeDamage( pev, pev, 20, DMG_CRUSH );
	}
}


void CGeneWorm::Killed(entvars_t *pevInflictor, entvars_t *pevAttacker, int iGib)
{
	CBaseMonster::Killed(pevInflictor, pevAttacker, iGib);
}

void CGeneWorm::DyingThink(void)
{
	pev->nextthink = gpGlobals->time + 0.1;
	GlowShellUpdate();

	DispatchAnimEvents();
	StudioFrameAdvance();

	if(pev->deadflag == DEAD_DYING)
	{
		pev->renderamt--;

		if( pev->renderamt == 0 )
			UTIL_Remove( this );

		if(gpGlobals->time - m_flDeathStart >= 15)
		{
			CBaseEntity* player = UTIL_FindEntityByClassname(0, "player");
			if (player)
			{
				CBaseEntity* teleport = UTIL_FindEntityByTargetname(0, "GeneWormTeleport");
				if (teleport)
					teleport->Touch(player);
				FireTargets("GeneWormTeleport", player, player, USE_ON, 1.0);
			}
			m_flDeathStart = gpGlobals->time + 999;
		}
	}

	if(pev->deadflag == DEAD_NO)
	{
		pev->frame = 0;
		pev->sequence = LookupSequence("death");

		ResetSequenceInfo();

		pev->renderfx = 0;
		pev->rendermode = kRenderTransTexture;
		pev->renderamt = 255;
		pev->solid = SOLID_NOT;

		EmitSoundScript(dieSoundScript);

		FireTargets("GeneWormDead", this, this);

		m_flDeathStart = gpGlobals->time;
		pev->deadflag = DEAD_DYING;

		if(m_pCloud)
		{
			UTIL_Remove(m_pCloud);
			m_pCloud = NULL;
		}

		if(m_orificeGlow)
		{
			UTIL_Remove(m_orificeGlow);
			m_orificeGlow = NULL;
		}

		if(m_pBall)
		{
			UTIL_Remove(m_pBall);
			m_pBall = NULL;
		}
		CBaseEntity* entity = NULL;
		while((entity = UTIL_FindEntityByClassname(entity, "monster_shocktrooper")) != NULL)
			entity->SUB_StartFadeOut();
	}
}
//=========================================================
//=========================================================

void CGeneWorm::FireHurtTargets(const char *targetName, CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	edict_t *pentTarget = NULL;

	if( !targetName )
		return;

	ALERT( at_aiconsole, "Firing: (%s)\n", targetName );

	for( ; ; )
	{
		pentTarget = FIND_ENTITY_BY_TARGETNAME( pentTarget, targetName );
		if( FNullEnt( pentTarget ) )
			break;

		CBaseEntity *pTarget = CBaseEntity::Instance( pentTarget );
		if( pTarget && !( pTarget->pev->flags & FL_NOTARGET ) )
		{
			ALERT( at_aiconsole, "Found: %s, firing (%s)\n", STRING( pTarget->pev->classname ), targetName );
			pTarget->Use( pActivator, pCaller, useType, value );
		}
	}
}

//=========================================================
//=========================================================
void CGeneWorm::NextActivity(void)
{
	if (m_flLastSeen + 15 < gpGlobals->time)
	{
		if (m_hEnemy != 0)
		{
			if ((pev->origin - m_hEnemy->pev->origin).Length() > 700)
				m_hEnemy = NULL;
		}
	}

	if (m_hEnemy == 0)
	{
		Look(4096);
		m_hEnemy = BestVisibleEnemy();
	}

	if(m_fGetMad)
	{
		pev->sequence = LookupSequence("mad");
		m_flMadDelayTime = gpGlobals->time;
		m_fGetMad = false;
		return;
	}

	if(m_fRightEyeHit && m_fLeftEyeHit)
	{
		if(gpGlobals->time <= m_flOrificeOpenTime  && !m_fOrificeHit)
		{
			pev->sequence = LookupSequence("bigpain2");
			EmitSoundScript(bigPain2SoundScript);
			return;
		}

		m_flOrificeOpenTime = gpGlobals->time;

		if(!m_fSpawningTrooper)
		{
			pev->sequence = LookupSequence("bigpain4");
			EmitSoundScript(bigPain4SoundScript);
			m_fSpawningTrooper = true;
			return;
		}

		m_fLeftEyeHit = false;
		m_fRightEyeHit = false;
		m_fOrificeHit = false;
		m_fSpawningTrooper = false;
		pev->skin = 0;
	}

	if(ClawAttack())
		return;

	pev->sequence = LookupSequence("idle");
}

void CGeneWorm::TrackHead()
{
	Vector vecMouthPos, vecMouthAngle;
	GetAttachment(0, vecMouthPos, vecMouthAngle);
	Vector angles = UTIL_VecToAngles(m_posTarget - vecMouthPos);
	float diff = UTIL_AngleDiff(angles.y, pev->angles.y);
	if (diff < -30)
	{
		diff = -30;
	}
	else if (diff > 30)
	{
		diff = 30;
	}
	SetBoneController(0, diff);
}

bool CGeneWorm::ClawAttack()
{
	Vector targetAngle;

	if(m_hEnemy)
	{
		m_posTarget = m_hEnemy->pev->origin;
		targetAngle = UTIL_VecToAngles(m_posTarget - pev->origin);
		float AngleDiff = UTIL_AngleDiff(targetAngle.y, pev->angles.y);

		if(m_flNextRangeTime <= gpGlobals->time)
		{
			if((m_posTarget - pev->origin).Length() >= 1200)
				pev->sequence = LookupSequence("dattack1");
			else if(AngleDiff > 10)
				pev->sequence = LookupSequence("dattack2");
			else if(AngleDiff < 0)
				pev->sequence = LookupSequence("dattack3");

			EmitSoundScript(beamAttackSoundScript);

			m_flNextRangeTime = gpGlobals->time + RANDOM_FLOAT(10,15);

			return true;
		}

		if(m_flNextMeleeTime <= gpGlobals->time)
		{
			if(m_posTarget.z <= pev->origin.z)
				pev->sequence = LookupSequence("melee3");

			else
			{
				if(AngleDiff >= 10)
				{
					pev->sequence = LookupSequence("melee1");
					EmitSoundScript(attack1SoundScript);
				}
				else if(AngleDiff <= -2)
				{
					pev->sequence = LookupSequence("melee2");
					EmitSoundScript(attack2SoundScript);
				}
				else
				{
					pev->sequence = LookupSequence("melee3");
					EmitSoundScript(attack3SoundScript);
				}

				m_flNextMeleeTime = gpGlobals->time + RANDOM_FLOAT(3,5);
				return true;
			}
		}
	}

	return false;
}

void CGeneWorm::TraceAttack(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	if(!FNullEnt(pevInflictor) && FClassnameIs(pevInflictor, "monster_penguin"))
	{
		pev->health = 0;
		SetThink(&CGeneWorm::DyingThink);
		return;
	}

	if(gpGlobals->time <= m_flTakeHitTime)
		return;

	if(ptr->iHitgroup != 5 && ptr->iHitgroup != 4 && ptr->iHitgroup != 6)
	{
		if(pev->dmgtime != gpGlobals->time|| RANDOM_LONG(0, 10) <= 0)
		{
			if(!FNullEnt(pevInflictor) && FClassnameIs(pevInflictor, "env_laser"))
			{
				UTIL_Sparks(ptr->vecEndPos);
			}
			else if(bitsDamageType & DMG_BULLET)
			{
				UTIL_Ricochet(ptr->vecEndPos, RANDOM_LONG(1, 2));
			}
			pev->dmgtime = gpGlobals->time;
		}
		return;
	}

	if(!FNullEnt(pevInflictor) && FClassnameIs(pevInflictor, "env_laser"))
	{
		if(!m_fHasEntered)
			return;

		if(ptr->iHitgroup == 4 && !m_fLeftEyeHit && FStrEq("left_eye_laser", STRING(pevInflictor->targetname)))
		{
			m_fLeftEyeHit = true;
			m_iWasHit = 1;

			if(!m_fRightEyeHit)
			{
				pev->skin = GENEWORM_SKIN_EYE_LEFT;

				if(gpGlobals->time - m_flMadDelayTime >= 6)
					m_fGetMad = true;
			}
			else
			{
				pev->skin = GENEWORM_SKIN_EYE_CLOSED;
				m_flOrificeOpenTime = gpGlobals->time + 20;
				m_fGetMad = false;
			}

			UTIL_BloodDrips(ptr->vecEndPos, ptr->vecEndPos, m_bloodColor, 256);
		}

		if(ptr->iHitgroup == 5 && !m_fRightEyeHit && FStrEq("right_eye_laser", STRING(pevInflictor->targetname)))
		{
			m_fRightEyeHit = true;
			m_iWasHit = 1;

			if(!m_fLeftEyeHit)
			{
				pev->skin = GENEWORM_SKIN_EYE_RIGHT;

				if(gpGlobals->time - m_flMadDelayTime >= 6)
					m_fGetMad = true;
			}
			else
			{
				pev->skin = GENEWORM_SKIN_EYE_CLOSED;
				m_flOrificeOpenTime = gpGlobals->time + 20;
				m_fGetMad = false;
			}

			UTIL_BloodDrips(ptr->vecEndPos, ptr->vecEndPos, m_bloodColor, 256);
		}
	}

	if(m_flOrificeOpenTime >= gpGlobals->time && m_pBall && ptr->iHitgroup == 6 && !m_fOrificeHit)
	{
		pev->health -= flDamage;

		if(pev->health <= 0)
		{
			if(m_iHitTimes < m_iMaxHitTimes)
			{
				UTIL_BloodDecalTrace(ptr, m_bloodColor);
				pev->health = gSkillData.gwormHealth;
				m_iHitTimes++;
				m_fOrificeHit = true;
				m_flOrificeOpenTime = gpGlobals->time;
				m_iWasHit = 1;
			}
			else
			{
				m_iWasHit = 2;
				SetThink(&CGeneWorm::DyingThink);
			}
		}
	}
}

void CGeneWorm::HuntThink(void)
{
	pev->nextthink = gpGlobals->time + 0.1;
	GlowShellUpdate();

	if(!m_fActivated)
		return;

	DispatchAnimEvents();
	StudioFrameAdvance();

	if (m_iWasHit > 1)
		return;

	if(pev->rendermode == kRenderTransTexture)
	{
		if(pev->renderamt < 248)
			pev->renderamt += 3;
		else
		{
			pev->renderamt = 255;
			pev->renderfx = 0;
			pev->rendermode = kRenderNormal;
		}
	}

	if (!m_iWasHit)
	{
		if(m_fSequenceFinished)
		{
			m_fHasEntered = true;
			int oldSeq = pev->sequence;
			NextActivity();
			if (pev->sequence != oldSeq || !m_fSequenceLoops)
			{
				pev->frame = 0;
				ResetSequenceInfo();
			}
		}
	}
	else if (m_iWasHit == 1)
	{
		UTIL_Remove(m_pCloud);
		m_pCloud = NULL;
		m_fSpiting = false;

		int iDir = 1;
		const char* painAnimation = 0;
		const char* painSoundScript = eyePainSoundScript;
		if (m_fLeftEyeHit && m_fRightEyeHit)
		{
			if (!m_fOrificeHit)
			{
				painAnimation = "bigpain1";
				painSoundScript = bigPain1SoundScript;

				if(!m_pBall)
				{
					Vector pos, angle;
					GetAttachment(GENEWORM_ATTACHMENT_SPAWN, pos, angle);
					m_pBall = CSprite::SpriteCreate( "sprites/boss_glow.spr", pos, true );
					if( m_pBall )
					{
						m_pBall->SetTransparency( kRenderTransAdd, 255, 255, 255, 255, kRenderFxNoDissipation );
						m_pBall->SetAttachment( edict(), 2 );
						m_pBall->SetScale( 1.5 );
						m_pBall->pev->framerate = 10.0;
						m_pBall->TurnOn();
					}
				}
			}
			else
			{
				painAnimation = "bigpain3";
				painSoundScript = bigPain3SoundScript;
			}
		}
		else if (m_fLeftEyeHit)
		{
			painAnimation = "eyepain1";
		}
		else if (m_fRightEyeHit)
		{
			painAnimation = "eyepain2";
		}

		EmitSoundScript(painSoundScript);

		if (painAnimation)
		{
			pev->sequence = FindTransition(pev->sequence, LookupSequence(painAnimation), &iDir);
			if (iDir > 0)
			{
				pev->frame = 0;
			}
			else
			{
				pev->frame = 255;
			}
			ResetSequenceInfo();
		}

		m_iWasHit = 0;
	}

	if (!m_fRightEyeHit)
	{
		Vector vecOrigin, vecAngles;
		GetAttachment(GENEWORM_ATTACHMENT_RIGHTEYE, vecOrigin, vecAngles);

		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE( TE_ELIGHT );
			WRITE_SHORT( entindex() );		// entity, attachment
			WRITE_VECTOR( vecOrigin );		// origin
			WRITE_COORD( 48 );	// radius
			WRITE_BYTE( 128 );	// R
			WRITE_BYTE( 255 );	// G
			WRITE_BYTE( 128 );	// B
			WRITE_BYTE( 1 );	// life * 10
			WRITE_COORD( 2 ); // decay
		MESSAGE_END();
	}
	if (!m_fLeftEyeHit)
	{
		Vector vecOrigin, vecAngles;
		GetAttachment(GENEWORM_ATTACHMENT_LEFTEYE, vecOrigin, vecAngles);

		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE( TE_ELIGHT );
			WRITE_SHORT( entindex() );		// entity, attachment
			WRITE_VECTOR( vecOrigin );		// origin
			WRITE_COORD( 48 );	// radius
			WRITE_BYTE( 128 );	// R
			WRITE_BYTE( 255 );	// G
			WRITE_BYTE( 128 );	// B
			WRITE_BYTE( 1 );	// life * 10
			WRITE_COORD( 1 ); // decay
		MESSAGE_END();
	}
	if (m_pBall)
	{
		Vector pos, angle;
		GetAttachment(GENEWORM_ATTACHMENT_SPAWN, pos, angle);
		UTIL_SetOrigin(m_pBall->pev, pos);
	}

	if(m_hEnemy)
	{
		m_posTarget = m_hEnemy->pev->origin;
		m_posTarget.z += 24;
		TrackHead();

		if(m_fSpiting && gpGlobals->time - m_flSpitStartTime <= 2)
		{
			Vector pos, angle;

			GetAttachment(0, pos, angle);

			angle = (m_hEnemy->pev->origin - pos).Normalize();

			m_pCloud = CGeneWormCloud::LaunchCloud(pos, angle, 700, ENT(pev), 0.15, GetProjectileOverrides());
			m_pCloud->pev->rendermode = 3;
			m_pCloud->pev->rendercolor.x = 0;
			m_pCloud->pev->rendercolor.y = 255;
			m_pCloud->pev->rendercolor.z = 0;
			m_pCloud->pev->renderfx = 14;
			m_pCloud = NULL;
		}
		else
		{
			m_fSpiting = false;
		}
	}
	else
	{
		SetBoneController(0, 0);
	}
}

//=========================================================
//=========================================================

void CGeneWorm::HandleAnimEvent(MonsterEvent_t *pEvent)
{
	switch (pEvent->event)
	{
	case GENEWORM_AE_BEAM:
		m_fSpiting = true;
		m_flSpitStartTime = gpGlobals->time;
		break;
	case GENEWORM_AE_PORTAL:
	{
		Vector vecPos, vecAng;

		if(m_pBall)
		{
			UTIL_Remove(m_pBall);
			m_pBall = NULL;

			GetAttachment(GENEWORM_ATTACHMENT_SPAWN, vecPos, vecAng);
			vecAng = pev->angles;

			m_orificeGlow = CGeneWormSpawn::LaunchSpawn(vecPos, -vecAng, 1.25, ENT(pev), GetProjectileOverrides());
			pev->health = gSkillData.gwormHealth;

			m_orificeGlow = NULL;

			EmitSoundScript(launchSpawnSoundScript);
		}
		break;
	}
	case GENEWORM_AE_MELEE_LEFT1:
		FireHurtTargets("GeneWormLeftSlash", this, this, USE_ON);
		break;
	case GENEWORM_AE_MELEE_LEFT2:
		FireHurtTargets("GeneWormLeftSlash", this, this, USE_OFF);
		break;
	case GENEWORM_AE_MELEE_RIGHT1:
		FireHurtTargets("GeneWormRightSlash", this, this, USE_ON);
		break;
	case GENEWORM_AE_MELEE_RIGHT2:
		FireHurtTargets("GeneWormRightSlash", this, this, USE_OFF);
		break;
	case GENEWORM_AE_MELEE_FORWARD1:
		FireHurtTargets("GeneWormCenterSlash", this, this, USE_ON);
		break;
	case GENEWORM_AE_MELEE_FORWARD2:
		FireHurtTargets("GeneWormCenterSlash", this, this, USE_OFF);
		break;
	case GENEWORM_AE_MAD:
		FireHurtTargets("GeneWormWallHit", this, this, USE_TOGGLE);
		UTIL_ScreenShake(pev->origin, 24, 3, 5, 2048);
		break;
	default:
		break;
	}
}

void CGeneWorm::CommandUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if(useType == USE_TOGGLE && !m_fActivated)
	{
		pev->sequence = LookupSequence("entry");
		pev->frame = 0;
		ResetSequenceInfo();
		pev->rendermode = kRenderTransTexture;
		pev->renderfx = 0;
		pev->renderamt = 0;

		m_fActivated = true;

		pev->solid = SOLID_BBOX;

		UTIL_SetOrigin(pev, pev->origin);
		EmitSoundScript(entrySoundScript);
	}
}

int CGeneWorm::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	return 0;
}

void CGeneWorm::IdleSound(void)
{
	EmitSoundScript(idleSoundScript);
}

#endif
