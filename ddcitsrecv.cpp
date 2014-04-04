/////////////////////////////////////////////////////////////////////////////
//
// @file ddcitsrecv.cpp @brief Digital Devices Common Interface plugin for VDR.
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

#include "ddcitsrecv.h"
#include "ddcicommon.h"
#include "ddciadapter.h"
#include "logging.h"

#include <vdr/tools.h>

//------------------------------------------------------------------------

void DdCiTsRecv::CleanUp()
{
	LOG_FUNCTION_ENTER;

	if (fd != -1) {
		close( fd );
		fd = -1;
	}

	LOG_FUNCTION_EXIT;
}

//------------------------------------------------------------------------

void DdCiTsRecv::Deliver()
{
	while (Running()) {
		int cnt = 0;
		uchar *data = rb.Get( cnt );
		if (!data)
			return;

		int skipped;
		uchar *frame = CheckTsSync( data, cnt, skipped );
		if (skipped) {
			L_ERR_LINE( "skipped %d bytes to sync on start of TS packet", skipped );
			rb.Del( skipped );
		}

		if (adapter.DataRecv( frame ) != -1)
			rb.Del( TS_SIZE );
	}
}

//------------------------------------------------------------------------

DdCiTsRecv::DdCiTsRecv( DdCiAdapter &the_adapter, int ci_fdr, cString &devNameCi )
: cThread()
, adapter( the_adapter )
, fd( ci_fdr )
, ciDevName( devNameCi )
, rb( BUF_SIZE, BUF_MARGIN, false, "DDCI TS Recv" )
{
	// don't use adapter in this function,, unless you know what you are doing!

	SetDescription( "DDCI TS Recv buffer on %s", *ciDevName );
	L_DBG( "DdCiTsRecv for %s created", *ciDevName );
}

//------------------------------------------------------------------------

DdCiTsRecv::~DdCiTsRecv()
{
	LOG_FUNCTION_ENTER;

	Cancel( 3 );
	CleanUp();

	LOG_FUNCTION_EXIT;
}

//------------------------------------------------------------------------

bool DdCiTsRecv::Start()
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

void DdCiTsRecv::Cancel( int waitSec )
{
	LOG_FUNCTION_ENTER;

	cThread::Cancel( waitSec );

	LOG_FUNCTION_EXIT;
}

//------------------------------------------------------------------------

void DdCiTsRecv::Action()
{
	static const int RUN_POLL_TMO = 100;   // get may wait 100ms

	LOG_FUNCTION_ENTER;

	rb.SetTimeouts( RUN_POLL_TMO, 0 );

	bool firstRead( true );
	cPoller Poller( fd );

	while (Running()) {
		if (firstRead || Poller.Poll( RUN_POLL_TMO )) {
			firstRead = false;
			errno = 0;
			int r = rb.Read( fd );
			if ((r < 0) && FATALERRNO) {
				if (errno == EOVERFLOW)
					L_ERR_LINE( "Driver buffer overflow on file %s:%m", *ciDevName );
				else {
					L_ERROR();
					break;
				}
			}
		}
	}

	CleanUp();

	LOG_FUNCTION_EXIT;
}
