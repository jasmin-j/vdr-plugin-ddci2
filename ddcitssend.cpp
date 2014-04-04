/////////////////////////////////////////////////////////////////////////////
//
// @file ddcitssend.cpp @brief Digital Devices Common Interface plugin for VDR.
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

#include "ddcitssend.h"
#include "ddcicommon.h"
#include "ddciadapter.h"
#include "logging.h"

#include <vdr/tools.h>

//------------------------------------------------------------------------

void DdCiTsSend::CleanUp()
{
	LOG_FUNCTION_ENTER;

	if (fd != -1) {
		close( fd );
		fd = -1;
	}

	LOG_FUNCTION_EXIT;
}

//------------------------------------------------------------------------

DdCiTsSend::DdCiTsSend( DdCiAdapter &the_adapter, int ci_fdw, cString &devNameCi )
: cThread()
, adapter( the_adapter )
, fd( ci_fdw )
, ciDevName( devNameCi )
, rb( BUF_SIZE, BUF_MARGIN, false, "DDCI TS Send" )
{
	// don't use adapter in this function,, unless you know what you are doing!

	SetDescription( "DDCI TS Send buffer on %s", *ciDevName );
	L_DBG( "DdCiTsSend for %s created", *ciDevName );
}

//------------------------------------------------------------------------

DdCiTsSend::~DdCiTsSend()
{
	LOG_FUNCTION_ENTER;

	Cancel( 3 );
	CleanUp();

	LOG_FUNCTION_EXIT;
}

//------------------------------------------------------------------------

bool DdCiTsSend::Start()
{
	LOG_FUNCTION_ENTER;

	if (fd == -1) {
		L_ERR_LINE( "Invalid file handle" );
		return false;
	}

	LOG_FUNCTION_EXIT;

	return cThread::Start();
}

//------------------------------------------------------------------------

void DdCiTsSend::Cancel( int waitSec )
{
	LOG_FUNCTION_ENTER;

	cThread::Cancel( waitSec );

	LOG_FUNCTION_EXIT;
}

//------------------------------------------------------------------------

int DdCiTsSend::Write( const uchar *data, int count )
{
	if (count % TS_SIZE)
		L_ERR_LINE("Got a truncated frame");

	mutex.Lock();
	int written = rb.Put( data, count );
	mutex.Unlock();

	if (written % TS_SIZE)
		L_ERR_LINE("Couldn't write a whole frame to CI send buffer");

	return written;
}

//------------------------------------------------------------------------

void DdCiTsSend::Action()
{
	static const int RUN_CHECK_TMO = 100;   // get may wait 100ms

	LOG_FUNCTION_ENTER;

	rb.SetTimeouts( 0, RUN_CHECK_TMO );

	while (Running()) {
		int cnt = 0;
		uchar *data = rb.Get( cnt );
		if (data) {
			int skipped;
			uchar *frame = CheckTsSync( data, cnt, skipped );
			if (skipped) {
				L_ERR_LINE( "skipped %d bytes to sync on start of TS packet", skipped );
				rb.Del( skipped );
			}

			int len = cnt - skipped;
			len -= (len % TS_SIZE);
			if (len >= TS_SIZE) {
				int w = WriteAllOrNothing( fd, frame, len, 5 * RUN_CHECK_TMO, RUN_CHECK_TMO );
				if (w >= 0) {
					int remain = len - w;
					if (remain > 0) {
						L_ERR_LINE( "couldn't write all data to CAM %s", *ciDevName );
						len -= remain;
					}
				} else {
					L_ERR_LINE( "couldn't write to CAM %s:%m", *ciDevName );
					break;
				}
				rb.Del( len );
			}
		}
	}

	CleanUp();

	LOG_FUNCTION_EXIT;
}
