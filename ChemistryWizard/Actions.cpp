#include "Actions.h"

#include <algorithm>
#include <concepts>
#include <cstring>
#include <iostream>
#include <optional>
#include <ranges>

#include <QDebug>
#include <QTextStream>

template <typename T>
concept QTStream = std::same_as<T, QDebug> || std::same_as<T, QTextStream>;

template <typename T>
concept OStream = std::same_as<T, std::ostream> || QTStream<T>;

decltype(auto) operator<<(QTStream auto& dbg, const std::string& str) {
    if constexpr (std::same_as<decltype(dbg), QDebug>) {
        QDebugStateSaver save(dbg);
        dbg.nospace() << QString::fromStdString(str);
    } else {
        dbg << QString::fromStdString(str);
    }
    return dbg;
}

decltype(auto) operator<<(QTStream auto& dbg, const std::wstring& str) {
    if constexpr (std::same_as<decltype(dbg), QDebug>) {
        QDebugStateSaver save(dbg);
        dbg.nospace() << QString::fromStdWString(str);
    } else {
        dbg << QString::fromStdWString(str);
    }
    return dbg;
}

decltype(auto) operator<<(QTStream auto& dbg, const std::string_view& str) {
    if constexpr (std::same_as<decltype(dbg), QDebug>) {
        QDebugStateSaver save(dbg);
        dbg.nospace() << QString::fromLocal8Bit(str.data(), str.size());
    } else {
        dbg << QString::fromLocal8Bit(str.data(), str.size());
    }
    return dbg;
}

template <OStream StreamT,
          typename StringT = std::conditional_t<std::same_as<StreamT, std::wostream>, std::wstring, std::string>>
void print_single_element(StreamT& os, const SingleElementQt& elem, size_t n_tabs) {
    StringT tabs(n_tabs, '\t');
    const auto& [belem, sz] = elem;
    os << tabs << belem->full_name() << ' ' << belem->name() << ' ' << belem->na() << ' ' << belem->ma();
    if (belem->size_no() > 0) {
        os << ' ';
        for (size_t i = 0; i < belem->size_no(); i++) {
            os << (*belem)[i] << ' ';
        }
    }
    os << ", quantità: " << sz << '\n';
}

template <OStream StreamT>
decltype(auto) operator<<(StreamT& os, const SingleElementQt& elem) {
    if constexpr (std::same_as<StreamT, QDebug>) { QDebugStateSaver saver(os); }
    print_single_element(os, elem, 0);
    return os;
}

template <OStream StreamT>
decltype(auto) operator<<(StreamT& os, const GroupElementQt& group) {
    if constexpr (std::same_as<StreamT, QDebug>) { QDebugStateSaver saver(os); }
    const auto& [group, size] = group;
    os << "\tGruppo (quantità " << size << "): \n";
    for (const auto& elem : group) {
        print_single_element(os, elem, 1);
    }
    os << "\tQuantità del gruppo: " << size << '\n';
}

template <OStream StreamT>
decltype(auto) operator<<(StreamT& os, const Composto& compo) {
    if constexpr (std::same_as<StreamT, QDebug>) { QDebugStateSaver saver(os); }
    os << "Quantità di questo composto: " << compo.quantity() << '\n';
    for (const auto& variant : compo) {
        if (std::holds_alternative<SingleElementQt>(variant)) {
            print_single_element(os, std::get<SingleElementQt>(variant), 1);
        } else if (std::holds_alternative<GroupElementQt>(variant)) {
            const auto& group_element = std::get<GroupElementQt>(variant);
            const auto& [group, size] = group_element;
            os << "\tGruppo (quantità " << size << "): \n";
            for (const auto& elem : group) {
                print_single_element(os, elem, 2);
            }
            os << "\tQuantità del gruppo: " << size << '\n';
        } else {
            error("Variant is empty");
        }
    }
    os << "\tMassa molecolare del composto: " << compo.molecular_mass();
    return os;
}

inline auto print = [](const auto& obj) {
    qDebug() << obj << '\n';
};

inline auto print_text = [](const auto& obj) -> QString {
    QString text{};
    QTextStream stream{&text};
    stream << obj << '\n';
    return text;
};

inline auto range_to_view_strip = [](std::ranges::range auto&& range) {
    // this (unnecessarily) convoluted function effectively does std::string_view view{range} -> return strip(view)
    // where strip() removes all whitespace at the start and end of the range

    size_t skipped = 0;
    size_t range_size = std::ranges::distance(range);
    auto it = std::ranges::begin(range);
    auto end = std::ranges::end(range);
    while (it != end && std::isspace(*it)) {
        ++it;
        skipped++;
    }
    range_size -= skipped;
    if (it == end || range_size == 0) return std::string_view{};
    const auto* begin_ptr = &(*it);

    // this is because the ranges iterator is only a forward iterator
    std::string_view v{begin_ptr, range_size};
    auto last_char = v.end() - 1;
    while (std::isspace(*last_char)) {
        --last_char;
        range_size--;
    }
    return std::string_view{begin_ptr, range_size};
};

enum class UpperState { Unknown, False, True };

inline auto parse_group = [](std::string_view::const_iterator& it,
                             std::string_view::const_iterator end) -> std::optional<GroupElementRef> {
    std::vector<SingleElementQt> elements{};
    std::string curr_elem{};
    std::string curr_elem_size{};
    bool previous_is_num = false;
    auto previous_is_upper = UpperState::Unknown;
    auto upper = UpperState::Unknown;
    auto map_end = elements_map.end();
    auto check_upper = [&]() {
        return (upper == UpperState::True && previous_is_upper == UpperState::False) ||
               (upper == UpperState::True && previous_is_upper == UpperState::True);
    };
    for (; it != end; ++it) {
        auto c = *it;
        bool num = std::isdigit(c) != 0;
        upper = std::isupper(c) != 0 ? UpperState::True : UpperState::False;
        if ((!num && previous_is_num) || check_upper() || c == ')') {
            if (curr_elem.empty()) {
                error_invalid_element(curr_elem, c);
                return {};
            }
            auto it = elements_map.find(curr_elem);
            if (it == map_end) {
                error_invalid_element(curr_elem);
                return {};
            }
            size_t sz = curr_elem_size.empty() ? 1 : std::stoull(curr_elem_size);
            elements.emplace_back((*it).second, sz);
            curr_elem_size.clear();
            curr_elem.clear();
            if (c == ')') break;
            curr_elem += c;
        } else if (num) {
            curr_elem_size += c;
        } else if (c == ')') {
            ++it;
            break;
        } else {
            curr_elem += c;
            if (curr_elem.size() > 2) {
                error_invalid_element(curr_elem);
                return {};
            }
        }
        previous_is_num = num;
        previous_is_upper = upper;
    }
    if (!curr_elem.empty()) {
        error_invalid_element(curr_elem);
        return {};
    }
    return elements;
};

inline auto split_molecule_in_elements = [](std::string_view view) -> std::optional<Composto> {
    std::vector<ElementQt> elements{};
    std::string curr_elem{};
    std::string curr_elem_size{};
    std::string quantity_str{};
    bool previous_is_num = false;
    auto previous_is_upper = UpperState::Unknown;
    auto upper = UpperState::Unknown;
    auto map_end = elements_map.end();
    auto check_upper = [&]() {
        return (upper == UpperState::True && previous_is_upper == UpperState::False) ||
               (upper == UpperState::True && previous_is_upper == UpperState::True);
    };
    GroupElementRef group{};
    bool previous_was_group = false;

    auto append_elem_or_group = [&](char c) -> bool {
        size_t sz = curr_elem_size.empty() ? 1 : std::stoull(curr_elem_size);
        if (previous_was_group) {
            elements.push_back(ElementQt{std::in_place_index<1>, move(group), sz});
            previous_was_group = false;
        } else {
            if (curr_elem.empty()) {
                error_invalid_element(curr_elem, c);
                return false;
            }
            auto it = elements_map.find(curr_elem);
            if (it == map_end) {
                error_invalid_element(curr_elem);
                return false;
            }
            elements.push_back(ElementQt{std::in_place_index<0>, (*it).second, sz});
            curr_elem_size.clear();
            curr_elem.clear();
            curr_elem += c;
        }
        return true;
    };

    for (auto it = view.begin(); it != view.end(); ++it) {
        auto c = *it;
        bool num = std::isdigit(c) != 0;
        if (num && upper == UpperState::Unknown) {
            // numero prima della molecola, quindi la quantità.
            quantity_str += c;
            continue;
        }
        upper = std::isupper(c) != 0 ? UpperState::True : UpperState::False;
        if ((!num && previous_is_num) || check_upper()) {
            if (!append_elem_or_group(c)) { return {}; }
        } else if (num) {
            curr_elem_size += c;
        } else if (c == '(') {
            if (!append_elem_or_group(c)) { return {}; }
            previous_was_group = true;
            auto maybe_group = parse_group(++it, view.end());
            if (!maybe_group.has_value()) { return {}; }
            group = std::move(maybe_group.value());
        } else {
            curr_elem += c;
            if (curr_elem.size() > 2) {
                error_invalid_element(curr_elem);
                return {};
            }
        }
        previous_is_num = num;
        previous_is_upper = upper;
    }
    size_t sz = curr_elem_size.empty() ? 1 : std::stoull(curr_elem_size);
    if (previous_was_group) {
        elements.emplace_back(ElementQt{std::in_place_index<1>, std::move(group), sz});
    } else {
        if (curr_elem.empty()) {
            error_invalid_element(curr_elem);
            return {};
        }
        auto it = elements_map.find(curr_elem);
        if (it == map_end) {
            error_invalid_element(curr_elem);
            return {};
        }
        elements.emplace_back(ElementQt{std::in_place_index<0>, (*it).second, sz});
    }
    size_t quantity = quantity_str.empty() ? 1 : std::stoull(quantity_str);
    return Composto{std::move(elements), quantity};
};

int levenshtein_distance(std::string_view s1, std::string_view s2) {
    size_t m = s1.size();
    size_t n = s2.size();

    int* prev = new int[(n + 1) * sizeof(int)];
    int* curr = new int[(n + 1) * sizeof(int)];

    for (size_t i = 0; i <= n; i++)
        prev[i] = static_cast<int>(i);

    for (size_t i = 1; i <= m; i++) {
        curr[0] = static_cast<int>(i);
        for (size_t j = 1; j <= n; j++) {
            if (std::tolower(s1[i - 1]) != std::tolower(s2[j - 1])) {
                int k = std::min({curr[j - 1], prev[j - 1], prev[j]});
                curr[j] = k + 1;
            } else {
                curr[j] = prev[j - 1];
            }
        }
        std::swap(prev, curr);
        std::memset(curr, 0, sizeof(int) * (n + 1));
    }
    int distance = prev[n];
    delete[] curr;
    delete[] prev;
    return distance;
}

void error_invalid_element(const std::string& element, char c) {
    if (element.empty()) {
        if (c == '\0')
            error("Manca l'ultimo elemento");
        else
            error("Elemento non valido: %s, arrivato a %c", element.c_str(), c);
    } else {
        std::array<int, elements.size()> distances{};
        for (size_t i = 0; auto& elem : elements) {
            distances[i++] = levenshtein_distance(elem->name(), element);
        }
        auto min_it = std::min_element(distances.begin(), distances.end());
        auto idx = std::distance(distances.begin(), min_it);
        error("Elemento non valido %s, forse intendevi %s (%s)?", element.c_str(), elements[idx]->name().data(),
              elements[idx]->full_name().data());
    }
}

QString do_balance(const std::string& argument) {
    auto view = argument | std::views::split("->"sv) | std::views::transform(range_to_view_strip) | std::views::common;
    if (std::ranges::distance(view) != 2) {
        error("Formato della reazione non valido");
        return last_error;
    }

    bool had_error = false;
    Reazione r{};
    auto vit = view.begin();
    std::ranges::copy(*vit | std::views::split('+') | std::views::transform(range_to_view_strip) |
                      std::views::transform([&](std::string_view view) -> Composto {
                          auto maybe_composto = split_molecule_in_elements(view);
                          if (!maybe_composto.has_value()) {
                              had_error = true;
                              return Composto{{}, 0};
                          }
                          return maybe_composto.value();
                      }),
                      std::back_inserter(r.reagenti));
    if (had_error) return last_error;
    ++vit;
    std::ranges::copy(*vit | std::views::split('+') | std::views::transform(range_to_view_strip) |
                      std::views::transform([&](std::string_view view) -> Composto {
                          auto maybe_composto = split_molecule_in_elements(view);
                          if (!maybe_composto.has_value()) {
                              had_error = true;
                              return Composto{{}, 0};
                          }
                          return maybe_composto.value();
                      }),
                      std::back_inserter(r.prodotti));
    if (had_error) return last_error;

    QString result{"Reagenti:\n"};
    for (const auto& reagente : r.reagenti) {
        result.append(print_text(reagente));
    }
    result.append("Prodotti:\n");
    for (const auto& prodotto : r.prodotti) {
        result.append(print_text(prodotto));
    }

    return result;
}
QString do_naming(const std::string& argument) {
    std::string_view formula = argument;
    TODO();
    return last_error;
}
QString do_reduction(const std::string& argument) {
    TODO();
    return last_error;
}
QString do_other(const std::string& argument) {
    TODO();
    return last_error;
}