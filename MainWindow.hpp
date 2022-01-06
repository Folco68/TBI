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

#include "ContextMenuAction.hpp"
#include "TechnicalBulletin.hpp"
#include <QByteArray>
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QLabel>
#include <QMainWindow>
#include <QString>
#include <QStringList>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    MainWindow(bool ForceDBCheck);
    ~MainWindow() override;

  private:
    Ui::MainWindow* ui;
    bool Modified;

    // Status bar
    QLabel* MessageTBCount;
    QLabel* MessagePendingModifications;

    // Context menu
    QMenu* TableContextMenu;
    ContextMenuAction* ActionNewTB;
    ContextMenuAction* ActionEditTB;
    ContextMenuAction* ActionDeleteTB;
    ContextMenuAction* ActionCopyUrl;
    ContextMenuAction* ActionOpenUrl;
    ContextMenuAction* ActionDownloadRM;
    ContextMenuAction* ActionSettings;
    ContextMenuAction* ActionHelp;

    // TBs
    void updateUI();
    void newTB();
    void editTB();
    void deleteTB();
    void save();
    void search(bool ForceNewSearch = false);
    void addTB(TechnicalBulletin* tb, bool PerformAddChecks = false);
    void updateTB(TechnicalBulletin* tb, int row);

    // Drag & drop stuff
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

    // Paste TB from mail to UI
    void paste();

    // Close handling (prevent from closing with unsaved data)
    void closeEvent(QCloseEvent* event) override;

    // URL handling
    void copyURLToClipboard();
    void openURL();
    void downloadRM();
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
#define TB_ROLE Qt::UserRole
#define COLUMN_METADATA COLUMN_NUMBER

// Search option
#define FORCE_SEARCH true

// Enable consistency and update checks when adding a TB
#define PERFORM_ADD_CHECKS true

#endif // MAINWINDOW_HPP
