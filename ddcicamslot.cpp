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
#include "logging.h"

// #include <vdr/device.h>

// #include <sys/ioctl.h>
// #include <linux/dvb/ca.h>

DdCiCamSlot::DdCiCamSlot(DdCiAdapter *adapter)
: cCamSlot(adapter, true)
{
	LOG_FUNCTION_ENTER;
	LOG_FUNCTION_EXIT;

/*
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
					new DdCiCamSlot(this);
				L_DBG("DdCiCamSlot(%s) for device %d created", *devName, device->DeviceNumber());
				Start();
			} else
				L_ERR("no CAM slots found on device %d", device->DeviceNumber());
		} else
			L_INF("device %d doesn't support CI link layer interface", device->DeviceNumber());
	} else
		L_ERR("can't get CA capabilities on device %d", device->DeviceNumber());
*/
}

//------------------------------------------------------------------------

DdCiCamSlot::~DdCiCamSlot()
{
	LOG_FUNCTION_ENTER;
	LOG_FUNCTION_EXIT;
}

//------------------------------------------------------------------------

uchar *DdCiCamSlot::Decrypt(uchar *Data, int &Count)
{
	Count = 0;
	return 0;
}
