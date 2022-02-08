#pragma once
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
    class ChemistryWizardUI;
}
QT_END_NAMESPACE

class ChemistryWizardUI : public QMainWindow {
    Q_OBJECT

    public:
    ChemistryWizardUI(QWidget* parent = nullptr);
    ~ChemistryWizardUI();

    private:
    Ui::ChemistryWizardUI* ui;
};