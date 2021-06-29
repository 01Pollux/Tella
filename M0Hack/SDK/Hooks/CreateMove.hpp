#pragma once

#include "GlobalHooks/tellahook.hpp"
#include "cdll_int.hpp"
#include "UserCmd.hpp"

TH_DECL_REFERENCE_MFP(CreateMove, "CreateMove", bool, float, UserCmd*);

namespace Interfaces { extern bool* pSendPacket; }