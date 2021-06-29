#include "Library/Lib.hpp"
#include "Library/Lib_Win.hpp"
#include "SDK/Interfaces.hpp"

#include <iostream>
#include <fstream>
#include <minidumpapiset.h>
#include <mmsystem.h>

#include "Profiler/mprofiler.hpp"

#include "Helper/Config.hpp"
#include "GlobalHooks/event_listener.hpp"
#include "GlobalHooks/detour.hpp"

#include "Helper/Debug.hpp"
#include "VGUI.hpp"


namespace tella
{
    namespace this_dll
    {
        static HMODULE hMod = nullptr;

        static FILE* STDOut = nullptr;

        static PVOID ExceptionHandler = nullptr;

        static __declspec(noreturn) void Close(bool manualfree)
        {
            if (manualfree)
            {
                _asm {
                    push NULL

                    push MEM_RELEASE
                    push NULL
                    push hMod

                    push ExitThread
                    jmp VirtualFree
                };
            }
            else FreeLibraryAndExitThread(hMod, NULL);
        }

        LONG NTAPI OnRaiseException(_In_ PEXCEPTION_POINTERS ExceptionInfo);

        static void ReleaseAllProfilers()
        {
            std::string time = FormatTime("__{0:%g}_{0:%h}_{0:%d}_{0:%H}_{0:%OM}_{0:%OS}"sv);
            IFormatterSV fmt("{}/{}{}.log"sv);
            constexpr M0PROFILER_FLAGS flags = bitmask::to_mask(M0PROFILER_FLAGS_::Stream_Seek_Beg, M0PROFILER_FLAGS_::Clear_State);

            for (M0PROFILER_GROUP_T i = 0; i < static_cast<M0PROFILER_GROUP_T>(M0PROFILER_GROUP::COUNT); ++i)
            {
                M0Profiler::OutputToStream(
                    static_cast<M0PROFILER_GROUP>(i),
                    fmt(M0PROFILER_OUT_STREAM, M0PROFILE_NAMES[i], time).c_str(),
                    flags
                );
            }
        }
    }
}

TH_DECL_DETOUR_SFP(void, OnWriteSteamMiniDump, UINT32, PVOID, LPCSTR) { }


static DWORD WINAPI InitDLL(PVOID)
{
    using namespace tella;

    Json::Value mainConfig;
    {
        std::ifstream file(TCONFIG_PATH);
        file >> mainConfig;
    }

    bool is_manual_mapped = TCONFIG var_storage::read_var(mainConfig, "DLL Info.Manual Mapping"sv).asBool();

    {
        if (Json::Value open_console = TCONFIG var_storage::read_var(mainConfig, "DLL Info.Console"sv);
            open_console.asBool())
        {
            if (!AllocConsole())
            {
                this_dll::Close(is_manual_mapped);
                return FALSE;
            }
            freopen_s(&this_dll::STDOut, "CONOUT$", "w", stderr);
        }
    }


    if (Json::Value exception_handler = TCONFIG var_storage::read_var(mainConfig, "DLL Info.Exception Handler"sv);
        exception_handler.asBool())
        this_dll::ExceptionHandler = AddVectoredExceptionHandler(1, this_dll::OnRaiseException);

    Interfaces::InitAllInterfaces();

    {
        if (Json::Value writesteammd = TCONFIG var_storage::read_var(mainConfig, "DLL Info.No SteamMiniDump"sv);
            writesteammd.asBool())
        {
            TH_DETOUR_LINK_TO_SFP(OnWriteSteamMiniDump, M0Library{ M0ENGINE_DLL }.FindPattern("FakeSteamMiniDump"));
        }
    }


    {
        if (Json::Value load_vars = TCONFIG var_storage::read_var(mainConfig, "DLL Info.Load.Auto Init Vars"sv);
            load_vars.asBool())
            TCONFIG var_storage::read_var();
    }

    if (auto pEvent = event_listener::find(event_listener::names::LoadDLL_Early))
    {
        pEvent();
        event_listener::destroy(pEvent);
    }

    if (auto pEvent = event_listener::find(event_listener::names::LoadDLL))
    {
        pEvent();
        event_listener::destroy(pEvent);
    }

    if (Json::Value play_sound = TCONFIG var_storage::read_var(mainConfig, "DLL Info.Load.Play Sound"sv);
        !play_sound.isNull())
        PlaySoundA(play_sound.asCString(), NULL, SND_ASYNC | SND_NODEFAULT | SND_NOSTOP);

    return TRUE;
}


static DWORD WINAPI UnloadDLL(PVOID)
{
    bool is_manual_mapped = TCONFIG var_storage::read_var("DLL Info.Manual Mapping"sv).asBool();
    tella::this_dll::Close(is_manual_mapped);
    return TRUE;
}


void ThisDLL::Unload()
{
    using namespace tella;

    if (auto pEvent = event_listener::find(event_listener::names::UnLoadDLL))
        pEvent();

    if (auto pEvent = event_listener::find(event_listener::names::UnLoadDLL_Late))
        pEvent();

    if (this_dll::STDOut)
    {
        fclose(this_dll::STDOut);
        FreeConsole();
    }

    Json::Value mainConfig;
    {
        std::ifstream file(TCONFIG_PATH);
        file >> mainConfig;
    }

    if (Json::Value save_profilers = TCONFIG var_storage::read_var(mainConfig, "DLL Info.Save Profilers"sv);
        save_profilers.asBool())
        this_dll::ReleaseAllProfilers();

    {
        if (hook::OnWriteSteamMiniDump_org)
            TH_DETOUR_UNLINK_FROM_SFP(OnWriteSteamMiniDump);
    }
    
    {
        if (Json::Value load_vars = TCONFIG var_storage::read_var(mainConfig, "DLL Info.Unload.Auto Save Vars"sv);
            load_vars.asBool())
            TCONFIG var_storage::write_var();
     
        if (Json::Value play_sound = TCONFIG var_storage::read_var(mainConfig, "DLL Info.Unload.Play Sound"sv);
            !play_sound.isNull())
            PlaySoundA(play_sound.asCString(), NULL, SND_ASYNC | SND_NODEFAULT | SND_NOSTOP);
    }

    if (this_dll::ExceptionHandler)
        RemoveVectoredExceptionHandler(this_dll::ExceptionHandler);

    HANDLE hThread = CreateThread(NULL, NULL, UnloadDLL, NULL, NULL, NULL);
    if (hThread)
        CloseHandle(hThread);
}


LONG NTAPI tella::this_dll::OnRaiseException(_In_ PEXCEPTION_POINTERS ExceptionInfo)
{
    class _AutoDetachSteamMinidump
    {
    public:
        ~_AutoDetachSteamMinidump() { TH_DETOUR_UNLINK_FROM_SFP(OnWriteSteamMiniDump); }
    } dummy_minidump;

    AutoLib DBGHelp("DBGHelp.dll");
    if (!DBGHelp)
        return EXCEPTION_CONTINUE_SEARCH;
   
    switch (ExceptionInfo->ExceptionRecord->ExceptionCode)
    {
    case EXCEPTION_ACCESS_VIOLATION:
    case EXCEPTION_INVALID_HANDLE:
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
    case EXCEPTION_DATATYPE_MISALIGNMENT:
    case EXCEPTION_ILLEGAL_INSTRUCTION:
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
    case EXCEPTION_STACK_OVERFLOW:
    case EXCEPTION_STACK_INVALID:
    case EXCEPTION_WRITE_FAULT:
    case EXCEPTION_READ_FAULT:
    case STATUS_STACK_BUFFER_OVERRUN:
    case STATUS_HEAP_CORRUPTION:
        break;
    case EXCEPTION_BREAKPOINT:
        if (!(ExceptionInfo->ExceptionRecord->ExceptionFlags & EXCEPTION_NONCONTINUABLE))
            return EXCEPTION_CONTINUE_SEARCH;
        break;
    default:
        return EXCEPTION_CONTINUE_SEARCH;
    }

    using MiniDumpWriteDumpFn =
        BOOL(WINAPI*)(
            _In_ HANDLE hProcess,
            _In_ DWORD ProcessId,
            _In_ HANDLE hFile,
            _In_ MINIDUMP_TYPE DumpType,
            _In_opt_ PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
            _In_opt_ PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
            _In_opt_ PMINIDUMP_CALLBACK_INFORMATION CallbackParam
        );

    AutoFile hFile("./Miku/Log/FATAL_{0:%g}_{0:%h}_{0:%d}_{0:%H}_{0:%OM}_{0:%OS}.dmp"sv);

    MiniDumpWriteDumpFn WriteMiniDump = std::bit_cast<MiniDumpWriteDumpFn>(GetProcAddress(*DBGHelp, "MiniDumpWriteDump"));

    tella::this_dll::ReleaseAllProfilers();

    MINIDUMP_EXCEPTION_INFORMATION MiniDumpInfo{
           .ThreadId = GetCurrentThreadId(),
           .ExceptionPointers = ExceptionInfo,
           .ClientPointers = FALSE };

    if (WriteMiniDump(
        GetCurrentProcess(),
        GetCurrentProcessId(),
        *hFile,
        static_cast<MINIDUMP_TYPE>(MiniDumpWithUnloadedModules | MiniDumpWithFullMemoryInfo | MiniDumpWithCodeSegs),
        &MiniDumpInfo,
        NULL,
        NULL)
        )
    {
        ExceptionInfo->ExceptionRecord->ExceptionCode = EXCEPTION_BREAKPOINT;
        return EXCEPTION_EXECUTE_HANDLER;
    }
    else return EXCEPTION_CONTINUE_SEARCH;
}


BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);
        tella::this_dll::hMod = hModule;
        HANDLE hThread = CreateThread(NULL, NULL, InitDLL, NULL, NULL, NULL);
        if (hThread)
            CloseHandle(hThread);
    }
    return TRUE;
}

#include "ConVar.hpp"
M01_CONCOMMAND(shutdown_dll, ThisDLL::Unload, "Shutdown DLL");


void HackWinAPI_StopSound()
{
    PlaySoundA(NULL, NULL, NULL);
}
M01_CONCOMMAND(stop_sound, HackWinAPI_StopSound);