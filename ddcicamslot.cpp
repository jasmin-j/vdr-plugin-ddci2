/////////////////////////////////////////////////////////////////////////////
//
// @file ddcicamslot.cpp @brief Digital Devices Common Interface plugin for VDR.
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

#include "ddcicamslot.h"
#include "ddciadapter.h"
#include "ddcitssend.h"
#include "logging.h"

//------------------------------------------------------------------------

DdCiCamSlot::DdCiCamSlot( DdCiAdapter &adapter, DdCiTsSend &sendCi )
: cCamSlot( &adapter, true )
, ciSend( sendCi )
{
	LOG_FUNCTION_ENTER;
	LOG_FUNCTION_EXIT;
}

//------------------------------------------------------------------------

DdCiCamSlot::~DdCiCamSlot()
{
	LOG_FUNCTION_ENTER;
	LOG_FUNCTION_EXIT;
}

//------------------------------------------------------------------------

uchar *DdCiCamSlot::Decrypt( uchar *Data, int &Count )
{
	uchar *ret( 0 );

	int stored = ciSend.Write( Data, Count );
	Count = stored;

	// FIXME: Implement read buffer reading
	return ret;
}