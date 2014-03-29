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
// $Id:  $
/////////////////////////////////////////////////////////////////////////////

#include "ddciadapter.h"
#include "logging.h"

#include <vdr/device.h>

#include <sys/ioctl.h>
#include <linux/dvb/ca.h>


/* NOTE: Most of the code is copied from vdr/dvbci.c
 */

DdCiAdapter::DdCiAdapter(cDevice *dev, int ca_fd, int ci_fd, cString &devName)
: device(dev)
, fd_ca(ca_fd)
, fd_ci(ci_fd)
, caDevName(devName)
{
	if (!dev) {
		L_ERROR_STR("dev=NULL!");
		return;
	}

	SetDescription("DDCI adapter on device %d (%s)", device->DeviceNumber(), *devName);

	ca_caps_t Caps;
	if (ioctl(fd_ca, CA_GET_CAP, &Caps) == 0) {
		if ((Caps.slot_type & CA_CI_LINK) != 0) {
			int NumSlots = Caps.slot_num;
			if (NumSlots > 0) {
				for (int i = 0; i < NumSlots; i++)
					new cCamSlot(this);
				L_DBG("DdCiAdapter(%s) for device %d created", *devName, device->DeviceNumber());
				Start();
			} else
				L_ERR("no CAM slots found on device %d", device->DeviceNumber());
		} else
			L_INF("device %d doesn't support CI link layer interface", device->DeviceNumber());
	} else
		L_ERR("can't get CA capabilities on device %d", device->DeviceNumber());
}

//------------------------------------------------------------------------

DdCiAdapter::~DdCiAdapter()
{
	Cancel(3);
}

//------------------------------------------------------------------------

int DdCiAdapter::Read(uint8_t *Buffer, int MaxLength)
{
	if (Buffer && MaxLength > 0) {
		struct pollfd pfd[ 1 ];
		pfd[ 0 ].fd = fd_ca;
		pfd[ 0 ].events = POLLIN;
		if (poll(pfd, 1, CAM_READ_TIMEOUT) > 0 && (pfd[ 0 ].revents & POLLIN)) {
			int n = safe_read(fd_ca, Buffer, MaxLength);
			if (n >= 0)
				return n;
			L_ERR("can't read from CI adapter on device %d: %m", device->DeviceNumber());
		}
	}
	return 0;
}

//------------------------------------------------------------------------

void DdCiAdapter::Write(const uint8_t *Buffer, int Length)
{
	if (Buffer && Length > 0) {
		if (safe_write(fd_ca, Buffer, Length) != Length)
			L_ERR("can't write to CI adapter on device %d: %m", device->DeviceNumber());
	}
}

//------------------------------------------------------------------------

bool DdCiAdapter::Reset(int Slot)
{
	if (ioctl(fd_ca, CA_RESET, 1 << Slot) != -1)
		return true;
	else
		L_ERR("can't reset CAM slot %d on device %d: %m", Slot, device->DeviceNumber());
	return false;
}

//------------------------------------------------------------------------

eModuleStatus DdCiAdapter::ModuleStatus(int Slot)
{
	ca_slot_info_t sinfo;
	sinfo.num = Slot;
	if (ioctl(fd_ca, CA_GET_SLOT_INFO, &sinfo) != -1) {
		if ((sinfo.flags & CA_CI_MODULE_READY) != 0)
			return msReady;
		else if ((sinfo.flags & CA_CI_MODULE_PRESENT) != 0)
			return msPresent;
	} else
		L_ERR("can't get info of CAM slot %d on device %d: %m", Slot, device->DeviceNumber());
	return msNone;
}

//------------------------------------------------------------------------

bool DdCiAdapter::Assign(cDevice *Device, bool Query)
{
	// The CI is hardwired to its device, so there's not really much to do here
	if (Device)
		return Device == device;
	return true;
}

//------------------------------------------------------------------------

//DdCiAdapter *DdCiAdapter::CreateCiAdapter(cDevice *Device, int Fd)
//{
//  // TODO check whether a CI is actually present?
//  if (Device)
//     return new DdCiAdapter(Device, Fd);
//  return NULL;
//}
