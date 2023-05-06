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

#ifndef DLGTB_HPP
#define DLGTB_HPP

#include "MainWindow.hpp"
#include "TechnicalBulletin.hpp"
#include <QByteArray>
#include <QDialog>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QString>

namespace Ui {
class DlgTB;
}

//  DlgTB
//
// Class managing the UI allowing TB creation/edition
// Static methods are used to call the dialog
//
class DlgTB : public QDialog
{
    Q_OBJECT

  public:
    static TechnicalBulletin* newDlgTB(MainWindow* parent);                     // New TB, created by hand
    static TechnicalBulletin* newDlgTB(MainWindow* parent, QByteArray data); // New TB, created by drag'n drop
    static bool editDlgTB(MainWindow* parent, TechnicalBulletin* tb);           // Existing TB edition

  private:
    Ui::DlgTB* ui;

    DlgTB(MainWindow* parent, QString title);
    DlgTB(MainWindow* parent, QString title, TechnicalBulletin* tb);
    ~DlgTB() override;
    void accept() override;

    // Drag & drop stuff
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

    // Menus actions
    void copyScreenshot();
    void saveToFile();
    void copyHeader();
    void copyAll();

    QString getHeader();

    // Misc
    void fillUI(TechnicalBulletin* tb);
    void fillTB(TechnicalBulletin* tb);
};

#endif // DLGTB_HPP
