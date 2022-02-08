#pragma once
#include "Actions.h"
#include <type_traits>
#include <cstdlib>
#include <QString>

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

using callback_t = QString (*)(const std::string&);

struct NamedCallback {
    std::string name;
    callback_t callback;
};

static inline NamedCallback callbacks[from_enum(Azione::__SIZE__)] = {{"Nomenclatura"s, &do_naming},
                                                                      {"Bilanciamento"s, &do_balance},
                                                                      {"Riduzione"s, &do_reduction},
                                                                      {"Altro..."s, &do_other}};