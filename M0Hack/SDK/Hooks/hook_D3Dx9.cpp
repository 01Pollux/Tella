
#include "winmain.hpp"
#include "D3Dx9.hpp"
#include "Helper/DrawTools.hpp"
#include "Helper/Offsets.hpp"

#include "Library/Lib_Sig.hpp"
#include "Profiler/mprofiler.hpp"

#include "GlobalHook/vtable.hpp"
#include "GlobalHook/listener.hpp"
#include "Cheats/Main.hpp"

#include "VGUI.hpp"

namespace WindowProcedureHook
{
	static WNDPROC OldWinProc = nullptr;
	static HWND TF2Window = nullptr;
	static bool ImGui_Init = false;

	static IUniqueVHook v_D3DEndScene;
	static IUniqueVHook v_D3DReset;

	LRESULT WINAPI WinProcCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void InitDevice();

	void InitWindow(bool first_time)
	{
		TF2Window = FindWindowA("Valve001", nullptr);
		OldWinProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(TF2Window, GWLP_WNDPROC, reinterpret_cast<LONG>(WinProcCallback)));

		if (first_time)
			InitDevice();
	}

	void InitImGui(IDirect3DDevice9* Device)
	{
		ImGui::SetCurrentContext(ImGui::CreateContext());
		ImGui::GetIO().IniFilename = NULL;

		ImGui_Impl_Init(TF2Window, Device);

		if (auto pEvent = M0EventManager::Find(EVENT_KEY_INIT_MENU))
		{
			pEvent();
			M0EventManager::Destroy(pEvent);
		}
	}
}


DECL_VHOOK_STATIC(HRESULT, WINAPI, WINAPI, EndScene, PDIRECT3DDEVICE9 ThisDevice)
{
	if (!WindowProcedureHook::ImGui_Init)
	{
		WindowProcedureHook::ImGui_Init = true;
		WindowProcedureHook::InitImGui(VHOOK_CAST_S(IDirect3DDevice9, ThisDevice));
	}

	if (MenuPanel::IsActive())
	{
		PROFILE_USECTION("ImGui Render", M0PROFILER_GROUP::DISPATCH_IMGUI);
		ImGui_ImplDX9_NewFrame();

		MenuPanel::Render();

		ImGui::EndFrame();
		ImGui::Render();

		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}

	return VHOOK_EXECUTE_S(EndScene, ThisDevice);
}


DECL_VHOOK_STATIC(HRESULT, WINAPI, WINAPI, Reset, PDIRECT3DDEVICE9 ThisDevice, D3DPRESENT_PARAMETERS* pParams)
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	SetWindowLongPtr(WindowProcedureHook::TF2Window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProcedureHook::OldWinProc));

	const HRESULT ret = VHOOK_EXECUTE_S(Reset, ThisDevice, pParams);

	if (SUCCEEDED(ret))
	{
		ImGui_ImplDX9_CreateDeviceObjects();

		WindowProcedureHook::InitWindow(false);
		ImGui_Impl_Init(WindowProcedureHook::TF2Window, VHOOK_CAST_S(IDirect3DDevice9, ThisDevice));
	}

	return ret;
}


extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
LRESULT WINAPI WindowProcedureHook::WinProcCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_KEYUP && wParam == VK_END)
	{
		MenuPanel::Toggle();
		if (MenuPanel::IsActive())
		{
			Interfaces::VGUISurface->UnlockCursor();
			Interfaces::VGUISurface->SetCursorAlwaysVisible(true);
		}
		else
		{
			Interfaces::VGUISurface->LockCursor();
			Interfaces::VGUISurface->SetCursorAlwaysVisible(false);
		}
	}

	if (MenuPanel::IsActive())
		ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
	return CallWindowProc(WindowProcedureHook::OldWinProc, hWnd, uMsg, wParam, lParam);
}


void WindowProcedureHook::InitDevice()
{
	M0Pointer pDevice = M0Libraries::D3DX9->FindPattern("pDirect3DDevice");

	v_D3DEndScene = VHOOK_REG_S(EndScene, pDevice, Offsets::D3D::VTIdx_EndScene);
	VHOOK_LINK_S(v_D3DEndScene, EndScene);

	v_D3DReset = VHOOK_REG_S(Reset, pDevice, Offsets::D3D::VTIdx_Reset);
	VHOOK_LINK_S(v_D3DReset, Reset);

	VGUI::M0SurfaceTools::Init();
	DrawTools::Update();
}


class D3Dx9_Loader
{
public:
	D3Dx9_Loader()
	{
		M0EventManager::AddListener(
			EVENT_KEY_LOAD_DLL_EARLY,
			[](M0EventData*)
			{
				WindowProcedureHook::InitWindow(true);
			},
			EVENT_NULL_NAME
		);

		M0EventManager::AddListener(
			EVENT_KEY_UNLOAD_DLL_LATE,
			std::bind(&D3Dx9_Loader::OnUnLoadDLL, this),
			EVENT_NULL_NAME
		);
	}

	void OnUnLoadDLL();

private:
} static dxd39_hook;


void D3Dx9_Loader::OnUnLoadDLL()
{
	WindowProcedureHook::v_D3DEndScene = nullptr;
	WindowProcedureHook::v_D3DReset = nullptr;

	SetWindowLongPtr(WindowProcedureHook::TF2Window, GWLP_WNDPROC, reinterpret_cast<LONG>(WindowProcedureHook::OldWinProc));

	MenuPanel::IsMenuActive = false;
	if (ImGui::GetCurrentContext())
	{
		ImGui_ImplDX9_Shutdown();
		ImGui::DestroyContext();
	}
}