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

#ifndef DLGSETTINGS_HPP
#define DLGSETTINGS_HPP

#include <QDialog>

namespace Ui {
    class DlgSettings;
}

//  DlgSettings
//
// This class allow to change global application settings
// It is accessed with a static member
//
class DlgSettings: public QDialog
{
    Q_OBJECT

  public:
    static bool showDlgSettings(QWidget* parent = nullptr);
    ~DlgSettings() override;

  private:
    explicit DlgSettings(QWidget* parent = nullptr);
    Ui::DlgSettings* ui;
};

#endif // DLGSETTINGS_HPP
