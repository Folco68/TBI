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
#include "DlgTB.hpp"
#include "Global.hpp"
#include "TechnicalBulletin.hpp"
#include "ui_MainWindow.h"
#include <QAbstractScrollArea>
#include <QDataStream>
#include <QFile>
#include <QFileInfo>
#include <QLineEdit>
#include <QList>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QStatusBar>
#include <QTableWidget>
#include <QTableWidgetItem>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , Modified(false)
    , MessageTBCount(new QLabel)
    , MessagePendingModifications(new QLabel)
{
    ui->setupUi(this);
    setWindowTitle(WINDOW_TITLE);
    setMinimumSize(MAIN_MINIMUM_WIDTH, MAIN_MINIMUM_HEIGHT);
    ui->StatusBar->addWidget(this->MessageTBCount);
    ui->StatusBar->addPermanentWidget(this->MessagePendingModifications);

    // Set table
    ui->TableTB->setShowGrid(true);
    ui->TableTB->setSortingEnabled(true);
    ui->TableTB->setAlternatingRowColors(true);
    ui->TableTB->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->TableTB->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->TableTB->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->TableTB->verticalHeader()->setVisible(false);
    ui->TableTB->horizontalHeader()->setStretchLastSection(true);

    // Connections
    connect(ui->ButtonSave, &QPushButton::clicked, [this]() { save(); });
    connect(ui->ButtonNewTB, &QPushButton::clicked, [this]() { newTB(); });
    connect(ui->ButtonEditTB, &QPushButton::clicked, [this]() { editTB(); });
    connect(ui->ButtonDeleteTB, &QPushButton::clicked, [this]() { deleteTB(); });
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
    else {
        QMessageBox::critical(this, WINDOW_TITLE, tr("Unable to open file %1").arg(TBI_FILENAME));
    }

    // Make UI consistent
    updateUI();
    adjustSize();
}

MainWindow::~MainWindow()
{
    delete ui;
}

//  updateUI
//
// Adjust display according to index state
//
void MainWindow::updateUI()
{
    // Window title
    setWindowTitle(QString("%1 %2").arg(WINDOW_TITLE).arg(this->Modified ? "- (modified)" : ""));

    // Buttons
    ui->ButtonSave->setEnabled(this->Modified);
    ui->ButtonEditTB->setEnabled(!ui->TableTB->selectedItems().isEmpty());
    ui->ButtonDeleteTB->setEnabled(!ui->TableTB->selectedItems().isEmpty());

    // Status bar
    int     count  = ui->TableTB->rowCount();
    QString plural = count > 1 ? "s" : "";
    this->MessageTBCount->setText(tr("%1 Technical Bulletin%2 registered").arg(count).arg(plural));
    this->MessagePendingModifications->setText(this->Modified ? tr("Modifications have to be saved") : tr("Index is saved"));

    // Adjust columns size
    for (int i = 0; i < ui->TableTB->columnCount() - 1; i++) {
        ui->TableTB->resizeColumnToContents(i);
    }
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
    int                      row   = items.at(0)->row();
    TechnicalBulletin*       tb    = ui->TableTB->item(row, COLUMN_METADATA)->data(TB_ROLE).value<TechnicalBulletin*>();

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
    int                      row       = selection.at(0)->row();
    TechnicalBulletin*       tb        = ui->TableTB->item(row, COLUMN_METADATA)->data(TB_ROLE).value<TechnicalBulletin*>();

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
    ui->TableTB->item(row, COLUMN_RELEASE_DATE)->setText(tb->releaseDate().toString());
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
