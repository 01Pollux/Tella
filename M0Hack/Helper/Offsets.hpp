#pragma once

#define M0OFFSET constexpr int
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
		M0OFFSET pUpdatePlayerList_ValidateRet = 0x993;

		M0OFFSET _VTABLE(CreateMove)	= 21;
		M0OFFSET _VTABLE(LevelInit)		= 22;
		M0OFFSET _VTABLE(LevelShutdown)	= 23;

		M0OFFSET _VTABLE(FrameStageNotify)= 35;
		M0OFFSET _VTABLE(DisptachUserMsg) = 36;

		M0OFFSET _VTABLE(SendDataGram)	 = 47;

		namespace PlayerData_t
		{
			M0OFFSET OffsetToModel	= 0x20;
			M0OFFSET SizeOfModel	= 0x80;
		}

		namespace HUD
		{
			/**
			 * string: "Could not find Hud Element: %s\n"
			 * 
			 * cmp		[edi+28h], esi;		(edi = this) & ([edi + 28h] = CValveUtlVector::Size)
			 *
			 * mov		ecx, [eax+esi*4];	(eax = [edi + 1Ch]) & esi is the index
			 */
			namespace Manager
			{
				M0OFFSET This_To_Vec = 7;
				M0OFFSET This_To_SizeOfVec = 10;
			}
		}

		/**
		 * CHLClient::LevelShutdown has a cl_entitylist_ptr
		 *
		 * use it find virtual dtor for CClientEntityList
		 * Note: it should lead you to two paths: one has alot of virtual functions, one doesn't, use the latter one
		 * 
		 * CClientEntityList::OnRemoveEntity
		 * CClientEntityList::~CClientEntityList
		 * ...
		 * ...
		 */
		namespace ClientEntList
		{
			M0OFFSET EntListenerVec = 0x10018;

			M0OFFSET HighestEntityIndex = 0x10038;
		}
	}

	namespace IBaseEntity
	{
		namespace TFPlayerShared
		{
			M0OFFSET m_pOuter = 392;
		}

		namespace Dispenser
		{
			M0OFFSET m_iMiniBombCounter__To__m_hHealingTargets = -32;
			M0OFFSET m_iState__To__m_iAmmoMetal = 4;
		}

		M0OFFSET m_flElasticity__To__ParticleProp = -32;

		M0OFFSET _VTABLE(IsBaseCombatWeapon) = 137;
		M0OFFSET _VTABLE(GetSlot)			= 327;
		M0OFFSET _VTABLE(DoSwingTrace)		= 454;
		M0OFFSET _VTABLE(GetSwingRange)		= 452;
		M0OFFSET _VTABLE(UpdateGlowEffect)	= 226;
		M0OFFSET _VTABLE(DestroyGlowEffect)	= 227;
		M0OFFSET _VTABLE(GetDataMapDesc)	= 15;
		M0OFFSET _VTABLE(EyePosition)		= 140;
	}

	namespace D3D
	{
		M0OFFSET _VTABLE(Reset)		= 16;
		M0OFFSET _VTABLE(EndScene)	= 42;
	}

	namespace VGUI
	{
		M0OFFSET _VTABLE(LockCursor)	= 67;
		M0OFFSET _VTABLE(PaintTraverse) = 41;

		namespace Panel
		{
			M0OFFSET VPanel = 0x50 - 4;
			M0OFFSET Name = 0x54-4;

			M0OFFSET FGColor = 0x88;
			M0OFFSET BGColor = 0x8C;

		}
	}
}