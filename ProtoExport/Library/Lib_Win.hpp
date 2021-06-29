#pragma once

#include "winmain.hpp"
#include "Helper/Format.hpp"

class AutoLib
{
public:
    AutoLib(LPCSTR Name) noexcept
    {
        hModule = LoadLibraryA(Name);
    }

    constexpr operator bool() const noexcept
    {
        return hModule != NULL;
    }

    const HMODULE operator*() const noexcept
    {
        return hModule;
    }

    ~AutoLib()
    {
        if (hModule)
        {
            FreeLibrary(hModule);
            hModule = NULL;
        }
    }

    AutoLib(AutoLib&) = delete;
    AutoLib operator=(const AutoLib&) = delete;
    AutoLib(AutoLib&&) = delete;
    AutoLib operator=(AutoLib&&) = delete;

private:
    HMODULE hModule;
};

class AutoFile
{
    HANDLE File{ INVALID_HANDLE_VALUE };

public:
    AutoFile(const std::string_view& unformatted_name) noexcept
    {
        std::string time = FormatTime(unformatted_name);

        File = CreateFileA(time.c_str(), GENERIC_WRITE, NULL, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    }

    constexpr HANDLE operator*() const noexcept
    {
        return File;
    }

    ~AutoFile()
    {
        if (File != INVALID_HANDLE_VALUE)
        {
            CloseHandle(File);
            File = INVALID_HANDLE_VALUE;
        }
    }

    AutoFile(const AutoFile&) = delete;
    AutoFile& operator=(const AutoFile&) = delete;
    AutoFile(AutoFile&&) = delete;
    AutoFile& operator=(AutoFile&&) = delete;
};