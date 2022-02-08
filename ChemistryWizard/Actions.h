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

template <char... Chars>
struct tstring {
    const char p[sizeof...(Chars) + 1] = {Chars..., '\0'};
    const wchar_t wp[sizeof...(Chars) + 1] = {Chars..., L'\0'};
    const size_t N = sizeof...(Chars);
    consteval const char* const_string() const { return p; }
    consteval const wchar_t* const_wstring() const { return wp; }
    consteval size_t const_size() const { return N; }
};

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
    virtual std::wstring_view wfull_name() const = 0;
    virtual std::wstring_view wname() const = 0;
    virtual int na() const = 0;
    virtual double ma() const = 0;
    virtual int operator[](size_t idx) const = 0;
    virtual size_t size_no() const = 0;
};

using ElementRef = RefWrap<BaseElement>;

template <tstring tname, int... NOs>
class Element : public BaseElement {
    const std::string_view m_full_name;
    const std::string_view m_name{tname.const_string(), tname.const_size()};
    const std::wstring_view m_wfull_name;
    const std::wstring_view m_wname{tname.const_wstring(), tname.const_size()};
    const int m_numero_atomico;
    const double m_massa_atomica;
    const std::array<int, sizeof...(NOs)> m_numeri_ossidazione{NOs...};

    public:
    constexpr Element(int na, double ma, std::string_view full_name, std::wstring_view wfull_name) :
        m_numero_atomico{na}, m_massa_atomica{ma}, m_full_name{full_name}, m_wfull_name{wfull_name} {}

    constexpr std::string_view name() const override { return m_name; }
    constexpr std::string_view full_name() const override { return m_full_name; }
    constexpr std::wstring_view wname() const override { return m_wname; }
    constexpr std::wstring_view wfull_name() const override { return m_wfull_name; }
    constexpr int na() const override { return m_numero_atomico; }
    constexpr double ma() const override { return m_massa_atomica; }
    constexpr int operator[](size_t idx) const override {
        if (idx >= m_numeri_ossidazione.size()) {
            error("Elemento %s non ha il numero di ossidazione all'indice %llu", m_name.data(), idx);
            return 0;
        }
        return m_numeri_ossidazione[idx];
    }
    constexpr size_t size_no() const override { return m_numeri_ossidazione.size(); }
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

// gruppo 1 -> metalli alcalini
static constexpr inline Element<tstring<'H'>{}, -1, +1> idrogeno{1, 1.008, "idrogeno"sv, L"idrogeno"sv};
static constexpr inline Element<tstring<'L', 'i'>{}, +1> litio{3, 6.941, "litio"sv, L"litio"sv};
static constexpr inline Element<tstring<'N', 'a'>{}, +1> sodio{11, 22.99, "sodio"sv, L"sodio"sv};
static constexpr inline Element<tstring<'K'>{}, +1> potassio{19, 39.1, "potassio"sv, L"potassio"sv};
static constexpr inline Element<tstring<'R', 'b'>{}, +1> rubidio{37, 85.47, "rubidio"sv, L"rubidio"sv};
static constexpr inline Element<tstring<'C', 's'>{}, +1> cesio{55, 132.9, "cesio"sv, L"cesio"sv};
static constexpr inline Element<tstring<'F', 'r'>{}, +1> francio{87, 223.0, "francio"sv, L"francio"sv};

// gruppo 2 -> metalli alcalino terrosi
static constexpr inline Element<tstring<'B', 'e'>{}, +2> berillio{4, 9.012, "berillio"sv, L"berillio"sv};
static constexpr inline Element<tstring<'M', 'g'>{}, +2> magnesio{12, 24.31, "magnesio"sv, L"magnesio"sv};
static constexpr inline Element<tstring<'C', 'a'>{}, +2> calcio{20, 40.08, "calcio"sv, L"calcio"sv};
static constexpr inline Element<tstring<'S', 'r'>{}, +2> stronzio{38, 87.62, "stronzio"sv, L"stronzio"sv};
static constexpr inline Element<tstring<'B', 'a'>{}, +2> bario{56, 137.3, "bario"sv, L"bario"sv};
static constexpr inline Element<tstring<'R', 'a'>{}, +2> radio{88, 226.0, "radio"sv, L"radio"sv};

// gruppo 3-12 -> transizione
static constexpr inline Element<tstring<'S', 'c'>{}, +3> scandio{21, 44.96, "scandio"sv, L"scandio"sv};
static constexpr inline Element<tstring<'Y'>{}, +3> ittrio{39, 88.91, "ittrio"sv, L"ittrio"sv};
static constexpr inline Element<tstring<'L', 'a'>{}, +3> lantanio{57, 138.9, "lantanio"sv, L"lantanio"sv};
static constexpr inline Element<tstring<'A', 'c'>{}, +3> attinio{89, 227.0, "attinio"sv, L"attinio"sv};

static constexpr inline Element<tstring<'T', 'i'>{}, +2, +3, +4> titanio{22, 47.87, "titanio"sv, L"titanio"sv};
static constexpr inline Element<tstring<'Z', 'r'>{}, +4> zirconio{40, 91.22, "zirconio"sv, L"zirconio"sv};
static constexpr inline Element<tstring<'H', 'f'>{}, +4> afnio{72, 178.5, "afnio"sv, L"afnio"sv};
static constexpr inline Element<tstring<'R', 'f'>{}, +4> rutherfordio{104, 261.0, "rutherfordio"sv, L"rutherfordio"sv};

static constexpr inline Element<tstring<'V'>{}, +2, +3, +4, +5> vanadio{23, 50.94, "vanadio"sv, L"vanadio"sv};
static constexpr inline Element<tstring<'N', 'b'>{}, +3, +5> niobio{41, 92.91, "niobio"sv, L"niobio"sv};
static constexpr inline Element<tstring<'T', 'a'>{}, +5> tantalio{73, 180.9, "tantalio"sv, L"tantalio"sv};
static constexpr inline Element<tstring<'D', 'b'>{}> dubnio{105, 262.0, "dubnio"sv,
                                                            L"dubnio"sv}; // non ha numeri di ossidazione

static constexpr inline Element<tstring<'C', 'r'>{}, +2, +3, +6> cromo{24, 52.0, "cromo"sv, L"cromo"sv};
static constexpr inline Element<tstring<'M', 'o'>{}, +1, +2, +3, +4, +5, +6> molibdeno{42, 95.94, "molibdeno"sv,
                                                                                       L"molibdeno"sv};
static constexpr inline Element<tstring<'W'>{}, +2, +3, +4, +5, +6> tungsteno{74, 183.8, "tungsteno"sv, L"tungsteno"sv};
static constexpr inline Element<tstring<'S', 'g'>{}> seaborgio{106, 266.0, "seaborgio"sv, L"seaborgio"sv};

static constexpr inline Element<tstring<'M', 'n'>{}, +2, +3, +4, +6, +7> manganese{25, 54.94, "manganese"sv,
                                                                                   L"manganese"sv};
static constexpr inline Element<tstring<'T', 'c'>{}, +4, +5, +6, +7> tecnezio{43, 98.91, "tecnezio"sv, L"tecnezio"sv};
static constexpr inline Element<tstring<'R', 'e'>{}, +4, +6, +7> renio{75, 186.2, "renio"sv, L"renio"sv};
static constexpr inline Element<tstring<'B', 'h'>{}> bohrio{107, 264.0, "bohrio"sv, L"bohrio"sv};

static constexpr inline Element<tstring<'F', 'e'>{}, +2, +3> ferro{26, 55.85, "ferro"sv, L"ferro"sv};
static constexpr inline Element<tstring<'R', 'u'>{}, +2, +3, +4, +5, +6, +7> rutenio{44, 101.1, "rutenio"sv,
                                                                                     L"rutenio"sv};
static constexpr inline Element<tstring<'O', 's'>{}, +2, +3, +4, +6, +8> osmio{76, 190.2, "osmio"sv, L"osmio"sv};
static constexpr inline Element<tstring<'H', 's'>{}> hassio{108, 265.0, "hassio"sv, L"hassio"sv};

static constexpr inline Element<tstring<'C', 'o'>{}, +2, +3> cobalto{27, 58.93, "cobalto"sv, L"cobalto"sv};
static constexpr inline Element<tstring<'R', 'h'>{}, +3> rodio{45, 102.9, "rodio"sv, L"rodio"sv};
static constexpr inline Element<tstring<'I', 'r'>{}, +3, +4> iridio{77, 192.2, "iridio"sv, L"iridio"sv};
static constexpr inline Element<tstring<'M', 't'>{}> meitnerio{109, 268.0, "meitnerio"sv, L"meitnerio"sv};

static constexpr inline Element<tstring<'N', 'i'>{}, +2, +3> nichel{28, 58.69, "nichel"sv, L"nichel"sv};
static constexpr inline Element<tstring<'P', 'd'>{}, +2, +4> palladio{46, 106.4, "palladio"sv, L"palladio"sv};
static constexpr inline Element<tstring<'P', 't'>{}, +2, +4> platino{78, 195.1, "platino"sv, L"platino"sv};
static constexpr inline Element<tstring<'D', 's'>{}> darmstadio{110, 271.0, "darmstadio"sv, L"darmstadio"sv};

static constexpr inline Element<tstring<'C', 'u'>{}, +1, +2> rame{29, 63.55, "rame"sv, L"rame"sv};
static constexpr inline Element<tstring<'A', 'g'>{}, +1> argento{47, 107.9, "argento"sv, L"argento"sv};
static constexpr inline Element<tstring<'A', 'u'>{}, +1, +3> oro{79, 197.0, "oro"sv, L"oro"sv};
static constexpr inline Element<tstring<'R', 'g'>{}> roentgenio{111, 272.0, "roentgenio"sv, L"roentgenio"sv};

static constexpr inline Element<tstring<'Z', 'n'>{}, +2> zinco{30, 65.37, "zinco"sv, L"zinco"sv};
static constexpr inline Element<tstring<'C', 'd'>{}, +2> cadmio{48, 112.4, "cadmio"sv, L"cadmio"sv};
static constexpr inline Element<tstring<'H', 'g'>{}, +1, +2> mercurio{80, 200.6, "mercurio"sv, L"mercurio"sv};
static constexpr inline Element<tstring<'C', 'n'>{}> copernicio{112, 285.0, "copernicio"sv, L"copernicio"sv};

// gruppo 13 -> semimetalli
static constexpr inline Element<tstring<'B'>{}, +3> boro{5, 10.81, "boro"sv, L"boro"sv};
static constexpr inline Element<tstring<'A', 'l'>{}, +3> alluminio{13, 26.98, "alluminio"sv, L"alluminio"sv};
static constexpr inline Element<tstring<'G', 'a'>{}, +3> gallio{31, 69.72, "gallio"sv, L"gallio"sv};
static constexpr inline Element<tstring<'I', 'n'>{}, +3> indio{49, 114.8, "indio"sv, L"indio"sv};
static constexpr inline Element<tstring<'T', 'l'>{}, +1, +3> tallio{81, 204.4, "tallio"sv, L"tallio"sv};
static constexpr inline Element<tstring<'N', 'h'>{}> nihonio{113, 284.0, "nihonio"sv, L"nihonio"sv};

// gruppo 14-15-16 -> non metalli
static constexpr inline Element<tstring<'C'>{}, +2, +4, -4> carbonio{6, 12.01, "carbonio"sv, L"carbonio"sv};
static constexpr inline Element<tstring<'S', 'i'>{}, +2, +4, -4> silicio{14, 28.09, "silicio"sv, L"silicio"sv};
static constexpr inline Element<tstring<'G', 'e'>{}, +2, +4> germanio{32, 72.64, "germanio"sv, L"germanio"sv};
static constexpr inline Element<tstring<'S', 'n'>{}, +2, +4> stagno{50, 118.7, "stagno"sv, L"stagno"sv};
static constexpr inline Element<tstring<'P', 'b'>{}, +2, +4> piombo{82, 207.2, "piombo"sv, L"piombo"sv};
static constexpr inline Element<tstring<'F', 'l'>{}> flerovio{114, 289.0, "flerovio"sv, L"flerovio"sv};

static constexpr inline Element<tstring<'N'>{}, +2, +3, -3, +4, +5> azoto{7, 14.01, "azoto"sv, L"azoto"sv};
static constexpr inline Element<tstring<'P'>{}, +3, -3, +5> fosforo{15, 30.97, "fosforo"sv, L"fosforo"sv};
static constexpr inline Element<tstring<'A', 's'>{}, +3, -3, +5> arsenico{33, 74.92, "arsenico"sv, L"arsenico"sv};
static constexpr inline Element<tstring<'S', 'b'>{}, +3, -3, +5> antimonio{51, 121.8, "antimonio"sv, L"antimonio"sv};
static constexpr inline Element<tstring<'B', 'i'>{}, +3, +5> bismuto{83, 209.0, "bismuto"sv, L"bismuto"sv};
static constexpr inline Element<tstring<'M', 'c'>{}> moscovio{115, 288.0, "moscovio"sv, L"moscovio"sv};

static constexpr inline Element<tstring<'O'>{}, -2> ossigeno{8, 16.0, "ossigeno"sv, L"ossigeno"sv};
static constexpr inline Element<tstring<'S'>{}, -2, +4, +6> zolfo{16, 32.07, "zolfo"sv, L"zolfo"sv};
static constexpr inline Element<tstring<'S', 'e'>{}, -2, +4, +6> selenio{34, 78.96, "selenio"sv, L"selenio"sv};
static constexpr inline Element<tstring<'T', 'e'>{}, -2, +4, +6> tellurio{52, 127.6, "tellurio"sv, L"tellurio"sv};
static constexpr inline Element<tstring<'P', 'o'>{}, +2, +4, +6> polonio{84, 209.0, "polonio"sv, L"polonio"sv};
static constexpr inline Element<tstring<'L', 'v'>{}> livermorio{116, 293.0, "livermorio"sv, L"livermorio"sv};

// gruppo 17 -> alogeni
static constexpr inline Element<tstring<'F'>{}, -1> fluoro{9, 19.0, "fluoro"sv, L"fluoro"sv};
static constexpr inline Element<tstring<'C', 'l'>{}, +1, -1, +3, +5, +7> cloro{17, 35.45, "cloro"sv, L"cloro"sv};
static constexpr inline Element<tstring<'B', 'r'>{}, +1, -1, +3, +5> bromo{35, 79.91, "bromo"sv, L"bromo"sv};
static constexpr inline Element<tstring<'I'>{}, +1, -1, +5, +7> iodio{53, 126.9, "iodio"sv, L"iodio"sv};
static constexpr inline Element<tstring<'A', 't'>{}, +1, -1, +3, +5, +7> astato{85, 210.0, "astato"sv, L"astato"sv};
static constexpr inline Element<tstring<'T', 's'>{}> tennessinio{117, 310.0, "tennessinio"sv, L"tennessinio"sv};

// gruppo 18 -> gas nobili
static constexpr inline Element<tstring<'H', 'e'>{}> elio{2, 4.003, "elio"sv, L"elio"sv};
static constexpr inline Element<tstring<'N', 'e'>{}> neon{10, 20.18, "neon"sv, L"neon"sv};
static constexpr inline Element<tstring<'A', 'r'>{}> argon{18, 39.95, "argon"sv, L"argon"sv};
static constexpr inline Element<tstring<'K', 'r'>{}> cripton{36, 83.8, "cripton"sv, L"cripton"sv};
static constexpr inline Element<tstring<'X', 'e'>{}> xenon{54, 131.3, "xenon"sv, L"xenon"sv};
static constexpr inline Element<tstring<'R', 'n'>{}> radon{86, 222.0, "radon"sv, L"radon"sv};
static constexpr inline Element<tstring<'O', 'g'>{}> oganessio{118, 314.0, "oganessio"sv, L"oganessio"sv};

// lantanidi
static constexpr inline Element<tstring<'C', 'e'>{}, +3, +4> cerio{58, 140.1, "cerio"sv, L"cerio"sv};
static constexpr inline Element<tstring<'P', 'r'>{}, +3> praseodimio{59, 140.9, "praseodimio"sv, L"praseodimio"sv};
static constexpr inline Element<tstring<'N', 'd'>{}, +3> neodimio{60, 144.2, "neodimio"sv, L"neodimio"sv};
static constexpr inline Element<tstring<'P', 'm'>{}, +3> promezio{61, 145.0, "promezio"sv, L"promezio"sv};
static constexpr inline Element<tstring<'S', 'm'>{}, +2, +3> samario{62, 150.4, "samario"sv, L"samario"sv};
static constexpr inline Element<tstring<'E', 'u'>{}, +2, +3> europio{63, 152.0, "europio"sv, L"europio"sv};
static constexpr inline Element<tstring<'G', 'd'>{}, +3> gadolinio{64, 157.3, "gadolinio"sv, L"gadolinio"sv};
static constexpr inline Element<tstring<'T', 'b'>{}, +3> terbio{65, 158.9, "terbio"sv, L"terbio"sv};
static constexpr inline Element<tstring<'D', 'y'>{}, +3> disprosio{66, 162.5, "disprosio"sv, L"disprosio"sv};
static constexpr inline Element<tstring<'H', 'o'>{}, +3> olmio{67, 164.9, "olmio"sv, L"olmio"sv};
static constexpr inline Element<tstring<'E', 'r'>{}, +3> erbio{68, 167.3, "erbio"sv, L"erbio"sv};
static constexpr inline Element<tstring<'T', 'm'>{}, +2, +3> tulio{69, 168.9, "tulio"sv, L"tulio"sv};
static constexpr inline Element<tstring<'Y', 'b'>{}, +2, +3> itterbio{70, 173.0, "itterbio"sv, L"itterbio"sv};
static constexpr inline Element<tstring<'L', 'u'>{}, +3> lutezio{71, 175.0, "lutezio"sv, L"lutezio"sv};

// attinidi
static constexpr inline Element<tstring<'T', 'h'>{}, +4> torio{90, 232.0, "torio"sv, L"torio"sv};
static constexpr inline Element<tstring<'P', 'a'>{}, +4, +5> protoattinio{91, 231.0, "protoattinio"sv,
                                                                          L"protoattinio"sv};
static constexpr inline Element<tstring<'U'>{}, +3, +4, +5, +6> uranio{92, 238.0, "uranio"sv, L"uranio"sv};
static constexpr inline Element<tstring<'N', 'p'>{}, +3, +4, +5, +6> nettunio{93, 237.0, "nettunio"sv, L"nettunio"sv};
static constexpr inline Element<tstring<'P', 'u'>{}, +3, +4, +5, +6> plutonio{94, 244.0, "plutonio"sv, L"plutonio"sv};
static constexpr inline Element<tstring<'A', 'm'>{}, +3, +4, +5, +6> americio{95, 243.0, "americio"sv, L"americio"sv};
static constexpr inline Element<tstring<'C', 'm'>{}, +3> curio{96, 247.0, "curio"sv, L"curio"sv};
static constexpr inline Element<tstring<'B', 'k'>{}, +3, +4> berkelio{97, 247.0, "berkelio"sv, L"berkelio"sv};
static constexpr inline Element<tstring<'C', 'f'>{}, +3> californio{98, 251.0, "californio"sv, L"californio"sv};
static constexpr inline Element<tstring<'E', 's'>{}, +3> einstenio{99, 252.0, "einstenio"sv, L"einstenio"sv};
static constexpr inline Element<tstring<'F', 'm'>{}, +3> fermio{100, 257.0, "fermio"sv, L"fermio"sv};
static constexpr inline Element<tstring<'M', 'd'>{}, +2, +3> mendelevio{101, 258.0, "mendelevio"sv, L"mendelevio"sv};
static constexpr inline Element<tstring<'N', 'o'>{}, +2, +3> nobelio{102, 259.0, "nobelio"sv, L"nobelio"sv};
static constexpr inline Element<tstring<'L', 'r'>{}, +3> laurenzio{103, 262.0, "laurenzio"sv, L"laurenzio"sv};

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