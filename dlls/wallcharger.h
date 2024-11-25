#pragma once
#ifndef WALLCHARGER_H
#define WALLCHARGER_H

#include "cbase.h"

class CWallCharger : public CBaseEntity
{
public:
	void Spawn();
	void Precache( void );
	void EXPORT Off( void );
	void EXPORT Recharge( void );

	virtual int RechargeTime() = 0;
	virtual int ChargerCapacity() = 0;

	virtual bool GiveCharge(CBaseEntity* pActivator) = 0;
	virtual bool AllowNoSuit() { return false; }

	virtual const NamedSoundScript& LoopingSoundScript() = 0;
	virtual const NamedSoundScript& DenySoundScript() = 0;
	virtual const NamedSoundScript& ChargeStartSoundScript() = 0;
	virtual const NamedSoundScript& RechargeSoundScript() = 0;

	const char* CustomLoopingSound();
	const char* CustomDenySound();
	const char* CustomChargeStartSound();
	const char* CustomRechargeSound();

	void PlayChargerSound(const NamedSoundScript& soundScript, const char* customSample);
	void StopChargerSound(const NamedSoundScript& soundScript, const char* customSample);

	void KeyValue( KeyValueData *pkvd );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual int ObjectCaps( void );
	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );

	static TYPEDESCRIPTION m_SaveData[];

	float m_flNextCharge;
	int m_iReactivate ; // DeathMatch Delay until reactvated
	int m_iJuice;
	int m_iOn;			// 0 = off, 1 = startup, 2 = going
	float m_flSoundTime;
	string_t m_triggerOnFirstUse;
	string_t m_triggerOnEmpty;
	string_t m_triggerOnRecharged;

	int OnStateFrame();
	int OffStateFrame();

	bool CalcRatio( CBaseEntity *pLocus, float* outResult );
	bool IsUsefulToDisplayHint(CBaseEntity* pPlayer) { return m_iJuice > 0; }
};
#endif
