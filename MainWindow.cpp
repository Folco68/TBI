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

#include "MainWindow.hpp"
#include "Dialog/DlgHelp.hpp"
#include "Dialog/DlgSettings.hpp"
#include "Dialog/DlgTB.hpp"
#include "DownloadMenu.hpp"
#include "Global.hpp"
#include "Settings.hpp"
#include "TechnicalBulletin.hpp"
#include "ui_MainWindow.h"
#include <QAbstractButton>
#include <QAbstractScrollArea>
#include <QClipboard>
#include <QCursor>
#include <QDataStream>
#include <QDesktopServices>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QKeySequence>
#include <QLineEdit>
#include <QList>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QShortcut>
#include <QStatusBar>
#include <QTableWidget>
#include <QTableWidgetItem>

MainWindow::MainWindow(bool ForceDBCheck)
    : QMainWindow()
    , ui(new Ui::MainWindow)
    , Modified(false)
    , MessageTBCount(new QLabel)
    , MessagePendingModifications(new QLabel)
    , TableContextMenu(new QMenu(this))
    , ActionNewTB(new ContextMenuAction(tr("New TB"), this, QKeySequence(Qt::CTRL | Qt::Key_N)))
    , ActionEditTB(new ContextMenuAction(tr("Edit TB"), this, QKeySequence(Qt::CTRL | Qt::Key_E)))
    , ActionDeleteTB(new ContextMenuAction(tr("Delete TB"), this, QKeySequence(Qt::Key_Delete)))
    , ActionCopyUrl(new ContextMenuAction(tr("Copy URL"), this, QKeySequence(Qt::CTRL | Qt::Key_C)))
    , ActionOpenUrl(new ContextMenuAction(tr("Open URL"), this, QKeySequence(Qt::CTRL | Qt::Key_O)))
    , ActionDownload(new ContextMenuAction(tr("Download"), this))
    , ActionSettings(new ContextMenuAction(tr("Settings"), this))
    , ActionHelp(new ContextMenuAction(tr("Help / About"), this, QKeySequence(Qt::Key_F1)))
    , DLMenu(new DownloadMenu)
{
    // Window
    ui->setupUi(this);
    setMinimumSize(MAIN_MINIMUM_WIDTH, MAIN_MINIMUM_HEIGHT);
    resize(Settings::instance()->mainWindowSize());

    // Status bar
    ui->StatusBar->addPermanentWidget(this->MessageTBCount);
    ui->StatusBar->addPermanentWidget(this->MessagePendingModifications);

    // TB table
    ui->TableTB->setShowGrid(true);
    ui->TableTB->setSortingEnabled(true);
    ui->TableTB->setAlternatingRowColors(true);
    ui->TableTB->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->TableTB->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->TableTB->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->TableTB->verticalHeader()->setVisible(false);
    ui->TableTB->horizontalHeader()->setStretchLastSection(true);

    // TB table context menu
    ui->TableTB->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->TableTB, &QWidget::customContextMenuRequested, this, [this]() { this->TableContextMenu->exec(QCursor::pos()); });
    connect(this->ActionNewTB, &QAction::triggered, this, [this]() { newTB(); });
    connect(this->ActionEditTB, &QAction::triggered, this, [this]() { editTB(); });
    connect(this->ActionDeleteTB, &QAction::triggered, this, [this]() {
        deleteTB();
        updateUI();
    });
    connect(this->ActionCopyUrl, &QAction::triggered, this, [this]() { copyURLToClipboard(); });
    connect(this->ActionOpenUrl, &QAction::triggered, this, [this]() { openURL(); });
    connect(this->ActionSettings, &QAction::triggered, this, [this]() {
        if (DlgSettings::showDlgSettings()) {
            search(FORCE_SEARCH);
        }
        updateUI();
    });
    connect(this->ActionHelp, &QAction::triggered, []() { DlgHelp::showDlgHelp(); });

    // Add actions to the context menu and to the main window to allow kbd shortcuts
    QList<QAction*> Actions;
    Actions << this->ActionNewTB << this->ActionEditTB << this->ActionDeleteTB << this->ActionCopyUrl << this->ActionOpenUrl << this->ActionDownload << this->ActionSettings
            << this->ActionHelp;
    this->TableContextMenu->addActions(Actions);
    this->TableContextMenu->insertSeparator(this->ActionCopyUrl);
    this->TableContextMenu->insertSeparator(this->ActionSettings);
    this->addActions(Actions);

    // Paste shortcut
    connect(new QShortcut(QKeySequence(QKeySequence::Paste), this), &QShortcut::activated, this, [this]() { paste(); });

    // Save shortcut
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_S), this), &QShortcut::activated, this, [this]() {
        if (this->Modified) {
            save();
            updateUI();
        }
    });

    // Search shortcut
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this), &QShortcut::activated, this, [this]() { ui->EditKeywords->setFocus(); });

    // Buttons connections
    connect(ui->ButtonSave, &QPushButton::clicked, this, [this]() {
        save();
        updateUI();
    });
    connect(ui->ButtonSearch, &QPushButton::clicked, this, [this]() { search(); });

    // Search connection
    connect(ui->EditKeywords, &QLineEdit::returnPressed, this, [this]() { search(); });
    connect(ui->EditKeywords, &QLineEdit::textChanged, this, [this]() {
        if (ui->EditKeywords->text().isEmpty() || Settings::instance()->realTimeSearchEnabled())
            search();
    });

    // Table connections
    connect(ui->TableTB, &QTableWidget::itemSelectionChanged, this, [this]() { updateUI(); });
    connect(ui->TableTB, &QTableWidget::cellDoubleClicked, this, [this]() { editTB(); });

    // Open TBI if one exists

    // Block the signals until the UI is setup and consistent
    ui->TableTB->blockSignals(true);

    QFile file(TBI_FILENAME);
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream Stream(&file);

        // The first version of .tbi files were not versionned. They contain the number of TB as an int, then the serialized TB themselves.
        // The next versions set this counter to 0, then stores a magic, then the DB version.
        // This makes the old executables unable to open recent files.
        // And recent executables able to understand the old DB.
        qint32 Count;
        Stream >> Count;

        // If count == 0, two cases:
        // - it's an empty old file. Reading the magic will lead to a stream reading error. Let's fail silently the opening.
        // - it's a versionned file. Read the magic then the version number to decide which opener must be used
        if (Count == 0) {
            // Try to read a magic
            QString Magic;
            Stream >> Magic;

            // If the stream failed to read data, it "should" be an empty unversionned file.
            // Ok, it could also be an USB stick pulled out when reading, but we cannot make the difference.
            // Let's assume it's an old empty file and let's do nothing.
            // So, we only consider streams with a successful magic reading.
            if (Stream.status() == QDataStream::Ok) {
                if (Magic == QString(TBI_MAGIC)) {
                    // If the magic is valid, read the version and open the file according to it
                    qint32 Version;
                    Stream >> Version;

                    switch (Version) {
                        case 1:
                            openDBv1(Stream, ForceDBCheck);
                            break;

                        default:
                            // Version of the future, unhandled by this binary...
                            QMessageBox::critical(this, WINDOW_TITLE, tr("The DB file is too recent for this executable. Please find a newer one. Opening aborted."));
                    }
                }

                else {
                    QMessageBox::critical(this,
                                          WINDOW_TITLE,
                                          tr("Invalid file identifier. It looks that the file %1 is corrupted or not authentic. Opening aborted.").arg(TBI_FILENAME));
                }
            }

            // Stream status control failed
            // But don't throw a message, it just means that it was an empty and unversionned file
            // else {
            //     QMessageBox::critical(this, WINDOW_TITLE, tr("Invalid file %1").arg(TBI_FILENAME));
            // }
        }

        // If count != 0, it's an old file, no doubt.
        else {
            openDBv0(Count, Stream, ForceDBCheck);
        }
    }
    // Throw an error if the file exists and couldn't be opened
    else if (QFileInfo::exists(TBI_FILENAME)) {
        QMessageBox::critical(this, WINDOW_TITLE, tr("Unable to open file %1").arg(TBI_FILENAME));
    }

    // Restore signals handling
    ui->TableTB->blockSignals(false);

    // Make UI consistent
    updateUI();

    // Adjust column size
    for (int i = 0; i < ui->TableTB->columnCount() - 1; i++) {
        ui->TableTB->resizeColumnToContents(i);
    }
}

MainWindow::~MainWindow()
{
    // Delete the TBs associated to each row
    for (int i = 0; i < ui->TableTB->rowCount(); i++) {
        delete ui->TableTB->item(i, COLUMN_METADATA)->data(TB_ROLE).value<TechnicalBulletin*>();
    }

    // Save window size
    Settings::instance()->setMainWindowSize(size());

    // Destroy Settings instance (ie. save settings) here and not in main(),
    // because QGuiApplication doesn't return on all platforms
    Settings::release();

    // UI
    delete this->DLMenu;
    delete ui;
}

//  updateUI
//
// Adjust display according to index state
//
void MainWindow::updateUI()
{

    // Window title
    setWindowTitle(QString("%1 %2").arg(WINDOW_TITLE, this->Modified ? "- (modified)" : ""));

    // Button
    ui->ButtonSave->setEnabled(this->Modified);
    ui->ButtonSearch->setVisible(!Settings::instance()->realTimeSearchEnabled());

    // Status bar
    int     Count  = ui->TableTB->rowCount();
    QString Plural = Count > 1 ? "s" : "";
    this->MessageTBCount->setText(tr("%1 Technical Bulletin%2 registered   %3").arg(Count).arg(Plural));
    this->MessagePendingModifications->setText(this->Modified ? tr("Modifications pending") : tr("Index is saved"));

    // Actions (context menu)
    bool ItemSelected = !ui->TableTB->selectedItems().isEmpty();
    this->ActionEditTB->setEnabled(ItemSelected);
    this->ActionDeleteTB->setEnabled(ItemSelected);
    this->ActionCopyUrl->setEnabled(ItemSelected);
    this->ActionOpenUrl->setEnabled(ItemSelected);

    // Download action and sub-menu
    if (ItemSelected) {
        int     Row       = ui->TableTB->currentRow();
        QString DocsField = ui->TableTB->item(Row, COLUMN_TECH_PUB)->text();
        this->DLMenu->setItems(DocsField);
        this->ActionDownload->setMenu(this->DLMenu);
        this->ActionDownload->setDisabled(this->DLMenu->isEmpty());
    }
}

//  save
//
// Save current TBI
//
void MainWindow::save()
{
    // First, create a backup by renaming the current index database
    // Remove current backup because File::rename() won't overwrite backup file
    QFile::remove(TBI_BACKUP_FILENAME);
    if (!QFile::rename(TBI_FILENAME, TBI_BACKUP_FILENAME)) {
        if (QMessageBox::question(this, WINDOW_TITLE, tr("Couldn't create database backup. Save anyway?")) == QMessageBox::No) {
            return;
        }
    }

    // Try to open the file
    QFile File(TBI_FILENAME);
    if (!File.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, WINDOW_TITLE, tr("Couldn't open file %1").arg(TBI_FILENAME));
        return;
    }

    // Open a data stream and write into it
    QDataStream Stream(&File);

    // First, write 0 to support old DB
    Stream << (qint32)0;

    // Then write magic + current version
    Stream << QString(TBI_MAGIC) << (qint32)CURRENT_TBI_VERSION;

    // Write TB count
    Stream << (qint32)(ui->TableTB->rowCount());

    // Serialize TBs
    for (int i = 0; i < ui->TableTB->rowCount(); i++) {
        // Get TB ptr
        TechnicalBulletin* TB = ui->TableTB->item(i, COLUMN_METADATA)->data(TB_ROLE).value<TechnicalBulletin*>();
        Stream << *TB;

        // Check stream status
        if (Stream.status() != QDataStream::Ok) {
            QMessageBox::critical(this, WINDOW_TITLE, tr("Failed to save file %1").arg(TBI_FILENAME));
            return;
        }
    }

    this->Modified = false;
}

//  newTB
//
// Open a dialog allowing to create a TB by hand
//
void MainWindow::newTB()
{
    TechnicalBulletin* TB = DlgTB::newDlgTB(this);
    if (TB != nullptr) {
        this->Modified = true;
        addTB(TB, PERFORM_ADD_CHECKS);
        updateUI();
    }
}

//  editTB
//
// Open a dialog allowing to edit an existing TB
void MainWindow::editTB()
{
    // Get current TB
    QList<QTableWidgetItem*> Items = ui->TableTB->selectedItems();
    int                      Row   = Items.at(0)->row();
    TechnicalBulletin*       TB    = ui->TableTB->item(Row, COLUMN_METADATA)->data(TB_ROLE).value<TechnicalBulletin*>();

    // Open edition dialog
    if (DlgTB::editDlgTB(this, TB)) {
        this->Modified = true;
        updateTB(TB, Row);
    }
}

//  deleteTB
//
// Delete the TB currently selected
//
void MainWindow::deleteTB()
{
    // Get current TB
    QList<QTableWidgetItem*> Selection = ui->TableTB->selectedItems();
    int                      Row       = Selection.at(0)->row();
    TechnicalBulletin*       TB        = ui->TableTB->item(Row, COLUMN_METADATA)->data(TB_ROLE).value<TechnicalBulletin*>();

    // Show a confirmation dialog
    QMessageBox::StandardButton Answer = QMessageBox::question(this, WINDOW_TITLE, tr("Do you want to delete Technical Bulletin %1 (%2)?").arg(TB->number(), TB->title()));
    if (Answer == QMessageBox::Yes) {
        delete TB;
        ui->TableTB->removeRow(Row);

        // Update UI
        this->Modified = true;
    }
}

//  search
//
// Search the TB with keywords
//
void MainWindow::search(bool ForceNewSearch)
{
    // Split and clean the list
    static QStringList Keywords;
    QStringList        UIkeywords = ui->EditKeywords->text().split(KEYWORD_SEPARATOR, Qt::SkipEmptyParts);
    UIkeywords.removeDuplicates();

    // Early return if the list didn't change and we don't force a new search
    if ((UIkeywords == Keywords) && !ForceNewSearch) {
        return;
    }
    Keywords = UIkeywords;

    // Display status bar message
    ui->StatusBar->showMessage(tr("Searching..."));

    // Display all entries if there is no filter
    if (Keywords.count() == 0) {
        // No keyword: display all TBs
        for (int i = 0; i < ui->TableTB->rowCount(); i++) {
            ui->TableTB->setRowHidden(i, false);
        }
    }
    else {
        // Else filter TBs with keywords
        for (int i = 0; i < ui->TableTB->rowCount(); i++) {
            TechnicalBulletin* TB         = ui->TableTB->item(i, COLUMN_METADATA)->data(TB_ROLE).value<TechnicalBulletin*>();
            QStringList        TBkeywords = TB->keywords();

            // Add some fields to the keyword list, depending on global configuration
            if (Settings::instance()->searchNumberEnabled()) {
                TBkeywords << TB->number().split(KEYWORD_SEPARATOR, Qt::SkipEmptyParts);
            }
            if (Settings::instance()->searchTitleEnabled()) {
                TBkeywords << TB->title().split(KEYWORD_SEPARATOR, Qt::SkipEmptyParts);
            }
            if (Settings::instance()->searchCategoryEnabled()) {
                TBkeywords << TB->category().split(KEYWORD_SEPARATOR, Qt::SkipEmptyParts);
            }
            if (Settings::instance()->searchRKEnabled()) {
                TBkeywords << TB->rk().split(KEYWORD_SEPARATOR, Qt::SkipEmptyParts);
            }
            if (Settings::instance()->searchTechPubEnabled()) {
                TBkeywords << TB->techpub().split(KEYWORD_SEPARATOR, Qt::SkipEmptyParts);
            }
            if (Settings::instance()->searchReleaseDateEnabled()) {
                TBkeywords << TB->releaseDate().toString().split(KEYWORD_SEPARATOR, Qt::SkipEmptyParts);
            }
            if (Settings::instance()->searchRegisteredByEnabled()) {
                TBkeywords << TB->registeredBy().split(KEYWORD_SEPARATOR, Qt::SkipEmptyParts);
            }
            if (Settings::instance()->searchReplacesEnabled()) {
                TBkeywords << TB->replaces().split(KEYWORD_SEPARATOR, Qt::SkipEmptyParts);
            }
            if (Settings::instance()->searchReplacedByEnabled()) {
                TBkeywords << TB->replacedBy().split(KEYWORD_SEPARATOR, Qt::SkipEmptyParts);
            }
            if (Settings::instance()->searchCommentEnabled()) {
                TBkeywords << TB->comment().split(KEYWORD_SEPARATOR, Qt::SkipEmptyParts);
            }

            // Default: current row must be shown
            bool Hidden = false;

            // Only exact matches are allowed
            if (Settings::instance()->wholeWordsOnlyEnabled()) {
                for (int j = 0; j < Keywords.count(); j++) {
                    if (!TBkeywords.contains(Keywords[j], Qt::CaseInsensitive)) {
                        Hidden = true;
                        break;
                    }
                }
            }

            // Partial matches are allowed
            else {
                int Match = 0;
                for (int j = 0; j < Keywords.count(); j++) {
                    for (int k = 0; k < TBkeywords.count(); k++) {
                        if (TBkeywords.at(k).contains(Keywords[j], Qt::CaseInsensitive)) {
                            Match++;
                            break;
                        }
                    }
                }
                Hidden = Match < Keywords.count();
            }
            ui->TableTB->setRowHidden(i, Hidden);
        }
    }
    ui->StatusBar->clearMessage();
}

//  addTB
//
// Add a TB at the bottom of the table
//
void MainWindow::addTB(TechnicalBulletin* tb, bool PerformAddChecks)
{
    if (PerformAddChecks) {
        for (int i = 0; i < ui->TableTB->rowCount(); i++) {
            TechnicalBulletin* CurrentTB = ui->TableTB->item(i, COLUMN_METADATA)->data(TB_ROLE).value<TechnicalBulletin*>();

            // Check that the TB doesn't exist yet
            // Don't allow to add twice the same TB
            if (tb->number() == CurrentTB->number()) {
                QMessageBox::critical(this, tr("Error"), tr("TB %1 already exists in the database").arg(tb->title()), QMessageBox::Ok);
                return;
            }

            // Check for newer TB
            // Don't allow to add an old TB
            QString ReplacedBy = tb->replacedBy();
            if ((!ReplacedBy.isNull()) && (ReplacedBy == CurrentTB->number())) {
                QMessageBox::critical(this, tr("Error"), tr("A new version of TB %1 already exists in the database").arg(CurrentTB->title()), QMessageBox::Ok);
                return;
            }

            // Check for older TB
            // Don't allow to keep old TB
            // Offer to merge keywords
            if (tb->replaces() == CurrentTB->number()) {
                QMessageBox* MessageBox = new QMessageBox(QMessageBox::Question,
                                                          tr("Replace previous TB"),
                                                          tr("An older version of TB %1 is present. Do you want to update it?").arg(CurrentTB->title()));
                MessageBox->addButton(tr("Update old TB"), QMessageBox::AcceptRole);
                QPushButton* ButtonMerge  = MessageBox->addButton(tr("Update old TB and merge keywords"), QMessageBox::YesRole);
                QPushButton* ButtonCancel = MessageBox->addButton(tr("Cancel"), QMessageBox::RejectRole);
                MessageBox->setDefaultButton(ButtonMerge);

                MessageBox->exec();
                QAbstractButton* ClickedButton = MessageBox->clickedButton();
                delete MessageBox;

                // Nothing to do if the user cancelled the dialog
                if (ClickedButton == ButtonCancel) {
                    return;
                }

                // Merge keywords
                if (ClickedButton == ButtonMerge) {
                    QList<QString> NewKeywords = tb->keywords();
                    QList<QString> OldKeywords = CurrentTB->keywords();
                    for (int j = 0; j < OldKeywords.count(); j++) {
                        if (!NewKeywords.contains(OldKeywords.at(j))) {
                            NewKeywords << OldKeywords.at(j);
                        }
                    }
                    tb->setKeywords(NewKeywords);
                }

                // Delete old TB
                delete CurrentTB;
                ui->TableTB->removeRow(i);

                // Finally, quit the loop to add the new TB
                this->Modified = true;
                break;
            }
        }
    }

    // Disable table sorting to prevent a null ptr dereferencing
    ui->TableTB->setSortingEnabled(false);

    // Update table size
    int RowCount = ui->TableTB->rowCount();
    ui->TableTB->setRowCount(RowCount + 1);

    // Populate the new line with empty items
    for (int i = 0; i < ui->TableTB->columnCount(); i++) {
        ui->TableTB->setItem(RowCount, i, new QTableWidgetItem);
    }

    // Save an item ptr to make the last entry become the current one
    QTableWidgetItem* Item = ui->TableTB->item(RowCount, 0);

    // Display new TB in the new line
    updateTB(tb, RowCount);

    // Re-enable table sorting, and set the TB as the current one
    ui->TableTB->setSortingEnabled(true);
    ui->TableTB->setCurrentItem(Item);
    ui->TableTB->scrollToItem(Item);
}

//  updateTB
//
// Update the displayed data of an existing TB
//
void MainWindow::updateTB(TechnicalBulletin* tb, int row)
{
    // Set text corresponding to each TB data
    ui->TableTB->item(row, COLUMN_NUMBER)->setText(tb->number());
    ui->TableTB->item(row, COLUMN_TITLE)->setText(tb->title());
    ui->TableTB->item(row, COLUMN_CATEGORY)->setText(tb->category());
    ui->TableTB->item(row, COLUMN_RK)->setText(tb->rk());
    ui->TableTB->item(row, COLUMN_TECH_PUB)->setText(tb->techpub());
    ui->TableTB->item(row, COLUMN_RELEASE_DATE)->setText(tb->releaseDate().toString("yyyy/MM/dd"));
    ui->TableTB->item(row, COLUMN_REGISTERED_BY)->setText(tb->registeredBy());
    ui->TableTB->item(row, COLUMN_REPLACES)->setText(tb->replaces());
    ui->TableTB->item(row, COLUMN_REPLACED_BY)->setText(tb->replacedBy());
    ui->TableTB->item(row, COLUMN_KEYWORDS)->setText(tb->keywordsString());

    // Save tb ptr in the corresponding column and update UI
    ui->TableTB->item(row, COLUMN_METADATA)->setData(TB_ROLE, QVariant::fromValue(tb));
}

//  dragEnterEvent
//
// Allow to drop data if data type can be handled
//
void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat("text/plain")) {
        event->acceptProposedAction();
    }
}

//  dropEvent
//
// Handle dropped data. It should be a mail content
//
void MainWindow::dropEvent(QDropEvent* event)
{
    TechnicalBulletin* TB = DlgTB::newDlgTB(this, event->mimeData()->data("text/plain"));
    if (TB != nullptr) {
        this->Modified = true;
        addTB(TB, PERFORM_ADD_CHECKS);
        updateUI();
    }
}

//  paste
//
// Accept TB copy/pasted from mails
//
void MainWindow::paste()
{
    const QClipboard* Clipboard = QApplication::clipboard();
    if (Clipboard->mimeData()->hasFormat("text/plain")) {
        TechnicalBulletin* TB = DlgTB::newDlgTB(this, Clipboard->mimeData()->data("text/plain"));
        if (TB != nullptr) {
            this->Modified = true;
            addTB(TB, PERFORM_ADD_CHECKS);
            updateUI();
        }
    }
}

//  replaceExistent
//
// Return true if an older TB exits in the database
//
bool MainWindow::replaceExistent(TechnicalBulletin* tb)
{
    QString CurrentNumber = tb->replaces();
    if (!CurrentNumber.isNull()) {
        for (int i = 0; i < ui->TableTB->rowCount(); i++) {
            if (CurrentNumber == ui->TableTB->item(i, COLUMN_METADATA)->data(TB_ROLE).value<TechnicalBulletin*>()->number()) {
                return true;
            }
        }
    }
    return false;
}

//  closeEvent
//
// Prevent the program from closing with modified data
//
void MainWindow::closeEvent(QCloseEvent* event)
{
    if (this->Modified) {
        QMessageBox::StandardButtons Answer
          = QMessageBox::question(this, WINDOW_TITLE, tr("Do you want to save changes before exiting?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        // User wants to save index
        if (Answer == QMessageBox::Yes) {
            save();
        }

        // User wants to cancel closing process
        if (Answer == QMessageBox::Cancel) {
            event->ignore();
        }

        // User wants to close without saving changes
        else {
            event->accept();
        }
    }
}

void MainWindow::copyURLToClipboard()
{
    QList<QTableWidgetItem*> Selection = ui->TableTB->selectedItems();
    int                      Row       = Selection.at(0)->row();
    TechnicalBulletin*       TB        = ui->TableTB->item(Row, COLUMN_METADATA)->data(TB_ROLE).value<TechnicalBulletin*>();
    QGuiApplication::clipboard()->setText(Settings::instance()->baseURLTechnicalBulletin().arg(TB->number()));
}

void MainWindow::openURL()
{
    QList<QTableWidgetItem*> Selection = ui->TableTB->selectedItems();
    int                      Row       = Selection.at(0)->row();
    TechnicalBulletin*       TB        = ui->TableTB->item(Row, COLUMN_METADATA)->data(TB_ROLE).value<TechnicalBulletin*>();
    QDesktopServices::openUrl(QString(Settings::instance()->baseURLTechnicalBulletin()).arg(TB->number()));
}

void MainWindow::downloadRM()
{
    QList<QTableWidgetItem*> Selection = ui->TableTB->selectedItems();
    int                      Row       = Selection.at(0)->row();
    TechnicalBulletin*       TB        = ui->TableTB->item(Row, COLUMN_METADATA)->data(TB_ROLE).value<TechnicalBulletin*>();
    QDesktopServices::openUrl(QString(Settings::instance()->baseURLTechnicalPublication()).arg(TB->techpub()));
}

void MainWindow::openDBv0(int count, QDataStream& stream, bool ForceDBCheck)
{
    for (int i = 0; i < count; i++) {
        // Read a TB and add it to UI
        TechnicalBulletin* TB = new TechnicalBulletin;
        stream >> TB;
        addTB(TB, ForceDBCheck);

        // Check stream status
        if (stream.status() != QDataStream::Ok) {
            QMessageBox::critical(this, WINDOW_TITLE, tr("Unable to read file %1").arg(TBI_FILENAME));
            break;
        }
    }
}

void MainWindow::openDBv1(QDataStream& stream, bool ForceDBCheck)
{
    // Read the number of TB
    qint32 Count;
    stream >> Count;

    for (int i = 0; i < Count; i++) {
        // Read a TB and add it to UI
        TechnicalBulletin* TB = new TechnicalBulletin;
        stream >> TB;
        addTB(TB, ForceDBCheck);

        // Check stream status
        if (stream.status() != QDataStream::Ok) {
            QMessageBox::critical(this, WINDOW_TITLE, tr("Unable to read file %1").arg(TBI_FILENAME));
            break;
        }
    }
}
