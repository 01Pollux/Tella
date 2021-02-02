#pragma warning (disable : 26812)

//#define _01_MANUALMAPPED_DLL
#define _01_ALLOC_CONSOLE
#define _01_USE_VECTORED_HANDLER

#include "Helpers/DrawTools.h"
#include "Helpers/sdk.h"
#include "Interfaces/HatCommand.h"
#include "Interfaces/Main.h"
#include "Helpers/VTable.h"

#include "Helpers/NetVars.h"
#include "Helpers/Config.h"

#include <convar.h>

#include <thread>
#include <minidumpapiset.h>


IClientEntityList* clientlist;
IVEngineClient* engineclient;
INetworkStringTableContainer* nstcontainer;
IBaseClientDLL* clientdll;
IClientTools* clienttools;
IFileSystem* filesystem;
IGameEventManager2* eventmanager;
CGlobalVarsBase* gpGlobals;
IVModelInfo* modelinfo;
vgui::ISurface* surface;

static LPVOID hMod;



#if defined _01_USE_VECTORED_HANDLER
LONG WINAPI OnExceptionHandle(_In_ PEXCEPTION_POINTERS ExceptionInfo)
{
    class AUTOLIB
    {
        HMODULE Module{ };
    public:
        explicit AUTOLIB(LPCSTR Name)
        {
            Module = LoadLibraryA(Name);
        }

        operator bool()
        {
            return Module != NULL;
        }

        CONST HMODULE operator*() CONST noexcept
        {
            return Module;
        }
        ~AUTOLIB()
        {
            if (Module)
            {
                FreeLibrary(Module);
                Module = NULL;
            }
        }
    };

    AUTOLIB DBGHelp("DBGHelp.dll");
    if (!DBGHelp)
        return EXCEPTION_CONTINUE_SEARCH;

    using MiniDumpWriteDumpFn = 
        BOOL (WINAPI*)(
            _In_ HANDLE hProcess,
            _In_ DWORD ProcessId,
            _In_ HANDLE hFile,
            _In_ MINIDUMP_TYPE DumpType,
            _In_opt_ PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
            _In_opt_ PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
            _In_opt_ PMINIDUMP_CALLBACK_INFORMATION CallbackParam
        );

    MiniDumpWriteDumpFn WriteDump = reinterpret_cast<MiniDumpWriteDumpFn>(GetProcAddress(*DBGHelp, "MiniDumpWriteDump"));
    if (!WriteDump)
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

    class AUTOFILE
    {
        HANDLE File{ INVALID_HANDLE_VALUE };

    public:
        AUTOFILE()
        {
            CHAR Buffer[256];
            time_t this_time; time(&this_time);
            tm* time_info = localtime(&this_time);
            strftime(Buffer, sizeof(Buffer), "./Miku/Log/FATAL_%g_%h_%d_%H_%M_%S.dmp", time_info);

            File = CreateFileA(Buffer, GENERIC_WRITE, NULL, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        }

        CONST HANDLE operator*() CONST noexcept
        {
            return File;
        }

        ~AUTOFILE()
        {
            if (File != INVALID_HANDLE_VALUE)
            {
                CloseHandle(File);
                File = INVALID_HANDLE_VALUE;
            }
        }
    } AutoFile;

    MINIDUMP_EXCEPTION_INFORMATION MiniDumpInfo {
            .ThreadId = GetCurrentThreadId(),
            .ExceptionPointers = ExceptionInfo,
            .ClientPointers = FALSE };


    if (WriteDump(GetCurrentProcess(),
                  GetCurrentProcessId(), 
                  *AutoFile,
                  MINIDUMP_TYPE(MiniDumpWithUnloadedModules | MiniDumpWithFullMemoryInfo), 
                  &MiniDumpInfo, 
                  NULL, 
                  NULL))
    {
        ExceptionInfo->ExceptionRecord->ExceptionCode = EXCEPTION_BREAKPOINT;
        return EXCEPTION_EXECUTE_HANDLER;
    }
    else return EXCEPTION_CONTINUE_SEARCH;
}
#endif


static _declspec(noreturn) void FreeMappedDllAndExitThread()
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

VOID MainThread(LPVOID)
{
#ifdef _01_ALLOC_CONSOLE
    FILE* _std_cout;
    AllocConsole();
    freopen_s(&_std_cout, "CONOUT$", "w", stdout);
    if (!_std_cout)
    {
        Beep(400, 1000);
#ifdef _01_MANUALMAPPED_DLL
        FreeMappedDllAndExitThread();
#else
        FreeLibraryAndExitThread(HMODULE(hMod), NULL);
#endif
        return;
    }
#endif

#ifdef _01_USE_VECTORED_HANDLER
    PVOID VecHandler = AddVectoredExceptionHandler(1, OnExceptionHandle);
#endif


#pragma region Init_
    {
        Interfaces::Init();
        ConVar_Register();

        IGlobalEvent::LoadDLL::Hook::RunAllHooks();
        IGlobalEvent::LoadDLL::Hook::Clear();

        InitializeRecvProp();
        DrawTools::Init();

        g_pCVar->ConsoleColorPrintf(DrawTools::ColorTools::FromArray(DrawTools::ColorTools::Cyan<char8_t>), "Miku Miku~\n");
    }
#pragma region

    while (TRUE)
    {
        if (DrawTools::IsMarkedForDeletion())
        {
            Beep(400, 1000);
            break;
        }

        Sleep(10000);
    }

#pragma region Unload_
    {
        IGlobalEvent::UnloadDLL::Hook::RunAllHooks();

        ConVar_Unregister();

        DrawTools::Destroy();
        Interfaces::Delete();

        CleanupRecvProp();

#ifdef _01_ALLOC_CONSOLE
        fclose(_std_cout);
        FreeConsole();
#endif
    }
#pragma region

#ifdef _01_USE_VECTORED_HANDLER
    RemoveVectoredExceptionHandler(VecHandler);
#endif

#ifdef _01_MANUALMAPPED_DLL
    FreeMappedDllAndExitThread();
#else
    FreeLibraryAndExitThread(HMODULE(hMod), NULL);
#endif
}


BOOL WINAPI DllMain(HMODULE hModule, ulong ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) 
    {
        hMod = hModule;
        CloseHandle(
        CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(MainThread), NULL, NULL, NULL)
        );
    }
    return TRUE;
}


HAT_COMMAND(shutdown_dll, "Shutdown DLL")
{
    DrawTools::MarkForDeletion();
}

HAT_COMMAND(toggle_miku_menu, "Toggle Menu Hack")
{
    Mmain.m_bIsActive = !Mmain.m_bIsActive;
}