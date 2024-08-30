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

#ifndef CONTEXTMENUACTION_HPP
#define CONTEXTMENUACTION_HPP

#include <QAction>
#include <QKeySequence>
#include <QWidget>

//  ContextMenuAction
//
// This class allow to create a QAction with a shortcut
// It makes the shortcut visible in the menu
//
class ContextMenuAction: public QAction
{
  public:
    ContextMenuAction(const QString& text, QObject* parent, QKeySequence shortcut = QKeySequence());
};

#endif // CONTEXTMENUACTION_HPP
