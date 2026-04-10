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

// definition of the mapped particle system

#include "hud.h"
#include "cl_util.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "particle_header.h"

#if OPENGL_AVAILABLE

// creates a new mapped particle system
CMappedParticleSystem::CMappedParticleSystem( char *sParticleDefinition, particle_system_management *pSysDetails ) : CParticleSystem()
{
	m_sParticleFile = sParticleDefinition;
	m_iID = pSysDetails->iID;

	CreateDefaultParticleSystem();

	m_pSystem->vPostion = pSysDetails->vPosition;
	m_pSystem->vDirection = pSysDetails->vDirection;

	if(LoadParticleDefinition()) {
		m_iStartingParticles = m_pSystem->iStartingParticles;
		m_flNewParticles = m_pSystem->flNewParticlesPerSecond;
		m_flSystemMaxAge = m_pSystem->flSystemLifetime;
		m_pSystem->iId = m_iID;

		pParticleTexture = LoadTGA(NULL, m_pSystem->sParticleTexture);

		int iParticles = StartingParticles();
		CParticle *pParticle = NULL;
		for(int i = 0; i <iParticles; i++) {
			pParticle = new CMappedParticle(m_pSystem, pParticleTexture);
			AddParticle(pParticle);
		}
	} else {
		m_pSystem->flSystemLifetime = 0.01f;
		m_flSystemMaxAge = 0.01f;
	}
}

CMappedParticleSystem::~CMappedParticleSystem() 
{
	delete m_pSystem;
	m_pSystem = NULL;
}

bool CMappedParticleSystem::TestSystem( void )
{
	if(m_pSystem->flSystemLifetime == -1.0)
		return true;

	return CParticleSystem::TestSystem();
}

void CMappedParticleSystem::UpdateSystem( void )
{
	CParticle *pParticle = NULL;
	CParticleSystem::UpdateSystem();
	m_flParticleCreationTime += TimeSinceLastDraw();

	float flParticleDelay = NewParticlesCreationDelay();

	float flLongestLife = m_flSystemsAge + m_pSystem->flParticleLife + m_pSystem->flParticleLifeVariation;
	pParticleTexture = LoadTGA(NULL, m_pSystem->sParticleTexture);

	if(m_pSystem->flSystemLifetime == -1.0) 
	{
		if (m_flParticleCreationTime > 3)
			m_flParticleCreationTime = 3;
		
		while(m_flParticleCreationTime > 0) 
		{
			pParticle = new CMappedParticle(m_pSystem, pParticleTexture);
			AddParticle(pParticle);
			m_flParticleCreationTime -= (1.0f / flParticleDelay);
		}
	}
	else {
		while((m_flParticleCreationTime > 0) && (m_flSystemsAge < (0.9999f * m_flSystemMaxAge))) {
			pParticle = new CMappedParticle(m_pSystem, pParticleTexture);
			AddParticle(pParticle);

			if((m_pSystem->iGenerationFalloff == LINEAR_HALF_LIFETIME) &&  (flLongestLife > (m_flSystemMaxAge * 0.5))) {
				m_flParticleCreationTime -= 1.0f / ( flParticleDelay * (1 - ((flLongestLife - (0.5 * m_flSystemMaxAge)) / m_flSystemMaxAge)));
			}
			else if((m_pSystem->iGenerationFalloff == LINEAR_QUARTER_LIFETIME) && (flLongestLife > (m_flSystemMaxAge * 0.25))) {
				m_flParticleCreationTime -= 1.0f / ( flParticleDelay * (1 - ((flLongestLife - (0.25 * m_flSystemMaxAge)) / m_flSystemMaxAge)));
			}
			else {
				m_flParticleCreationTime -= (1.0f / flParticleDelay);
			}
		}
	}
}

void CMappedParticleSystem::CreateDefaultParticleSystem( void )
{
	m_pSystem = new mapped_particle_system;

	m_pSystem->iStartingParticles = 5;
	m_pSystem->flNewParticlesPerSecond = 5.0;
	m_pSystem->iGenerationFalloff = NO_FALLOFF;

	snprintf(m_pSystem->sParticleTexture, MAX_PARTICLE_PATH, "particles/smoke1.tga");

	m_pSystem->flParticleScaleSize = 1.0;
	m_pSystem->flScaleIncreasePerSecond = 1.0;
	m_pSystem->flGravity = -1.0;
	m_pSystem->flParticleRotationSpeed = 0.0;

	m_pSystem->flParticleLife = 5.0;
	m_pSystem->flParticleLifeVariation = 1.0;
	m_pSystem->flSystemLifetime = -1.0;

	m_pSystem->vStartingVel.x = 0.0;
	m_pSystem->vStartingVel.y = 0.0;
	m_pSystem->vStartingVel.z = 5.0;
	m_pSystem->vVelocityVar.x = 2.0;
	m_pSystem->vVelocityVar.y = 2.0;
	m_pSystem->vVelocityVar.z = 2.0;
	m_pSystem->flVelocityDampening = 1.0;

	m_pSystem->iTransparency = 255;
	m_pSystem->iDisplayMode = THIRTY_TWO_BIT;
    
	m_pSystem->iSystemShape = SHAPE_POINT;
	m_pSystem->iPlaneXLength = 5;
	m_pSystem->iPlaneYLength = 5;

	m_pSystem->iAnimBehaviour = ONCE_THROUGH;
	m_pSystem->iAnimSpeed = ANIMATE_OVER_LIFE;
	m_pSystem->iFPS = 10;
	m_pSystem->iStartingFrame = 1;
	m_pSystem->iEndingFrame = 1;
	m_pSystem->iFramesPerTexture = 1;

	m_pSystem->iParticleCollision = PARTICLE_PASS_THROUGH;
	snprintf(m_pSystem->sParticleNewSys, MAX_PARTICLE_PATH, "%s", "");

	m_pSystem->iParticleLightCheck = NO_CHECK;
	
	m_pSystem->iParticleAlign = PLAYER_VIEW;

	m_pSystem->vRotationVel.x = 0.0f;
	m_pSystem->vRotationVel.y = 0.0f;
	m_pSystem->vRotationVel.z = 0.0f;
	m_pSystem->vRotationVelVarMax.x = 0.0f;
	m_pSystem->vRotationVelVarMax.y = 0.0f;
	m_pSystem->vRotationVelVarMax.z = 0.0f;

	m_pSystem->bWindy = true;
	m_pSystem->bFadeIn = true;
	m_pSystem->bFadeOut = true;
	m_pSystem->bSmoke = false;
	m_pSystem->bIgnoreSort = false;
}

bool CMappedParticleSystem::LoadParticleDefinition( void )
{
	if(!m_sParticleFile) {
		gEngfuncs.Con_Printf("No Mapped Particle definition file specified");
		return false;
	}

	char *sFile = (char *)gEngfuncs.COM_LoadFile(m_sParticleFile, 5 , NULL);
	if(!sFile) {
		gEngfuncs.Con_Printf("Bad Mapped Particle definition file specified %s\n", m_sParticleFile);
		return false;
	}

	char sSetting[256];
	char sValue[256];

	bool bLooping = true;
	while(bLooping) {
		sFile = gEngfuncs.COM_ParseFile(sFile, sSetting);

		if(!sFile) {
			break;
		}

		if(!sSetting) {
			gEngfuncs.Con_Printf("Unexpected error in file after %sin %s", sValue, m_sParticleFile);
			gEngfuncs.COM_FreeFile(sFile);
			return false;
		}

		sFile = gEngfuncs.COM_ParseFile(sFile, sValue);

		if(!sValue) {
			gEngfuncs.Con_Printf("Unexpected error in file after %s in %s", sSetting, m_sParticleFile);
			gEngfuncs.COM_FreeFile(sFile);
			return false;
		}

		int iTemp = 0;
		float flTemp = 0.0;

		if(!stricmp(sSetting, "starting_particles")) {
			m_pSystem->iStartingParticles = atoi(sValue);
		}
		else if(!stricmp(sSetting, "new_particles_per_second")) {
			m_pSystem->flNewParticlesPerSecond = atof(sValue);
		}
		else if(!stricmp(sSetting, "texture")) {
			snprintf(m_pSystem->sParticleTexture, MAX_PARTICLE_PATH, "%s", sValue);
		}
		else if(!stricmp(sSetting, "scale")) {
			m_pSystem->flParticleScaleSize = atof(sValue);
		}
		else if(!stricmp(sSetting, "growth")) {
			m_pSystem->flScaleIncreasePerSecond = atof(sValue);
		}
		else if(!stricmp(sSetting, "lifetime")) {
			m_pSystem->flParticleLife = atof(sValue);
		} 
		else if(!stricmp(sSetting, "lifetime_var")) {
			m_pSystem->flParticleLifeVariation = atof(sValue);
		}
		else if(!stricmp(sSetting, "rotation_speed")) {
			m_pSystem->flParticleRotationSpeed = atof(sValue);
		} 
		else if(!stricmp(sSetting, "gravity")) {
			m_pSystem->flGravity = atof(sValue);
		}
		else if(!stricmp(sSetting, "starting_velocity_x")) {
			m_pSystem->vStartingVel.x = atof(sValue);
		} else if(!stricmp(sSetting, "starting_velocity_y")) {
			m_pSystem->vStartingVel.y = atof(sValue);
		} else if(!stricmp(sSetting, "starting_velocity_z")) {
			m_pSystem->vStartingVel.z = atof(sValue);
		}
		else if(!stricmp(sSetting, "velocity_var_x")) {
			m_pSystem->vVelocityVar.x = atof(sValue);
		} else if(!stricmp(sSetting, "velocity_var_y")) {
			m_pSystem->vVelocityVar.y = atof(sValue);
		} else if(!stricmp(sSetting, "velocity_var_z")) {
			m_pSystem->vVelocityVar.z = atof(sValue);
		}
		else if(!stricmp(sSetting, "damping")) {
			m_pSystem->flVelocityDampening = atof(sValue);
		}
		else if(!stricmp(sSetting, "transparency")) {
			iTemp = atoi(sValue);
			if(iTemp > 255 || iTemp < 0)
				m_pSystem->iTransparency = 255;
			else
				m_pSystem->iTransparency = iTemp;
		}
		else if(!stricmp(sSetting, "system_life")) {
			flTemp = atof(sValue);
			if(flTemp < 0.0)
				flTemp = -1.0;
			m_pSystem->flSystemLifetime = flTemp;
		}
		else if(!stricmp(sSetting, "display_mode")) {
			if(!stricmp(sValue, "24"))
				m_pSystem->iDisplayMode = TWENTY_FOUR_BIT_ADDITIVE;
			else
				m_pSystem->iDisplayMode = THIRTY_TWO_BIT;
		}
		else if(!stricmp(sSetting, "emitter_shape")) {
			if(!stricmp(sValue, "plane"))
				m_pSystem->iSystemShape = SHAPE_PLANE;
			else if(!stricmp(sValue, "around_player"))
				m_pSystem->iSystemShape = SHAPE_AROUND_PLAYER;
			else
				m_pSystem->iSystemShape = SHAPE_POINT;
		}
		else if(!stricmp(sSetting, "plane_x_length")) {
			m_pSystem->iPlaneXLength = atoi(sValue);
		}
		else if(!stricmp(sSetting, "plane_y_length")) {
			m_pSystem->iPlaneYLength = atoi(sValue);
		}
		else if(!stricmp(sSetting, "anim_speed")) {
			if(!stricmp(sValue, "fast_to_slow"))
				m_pSystem->iAnimSpeed = START_FAST_END_SLOW;
			else if(!stricmp(sValue, "custom"))
				m_pSystem->iAnimSpeed = CUSTOM;
			else
				m_pSystem->iAnimSpeed = ANIMATE_OVER_LIFE;
		}
		else if(!stricmp(sSetting, "fps")) {
			m_pSystem->iFPS = abs(atoi(sValue));
		}
		else if(!stricmp(sSetting, "starting_frame")) {
			m_pSystem->iStartingFrame = abs(atoi(sValue));
		}
		else if(!stricmp(sSetting, "ending_frame")) {
			m_pSystem->iEndingFrame = abs(atoi(sValue));
		}
		else if(!stricmp(sSetting, "frames_in_image")) {
			flTemp = (float)abs((int)atof(sValue));
			if((int)sqrt(flTemp) == (int)fabs(sqrt(flTemp))) {
				if(flTemp <= 0.0f)
					flTemp = 1.0f;
				m_pSystem->iFramesPerTexture = (int)flTemp;
			}
			else {
				gEngfuncs.Con_Printf("Frames in Image (%i), not square in %s", (int)flTemp, m_sParticleFile);
			}
		}
		else if(!stricmp(sSetting, "loop_behaviour")) {
			if(!stricmp(sValue, "loop"))
				m_pSystem->iAnimBehaviour = LOOP;
			else if(!stricmp(sValue, "reverse"))
				m_pSystem->iAnimBehaviour = REVERSE_LOOP;
			else
				m_pSystem->iAnimBehaviour = ONCE_THROUGH;
		}
		else if(!stricmp(sSetting, "collision")) {
			if(!stricmp(sValue, "stuck"))
				m_pSystem->iParticleCollision = PARTICLE_STUCK;
			else if(!stricmp(sValue, "die"))
				m_pSystem->iParticleCollision = PARTICLE_DIE;
			else if(!stricmp(sValue, "bounce"))
				m_pSystem->iParticleCollision = PARTICLE_BOUNCE;
			else if(!stricmp(sValue, "splash"))
				m_pSystem->iParticleCollision = PARTICLE_SPLASH;
			else if(!stricmp(sValue, "off"))
				m_pSystem->iParticleCollision = PARTICLE_PASS_THROUGH;
			else {
				m_pSystem->iParticleCollision = PARTICLE_NEW_SYSTEM;
				snprintf(m_pSystem->sParticleNewSys, MAX_PARTICLE_PATH, "%s", sValue);
			}
		}
		else if(!stricmp(sSetting, "falloff")) {
			if(!stricmp(sValue, "lifetime"))
				m_pSystem->iGenerationFalloff = LINEAR_LIFETIME;
			else if(!stricmp(sValue, "half"))
				m_pSystem->iGenerationFalloff = LINEAR_HALF_LIFETIME;
			else if(!stricmp(sValue, "quarter"))
				m_pSystem->iGenerationFalloff = LINEAR_QUARTER_LIFETIME;
			else
				m_pSystem->iGenerationFalloff = NO_FALLOFF;
		}
		else if(!stricmp(sSetting, "light_check")) {
			if(!stricmp(sValue, "never"))
				m_pSystem->iParticleLightCheck = NO_CHECK;
			else if(!stricmp(sValue, "once"))
				m_pSystem->iParticleLightCheck = CHECK_ONCE;
			else if(!stricmp(sValue, "every_sort"))
				m_pSystem->iParticleLightCheck = CHECK_EVERY_SORT;
			else
				m_pSystem->iParticleLightCheck = NO_CHECK;
		}
		else if(!stricmp(sSetting, "particle_align")) {
			if(!stricmp(sValue, "player_view"))
				m_pSystem->iParticleAlign = PLAYER_VIEW;
			else if(!stricmp(sValue, "locked_z"))
				m_pSystem->iParticleAlign = LOCKED_Z;
			else if(!stricmp(sValue, "planar"))
				m_pSystem->iParticleAlign = PLANAR;
			else if(!stricmp(sValue, "none"))
				m_pSystem->iParticleAlign = NO_ALIGN;
			else if(!stricmp(sValue, "velocity_vector"))
				m_pSystem->iParticleAlign = VELOCITY_VECTOR;
			else
				m_pSystem->iParticleAlign = PLAYER_VIEW;
		}
		else if(!stricmp(sSetting, "rotation_velocity_pitch")) {
			m_pSystem->vRotationVel.x = atof(sValue);
		}
		else if(!stricmp(sSetting, "rotation_velocity_yaw")) {
			m_pSystem->vRotationVel.y = atof(sValue);
		}
		else if(!stricmp(sSetting, "rotation_velocity_roll")) {
			m_pSystem->vRotationVel.z = atof(sValue);
		}
		else if(!stricmp(sSetting, "rotation_velocity_var_pitch")) {
			m_pSystem->vRotationVelVarMax.x = atof(sValue);
		}
		else if(!stricmp(sSetting, "rotation_velocity_var_yaw")) {
			m_pSystem->vRotationVelVarMax.y = atof(sValue);
		}
		else if(!stricmp(sSetting, "rotation_velocity_var_roll")) {
			m_pSystem->vRotationVelVarMax.z = atof(sValue);
		}
		else if(!stricmp(sSetting, "wind")) {
			m_pSystem->bWindy = (!!(atoi(sValue)));
		}
		else if(!stricmp(sSetting, "fade_in")) {
			m_pSystem->bFadeIn = (!!(atoi(sValue)));
		}
		else if(!stricmp(sSetting, "fade_out")) {
			m_pSystem->bFadeOut = (!!(atoi(sValue)));
		}
		else if(!stricmp(sSetting, "smoke")) {
			m_pSystem->bSmoke = (!!(atoi(sValue)));
		}
		else if(!stricmp(sSetting, "ignore_sort")) {
			m_pSystem->bIgnoreSort = (!!(atoi(sValue)));
		}
		else {
			gEngfuncs.Con_Printf("Unknown setting - %s in %s\n", sSetting, m_sParticleFile);
		}
	}

	gEngfuncs.COM_FreeFile(sFile);
	return true;
}

#endif // OPENGL_AVAILABLE
