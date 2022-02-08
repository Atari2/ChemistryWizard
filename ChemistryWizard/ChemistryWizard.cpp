#include "ChemistryWizardUI.h"

#include <cstdio>
#include <string>
#include <vector>
#include <QApplication>

#ifdef _WIN32
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(GetACP());
#endif
    QApplication a(argc, argv);
    ChemistryWizardUI w;
    w.show();
    return a.exec();
}