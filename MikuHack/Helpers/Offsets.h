#pragma once

#define _OFFSET constexpr int
#define _VTABLE(NAME) VTIdx_##NAME

/// Fuck it, i ain't going to hardcode it deep into this project,
///
/// Signatures.json contains signatures 
/// 		signatures = pattern, use '?' or '2A' or '*', all three of them behave as wildcards
///			offset = Address + X
///			read = *(Address + offset)
/// 
/// Offsets.h	contains well organized constants compile-time offsets 

namespace Offsets
{
	namespace ClientDLL 
	{
		_OFFSET pUpdatePlayerList_ValidateRet = 0x993;
		_OFFSET To_EntListenerVec		= 0x10018;

		_OFFSET _VTABLE(CreateMove)		= 21;
		_OFFSET _VTABLE(LevelInit)		= 22;
		_OFFSET _VTABLE(LevelShutdown)	= 23;

		_OFFSET _VTABLE(PaintTraverse)	= 41;

		_OFFSET _VTABLE(FrameStageNotify)= 35;
		_OFFSET _VTABLE(DisptachUserMsg) = 36;

		namespace PlayerData_t
		{
			_OFFSET OffsetToModel	= 0x20;
			_OFFSET	SizeOfModel		= 0x80;
		}
	}

	namespace IBaseEntity
	{
		namespace TFPlayerShared
		{
			_OFFSET m_pOuter = 392;
		}
		_OFFSET _VTABLE(IsBaseCombatWeapon) = 137;
		_OFFSET _VTABLE(GetSlot)			= 327;
		_OFFSET _VTABLE(DoSwingTrace)		= 454;
		_OFFSET _VTABLE(GetSwingRange)		= 452;
		_OFFSET _VTABLE(UpdateGlowEffect)	= 226;
		_OFFSET _VTABLE(DestroyGlowEffect)	= 227;
	}

	namespace D3D
	{
		_OFFSET _VTABLE(Reset)		= 16;
		_OFFSET _VTABLE(EndScene)	= 42;
	}

	namespace VGUI
	{
		_OFFSET _VTABLE(LockCursor) = 67;
	}
}