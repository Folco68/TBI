/***********************************************************************************************************************
 *                                                                                                                     *
 *                        TBI - Technical Bulletin Indexer - Save and index Technical Bulletins                        *
 *                                    allowing to use keywords to find them easily                                     *
 *                                 Copyright (C) 2020-2025 Martial Demolins AKA Folco                                  *
 *                                                                                                                     *
 *                        This program is free software: you can redistribute it and/or modify                         *
 *                        it under the terms of the GNU General Public License as published by                         *
 *                          the Free Software Foundation, either version 3 of the License, or                          *
 *                                         (at your option) any later version                                          *
 *                                                                                                                     *
 *                           This program is distributed in the hope that it will be useful                            *
 *                           but WITHOUT ANY WARRANTY; without even the implied warranty of                            *
 *                            MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                            *
 *                                     GNU General Public License for more details                                     *
 *                                                                                                                     *
 *                          You should have received a copy of the GNU General Public License                          *
 *                         along with this program.  If not, see <https://www.gnu.org/licenses                         *
 *                                                                                                                     *
 *                                  mail: martial <dot> demolins <at> gmail <dot> com                                  *
 *                                                                                                                     *
 **********************************************************************************************************************/

#include "DlgTB.hpp"
#include "Global.hpp"
#include "Index/Index.hpp"
#include "Settings.hpp"
#include "UI/LineEditDeselect.hpp"
#include "ui_DlgTB.h"
#include <Event/EventNewTB.hpp>
#include <QAction>
#include <QApplication>
#include <QChar>
#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
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
DlgTB::DlgTB(MainWindow* parent)
    : QDialog(parent)
    , ui(new Ui::DlgTB)
    , DLMenu(new DownloadMenu)
{
    // UI
    ui->setupUi(this);
    setMinimumSize(DLGTB_WINDOW_WIDTH, DLGTB_WINDOW_HEIGHT);
    ui->ComboCategory->addItems(Settings::instance()->categories());
    ui->EditKeywords->setFocus();

    // Create the Screen menu
    QMenu*   ScreenMenu           = new QMenu(this);
    QAction* ActionCopyScreenshot = ScreenMenu->addAction(tr("Copy to clipboard"));
    QAction* ActionSaveToFile     = ScreenMenu->addAction(tr("Save to file"));
    ui->ButtonScreen->setMenu(ScreenMenu);

    // Create the Copy menu
    QMenu*   CopyMenu         = new QMenu(this);
    QAction* ActionCopyHeader = CopyMenu->addAction(tr("Header"));
    QAction* ActionCopyAll    = CopyMenu->addAction(tr("All"));
    ui->ButtonCopy->setMenu(CopyMenu);

    // Standard buttons
    connect(ui->ButtonOK, &QPushButton::clicked, [this]() { accept(); });
    connect(ui->ButtonCancel, &QPushButton::clicked, [this]() { reject(); });

    // Button Web page
    connect(ui->EditNumber, &QLineEdit::textChanged, [this]() {
        ui->ButtonWebPage->setDisabled(ui->EditNumber->text().isEmpty());
    });
    connect(ui->ButtonWebPage, &QPushButton::clicked, [this]() {
        QDesktopServices::openUrl(QString(Settings::instance()->baseURLTechnicalBulletinWebpage()).arg(ui->EditNumber->text()));
        ui->EditKeywords->setFocus();
    });

    // Button Download
    // The downloads are available in the menu attached to the button
    connect(ui->EditNumber, &QLineEdit::textChanged, [this]() { updateButtonDownload(); });
    connect(ui->EditTechPub, &QLineEdit::textChanged, [this]() { updateButtonDownload(); });

    // Menus actions
    connect(ActionCopyScreenshot, &QAction::triggered, [this]() { copyScreenshot(); });
    connect(ActionSaveToFile, &QAction::triggered, [this]() { saveScreenshot(); });
    connect(ActionCopyHeader, &QAction::triggered, [this]() { copyHeader(); });
    connect(ActionCopyAll, &QAction::triggered, [this]() { copyAll(); });

    // Default: don't display the warning about an existing older TB
    ui->LabelReplaceExistent->setVisible(false);
}

DlgTB::~DlgTB()
{
    delete ui;
    delete this->DLMenu;
}

void DlgTB::accept() // override
{
    // Save the category if it does not exist in the list yet
    QStringList List     = Settings::instance()->categories();
    QString     Category = ui->ComboCategory->currentText();
    if (!List.contains(Category, Qt::CaseInsensitive)) {
        List << Category;
        Settings::instance()->setCategories(List);
    }
    QDialog::accept();
}

//  newDlgTB (static)
//
// New TB from scratch
//
void DlgTB::newDlgTB(MainWindow* parent)
{
    // Create and exec dialog
    DlgTB* Dlg = new DlgTB(parent);
    Dlg->setWindowTitle(tr("%1 - %2").arg(WINDOW_TITLE).arg(tr("Add a new Technical Bulletin")));
    Dlg->ui->EditReleaseDate->setDate(QDate::currentDate());
    Dlg->updateButtonDownload();
    if (Dlg->exec() == QDialog::Accepted) {
        Dlg->postTBcreationEvent(Dlg);
    }
    delete Dlg;
}

//  newDlgTB (static)
//
// New TB from drop
//
void DlgTB::newDlgTB(MainWindow* parent, QByteArray data)
{
    // Create and exec the dialog
    DlgTB* Dlg = new DlgTB(parent);
    Dlg->parseDroppedData(data);
    Dlg->setWindowTitle(QString("%1 - %2: %3").arg(WINDOW_TITLE, tr("Import Technical Bulletin: "), Dlg->ui->EditNumber->text()));
    Dlg->updateButtonDownload();
    if (Dlg->exec() == QDialog::Accepted) {
        Dlg->postTBcreationEvent(Dlg);
    }
    delete Dlg;
}

//  editDlgTB (static)
//
// Existing TB edition
//
bool DlgTB::editDlgTB(MainWindow* parent, TechnicalBulletin* tb)
{
    bool Return = false;

    DlgTB* Dlg = new DlgTB(parent);
    Dlg->ui->EditNumber->setText(tb->number());
    Dlg->ui->EditTitle->setText(tb->title());
    Dlg->ui->ComboCategory->setCurrentText(tb->category());
    Dlg->ui->EditRK->setText(tb->rk());
    Dlg->ui->EditTechPub->setText(tb->techpub());
    Dlg->ui->TexteditComment->setPlainText(tb->comment());
    Dlg->ui->EditReleaseDate->setDate(tb->releaseDate());
    Dlg->ui->EditRegisteredBy->setText(tb->registeredBy());
    Dlg->ui->EditReplaces->setText(tb->replaces());
    Dlg->ui->EditReplacedBy->setText(tb->replacedBy());
    Dlg->ui->EditKeywords->setText(tb->keywordsString());

    Dlg->ui->LabelReplaceExistent->setVisible(parent->tbNumberAlreadyExists(tb));
    Dlg->updateButtonDownload();

    Dlg->setWindowTitle(QString("%1 - %2: %3").arg(WINDOW_TITLE, tr("Edit Technical Bulletin"), tb->number()));
    if (Dlg->exec() == QDialog::Accepted) {
        tb->updateData(Dlg->ui->EditNumber->text(),
                       Dlg->ui->EditTitle->text(),
                       Dlg->ui->ComboCategory->currentText(),
                       Dlg->ui->EditRK->text(),
                       Dlg->ui->EditTechPub->text(),
                       Dlg->ui->TexteditComment->toPlainText(),
                       Dlg->ui->EditReleaseDate->date(),
                       Dlg->ui->EditRegisteredBy->text(),
                       Dlg->ui->EditReplaces->text(),
                       Dlg->ui->EditReplacedBy->text(),
                       Dlg->ui->EditKeywords->text().split(KEYWORD_SEPARATOR, Qt::SkipEmptyParts));
        Return = true;
    }

    delete Dlg;
    return Return;
}

void DlgTB::postTBcreationEvent(DlgTB* dlg) const
{
    EventNewTB* Event = new EventNewTB(dlg->ui->EditNumber->text(),
                                       dlg->ui->EditTitle->text(),
                                       dlg->ui->ComboCategory->currentText(),
                                       dlg->ui->EditRK->text(),
                                       dlg->ui->EditTechPub->text(),
                                       dlg->ui->TexteditComment->toPlainText(),
                                       dlg->ui->EditReleaseDate->date(),
                                       dlg->ui->EditRegisteredBy->text(),
                                       dlg->ui->EditReplaces->text(),
                                       dlg->ui->EditReplacedBy->text(),
                                       dlg->ui->EditKeywords->text().split(KEYWORD_SEPARATOR, Qt::SkipEmptyParts));
    QCoreApplication::postEvent(Index::instance(), Event);
}

void DlgTB::updateButtonDownload()
{
    QString DocsField   = ui->EditTechPub->text();
    QString NumberField = ui->EditNumber->text().trimmed();
    this->DLMenu->setItems(DocsField, NumberField, ui->EditKeywords);
    ui->ButtonDownload->setMenu(this->DLMenu);
    ui->ButtonDownload->setDisabled(this->DLMenu->isEmpty());
}

void DlgTB::parseDroppedData(QByteArray data)
{
    qsizetype Start, End;

    // Parse most of the strings
    QList<QString> Strings;
    QList<QString> StringLabels;
    StringLabels << "Bulletin No:"
                 << "Title:"
                 << "TB Category:"
                 << "Rebuilding Kit(s):"
                 << "Technical Publication(s):"
                 << "Registered by:"
                 << "Replaces:"
                 << "Replaced by:";

    for (int i = 0; i < StringLabels.count(); i++) {
        Start = data.indexOf(QByteArrayView(StringLabels.at(i).toUtf8())); // Look for a label
        if (Start == -1) {
            // No label found, don't write anything in the field
            Strings << "";
        }
        else {
            // Label found
            Start = data.indexOf('\t', Start) + 1;                                  // Skip the label, and find the fist byte of the data string
            End   = std::min(data.indexOf('\t', Start), data.indexOf('\n', Start)); // End of data string. May terminate with a Tab or a New Line
            Strings << data.mid(Start, End - Start);                                // Grab and save data
        }
    }

    // Comment box. Don't fill it if no comment field is found
    Start = data.indexOf("Comments:");
    if (Start == -1) {
        Start = 0;
        End   = 0;
    }
    else {
        Start = data.indexOf('\t', Start) + 1;
        End   = data.indexOf('\t', Start);
    }
    QString Comment(data.mid(Start, End - Start));

    // Release date
    Start = data.indexOf("Release date:");
    Start = data.indexOf('\t', Start) + 1;
    End   = data.indexOf('\n', Start);
    QDate Date(QDate::fromString(data.mid(Start, End - Start), "yyyy-MM-dd"));

    // Fill UI
    ui->EditNumber->setText(Strings.at(0));
    ui->EditTitle->setText(Strings.at(1));
    ui->ComboCategory->setCurrentText(Strings.at(2));
    ui->EditRK->setText(Strings.at(3));
    ui->EditTechPub->setText(Strings.at(4));
    ui->EditRegisteredBy->setText(Strings.at(5));
    ui->EditReplaces->setText(Strings.at(6));
    ui->EditReplacedBy->setText(Strings.at(7));
    ui->TexteditComment->setPlainText(Comment);
    ui->EditReleaseDate->setDate(Date);
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
// Fill UI with dropped data
//
void DlgTB::dropEvent(QDropEvent* event)
{
    parseDroppedData(event->mimeData()->data("text/plain"));
}

void DlgTB::copyScreenshot()
{
    QPixmap Screenshot = this->grab();
    QApplication::clipboard()->setPixmap(Screenshot);
}

void DlgTB::saveScreenshot()
{
    // Build filename:
    // - get title
    // - split it and set to upper case every first char
    // - join
    QString     Filename = ui->EditTitle->text();
    QStringList Words    = Filename.split(' ', Qt::SkipEmptyParts);
    Filename             = QDir::homePath() + '/';

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
    QPixmap Screenshot = this->grab();
    if (!Screenshot.save(Filename)) {
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
    QString Data = getHeader().append("\nNotes: \n").append(ui->TexteditComment->toPlainText());
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
