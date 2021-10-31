#pragma once
#include "Actions.h"
#include <type_traits>
#include <cstdlib>

template <typename E>
requires std::is_enum_v<E>
constexpr E to_enum(std::underlying_type_t<E> value) {
    return static_cast<E>(value);
}

template <typename E>
requires std::is_enum_v<E>
constexpr std::underlying_type_t<E> from_enum(E value) {
    return static_cast<std::underlying_type_t<E>>(value);
}

enum class Azione {
    Nomenclatura,
    Bilanciamento,
    Riduzione,
    Altro,

    __SIZE__
};