/////////////////////////////////////////////////////////////////////////////
//
// @file ddcireadbuf.cpp @brief Digital Devices Common Interface plugin for VDR.
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

#include "ddcireadbuf.h"
#include "logging.h"

//------------------------------------------------------------------------

int inline DdCiReadBuf::ReadBlock( int FileHandle, uchar *Data, int Size )
{
	Size -= Size % margin;   // we read in junks of margin
	int Count = safe_read( FileHandle, Data, Size );
	if ((Count > 0) && (Count % margin))
		L_ERR( "Couldn't read a complete junk, got only %d bytes", Count );

	return Count;
}

//------------------------------------------------------------------------

DdCiReadBuf::DdCiReadBuf()
: cRingBufferLinear( BUF_SIZE, BUF_MARGIN, false, "DDCI CAM Recv" )
{
	LOG_FUNCTION_ENTER;
	LOG_FUNCTION_EXIT;
}

//------------------------------------------------------------------------

DdCiReadBuf::~DdCiReadBuf()
{
	LOG_FUNCTION_ENTER;
	LOG_FUNCTION_EXIT;
}

//------------------------------------------------------------------------

int DdCiReadBuf::ReadJunk( int FileHandle, int Max )
{
	int Tail = tail;
	int rest = Size() - head;
	int diff = Tail - head;
	int free = ((Tail < margin) ? rest : (diff > 0) ? diff : Size() + diff - margin) - 1;
	if ((0 < Max) && (Max < free))
		free = Max;
	free -= free % margin;   // we read in junks of margin
	int Count = -1;
	errno = EAGAIN;
	if (free > 0) {
		if (rest <= margin) {
			static const int TEMP_MAX = 20 * TS_SIZE;
			int szTemp = free < TEMP_MAX ? free : TEMP_MAX;
			szTemp -= szTemp % margin;
			uchar temp[ szTemp ];
			Count = ReadBlock( FileHandle, temp, szTemp );
			if (Count > 0) {
				memcpy( buffer + head, temp, rest );
				int part = Count - rest;
				if (part)
					memcpy( buffer + margin, temp + rest, part );
				head = margin + part;
			}
		} else {
			int size = (diff > 0) ? diff - 1 : rest;
			if (Tail <= margin)
				size--;
			Count = ReadBlock( FileHandle, buffer + head, size );
			if (Count > 0)
				head += Count;
		}
		if (Count > 0) {
			if (statistics) {
				int fill = head - Tail;
				if (fill < 0)
					fill = Size() + fill;
				else if (fill >= Size())
					fill = Size() - 1;
				UpdatePercentage( fill );
			}
		}
	}
#ifdef DEBUGRINGBUFFERS
	lastHead = head;
	lastPut = Count;
#endif
	EnableGet();
	if (free == 0)
		WaitForPut();
	return Count;
}
