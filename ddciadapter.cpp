/////////////////////////////////////////////////////////////////////////////
//
// @file ddciadapter.cpp @brief Digital Devices Common Interface plugin for VDR.
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

#include "ddciadapter.h"
#include "ddcicamslot.h"
#include "logging.h"

#include <vdr/device.h>

#include <sys/ioctl.h>
#include <linux/dvb/ca.h>

/* NOTE: Most of the code is copied from vdr/dvbci.c
 */

//------------------------------------------------------------------------

void DdCiAdapter::CleanUp()
{
	LOG_FUNCTION_ENTER;

	if (fd != -1) {
		close( fd );
		fd = -1;
	}

	LOG_FUNCTION_EXIT;
}

int DdCiAdapter::GetDeviceNumber(cDevice *Device)
{
	cDevice *dev = Device;

	if (!dev)
		dev = device;

	if (dev)
		return dev->DeviceNumber() + 1;
	else
		return -1;
}

//------------------------------------------------------------------------

DdCiAdapter::DdCiAdapter( int ca_fd, int ci_fdw, int ci_fdr, cString &devNameCa, cString &devNameCi )
: device( NULL )
, fd( ca_fd )
, caDevName( devNameCa )
, ciSend( *this, ci_fdw, devNameCi )
, ciRecv( *this, ci_fdr, devNameCi )
, camSlot( 0 )
{
	LOG_FUNCTION_ENTER;

	SetDescription( "DDCI adapter %s", *caDevName );

	ca_caps_t Caps;
	if (ioctl( fd, CA_GET_CAP, &Caps ) == 0) {
		if ((Caps.slot_type & CA_CI_LINK) != 0) {
			int NumSlots = Caps.slot_num;
			if (NumSlots > 0) {
				for (int i = 0; i < NumSlots; i++) {
					if (!camSlot) {
						camSlot = new DdCiCamSlot( *this, ciSend );
					} else {
						L_ERR( "CAM(%s) Currently only ONE CAM slot supported", GetCaDevName() );
					}
				}
				L_DBG( "DdCiAdapter(%s) created: DescrNum: %d, DescrType: %d, SlotNum: %d, , SlotType: %d"
					 , GetCaDevName(), Caps.descr_num, Caps.descr_type, Caps.slot_num, Caps.slot_type );
				Start();
			} else
				L_ERR( "no CAM slots found on CAM(%s)", GetCaDevName() );
		} else
			L_INF( "CAM(%s) doesn't support CI link layer interface", GetCaDevName() );
	} else
		L_ERR( "can't get CA capabilities from CAM(%s)", GetCaDevName() );

	LOG_FUNCTION_EXIT;
}

//------------------------------------------------------------------------

DdCiAdapter::~DdCiAdapter()
{
	LOG_FUNCTION_ENTER;

	ciSend.Cancel( 3 );  // stop the TS sender thread, before we stop this thread
	ciRecv.Cancel( 3 );  // stop the TS receiver thread, before we stop this thread
	Cancel( 3 );
	CleanUp();

	LOG_FUNCTION_EXIT;
}

//------------------------------------------------------------------------

int DdCiAdapter::DataRecv( uchar *data )
{
	int ret = 0;

	if ( camSlot ) {
		camSlot->DataRecv( data );
	} else
		ret = -2;

	return ret;
}

//------------------------------------------------------------------------

void DdCiAdapter::Action()
{
	LOG_FUNCTION_ENTER;

	if (ciSend.Start())
		if (ciRecv.Start())
			cCiAdapter::Action();
		else {
			L_ERR( "couldn't start CAM TS Recv on device %d", GetDeviceNumber() );
			ciSend.Cancel( 3 );
		}
	else
		L_ERR( "couldn't start CAM TS Send on device %d", GetDeviceNumber() );

	LOG_FUNCTION_EXIT;
}

//------------------------------------------------------------------------

int DdCiAdapter::Read( uint8_t *Buffer, int MaxLength )
{
	if (Buffer && MaxLength > 0) {
		struct pollfd pfd[ 1 ];
		pfd[ 0 ].fd = fd;
		pfd[ 0 ].events = POLLIN;
		if (poll( pfd, 1, CAM_READ_TIMEOUT ) > 0 && (pfd[ 0 ].revents & POLLIN)) {
			int n = safe_read( fd, Buffer, MaxLength );
			if (n >= 0)
				return n;
			L_ERR( "can't read from CI adapter (%s) on device %d: %m", *caDevName, GetDeviceNumber() );
		}
	}
	return 0;
}

//------------------------------------------------------------------------

void DdCiAdapter::Write( const uint8_t *Buffer, int Length )
{
	if (Buffer && Length > 0) {
		if (safe_write( fd, Buffer, Length ) != Length)
			L_ERR( "can't write to CI adapter (%s) on device %d: %m", *caDevName, GetDeviceNumber() );
	}
}

//------------------------------------------------------------------------

bool DdCiAdapter::Reset( int Slot )
{
	ciRecv.ClrBuffer();
	ciSend.ClrBuffer();

	if (ioctl( fd, CA_RESET, 1 << Slot ) != -1)	{
		L_DBG( "DdCiAdapter(%s) Reset slot %d on device %d", *caDevName, Slot, GetDeviceNumber() );
		return true;
	}
	else
		L_ERR( "can't reset CAM slot %d on device %d: %m", Slot, GetDeviceNumber() );
	return false;
}

//------------------------------------------------------------------------

eModuleStatus DdCiAdapter::ModuleStatus( int Slot )
{
	ca_slot_info_t sinfo;
	sinfo.num = Slot;
	if (ioctl( fd, CA_GET_SLOT_INFO, &sinfo ) != -1) {
		if ((sinfo.flags & CA_CI_MODULE_READY) != 0)
			return msReady;
		else if ((sinfo.flags & CA_CI_MODULE_PRESENT) != 0)
			return msPresent;
	} else
		L_ERR( "can't get info of CAM slot %d on device %d: %m", Slot, GetDeviceNumber() );
	return msNone;
}

//------------------------------------------------------------------------

bool DdCiAdapter::Assign( cDevice *Device, bool Query )
{
	// Currently no MTD, so we can need to check only one device

	bool ret = true;

	if (Device && device)  // want to assign and there is already an assignment?
		ret = (Device == device);  // needs to be the same device

	if (!Query && ret) {  // is it an allowed de/assignment?
		if (Device)
			L_DBG( "DdCiAdapter(%s) assigned to device %d", *caDevName, GetDeviceNumber(Device) );
		else if (device)
			L_DBG( "DdCiAdapter(%s) unassigned from device %d", *caDevName, GetDeviceNumber() );
		device = Device;
	}

	return ret;
}
