#pragma once

#include "policy.hpp"

namespace tella
{
	namespace hook
	{
		class vtable
		{
		public:
			using thisptr_t = void*;
			using vtidx_t = uint32_t;

			vtable(thisptr_t thisptr, vtidx_t index) noexcept { VTPtr = std::bit_cast<void**>(*std::bit_cast<uintptr_t*>(thisptr) + index); }

			auto operator<=>(const vtable&) const noexcept = default;

			operator storage::key_type() const noexcept { return std::bit_cast<storage::key_type>(VTPtr); }

			void* operator*() const noexcept { return *VTPtr; }

		private:
			void** VTPtr;
		};
	}
}
