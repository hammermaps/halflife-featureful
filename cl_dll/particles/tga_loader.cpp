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

// TGA texture loader for the particle system

#include "hud.h"
#include "cl_util.h"
#include <string.h>
#include <stdio.h>
#include "gl_dynamic.h"

#include "particle_header.h"

#if OPENGL_AVAILABLE

#ifndef GL_GENERATE_MIPMAP
#define GL_GENERATE_MIPMAP 0x8191
#endif

static unsigned char cUncompressedHeader[12] = {0,0,2,0,0,0,0,0,0,0,0,0};
static unsigned char cCompressedHeader[12] = {0,0,10,0,0,0,0,0,0,0,0,0};

particle_texture_s * LoadTGA(particle_texture_s *pTexture, const char *filename)
{
	particle_texture_s *pCached = pParticleManager->HasTexture(filename);
	if(pCached)
		return pCached;

	// Use engine's COM_LoadFile to find the file in the game directory
	int iLength = 0;
	char *pFileData = (char *)gEngfuncs.COM_LoadFile(filename, 5, &iLength);
	
	if(!pFileData || iLength < (int)sizeof(tga_header))
	{
		gEngfuncs.Con_Printf("Could not find TGA file: %s\n", filename);
		if(pFileData) gEngfuncs.COM_FreeFile(pFileData);
		return NULL;
	}

	// Use an anonymous temp file (portable, no predictable path, auto-deleted on close)
	FILE *fTGA = tmpfile();
	if(!fTGA)
	{
		gEngfuncs.Con_Printf("Could not create temp file for TGA: %s\n", filename);
		gEngfuncs.COM_FreeFile(pFileData);
		return NULL;
	}
	if(fwrite(pFileData, 1, iLength, fTGA) != (size_t)iLength)
	{
		gEngfuncs.Con_Printf("Could not write temp TGA data: %s\n", filename);
		fclose(fTGA);
		gEngfuncs.COM_FreeFile(pFileData);
		return NULL;
	}
	gEngfuncs.COM_FreeFile(pFileData);
	rewind(fTGA);

	tga_header header;
	if(fread(&header, sizeof(tga_header), 1, fTGA) == 0)
	{
		gEngfuncs.Con_Printf("Could not read TGA header: %s\n", filename);
		fclose(fTGA);
		return NULL;
	}

	pTexture = new particle_texture_s;

	bool bLoaded = false;
	if(memcmp(cUncompressedHeader, &header, sizeof(header)) == 0)
	{
		bLoaded = LoadUncompressedTGA(pTexture, fTGA);
	}
	else if(memcmp(cCompressedHeader, &header, sizeof(header)) == 0)
	{
		bLoaded = LoadCompressedTGA(pTexture, fTGA);
	}
	else
	{
		gEngfuncs.Con_Printf("Unsupported TGA type: %s\n", filename);
		fclose(fTGA);
		delete pTexture;
		return NULL;
	}

	fclose(fTGA);

	if(!bLoaded || !pTexture->imageData)
	{
		gEngfuncs.Con_Printf("Failed to load TGA: %s\n", filename);
		delete pTexture;
		return NULL;
	}

	// Upload to OpenGL
	pTexture->iID = new GLuint;
	glGenTextures(1, pTexture->iID);
	glBindTexture(GL_TEXTURE_2D, *pTexture->iID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

	if(pTexture->iType == GL_RGB)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pTexture->iWidth, pTexture->iHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, pTexture->imageData);
	}
	else if(pTexture->iType == GL_RGBA)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pTexture->iWidth, pTexture->iHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pTexture->imageData);
	}

	pParticleManager->AddTexture(filename, pTexture);

	return pTexture;
}

bool LoadUncompressedTGA(particle_texture_s *texture, FILE *fTGA)
{
	tga tgaFile;

	if(fread(tgaFile.cHeader, sizeof(tgaFile.cHeader), 1, fTGA) == 0)
		return false;

	texture->iWidth  = tgaFile.cHeader[1] * 256 + tgaFile.cHeader[0];
	texture->iHeight = tgaFile.cHeader[3] * 256 + tgaFile.cHeader[2];
	texture->iBpp    = tgaFile.cHeader[4];

	tgaFile.iWidth   = texture->iWidth;
	tgaFile.iHeight  = texture->iHeight;
	tgaFile.iBpp     = texture->iBpp;

	if((texture->iWidth <= 0) || (texture->iHeight <= 0) || ((texture->iBpp != 24) && (texture->iBpp !=32)))
	{
		gEngfuncs.Con_Printf("Invalid TGA file parameters\n");
		return false;
	}

	if(texture->iBpp == 24)
		texture->iType = GL_RGB;
	else
		texture->iType = GL_RGBA;

	tgaFile.iBytespp = (tgaFile.iBpp / 8);
	tgaFile.iImageSize = (tgaFile.iBytespp * tgaFile.iWidth * tgaFile.iHeight);

	texture->imageData = new GLubyte[tgaFile.iImageSize];
	if(!texture->imageData)
		return false;

	if(fread(texture->imageData, 1, tgaFile.iImageSize, fTGA) != tgaFile.iImageSize)
	{
		delete [] texture->imageData;
		texture->imageData = NULL;
		return false;
	}

	// swap BGR to RGB
	for(GLuint cswap = 0; cswap < (GLuint)tgaFile.iImageSize; cswap += tgaFile.iBytespp)
	{
		GLubyte temp = texture->imageData[cswap];
		texture->imageData[cswap] = texture->imageData[cswap + 2];
		texture->imageData[cswap + 2] = temp;
	}

	return true;
}

bool LoadCompressedTGA(particle_texture_s *texture, FILE *fTGA)
{
	tga tgaFile;

	if(fread(tgaFile.cHeader, sizeof(tgaFile.cHeader), 1, fTGA) == 0)
		return false;

	texture->iWidth  = tgaFile.cHeader[1] * 256 + tgaFile.cHeader[0];
	texture->iHeight = tgaFile.cHeader[3] * 256 + tgaFile.cHeader[2];
	texture->iBpp    = tgaFile.cHeader[4];

	tgaFile.iWidth   = texture->iWidth;
	tgaFile.iHeight  = texture->iHeight;
	tgaFile.iBpp     = texture->iBpp;

	if((texture->iWidth <= 0) || (texture->iHeight <= 0) || ((texture->iBpp != 24) && (texture->iBpp !=32)))
	{
		gEngfuncs.Con_Printf("Invalid compressed TGA parameters\n");
		return false;
	}

	if(texture->iBpp == 24)
		texture->iType = GL_RGB;
	else
		texture->iType = GL_RGBA;

	tgaFile.iBytespp = (tgaFile.iBpp / 8);
	tgaFile.iImageSize = (tgaFile.iBytespp * tgaFile.iWidth * tgaFile.iHeight);

	texture->imageData = new GLubyte[tgaFile.iImageSize];
	if(!texture->imageData)
		return false;

	GLuint pixelcount = tgaFile.iWidth * tgaFile.iHeight;
	GLuint currentpixel = 0;
	GLuint currentbyte = 0;
	GLubyte *colorbuffer = new GLubyte[tgaFile.iBytespp];

	do {
		GLubyte chunkheader = 0;

		if(fread(&chunkheader, sizeof(GLubyte), 1, fTGA) == 0)
		{
			delete [] colorbuffer;
			delete [] texture->imageData;
			texture->imageData = NULL;
			return false;
		}

		if(chunkheader < 128) // raw chunk
		{
			chunkheader++;
			for(short counter = 0; counter < chunkheader; counter++)
			{
				if(fread(colorbuffer, 1, tgaFile.iBytespp, fTGA) != tgaFile.iBytespp)
				{
					delete [] colorbuffer;
					delete [] texture->imageData;
					texture->imageData = NULL;
					return false;
				}

				texture->imageData[currentbyte    ] = colorbuffer[2]; // R
				texture->imageData[currentbyte + 1] = colorbuffer[1]; // G
				texture->imageData[currentbyte + 2] = colorbuffer[0]; // B

				if(tgaFile.iBytespp == 4)
					texture->imageData[currentbyte + 3] = colorbuffer[3]; // A

				currentbyte += tgaFile.iBytespp;
				currentpixel++;

				if(currentpixel > pixelcount)
				{
					delete [] colorbuffer;
					delete [] texture->imageData;
					texture->imageData = NULL;
					return false;
				}
			}
		}
		else // RLE chunk
		{
			chunkheader -= 127;

			if(fread(colorbuffer, 1, tgaFile.iBytespp, fTGA) != tgaFile.iBytespp)
			{
				delete [] colorbuffer;
				delete [] texture->imageData;
				texture->imageData = NULL;
				return false;
			}

			for(short counter = 0; counter < chunkheader; counter++)
			{
				texture->imageData[currentbyte    ] = colorbuffer[2]; // R
				texture->imageData[currentbyte + 1] = colorbuffer[1]; // G
				texture->imageData[currentbyte + 2] = colorbuffer[0]; // B

				if(tgaFile.iBytespp == 4)
					texture->imageData[currentbyte + 3] = colorbuffer[3]; // A

				currentbyte += tgaFile.iBytespp;
				currentpixel++;

				if(currentpixel > pixelcount)
				{
					delete [] colorbuffer;
					delete [] texture->imageData;
					texture->imageData = NULL;
					return false;
				}
			}
		}
	} while(currentpixel < pixelcount);

	delete [] colorbuffer;
	return true;
}

#endif // OPENGL_AVAILABLE
