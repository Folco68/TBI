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
#include "Settings.hpp"
#include "ui_DlgTB.h"
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStringList>

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
    ui->ComboCategory->addItems(Settings::instance()->categories());

    // Create the Screen menu
    QMenu* ScreenMenu             = new QMenu(this);
    QAction* ActionCopyScreenshot = ScreenMenu->addAction("Copy to clipboard");
    QAction* ActionSaveToFile     = ScreenMenu->addAction("Save to file");
    ui->ButtonScreen->setMenu(ScreenMenu);

    // Create the Copy menu
    QMenu* CopyMenu           = new QMenu(this);
    QAction* ActionCopyHeader = CopyMenu->addAction("Header");
    QAction* ActionCopyAll    = CopyMenu->addAction("All");
    ui->ButtonCopy->setMenu(CopyMenu);

    // Connections
    connect(ui->ButtonOK, &QPushButton::clicked, [this]() { accept(); });
    connect(ui->ButtonCancel, &QPushButton::clicked, [this]() { reject(); });
    connect(ui->ButtonWebPage, &QPushButton::clicked, [this]() {
        QDesktopServices::openUrl(QString(Settings::instance()->baseURLTechnicalPublication()).arg(ui->EditNumber->text()));
        ui->EditKeywords->setFocus();
    });
    connect(ui->ButtonDownloadRM, &QPushButton::clicked, [this]() {
        QDesktopServices::openUrl(QString(Settings::instance()->baseURLRebuildingManual()).arg(ui->EditTechPub->text()));
        ui->EditKeywords->setFocus();
    });

    // Menus actions
    connect(ActionCopyScreenshot, &QAction::triggered, [this]() { copyScreenshot(); });
    connect(ActionSaveToFile, &QAction::triggered, [this]() { saveToFile(); });
    connect(ActionCopyHeader, &QAction::triggered, [this]() { copyHeader(); });
    connect(ActionCopyAll, &QAction::triggered, [this]() { copyAll(); });

    // Enable/disable buttons
    connect(ui->EditNumber, &QLineEdit::textChanged, [this]() { ui->ButtonWebPage->setDisabled(ui->EditNumber->text().isEmpty()); });
    connect(ui->EditTechPub, &QLineEdit::textChanged, [this]() { ui->ButtonDownloadRM->setDisabled(ui->EditTechPub->text().isEmpty()); });

    // Set the edit keyword field as the current one
    ui->EditKeywords->setFocus();
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

void DlgTB::accept()
{
    // Save the category in the category list if it's a new one
    QStringList list = Settings::instance()->categories();
    QString category = ui->ComboCategory->currentText();
    if (!list.contains(category, Qt::CaseInsensitive)) {
        list << category;
        Settings::instance()->setCategories(list);
    }

    // Finally, run QDialog accept
    QDialog::accept();
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
                ui->EditKeywords->text().split(KEYWORD_SEPARATOR, Qt::SkipEmptyParts));
}

//  newDlgTB (static)
//
// New TB from scratch
//
TechnicalBulletin* DlgTB::newDlgTB(QWidget* parent)
{
    TechnicalBulletin* tb = nullptr;

    // Create and exec dialog
    DlgTB* dlg = new DlgTB(parent, QString("%1 - %2").arg(WINDOW_TITLE, tr("Add a new Technical Bulletin")));
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
    DlgTB* dlg = new DlgTB(parent, QString("%1 - %2: %3").arg(WINDOW_TITLE, tr("Import Technical Bulletin: "), tb->number()), tb);
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

    DlgTB* dlg = new DlgTB(parent, QString("%1 - %2: %3").arg(WINDOW_TITLE, tr("Edit Technical Bulletin"), tb->number()), tb);
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

void DlgTB::copyScreenshot()
{
        QPixmap screenshot = this->grab();
        QApplication::clipboard()->setPixmap(screenshot);
}

void DlgTB::saveToFile()
{
    // Build filename:
    // - get title
    // - split it and set to upper case every first char
    // - join
    QString Filename  = ui->EditTitle->text();
    QStringList Words = Filename.split(' ', Qt::SkipEmptyParts);
    Filename          = QDir::homePath() + '/';

    for (int i = 0; i < Words.count(); i++) {
        QString Word = Words.at(i);
        Filename += Word.at(0).toUpper() + Word.mid(1);
    }
    Filename += SCREENSHOT_EXTENSION;

    // Prompt to save the file
    Filename = QFileDialog::getSaveFileName(this, tr("Select the destination file"), Filename, tr("Images (*.png)"));
    if (Filename.isEmpty()) {
        return;
    }

    // Perform the screenshot and try to save it
    QPixmap screen = this->grab();
    if (!screen.save(Filename)) {
        QMessageBox::critical(this, WINDOW_TITLE, tr("Can't save the screenshot"));
    }
}

void DlgTB::copyHeader()
{
    QString Data = getHeader();
    QApplication::clipboard()->setText(Data);
}
    
void DlgTB::copyAll()
{
    QString Data = getHeader() + '\n';
    Data.append("Notes: %1");
    Data = Data.arg(ui->TexteditComment->toPlainText());
    QApplication::clipboard()->setText(Data);
}

QString DlgTB::getHeader()
{
    QString Data;
    Data.append("Bulletin No: %1\n")
        .append("Title: %2\n")
        .append("TB Category: %3\n")
        .append("Rebuilding Kit: %4\n")
        .append("Technical Publication: %5\n")
        .append("Release date: %6\n")
        .append("Registered by: %7\n")
        .append("Replaces: %8\n")
        .append("Replaced by: %9");

    return Data.arg(ui->EditNumber->text(),
                    ui->EditTitle->text(),
                    ui->ComboCategory->currentText(),
                    ui->EditRK->text(),
                    ui->EditTechPub->text(),
                    ui->EditReleaseDate->text(),
                    ui->EditRegisteredBy->text(),
                    ui->EditReplaces->text(),
                    ui->EditReplacedBy->text());
}
