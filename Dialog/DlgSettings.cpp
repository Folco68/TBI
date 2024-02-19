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
    connect(ui->ButtonResetRK, &QPushButton::clicked, this, [this]() {
        Settings::instance()->resetBaseURLTechnicalPublications();
        ui->EditRK->setText(Settings::instance()->baseURLTechnicalPublication());
    });
    connect(ui->ButtonResetRM, &QPushButton::clicked, this, [this]() {
        Settings::instance()->resetBaseURLRebuildingManual();
        ui->EditRM->setText(Settings::instance()->baseURLRebuildingManual());
    });

    // Fill UI
    ui->EditRK->setText(Settings::instance()->baseURLTechnicalPublication());
    ui->EditRM->setText(Settings::instance()->baseURLRebuildingManual());
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
    bool SearchChanged = false;
    DlgSettings* dlg = new DlgSettings(parent);

    // Save initial search state
    bool OrgNumber = dlg->ui->CheckSearchNumber->isChecked();
    bool OrgTitle = dlg->ui->CheckSearchTitle->isChecked();
    bool OrgCaterogy = dlg->ui->CheckSearchCategory->isChecked();
    bool OrgRK = dlg->ui->CheckSearchRK->isChecked();
    bool OrgTechPub = dlg->ui->CheckSearchTechPub->isChecked();
    bool OrgReleaseDate = dlg->ui->CheckSearchReleaseDate->isChecked();
    bool OrgRegisteredBy = dlg->ui->CheckSearchRegisteredBy->isChecked();
    bool OrgReplaces = dlg->ui->CheckSearchReplaces->isChecked();
    bool OrgReplacedBy = dlg->ui->CheckSearchReplacedBy->isChecked();
    bool OrgWholeWordsOnly = dlg->ui->CheckWholeWordsOnly->isChecked();
    bool OrgRealTimeSearch = dlg->ui->CheckRTSearch->isChecked();
    bool OrgNotes = dlg->ui->CheckSearchNotes->isChecked();

    if (dlg->exec() == QDialog::Accepted) {
        Settings::instance()->setBaseURLRebuildingManual(dlg->ui->EditRM->text());
        Settings::instance()->setBaseURLTechnicalPublications(dlg->ui->EditRK->text());
        Settings::instance()->setRealTimeSearchEnabled(dlg->ui->CheckRTSearch->isChecked());
        Settings::instance()->setWholeWordsOnlyEnabled(dlg->ui->CheckWholeWordsOnly->isChecked());
        Settings::instance()->setSearchNumber(dlg->ui->CheckSearchNumber->isChecked());
        Settings::instance()->setSearchTitle(dlg->ui->CheckSearchTitle->isChecked());
        Settings::instance()->setSearchCategory(dlg->ui->CheckSearchCategory->isChecked());
        Settings::instance()->setSearchRK(dlg->ui->CheckSearchRK->isChecked());
        Settings::instance()->setSearchTechPub(dlg->ui->CheckSearchTechPub->isChecked());
        Settings::instance()->setSearchReleaseDate(dlg->ui->CheckSearchReleaseDate->isChecked());
        Settings::instance()->setSearchRegisteredBy(dlg->ui->CheckSearchRegisteredBy->isChecked());
        Settings::instance()->setSearchReplaces(dlg->ui->CheckSearchReplaces->isChecked());
        Settings::instance()->setSearchReplacedBy(dlg->ui->CheckSearchReplacedBy->isChecked());
        Settings::instance()->setSearchComment(dlg->ui->CheckSearchNotes->isChecked());

        // We return true if search conditions have changed
        SearchChanged = (OrgNumber != dlg->ui->CheckSearchNumber->isChecked()) || (OrgTitle != dlg->ui->CheckSearchTitle->isChecked())
                        || (OrgCaterogy != dlg->ui->CheckSearchCategory->isChecked()) || (OrgRK != dlg->ui->CheckSearchRK->isChecked())
                        || (OrgTechPub != dlg->ui->CheckSearchTechPub->isChecked()) || (OrgReleaseDate != dlg->ui->CheckSearchReleaseDate->isChecked())
                        || (OrgRegisteredBy != dlg->ui->CheckSearchRegisteredBy->isChecked()) || (OrgReplaces != dlg->ui->CheckSearchReplaces->isChecked())
                        || (OrgReplacedBy != dlg->ui->CheckSearchReplacedBy->isChecked()) || (OrgWholeWordsOnly != dlg->ui->CheckWholeWordsOnly->isChecked())
                        || (OrgRealTimeSearch != dlg->ui->CheckRTSearch->isChecked()) || (OrgNotes != dlg->ui->CheckSearchNotes->isChecked());
    }

    delete dlg;
    return SearchChanged;
}
