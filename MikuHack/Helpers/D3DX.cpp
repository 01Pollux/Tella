
#include "D3DX.h"
#include "DrawTools.h"
#include "Config.h"
#include "Library.h"
#include "../Source/Main.h"
#include "VTable.h"

#include "../Interfaces/VGUIS.h"
#include "../Profiler/mprofiler.h"


static WNDPROC		oWndProc = nullptr;
static HWND			hWindow = nullptr;
LRESULT WINAPI		hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static inline bool IsKeyDown(int key)
{
	return GetAsyncKeyState(key) & 1;
}

static inline void InitWindow()
{
	hWindow = FindWindowA("Valve001", nullptr);
	oWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(hooked_WndProc)));
}

static IUniqueVHook v_D3DEndScene;
DECL_VHOOK_STATIC(HRESULT, WINAPI, WINAPI, EndScene, PDIRECT3DDEVICE9 ThisDevice)
{
	static std::once_flag init_once;

	std::call_once(init_once, 
	[&]() {
		ImGui::CreateContext();
		ImGui::GetIO().IniFilename = NULL;

		ImGui_Impl_Init(hWindow, VHOOK_CAST_S(IDirect3DDevice9, ThisDevice));

		for (auto& list = IAutoList<MenuPanel>::List(); auto& tab : list)
			tab->OnMenuInit();
	});

	if (Mmain.m_bIsActive)
	{
		M0Profiler imgui_profiler("MainMenu::DoRender", M0PROFILER_GROUP::DISPATCH_IMGUI);
		
		ImGui_ImplDX9_NewFrame();

		MainMenu::DoRender();

		ImGui::EndFrame();

		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}

	return VHOOK_EXECUTE_S(EndScene, ThisDevice);
}


static IUniqueVHook v_D3DReset;
DECL_VHOOK_STATIC(HRESULT, WINAPI, WINAPI, Reset, PDIRECT3DDEVICE9 ThisDevice, D3DPRESENT_PARAMETERS* pParams)
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	SetWindowLongPtr(hWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(oWndProc));

	HRESULT ret = VHOOK_EXECUTE_S(Reset, ThisDevice, pParams);

	ImGui_ImplDX9_CreateDeviceObjects();

	InitWindow();
	ImGui_Impl_Init(hWindow, VHOOK_CAST_S(IDirect3DDevice9, ThisDevice));

	return ret;
}


extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
LRESULT STDMETHODCALLTYPE hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static bool is_down = false;
	static bool is_clicked = false;
	static bool check_closed = false;

	if (IsKeyDown(VK_END))
	{
		is_clicked = false;
		is_down = true;
	}
	else if (!IsKeyDown(VK_END) && is_down)
	{
		is_clicked = true;
		is_down = false;
	}
	else
	{
		is_clicked = false;
		is_down = false;
	}

	if (!Mmain.m_bIsActive && !is_clicked && check_closed)
	{
		check_closed = false;
	}

	if (is_clicked)
	{
		Mmain.m_bIsActive = !Mmain.m_bIsActive;
		if (Mmain.m_bIsActive)
		{
			surface->UnlockCursor();
			surface->SetCursorAlwaysVisible(true);
		}
		else
		{
			surface->LockCursor();
			surface->SetCursorAlwaysVisible(false);
		}

		if (!check_closed)
			check_closed = true;
	}

	if (Mmain.m_bIsActive)
	{
		ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
		return true;
	}

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}



bool DrawTools::Init()
{
	Update();

	InitWindow();

	uintptr_t pDevice = Library::d3dlib.FindPattern("pDirect3DDevice");

	v_D3DEndScene = VHOOK_REG_S(EndScene, pDevice, Offsets::D3D::VTIdx_EndScene);
	VHOOK_LINK_S(v_D3DEndScene, EndScene);

	v_D3DReset = VHOOK_REG_S(Reset, pDevice, Offsets::D3D::VTIdx_Reset);
	VHOOK_LINK_S(v_D3DReset, Reset);

	InitSurface();

	return true;
}

void DrawTools::Destroy()
{
	v_D3DEndScene = nullptr;
	v_D3DReset = nullptr;

	SetWindowLongPtr(hWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(oWndProc));

	if (ImGui::GetCurrentContext())
	{
		ImGui_ImplDX9_Shutdown();
		ImGui::DestroyContext();
	}
}