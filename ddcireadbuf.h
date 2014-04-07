/////////////////////////////////////////////////////////////////////////////
//
// @file ddcireadbuf.h @brief Digital Devices Common Interface plugin for VDR.
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

#ifndef __DDCIREADBUF_H
#define __DDCIREADBUF_H

#include <vdr/ringbuffer.h>
#include <vdr/remux.h>   // TS_SIZE, TS_SYNC_BYTE

/**
 * This class implements the receive buffer for one device.
 */
class DdCiReadBuf: public cRingBufferLinear
{
private:
	static const int BUF_NUM = 2000;
	static const int BUF_MARGIN = TS_SIZE;

	// cRingBufferLinear requires one margin and 1 byte for internal reasons
	static const int BUF_SIZE = (BUF_MARGIN * (BUF_NUM + 1)) + 1;

	int ReadBlock( int FileHandle, uchar *Data, int Size );

public:

	/// Constructor.
	DdCiReadBuf();

	/// Destructor.
	virtual ~DdCiReadBuf();

	int ReadJunk( int FileHandle, int Max = 0 );
    ///< Reads at most Max bytes in junks of margin from FileHandle and stores
    ///< them in the ring buffer. If Max is 0, reads as many bytes as possible
	///< in junks of margin. Only one actual read() call is done.
	///< Returns the number of bytes actually read and stored, or
	///< an error value from the actual read() call.
};

#endif //__DDCIREADBUF_H
