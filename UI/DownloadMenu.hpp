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

#ifndef DOWNLOADMENU_HPP
#define DOWNLOADMENU_HPP

#include <QAction>
#include <QList>
#include <QMenu>
#include <QString>

//  DownloadMenu
//
// The purpose of this class is to manage a menu which changes often and dynamically,
// because the number of contained actions can vary, and when they are triggered,
// the download target URL varies.
// When an action is added, the triggered() slot is connected to a lambda
//
class DownloadMenu: public QMenu
{
  public:
    DownloadMenu(QWidget* parent = nullptr);
    ~DownloadMenu();
    void setItems(QString DocsString, QString TBnumber, QWidget* WidgetToFocus = nullptr);

  private:
    QList<QAction*> ActionList;
    void            deleteActions();
};


#endif // DOWNLOADMENU_HPP
