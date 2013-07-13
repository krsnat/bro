
#include "plugin/Plugin.h"

#include "TFTP.h"

BRO_PLUGIN_BEGIN(Bro, TFTP)
	BRO_PLUGIN_DESCRIPTION("TFTP analyzer");
	BRO_PLUGIN_ANALYZER("TFTP", tftp::TFTP_Analyzer);
	BRO_PLUGIN_BIF_FILE(events);
BRO_PLUGIN_END
