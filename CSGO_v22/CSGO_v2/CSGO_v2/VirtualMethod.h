#pragma once

#include <cstddef>
#include <print>
#include <type_traits>

namespace VirtualMethod
{
    template <typename T, std::size_t Idx, typename ...Args>
    constexpr T call(void* classBase, Args... args) noexcept
    {
        if (!classBase) {
            printf("Error: classBase is null\n");
            if constexpr (std::is_reference_v<T>) {
                static typename std::remove_reference<T>::type defaultObj{};
                return defaultObj;
            } else {
                return T{};
            }
        }

        auto vtable = *reinterpret_cast<void***>(classBase);
        if (!vtable) {
            printf("Error: vtable is null\n");
            if constexpr (std::is_reference_v<T>) {
                static typename std::remove_reference<T>::type defaultObj{};
                return defaultObj;
            } else {
                return T{};
            }
        }

        auto funcPtr = vtable[Idx];
        if (!funcPtr) {
            printf("Error: Function pointer at index %zu is null\n", Idx);
            if constexpr (std::is_reference_v<T>) {
                static typename std::remove_reference<T>::type defaultObj{};
                return defaultObj;
            } else {
                return T{};
            }
        }

        return ((*reinterpret_cast<T(__thiscall***)(void*, Args...)>(classBase))[Idx])(classBase, args...);
    }
}

#define VIRTUAL_METHOD(returnType, name, idx, args, argsRaw) \
returnType name args noexcept \
{ \
    return VirtualMethod::call<returnType, idx>argsRaw; \
}

