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

#include "DlgTB.hpp"
#include "Global.hpp"
#include "ui_DlgTB.h"
#include <QDesktopServices>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>

//  DlgTB
//
// New TB from scratch
//
DlgTB::DlgTB(QWidget* parent, QString title)
    : QDialog(parent)
    , ui(new Ui::DlgTB)
{
    // UI
    ui->setupUi(this);
    setWindowTitle(title);
    setMinimumSize(DLGTB_WINDOW_WIDTH, DLGTB_WINDOW_HEIGHT);
    ui->EditReleaseDate->setDate(QDate::currentDate());

    // Connections
    connect(ui->ButtonOK, &QPushButton::clicked, [this]() { accept(); });
    connect(ui->ButtonCancel, &QPushButton::clicked, [this]() { reject(); });
    connect(ui->ButtonWebPage, &QPushButton::clicked, [this]() { QDesktopServices::openUrl(QString(BASE_URL_TECH_PUB).arg(ui->EditNumber->text())); });
    connect(ui->ButtonDownloadRM, &QPushButton::clicked, [this]() { QDesktopServices::openUrl(QString(BASE_URL_RM).arg(ui->EditTechPub->text())); });

    // Enable/disable buttons
    connect(ui->EditNumber, &QLineEdit::textChanged, [this]() { ui->ButtonWebPage->setDisabled(ui->EditNumber->text().isEmpty()); });
    connect(ui->EditTechPub, &QLineEdit::textChanged, [this]() { ui->ButtonDownloadRM->setDisabled(ui->EditTechPub->text().isEmpty()); });
}

//  DlgTB
//
// New TB from drop, or TB edition
//
DlgTB::DlgTB(QWidget* parent, QString title, TechnicalBulletin* tb)
    : DlgTB(parent, title)
{
    fillUI(tb);
}

DlgTB::~DlgTB()
{
    delete ui;
}

//  fillUI
//
// Populate UI fields with an existing TB
//
void DlgTB::fillUI(TechnicalBulletin* tb)
{
    ui->EditNumber->setText(tb->number());
    ui->EditTitle->setText(tb->title());
    ui->ComboCategory->setCurrentText(tb->category());
    ui->EditRK->setText(tb->rk());
    ui->EditTechPub->setText(tb->techpub());
    ui->TexteditComment->setPlainText(tb->comment());
    ui->EditReleaseDate->setDate(tb->releaseDate());
    ui->EditRegisteredBy->setText(tb->registeredBy());
    ui->EditReplaces->setText(tb->replaces());
    ui->EditReplacedBy->setText(tb->replacedBy());
    ui->EditKeywords->setText(tb->keywordsString());
}

//  fillTB
//
// Grab UI data and save them in a TB
//
void DlgTB::fillTB(TechnicalBulletin* tb)
{
    tb->setData(ui->EditNumber->text(),
                ui->EditTitle->text(),
                ui->ComboCategory->currentText(),
                ui->EditRK->text(),
                ui->EditTechPub->text(),
                ui->TexteditComment->toPlainText(),
                ui->EditReleaseDate->date(),
                ui->EditRegisteredBy->text(),
                ui->EditReplaces->text(),
                ui->EditReplacedBy->text(),
                ui->EditKeywords->text().split(KEYWORD_SEPARATOR, QString::SkipEmptyParts));
}

//  newDlgTB (static)
//
// New TB from scratch
//
TechnicalBulletin* DlgTB::newDlgTB(QWidget* parent)
{
    TechnicalBulletin* tb = nullptr;

    // Create and exec dialog
    DlgTB* dlg = new DlgTB(parent, QString("%1 - %2").arg(WINDOW_TITLE).arg(tr("Add a new Technical Bulletin")));
    if (dlg->exec() == QDialog::Accepted) {
        tb = new TechnicalBulletin;
        dlg->fillTB(tb);
    }

    delete dlg;
    return tb;
}

//  newDlgTB (static)
//
// New TB from drop
//
TechnicalBulletin* DlgTB::newDlgTB(QWidget* parent, QByteArray data)
{
    TechnicalBulletin* tb = new TechnicalBulletin(data);

    // Create and exec dialog. Update TB if dialog was accepted, else destroy it
    DlgTB* dlg = new DlgTB(parent, QString("%1 - %2: %3").arg(WINDOW_TITLE).arg(tr("Import Technical Bulletin: ")).arg(tb->number()), tb);
    if (dlg->exec() == QDialog::Accepted) {
        dlg->fillTB(tb);
    }
    else {
        delete tb;
        tb = nullptr;
    }

    delete dlg;
    return tb;
}

//  editDlgTB (static)
//
// Existing TB edition
//
bool DlgTB::editDlgTB(QWidget* parent, TechnicalBulletin* tb)
{
    bool ret = false;

    DlgTB* dlg = new DlgTB(parent, QString("%1 - %2: %3").arg(WINDOW_TITLE).arg(tr("Edit Technical Bulletin")).arg(tb->number()), tb);
    if (dlg->exec() == QDialog::Accepted) {
        dlg->fillTB(tb);
        ret = true;
    }

    delete dlg;
    return ret;
}

//  dragEnterEvent
//
// Allow a drop to start if dragged data is valid
//
void DlgTB::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat("text/plain"))
        event->acceptProposedAction();
}

//  dropEvent
//
// Offer to create a new TB from dropped data
//
void DlgTB::dropEvent(QDropEvent* event)
{
    TechnicalBulletin* tb = new TechnicalBulletin(event->mimeData()->data("text/plain"));
    fillUI(tb);
    delete tb;
}
