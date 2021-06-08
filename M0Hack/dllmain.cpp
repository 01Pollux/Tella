#include "Library/Lib.hpp"
#include "Library/Lib_Win.hpp"
#include "SDK/Interfaces.hpp"

#include <iostream>
#include <minidumpapiset.h>

#include "Profiler/mprofiler.hpp"

#include "Helper/Config.hpp"
#include "GlobalHook/listener.hpp"
#include "GlobalHook/Detour/detours.hpp"

#include "Helper/Debug.hpp"

//#define M01_MANUAL_MAPPED_DLL

namespace ThisDLL
{
    static HMODULE hMod = nullptr;

#ifdef M01_USING_STDOUT
    static FILE* STDOut = nullptr;
#endif

#ifdef M01_USING_VECTORED_HANDLER
    static PVOID ExceptionHandler = nullptr;
#endif

    static __declspec(noreturn) void Close()
    {
#ifdef M01_MANUAL_MAPPED_DLL
        _asm {
            push NULL

            push MEM_RELEASE
            push NULL
            push hMod

            push ExitThread
            jmp VirtualFree
    };
#else
        FreeLibraryAndExitThread(ThisDLL::hMod, NULL);
#endif
    }

#ifdef M01_USING_VECTORED_HANDLER
    LONG NTAPI OnRaiseException(_In_ PEXCEPTION_POINTERS ExceptionInfo);
#endif

    static void ReleaseAllProfilers()
    {
        std::string time = FormatTime("__{0:%g}_{0:%h}_{0:%d}_{0:%H}_{0:%OM}_{0:%OS}"sv);
        IFormatterSV fmt("{}/{}{}.txt"sv);

        for (M0PROFILER_GROUP_T i = 0; i < static_cast<M0PROFILER_GROUP_T>(M0PROFILER_GROUP::COUNT); ++i)
        {
            M0Profiler::OutputToStream(
                static_cast<M0PROFILER_GROUP>(i),
                fmt(M0PROFILER_OUT_STREAM, M0PROFILE_NAMES[i], time).c_str(),
                M0PROFILER_FLAGS::DEFAULT_OUTPUT
            );
        }
    }
    
    DETOUR_CREATE_STATIC(void, OnWriteSteamMiniDump, UINT32, PVOID, LPCSTR) { }
}


static DWORD WINAPI InitDLL(PVOID)
{
#ifdef M01_USING_STDOUT
    AllocConsole();
    freopen_s(&ThisDLL::STDOut, "CONOUT$", "w", stdout);
    if (!ThisDLL::STDOut)
    {
        Beep(400, 1000);
        ThisDLL::Close();
        return FALSE;
    }
#endif

#ifdef M01_USING_VECTORED_HANDLER
    ThisDLL::ExceptionHandler = AddVectoredExceptionHandler(1, ThisDLL::OnRaiseException);
#endif

    Interfaces::InitAllInterfaces();

    {
        PVOID addr = M0Libraries::Engine->FindPattern("FakeSteamMiniDump");
        DETOUR_LINK_TO_STATIC(ThisDLL::OnWriteSteamMiniDump, addr);
    }

    if (auto pEvent = M0EventManager::Find(EVENT_KEY_LOAD_DLL_EARLY))
    {
        pEvent();
        M0EventManager::Destroy(pEvent);
    }

    if (auto pEvent = M0EventManager::Find(EVENT_KEY_LOAD_DLL))
    {
        pEvent();
        M0EventManager::Destroy(pEvent);
    }

    // Todo M0GConfig for tempo globals like auto save
    {
        //M0Config::M0VarStorage::read_var();
    }

    return TRUE;
}


static DWORD WINAPI UnloadDLL(PVOID)
{
    ThisDLL::Close();
    return TRUE;
}


void ThisDLL::Unload()
{
    if (auto pEvent = M0EventManager::Find(EVENT_KEY_UNLOAD_DLL))
        pEvent();

    if (auto pEvent = M0EventManager::Find(EVENT_KEY_UNLOAD_DLL_LATE))
        pEvent();

    ThisDLL::ReleaseAllProfilers();

#ifdef M01_USING_STDOUT
    fclose(ThisDLL::STDOut);
    FreeConsole();
#endif

#ifdef M01_USING_VECTORED_HANDLER
    if (ThisDLL::ExceptionHandler)
        RemoveVectoredExceptionHandler(ThisDLL::ExceptionHandler);
#endif

    DETOUR_UNLINK_FROM_STATIC(ThisDLL::OnWriteSteamMiniDump);
    
    HANDLE hThread = CreateThread(NULL, NULL, UnloadDLL, NULL, NULL, NULL);
    if (hThread)
        CloseHandle(hThread);
}


#ifdef M01_USING_VECTORED_HANDLER
LONG NTAPI ThisDLL::OnRaiseException(_In_ PEXCEPTION_POINTERS ExceptionInfo)
{
    class _AutoDetachSteamMinidump
    {
    public:
        ~_AutoDetachSteamMinidump() { DETOUR_UNLINK_FROM_STATIC(ThisDLL::OnWriteSteamMiniDump); }
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

    MiniDumpWriteDumpFn WriteMiniDump = reinterpret_cast<MiniDumpWriteDumpFn>(GetProcAddress(*DBGHelp, "MiniDumpWriteDump"));

    ThisDLL::ReleaseAllProfilers();

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
#endif


BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);
        ThisDLL::hMod = hModule;
        HANDLE hThread = CreateThread(NULL, NULL, InitDLL, NULL, NULL, NULL);
        if (hThread)
            CloseHandle(hThread);
    }
    return TRUE;
}

#include "ConVar.hpp"
M01_CONCOMMAND(shutdown_dll, ThisDLL::Unload, "Shutdown DLL");