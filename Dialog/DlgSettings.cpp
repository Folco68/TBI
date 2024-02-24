/*
 * TBI - Technical Bulletin Indexer - Save and index Technical Bulletins,
 * allowing to use keywords to find them easily
 * Copyright (C) 2020 Martial Demolins AKA Folco
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * mail: martial <dot> demolins <at> gmail <dot> com
 */

#include "DlgSettings.hpp"
#include "Global.hpp"
#include "Settings.hpp"
#include "ui_DlgSettings.h"
#include <QPushButton>

DlgSettings::DlgSettings(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::DlgSettings)
{
    ui->setupUi(this);
    setMinimumWidth(DLGSETTINGS_WINDOW_WIDTH);

    // Connections
    connect(ui->ButtonOK, &QPushButton::clicked, this, [this]() { accept(); });
    connect(ui->ButtonCancel, &QPushButton::clicked, this, [this]() { reject(); });
    connect(ui->ButtonClearCategories, &QPushButton::clicked, this, []() { Settings::instance()->resetCategories(); });
    connect(ui->ButtonResetTBurl, &QPushButton::clicked, this, [this]() {
        Settings::instance()->resetBaseURLTechnicalBulletin();
        ui->EditTBurl->setText(Settings::instance()->baseURLTechnicalBulletin());
    });
    connect(ui->ButtonResetTechPubUrl, &QPushButton::clicked, this, [this]() {
        Settings::instance()->resetBaseURLTechnicalPublication();
        ui->EditTechPubUrl->setText(Settings::instance()->baseURLTechnicalPublication());
    });

    // Fill UI
    ui->EditTBurl->setText(Settings::instance()->baseURLTechnicalBulletin());
    ui->EditTechPubUrl->setText(Settings::instance()->baseURLTechnicalPublication());
    ui->CheckRTSearch->setChecked(Settings::instance()->realTimeSearchEnabled());
    ui->CheckWholeWordsOnly->setChecked(Settings::instance()->wholeWordsOnlyEnabled());
    ui->CheckSearchNumber->setChecked(Settings::instance()->searchNumberEnabled());
    ui->CheckSearchTitle->setChecked(Settings::instance()->searchTitleEnabled());
    ui->CheckSearchCategory->setChecked(Settings::instance()->searchCategoryEnabled());
    ui->CheckSearchRK->setChecked(Settings::instance()->searchRKEnabled());
    ui->CheckSearchTechPub->setChecked(Settings::instance()->searchTechPubEnabled());
    ui->CheckSearchReleaseDate->setChecked(Settings::instance()->searchReleaseDateEnabled());
    ui->CheckSearchRegisteredBy->setChecked(Settings::instance()->searchRegisteredByEnabled());
    ui->CheckSearchReplaces->setChecked(Settings::instance()->searchReplacesEnabled());
    ui->CheckSearchReplacedBy->setChecked(Settings::instance()->searchReplacedByEnabled());
    ui->CheckSearchNotes->setChecked(Settings::instance()->searchCommentEnabled());
}

DlgSettings::~DlgSettings()
{
    delete ui;
}

bool DlgSettings::showDlgSettings(QWidget* parent)
{
    bool         SearchChanged = false;
    DlgSettings* Dlg           = new DlgSettings(parent);

    // Save initial search state
    bool OrgNumber         = Dlg->ui->CheckSearchNumber->isChecked();
    bool OrgTitle          = Dlg->ui->CheckSearchTitle->isChecked();
    bool OrgCaterogy       = Dlg->ui->CheckSearchCategory->isChecked();
    bool OrgRK             = Dlg->ui->CheckSearchRK->isChecked();
    bool OrgTechPub        = Dlg->ui->CheckSearchTechPub->isChecked();
    bool OrgReleaseDate    = Dlg->ui->CheckSearchReleaseDate->isChecked();
    bool OrgRegisteredBy   = Dlg->ui->CheckSearchRegisteredBy->isChecked();
    bool OrgReplaces       = Dlg->ui->CheckSearchReplaces->isChecked();
    bool OrgReplacedBy     = Dlg->ui->CheckSearchReplacedBy->isChecked();
    bool OrgWholeWordsOnly = Dlg->ui->CheckWholeWordsOnly->isChecked();
    bool OrgRealTimeSearch = Dlg->ui->CheckRTSearch->isChecked();
    bool OrgNotes          = Dlg->ui->CheckSearchNotes->isChecked();

    // Execute the dialog
    // Save the settings if it was accepted
    if (Dlg->exec() == QDialog::Accepted) {
        Settings::instance()->setBaseURLTechnicalPublication(Dlg->ui->EditTechPubUrl->text());
        Settings::instance()->setBaseURLTechnicalBulletin(Dlg->ui->EditTBurl->text());
        Settings::instance()->setRealTimeSearchEnabled(Dlg->ui->CheckRTSearch->isChecked());
        Settings::instance()->setWholeWordsOnlyEnabled(Dlg->ui->CheckWholeWordsOnly->isChecked());
        Settings::instance()->setSearchNumber(Dlg->ui->CheckSearchNumber->isChecked());
        Settings::instance()->setSearchTitle(Dlg->ui->CheckSearchTitle->isChecked());
        Settings::instance()->setSearchCategory(Dlg->ui->CheckSearchCategory->isChecked());
        Settings::instance()->setSearchRK(Dlg->ui->CheckSearchRK->isChecked());
        Settings::instance()->setSearchTechPub(Dlg->ui->CheckSearchTechPub->isChecked());
        Settings::instance()->setSearchReleaseDate(Dlg->ui->CheckSearchReleaseDate->isChecked());
        Settings::instance()->setSearchRegisteredBy(Dlg->ui->CheckSearchRegisteredBy->isChecked());
        Settings::instance()->setSearchReplaces(Dlg->ui->CheckSearchReplaces->isChecked());
        Settings::instance()->setSearchReplacedBy(Dlg->ui->CheckSearchReplacedBy->isChecked());
        Settings::instance()->setSearchComment(Dlg->ui->CheckSearchNotes->isChecked());

        // We return true if search conditions have changed
        SearchChanged = (OrgNumber != Dlg->ui->CheckSearchNumber->isChecked()) || (OrgTitle != Dlg->ui->CheckSearchTitle->isChecked())
                     || (OrgCaterogy != Dlg->ui->CheckSearchCategory->isChecked()) || (OrgRK != Dlg->ui->CheckSearchRK->isChecked())
                     || (OrgTechPub != Dlg->ui->CheckSearchTechPub->isChecked()) || (OrgReleaseDate != Dlg->ui->CheckSearchReleaseDate->isChecked())
                     || (OrgRegisteredBy != Dlg->ui->CheckSearchRegisteredBy->isChecked()) || (OrgReplaces != Dlg->ui->CheckSearchReplaces->isChecked())
                     || (OrgReplacedBy != Dlg->ui->CheckSearchReplacedBy->isChecked()) || (OrgWholeWordsOnly != Dlg->ui->CheckWholeWordsOnly->isChecked())
                     || (OrgRealTimeSearch != Dlg->ui->CheckRTSearch->isChecked()) || (OrgNotes != Dlg->ui->CheckSearchNotes->isChecked());
    }

    delete Dlg;
    return SearchChanged;
}
