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

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "../Index/TechnicalBulletin.hpp" // Probably to be removed after the data handling revamping?
#include "../Index/ThreadIndex.hpp"
#include "ContextMenuAction.hpp"
#include "DownloadMenu.hpp"
#include <QByteArray>
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QLabel>
#include <QMainWindow>
#include <QString>
#include <QStringList>

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

//  MainWindow
//
// This class handles the main window of the application
//
class MainWindow: public QMainWindow
{
    Q_OBJECT

  public:
    MainWindow(bool ForceIndexCheck);
    ~MainWindow() override;
    bool tbNumberAlreadyExists(TechnicalBulletin* tb); // To be removed when DlgTB requests the Index directly

  private:
    Ui::MainWindow* ui;
    ThreadIndex*    Index;
    bool            SaveInProgress;

    // Status bar
    QLabel* MessageTBCount;
    QLabel* MessagePendingModifications;

    // Context menu
    QMenu*             TableContextMenu;
    ContextMenuAction* ActionNewTB;
    ContextMenuAction* ActionEditTB;
    ContextMenuAction* ActionDeleteTB;
    ContextMenuAction* ActionCopyUrl;
    ContextMenuAction* ActionOpenUrl;
    ContextMenuAction* ActionDownload;
    ContextMenuAction* ActionSettings;
    ContextMenuAction* ActionHelp;

    // Download sub-menu
    DownloadMenu* DLMenu;

    // TBs
    void populateUI();
    void updateUI();
    void newTB();
    void editTB();
    void deleteTB();
    void search(bool ForceNewSearch = false);
    void addTB(TechnicalBulletin* tb, bool PerformAddChecks = false);
    void updateTB(TechnicalBulletin* tb, int row);

    // Drag & drop stuff
    void dragEnterEvent(QDragEnterEvent* event) override;
    //    void dropEvent(QDropEvent* event) override;

    // Paste TB from mail to UI
    void paste();

    // Close handling (prevent from closing with unsaved data)
    //    void closeEvent(QCloseEvent* event) override;

    // URL handling
    void copyURLToClipboard();
    void openURL();

    // Central stack toggling
    void toggleStackCentral();

    // Log widget
    void  addLogEntry(QString text);
    void  addLogText(QString text);
    void  startLogTimer();
    void  addLogTimer();
    bool  FirstLogEntry;
    bool  TBreadFirst;
    QTime LogTimer;

    // Signals received from ThreadIndex
    void openingIndex(qint32 version, qint32 count);
    void tbRead(int count);
    void indexOpenedSuccessfully(qint32 count);
    void noIndexFound();
    void failedToOpenIndex();
    void invalidIndexIdentifier(QString magic);
    void indexTooRecent(qint32 version);
    void indexReadingFailed(int count);
    void openingComplete();
    void saveComplete(int result);

    // Signals emitted to ThreadIndex
  signals:
    void save(bool backup);
};

// Table header index
typedef enum {
    COLUMN_NUMBER,
    COLUMN_TITLE,
    COLUMN_CATEGORY,
    COLUMN_RK,
    COLUMN_TECH_PUB,
    COLUMN_RELEASE_DATE,
    COLUMN_REGISTERED_BY,
    COLUMN_REPLACES,
    COLUMN_REPLACED_BY,
    COLUMN_KEYWORDS
} COLUMN_INDEX;

// Role number of TB data ptr in TB table
#define TB_ROLE         Qt::UserRole
#define COLUMN_METADATA COLUMN_NUMBER

// Search option
#define FORCE_SEARCH true

// Enable consistency and update checks when adding a TB
#define PERFORM_ADD_CHECKS true

#endif // MAINWINDOW_HPP
