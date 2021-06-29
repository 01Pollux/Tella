#include "Library/Lib.hpp"
#include "Interfaces.hpp"

#include "cdll_int.hpp"

#include "ConVar.hpp"

#include "GlobalVars.hpp"

#include "VGUI.hpp"
#include "Helper/DrawTools.hpp"

#include "ModelInfo.hpp"

#include "DebugOverlay.hpp"

#include "Trace.hpp"

#include "Glow.hpp"

#include "Entity/EntityList.hpp"

#include "NetStringTable.hpp"

#include "GameEvent.hpp"

M0_INTERFACE;

IVEngineClient* EngineClient = nullptr;
IBaseClientDLL* ClientDLL = nullptr;

ICVar* CVar = nullptr;

IValveGlobalVars* GlobalVars = nullptr;

VGUI::IPanel* VGUIPanel = nullptr;
VGUI::ISurface* VGUISurface = nullptr;

IVModelInfo* MdlInfo = nullptr;

IVDebugOverlay* DebugOverlay = nullptr;

IEngineTrace* ClientTrace = nullptr;

IGlowManager* GlowManager = nullptr;

IClientEntityList* EntityList = nullptr;

INetworkStringTableContainer* NSTContainer = nullptr;

IGameEventManager* GameEventMgr = nullptr;

M0_END;



void Interfaces::InitAllInterfaces()
{
	M0Library engine_dll{ M0ENGINE_DLL }, client_dll{ M0CLIENT_DLL }, vstd_dll{ M0VALVESTD_DLL }, matsurface_dll{ M0MATSURFACE_DLL };

	EngineClient		= static_cast<IVEngineClient*>(engine_dll.FindInterface(EngineClientName));
	ClientDLL			= static_cast<IBaseClientDLL*>(client_dll.FindInterface(ClientDLLName));

	CVar				= static_cast<ICVar*>(vstd_dll.FindInterface(CVarName));

	GlobalVars			= static_cast<IValveGlobalVars*>(engine_dll.FindPattern("pGlobalVarBasePtr"));

	VGUIPanel			= static_cast<VGUI::IPanel*>(client_dll.FindPattern("g_pVGUIPanel"));
	VGUISurface			= static_cast<VGUI::ISurface*>(matsurface_dll.FindInterface(SurfaceName));

	MdlInfo				= static_cast<IVModelInfo*>(engine_dll.FindInterface(MdlInfoName));

	DebugOverlay		= static_cast<IVDebugOverlay*>(engine_dll.FindInterface(DebugOverlayName));

	ClientTrace			= static_cast<IEngineTrace*>(engine_dll.FindInterface(ClientTraceName));

	GlowManager			= static_cast<IGlowManager*>(client_dll.FindPattern("GlowManager"));

	EntityList			= static_cast<IClientEntityList*>(client_dll.FindInterface(EntityListName));

	NSTContainer		= static_cast<INetworkStringTableContainer*>(engine_dll.FindInterface(NSTContainerName));

	GameEventMgr		= static_cast<IGameEventManager*>(engine_dll.FindInterface(GameEventMgrName));
}

namespace VGUI
{
	namespace M0SurfaceTools
	{
		namespace Fonts
		{
			HFont Arial{ };
			HFont ArialSmall{ };
			HFont ArialLarge{ };
		}

		namespace Textures
		{
			int White{ };
			int Cyan{ };
			int Fuschia{ };
		}
		void Init()
		{
			using namespace Interfaces;

			Fonts::Arial = VGUISurface->CreateFontW();
			VGUISurface->SetFontGlyphSet(Fonts::Arial, "Arial", VGUI::M0SurfaceTools::StringOffset, 400, 0, 0, VGUI::ISurface::FONTFLAG_ANTIALIAS | VGUI::ISurface::FONTFLAG_OUTLINE);

			Fonts::ArialSmall = VGUISurface->CreateFontW();
			VGUISurface->SetFontGlyphSet(Fonts::ArialSmall, "Arial", VGUI::M0SurfaceTools::StringOffset, 200, 0, 0, VGUI::ISurface::FONTFLAG_ANTIALIAS);

			Fonts::ArialLarge = VGUISurface->CreateFontW();
			VGUISurface->SetFontGlyphSet(Fonts::ArialLarge, "Arial", VGUI::M0SurfaceTools::StringOffset, 600, 0, 0, VGUI::ISurface::FONTFLAG_ANTIALIAS | VGUI::ISurface::FONTFLAG_DROPSHADOW);


			Textures::White = VGUISurface->CreateNewTextureID();
			VGUISurface->DrawSetTextureRGBA(Textures::White, reinterpret_cast<const unsigned char*>(color::names::white.data()), 1, 1, false, true);

			Textures::Cyan = VGUISurface->CreateNewTextureID();
			VGUISurface->DrawSetTextureRGBA(Textures::Cyan, reinterpret_cast<const unsigned char*>(color::names::cyan.data()), 1, 1, false, true);

			Textures::Fuschia = VGUISurface->CreateNewTextureID();
			VGUISurface->DrawSetTextureRGBA(Textures::Fuschia, reinterpret_cast<const unsigned char*>(color::names::fuschia.data()), 1, 1, false, true);
		}
	}
}

void Interfaces::ForceClientFullUpdate()
{
	if (IBaseEntityInternal::BadLocal())
		return;

	static void* ClientState = M0Library{ M0ENGINE_DLL }.FindPattern("pBaseClientStatePtr");
	static IMemberFuncThunk<void> ForceFullupdate(M0ENGINE_DLL, "ClientState::ForceFullUpdate");

	ForceFullupdate(ClientState);
}