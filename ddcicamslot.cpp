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

void DdCiCamSlot::StopIt()
{
	active = false;
	rBuffer.Clear();

	// FIXME: need to be removed for MTD
	ciSend.ClrBuffer();
}

//------------------------------------------------------------------------

DdCiCamSlot::DdCiCamSlot( DdCiAdapter &adapter, DdCiTsSend &sendCi )
: cCamSlot( &adapter, true )
, ciSend( sendCi )
, delivered( false )
, active( false )
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

bool DdCiCamSlot::Reset()
{
	LOG_FUNCTION_ENTER;

	L_FUNC_NAME();

	bool ret = cCamSlot::Reset();
	if (ret)
		StopIt();

	LOG_FUNCTION_EXIT;

	return ret;
}

//------------------------------------------------------------------------

void DdCiCamSlot::StartDecrypting()
{
	LOG_FUNCTION_ENTER;

	L_FUNC_NAME();

	StopIt();
	active = true;
	cCamSlot::StartDecrypting();

	LOG_FUNCTION_EXIT;
}

//------------------------------------------------------------------------

void DdCiCamSlot::StopDecrypting()
{
	LOG_FUNCTION_ENTER;

	L_FUNC_NAME();

	cCamSlot::StopDecrypting();
	StopIt();

	LOG_FUNCTION_EXIT;
}

//------------------------------------------------------------------------

uchar *DdCiCamSlot::Decrypt( uchar *Data, int &Count )
{
	if (!active) {
		L_ERR_LINE( "Decrypt in deactivated state ?!?" );
		Count = 0;
		return 0;
	}

	/*
	 *  WRITE
	 */

	int cnt = Count - (Count % TS_SIZE);  // we write only whole TS frames

	int stored = ciSend.Write( Data, cnt );
	Count = stored;

	/*
	 * READ
	 */

	if (delivered) {
		rBuffer.Del( TS_SIZE );
		delivered = false;
	}

	cnt = 0;
	uchar *data = rBuffer.Get( cnt );
	if (!data || (cnt < TS_SIZE)) {
		data = 0;
	}
	else
		delivered = true;

	return data;
}

//------------------------------------------------------------------------

int DdCiCamSlot::DataRecv( uchar *data )
{
	if (!active) {
		return 0;
	}

	int written = -1;     // default, try again

	int free = rBuffer.Free();
	if (free >= TS_SIZE) {
		free = TS_SIZE;
		written = rBuffer.Put( data, free );
		if (written != free)
			L_ERR_LINE( "Couldn't write previously checked free data ?!?" );
		written = 0;
	}

	return written;
}
