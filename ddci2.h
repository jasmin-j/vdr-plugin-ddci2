/////////////////////////////////////////////////////////////////////////////
//
// @file ddci.hp @brief Digital Devices Common Interface plugin for VDR.
//
// Copyright (c) 2013 - 2014 by Jasmin Jessich.  All Rights Reserved.
//
// Contributor(s):
//
// License: GPLv2
//
// This file is part of vdr_plugin_ddci2.
//
// vdr_plugin_ddci2 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// vdr_plugin_ddci2 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with vdr_plugin_ddci2.  If not, see <http://www.gnu.org/licenses/>.
//
/////////////////////////////////////////////////////////////////////////////


#ifndef __DDCI2_H
#define __DDCI2_H

#include <vdr/config.h>

#if (VDRVERSNUM >= 20303)
# define DDCI_MTD         1   // MTD enabled
# define DDCI_RB_CLR_MTX  0   /* cRingBufferLinear::Clear is thread save, when
                               * executed from reader thread */
#else
# define DDCI_MTD         0   // no MTD older versions
# define DDCI_RB_CLR_MTX  1   /* cRingBufferLinear::Clear is not thread save and
                               * we need a mutex */
#endif


// global config options
extern int cfgClrSct;

inline bool CfgIsClrSct()
{
  return cfgClrSct != 0;
}


#if DDCI_RB_CLR_MTX

#define DDCI_RB_CLR_MTX_DECL(_m)  cMutex _m;
#define DDCI_RB_CLR_MTX_LOCK(_m)  cMutexLock MutexLock( _m );

#else

#define DDCI_RB_CLR_MTX_DECL(_m)
#define DDCI_RB_CLR_MTX_LOCK(_m)

#endif // DDCI_RB_CLR_MTX

#endif // __DDCI2_H
