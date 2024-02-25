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

#ifndef GLOBAL_HPP
#define GLOBAL_HPP

// Window parameters
#define WINDOW_TITLE "Technical Bulletin Indexer"

#define MAIN_MINIMUM_WIDTH  1024
#define MAIN_MINIMUM_HEIGHT 768

#define DLGTB_WINDOW_WIDTH  800
#define DLGTB_WINDOW_HEIGHT 600

#define DLGSETTINGS_WINDOW_WIDTH 800

// Keyword separator in UI
#define KEYWORD_SEPARATOR " "

// Suffix of a CTI PDF file, added to the regular TB PDF file
#define CTI_SUFFIX "_CTI"

// QCoreApplication settings (for QSettings)
#define ORGANIZATION_NAME "FolcoSoft"
#define APPLICATION_NAME  "TBI"

// Command line option
#define OPTION_FORCE_DATABASE_CHECK "--check-database"

// Data filename
#define TBI_BACKUP_FILENAME "index.bak"
#define TBI_FILENAME        "index.tbi"

// Screenshot file extension
#define SCREENSHOT_EXTENSION ".png"

// Current TBI version
#define CURRENT_TBI_VERSION 1

// Magic string to identify a TBI DB
#define TBI_MAGIC "TBI_DB_BY_MARTIAL_DEMOLINS"

#endif // GLOBAL_HPP
