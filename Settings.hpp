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

#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <Global.hpp>
#include <QSettings>
#include <QSize>
#include <QString>
#include <QStringList>

// Key names
#define KEY_BASE_URL_TB          "baseUrlTechPub"
#define KEY_BASE_URL_TECH_PUB    "baseUrlRM"
#define KEY_REAL_TIME_SEARCH     "realTimeSearch"
#define KEY_WHOLE_WORDS_ONLY     "wholeWordsOnly"
#define KEY_CATEGORY_LIST        "categoryList"
#define KEY_MAIN_WINDOW_SIZE     "mainWindowSize"
#define KEY_SEARCH_NUMBER        "searchNumber"
#define KEY_SEARCH_TITLE         "searchTitle"
#define KEY_SEARCH_CATEGORY      "searchCategory"
#define KEY_SEARCH_RK            "searchRK"
#define KEY_SEARCH_TECH_PUB      "searchTechPub"
#define KEY_SEARCH_RELEASE_DATE  "searchReleaseDate"
#define KEY_SEARCH_REGISTERED_BY "searchRegisteredBy"
#define KEY_SEARCH_REPLACES      "searchReplaces"
#define KEY_SEARCH_REPLACED_BY   "searchReplacedBy"
#define KEY_SEARCH_COMMENT       "searchComment"

// Default values
#define DEFAULT_BASE_URL_TB          "https://piv.tetrapak.com/techbull/detail_techbull.aspx?id=%1"
#define DEFAULT_BASE_URL_TECH_PUB    "https://piv.tetrapak.com/piv-tp-service/api/techpubs/%1/file"
#define DEFAULT_REAL_TIME_SEARCH     false
#define DEFAULT_WHOLE_WORDS_ONLY     false
#define DEFAULT_MAIN_WINDOW_SIZE     QSize(MAIN_MINIMUM_WIDTH, MAIN_MINIMUM_HEIGHT)
#define DEFAULT_SEARCH_NUMBER        false
#define DEFAULT_SEARCH_TITLE         false
#define DEFAULT_SEARCH_CATEGORY      false
#define DEFAULT_SEARCH_RK            false
#define DEFAULT_SEARCH_TECH_PUB      false
#define DEFAULT_SEARCH_RELEASE_DATE  false
#define DEFAULT_SEARCH_REGISTERED_BY false
#define DEFAULT_SEARCH_REPLACES      false
#define DEFAULT_SEARCH_REPLACED_BY   false
#define DEFAULT_SEARCH_COMMENT       false

//  Settings
//
// This class handles the global configuration of the software
//
class Settings: public QSettings
{
  public:
    static Settings* instance();
    static void      release();

    QString baseURLTechnicalBulletin() { return value(KEY_BASE_URL_TB, DEFAULT_BASE_URL_TB).toString(); }
    void    setBaseURLTechnicalBulletin(QString url) { setValue(KEY_BASE_URL_TB, url); }
    void    resetBaseURLTechnicalBulletin() { setValue(KEY_BASE_URL_TB, DEFAULT_BASE_URL_TB); }

    QString baseURLTechnicalPublication() { return value(KEY_BASE_URL_TECH_PUB, DEFAULT_BASE_URL_TECH_PUB).toString(); }
    void    setBaseURLTechnicalPublication(QString url) { setValue(KEY_BASE_URL_TECH_PUB, url); }
    void    resetBaseURLTechnicalPublication() { setValue(KEY_BASE_URL_TECH_PUB, DEFAULT_BASE_URL_TECH_PUB); }

    bool realTimeSearchEnabled() { return value(KEY_REAL_TIME_SEARCH, DEFAULT_REAL_TIME_SEARCH).toBool(); }
    void setRealTimeSearchEnabled(bool enabled) { setValue(KEY_REAL_TIME_SEARCH, enabled); }

    bool wholeWordsOnlyEnabled() { return value(KEY_WHOLE_WORDS_ONLY, DEFAULT_WHOLE_WORDS_ONLY).toBool(); }
    void setWholeWordsOnlyEnabled(bool enabled) { setValue(KEY_WHOLE_WORDS_ONLY, enabled); }

    QStringList categories() { return value(KEY_CATEGORY_LIST).toStringList(); }
    void        setCategories(QStringList categories)
    {
        categories.sort(Qt::CaseInsensitive);
        setValue(KEY_CATEGORY_LIST, categories);
    }
    void resetCategories() { remove(KEY_CATEGORY_LIST); }

    QSize mainWindowSize() { return value(KEY_MAIN_WINDOW_SIZE, DEFAULT_MAIN_WINDOW_SIZE).toSize(); }
    void  setMainWindowSize(QSize size) { setValue(KEY_MAIN_WINDOW_SIZE, size); }

    bool searchNumberEnabled() { return value(KEY_SEARCH_NUMBER, DEFAULT_SEARCH_NUMBER).toBool(); }
    void setSearchNumber(bool enabled) { setValue(KEY_SEARCH_NUMBER, enabled); }

    bool searchTitleEnabled() { return value(KEY_SEARCH_TITLE, DEFAULT_SEARCH_TITLE).toBool(); }
    void setSearchTitle(bool enabled) { setValue(KEY_SEARCH_TITLE, enabled); }

    bool searchCategoryEnabled() { return value(KEY_SEARCH_CATEGORY, DEFAULT_SEARCH_CATEGORY).toBool(); }
    void setSearchCategory(bool enabled) { setValue(KEY_SEARCH_CATEGORY, enabled); }

    bool searchRKEnabled() { return value(KEY_SEARCH_RK, DEFAULT_SEARCH_RK).toBool(); }
    void setSearchRK(bool enabled) { setValue(KEY_SEARCH_RK, enabled); }

    bool searchTechPubEnabled() { return value(KEY_SEARCH_TECH_PUB, DEFAULT_SEARCH_TECH_PUB).toBool(); }
    void setSearchTechPub(bool enabled) { setValue(KEY_SEARCH_TECH_PUB, enabled); }

    bool searchReleaseDateEnabled() { return value(KEY_SEARCH_RELEASE_DATE, DEFAULT_SEARCH_RELEASE_DATE).toBool(); }
    void setSearchReleaseDate(bool enabled) { setValue(KEY_SEARCH_RELEASE_DATE, enabled); }

    bool searchRegisteredByEnabled() { return value(KEY_SEARCH_REGISTERED_BY, DEFAULT_SEARCH_REGISTERED_BY).toBool(); }
    void setSearchRegisteredBy(bool enabled) { setValue(KEY_SEARCH_REGISTERED_BY, enabled); }

    bool searchReplacesEnabled() { return value(KEY_SEARCH_REPLACES, DEFAULT_SEARCH_REPLACES).toBool(); }
    void setSearchReplaces(bool enabled) { setValue(KEY_SEARCH_REPLACES, enabled); }

    bool searchReplacedByEnabled() { return value(KEY_SEARCH_REPLACED_BY, DEFAULT_SEARCH_REPLACED_BY).toBool(); }
    void setSearchReplacedBy(bool enabled) { setValue(KEY_SEARCH_REPLACED_BY, enabled); }

    bool searchCommentEnabled() { return value(KEY_SEARCH_COMMENT, DEFAULT_SEARCH_COMMENT).toBool(); }
    void setSearchComment(bool enabled) { setValue(KEY_SEARCH_COMMENT, enabled); }

  private:
    static Settings* settings;
    Settings(QString organization, QString application);
    ~Settings();
};

#endif // SETTINGS_HPP
