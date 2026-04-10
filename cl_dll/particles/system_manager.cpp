/*
    Copyright 2001 to 2004. The Battle Grounds Team and Contributors

    This file is part of the Battle Grounds Modification for Half-Life.

    The Battle Grounds Modification for Half-Life is free software;
    you can redistribute it and/or modify it under the terms of the
    GNU Lesser General Public License as published by the Free
    Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.

    The Battle Grounds Modification for Half-Life is distributed in
    the hope that it will be useful, but WITHOUT ANY WARRANTY; without
    even the implied warranty of MERCHANTABILITY or FITNESS FOR A
    PARTICULAR PURPOSE.  See the GNU Lesser General Public License
    for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with The Battle Grounds Modification for Half-Life;
    if not, write to the Free Software Foundation, Inc., 59 Temple Place,
    Suite 330, Boston, MA  02111-1307  USA

    You must obey the GNU Lesser General Public License in all respects for
    all of the code used other than code distributed with the Half-Life
    SDK developed by Valve.  If you modify this file, you may extend this
    exception to your version of the file, but you are not obligated to do so.
    If you do not wish to do so, delete this exception statement from your
    version.
*/

// definition of the particle system manager

#include "hud.h"
#include "cl_util.h"
#include <string.h>
#include <stdio.h>
#include "gl_dynamic.h"

#include "particle_header.h"
#include "event_api.h"
#include "r_efx.h"
#include "pm_shared.h"

#if OPENGL_AVAILABLE

CParticleSystemManager *pParticleManager = NULL;
cvar_t* g_ParticleCount = NULL;
cvar_t* g_ParticleDebug = NULL;
cvar_t* g_ParticleSorts = NULL;
Vector flPlayerOrigin;

// updates all systems
void CParticleSystemManager::UpdateSystems( void )
{
	// Update the player origin each frame
	cl_entity_t *pLocal = gEngfuncs.GetLocalPlayer();
	if( pLocal )
	{
		flPlayerOrigin = pLocal->origin;
	}

	CParticleSystem *pSystem = NULL;
	signed int i = 0;
	signed int iSystems = (signed)m_pParticleSystems.size();
	for (; i < iSystems; i++)
	{
		pSystem = m_pParticleSystems[i];
		if( pSystem && pSystem->DrawSystem() == false)
		{
			RemoveSystem(pSystem->SystemID());
			i--;
			iSystems--;
		}
	}

	if(CheckDrawSystem() == false)
		return;

	// prepare opengl
	Particle_InitOpenGL();

	int iParticles = m_pUnsortedParticles.size();
	float flTimeSinceLastDraw = TimeSinceLastDraw();
	int iDrawn = 0;

	// draw all unsorted particles first, so they are at the back of the screen.
	if(iParticles > 0) {
		CParticle *pParticle = NULL;
		for(i = 0; i < iParticles ; i++) {
			if(m_pUnsortedParticles[i]) {
				pParticle = m_pUnsortedParticles[i];
				if(pParticle && pParticle->Test()) {
					pParticle->Update(flTimeSinceLastDraw);

					if(g_iUser1 != OBS_MAP_FREE && g_iUser1 != OBS_MAP_CHASE) {
						pParticle->Prepare(); 
						pParticle->Draw();
						iDrawn++;
					}
				} else {
					RemoveParticle(pParticle);
					i--;
					iParticles--;
				}
			}
		}
	}

	iParticles = m_pParticles.size();

	// sort and draw the sorted particles list
	if(iParticles > 0) {
		float flTimeSinceLastSort = (gEngfuncs.GetClientTime() - m_flLastSort);
		if(g_ParticleSorts->value > 0 && flTimeSinceLastSort >= 1.0f / g_ParticleSorts->value) {
			m_flLastSort = gEngfuncs.GetClientTime();
			std::sort(m_pParticles.begin(), m_pParticles.end(), less_than);
		}

		CParticle *pParticle = NULL;
		for(i = 0; i < iParticles ; i++) {
			if(m_pParticles[i]) {
				pParticle = m_pParticles[i];
				if(pParticle && pParticle->Test()) {
					pParticle->Update(flTimeSinceLastDraw);

					if(g_iUser1 != OBS_MAP_FREE && g_iUser1 != OBS_MAP_CHASE) {
						pParticle->Prepare(); 
						pParticle->Draw();
						iDrawn++;
					}
				} else {
					RemoveParticle(pParticle);
					i--;
					iParticles--;
				}
			}
		}
	}

	// finished particle drawing
	Particle_FinishOpenGL();

	// print out how fast we've been drawing the systems in debug mode
	if (g_ParticleDebug && g_ParticleDebug->value != 0 && ((m_flLastDebug + 1) <= gEngfuncs.GetClientTime()))
	{
		gEngfuncs.Con_Printf("%i Particles Drawn this pass in %i systems %i Textures in Cache\n", iDrawn, (int)m_pParticleSystems.size(), (int)m_pTextures.size());
		m_flLastDebug = gEngfuncs.GetClientTime();
	}

	m_flLastDraw = gEngfuncs.GetClientTime();
}

// handles all the preset particle systems
void CParticleSystemManager::CreatePresetPS(unsigned int iPreset, particle_system_management *pSystem)
{
	// cannons, mortar, barrels exploding, etc
	if(iPreset == iDefaultExplosion) {
		CreateMappedPS("particles/explo1_darksmoke.txt", pSystem);
		CreateMappedPS("particles/explo1_grounddust.txt", pSystem);
		CreateMappedPS("particles/explo1_firedust.txt", pSystem);
		CreateMappedPS("particles/explo1_fire.txt", pSystem);
		CreateMappedPS("particles/explo1_shockwave.txt", pSystem);

		if(pSystem == NULL) {
			return;
		}

		gEngfuncs.pEventAPI->EV_PlaySound( 0, pSystem->vPosition, 0, "sound/weapons/cannon/cannon_explode.wav", 1.0, ATTN_NONE, 0, PITCH_NORM );

		dlight_t *dl = gEngfuncs.pEfxAPI->CL_AllocDlight (0);
		VectorCopy (pSystem->vPosition, dl->origin);
		dl->radius = 500;
		dl->color.r = 254;
		dl->color.g = 160;
		dl->color.b = 24;
		dl->decay = 0.2;
		dl->die = (gEngfuncs.GetClientTime() + 0.1);
	}
	
	// Capture Smoke Brits
	if(iPreset == iDefaultRedSmoke) {
		CreateMappedPS("particles/capture_red.txt", pSystem);

		if(pSystem == NULL) {
			return;
		}
	}

	// Capture Smoke Americans
	if(iPreset == iDefaultBlueSmoke) {
		CreateMappedPS("particles/capture_blue.txt", pSystem);

		if(pSystem == NULL) {
			return;
		}
	}
	// Cannon Fire Smoke
	if(iPreset == iDefaultCannonSmoke) 
	{
		CreateBarrelPS(pSystem->vPosition, pSystem->vDirection);
		CreateMappedPS("particles/explo1_darksmoke.txt", pSystem);
		CreateMappedPS("particles/explo1_grounddust.txt", pSystem);

		gEngfuncs.pEventAPI->EV_PlaySound( 0, pSystem->vPosition, 0, "sound/weapons/cannon/cannon_fire.wav", 1.0, ATTN_NONE, 0, PITCH_NORM );
		
		if(pSystem == NULL) 
		{
			return;
		}
	}
}

// wrappers to create particle systems
void CParticleSystemManager::CreateFlintPS(Vector vPosition)
{
	if(CheckDrawSystem() == false)
		return;

	AddSystem(new CFlintlockSmokeParticleSystem(vPosition));
}

void CParticleSystemManager::CreateBarrelPS(Vector vPosition, Vector vDirection)
{
	if(CheckDrawSystem() == false)
		return;

	AddSystem(new CBarrelSmokeParticleSystem(vPosition, vDirection));
}

void CParticleSystemManager::CreateSparkPS(Vector vPosition, Vector vDirection)
{
	if(CheckDrawSystem() == false)
		return;

	AddSystem(new CSparkParticleSystem(vPosition, vDirection));
}

void CParticleSystemManager::CreateWhitePS(Vector vPosition, Vector vDirection)
{
	if(CheckDrawSystem() == false)
		return;

	AddSystem(new CWhiteSmokeParticleSystem(vPosition, vDirection));
}

void CParticleSystemManager::CreateBrownPS(Vector vPosition, Vector vDirection)
{
	if(CheckDrawSystem() == false)
		return;

	AddSystem(new CBrownSmokeParticleSystem(vPosition, vDirection));
}

void CParticleSystemManager::CreateGrassPS( const char* sFile, particle_system_management* pSystem )
{
	if(pSystem == NULL) {
		return;
	}

	if (IEngineStudio.IsHardware() == false)
		return;

	AddSystem(new CGrassParticleSystem(sFile, pSystem));
}

void CParticleSystemManager::CreateMappedPS( const char* sFile, particle_system_management* pSystem )
{
	if(pSystem == NULL) {
		return;
	}
	if (IEngineStudio.IsHardware() == false)
		return;

	AddSystem(new CMappedParticleSystem(sFile, pSystem));
}

// are we allowed to draw atm
bool CParticleSystemManager::CheckDrawSystem( void )
{
	if (gHUD.m_iHideHUDDisplay & (HIDEHUD_ALL))
		return false;

	if(g_iUser1 == OBS_MAP_FREE || g_iUser1 == OBS_MAP_CHASE)
		return false;

	if (IEngineStudio.IsHardware() == false)
		return false;

	return true;
}

// adds a new texture to our cache
void CParticleSystemManager::AddTexture(const char* sName, particle_texture_s *pTexture) {
	particle_texture_cache *pCacheEntry = new particle_texture_cache;
	snprintf(pCacheEntry->sTexture, MAX_PARTICLE_PATH-1, "%s", sName);
	pCacheEntry->pTexture = pTexture;

	m_pTextures.push_back(pCacheEntry);
}

// check for a texture with the same path
particle_texture_s* CParticleSystemManager::HasTexture(const char* sName) {
	unsigned int i = 0;
	unsigned int iTextures = m_pTextures.size();
	particle_texture_cache *pCacheEntry = NULL;

	for (; i < iTextures; i++)
	{
		pCacheEntry = m_pTextures[i];
		if(!stricmp(pCacheEntry->sTexture, sName)) {
			return pCacheEntry->pTexture;
		}
	}
	return NULL;
}

// cache the most used tgas
void CParticleSystemManager::PrecacheTextures( void ) {
	gEngfuncs.Con_Printf("Caching frequently used particles, this may take a few moments\n");
	LoadTGA(NULL, FLINTLOCK_SMOKE_PARTICLE);
	LoadTGA(NULL, BARREL_SMOKE_PARTICLES[0]);
	LoadTGA(NULL, BARREL_SMOKE_PARTICLES[1]);
	LoadTGA(NULL, BARREL_SMOKE_PARTICLES[2]);
	LoadTGA(NULL, BROWN_SMOKE_PARTICLE);
	gEngfuncs.Con_Printf("Finished caching frequently used particles, game loading will now continue\n");
}

// deletes all textures and their entries
void CParticleSystemManager::RemoveTextures( void ) {
	unsigned int i = 0;
	unsigned int iTextures = m_pTextures.size();
	particle_texture_cache *pCacheEntry = NULL;

	for (; i < iTextures; i++) {
		pCacheEntry = m_pTextures[i];
		if(pCacheEntry) {
			delete pCacheEntry->pTexture;
			pCacheEntry->pTexture = NULL;
		}
		memset(pCacheEntry->sTexture, 0, MAX_PARTICLE_PATH);
		delete pCacheEntry;
		pCacheEntry = NULL;
		m_pTextures.erase(m_pTextures.begin());
		i--;
		iTextures--;
	}

	m_pTextures.clear();
}

// adds a particle into the global particle tracker
void CParticleSystemManager::AddParticle(CParticle* pParticle) {
	if(pParticle->sParticle.bIgnoreSort == true) {
		m_pUnsortedParticles.push_back(pParticle);
	} else {
		m_pParticles.push_back(pParticle);
	}
	pParticle = NULL;
}

// removes a particle from the global tracker and from the system
void CParticleSystemManager::RemoveParticle(CParticle* pParticle) {
	unsigned int i = 0;
	unsigned int iParticles = m_pParticles.size();

	for (; i < iParticles; i++) {
		if(pParticle == m_pParticles[i]) {
			delete m_pParticles[i];
			pParticle = NULL;
			m_pParticles.erase(m_pParticles.begin() + i);
			i--;
			iParticles--;
			return;
		}
	}

	iParticles = m_pUnsortedParticles.size();
	for (i = 0; i < iParticles; i++) {
		if(pParticle == m_pUnsortedParticles[i]) {
			delete m_pUnsortedParticles[i];
			pParticle = NULL;
			m_pUnsortedParticles.erase(m_pUnsortedParticles.begin() + i);
			i--;
			iParticles--;
			return;
		}
	}
}

// remove all trackers in the system
void CParticleSystemManager::RemoveParticles()  {
	unsigned int i = 0;
	unsigned int iParticles = m_pParticles.size();

	for (i = 0; i < iParticles; i++) {
		delete m_pParticles[i];
		m_pParticles[i] = NULL;
		m_pParticles.erase(m_pParticles.begin() + i);
		i--;
		iParticles--;
	}
	m_pParticles.clear();

	iParticles = m_pUnsortedParticles.size();
	for (i = 0; i < iParticles; i++) {
		delete m_pUnsortedParticles[i];
		m_pUnsortedParticles[i] = NULL;
		m_pUnsortedParticles.erase(m_pUnsortedParticles.begin() + i);
		i--;
		iParticles--;
	}
	m_pUnsortedParticles.clear();
}

// adds a new system
void CParticleSystemManager::AddSystem(CParticleSystem *pSystem) {
	m_pParticleSystems.push_back(pSystem);
}

void CParticleSystemManager::RemoveSystem( unsigned int iSystem )
{
	unsigned int i = 0;
	unsigned int iParticles = m_pParticles.size();
	CParticle *pParticle = NULL;
	for (i = 0; i < iParticles; i++) {
		pParticle = m_pParticles[i];
		if(pParticle && pParticle->SystemID() == iSystem) {
			delete pParticle;
			pParticle = NULL;
			m_pParticles.erase(m_pParticles.begin() + i);
			i--;
			iParticles--;
		}
	}

	iParticles = m_pUnsortedParticles.size();
	for (i = 0; i < iParticles; i++) {
		pParticle = m_pUnsortedParticles[i];
		if(pParticle && pParticle->SystemID() == iSystem) {
			delete pParticle;
			pParticle = NULL;
			m_pUnsortedParticles.erase(m_pUnsortedParticles.begin() + i);
			i--;
			iParticles--;
		}
	}

	CParticleSystem *pSystem = NULL;	
	unsigned int iSystems = m_pParticleSystems.size();
	for (i = 0; i < iSystems; i++)
	{
		pSystem = m_pParticleSystems[i];
		if(pSystem && pSystem->SystemID() == iSystem) {
			delete pSystem;
			pSystem = NULL;
			m_pParticleSystems.erase(m_pParticleSystems.begin() + i);
			i--;
			iSystems--;
		}
	}
}

// deletes all systems
void CParticleSystemManager::RemoveSystems( void )
{
	unsigned int i = 0;
	unsigned int iSystems = m_pParticleSystems.size();
	for (; i < iSystems; i++) {
		delete m_pParticleSystems[i];
		m_pParticleSystems[i] = NULL;
		m_pParticleSystems.erase(m_pParticleSystems.begin() + i);
		i--;
		iSystems--;
	}
	m_pParticleSystems.clear();
}

#endif // OPENGL_AVAILABLE
