/*
    BG Particle Emitter - Server Side Entity
    Ported from BattleGrounds for halflife-featureful
*/

#ifndef PARTICLE_EMITTER_H
#define PARTICLE_EMITTER_H

// particle emitter entity
class CParticleEmitter : public CPointEntity
{
public:
	void Spawn( void );
	void Precache( void );
	void EXPORT ParticleThink( void );
	void KeyValue( KeyValueData *pkvd );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

private:
	void MakeAware( CBaseEntity *pPlayer );

	string_t m_iParticleFile;
	unsigned int m_iPresetSystem;
	bool m_bRepeatable;
};

// grass entity
class CFuncGrass : public CBaseEntity
{
public:
	void Spawn( void );
	void Precache( void );
	void EXPORT GrassThink( void );
	void KeyValue( KeyValueData *pkvd );

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

private:
	void MakeAware( CBaseEntity *pPlayer );

	string_t m_iGrassFile;
};

#endif
