#pragma once

#include <vector>
#include <string>
#include <convar.h>

#define CMD_TAG "_01_"


#define HAT_COMMAND(NAME, DESC) \
		static void NAME( const CCommand &args ); \
		static ConCommand NAME##_command( CMD_TAG###NAME, NAME, DESC ); \
		static void NAME( const CCommand &args )
