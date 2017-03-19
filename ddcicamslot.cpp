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
#include "ddci2.h"
#include "logging.h"

#include <vdr/remux.h>

static const int SCT_DBG_TMO = 2000;   // 2 seconds
static const int CNT_SCT_DBG_MAX = 20;


//------------------------------------------------------------------------

void DdCiCamSlot::StopIt()
{
	active = false;
	rBuffer.Clear();
	delivered = false;
	cntSctPkt = 0;
	cntSctClrPkt = 0;
	cntSctDbg = 0;
	timSctDbg.Set(SCT_DBG_TMO);

	ciSend.ClrBuffer();
}

//------------------------------------------------------------------------

DdCiCamSlot::DdCiCamSlot( DdCiAdapter &adapter, DdCiTsSend &sendCi )
: cCamSlot( &adapter, true )
, ciSend( sendCi )
, delivered( false )
, active( false )
, cntSctPkt( 0 )
, cntSctClrPkt( 0 )
, cntSctDbg( 0 )
{
	LOG_FUNCTION_ENTER;
	MtdEnable();
	LOG_FUNCTION_EXIT;
}

//------------------------------------------------------------------------

DdCiCamSlot::~DdCiCamSlot()
{
	LOG_FUNCTION_ENTER;
	StopIt();
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

	active = true;
	if (!MtdActive())
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

		// must consume the data to avoid overflow
		Count -= (Count / TS_SIZE) * TS_SIZE;
		return 0;
	}

	/*
	 *  WRITE
	 */

	/* It would be possible to store more of the given data, but this did
	 * not work during my tests. So we need to write frame by frame to the
	 * send buffer.
	 */
	// int cnt = Count - (Count % TS_SIZE);  // we write only whole TS frames
	int cnt = TS_SIZE;
	int stored = ciSend.Write( Data, cnt );
	Count = stored;

	/*
	 * MTD
	 *
	 * With MTD support active, decrypted TS packets are sent to the individual
	 * MTD CAM slots in DataRecv().
	 */

	if (MtdActive())
		return NULL;

	/*
	 * READ
	 */

	/* Decrypt is called for each frame and we need to return the decoded
	 * frame. But there is no "I_have_the_frame_consumed" function, so the
	 * only chance we have is to delete now the last sent frame from the
	 * buffer.
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
	else {
		if (TsIsScrambled( data )) {
			++cntSctPkt;

			// remove the scrambling bit?
			if (CfgIsClrSct()) {
				data[3] &= ~TS_SCRAMBLING_CONTROL;
				++cntSctClrPkt;
			}
		}

		if ((cntSctPkt) && (cntSctDbg < CNT_SCT_DBG_MAX) && timSctDbg.TimedOut()) {
			++cntSctDbg;
			L_DBG_M( LDM_SCT, "DdCiCamSlot for %s got %d scrambled packets from CAM"
				   , ciSend.GetCiDevName(), cntSctPkt );
			L_DBG_M( LDM_SCT, "DdCiCamSlot for %s clr %d scrambling control bits"
				   , ciSend.GetCiDevName(), cntSctClrPkt );
			timSctDbg.Set(SCT_DBG_TMO);
		}

		delivered = true;
	}

	return data;
}

//------------------------------------------------------------------------

int DdCiCamSlot::DataRecv( uchar *data )
{
	if (!active) {
		return 0;
	}

	int written = -1;     // default, try again

	if (MtdActive()) {
		written = MtdPutData(data, TS_SIZE);
	} else {
		int free = rBuffer.Free();
		if (free >= TS_SIZE) {
			free = TS_SIZE;
			written = rBuffer.Put( data, free );
			if (written != free)
				L_ERR_LINE( "Couldn't write previously checked free data ?!?" );
			written = 0;
		}
	}
	return written;
}
