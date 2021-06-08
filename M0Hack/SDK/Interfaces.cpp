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
	M0Libraries::InitLibrary();
	
	EngineClient		= reinterpret_cast<IVEngineClient*>(M0Libraries::Engine->FindInterface(EngineClientName));
	ClientDLL			= reinterpret_cast<IBaseClientDLL*>(M0Libraries::Client->FindInterface(ClientDLLName));

	CVar				= reinterpret_cast<ICVar*>(M0Libraries::ValveSTD->FindInterface(CVarName));

	GlobalVars			= reinterpret_cast<IValveGlobalVars*>(M0Libraries::Engine->FindPattern("pGlobalVarBasePtr"));

	VGUIPanel			= reinterpret_cast<VGUI::IPanel*>(M0Libraries::Client->FindPattern("g_pVGUIPanel"));
	VGUISurface			= reinterpret_cast<VGUI::ISurface*>(M0Libraries::MatSurface->FindInterface(SurfaceName));

	MdlInfo				= reinterpret_cast<IVModelInfo*>(M0Libraries::Engine->FindInterface(MdlInfoName));

	DebugOverlay		= reinterpret_cast<IVDebugOverlay*>(M0Libraries::Engine->FindInterface(DebugOverlayName));

	ClientTrace			= reinterpret_cast<IEngineTrace*>(M0Libraries::Engine->FindInterface(ClientTraceName));

	GlowManager			= reinterpret_cast<IGlowManager*>(M0Libraries::Client->FindPattern("GlowManager"));

	EntityList			= reinterpret_cast<IClientEntityList*>(M0Libraries::Client->FindInterface(EntityListName));

	NSTContainer		= reinterpret_cast<INetworkStringTableContainer*>(M0Libraries::Engine->FindInterface(NSTContainerName));

	GameEventMgr		= reinterpret_cast<IGameEventManager*>(M0Libraries::Engine->FindInterface(GameEventMgrName));
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

	static M0Pointer ClientState = M0Libraries::Engine->FindPattern("pBaseClientStatePtr");
	static IMemberFuncThunk<void> ForceFullupdate(M0Libraries::Engine->FindPattern("ClientState::ForceFullUpdate"));

	ForceFullupdate(ClientState);
}