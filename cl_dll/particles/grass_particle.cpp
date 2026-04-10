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

// definition of the grass particle

#include "hud.h"
#include "cl_util.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "gl_dynamic.h"
#include "pm_shared.h"
#include "pmtrace.h"
#include "pm_defs.h"
#include "particle_header.h"

#if OPENGL_AVAILABLE

extern Vector v_angles;

CGrassParticle::CGrassParticle(grass_particle_system *pSystem, particle_texture_s *pParticleTexture) : CParticle()
{	
	pSys = pSystem;
	sParticle.pTexture = pParticleTexture;
	InitValues();
}

void CGrassParticle::InitValues( void )
{
	sParticle.vPosition = pSys->vPostion;
	sParticle.vDirection = pSys->vDirection;
	sParticle.vVelocity = Vector(0, 0, 0);
	
	sParticle.iSystemId = pSys->iId;
	sParticle.iRed = 255;
	sParticle.iBlue = sParticle.iGreen = sParticle.iRed;
	sParticle.iTransparency = pSys->iTransparency;

	sParticle.flSize = pSys->flSize;

	sParticle.bIgnoreSort = pSys->bIgnoreSort;

	m_bCollisionChecked = false;
	m_bIngoreParticle = false;
	
	float flLeanMin = 90 - pSys->flLeaningMin;
	float flLeanMax = 90 - pSys->flLeaningMax;

	m_vNormal[0] = gEngfuncs.pfnRandomFloat( flLeanMin, flLeanMax );
	m_vNormal[1] = pSys->Yaw;
	m_vNormal[2] = gEngfuncs.pfnRandomFloat( 0.0, 0.0 );

	m_vWaveNormal[0] = m_vNormal[0];
	m_vWaveNormal[1] = m_vNormal[1];
	m_vWaveNormal[2] = m_vNormal[2];

	m_flLodMinDistance = GRASS_THRESHOLD_START;
	
	int OneInSix = (int)gEngfuncs.pfnRandomFloat( 0, 6 );
	if (OneInSix == 1)
		m_flLodMaxDistance = 10000.0f;
	else 
		m_flLodMaxDistance = GRASS_THRESHOLD_END;
	
	m_flParticleTime = 0.0;

	alight_t lighting;
	cl_entity_t *LightEntity;
	Vector dir;

	LightEntity = gEngfuncs.GetLocalPlayer();

	if ( !LightEntity )
		return;

	LightEntity->origin = sParticle.vPosition;
	
	lighting.plightvec = dir;

	IEngineStudio.StudioDynamicLight(LightEntity, &lighting );
	IEngineStudio.StudioSetupLighting (&lighting);
	IEngineStudio.StudioEntityLight( &lighting );
	
	sParticle.iRed = (int)(lighting.color[0] * lighting.shadelight);
	sParticle.iGreen = (int)(lighting.color[1] * lighting.shadelight);
	sParticle.iBlue = (int)(lighting.color[2] * lighting.shadelight);
}

void CGrassParticle::Draw( void )
{
	if (m_bIngoreParticle == true)
		return;

	if(sParticle.pTexture == NULL) {
		gEngfuncs.Con_Printf("Null texture in particle\n");
		return;
	}

	Vector vForward, vRight, vUp, vDir;
	AngleVectors(v_angles, vForward, vRight, vUp );
	vDir = ( sParticle.vPosition - flPlayerOrigin ).Normalize( );
	if ( DotProduct ( vDir, vForward ) < 0 )
		return;

	int iHealth = 0;

	if (sParticle.iTransparency > 255)
		sParticle.iTransparency = 255;
	if (sParticle.iTransparency < 0)
		sParticle.iTransparency = 0;
	iHealth = sParticle.iTransparency;

	if (pSys->bLOD)
	{
		float flDistance = sqrt(sParticle.flSquareDistanceToPlayer);
		if ((flDistance > m_flLodMinDistance) && (flDistance < m_flLodMaxDistance))
		{
			float flTransparencyFactor = 1 - ((flDistance - m_flLodMinDistance) / (m_flLodMaxDistance - m_flLodMinDistance));
				
			if (flTransparencyFactor > 1)
				flTransparencyFactor = 1;
			if (flTransparencyFactor < 0)
				flTransparencyFactor = 0;
			iHealth *= flTransparencyFactor;
		}
	}

	Vector vPoint, vPosition;
	Vector vWaveForward, vWaveRight, vWaveUp;

	VectorCopy( sParticle.vPosition, vPosition );

	AngleVectors(m_vNormal, vForward, vRight, vUp);
	AngleVectors(m_vWaveNormal, vWaveForward, vWaveRight, vWaveUp);

	glColor4ub(sParticle.iRed, sParticle.iGreen, sParticle.iBlue,iHealth);  

	glBindTexture(GL_TEXTURE_2D, (*sParticle.pTexture->iID));
	glBegin(GL_QUADS);

	glTexCoord2f(0, 0.95f);
	VectorMA (sParticle.vPosition, sParticle.flSize, vWaveUp, vPoint);
	VectorMA (vPoint, -sParticle.flSize, vWaveRight, vPoint);
	glVertex3fv(vPoint); 

	glTexCoord2f(0.95f, 0.95f); 
	VectorMA (sParticle.vPosition, sParticle.flSize, vWaveUp, vPoint);
	VectorMA (vPoint, sParticle.flSize, vWaveRight, vPoint);
	glVertex3fv(vPoint);

	glTexCoord2f(0.95f, 0);
	VectorMA (sParticle.vPosition, -sParticle.flSize, vUp, vPoint);
	VectorMA (vPoint, sParticle.flSize, vRight, vPoint);
	glVertex3fv(vPoint);

	glTexCoord2f(0, 0);
	VectorMA (sParticle.vPosition, -sParticle.flSize, vUp, vPoint);
	VectorMA (vPoint, -sParticle.flSize, vRight, vPoint);
	glVertex3fv(vPoint); 
   
	glEnd();
}

void CGrassParticle::Update(float flTimeSinceLastDraw)
{
	m_flParticleTime += flTimeSinceLastDraw;
	
	if (flTimeSinceLastDraw < 0)
		flTimeSinceLastDraw = -flTimeSinceLastDraw;

	if (pSys->bLOD) {	
		if (sqrt(sParticle.flSquareDistanceToPlayer) > m_flLodMaxDistance)
			m_bIngoreParticle = true;
		else
			m_bIngoreParticle = false;
	}
	
	if (pSys->flWaveSpeed > 0)	
	{
		float m_flWaveState = ((pSys->flWaveSpeed / 3) * flTimeSinceLastDraw * 
			cosf((m_flParticleTime * pSys->flWaveSpeed / 30) + 
			(sParticle.vPosition.x / 100) + (sParticle.vPosition.y / 100)));
		if (m_vWaveNormal.y > 180)
			m_vWaveNormal.x += m_flWaveState;
		else
			m_vWaveNormal.x -= m_flWaveState;
	}
	
	VectorMA( sParticle.vPosition, 60.0*flTimeSinceLastDraw, sParticle.vVelocity, sParticle.vPosition );
}

bool CGrassParticle::Test()
{
	if( !pSys )
		return false;

	if (pSys->bDropOnGround == false)
		return true;
	
	if (m_bCollisionChecked == true)
		return true;

	pmtrace_t* pTrace = NULL;
	int iIgnoreEnt = -1;	

	pTrace = gEngfuncs.PM_TraceLine( sParticle.vPosition, (sParticle.vPosition + Vector( 0, 0, -1 ) * 8192), PM_TRACELINE_PHYSENTSONLY, 2, iIgnoreEnt );
	
	float flAngleFactor = sinf(((90 - m_vNormal.x) / 180) * 3.1516f);
	
	float flGrassHeight = (sParticle.flSize * flAngleFactor ); 

	sParticle.vPosition.x = pTrace->endpos.x;
	sParticle.vPosition.y = pTrace->endpos.y;
	sParticle.vPosition.z = pTrace->endpos.z + flGrassHeight;

	m_bCollisionChecked = true;

	return true;
}

#endif // OPENGL_AVAILABLE
