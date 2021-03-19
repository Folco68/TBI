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
#include "DlgHelp.hpp"
#include "DlgSettings.hpp"
#include "DlgTB.hpp"
#include "Global.hpp"
#include "Settings.hpp"
#include "TechnicalBulletin.hpp"
#include "ui_MainWindow.h"
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
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>

MainWindow::MainWindow()
    : QMainWindow()
    , ui(new Ui::MainWindow)
    , Modified(false)
    , MessageTBCount(new QLabel)
    , MessagePendingModifications(new QLabel)
    , TableContextMenu(new QMenu(this))
    , ActionNewTB(new ContextMenuAction(tr("New TB"), this, QKeySequence(Qt::CTRL + Qt::Key_N)))
    , ActionEditTB(new ContextMenuAction(tr("Edit TB"), this, QKeySequence(Qt::CTRL + Qt::Key_E)))
    , ActionDeleteTB(new ContextMenuAction(tr("Delete TB"), this, QKeySequence(Qt::Key_Delete)))
    , ActionCopyUrl(new ContextMenuAction(tr("Copy URL"), this, QKeySequence(Qt::CTRL + Qt::Key_C)))
    , ActionOpenUrl(new ContextMenuAction(tr("Open URL"), this, QKeySequence(Qt::CTRL + Qt::Key_O)))
    , ActionDownloadRM(new ContextMenuAction(tr("Download RM"), this, QKeySequence(Qt::CTRL + Qt::Key_D)))
    , ActionSettings(new ContextMenuAction(tr("Settings"), this))
    , ActionHelp(new ContextMenuAction(tr("Help / About"), this, QKeySequence(Qt::Key_F1)))
{
    // Window
    ui->setupUi(this);
    setWindowTitle(WINDOW_TITLE);
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
    connect(ui->TableTB, &QWidget::customContextMenuRequested, [this]() { this->TableContextMenu->exec(QCursor::pos()); });
    connect(this->ActionNewTB, &QAction::triggered, [this]() { newTB(); });
    connect(this->ActionEditTB, &QAction::triggered, [this]() { editTB(); });
    connect(this->ActionDeleteTB, &QAction::triggered, [this]() { deleteTB(); });
    connect(this->ActionCopyUrl, &QAction::triggered, [this]() { copyURLToClipboard(); });
    connect(this->ActionOpenUrl, &QAction::triggered, [this]() { openURL(); });
    connect(this->ActionDownloadRM, &QAction::triggered, [this]() { downloadRM(); });
    connect(this->ActionSettings, &QAction::triggered, [this]() {
        if (DlgSettings::showDlgSettings()) {
            search(FORCE_SEARCH);
        }
        updateUI();
    });
    connect(this->ActionHelp, &QAction::triggered, []() { DlgHelp::showDlgHelp(); });

    // Add actions to the context menu and to the main window to allow kbd shortcuts
    QList<QAction*> actions;
    actions << this->ActionNewTB << this->ActionEditTB << this->ActionDeleteTB << this->ActionCopyUrl << this->ActionOpenUrl << this->ActionDownloadRM
            << this->ActionSettings << this->ActionHelp;
    this->TableContextMenu->addActions(actions);
    this->TableContextMenu->insertSeparator(this->ActionCopyUrl);
    this->TableContextMenu->insertSeparator(this->ActionSettings);
    this->addActions(actions);

    // Save shortcut
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_S), this), &QShortcut::activated, [this]() {
        if (this->Modified)
            save();
    });

    // Buttons connections
    connect(ui->ButtonSave, &QPushButton::clicked, [this]() { save(); });
    connect(ui->ButtonSearch, &QPushButton::clicked, [this]() { search(); });

    // Search connection
    connect(ui->EditKeywords, &QLineEdit::returnPressed, [this]() { search(); });
    connect(ui->EditKeywords, &QLineEdit::textChanged, [this]() {
        if (ui->EditKeywords->text().isEmpty() || Settings::instance()->realTimeSearchEnabled())
            search();
    });

    // Table connections
    connect(ui->TableTB, &QTableWidget::itemSelectionChanged, [this]() { updateUI(); });
    connect(ui->TableTB, &QTableWidget::cellDoubleClicked, [this]() { editTB(); });

    // Open TBI if one exists
    QFile file(TBI_FILENAME);
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream stream(&file);
        int         count;
        stream >> count;
        for (int i = 0; i < count; i++) {
            // Read a TB and add it to UI
            TechnicalBulletin* tb = new TechnicalBulletin;
            stream >> tb;
            addTB(tb);

            // Check stream status
            if (stream.status() != QDataStream::Ok) {
                QMessageBox::critical(this, WINDOW_TITLE, tr("Unable to read file %1").arg(TBI_FILENAME));
                break;
            }
        }
    }
    // Throw an error if the file exists and couldn't be opened
    else if (QFileInfo::exists(TBI_FILENAME)) {
        QMessageBox::critical(this, WINDOW_TITLE, tr("Unable to open file %1").arg(TBI_FILENAME));
    }

    // Make UI consistent
    updateUI();
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
    delete ui;
}

//  updateUI
//
// Adjust display according to index state
//
void MainWindow::updateUI()
{
    // Shortcut
    bool ItemSelected = !ui->TableTB->selectedItems().isEmpty();

    // Window title
    setWindowTitle(QString("%1 %2").arg(WINDOW_TITLE).arg(this->Modified ? "- (modified)" : ""));

    // Button
    ui->ButtonSave->setEnabled(this->Modified);
    ui->ButtonSearch->setVisible(!Settings::instance()->realTimeSearchEnabled());

    // Status bar
    int count      = ui->TableTB->rowCount();
    QString plural = count > 1 ? "s" : "";
    this->MessageTBCount->setText(tr("%1 Technical Bulletin%2 registered   ").arg(count).arg(plural));
    this->MessagePendingModifications->setText(this->Modified ? tr("Modifications have to be saved") : tr("Index is saved"));

    // Adjust columns size
    for (int i = 0; i < ui->TableTB->columnCount() - 1; i++) {
        ui->TableTB->resizeColumnToContents(i);
    }

    // Actions (context menu)
    this->ActionEditTB->setEnabled(ItemSelected);
    this->ActionDeleteTB->setEnabled(ItemSelected);
    this->ActionCopyUrl->setEnabled(ItemSelected);
    this->ActionOpenUrl->setEnabled(ItemSelected);
    this->ActionDownloadRM->setEnabled(ItemSelected);
}

//  save
//
// Save current TBI
//
void MainWindow::save()
{
    // Try to open the file
    QFile file(TBI_FILENAME);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, WINDOW_TITLE, tr("Couldn't open file %1").arg(TBI_FILENAME));
        return;
    }

    // Open a data stream and write into it
    QDataStream stream(&file);
    stream << ui->TableTB->rowCount();
    for (int i = 0; i < ui->TableTB->rowCount(); i++) {
        // Get TB ptr
        TechnicalBulletin* tb = ui->TableTB->item(i, COLUMN_METADATA)->data(TB_ROLE).value<TechnicalBulletin*>();
        stream << *tb;

        // Check stream status
        if (stream.status() != QDataStream::Ok) {
            QMessageBox::critical(this, WINDOW_TITLE, tr("Failed to save file %1").arg(TBI_FILENAME));
            return;
        }
    }

    this->Modified = false;
    updateUI();
}

//  newTB
//
// Open a dialog allowing to create a TB by hand
//
void MainWindow::newTB()
{
    TechnicalBulletin* tb = DlgTB::newDlgTB(this);
    if (tb != nullptr) {
        this->Modified = true;
        addTB(tb);
    }
}

//  editTB
//
// Open a dialog allowing to edit an existing TB
void MainWindow::editTB()
{
    // Get current TB
    QList<QTableWidgetItem*> items = ui->TableTB->selectedItems();
    int row                        = items.at(0)->row();
    TechnicalBulletin* tb          = ui->TableTB->item(row, COLUMN_METADATA)->data(TB_ROLE).value<TechnicalBulletin*>();

    // Open edition dialog
    if (DlgTB::editDlgTB(this, tb)) {
        this->Modified = true;
        updateTB(tb, row);
    }
}

//  deleteTB
//
// Delete the TB currently selected
//
void MainWindow::deleteTB()
{
    // Get current TB
    QList<QTableWidgetItem*> selection = ui->TableTB->selectedItems();
    int row                            = selection.at(0)->row();
    TechnicalBulletin* tb              = ui->TableTB->item(row, COLUMN_METADATA)->data(TB_ROLE).value<TechnicalBulletin*>();

    // Show a confirmation dialog
    QMessageBox::StandardButton answer
        = QMessageBox::question(this, WINDOW_TITLE, tr("Do you want to delete Technical Bulletin %1 (%2)?").arg(tb->number()).arg(tb->title()));
    if (answer == QMessageBox::Yes) {
        delete tb;
        ui->TableTB->removeRow(row);

        // Update UI
        this->Modified = true;
        updateUI();
    }
}

//  search
//
// Search the TB with keywords
//
void MainWindow::search(bool ForceNewSearch)
{
    // Split and clean the list
    static QStringList keywords;
    QStringList UIkeywords = ui->EditKeywords->text().split(KEYWORD_SEPARATOR, QString::SkipEmptyParts);
    UIkeywords.removeDuplicates();

    // Early return if the list didn't change and we don't force a new search
    if ((UIkeywords == keywords) && !ForceNewSearch) {
        return;
    }
    keywords = UIkeywords;

    // Display status bar message
    ui->StatusBar->showMessage(tr("Searching..."));

    // Display all entries if there is no filter
    if (keywords.count() == 0) {
        // No keyword: display all TBs
        for (int i = 0; i < ui->TableTB->rowCount(); i++) {
            ui->TableTB->setRowHidden(i, false);
        }
    }
    else {
        // Else filter TBs with keywords
        for (int i = 0; i < ui->TableTB->rowCount(); i++) {
            TechnicalBulletin* tb  = ui->TableTB->item(i, COLUMN_METADATA)->data(TB_ROLE).value<TechnicalBulletin*>();
            QStringList tbkeywords = tb->keywords();

            // Add some fields to the keyword list, depending on global configuration
            if (Settings::instance()->searchNumberEnabled()) {
                tbkeywords << tb->number().split(KEYWORD_SEPARATOR, QString::SkipEmptyParts);
            }
            if (Settings::instance()->searchTitleEnabled()) {
                tbkeywords << tb->title().split(KEYWORD_SEPARATOR, QString::SkipEmptyParts);
            }
            if (Settings::instance()->searchCategoryEnabled()) {
                tbkeywords << tb->category().split(KEYWORD_SEPARATOR, QString::SkipEmptyParts);
            }
            if (Settings::instance()->searchRKEnabled()) {
                tbkeywords << tb->rk().split(KEYWORD_SEPARATOR, QString::SkipEmptyParts);
            }
            if (Settings::instance()->searchTechPubEnabled()) {
                tbkeywords << tb->techpub().split(KEYWORD_SEPARATOR, QString::SkipEmptyParts);
            }
            if (Settings::instance()->searchReleaseDateEnabled()) {
                tbkeywords << tb->releaseDate().toString().split(KEYWORD_SEPARATOR, QString::SkipEmptyParts);
            }
            if (Settings::instance()->searchRegisteredByEnabled()) {
                tbkeywords << tb->registeredBy().split(KEYWORD_SEPARATOR, QString::SkipEmptyParts);
            }
            if (Settings::instance()->searchReplacesEnabled()) {
                tbkeywords << tb->replaces().split(KEYWORD_SEPARATOR, QString::SkipEmptyParts);
            }
            if (Settings::instance()->searchReplacedByEnabled()) {
                tbkeywords << tb->replacedBy().split(KEYWORD_SEPARATOR, QString::SkipEmptyParts);
            }
            if (Settings::instance()->searchCommentEnabled()) {
                tbkeywords << tb->comment().split(KEYWORD_SEPARATOR, QString::SkipEmptyParts);
            }

            // Default: current row must be hidden
            bool hidden = false;

            // Only exact matches are allowed
            if (Settings::instance()->wholeWordsOnlyEnabled()) {
                for (int j = 0; j < keywords.count(); j++) {
                    if (!tbkeywords.contains(keywords[j], Qt::CaseInsensitive)) {
                        hidden = true;
                        break;
                    }
                }
            }

            // Partial matches are allowed
            else {
                int match = 0;
                for (int j = 0; j < keywords.count(); j++) {
                    for (int k = 0; k < tbkeywords.count(); k++) {
                        if (tbkeywords.at(k).contains(keywords[j], Qt::CaseInsensitive)) {
                            match++;
                            break;
                        }
                    }
                }
                hidden = match < keywords.count();
            }
            ui->TableTB->setRowHidden(i, hidden);
        }
    }
    ui->StatusBar->clearMessage();
}

//  addTB
//
// Add a TB at the bottom of the table
//
void MainWindow::addTB(TechnicalBulletin* tb)
{
    // Update table size
    int rowcount = ui->TableTB->rowCount();
    ui->TableTB->setRowCount(rowcount + 1);

    // Populate the new line with empry items
    for (int i = 0; i < ui->TableTB->columnCount(); i++) {
        ui->TableTB->setItem(rowcount, i, new QTableWidgetItem);
    }

    // Display new TB in the new line
    updateTB(tb, rowcount);
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
    updateUI();
}

//  dragEnterEvent
//
// Allow to drop data if data type can be handled
//
void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat("text/plain"))
        event->acceptProposedAction();
}

//  dropEvent
//
// Handle dropped data. It should be a mail content
//
void MainWindow::dropEvent(QDropEvent* event)
{
    TechnicalBulletin* tb = DlgTB::newDlgTB(this, event->mimeData()->data("text/plain"));
    if (tb != nullptr) {
        this->Modified = true;
        addTB(tb);
    }
}

//  closeEvent
//
// Prevent the program from closing with modified data
//
void MainWindow::closeEvent(QCloseEvent* event)
{
    if (this->Modified) {
        QMessageBox::StandardButtons answer = QMessageBox::question(
            this, WINDOW_TITLE, tr("Do you want to save changes before exiting?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        // User wants to save index
        if (answer == QMessageBox::Yes) {
            save();
        }

        // User wants to cancel closing process
        if (answer == QMessageBox::Cancel) {
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
    QList<QTableWidgetItem*> selection = ui->TableTB->selectedItems();
    int row                            = selection.at(0)->row();
    TechnicalBulletin* tb              = ui->TableTB->item(row, COLUMN_METADATA)->data(TB_ROLE).value<TechnicalBulletin*>();
    QGuiApplication::clipboard()->setText(Settings::instance()->baseURLTechnicalPublication().arg(tb->number()));
}

void MainWindow::openURL()
{
    QList<QTableWidgetItem*> selection = ui->TableTB->selectedItems();
    int row                            = selection.at(0)->row();
    TechnicalBulletin* tb              = ui->TableTB->item(row, COLUMN_METADATA)->data(TB_ROLE).value<TechnicalBulletin*>();
    QDesktopServices::openUrl(QString(Settings::instance()->baseURLTechnicalPublication()).arg(tb->number()));
}

void MainWindow::downloadRM()
{
    QList<QTableWidgetItem*> selection = ui->TableTB->selectedItems();
    int row                            = selection.at(0)->row();
    TechnicalBulletin* tb              = ui->TableTB->item(row, COLUMN_METADATA)->data(TB_ROLE).value<TechnicalBulletin*>();
    QDesktopServices::openUrl(QString(Settings::instance()->baseURLRebuildingManual()).arg(tb->techpub()));
}
