#pragma once

#define WIN32_MEAN_AND_LEAN
#include <Windows.h>
#include "Detour/detours.hpp"
#include <memory>

namespace tella
{
	namespace hook
	{
        class detour
        {
        public:
            static constexpr LONG ERROR_NOT_SET = 0x4860139;

            detour(void* addr, void* callback)
            {
                try_attach(addr, callback);
            }

            LONG try_attach(void* addr, void* callback)
            {
                this->Callback = callback;
                this->ActualFunc = addr;

                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());
                DetourAttach(&ActualFunc, Callback);
                return DetourTransactionCommit();
            }

            LONG try_detach()
            {
                bool res = false;
                if (IsSet)
                {
                    DetourTransactionBegin();
                    DetourUpdateThread(GetCurrentThread());
                    DetourDetach(&ActualFunc, Callback);
                    return DetourTransactionCommit();
                }
                
                return ERROR_NOT_SET;
            }

            bool is_set() const noexcept { return IsSet; }

            void* original_function() const noexcept { return ActualFunc; }

            void* callback_function() const noexcept { return Callback; }

            ~detour() noexcept(false)
            {
                try_detach();
            }

            bool operator==(const detour&) const = default;

        public:
            detour() = default;
            detour(const detour&)   = delete;  detour& operator=(const detour&) = delete;
            detour(detour&&)        = default; detour& operator=(detour&&)      = default;

        private:
            void*   Callback{ };
            void*   ActualFunc{ };
            bool    IsSet = false;
        };

        using unique_detour = std::unique_ptr<detour>;
	}
}


#define TH_DECL_DETOUR_MFP(RETURN, NAME, ...) \
namespace tella \
{ \
    namespace hook \
    { \
        class NAME##_class \
        { \
        public: \
            using NAME##_fn = RETURN(NAME##_class::*)(__VA_ARGS__); \
            RETURN NAME(__VA_ARGS__); \
            static inline NAME##_fn NAME##_org = nullptr; \
        }; \
    } \
} \
RETURN tella::hook::NAME##_class::NAME(__VA_ARGS__)

#define TH_DETOUR_CALL_MFP(NAME, ...)       (this->*NAME##_org)(__VA_ARGS__)

#define TH_DETOUR_GET_THIS(CLASS)           std::bit_cast<CLASS*>(this)

#define TH_DETOUR_LINK_TO_MFP(NAME, ADDRESS) \
do { \
    tella::hook::NAME##_class::NAME##_org = std::bit_cast<tella::hook::NAME##_class::NAME##_fn>(ADDRESS);	\
    DetourTransactionBegin(); \
    DetourUpdateThread(GetCurrentThread()); \
    DetourAttach(std::bit_cast<PVOID&>(&tella::hook::NAME##_class::NAME##_org), std::bit_cast<void*>(&tella::hook::NAME##_class::NAME)); \
    DetourTransactionCommit(); \
} while (false)

#define TH_DETOUR_UNLINK_FROM_MFP(NAME) \
do { \
    DetourTransactionBegin(); \
	DetourUpdateThread(GetCurrentThread()); \
	DetourDetach(std::bit_cast<PVOID&>(&tella::hook::NAME##_class::NAME##_org), std::bit_cast<void*>(&tella::hook::NAME##_class::NAME)); \
	DetourTransactionCommit();  \
} while (false)



#define TH_DECL_DETOUR_SFP(RETURN, NAME, ...) \
namespace tella \
{ \
    namespace hook \
    { \
        using NAME##_fn = RETURN(*)(__VA_ARGS__); \
        static NAME##_fn NAME##_org = nullptr; \
        RETURN NAME(__VA_ARGS__); \
    } \
} \
RETURN tella::hook::NAME(__VA_ARGS__)

#define TH_DETOUR_CALL_SFP(NAME, ...)       NAME##org(__VA_ARGS__)

#define TH_DETOUR_LINK_TO_SFP(NAME, ADDRESS) \
do { \
    tella::hook::NAME##_org = std::bit_cast<tella::hook::NAME##_fn>(ADDRESS);	\
	DetourTransactionBegin(); \
	DetourUpdateThread(GetCurrentThread()); \
	DetourAttach(std::bit_cast<PVOID*>(&tella::hook::NAME##_org), tella::hook::NAME); \
	DetourTransactionCommit();  \
} while (false)

#define TH_DETOUR_UNLINK_FROM_SFP(NAME) \
do { \
    DetourTransactionBegin(); \
	DetourUpdateThread(GetCurrentThread()); \
	DetourDetach(std::bit_cast<PVOID*>(&tella::hook::NAME##_org), tella::hook::NAME); \
	DetourTransactionCommit();  \
} while (false)