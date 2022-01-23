#include "ChemistryWizard.h"

#include <cstdio>
#include <string>
#include <vector>

#ifdef _WIN32
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

using callback = void (*)(std::vector<std::string>&);

int main(int argc, char* argv[]) {
    #ifdef _WIN32
    SetConsoleOutputCP(GetACP());
    #endif
    std::vector<std::string> arguments{};
    arguments.reserve(argc);

    callback callbacks[from_enum(Azione::__SIZE__)] = {&do_naming, &do_balance, &do_reduction, &do_other};

    if (argc > 1) {
        auto* action = argv[1];
        auto num_action = std::atoi(action);
        if (num_action >= from_enum(Azione::__SIZE__)) {
            error("Azione non valida, il massimo e' %d, tu hai inserito %d", from_enum(Azione::__SIZE__) - 1,
                  num_action);
        } else {
            for (int i = 2; i < argc; i++) {
                arguments.push_back(argv[i]);
            }
            callbacks[num_action](arguments);
        }
    } else {
        puts(R"(Inserisci un numero da 0 a 3 per eseguire una azione:
0.  Nomenclatura
1.  Bilanciamento
2.  Riduzione
3.  Altro)");
        int num_action = 0;
        (void)scanf("%d", &num_action);
        if (num_action >= from_enum(Azione::__SIZE__)) {
            error("Azione non valida, il massimo e' %d, tu hai inserito %d", from_enum(Azione::__SIZE__) - 1,
                  num_action);
        } else {
            callbacks[num_action](arguments);
        }
    }
    system("pause");
    return EXIT_SUCCESS;
}
