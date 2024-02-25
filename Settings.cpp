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

#include "Settings.hpp"
#include "Global.hpp"

Settings::Settings(QString organization, QString application)
    : QSettings(organization, application)
{
    // Remove some old keys to clean up the Windows Registry
    remove("baseUrlTechPub");
    remove("baseUrlRM");
}

Settings::~Settings()
{
}

// Singleton stuff
Settings* Settings::settings = nullptr;

Settings* Settings::instance()
{
    if (settings == nullptr) {
        settings = new Settings(ORGANIZATION_NAME, APPLICATION_NAME);
    }
    return settings;
}

void Settings::release()
{
    delete settings;
    settings = nullptr;
}
