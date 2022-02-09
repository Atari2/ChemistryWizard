#pragma once
#include <array>
#include <cctype>
#include <concepts>
#include <source_location>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include <QMessageBox>

using namespace std::string_view_literals;
using namespace std::string_literals;

static QString last_error{};

#define TODO()                                                                                                         \
    error("La funzione `%s` non e' ancora stata implementata", std::source_location::current().function_name())

int levenshtein_distance(std::string_view s1, std::string_view s2);

template <size_t N, typename... Args>
void error(const char (&fmt)[N], const Args&... args) {
    if constexpr (sizeof...(Args) == 0) {
        last_error = fmt;
    } else {
        last_error = QString::asprintf(fmt, args...);
    }
}

void error_invalid_element(const std::string& elem, char c = '\0');

template <size_t N>
struct StaticStr {
    char m_data_[N];
    constexpr StaticStr(const char* p) {
        for (int i = 0; i < N; ++i) {
            m_data_[i] = p[i];
        }
    }
    constexpr const char* string() const { return m_data_; }
    constexpr size_t size() const { return N - 1; }
    constexpr char index(size_t idx) const { return m_data_[idx]; }
};

template <size_t N>
StaticStr(const char (&)[N]) -> StaticStr<N>;

template <typename E>
struct RefWrap {
    const E& elem;
    const E* operator->() const { return &elem; }
    const E& operator*() const { return elem; }
};

// mock of an interface, since cpp can't do proper interfaces :)
struct BaseElement {
    virtual std::string_view full_name() const = 0;
    virtual std::string_view name() const = 0;
    virtual int na() const = 0;
    virtual double ma() const = 0;
    virtual int operator[](size_t idx) const = 0;
    virtual size_t size_no() const = 0;
};

using ElementRef = RefWrap<BaseElement>;

template <StaticStr tname, StaticStr fullname, int NA, double MA, int... NOs>
class Element final : public BaseElement {
    const std::string_view m_full_name{fullname.string(), fullname.size()};
    const std::string_view m_name{tname.string(), tname.size()};
    const std::array<int, sizeof...(NOs)> m_numeri_ossidazione{NOs...};

    public:
    constexpr std::string_view name() const override { return m_name; }
    constexpr std::string_view full_name() const override { return m_full_name; }
    constexpr int na() const override { return NA; }
    constexpr double ma() const override { return MA; }
    constexpr int operator[](size_t idx) const override {
        if (idx >= m_numeri_ossidazione.size()) {
            error("Elemento %s non ha il numero di ossidazione all'indice %llu", m_name.data(), idx);
            return 0;
        }
        return m_numeri_ossidazione[idx];
    }
    constexpr size_t size_no() const override { return sizeof...(NOs); }
};

struct SingleElementQt {
    ElementRef element;
    size_t quantity;
};

using GroupElementRef = std::vector<SingleElementQt>;

struct GroupElementQt {
    GroupElementRef group;
    size_t quantity;
};

using ElementQt = std::variant<SingleElementQt, GroupElementQt>;

class Composto {
    std::vector<ElementQt> m_elements{};
    size_t m_quantity;

    public:
    Composto(std::vector<ElementQt>&& elements, size_t quantity) :
        m_elements(std::move(elements)), m_quantity(quantity) {}
    auto begin() const { return m_elements.begin(); }
    auto end() const { return m_elements.end(); }
    const ElementQt& operator[](size_t index) const { return m_elements[index]; }
    size_t size() const { return m_elements.size(); }
    size_t quantity() const { return m_quantity; }
    double molecular_mass() const {
        double mass = 0.0;
        for (const auto& elem : m_elements) {
            if (std::holds_alternative<SingleElementQt>(elem)) {
                const auto& single_elem = std::get<SingleElementQt>(elem);
                mass += single_elem.element->ma() * static_cast<double>(single_elem.quantity);
            } else {
                const auto& group_elem = std::get<GroupElementQt>(elem);
                double group_mass = 0.0;
                for (const auto& single_elem : group_elem.group) {
                    group_mass += single_elem.element->ma() * static_cast<double>(single_elem.quantity);
                }
                mass += group_mass * static_cast<double>(group_elem.quantity);
            }
        }
        return mass;
    };
};

struct Reazione {
    std::vector<Composto> reagenti;
    std::vector<Composto> prodotti;
};

#define DECLARE_ELEMENT(name, short_name, na, ma, ...)                                                                 \
    static constexpr inline Element<short_name, #name, na, ma __VA_OPT__(, ) __VA_ARGS__> name {}

// gruppo 1 -> metalli alcalini
DECLARE_ELEMENT(idrogeno, "H", 1, 1.008, -1, +1);
DECLARE_ELEMENT(litio, "Li", 3, 6.941, +1);
DECLARE_ELEMENT(sodio, "Na", 11, 22.99, +1);
DECLARE_ELEMENT(potassio, "K", 19, 39.1, +1);
DECLARE_ELEMENT(rubidio, "Rb", 37, 85.47, +1);
DECLARE_ELEMENT(cesio, "Cs", 55, 132.9, +1);
DECLARE_ELEMENT(francio, "Fr", 87, 223.0, +1);

// gruppo 2 -> metalli alcalino terrosi
DECLARE_ELEMENT(berillio, "Be", 4, 9.012, +2);
DECLARE_ELEMENT(magnesio, "Mg", 12, 24.31, +2);
DECLARE_ELEMENT(calcio, "Ca", 20, 40.08, +2);
DECLARE_ELEMENT(stronzio, "Sr", 38, 87.62, +2);
DECLARE_ELEMENT(bario, "Ba", 56, 137.3, +2);
DECLARE_ELEMENT(radio, "Ra", 88, 226.0, +2);

// gruppo 3-12 -> transizione
DECLARE_ELEMENT(scandio, "Sc", 21, 44.96, +3);
DECLARE_ELEMENT(ittrio, "Y", 39, 88.91, +3);
DECLARE_ELEMENT(lantanio, "La", 57, 138.9, +3);
DECLARE_ELEMENT(attinio, "Ac", 89, 227.0, +3);

DECLARE_ELEMENT(titanio, "Ti", 22, 47.87, +2, +3, +4);
DECLARE_ELEMENT(zirconio, "Zr", 40, 91.22, +4);
DECLARE_ELEMENT(afnio, "Hf", 72, 178.5, +4);
DECLARE_ELEMENT(rutherfordio, "Rf", 104, 261.0, +4);

DECLARE_ELEMENT(vanadio, "V", 23, 50.94, +2, +3, +4, +5);
DECLARE_ELEMENT(niobio, "Nb", 41, 92.91, +3, +5);
DECLARE_ELEMENT(tantalio, "Ta", 73, 180.9, +5);
DECLARE_ELEMENT(dubnio, "Db", 105, 262.0); // non ha numeri di ossidazione

DECLARE_ELEMENT(cromo, "Cr", 24, 52.0, +2, +3, +6);
DECLARE_ELEMENT(molibdeno, "Mo", 42, 95.94, +1, +2, +3, +4, +5, +6);
DECLARE_ELEMENT(tungsteno, "W", 74, 183.8, +2, +3, +4, +5, +6);
DECLARE_ELEMENT(seaborgio, "Sg", 106, 266.0);

DECLARE_ELEMENT(manganese, "Mn", 25, 54.94, +2, +3, +4, +6, +7);
DECLARE_ELEMENT(tecnezio, "Tc", 43, 98.91, +4, +5, +6, +7);
DECLARE_ELEMENT(renio, "Re", 75, 186.2, +4, +6, +7);
DECLARE_ELEMENT(bohrio, "Bh", 107, 264.0);

DECLARE_ELEMENT(ferro, "Fe", 26, 55.85, +2, +3);
DECLARE_ELEMENT(rutenio, "Ru", 44, 101.1, +2, +3, +4, +5, +6, +7);
DECLARE_ELEMENT(osmio, "Os", 76, 190.2, +2, +3, +4, +6, +8);
DECLARE_ELEMENT(hassio, "Hs", 108, 265.0);

DECLARE_ELEMENT(cobalto, "Co", 27, 58.93, +2, +3);
DECLARE_ELEMENT(rodio, "Rh", 45, 102.9, +3);
DECLARE_ELEMENT(iridio, "Ir", 77, 192.2, +3, +4);
DECLARE_ELEMENT(meitnerio, "Mt", 109, 268.0);

DECLARE_ELEMENT(nichel, "Ni", 28, 58.69, +2, +3);
DECLARE_ELEMENT(palladio, "Pd", 46, 106.4, +2, +4);
DECLARE_ELEMENT(platino, "Pt", 78, 195.1, +2, +4);
DECLARE_ELEMENT(darmstadio, "Ds", 110, 271.0);

DECLARE_ELEMENT(rame, "Cu", 29, 63.55, +1, +2);
DECLARE_ELEMENT(argento, "Ag", 47, 107.9, +1);
DECLARE_ELEMENT(oro, "Au", 79, 197.0, +1, +3);
DECLARE_ELEMENT(roentgenio, "Rg", 111, 272.0);

DECLARE_ELEMENT(zinco, "Zn", 30, 65.37, +2);
DECLARE_ELEMENT(cadmio, "Cd", 48, 112.4, +2);
DECLARE_ELEMENT(mercurio, "Hg", 80, 200.6, +1, +2);
DECLARE_ELEMENT(copernicio, "Cn", 112, 285.0);

// gruppo 13 -> semimetalli
DECLARE_ELEMENT(boro, "B", 5, 10.81, +3);
DECLARE_ELEMENT(alluminio, "Al", 13, 26.98, +3);
DECLARE_ELEMENT(gallio, "Ga", 31, 69.72, +3);
DECLARE_ELEMENT(indio, "In", 49, 114.8, +3);
DECLARE_ELEMENT(tallio, "Tl", 81, 204.4, +1, +3);
DECLARE_ELEMENT(nihonio, "Nh", 113, 284.0);

// gruppo 14-15-16 -> non metalli
DECLARE_ELEMENT(carbonio, "C", 6, 12.01, +2, +4, -4);
DECLARE_ELEMENT(silicio, "Si", 14, 28.09, +2, +4, -4);
DECLARE_ELEMENT(germanio, "Ge", 32, 72.64, +2, +4);
DECLARE_ELEMENT(stagno, "Sn", 50, 118.7, +2, +4);
DECLARE_ELEMENT(piombo, "Pb", 82, 207.2, +2, +4);
DECLARE_ELEMENT(flerovio, "Fl", 114, 289.0);

DECLARE_ELEMENT(azoto, "N", 7, 14.01, +2, +3, -3, +4, +5);
DECLARE_ELEMENT(fosforo, "P", 15, 30.97, +3, -3, +5);
DECLARE_ELEMENT(arsenico, "As", 33, 74.92, +3, -3, +5);
DECLARE_ELEMENT(antimonio, "Sb", 51, 121.8, +3, -3, +5);
DECLARE_ELEMENT(bismuto, "Bi", 83, 209.0, +3, +5);
DECLARE_ELEMENT(moscovio, "Mc", 115, 288.0);

DECLARE_ELEMENT(ossigeno, "O", 8, 16.0, -2);
DECLARE_ELEMENT(zolfo, "S", 16, 32.07, -2, +4, +6);
DECLARE_ELEMENT(selenio, "Se", 34, 78.96, -2, +4, +6);
DECLARE_ELEMENT(tellurio, "Te", 52, 127.6, -2, +4, +6);
DECLARE_ELEMENT(polonio, "Po", 84, 209.0, +2, +4, +6);
DECLARE_ELEMENT(livermorio, "Lv", 116, 293.0);

// gruppo 17 -> alogeni
DECLARE_ELEMENT(fluoro, "F", 9, 19.0, -1);
DECLARE_ELEMENT(cloro, "Cl", 17, 35.45, +1, -1, +3, +5, +7);
DECLARE_ELEMENT(bromo, "Br", 35, 79.91, +1, -1, +3, +5);
DECLARE_ELEMENT(iodio, "I", 53, 126.9, +1, -1, +5, +7);
DECLARE_ELEMENT(astato, "At", 85, 210.0, +1, -1, +3, +5, +7);
DECLARE_ELEMENT(tennessinio, "Ts", 117, 310.0);

// gruppo 18 -> gas nobili
DECLARE_ELEMENT(elio, "He", 2, 4.003);
DECLARE_ELEMENT(neon, "Ne", 10, 20.18);
DECLARE_ELEMENT(argon, "Ar", 18, 39.95);
DECLARE_ELEMENT(cripton, "Kr", 36, 83.8);
DECLARE_ELEMENT(xenon, "Xe", 54, 131.3);
DECLARE_ELEMENT(radon, "Rn", 86, 222.0);
DECLARE_ELEMENT(oganessio, "Og", 118, 314.0);

// lantanidi
DECLARE_ELEMENT(cerio, "Ce", 58, 140.1, +3, +4);
DECLARE_ELEMENT(praseodimio, "Pr", 59, 140.9, +3);
DECLARE_ELEMENT(neodimio, "Nd", 60, 144.2, +3);
DECLARE_ELEMENT(promezio, "Pm", 61, 145.0, +3);
DECLARE_ELEMENT(samario, "Sm", 62, 150.4, +2, +3);
DECLARE_ELEMENT(europio, "Eu", 63, 152.0, +2, +3);
DECLARE_ELEMENT(gadolinio, "Gd", 64, 157.3, +3);
DECLARE_ELEMENT(terbio, "Tb", 65, 158.9, +3);
DECLARE_ELEMENT(disprosio, "Dy", 66, 162.5, +3);
DECLARE_ELEMENT(olmio, "Ho", 67, 164.9, +3);
DECLARE_ELEMENT(erbio, "Er", 68, 167.3, +3);
DECLARE_ELEMENT(tulio, "Tm", 69, 168.9, +2, +3);
DECLARE_ELEMENT(itterbio, "Yb", 70, 173.0, +2, +3);
DECLARE_ELEMENT(lutezio, "Lu", 71, 175.0, +3);

// attinidi
DECLARE_ELEMENT(torio, "Th", 90, 232.0, +4);
DECLARE_ELEMENT(protoattinio, "Pa", 91, 231.0, +4, +5);
DECLARE_ELEMENT(uranio, "U", 92, 238.0, +3, +4, +5, +6);
DECLARE_ELEMENT(nettunio, "Np", 93, 237.0, +3, +4, +5, +6);
DECLARE_ELEMENT(plutonio, "Pu", 94, 244.0, +3, +4, +5, +6);
DECLARE_ELEMENT(americio, "Am", 95, 243.0, +3, +4, +5, +6);
DECLARE_ELEMENT(curio, "Cm", 96, 247.0, +3);
DECLARE_ELEMENT(berkelio, "Bk", 97, 247.0, +3, +4);
DECLARE_ELEMENT(californio, "Cf", 98, 251.0, +3);
DECLARE_ELEMENT(einstenio, "Es", 99, 252.0, +3);
DECLARE_ELEMENT(fermio, "Fm", 100, 257.0, +3);
DECLARE_ELEMENT(mendelevio, "Md", 101, 258.0, +2, +3);
DECLARE_ELEMENT(nobelio, "No", 102, 259.0, +2, +3);
DECLARE_ELEMENT(laurenzio, "Lr", 103, 262.0, +3);

static inline std::array<ElementRef, 118> elements{
idrogeno,  litio,       sodio,        potassio,   rubidio,    cesio,     francio,      berillio,   magnesio,
calcio,    stronzio,    bario,        radio,      scandio,    ittrio,    lantanio,     attinio,    titanio,
zirconio,  afnio,       rutherfordio, vanadio,    niobio,     tantalio,  dubnio,       cromo,      molibdeno,
tungsteno, seaborgio,   manganese,    tecnezio,   renio,      bohrio,    ferro,        rutenio,    osmio,
hassio,    cobalto,     rodio,        iridio,     meitnerio,  nichel,    palladio,     platino,    darmstadio,
rame,      argento,     oro,          roentgenio, zinco,      cadmio,    mercurio,     copernicio, boro,
alluminio, gallio,      indio,        tallio,     nihonio,    carbonio,  silicio,      germanio,   stagno,
piombo,    flerovio,    azoto,        fosforo,    arsenico,   antimonio, bismuto,      moscovio,   ossigeno,
zolfo,     selenio,     tellurio,     polonio,    livermorio, fluoro,    cloro,        bromo,      iodio,
astato,    tennessinio, elio,         neon,       argon,      cripton,   xenon,        radon,      oganessio,
cerio,     praseodimio, neodimio,     promezio,   samario,    europio,   gadolinio,    terbio,     disprosio,
olmio,     erbio,       tulio,        itterbio,   lutezio,    torio,     protoattinio, uranio,     nettunio,
plutonio,  americio,    curio,        berkelio,   californio, einstenio, fermio,       mendelevio, nobelio,
laurenzio};

struct case_insensitive_view_compare {
    constexpr bool operator()(const std::string_view& a, const std::string_view& b) const {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); i++) {
            if (std::tolower(a[i]) != std::tolower(b[i])) return false;
        }
        return true;
    }
};

using element_map_tp =
std::unordered_map<std::string_view, ElementRef, std::hash<std::string_view>, case_insensitive_view_compare>;
using tp = element_map_tp::value_type;

static inline element_map_tp elements_map{
tp{"H"sv, idrogeno},      tp{"Li"sv, litio},        tp{"Na"sv, sodio},       tp{"K"sv, potassio},
tp{"Rb"sv, rubidio},      tp{"Cs"sv, cesio},        tp{"Fr"sv, francio},     tp{"Be"sv, berillio},
tp{"Mg"sv, magnesio},     tp{"Ca"sv, calcio},       tp{"Sr"sv, stronzio},    tp{"Ba"sv, bario},
tp{"Ra"sv, radio},        tp{"Sc"sv, scandio},      tp{"Y"sv, ittrio},       tp{"La"sv, lantanio},
tp{"Ac"sv, attinio},      tp{"Ti"sv, titanio},      tp{"Zr"sv, zirconio},    tp{"Hf"sv, afnio},
tp{"Rf"sv, rutherfordio}, tp{"V"sv, vanadio},       tp{"Nb"sv, niobio},      tp{"Ta"sv, tantalio},
tp{"Db"sv, dubnio},       tp{"Cr"sv, cromo},        tp{"Mo"sv, molibdeno},   tp{"W"sv, tungsteno},
tp{"Sg"sv, seaborgio},    tp{"Mn"sv, manganese},    tp{"Tc"sv, tecnezio},    tp{"Re"sv, renio},
tp{"Bh"sv, bohrio},       tp{"Fe"sv, ferro},        tp{"Ru"sv, rutenio},     tp{"Os"sv, osmio},
tp{"Hs"sv, hassio},       tp{"Co"sv, cobalto},      tp{"Rh"sv, rodio},       tp{"Ir"sv, iridio},
tp{"Mt"sv, meitnerio},    tp{"Ni"sv, nichel},       tp{"Pd"sv, palladio},    tp{"Pt"sv, platino},
tp{"Ds"sv, darmstadio},   tp{"Cu"sv, rame},         tp{"Ag"sv, argento},     tp{"Au"sv, oro},
tp{"Rg"sv, roentgenio},   tp{"Zn"sv, zinco},        tp{"Cd"sv, cadmio},      tp{"Hg"sv, mercurio},
tp{"Cn"sv, copernicio},   tp{"B"sv, boro},          tp{"Al"sv, alluminio},   tp{"Ga"sv, gallio},
tp{"In"sv, indio},        tp{"Tl"sv, tallio},       tp{"Nh"sv, nihonio},     tp{"C"sv, carbonio},
tp{"Si"sv, silicio},      tp{"Ge"sv, germanio},     tp{"Sn"sv, stagno},      tp{"Pb"sv, piombo},
tp{"Fl"sv, flerovio},     tp{"N"sv, azoto},         tp{"P"sv, fosforo},      tp{"As"sv, arsenico},
tp{"Sb"sv, antimonio},    tp{"Bi"sv, bismuto},      tp{"Mc"sv, moscovio},    tp{"O"sv, ossigeno},
tp{"S"sv, zolfo},         tp{"Se"sv, selenio},      tp{"Te"sv, tellurio},    tp{"Po"sv, polonio},
tp{"Lv"sv, livermorio},   tp{"F"sv, fluoro},        tp{"Cl"sv, cloro},       tp{"Br"sv, bromo},
tp{"I"sv, iodio},         tp{"At"sv, astato},       tp{"Ts"sv, tennessinio}, tp{"He"sv, elio},
tp{"Ne"sv, neon},         tp{"Ar"sv, argon},        tp{"Kr"sv, cripton},     tp{"Xe"sv, xenon},
tp{"Rn"sv, radon},        tp{"Og"sv, oganessio},    tp{"Ce"sv, cerio},       tp{"Pr"sv, praseodimio},
tp{"Nd"sv, neodimio},     tp{"Pm"sv, promezio},     tp{"Sm"sv, samario},     tp{"Eu"sv, europio},
tp{"Gd"sv, gadolinio},    tp{"Tb"sv, terbio},       tp{"Dy"sv, disprosio},   tp{"Ho"sv, olmio},
tp{"Er"sv, erbio},        tp{"Tm"sv, tulio},        tp{"Yb"sv, itterbio},    tp{"Lu"sv, lutezio},
tp{"Th"sv, torio},        tp{"Pa"sv, protoattinio}, tp{"U"sv, uranio},       tp{"Np"sv, nettunio},
tp{"Pu"sv, plutonio},     tp{"Am"sv, americio},     tp{"Cm"sv, curio},       tp{"Bk"sv, berkelio},
tp{"Cf"sv, californio},   tp{"Es"sv, einstenio},    tp{"Fm"sv, fermio},      tp{"Md"sv, mendelevio},
tp{"No"sv, nobelio},      tp{"Lr"sv, laurenzio},
};

QString do_balance(const std::string& argument);
QString do_naming(const std::string& argument);
QString do_reduction(const std::string& argument);
QString do_other(const std::string& argument);