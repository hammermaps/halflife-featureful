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
// voltigore
//=========================================================

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"schedule.h"
#include	"effects.h"
#include	"decals.h"
#include	"soundent.h"
#include	"scripted.h"
#include	"game.h"
#include	"squadmonster.h"
#include	"combat.h"
#include	"mod_features.h"
#include	"game.h"
#include	"common_soundscripts.h"

#if FEATURE_VOLTIFORE
#define		VOLTIGORE_SPRINT_DIST	256 // how close the voltigore has to get before starting to sprint and refusing to swerve

#define		VOLTIGORE_BEAM_COUNT		8

#define VOLTIGORE_ZAP_RED 180
#define VOLTIGORE_ZAP_GREEN 16
#define VOLTIGORE_ZAP_BLUE 255
#define VOLTIGORE_ZAP_BEAM "sprites/lgtning.spr"
#define VOLTIGORE_ZAP_NOISE 80
#define VOLTIGORE_ZAP_WIDTH 30
#define VOLTIGORE_ZAP_BRIGHTNESS 255
#define VOLTIGORE_ZAP_DISTANCE 512
#define VOLTIGORE_GLOW_SCALE 0.75f
#define VOLTIGORE_GIB_COUNT 10

class CChargedBolt : public CBaseEntity
{
public:
	void Precache();
	void Spawn();

	void InitBeams();
	void ClearBeams();

	void EXPORT ShutdownChargedBolt();

	static CChargedBolt* ChargedBoltCreate();

	void LaunchChargedBolt(const Vector& vecAim, edict_t* pOwner, int nSpeed);

	void SetAttachment(CBaseAnimating* pAttachEnt, int iAttachIdx);

	void ArmBeam(int side);

	void EXPORT AttachThink();

	void EXPORT FlyThink();
	void EXPORT PreShutdownThink();
	void DoRadiusDamage(float dmg, float radius);

	void EXPORT ChargedBoltTouch(CBaseEntity* pOther);

	int Save(CSave& save);
	int Restore(CRestore& restore);
	static TYPEDESCRIPTION m_SaveData[];

	EHANDLE m_pBeam[VOLTIGORE_BEAM_COUNT];
	int m_iBeams;
	CBaseAnimating* m_pAttachEnt;
	int m_iAttachIdx;
	float m_shutdownTime;
	float m_radiusCheckTime;
};

LINK_ENTITY_TO_CLASS(charged_bolt, CChargedBolt);

TYPEDESCRIPTION CChargedBolt::m_SaveData[] =
{
	DEFINE_ARRAY(CChargedBolt, m_pBeam, FIELD_EHANDLE, VOLTIGORE_BEAM_COUNT),
	DEFINE_FIELD(CChargedBolt, m_iBeams, FIELD_INTEGER),
	DEFINE_FIELD(CChargedBolt, m_pAttachEnt, FIELD_CLASSPTR),
	DEFINE_FIELD(CChargedBolt, m_iAttachIdx, FIELD_INTEGER),
	DEFINE_FIELD(CChargedBolt, m_shutdownTime, FIELD_TIME),
};

IMPLEMENT_SAVERESTORE(CChargedBolt, CBaseEntity);

void CChargedBolt::Precache()
{
	PRECACHE_MODEL("sprites/blueflare2.spr");
	PRECACHE_MODEL("sprites/lgtning.spr");
}

void CChargedBolt::Spawn()
{
	Precache();

	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_BBOX;

	pev->gravity = 0.5f;

	SET_MODEL(edict(), "sprites/blueflare2.spr");

	UTIL_SetOrigin(pev, pev->origin);

	UTIL_SetSize(pev, g_vecZero, g_vecZero);

	pev->rendermode = kRenderTransAdd;
	pev->renderamt = 255;
	pev->scale = 0.75f;

	InitBeams();
}

void CChargedBolt::InitBeams()
{
	memset(m_pBeam, 0, sizeof(m_pBeam));

	m_iBeams = 0;
	pev->skin = 0;
}

void CChargedBolt::ClearBeams()
{
	for (auto& pBeam : m_pBeam)
	{
		if (pBeam)
		{
			UTIL_Remove(pBeam);
			pBeam = nullptr;
		}
	}

	m_iBeams = 0;
	pev->skin = 0;
}

void CChargedBolt::ShutdownChargedBolt()
{
	ClearBeams();

	UTIL_Remove(this);
}

CChargedBolt* CChargedBolt::ChargedBoltCreate()
{
	auto pBolt = GetClassPtr<CChargedBolt>(nullptr);

	pBolt->pev->classname = MAKE_STRING("charged_bolt");

	pBolt->Spawn();

	return pBolt;
}

void CChargedBolt::LaunchChargedBolt(const Vector& vecAim, edict_t* pOwner, int nSpeed)
{
	pev->angles = vecAim;
	pev->owner = pOwner;
	pev->velocity = vecAim * nSpeed;

	pev->speed = nSpeed;

	SetTouch(&CChargedBolt::ChargedBoltTouch);
	SetThink(&CChargedBolt::FlyThink);

	m_radiusCheckTime = pev->nextthink = gpGlobals->time + 0.15f;
}

void CChargedBolt::SetAttachment(CBaseAnimating* pAttachEnt, int iAttachIdx)
{
	Vector vecOrigin;
	Vector vecAngles;

	m_iAttachIdx = iAttachIdx;
	m_pAttachEnt = pAttachEnt;

	pAttachEnt->GetAttachment(iAttachIdx, vecOrigin, vecAngles);

	UTIL_SetOrigin(pev, vecOrigin);

	SetThink(&CChargedBolt::AttachThink);

	pev->nextthink = gpGlobals->time + 0.05;
}

void CChargedBolt::ArmBeam(int side)
{
	TraceResult tr;
	float flDist = 1.0;

	if (m_iBeams >= VOLTIGORE_BEAM_COUNT)
		m_iBeams = 0;

	UTIL_MakeAimVectors(pev->angles);
	Vector vecSrc = pev->origin + gpGlobals->v_up * 36 + gpGlobals->v_right * side * 16 + gpGlobals->v_forward * 32;

	for (int i = 0; i < 3; i++)
	{
		Vector vecAim = gpGlobals->v_right * side * RANDOM_FLOAT(0, 1) + gpGlobals->v_up * RANDOM_FLOAT(-1, 1);
		TraceResult tr1;
		UTIL_TraceLine(vecSrc, vecSrc + vecAim * 512, dont_ignore_monsters, ENT(pev), &tr1);
		if (flDist > tr1.flFraction)
		{
			tr = tr1;
			flDist = tr.flFraction;
		}
	}

	// Couldn't find anything close enough
	if (flDist == 1.0)
		return;

	auto pBeam = m_pBeam[m_iBeams].Entity<CBeam>();

	if (!pBeam)
	{
		m_pBeam[m_iBeams] = pBeam = CBeam::BeamCreate("sprites/lgtning.spr", 30);
	}

	if (!pBeam)
		return;

	auto pHit = Instance(tr.pHit);

	if (pHit && pHit->pev->takedamage != DAMAGE_NO)
	{
		pBeam->EntsInit(entindex(), pHit->entindex());
		pBeam->SetColor(255, 16, 255);
		pBeam->SetBrightness(255);
		pBeam->SetNoise(20);
	}
	else
	{
		pBeam->PointEntInit(tr.vecEndPos, entindex());
		pBeam->SetColor(180, 16, 255);
		pBeam->SetBrightness(255);
		pBeam->SetNoise(80);
	}

	++m_iBeams;
}

void CChargedBolt::AttachThink()
{
	Vector vecOrigin;
	Vector vecAngles;

	m_pAttachEnt->GetAttachment(m_iAttachIdx, vecOrigin, vecAngles);
	UTIL_SetOrigin(pev, vecOrigin);

	pev->nextthink = gpGlobals->time + 0.05;
}

void CChargedBolt::FlyThink()
{
	ArmBeam(-1);
	ArmBeam(1);
	pev->nextthink = gpGlobals->time + 0.05f;

	if (m_radiusCheckTime <= gpGlobals->time)
	{
		DoRadiusDamage(gSkillData.voltigoreDmgBeam * 0.2f, 32);
		m_radiusCheckTime = gpGlobals->time + 0.1f;
	}
}

void CChargedBolt::PreShutdownThink()
{
	pev->nextthink = gpGlobals->time + 0.1f;
	DoRadiusDamage(gSkillData.voltigoreDmgBeam * 0.2f, 32);

	if (m_shutdownTime <= gpGlobals->time)
	{
		pev->nextthink = gpGlobals->time + 0.05f;
		SetThink(&CChargedBolt::ShutdownChargedBolt);
	}
}

void CChargedBolt::DoRadiusDamage(float dmg, float radius)
{
	CBaseMonster* pOwner = GetMonsterPointer(pev->owner);
	::RadiusDamage(nullptr, pev->origin, pev, pOwner ? pOwner->pev : pev, dmg, radius, DMG_SHOCK, RADIUSDAMAGE_CHECK_ATTACKER_TRACE,
				   [=](CBaseEntity* pEntity) {
		if (pOwner)
			pOwner->IRelationship(pEntity) != R_AL;
		return true;
	});
}

void CChargedBolt::ChargedBoltTouch(CBaseEntity* pOther)
{
	SetTouch(nullptr);
	SetThink(nullptr);

	if (pOther->pev->takedamage == DAMAGE_NO)
	{
		if (0 == strcmp("worldspawn", STRING(pOther->pev->classname)))
		{
			TraceResult tr;
			UTIL_TraceLine(pev->origin, pev->origin + pev->velocity * 10, dont_ignore_monsters, edict(), &tr);
			UTIL_DecalTrace(&tr, DECAL_SCORCH1 + RANDOM_LONG(0, 1));
		}
	}
	else
	{
		TraceResult tr = UTIL_GetGlobalTrace();
		ClearMultiDamage();
		entvars_t* pAttacker = VARS(pev->owner);
		if (!pAttacker)
			pAttacker = pev;

		pOther->TraceAttack(pev, pAttacker, gSkillData.voltigoreDmgBeam, pev->velocity, &tr, DMG_SHOCK|DMG_ALWAYSGIB);
		ApplyMultiDamage(pev, pAttacker);
	}

	pev->velocity = g_vecZero;

	SetThink(&CChargedBolt::PreShutdownThink);
	pev->nextthink = gpGlobals->time + 0.05f;
	m_shutdownTime = gpGlobals->time + 0.3f;
}

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define		VOLTIGORE_AE_THROW			( 1 )
#define		VOLTIGORE_AE_PUNCH_BOTH		( 12 )
#define		VOLTIGORE_AE_PUNCH_SINGLE	( 13 )
#define		VOLTIGORE_AE_GIB			( 2002 )


//=========================================================
// CVoltigore
//=========================================================
class CVoltigore : public CSquadMonster
{
public:
	virtual void Spawn(void);
	virtual void Precache(void);
	bool IsEnabledInMod() { return g_modFeatures.IsMonsterEnabled("voltigore"); }
	void SetYawSpeed(void);
	virtual int  DefaultClassify(void);
	const char* DefaultDisplayName() { return "Voltigore"; }
	virtual void HandleAnimEvent(MonsterEvent_t *pEvent);
	virtual void IdleSound();
	virtual void PainSound();
	virtual void PlayPainSound();
	virtual void DeathSound();
	virtual void AlertSound();
	virtual void StartTask(Task_t *pTask);
	virtual BOOL CheckMeleeAttack1(float flDot, float flDist);
	virtual BOOL CheckRangeAttack1(float flDot, float flDist);
	virtual void RunAI(void);
	virtual void GibMonster();
	void EXPORT CallDeathGibThink();
	virtual void DeathGibThink();
	Schedule_t *GetSchedule(void);
	Schedule_t *GetScheduleOfType(int Type);
	void OnDying();
	void UpdateOnRemove();
	const char* DefaultGibModel() {
		return "models/vgibs.mdl";
	}
	int DefaultGibCount() {
		return VOLTIGORE_GIB_COUNT;
	}
	
	int	Save(CSave &save);
	int Restore(CRestore &restore);

	CUSTOM_SCHEDULES
	static TYPEDESCRIPTION m_SaveData[];

	virtual int DefaultSizeForGrapple() { return GRAPPLE_LARGE; }
	bool IsDisplaceable() { return true; }
	Vector DefaultMinHullSize() {
		if (g_modFeatures.voltigore_lesser_size)
			return Vector( -40.0f, -40.0f, 0.0f );
		else
			return Vector( -80.0f, -80.0f, 0.0f );
	}
	Vector DefaultMaxHullSize() {
		if (g_modFeatures.voltigore_lesser_size)
			return Vector( 40.0f, 40.0f, 85.0f );
		else
			return Vector( 80.0f, 80.0f, 90.0f );
	}

	float m_flNextBeamAttackCheck; // next time the voltigore can use the spit attack.
	EHANDLE m_pBeam[3];
	float m_flNextPainTime;
	EHANDLE m_pChargedBolt;

	static const NamedSoundScript idleSoundScript;
	static const NamedSoundScript alertSoundScript;
	static const NamedSoundScript painSoundScript;
	static const NamedSoundScript dieSoundScript;
	static constexpr const char* attackHitSoundScript = "Voltigore.AttackHit";
	static constexpr const char* attackMissSoundScript = "Voltigore.AttackMiss";
	static const NamedSoundScript footstepSoundScript;
	static const NamedSoundScript beamAttackSoundScript;

	static const char* pComSounds[];
	static const char* pGruntSounds[];

	void UpdateBeamAndBoltPositions();
	void ClearBeams();
	Vector BoltPosition();
protected:
	void PrecacheImpl(const char* modelName);
};

LINK_ENTITY_TO_CLASS(monster_alien_voltigore, CVoltigore)


TYPEDESCRIPTION	CVoltigore::m_SaveData[] =
{
	DEFINE_FIELD(CVoltigore, m_flNextBeamAttackCheck, FIELD_TIME),
	DEFINE_ARRAY(CVoltigore, m_pBeam, FIELD_EHANDLE, 3),
	DEFINE_FIELD(CVoltigore, m_flNextPainTime, FIELD_TIME),
	DEFINE_FIELD(CVoltigore, m_pChargedBolt, FIELD_EHANDLE),
};

IMPLEMENT_SAVERESTORE(CVoltigore, CSquadMonster)

const NamedSoundScript CVoltigore::idleSoundScript = {
	CHAN_VOICE,
	{"voltigore/voltigore_idle1.wav", "voltigore/voltigore_idle2.wav", "voltigore/voltigore_idle3.wav"},
	"Voltigore.Idle"
};

const NamedSoundScript CVoltigore::alertSoundScript = {
	CHAN_VOICE,
	{"voltigore/voltigore_alert1.wav", "voltigore/voltigore_alert2.wav", "voltigore/voltigore_alert3.wav"},
	"Voltigore.Alert"
};

const NamedSoundScript CVoltigore::painSoundScript = {
	CHAN_VOICE,
	{"voltigore/voltigore_pain1.wav", "voltigore/voltigore_pain2.wav", "voltigore/voltigore_pain3.wav", "voltigore/voltigore_pain4.wav"},
	"Voltigore.Pain"
};

const NamedSoundScript CVoltigore::dieSoundScript = {
	CHAN_VOICE,
	{"voltigore/voltigore_die1.wav", "voltigore/voltigore_die2.wav", "voltigore/voltigore_die3.wav"},
	"Voltigore.Die"
};

const NamedSoundScript CVoltigore::footstepSoundScript = {
	CHAN_BODY,
	{"voltigore/voltigore_footstep1.wav", "voltigore/voltigore_footstep2.wav", "voltigore/voltigore_footstep3.wav"},
	"Voltigore.Footstep"
};

const NamedSoundScript CVoltigore::beamAttackSoundScript = {
	CHAN_STATIC,
	{"debris/beamstart2.wav"},
	0.5f,
	ATTN_NORM,
	IntRange(140, 160),
	"Voltigore.BeamAttack"
};

const char* CVoltigore::pComSounds[] =
{
	"voltigore/voltigore_communicate1.wav",
	"voltigore/voltigore_communicate2.wav",
	"voltigore/voltigore_communicate3.wav",
};

const char* CVoltigore::pGruntSounds[] =
{
	"voltigore/voltigore_run_grunt1.wav",
	"voltigore/voltigore_run_grunt2.wav",
};

BOOL CVoltigore::CheckRangeAttack1(float flDot, float flDist)
{
	if (IsMoving() && flDist >= 512)
	{
		// voltigore will far too far behind if he stops running to spit at this distance from the enemy.
		return FALSE;
	}

	if (flDist > 128.0f && flDist <= 1024.0f && flDot >= 0.5f && gpGlobals->time >= m_flNextBeamAttackCheck)
	{
		Vector vecStart, vecDir;
		TraceResult tr;

		UTIL_MakeVectors( pev->angles );
		GetAttachment( 3, vecStart, vecDir );
		UTIL_TraceLine( vecStart, m_hEnemy->BodyTarget( vecStart ), dont_ignore_monsters, ENT( pev ), &tr );

		if( tr.flFraction == 1.0f || tr.pHit == m_hEnemy->edict() )
		{
			m_flNextBeamAttackCheck = gpGlobals->time + RANDOM_FLOAT(1.5f, 5.0f);
			return TRUE;
		}
		else
		{
			m_flNextBeamAttackCheck = gpGlobals->time + 0.2f;
		}
	}

	return FALSE;
}

void CVoltigore::RunAI(void)
{
	UpdateBeamAndBoltPositions();
	CSquadMonster::RunAI();
}

void CVoltigore::GibMonster()
{
	pev->renderfx = kRenderFxExplode;

	pev->rendercolor.x = 255;
	pev->rendercolor.y = 0;
	pev->rendercolor.z = 0;
	pev->framerate = 0;

	CSquadMonster::GibMonster();

	pev->nextthink = gpGlobals->time + 0.15;
}

void CVoltigore::CallDeathGibThink()
{
	DeathGibThink();
}

void CVoltigore::DeathGibThink()
{
	pev->nextthink = gpGlobals->time + 0.1f;
	DispatchAnimEvents(0.1f);
	StudioFrameAdvance(0.0f);

	if (m_fSequenceFinished)
	{
		GibMonster();
	}
	else
	{
		for (auto i = 0; i < 2; ++i)
		{
			const auto side = i == 0 ? -1 : 1;

			UTIL_MakeAimVectors(pev->angles);

			TraceResult tr;

			const auto vecSrc = pev->origin + gpGlobals->v_forward * 32 + side * gpGlobals->v_right * 16 + gpGlobals->v_up * 36;

			float closest = 1.0f;

			//Do 3 ray traces and use the closest one to make a beam
			for (auto ray = 0; ray < 3; ++ray)
			{
				TraceResult tr1;
				UTIL_TraceLine(vecSrc, vecSrc + (side * gpGlobals->v_right * RANDOM_FLOAT(0, 1) + gpGlobals->v_up * RANDOM_FLOAT(-1, 1)) * 512, dont_ignore_monsters, edict(), &tr1);

				if (tr1.flFraction < closest)
				{
					tr = tr1;
					closest = tr1.flFraction;
				}
			}

			//No nearby objects found
			if (closest == 1.0f)
			{
				return;
			}

			auto pBeam = CBeam::BeamCreate("sprites/lgtning.spr", 30);

			if (!pBeam)
				return;

			auto pHit = Instance(tr.pHit);

			if (pHit && pHit->pev->takedamage != DAMAGE_NO)
			{
				pBeam->PointEntInit(pev->origin + Vector(0, 0, 32), pHit->entindex());

				pBeam->SetColor(180, 16, 255);
				pBeam->SetBrightness(255);
				pBeam->SetNoise(128);
			}
			else
			{
				pBeam->PointsInit(tr.vecEndPos, pev->origin + Vector(0, 0, 32));

				pBeam->SetColor(180, 16, 255);
				pBeam->SetBrightness(255);
				pBeam->SetNoise(192);
			}

			pBeam->SetThink(&CBaseEntity::SUB_Remove);
			pBeam->pev->nextthink = gpGlobals->time + 0.6;
		}

		const float attackRadius = Q_max(Q_min(gSkillData.voltigoreDmgExplode * 2.0f, 200.0f), 160.0f);
		ClearMultiDamage();
		::RadiusDamage(pev->origin, pev, pev, gSkillData.voltigoreDmgExplode, attackRadius, CLASS_NONE, DMG_SHOCK);
	}
}

BOOL CVoltigore::CheckMeleeAttack1(float flDot, float flDist)
{
	if (HasConditions(bits_COND_SEE_ENEMY) && flDist <= 128.0f && flDot >= 0.6 && m_hEnemy != 0)
	{
		return TRUE;
	}
	return FALSE;
}

//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int	CVoltigore::DefaultClassify(void)
{
	return	CLASS_RACEX_SHOCK;
}

//=========================================================
// IdleSound 
//=========================================================
void CVoltigore::IdleSound(void)
{
	EmitSoundScript(idleSoundScript);
}

//=========================================================
// PainSound 
//=========================================================
void CVoltigore::PainSound(void)
{
	if (m_flNextPainTime > gpGlobals->time)
		return;

	PlayPainSound();
	m_flNextPainTime = gpGlobals->time + 0.6;
}

void CVoltigore::PlayPainSound()
{
	EmitSoundScript(painSoundScript);
}

//=========================================================
// AlertSound
//=========================================================
void CVoltigore::AlertSound(void)
{
	EmitSoundScript(alertSoundScript);
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CVoltigore::SetYawSpeed(void)
{
	int ys;

	switch (m_Activity)
	{
	case	ACT_TURN_LEFT:
	case	ACT_TURN_RIGHT:
		ys = 80;
		break;
	default:
		ys = 70;
		break;
	}

	pev->yaw_speed = ys;
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CVoltigore::HandleAnimEvent(MonsterEvent_t *pEvent)
{
	switch (pEvent->event)
	{
	case VOLTIGORE_AE_THROW:
	{
		if (m_pChargedBolt)
		{
			UTIL_MakeVectors(pev->angles);

			const auto shootPosition = pev->origin + gpGlobals->v_forward * 50 + gpGlobals->v_up * 32;

			const auto direction = ShootAtEnemy(shootPosition);

			TraceResult tr;
			UTIL_TraceLine(shootPosition, shootPosition + direction * 1024, dont_ignore_monsters, edict(), &tr);

			CChargedBolt* bolt = m_pChargedBolt.Entity<CChargedBolt>();

			bolt->LaunchChargedBolt(direction, edict(), 1000);

			//We no longer have to manage the bolt now
			m_pChargedBolt = nullptr;

			ClearBeams();
		}
	}
	break;


	case VOLTIGORE_AE_PUNCH_SINGLE:
	{
		// SOUND HERE!
		CBaseEntity *pHurt = CheckTraceHullAttack(128.0f, gSkillData.voltigoreDmgPunch, DMG_CLUB);
		if (pHurt)
		{
			if (FBitSet(pHurt->pev->flags, FL_MONSTER|FL_CLIENT))
			{
				pHurt->pev->punchangle.z = -15;
				pHurt->pev->punchangle.x = 15;
				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_right * -150;
				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_up * 100;
			}

			EmitSoundScript(attackHitSoundScript);

			Vector vecArmPos, vecArmAng;
			GetAttachment( 0, vecArmPos, vecArmAng );
			SpawnBlood( vecArmPos, pHurt->BloodColor(), 25 );// a little surface blood.
		}
		else
		{
			EmitSoundScript(attackMissSoundScript);
		}
	}
	break;

	case VOLTIGORE_AE_PUNCH_BOTH:
	{
		// SOUND HERE!
		CBaseEntity *pHurt = CheckTraceHullAttack(128.0f, gSkillData.voltigoreDmgPunch, DMG_CLUB);
		if (pHurt)
		{
			if (FBitSet(pHurt->pev->flags, FL_MONSTER|FL_CLIENT))
			{
				pHurt->pev->punchangle.x = 20;
				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * 150;
				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_up * 100;
			}

			EmitSoundScript(attackHitSoundScript);

			Vector vecArmPos, vecArmAng;
			GetAttachment( 0, vecArmPos, vecArmAng );
			SpawnBlood( vecArmPos, pHurt->BloodColor(), 25 );// a little surface blood.
		}
		else
		{
			EmitSoundScript(attackMissSoundScript);
		}
	}
	break;

	case VOLTIGORE_AE_GIB:
	{
		pev->health = 0;
		GibMonster();
	}
	break;

	default:
		CSquadMonster::HandleAnimEvent(pEvent);
	}
}

//=========================================================
// Spawn
//=========================================================
void CVoltigore::Spawn()
{
	Precache();

	SetMyModel("models/voltigore.mdl");
	SetMySize( DefaultMinHullSize(), DefaultMaxHullSize() );

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	SetMyBloodColor(BLOOD_COLOR_GREEN);
	pev->effects		= 0;
	SetMyHealth(gSkillData.voltigoreHealth);
	SetMyFieldOfView(0.2f);// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;
	m_afCapability = bits_CAP_TURN_HEAD | bits_CAP_SQUAD;

	m_flNextBeamAttackCheck	= gpGlobals->time;

	MonsterInit();
	pev->view_ofs		= Vector(0, 0, 84);

	memset(m_pBeam, 0, sizeof(m_pBeam));
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CVoltigore::Precache()
{
	PrecacheImpl("models/voltigore.mdl");
	PrecacheMyGibModel("models/vgibs.mdl");

	RegisterAndPrecacheSoundScript(idleSoundScript);
	RegisterAndPrecacheSoundScript(alertSoundScript);
	RegisterAndPrecacheSoundScript(painSoundScript);
	RegisterAndPrecacheSoundScript(dieSoundScript);
	RegisterAndPrecacheSoundScript(attackHitSoundScript, NPC::attackHitSoundScript);
	RegisterAndPrecacheSoundScript(attackMissSoundScript, NPC::attackMissSoundScript);
	RegisterAndPrecacheSoundScript(footstepSoundScript);
	RegisterAndPrecacheSoundScript(beamAttackSoundScript);
}

void CVoltigore::PrecacheImpl(const char *modelName)
{
	PrecacheMyModel(modelName);

	PRECACHE_SOUND_ARRAY(pComSounds);
	PRECACHE_SOUND_ARRAY(pGruntSounds);

	PRECACHE_SOUND("voltigore/voltigore_attack_melee1.wav");
	PRECACHE_SOUND("voltigore/voltigore_attack_melee2.wav");

	PRECACHE_SOUND("voltigore/voltigore_attack_shock.wav");
	PRECACHE_SOUND("voltigore/voltigore_eat.wav");

	PRECACHE_MODEL(VOLTIGORE_ZAP_BEAM);

	UTIL_PrecacheOther("charged_bolt", GetProjectileOverrides());
}

//=========================================================
// DeathSound
//=========================================================
void CVoltigore::DeathSound(void)
{
	EmitSoundScript(dieSoundScript);
}

//========================================================
// AI Schedules Specific to this monster
//=========================================================

// primary range attack
Task_t	tlVoltigoreRangeAttack1[] =
{
	{ TASK_STOP_MOVING, 0 },
	{ TASK_FACE_ENEMY, (float)0 },
	{ TASK_RANGE_ATTACK1, (float)0 },
	{ TASK_SET_ACTIVITY, (float)ACT_IDLE },
};

Schedule_t	slVoltigoreRangeAttack1[] =
{
	{
		tlVoltigoreRangeAttack1,
		ARRAYSIZE(tlVoltigoreRangeAttack1),
		bits_COND_NEW_ENEMY |
		bits_COND_ENEMY_DEAD |
		bits_COND_ENEMY_LOST |
		bits_COND_HEAVY_DAMAGE,
		0,
		"Voltigore Range Attack1"
	},
};

//=========================================================
// Victory dance!
//=========================================================
Task_t tlVoltigoreVictoryDance[] =
{
	{ TASK_STOP_MOVING, (float)0 },
	{ TASK_WAIT, 0.2f },
	{ TASK_GET_PATH_TO_ENEMY_CORPSE,	50.0f },
	{ TASK_WALK_PATH, (float)0 },
	{ TASK_WAIT_FOR_MOVEMENT, (float)0 },
	{ TASK_FACE_ENEMY, (float)0 },
	{ TASK_PLAY_SEQUENCE, (float)ACT_STAND },
	{ TASK_PLAY_SEQUENCE, (float)ACT_VICTORY_DANCE },
	{ TASK_GET_HEALTH_FROM_FOOD, 0.2f },
	{ TASK_PLAY_SEQUENCE, (float)ACT_VICTORY_DANCE },
	{ TASK_GET_HEALTH_FROM_FOOD, 0.2f },
	{ TASK_PLAY_SEQUENCE, (float)ACT_VICTORY_DANCE },
	{ TASK_GET_HEALTH_FROM_FOOD, 0.2f },
	{ TASK_PLAY_SEQUENCE, (float)ACT_VICTORY_DANCE },
	{ TASK_GET_HEALTH_FROM_FOOD, 0.2f },
	{ TASK_PLAY_SEQUENCE, (float)ACT_STAND },
};

Schedule_t slVoltigoreVictoryDance[] =
{
	{
		tlVoltigoreVictoryDance,
		ARRAYSIZE( tlVoltigoreVictoryDance ),
		bits_COND_NEW_ENEMY |
		bits_COND_SCHEDULE_SUGGESTED |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE,
		0,
		"VoltigoreVictoryDance"
	},
};

DEFINE_CUSTOM_SCHEDULES(CVoltigore)
{
	slVoltigoreRangeAttack1,
	slVoltigoreVictoryDance
};

IMPLEMENT_CUSTOM_SCHEDULES(CVoltigore, CSquadMonster)

Schedule_t *CVoltigore::GetSchedule(void)
{
	switch (m_MonsterState)
	{
	case MONSTERSTATE_COMBAT:
	{
		// dead enemy
		if (HasConditions(bits_COND_ENEMY_DEAD|bits_COND_ENEMY_LOST))
		{
			// call base class, all code to handle dead enemies is centralized there.
			return CSquadMonster::GetSchedule();
		}

		if (HasConditions(bits_COND_NEW_ENEMY))
		{
			return GetScheduleOfType(SCHED_WAKE_ANGRY);
		}

		if( HasConditions( bits_COND_ENEMY_OCCLUDED ) )
		{
			return GetScheduleOfType(SCHED_CHASE_ENEMY);
		}

		if (HasConditions(bits_COND_CAN_RANGE_ATTACK1))
		{
			return GetScheduleOfType(SCHED_RANGE_ATTACK1);
		}

		if (HasConditions(bits_COND_CAN_MELEE_ATTACK1))
		{
			return GetScheduleOfType(SCHED_MELEE_ATTACK1);
		}

		return GetScheduleOfType(SCHED_CHASE_ENEMY);

		break;
	}
	default:
		break;
	}

	return CSquadMonster::GetSchedule();
}

Schedule_t* CVoltigore::GetScheduleOfType(int Type)
{
	switch (Type)
	{
	case SCHED_RANGE_ATTACK1:
		return &slVoltigoreRangeAttack1[0];
		break;
	case SCHED_VICTORY_DANCE:
		return &slVoltigoreVictoryDance[0];
		break;
	}

	return CSquadMonster::GetScheduleOfType(Type);
}

void CVoltigore::StartTask(Task_t *pTask)
{
	ClearBeams();

	switch (pTask->iTask)
	{
	case TASK_RANGE_ATTACK1:
		{
			UTIL_MakeVectors(pev->angles);

			const auto vecConverge = BoltPosition();

			for (auto i = 0; i < 3; ++i)
			{
				CBeam* pBeam = CBeam::BeamCreate(VOLTIGORE_ZAP_BEAM, VOLTIGORE_ZAP_WIDTH);
				m_pBeam[i] = pBeam;

				if (!pBeam)
					return;

				pBeam->PointEntInit(vecConverge, entindex());
				pBeam->SetEndAttachment(i + 1);
				pBeam->SetColor(VOLTIGORE_ZAP_RED, VOLTIGORE_ZAP_GREEN, VOLTIGORE_ZAP_BLUE);
				pBeam->SetBrightness(VOLTIGORE_ZAP_BRIGHTNESS);
				pBeam->SetNoise(20);
			}

			m_pChargedBolt = CChargedBolt::ChargedBoltCreate();

			UTIL_SetOrigin(m_pChargedBolt->pev, vecConverge);

			EmitSoundScriptAmbient(pev->origin, beamAttackSoundScript);
			CSquadMonster::StartTask(pTask);
		}
		break;
	case TASK_DIE:
	{
		SetThink(&CVoltigore::CallDeathGibThink);
		CSquadMonster::StartTask(pTask);
	}
	default:
		CSquadMonster::StartTask(pTask);
		break;
	}
}

void CVoltigore::OnDying()
{
	ClearBeams();
	CSquadMonster::OnDying();
}

void CVoltigore::UpdateOnRemove()
{
	if (m_pChargedBolt != 0)
		UTIL_Remove(m_pChargedBolt);
	ClearBeams();
	CSquadMonster::UpdateOnRemove();
}

void CVoltigore::UpdateBeamAndBoltPositions()
{
	if (m_pChargedBolt)
	{
		const Vector pos = BoltPosition();
		UTIL_SetOrigin(m_pChargedBolt->pev, pos);

		for (int i=0; i<ARRAYSIZE(m_pBeam); ++i)
		{
			CBeam* pBeam = m_pBeam[i].Entity<CBeam>();
			if (!pBeam)
				continue;
			pBeam->SetStartPos(pos);
			pBeam->RelinkBeam();
		}
	}
}

void CVoltigore::ClearBeams()
{
	for (auto& pBeam : m_pBeam)
	{
		if (pBeam)
		{
			UTIL_Remove(pBeam);
			pBeam = nullptr;
		}
	}

	if (m_pChargedBolt)
	{
		UTIL_Remove(m_pChargedBolt);
		m_pChargedBolt = nullptr;
	}
}

Vector CVoltigore::BoltPosition()
{
	UTIL_MakeVectors(pev->angles);
	return pev->origin + gpGlobals->v_forward * 50 + gpGlobals->v_up * 32;
}

//=========================================================
// CBabyAlienVoltigore
//=========================================================

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define		BABY_VOLTIGORE_AE_RUN			( 14 )

class CBabyVoltigore : public CVoltigore
{
public:
	void	Spawn(void);
	void	Precache(void);
	bool IsEnabledInMod() { return g_modFeatures.IsMonsterEnabled("babyvoltigore"); }
	const char* DefaultDisplayName() { return "Baby Voltigore"; }
	void	HandleAnimEvent(MonsterEvent_t* pEvent);
	BOOL	CheckMeleeAttack1(float flDot, float flDist);
	BOOL	CheckRangeAttack1(float flDot, float flDist);
	void	StartTask(Task_t *pTask);
	void	GibMonster();
	const char* DefaultGibModel() {
		return CSquadMonster::DefaultGibModel();
	}
	int DefaultGibCount() {
		return CSquadMonster::DefaultGibCount();
	}
	Schedule_t* GetSchedule();
	Schedule_t* GetScheduleOfType(int Type);

	virtual int DefaultSizeForGrapple() { return GRAPPLE_SMALL; }
	Vector DefaultMinHullSize() { return Vector( -16.0f, -16.0f, 0.0f ); }
	Vector DefaultMaxHullSize() { return Vector( 16.0f, 16.0f, 32.0f ); }

	void IdleSound();
	void AlertSound();
	void PlayPainSound();
	void DeathSound();
	void AttackSound();

	static constexpr const char* idleSoundScript = "BabyVoltigore.Idle";
	static constexpr const char* alertSoundScript = "BabyVoltigore.Alert";
	static constexpr const char* painSoundScript = "BabyVoltigore.Pain";
	static constexpr const char* dieSoundScript = "BabyVoltigore.Die";
	static constexpr const char* attackHitSoundScript = "BabyVoltigore.AttackHit";
	static constexpr const char* attackMissSoundScript = "BabyVoltigore.AttackMiss";
	static constexpr const char* footstepSoundScript = "BabyVoltigore.Footstep";
	static const NamedSoundScript attackSoundScript;
};

LINK_ENTITY_TO_CLASS(monster_alien_babyvoltigore, CBabyVoltigore)

const NamedSoundScript CBabyVoltigore::attackSoundScript = {
	CHAN_VOICE,
	{"voltigore/voltigore_attack_melee1.wav", "voltigore/voltigore_attack_melee2.wav"},
	130,
	"BabyVoltigore.Attack"
};

//=========================================================
// Spawn
//=========================================================
void CBabyVoltigore::Spawn()
{
	Precache();

	SetMyModel("models/baby_voltigore.mdl");
	SetMySize( DefaultMinHullSize(), DefaultMaxHullSize() );

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	SetMyBloodColor(BLOOD_COLOR_GREEN);
	pev->effects		= 0;
	SetMyHealth(gSkillData.babyVoltigoreHealth);
	SetMyFieldOfView(0.2f);// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;
	m_afCapability = bits_CAP_TURN_HEAD | bits_CAP_SQUAD;

	m_flNextBeamAttackCheck	= gpGlobals->time;

	MonsterInit();
	pev->view_ofs		= Vector(0, 0, 32);
}

//=========================================================
//=========================================================
void CBabyVoltigore::Precache(void)
{
	PrecacheImpl("models/baby_voltigore.mdl");

	SoundScriptParamOverride voiceParamOverride;
	voiceParamOverride.OverridePitchRelative(180);

	SoundScriptParamOverride bodyParamOverride;
	bodyParamOverride.OverridePitchRelative(130);

	RegisterAndPrecacheSoundScript(idleSoundScript, CVoltigore::idleSoundScript, voiceParamOverride);
	RegisterAndPrecacheSoundScript(alertSoundScript, CVoltigore::alertSoundScript, voiceParamOverride);
	RegisterAndPrecacheSoundScript(painSoundScript, CVoltigore::painSoundScript, voiceParamOverride);
	RegisterAndPrecacheSoundScript(dieSoundScript, CVoltigore::dieSoundScript, voiceParamOverride);
	RegisterAndPrecacheSoundScript(attackHitSoundScript, NPC::attackHitSoundScript, bodyParamOverride);
	RegisterAndPrecacheSoundScript(attackMissSoundScript, NPC::attackMissSoundScript, bodyParamOverride);
	RegisterAndPrecacheSoundScript(footstepSoundScript, CVoltigore::footstepSoundScript, bodyParamOverride);
	RegisterAndPrecacheSoundScript(attackSoundScript);
}

void CBabyVoltigore::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{
	case BABY_VOLTIGORE_AE_RUN:
		EmitSoundScript(footstepSoundScript);
		break;

	case VOLTIGORE_AE_PUNCH_SINGLE:
	{
		CBaseEntity *pHurt = CheckTraceHullAttack(64, gSkillData.babyVoltigoreDmgPunch, DMG_CLUB, pev->size.z);
		if (pHurt)
		{
			if (FBitSet(pHurt->pev->flags, FL_MONSTER|FL_CLIENT))
			{
				pHurt->pev->punchangle.z = -10;
				pHurt->pev->punchangle.x = 10;
				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_right * -100;
				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_up * 50;
			}

			EmitSoundScript(attackHitSoundScript);

			Vector vecArmPos, vecArmAng;
			GetAttachment( 0, vecArmPos, vecArmAng );
			SpawnBlood( vecArmPos, pHurt->BloodColor(), 25 );// a little surface blood.
		}
		else
		{
			EmitSoundScript(attackMissSoundScript);
		}
	}
	break;

	case VOLTIGORE_AE_PUNCH_BOTH:
	{
		CBaseEntity *pHurt = CheckTraceHullAttack(64, gSkillData.babyVoltigoreDmgPunch, DMG_CLUB, pev->size.z);
		if (pHurt)
		{
			if (FBitSet(pHurt->pev->flags, FL_MONSTER|FL_CLIENT))
			{
				pHurt->pev->punchangle.x = 15;
				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * 100;
				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_up * 50;

				Vector vecArmPos, vecArmAng;
				GetAttachment( 0, vecArmPos, vecArmAng );
				SpawnBlood( vecArmPos, pHurt->BloodColor(), 25 );// a little surface blood.
			}

			EmitSoundScript(attackHitSoundScript);
		}
		else
		{
			EmitSoundScript(attackMissSoundScript);
		}
	}
	break;
	default:
		CVoltigore::HandleAnimEvent(pEvent);
		break;
	}
}

BOOL CBabyVoltigore::CheckMeleeAttack1(float flDot, float flDist)
{
	if (HasConditions(bits_COND_SEE_ENEMY) && flDist <= 64.0f && flDot >= 0.6f && m_hEnemy != 0)
	{
		return TRUE;
	}
	return FALSE;
}

//=========================================================
// Start task - selects the correct activity and performs
// any necessary calculations to start the next task on the
// schedule.  OVERRIDDEN for voltigore because it needs to
// know explicitly when the last attempt to chase the enemy
// failed, since that impacts its attack choices.
//=========================================================
void CBabyVoltigore::StartTask(Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_MELEE_ATTACK1:
	{
		CSquadMonster::StartTask(pTask);
		break;
	}
	default:
		CSquadMonster::StartTask(pTask);
		break;
	}
}

void CBabyVoltigore::GibMonster()
{
	CSquadMonster::GibMonster();
}

BOOL CBabyVoltigore::CheckRangeAttack1(float flDot, float flDist)
{
	return FALSE;
}

//=========================================================
// GetSchedule 
//=========================================================
Schedule_t *CBabyVoltigore::GetSchedule(void)
{
	switch (m_MonsterState)
	{
	case MONSTERSTATE_COMBAT:
	{
		// dead enemy
		if (HasConditions(bits_COND_ENEMY_DEAD|bits_COND_ENEMY_LOST))
		{
			// call base class, all code to handle dead enemies is centralized there.
			return CSquadMonster::GetSchedule();
		}

		if (HasConditions(bits_COND_NEW_ENEMY))
		{
			return GetScheduleOfType(SCHED_WAKE_ANGRY);
		}

		if (HasConditions(bits_COND_CAN_MELEE_ATTACK1))
		{
			AttackSound();
			return GetScheduleOfType(SCHED_MELEE_ATTACK1);
		}

		return GetScheduleOfType(SCHED_CHASE_ENEMY);

		break;
	}
	default:
		break;
	}

	return CSquadMonster::GetSchedule();
}

Schedule_t *CBabyVoltigore::GetScheduleOfType(int Type)
{
	switch (Type) {
	// TODO:
	// For some cryptic reason baby voltigore tries to start the range attack even though its model does not have sequence with range attack activity. 
	// This hack is for preventing baby voltigore to do this.
	case SCHED_RANGE_ATTACK1:
		return GetScheduleOfType(SCHED_CHASE_ENEMY);
		break;
	default:
		return CVoltigore::GetScheduleOfType(Type);
		break;
	}
}

void CBabyVoltigore::IdleSound()
{
	EmitSoundScript(idleSoundScript);
}

void CBabyVoltigore::AlertSound()
{
	EmitSoundScript(idleSoundScript);
}

void CBabyVoltigore::PlayPainSound()
{
	EmitSoundScript(painSoundScript);
}

void CBabyVoltigore::DeathSound()
{
	EmitSoundScript(dieSoundScript);
}

void CBabyVoltigore::AttackSound()
{
	EmitSoundScript(attackSoundScript);
}

#endif
