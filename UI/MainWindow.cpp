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

#include "DlgHelp.hpp"
#include "DlgSettings.hpp"
#include "DlgTB.hpp"
#include "DownloadMenu.hpp"
#include "Event/EventDeleteTB.hpp"
#include "Event/EventOpenIndex.hpp"
#include "Event/EventSave.hpp"
#include "Global.hpp"
#include "Index/Index.hpp"
#include "Logger.hpp"
#include "MainWindow.hpp"
#include "Settings.hpp"
#include "ui_MainWindow.h"
#include <QAbstractButton>
#include <QAbstractScrollArea>
#include <QClipboard>
#include <QCoreApplication>
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
#include <QTimer>

MainWindow::MainWindow(bool ForceIndexCheck)
    : QMainWindow()
    , ui(new Ui::MainWindow)
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
    , ThreadIndex(new QThread)
    , Modified(false)
{
    // Logger. Establish the connection after the first log because UI is not set up yet
    Logger::instance()->newEntry(tr("TBI starting..."));
    connect(Logger::instance(), &Logger::textAdded, [this](QString text) { ui->TextEditLog->setPlainText(text); });

    //------------------------------------------------------------------------------------
    //                                       Window
    //------------------------------------------------------------------------------------
    ui->setupUi(this);
    setMinimumSize(MAIN_MINIMUM_WIDTH, MAIN_MINIMUM_HEIGHT);
    resize(Settings::instance()->mainWindowSize());
    ui->StackCentral->setCurrentIndex(PAGE_LOG);

    //------------------------------------------------------------------------------------
    //                                     Status bar
    //------------------------------------------------------------------------------------
    ui->StatusBar->addPermanentWidget(this->MessageTBCount);
    ui->StatusBar->addPermanentWidget(this->MessagePendingModifications);

    //------------------------------------------------------------------------------------
    //                             Technical bulletins table
    //------------------------------------------------------------------------------------
    ui->TableTB->setShowGrid(true);
    ui->TableTB->setSortingEnabled(true);
    ui->TableTB->setAlternatingRowColors(true);
    ui->TableTB->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->TableTB->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->TableTB->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->TableTB->verticalHeader()->setVisible(false);
    ui->TableTB->horizontalHeader()->setStretchLastSection(true);

    //------------------------------------------------------------------------------------
    //                                 Table context menu
    //------------------------------------------------------------------------------------
    ui->TableTB->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->TableTB, &QWidget::customContextMenuRequested, [this]() { this->TableContextMenu->exec(QCursor::pos()); });
    connect(this->ActionNewTB, &QAction::triggered, [this]() { newTB(); });
    connect(this->ActionEditTB, &QAction::triggered, [this]() { editTB(); });
    connect(this->ActionDeleteTB, &QAction::triggered, [this]() { deleteTB(); });
    connect(this->ActionCopyUrl, &QAction::triggered, [this]() { copyURLToClipboard(); });
    connect(this->ActionOpenUrl, &QAction::triggered, [this]() { openURL(); });
    connect(this->ActionSettings, &QAction::triggered, [this]() {
        if (DlgSettings::showDlgSettings()) {
            search(FORCE_SEARCH);
        }
        updateUI();
    });
    connect(this->ActionHelp, &QAction::triggered, []() { DlgHelp::showDlgHelp(); });

    //------------------------------------------------------------------------------------
    //   Add actions to the context menu and to the main window to allow kbd shortcuts
    //------------------------------------------------------------------------------------
    QList<QAction*> Actions;
    Actions << this->ActionNewTB << this->ActionEditTB << this->ActionDeleteTB << this->ActionCopyUrl << this->ActionOpenUrl
            << this->ActionDownload << this->ActionSettings << this->ActionHelp;
    this->TableContextMenu->addActions(Actions);
    this->TableContextMenu->insertSeparator(this->ActionCopyUrl);
    this->TableContextMenu->insertSeparator(this->ActionSettings);
    this->addActions(Actions);

    //------------------------------------------------------------------------------------
    //                                   Paste shortcut
    //------------------------------------------------------------------------------------
    connect(new QShortcut(QKeySequence(QKeySequence::Paste), this), &QShortcut::activated, [this]() { paste(); });

    //------------------------------------------------------------------------------------
    //                                   Save shortcut
    //------------------------------------------------------------------------------------
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_S), this), &QShortcut::activated, [this]() { save(); });

    //------------------------------------------------------------------------------------
    //               Search shortcut. Toggle between search field and table
    //------------------------------------------------------------------------------------
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this), &QShortcut::activated, [this]() {
        if (ui->EditKeywords->hasFocus()) {
            ui->TableTB->setFocus();
        }
        else {
            ui->EditKeywords->setFocus();
        }
    });

    //------------------------------------------------------------------------------------
    //                             Table/Log toggle shortcut
    //------------------------------------------------------------------------------------
    connect(new QShortcut(QKeySequence(Qt::Key_F5), this), &QShortcut::activated, [this]() {
        ui->StackCentral->setCurrentIndex(ui->StackCentral->currentIndex() ^ 1);
    });

    //------------------------------------------------------------------------------------
    //                                Buttons connections
    //------------------------------------------------------------------------------------
    connect(ui->ButtonSave, &QPushButton::clicked, [this]() { save(); });
    connect(ui->ButtonSearch, &QPushButton::clicked, [this]() { search(); });

    //------------------------------------------------------------------------------------
    //                                 Search connection
    //------------------------------------------------------------------------------------
    connect(ui->EditKeywords, &QLineEdit::returnPressed, [this]() { search(); });
    connect(ui->EditKeywords, &QLineEdit::textChanged, [this]() {
        if (ui->EditKeywords->text().isEmpty() || Settings::instance()->realTimeSearchEnabled())
            search();
    });

    //------------------------------------------------------------------------------------
    //                                 Table connections
    //------------------------------------------------------------------------------------
    connect(ui->TableTB, &QTableWidget::itemSelectionChanged, [this]() { updateUI(); });
    connect(ui->TableTB, &QTableWidget::cellDoubleClicked, [this]() {
        editTB();
        updateUI();
    });

    // Make UI consistent
    updateUI();

    // Adjust column size
    for (int i = 0; i < ui->TableTB->columnCount() - 1; i++) {
        ui->TableTB->resizeColumnToContents(i);
    }

    //------------------------------------------------------------------------------------
    //
    //                         Initialization of the index thread
    //
    //------------------------------------------------------------------------------------

    //------------------------------------------------------------------------------------
    //                      Execute the index in the separate thread
    //------------------------------------------------------------------------------------
    Index::instance()->moveToThread(this->ThreadIndex);

    //-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
    //    Send the OpenIndex event when both the UI and the Index threads are running
    //-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -

    connect(this->ThreadIndex, &QThread::started, [ForceIndexCheck]() {
        QCoreApplication::postEvent(Index::instance(), new EventOpenIndex(ForceIndexCheck));
    });

    //------------------------------------------------------------------------------------
    //                Start the index thread at first event loop of the UI
    //------------------------------------------------------------------------------------
    QTimer::singleShot(0, [this]() { this->ThreadIndex->start(); });

    /*******************************************************************************************************************
     *                                                                                                                 *
     *                                               Thread connections                                                *
     *                                                                                                                 *
     ******************************************************************************************************************/

    connect(this->ThreadIndex, &QThread::finished, this->ThreadIndex, &QThread::deleteLater);
    connect(Index::instance(), &Index::openingStarting, this, &MainWindow::openingStarting, Qt::QueuedConnection);
    connect(Index::instance(), &Index::openingHeader, this, &MainWindow::openingHeader, Qt::QueuedConnection);
    connect(Index::instance(), &Index::openingProgress, this, &MainWindow::openingProgress, Qt::QueuedConnection);
    connect(Index::instance(), &Index::openingSuccessful, this, &MainWindow::openingSuccessful, Qt::QueuedConnection);
    connect(Index::instance(), &Index::noIndexFound, this, &MainWindow::noIndexFound, Qt::QueuedConnection);
    connect(Index::instance(), &Index::indexTooRecent, this, &MainWindow::indexTooRecent, Qt::QueuedConnection);
    connect(Index::instance(), &Index::invalidMagic, this, &MainWindow::invalidMagic, Qt::QueuedConnection);
    connect(Index::instance(), &Index::failedToOpenIndex, this, &MainWindow::failedToOpenIndex, Qt::QueuedConnection);
    connect(Index::instance(), &Index::failedToReadFileContent, this, &MainWindow::failedToReadFileContent, Qt::QueuedConnection);
    connect(Index::instance(), &Index::openingFailed, this, &MainWindow::openingFailed, Qt::QueuedConnection);
    connect(Index::instance(), &Index::failedToCreateBackup, this, &MainWindow::failedToCreateBackup, Qt::QueuedConnection);
    connect(Index::instance(), &Index::failedToOpenFileForSaving, this, &MainWindow::failedToOpenFileForSaving, Qt::QueuedConnection);
    connect(Index::instance(), &Index::failedToWriteContent, this, &MainWindow::failedToWriteContent, Qt::QueuedConnection);
    connect(Index::instance(), &Index::savingSuccessful, this, &MainWindow::savingSuccessful, Qt::QueuedConnection);
    connect(Index::instance(), &Index::failedToDeleteTB, this, &MainWindow::failedToDeleteTB, Qt::QueuedConnection);
    connect(Index::instance(), &Index::tbDeletionSuccessful, this, &MainWindow::tbDeletionSuccessful, Qt::QueuedConnection);
}

MainWindow::~MainWindow()
{
    // Index thread termination. deleteLater() is called when the finished signal is emitted
    this->ThreadIndex->quit();

    // Destroy singletons here and not in main(),
    // because QGuiApplication doesn't return on all platforms
    Index::release();
    Logger::release();
    Settings::release();

    // Save window size
    Settings::instance()->setMainWindowSize(size());

    // UI
    delete this->DLMenu;
    delete ui;
}

/***********************************************************************************************************************
 *                                                                                                                     *
 *                                                       Window                                                        *
 *                                                                                                                     *
 **********************************************************************************************************************/

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
    this->MessageTBCount->setText(tr("%1 Technical Bulletin%2 registered").arg(Count).arg(Plural));
    this->MessagePendingModifications->setText(this->Modified ? tr("Modifications pending") : tr("Index is saved"));

    // Actions (context menu)
    bool ItemSelected = !ui->TableTB->selectedItems().isEmpty();
    this->ActionEditTB->setEnabled(ItemSelected);
    this->ActionDeleteTB->setEnabled(ItemSelected);
    this->ActionCopyUrl->setEnabled(ItemSelected);
    this->ActionOpenUrl->setEnabled(ItemSelected);

    // Download action and sub-menu
    if (ItemSelected) {
        int     Row           = ui->TableTB->currentRow();
        QString DocsField     = ui->TableTB->item(Row, COLUMN_TECH_PUB)->text();
        QString TBnumberField = ui->TableTB->item(Row, COLUMN_NUMBER)->text().trimmed();
        this->DLMenu->setItems(DocsField, TBnumberField);
        this->ActionDownload->setMenu(this->DLMenu);
        this->ActionDownload->setDisabled(this->DLMenu->isEmpty());
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (this->Modified) {
        QMessageBox::StandardButtons Answer = QMessageBox::question(this,
                                                                    WINDOW_TITLE,
                                                                    tr("Do you want to save changes before exiting?"),
                                                                    QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
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

/***********************************************************************************************************************
 *                                                                                                                     *
 *                                                    Index opening                                                    *
 *                                                                                                                     *
 **********************************************************************************************************************/

void MainWindow::openingStarting()
{
    QString Message = tr("Trying to open index file: %1").arg(TBI_FILENAME);
    Logger::instance()->newEntry(Message);
}

void MainWindow::openingHeader(int version, qint32 count)
{
    QString Message = tr("Opening index version %1, containing %2 technical bulletins").arg(version).arg(count);
    Logger::instance()->newEntry(Message);
    Logger::instance()->newEntry(tr("Reading technical bulletins: "));
    Logger::instance()->startTimer();
}

void MainWindow::openingProgress(int count)
{
    Logger::instance()->append(QString("%1... ").arg(count));
}

void MainWindow::openingSuccessful(int count)
{
    QString Message = tr("Opening successful. %1 technical bulletins read in %2").arg(count).arg(Logger::instance()->elapsedTime());
    Logger::instance()->newEntry(Message);
    fillTBtable();
    ui->StackCentral->setCurrentIndex(PAGE_TABLE);
}

void MainWindow::noIndexFound()
{
    Logger::instance()->newEntry(tr("No index file found"));
    ui->StackCentral->setCurrentIndex(PAGE_TABLE);
}

void MainWindow::indexTooRecent(qint32 version)
{
    QString Message = tr("Index version is too recent (%1), please update your program").arg(version);
    Logger::instance()->newEntry(Message);
}

void MainWindow::invalidMagic(QString magic)
{
    QString Message = tr("File corrupted or invalid. Magic found is: %1").arg(magic);
    Logger::instance()->newEntry(Message);
}

void MainWindow::failedToOpenIndex()
{
    QString Message = tr("Impossible to open the file '%1'").arg(TBI_FILENAME);
    Logger::instance()->newEntry(Message);
}

void MainWindow::failedToReadFileContent()
{
    Logger::instance()->newEntry(tr("Failed to read file content"));
}

void MainWindow::openingFailed(int count)
{
    QString Message = tr("Failed to fully open the index file. Nevetheless %1 technical bulletins could be opened").arg(count);
    Logger::instance()->newEntry(Message);
    fillTBtable();
}

void MainWindow::fillTBtable()
{
    Logger::instance()->newEntry(tr("Filling technical bulletins table..."));
    QCoreApplication::processEvents(); // Force the refresh of the log display
    Logger::instance()->startTimer();
    QList<TechnicalBulletin*> Bulletins = Index::instance()->bulletins();

    // Set the table size
    ui->TableTB->setRowCount(Bulletins.count());

    // Create a QTableWidgetItem in every cell
    // Once a line is completed, populate it with a TB
    for (int i = 0; i < Bulletins.count(); i++) {
        for (int j = 0; j < ui->TableTB->columnCount(); j++) {
            ui->TableTB->setItem(i, j, new QTableWidgetItem);
        }
        updateTB(Bulletins.at(i), i);
    }

    updateUI();

    // Resize columns
    for (int i = 0; i < ui->TableTB->columnCount() - 1; i++) {
        ui->TableTB->resizeColumnToContents(i);
    }

    QString Message = tr("Table filled in %1").arg(Logger::instance()->elapsedTime());
    Logger::instance()->newEntry(Message);
}

/***********************************************************************************************************************
 *                                                                                                                     *
 *                                                    Index saving                                                     *
 *                                                                                                                     *
 **********************************************************************************************************************/

void MainWindow::save()
{
    QCoreApplication::postEvent(Index::instance(), new EventSave);
}

void MainWindow::failedToCreateBackup()
{
    Logger::instance()->newEntry(tr("Unable to create the backup file before saving"));
    if (QMessageBox::critical(this,
                              tr("Save error"),
                              tr("Failed to create a backup file when saving the index. Save without backup?"),
                              QMessageBox::Yes | QMessageBox::No)
        == QMessageBox::Yes) {
        QCoreApplication::postEvent(Index::instance(), new EventSave(SAVE_WITHOUT_BACKUP));
    }
}

void MainWindow::failedToOpenFileForSaving()
{
    Logger::instance()->newEntry(tr("Unable to open the index file to write into"));
    if (QMessageBox::critical(this, tr("Save error"), tr("Failed to open the index file to save technical bulletins. Retry?"), QMessageBox::Yes | QMessageBox::No)
        == QMessageBox::Yes) {
        QCoreApplication::postEvent(Index::instance(), new EventSave);
    }
}

void MainWindow::failedToWriteContent(int count)
{
    Logger::instance()->newEntry(tr("Writing failure while saving the index file. %1 TB saved"));
    if (QMessageBox::critical(this,
                              tr("Save error"),
                              tr("Failed to fully save index file. Nevertheless %1 technical bulletins were saved. Retry?").arg(count),
                              QMessageBox::Yes | QMessageBox::No)
        == QMessageBox::Yes) {
        QCoreApplication::postEvent(Index::instance(), new EventSave);
    }
}

void MainWindow::savingSuccessful(int count)
{
    Logger::instance()->newEntry(tr("Saving successful, index contains %1 technical bulletins").arg(count));
    this->Modified = false;
    updateUI();
}

/***********************************************************************************************************************
 *                                                                                                                     *
 *                                                 New / Edit / Delete                                                 *
 *                                                                                                                     *
 **********************************************************************************************************************/

void MainWindow::newTB()
{
    TechnicalBulletin* TB = DlgTB::newDlgTB(this);
    if (TB != nullptr) {
        this->Modified = true;
        addTB(TB, PERFORM_ADD_CHECKS);
        updateUI();
    }
}

void MainWindow::editTB()
{
    TechnicalBulletin* TB = currentTB();
    if (DlgTB::editDlgTB(this, TB)) {
        this->Modified = true;
        updateTB(TB, ui->TableTB->currentRow());
    }
}

void MainWindow::deleteTB()
{
    TechnicalBulletin* TB = currentTB();
    if (QMessageBox::question(this, WINDOW_TITLE, tr("Do you want to delete Technical Bulletin %1 (%2)?").arg(TB->number(), TB->title())) == QMessageBox::Yes) {
        this->Modified = true;
        ui->TableTB->removeRow(ui->TableTB->currentRow());
        QCoreApplication::postEvent(Index::instance(), new EventDeleteTB(TB));
    }
}

void MainWindow::failedToDeleteTB(QString number, QString title)
{
    Logger::instance()->newEntry(tr("Failed to delete technical bulletin %1 (%2)").arg(number).arg(title));
}

void MainWindow::tbDeletionSuccessful(QString number, QString title)
{
    Logger::instance()->newEntry(tr("Technical bulletin %1 (%2) deleted successfully").arg(number).arg(title));
}

/***********************************************************************************************************************
 *                                                                                                                     *
 *                                                       Search                                                        *
 *                                                                                                                     *
 **********************************************************************************************************************/

void MainWindow::search(bool ForceNewSearch)
{
    // Split and clean the list
    static QStringList Keywords;
    QStringList        UIkeywords = ui->EditKeywords->text().split(KEYWORD_SEPARATOR, Qt::SkipEmptyParts);
    UIkeywords.removeDuplicates();

    // Early return if the list didn't change and we don't force a new search
    // ForceNewSearch is used when global settings have been changed
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

/***********************************************************************************************************************
 *                                                                                                                     *
 *                                                 Technical bulletin                                                  *
 *                                                                                                                     *
 **********************************************************************************************************************/

TechnicalBulletin* MainWindow::currentTB() const
{
    QList<QTableWidgetItem*> Selection = ui->TableTB->selectedItems();
    int                      Row       = Selection.at(0)->row();
    TechnicalBulletin*       TB        = ui->TableTB->item(Row, COLUMN_METADATA)->data(TB_ROLE).value<TechnicalBulletin*>();
    return TB;
}

void MainWindow::addTB(TechnicalBulletin* tb, bool PerformAddChecks)
{
    if (PerformAddChecks) {
        for (int i = 0; i < ui->TableTB->rowCount(); i++) {
            TechnicalBulletin* CurrentTB = ui->TableTB->item(i, COLUMN_METADATA)->data(TB_ROLE).value<TechnicalBulletin*>();

            // Check that the TB doesn't exist yet
            // Don't allow to add twice the same TB
            if (tb->number() == CurrentTB->number()) {
                QMessageBox::critical(this,
                                      tr("Error"),
                                      tr("TB %1 already exists in the database").arg(tb->title()),
                                      QMessageBox::Ok);
                return;
            }

            // Check for newer TB
            // Don't allow to add an old TB
            QString ReplacedBy = tb->replacedBy();
            if ((!ReplacedBy.isNull()) && (ReplacedBy == CurrentTB->number())) {
                QMessageBox::critical(this,
                                      tr("Error"),
                                      tr("A new version of TB %1 already exists in the database").arg(CurrentTB->title()),
                                      QMessageBox::Ok);
                return;
            }

            // Check for older TB
            // Don't allow to keep old TB
            // Offer to merge keywords
            if (tb->replaces() == CurrentTB->number()) {
                QMessageBox* MessageBox = new QMessageBox(QMessageBox::Question,
                                                          tr("Replace previous TB"),
                                                          tr("An older version of TB %1 is present. Do you want to update it?")
                                                              .arg(CurrentTB->title()));
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

    // Save tb ptr in the dedicated column
    ui->TableTB->item(row, COLUMN_METADATA)->setData(TB_ROLE, QVariant::fromValue(tb));
}

//  tbNumberAlreadyExists
//
// Return true if an older TB exits in the database
// Used by DlgTB to display a message saying that
// there is already an older version of the TB in the database
//
bool MainWindow::tbNumberAlreadyExists(TechnicalBulletin* tb)
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

/***********************************************************************************************************************
 *                                                                                                                     *
 *                                              Drag & drop, copy & paste                                              *
 *                                                                                                                     *
 **********************************************************************************************************************/

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat("text/plain")) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent* event)
{
    TechnicalBulletin* TB = DlgTB::newDlgTB(this, event->mimeData()->data("text/plain"));
    if (TB != nullptr) {
        this->Modified = true;
        addTB(TB, PERFORM_ADD_CHECKS);
        updateUI();
    }
}

void MainWindow::paste()
{
    const QClipboard* Clipboard = QApplication::clipboard();
    if (Clipboard->mimeData()->hasText()) {
        TechnicalBulletin* TB = DlgTB::newDlgTB(this, Clipboard->mimeData()->data("text/plain"));
        if (TB != nullptr) {
            this->Modified = true;
            addTB(TB, PERFORM_ADD_CHECKS);
            updateUI();
        }
    }
}

/***********************************************************************************************************************
 *                                                                                                                     *
 *                                                    URL handling                                                     *
 *                                                                                                                     *
 **********************************************************************************************************************/

void MainWindow::copyURLToClipboard()
{
    QGuiApplication::clipboard()->setText(Settings::instance()->baseURLTechnicalBulletinWebpage().arg(currentTB()->number()));
}

void MainWindow::openURL()
{
    QDesktopServices::openUrl(QString(Settings::instance()->baseURLTechnicalBulletinWebpage()).arg(currentTB()->number()));
}
