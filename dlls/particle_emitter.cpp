/*
    BG Particle Emitter - Server Side Entity
    Ported from BattleGrounds for halflife-featureful
*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "particle_emitter.h"

// -- Particle Emitter --

LINK_ENTITY_TO_CLASS( env_particles, CParticleEmitter )
LINK_ENTITY_TO_CLASS( env_particleemitter, CParticleEmitter )

int gmsgBGParticle = 0;
int gmsgBGGrass = 0;

TYPEDESCRIPTION CParticleEmitter::m_SaveData[] =
{
	DEFINE_FIELD( CParticleEmitter, m_iParticleFile, FIELD_STRING ),
	DEFINE_FIELD( CParticleEmitter, m_iPresetSystem, FIELD_INTEGER ),
	DEFINE_FIELD( CParticleEmitter, m_bRepeatable, FIELD_BOOLEAN ),
};

IMPLEMENT_SAVERESTORE( CParticleEmitter, CPointEntity )

void CParticleEmitter::Spawn( void )
{
	Precache();

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects |= EF_NODRAW;

	SET_MODEL( ENT( pev ), STRING( pev->model ) );
	UTIL_SetOrigin( pev, pev->origin );

	SetThink( &CParticleEmitter::ParticleThink );
	pev->nextthink = gpGlobals->time + 3.0;
}

void CParticleEmitter::Precache( void )
{
	if( !gmsgBGParticle )
		gmsgBGParticle = REG_USER_MSG( "BGParticle", -1 );
}

void CParticleEmitter::KeyValue( KeyValueData *pkvd )
{
	if( FStrEq( pkvd->szKeyName, "particle_file" ) || FStrEq( pkvd->szKeyName, "definition_file" ) )
	{
		m_iParticleFile = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = true;
	}
	else if( FStrEq( pkvd->szKeyName, "preset_system" ) )
	{
		m_iPresetSystem = atoi( pkvd->szValue );
		pkvd->fHandled = true;
	}
	else if( FStrEq( pkvd->szKeyName, "repeatable" ) )
	{
		m_bRepeatable = atoi( pkvd->szValue ) != 0;
		pkvd->fHandled = true;
	}
	else
	{
		CPointEntity::KeyValue( pkvd );
	}
}

void CParticleEmitter::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	// When triggered, send particles to all players
	for( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBaseEntity *pPlayer = UTIL_PlayerByIndex( i );
		if( pPlayer )
		{
			MakeAware( pPlayer );
		}
	}
}

void CParticleEmitter::MakeAware( CBaseEntity *pPlayer )
{
	if( !gmsgBGParticle )
		return;

	const char *sFile = "";
	if( m_iParticleFile )
		sFile = STRING( m_iParticleFile );

	MESSAGE_BEGIN( MSG_ONE, gmsgBGParticle, NULL, pPlayer->pev );
		WRITE_BYTE( m_iPresetSystem );
		WRITE_COORD( pev->origin.x );
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );
		WRITE_COORD( pev->angles.x );
		WRITE_COORD( pev->angles.y );
		WRITE_COORD( pev->angles.z );
		WRITE_LONG( ENTINDEX( ENT( pev ) ) );
		WRITE_STRING( sFile );
	MESSAGE_END();
}

void CParticleEmitter::ParticleThink( void )
{
	// every few seconds, check for new players and make them aware
	for( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBaseEntity *pPlayer = UTIL_PlayerByIndex( i );
		if( pPlayer )
		{
			MakeAware( pPlayer );
		}
	}

	if( m_bRepeatable )
	{
		pev->nextthink = gpGlobals->time + 5.0;
	}
}

// -- Grass Entity --

LINK_ENTITY_TO_CLASS( func_grass, CFuncGrass )

TYPEDESCRIPTION CFuncGrass::m_SaveData[] =
{
	DEFINE_FIELD( CFuncGrass, m_iGrassFile, FIELD_STRING ),
};

IMPLEMENT_SAVERESTORE( CFuncGrass, CBaseEntity )

void CFuncGrass::Spawn( void )
{
	Precache();

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects |= EF_NODRAW;

	SET_MODEL( ENT( pev ), STRING( pev->model ) );
	UTIL_SetOrigin( pev, pev->origin );

	SetThink( &CFuncGrass::GrassThink );
	pev->nextthink = gpGlobals->time + 3.0;
}

void CFuncGrass::Precache( void )
{
	if( !gmsgBGGrass )
		gmsgBGGrass = REG_USER_MSG( "BGGrass", -1 );
}

void CFuncGrass::KeyValue( KeyValueData *pkvd )
{
	if( FStrEq( pkvd->szKeyName, "grass_file" ) )
	{
		m_iGrassFile = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = true;
	}
	else
	{
		CBaseEntity::KeyValue( pkvd );
	}
}

void CFuncGrass::MakeAware( CBaseEntity *pPlayer )
{
	if( !gmsgBGGrass )
		return;

	const char *sFile = "";
	if( m_iGrassFile )
		sFile = STRING( m_iGrassFile );

	MESSAGE_BEGIN( MSG_ONE, gmsgBGGrass, NULL, pPlayer->pev );
		WRITE_COORD( pev->origin.x );
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );
		WRITE_COORD( pev->angles.x );
		WRITE_COORD( pev->angles.y );
		WRITE_COORD( pev->angles.z );
		WRITE_COORD( pev->absmin.x );
		WRITE_COORD( pev->absmin.y );
		WRITE_COORD( pev->absmin.z );
		WRITE_COORD( pev->absmax.x );
		WRITE_COORD( pev->absmax.y );
		WRITE_COORD( pev->absmax.z );
		WRITE_LONG( ENTINDEX( ENT( pev ) ) );
		WRITE_STRING( sFile );
	MESSAGE_END();
}

void CFuncGrass::GrassThink( void )
{
	for( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBaseEntity *pPlayer = UTIL_PlayerByIndex( i );
		if( pPlayer )
		{
			MakeAware( pPlayer );
		}
	}

	pev->nextthink = gpGlobals->time + 5.0;
}
