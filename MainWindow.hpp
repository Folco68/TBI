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

#include "TechnicalBulletin.hpp"
#include <QAction>
#include <QByteArray>
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QLabel>
#include <QMainWindow>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

  private:
    Ui::MainWindow* ui;
    bool            Modified;
    QLabel*         MessageTBCount;
    QLabel*         MessagePendingModifications;
    QMenu*          TableContextMenu;
    QAction*        ActionNewTB;
    QAction*        ActionEditTB;
    QAction*        ActionDeleteTB;
    QAction*        ActionCopyUrl;
    QAction*        ActionOpenUrl;

    // TBs
    void updateUI();
    void newTB();
    void editTB();
    void deleteTB();
    void save();
    void addTB(TechnicalBulletin* tb);
    void updateTB(TechnicalBulletin* tb, int row);

    // Drag & drop stuff
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

    // Close handling (prevent from closing with unsaved data
    void closeEvent(QCloseEvent* event) override;

    // URL handling
    void copyURLToClipboard();
    void openURL();
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

// Role number of TB data ptr in BT table
#define TB_ROLE Qt::UserRole
#define COLUMN_METADATA COLUMN_NUMBER

// Data filename
#define TBI_FILENAME "index.tbi"

#endif // MAINWINDOW_HPP
