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

// definition of the grass particle system

#include "hud.h"
#include "cl_util.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "gl_dynamic.h"
#include "particle_header.h"

#if OPENGL_AVAILABLE

CGrassParticleSystem::CGrassParticleSystem( const char *sParticleDefinition, particle_system_management *pSysDetails ) : CParticleSystem()
{
	m_sParticleFile = sParticleDefinition;
	m_iID = pSysDetails->iID;
	m_flSystemMaxAge = 0.0;

	if(LoadParticleDefinition(pSysDetails))
	{
		unsigned int iGrassTypes = m_cGrassTypes.size();
		for (unsigned int i = 0; i < iGrassTypes; i++)
		{
			grass_particle_types *pGrassType = m_cGrassTypes[i];
			if(!pGrassType || !pGrassType->pSystem)
				continue;

			pGrassType->pSystem->iId = m_iID;

			float flXRange = pGrassType->pSystem->vAbsMax.x - pGrassType->pSystem->vAbsMin.x;
			float flYRange = pGrassType->pSystem->vAbsMax.y - pGrassType->pSystem->vAbsMin.y;

			for(unsigned int j = 0; j < pGrassType->pSystem->iCount; j++)
			{
				grass_particle_system *pNew = new grass_particle_system;
				memcpy(pNew, pGrassType->pSystem, sizeof(grass_particle_system));

				pNew->vPostion.x = pGrassType->pSystem->vAbsMin.x + gEngfuncs.pfnRandomFloat( 0, flXRange);
				pNew->vPostion.y = pGrassType->pSystem->vAbsMin.y + gEngfuncs.pfnRandomFloat( 0, flYRange);
				pNew->vPostion.z = pGrassType->pSystem->vPostion.z;
				pNew->Yaw = gEngfuncs.pfnRandomFloat( 0, 360 );

				CParticle *pParticle = new CGrassParticle(pNew, pGrassType->pParticleTexture);
				AddParticle(pParticle, pGrassType);
			}
		}
	}
	else
	{
		m_flSystemMaxAge = 0.01;
	}
}

CGrassParticleSystem::~CGrassParticleSystem()
{
	unsigned int iGrassTypes = m_cGrassTypes.size();
	for (unsigned int i = 0; i < iGrassTypes; i++)
	{
		grass_particle_types *pGrassType = m_cGrassTypes[i];
		if(pGrassType)
		{
			delete pGrassType->pSystem;
			pGrassType->pSystem = NULL;
			pGrassType->pParticleTexture = NULL;
			delete pGrassType;
			pGrassType = NULL;
		}
	}
	m_cGrassTypes.clear();
}

void CGrassParticleSystem::AddParticle( CParticle *pParticle, grass_particle_types *pGrassType )
{
	pParticleManager->AddParticle(pParticle);
}

grass_particle_system* CGrassParticleSystem::CreateDefaultParticleSystem( void )
{
	grass_particle_system *pSystem = new grass_particle_system;

	pSystem->vAbsMin = Vector(0, 0, 0);
	pSystem->vAbsMax = Vector(0, 0, 0);
	pSystem->flLeaningMin = 0.0;
	pSystem->flLeaningMax = 0.0;
	pSystem->flWaveSpeed = 0.0;
	pSystem->Yaw = 0.0;

	pSystem->flSize = 15.0;
	pSystem->iTransparency = 255;
	pSystem->iCount = 100;
	pSystem->bLOD = true;
	pSystem->bDropOnGround = true;
	pSystem->bIgnoreSort = true;

	snprintf(pSystem->sParticleTexture, MAX_PARTICLE_PATH, "particles/grass1.tga");

	return pSystem;
}

bool CGrassParticleSystem::LoadParticleDefinition( particle_system_management *pSysDetails )
{
	if(!m_sParticleFile)
	{
		gEngfuncs.Con_Printf("No Grass Particle definition file specified");
		return false;
	}

	char *fileStart = (char *)gEngfuncs.COM_LoadFile(m_sParticleFile, 5, NULL);
	if(!fileStart)
	{
		gEngfuncs.Con_Printf("Bad Grass Particle definition file specified %s\n", m_sParticleFile);
		return false;
	}

	char *sFile = fileStart;
	char sSetting[256];
	char sValue[256];

	grass_particle_system *pCurrent = NULL;
	grass_particle_types *pCurrentType = NULL;
	bool bBegun = false;

	bool bLooping = true;
	while(bLooping)
	{
		sFile = gEngfuncs.COM_ParseFile(sFile, sSetting);

		if(!sFile)
			break;

		if(sSetting[0] == '\0')
		{
			gEngfuncs.Con_Printf("Unexpected error in file after %s in %s", sValue, m_sParticleFile);
			gEngfuncs.COM_FreeFile(fileStart);
			return false;
		}

		sFile = gEngfuncs.COM_ParseFile(sFile, sValue);

		if(!sFile || sValue[0] == '\0')
		{
			gEngfuncs.Con_Printf("Unexpected error in file after %s in %s", sSetting, m_sParticleFile);
			gEngfuncs.COM_FreeFile(fileStart);
			return false;
		}

		if(!stricmp(sSetting, "type_begin"))
		{
			pCurrent = CreateDefaultParticleSystem();
			pCurrent->vPostion = pSysDetails->vPosition;
			pCurrent->vDirection = pSysDetails->vDirection;
			pCurrent->vAbsMin = pSysDetails->vAbsMin;
			pCurrent->vAbsMax = pSysDetails->vAbsMax;
			bBegun = true;
			continue;
		}

		if(!stricmp(sSetting, "type_end"))
		{
			if(bBegun && pCurrent)
			{
				pCurrentType = new grass_particle_types;
				pCurrentType->pSystem = pCurrent;
				pCurrentType->pParticleTexture = LoadTGA(NULL, pCurrent->sParticleTexture);
				m_cGrassTypes.push_back(pCurrentType);
			}
			bBegun = false;
			pCurrent = NULL;
			continue;
		}

		if(!bBegun || !pCurrent)
			continue;

		if(!stricmp(sSetting, "texture"))
		{
			snprintf(pCurrent->sParticleTexture, MAX_PARTICLE_PATH, "%s", sValue);
		}
		else if(!stricmp(sSetting, "size"))
		{
			pCurrent->flSize = atof(sValue);
		}
		else if(!stricmp(sSetting, "count"))
		{
			pCurrent->iCount = abs(atoi(sValue));
		}
		else if(!stricmp(sSetting, "transparency"))
		{
			int iTemp = atoi(sValue);
			if(iTemp > 255 || iTemp < 0)
				pCurrent->iTransparency = 255;
			else
				pCurrent->iTransparency = iTemp;
		}
		else if(!stricmp(sSetting, "leaning_min"))
		{
			pCurrent->flLeaningMin = atof(sValue);
		}
		else if(!stricmp(sSetting, "leaning_max"))
		{
			pCurrent->flLeaningMax = atof(sValue);
		}
		else if(!stricmp(sSetting, "wave_speed"))
		{
			pCurrent->flWaveSpeed = atof(sValue);
		}
		else if(!stricmp(sSetting, "lod"))
		{
			pCurrent->bLOD = (!!(atoi(sValue)));
		}
		else if(!stricmp(sSetting, "drop_on_ground"))
		{
			pCurrent->bDropOnGround = (!!(atoi(sValue)));
		}
		else if(!stricmp(sSetting, "ignore_sort"))
		{
			pCurrent->bIgnoreSort = (!!(atoi(sValue)));
		}
		else
		{
			gEngfuncs.Con_Printf("Unknown setting - %s in %s\n", sSetting, m_sParticleFile);
		}
	}

	gEngfuncs.COM_FreeFile(fileStart);
	return true;
}

#endif // OPENGL_AVAILABLE
