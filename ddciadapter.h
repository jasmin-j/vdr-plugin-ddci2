/////////////////////////////////////////////////////////////////////////////
//
// @file ddciadapter.h @brief Digital Devices Common Interface plugin for VDR.
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

#ifndef __DDCIADAPTER_H
#define __DDCIADAPTER_H

#include "ddcitssend.h"
#include "ddcitsrecv.h"

#include <vdr/ci.h>

// forward declarations
class DdCiCamSlot;

/**
 * This class implements the physical interface to the CAM device.
 */
class DdCiAdapter: public cCiAdapter
{
private:
	/* VDR currently allows only *one* device per CI adapter. Moreover,
	 * it is bound in a 1:1 relation to that device, just from the
	 * creation.
	 */
	cDevice *device;    //< the bound device
	int fd;             //< .../frontendX/caX device file handle
	cString caDevName;  //< .../frontendX/caX device path
	DdCiTsSend ciSend;  //< the CAM TS sender
	DdCiTsRecv ciRecv;  //< the CAM TS receiver

	// FIXME: after VDR base class change, this is not necessary
	DdCiCamSlot *camSlot;  //< the one and only slot of a DD CI adapter

	void CleanUp();

protected:
	/* see file ci.h in the VDR include directory for the description of
	 * the following functions
	 */
	virtual void Action();
	virtual int Read( uint8_t *Buffer, int MaxLength );
	virtual void Write( const uint8_t *Buffer, int Length );
	virtual bool Reset( int Slot );
	virtual eModuleStatus ModuleStatus( int Slot );
	virtual bool Assign( cDevice *Device, bool Query = false );

public:
	/**
	 * Constructor.
	 * Checks for the available slots of the CAM and starts the
	 * controlling thread.
	 * @param dev the assigned device
	 * @param ca_fd the file handle for the .../frontendX/caX device
	 * @param ci_fdw the write file handle for the .../frontendX/ciX device
	 * @param ci_fdr the read file handle for the .../frontendX/ciX device
	 * @param devNameCa the name of the device (.../frontendX/caX)
	 * @param devNameCi the name of the device (.../frontendX/ciX)
	 **/
	DdCiAdapter( cDevice *dev, int ca_fd, int ci_fdw, int ci_fdr, cString &devNameCa, cString &devNameCi );

	/// Destructor.
	virtual ~DdCiAdapter();

	/**
	 * Deliver the received CAM TS Data to the receive buffer.
	 * data is always only one TS data packet of size TS_SIZE and it is
	 * guaranteed, that the first byte of data is a TS_SYNC_BYTE.
	 * @param data the received data to
	 * @return 0 .. data delivered
	 *        -1 .. receiver buffer full
	 *        -2 .. no appropriate target found, discard data
	 */
	int DataRecv( uchar *data );
};

#endif //__DDCIADAPTER_H
