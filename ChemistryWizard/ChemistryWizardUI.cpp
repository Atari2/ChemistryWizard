#include "ChemistryWizardUI.h"
#include "ui_ChemistryWizard.h"

#include "ChemistryWizard.h"

#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QDockWidget>

ChemistryWizardUI::ChemistryWizardUI(QWidget* parent) : QMainWindow(parent), ui(new Ui::ChemistryWizardUI) {
    ui->setupUi(this);

    auto input = new QTextEdit(this);
    auto output = new QTextEdit(this);
    output->setReadOnly(true);
    output->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);
    input->setAcceptRichText(false);
    auto widget = new QWidget(centralWidget());
    auto layout = new QVBoxLayout(widget);
    widget->setLayout(layout);
    layout->addWidget(input);

    QFont font{};
    font.setPointSize(12);

    for (const auto& named_callback : callbacks) {
        auto btn = new QPushButton(this);
        btn->setFont(font);
        btn->setText(QString::fromStdString(named_callback.name));
        btn->adjustSize();
        QObject::connect(btn, &QPushButton::clicked, this, [input, output, named_callback]() {
            output->clear();
            output->insertPlainText(named_callback.callback(input->toPlainText().toStdString()));
        });
        layout->addWidget(btn);
    }
    layout->addWidget(output);
    widget->adjustSize();
}

ChemistryWizardUI::~ChemistryWizardUI() {
    delete ui;
}
