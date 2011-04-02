/*
 *      Copyright (C) 2009 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "xbmc_ac_types.h"
#include "xbmc_ac_dll.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern "C"
{

#include "../lib/nosefart/src/types.h"
#include "../lib/nosefart/src/log.h"
#include "../lib/nosefart/src/version.h"
#include "../lib/nosefart/src/machine/nsf.h"

//-- Create -------------------------------------------------------------------
// Called on load. Addon should fully initalize or return error status
//-----------------------------------------------------------------------------
ADDON_STATUS Create(void* hdl, void* props)
{
  //if (!props)
    return STATUS_OK;

  //return STATUS_NEED_SETTINGS;
}

//-- Stop ---------------------------------------------------------------------
// This dll must cease all runtime activities
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
void Stop()
{
}

//-- Destroy ------------------------------------------------------------------
// Do everything before unload of this add-on
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
void Destroy()
{
}

//-- HasSettings --------------------------------------------------------------
// Returns true if this add-on use settings
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
bool HasSettings()
{
  return false;
}

//-- GetStatus ---------------------------------------------------------------
// Returns the current Status of this visualisation
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
ADDON_STATUS GetStatus()
{
  return STATUS_OK;
}

//-- GetSettings --------------------------------------------------------------
// Return the settings for XBMC to display
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
unsigned int GetSettings(StructSetting ***sSet)
{
  return 0;
}

//-- FreeSettings --------------------------------------------------------------
// Free the settings struct passed from XBMC
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------

void FreeSettings()
{
}

//-- SetSetting ---------------------------------------------------------------
// Set a specific Setting value (called from XBMC)
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
ADDON_STATUS SetSetting(const char *strSetting, const void* value)
{
  return STATUS_OK;
}

struct NSF
{
  nsf_t* info;
  char* buffer;
  int dataInBuffer;
  int buffer_size;
  int globalpos; // position of last byte in buffer
  int track;
};

void StartPlayback(NSF* track)
{
  nsf_playtrack(track->info,track->track,48000,16,false);
  for (int i = 0; i < 6; i++)
    nsf_setchan(track->info,i,true);

  track->buffer = new char[48000/track->info->playback_rate*2];
  track->buffer_size = 48000/track->info->playback_rate*2;
  track->dataInBuffer = 0;
  track->globalpos = 0;
}

AC_INFO* Init(const char* strFile, int track)
{
  nsf_init();
  log_init();
  AC_INFO* info = new AC_INFO;
  NSF* nsf = new NSF;
  nsf->info = nsf_load(const_cast<char*>(strFile),NULL,0);
  if (!nsf->info)
  {
    delete nsf;
    delete info;
    return NULL;
  }
  info->channels = 1;
  info->samplerate = 48000;
  info->bitpersample = 16;
  info->totaltime = 4*60*1000;
  strcpy(info->name,"NSF");
  nsf->track = track;
  info->mod = nsf;
  StartPlayback(nsf);
  return info;
}

void DeInit(AC_INFO* info)
{
  if (info && info->mod)
  {
    nsf_t* tmp = ((NSF*)info->mod)->info;
    nsf_free(&tmp);
    delete ((NSF*)info->mod)->buffer;
    delete (NSF*)info->mod;
  }
  delete info;
}

void AdvanceFrame(NSF* nsf)
{
  nsf_frame(nsf->info);
  nsf->info->process(nsf->buffer,nsf->buffer_size/2);
  nsf->dataInBuffer = nsf->buffer_size;
  nsf->globalpos += nsf->buffer_size;
}


int64_t Seek(AC_INFO* info, int64_t iSeekTime)
{
  if (!info || !info->mod)
    return -1;

  NSF* pNsf = (NSF*)info->mod;
  if (pNsf->globalpos > iSeekTime*48*2);
  {
    delete[] pNsf->buffer;
    StartPlayback(pNsf);
  }
  while (pNsf->globalpos < iSeekTime*48*2)
    AdvanceFrame(pNsf);

  pNsf->dataInBuffer -= pNsf->globalpos-iSeekTime*48*2;
  return iSeekTime;
}

int ReadPCM(AC_INFO* info, void* pBuffer, unsigned int size, unsigned int *actualsize)
{
  if (!info || !info->mod)
    return READ_ERROR;

  NSF* pNsf = (NSF*)info->mod;
  if (pNsf->dataInBuffer == 0)
    AdvanceFrame(pNsf);
  
  *actualsize = size<pNsf->dataInBuffer?size:pNsf->dataInBuffer;
  char* bufstart = pNsf->buffer+pNsf->buffer_size-pNsf->dataInBuffer;
  memcpy(pBuffer,bufstart,*actualsize);
  pNsf->dataInBuffer -= *actualsize;
  return READ_SUCCESS;
}

int GetNumberOfTracks(const char* strFile)
{
  nsf_init();
  log_init();
  nsf_t* tmp = nsf_load(const_cast<char*>(strFile),NULL,0);
  int result = tmp->num_songs;
  nsf_free(&tmp);

  return result;
}

}
