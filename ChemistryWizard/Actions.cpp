#include "Actions.h"

#include <algorithm>
#include <iostream>
#include <ranges>

std::ostream& operator<<(std::ostream& os, const Composto& compo) {
    os << "Quantità di questo composto: " << compo.quantity() << '\n';
    for (const auto& [belem_ref, sz] : compo) {
        const auto& belem = *belem_ref;
        os << belem.full_name() << ' ' << belem.name() << ' ' << belem.no() << ' ' << belem.nm();
        if (belem.size_no() > 0) {
            os << ' ';
            for (size_t i = 0; i < belem.size_no(); i++) {
                os << belem[i] << ' ';
            }
        }
        os << ", quantità: " << sz;
        os << '\n';
    }
    return os;
}

std::wostream& operator<<(std::wostream& os, const Composto& compo) {
    os << "Quantità di questo composto: " << compo.quantity() << '\n';
    for (const auto& [belem_ref, sz] : compo) {
        const auto& belem = *belem_ref;
        os << belem.wfull_name() << L' ' << belem.wname() << L' ' << belem.no() << L' ' << belem.nm();
        if (belem.size_no() > 0) {
            os << L' ';
            for (size_t i = 0; i < belem.size_no(); i++) {
                os << belem[i] << L' ';
            }
        }
        os << L", quantità: " << sz;
        os << L'\n';
    }
    return os;
}

static void populate_arguments(std::vector<std::string>& arguments, int necessary_arguments, std::string_view prompt) {
    std::cout << prompt;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    fflush(stdin);
    std::cin.clear();
    while (necessary_arguments > 0) {
        std::string line{};
        std::getline(std::cin, line);
        arguments.push_back(line);
        --necessary_arguments;
    }
}

inline auto print = [](const auto& obj) {
    std::cout << obj << '\n';
};

inline auto wprint = [](const auto& obj) {
    std::wcout << obj << L'\n';
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

inline auto split_molecule_in_elements = [](std::string_view view) {
    enum class UpperState { Unknown, False, True };
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
    for (auto c : view) {
        bool num = std::isdigit(c) != 0;
        if (num && upper == UpperState::Unknown) {
            // numero prima della molecola, quindi la quantità.
            quantity_str += c;
            continue;
        }
        upper = std::isupper(c) != 0 ? UpperState::True : UpperState::False;
        if ((!num && previous_is_num) || check_upper()) {
            if (curr_elem.empty()) error_invalid_element(curr_elem, c);
            auto it = elements_map.find(curr_elem);
            if (it == map_end) error_invalid_element(curr_elem);
            size_t sz = curr_elem_size.empty() ? 1 : std::stoull(curr_elem_size);
            elements.emplace_back((*it).second, sz);
            curr_elem_size.clear();
            curr_elem.clear();
            curr_elem += c;
        } else if (num) {
            curr_elem_size += c;
        } else {
            curr_elem += c;
            if (curr_elem.size() > 2) error_invalid_element(curr_elem);
        }
        previous_is_num = num;
        previous_is_upper = upper;
    }
    if (curr_elem.empty()) error_invalid_element(curr_elem);
    auto it = elements_map.find(curr_elem);
    if (it == map_end) error_invalid_element(curr_elem);
    size_t sz = curr_elem_size.empty() ? 1 : std::stoull(curr_elem_size);
    size_t quantity = quantity_str.empty() ? 1 : std::stoull(quantity_str);
    elements.emplace_back((*it).second, sz);
    return Composto{std::move(elements), quantity};
};

int levenshtein_distance(std::string_view s1, std::string_view s2) {
    size_t m = s1.size();
    size_t n = s2.size();

    int* prev = new int[(n + 1) * sizeof(int)];
    int* curr = new int[(n + 1) * sizeof(int)];

    for (size_t i = 0; i <= n; i++)
        prev[i] = i;
    
    for (size_t i = 1; i <= m; i++) {
        curr[0] = i;
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

[[noreturn]] void error_invalid_element(const std::string& element, char c) {
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
        error("Elemento non valido %s, forse intendevi %s (%s)?", element.c_str(), elements[idx]->name().data(), elements[idx]->full_name().data());
    }

}

void do_balance(std::vector<std::string>& arguments) {
    if (arguments.empty())
        populate_arguments(
        arguments, 1, "Inserisci <reagenti> -> <prodotti> (reagenti e prodotti devono essere divisi da un +):\n"sv);
    auto view =
    arguments[0] | std::views::split("->"sv) | std::views::transform(range_to_view_strip) | std::views::common;
    if (std::ranges::distance(view) != 2) { error("Formato della reazione non valido"); }

    std::vector<Composto> reagenti{};
    std::vector<Composto> prodotti{};

    auto vit = view.begin();
    std::ranges::copy(*vit | std::views::split('+') | std::views::transform(range_to_view_strip) |
                      std::views::transform(split_molecule_in_elements),
                      std::back_inserter(reagenti));
    ++vit;
    std::ranges::copy(*vit | std::views::split('+') | std::views::transform(range_to_view_strip) |
                      std::views::transform(split_molecule_in_elements),
                      std::back_inserter(prodotti));
    std::ranges::for_each(reagenti, print);
    std::ranges::for_each(prodotti, print);
    TODO();
}
void do_naming(std::vector<std::string>& arguments) {
    if (arguments.empty()) populate_arguments(arguments, 1, "Inserisci la formula della molecola da nominare:\n"sv);
    std::string_view formula = arguments[0];
    TODO();
}
void do_reduction(std::vector<std::string>& arguments) {
    if (arguments.empty()) populate_arguments(arguments, 1, "Inserisci la formula della molecola da ridurre:\n"sv);
    TODO();
}
void do_other(std::vector<std::string>& arguments) {
    if (arguments.empty()) populate_arguments(arguments, 1, "Inserisci...\n"sv);
    TODO();
}