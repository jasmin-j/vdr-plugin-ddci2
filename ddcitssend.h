/////////////////////////////////////////////////////////////////////////////
//
// @file ddcitssendbuf.h @brief Digital Devices Common Interface plugin for VDR.
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

#ifndef __DDCITSSEND_H
#define __DDCITSSEND_H

#include <vdr/thread.h>
#include <vdr/ringbuffer.h>
#include <vdr/remux.h>   // TS_SIZE

// forward declarations
class DdCiAdapter;

/**
 * This class implements the physical interface to the CAM TS device.
 * It implements a send buffer a sender thread to write the TS data
 * independent and with big junks.
 */
class DdCiTsSend: public cThread
{
private:
	DdCiAdapter &adapter;  //< the associated CI adapter
	int fd;                //< .../frontendX/caX device write file handle
	cString ciDevName;     //< .../frontendX/ciX device path
	cRingBufferLinear rb;  //< the send buffer
	cMutex mutex;          //< The synchronization mutex for rb

	static const int BUF_NUM = 1000;
	static const int BUF_MARGIN = TS_SIZE;

	// cRingBufferLinear requires one margin and 1 byte for internal reasons
	static const int BUF_SIZE = (BUF_MARGIN * (BUF_NUM + 1)) + 1;

	void CleanUp();

public:
	/**
	 * Constructor.
	 * Creates a new CAM TS send buffer.
	 * @param adapter the CAM adapter this slot is associated
	 * @param ci_fdw open file handle for the .../frontendX/ciX device
	 * @param devNameCi the name of the device (.../frontendX/ciX)
	 **/
	DdCiTsSend( DdCiAdapter &the_adapter, int ci_fdw, cString &devNameCi );

	/// Destructor.
	virtual ~DdCiTsSend();

	bool Start();
	void Cancel( int waitSec = 0 );

	/**
	 * Write the as most of the given data to the send buffer.
	 * This function is thread save for multiple writers.
	 * @param data the data to send
	 * @param count the length of the data
	 * @return the number of bytes actually written
	 */
	int Write(const uchar *data, int count);

	/**
	 * Waits for data present in the send buffer and tries to send them to
	 * the CAM.
	 **/
	virtual void Action();
};

#endif //__DDCITSSEND_H
