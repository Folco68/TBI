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

#ifndef DLGHELP_HPP
#define DLGHELP_HPP

#include <QDialog>

namespace Ui {
    class DlgHelp;
}

//  DlgHelp
//
// This class displays a static dialog with help, version, about, ...
// It is accessed with a static member
//
class DlgHelp: public QDialog
{
    Q_OBJECT

  public:
    static void showDlgHelp();
    ~DlgHelp() override;

  private:
    explicit DlgHelp(QWidget* parent = nullptr);
    Ui::DlgHelp* ui;
};

// Strings replaced in the about file at run time
#define REPLACE_APPLICATION_VERSION_STR "REPLACE_APPLICATION_VERSION_STR"
#define REPLACE_POSITION_STR            "REPLACE_POSITION_STR"
#define REPLACE_COPYRIGHT_STR           "REPLACE_COPYRIGHT_STR"
#define REPLACE_QT_VERSION_STR          "REPLACE_QT_VERSION_STR"

#endif // DLGHELP_HPP
