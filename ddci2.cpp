/*
 * dvbsddevice.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id: dvbsddevice.c 3.3 2014/03/15 12:28:14 kls Exp $
 */

#include <getopt.h>
#include <vdr/plugin.h>
// #include "dvbsdffdevice.h"

static const char *VERSION = "0.0.1";
static const char *DESCRIPTION = "External Digital Devices ci-adapter";

class cPluginDdci2: public cPlugin
{
private:
	// cDvbSdFfDeviceProbe *probe;
public:
	cPluginDdci2(void);
	virtual ~cPluginDdci2();
	virtual const char *Version(void)
	{
		return VERSION;
	}
	virtual const char *Description(void)
	{
		return DESCRIPTION;
	}
	virtual const char *CommandLineHelp(void);
	virtual bool ProcessArgs(int argc, char *argv[]);
};

cPluginDdci2::cPluginDdci2(void)
{
	// probe = new cDvbSdFfDeviceProbe;
}

cPluginDdci2::~cPluginDdci2()
{
	// delete probe;
}

const char *cPluginDdci2::CommandLineHelp(void)
{
	return "  -o        --outputonly   do not receive, just use as output device\n";
}

bool cPluginDdci2::ProcessArgs(int argc, char *argv[])
{
	static struct option long_options[] = {
		{ "outputonly", no_argument, NULL, 'o' },
		{ NULL, no_argument, NULL, 0 }
	};

	int c;
	while ((c = getopt_long( argc, argv, "o", long_options, NULL )) != -1) {
		switch (c) {
		case 'o': // probe->SetOutputOnly(true);
			break;
		default:
			return false;
		}
	}
	return true;
}

VDRPLUGINCREATOR( cPluginDdci2 ); // Don't touch this!
