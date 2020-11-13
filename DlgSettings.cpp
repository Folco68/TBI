#include "DlgSettings.hpp"
#include "Global.hpp"
#include "Settings.hpp"
#include "ui_DlgSettings.h"
#include <QPushButton>

DlgSettings::DlgSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgSettings)
{
    ui->setupUi(this);
    setMinimumWidth(DLGSETTINGS_WINDOW_WIDTH);

    // Connections
    connect(ui->ButtonOK, &QPushButton::clicked, [this]() { accept(); });
    connect(ui->ButtonCancel, &QPushButton::clicked, [this]() { reject(); });
    connect(ui->ButtonClearCategories, &QPushButton::clicked, []() { Settings::instance()->resetCategories(); });
    connect(ui->ButtonResetRK, &QPushButton::clicked, [this]() {
        Settings::instance()->resetBaseURLTechnicalPublications();
        ui->EditRK->setText(Settings::instance()->baseURLTechnicalPublication());
    });
    connect(ui->ButtonResetRM, &QPushButton::clicked, [this]() {
        Settings::instance()->resetBaseURLRebuildingManual();
        ui->EditRM->setText(Settings::instance()->baseURLRebuildingManual());
    });

    // Fill UI
    ui->EditRK->setText(Settings::instance()->baseURLTechnicalPublication());
    ui->EditRM->setText(Settings::instance()->baseURLRebuildingManual());
    ui->CheckRTSearch->setChecked(Settings::instance()->realTimeSearchEnabled());
    ui->CheckWholeWordsOnly->setChecked(Settings::instance()->wholeWordsOnlyEnabled());
}

DlgSettings::~DlgSettings()
{
    delete ui;
}

void DlgSettings::showDlgSettings(QWidget* parent)
{
    DlgSettings* dlg = new DlgSettings(parent);
    if (dlg->exec() == QDialog::Accepted) {
        Settings::instance()->setBaseURLRebuildingManual(dlg->ui->EditRM->text());
        Settings::instance()->setBaseURLTechnicalPublications(dlg->ui->EditRK->text());
        Settings::instance()->setRealTimeSearchEnabled(dlg->ui->CheckRTSearch->isChecked());
        Settings::instance()->setWholeWordsOnlyEnabled(dlg->ui->CheckWholeWordsOnly->isChecked());
    }
    delete dlg;
}
