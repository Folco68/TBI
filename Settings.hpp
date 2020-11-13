#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <Global.hpp>
#include <QSettings>
#include <QSize>
#include <QString>
#include <QStringList>

// Key names
#define KEY_BASE_URL_TECH_PUB "baseUrlTechPub"
#define KEY_BASE_URL_RM "baseUrlRM"
#define KEY_REAL_TIME_SEARCH "realTimeSearch"
#define KEY_WHOLE_WORDS_ONLY "wholeWordsOnly"
#define KEY_CATEGORY_LIST "categoryList"
#define KEY_MAIN_WINDOW_SIZE "mainWindowSize"

// Default values
#define DEFAULT_BASE_URL_TECH_PUB "https://piv.tetrapak.com/techbull/detail_techbull.aspx?id=%1"
#define DEFAULT_BASE_URL_RM "https://piv.tetrapak.com/piv-tp-service/api/techpubs/%1/file"
#define DEFAULT_REAL_TIME_SEARCH false
#define DEFAULT_WHOLE_WORDS_ONLY false
#define DEFAULT_MAIN_WINDOW_SIZE QSize(MAIN_MINIMUM_WIDTH, MAIN_MINIMUM_HEIGHT)

class Settings : public QSettings
{
  public:
    static Settings* instance();
    static void release();

    QString baseURLTechnicalPublication() { return value(KEY_BASE_URL_TECH_PUB, DEFAULT_BASE_URL_TECH_PUB).toString(); };
    void setBaseURLTechnicalPublications(QString url) { setValue(KEY_BASE_URL_TECH_PUB, url); };
    void resetBaseURLTechnicalPublications() { setValue(KEY_BASE_URL_TECH_PUB, DEFAULT_BASE_URL_TECH_PUB); };

    QString baseURLRebuildingManual() { return value(KEY_BASE_URL_RM, DEFAULT_BASE_URL_RM).toString(); };
    void setBaseURLRebuildingManual(QString url) { setValue(KEY_BASE_URL_RM, url); };
    void resetBaseURLRebuildingManual() { setValue(KEY_BASE_URL_RM, DEFAULT_BASE_URL_RM); };

    bool realTimeSearchEnabled() { return value(KEY_REAL_TIME_SEARCH, DEFAULT_REAL_TIME_SEARCH).toBool(); };
    void setRealTimeSearchEnabled(bool enabled) { setValue(KEY_REAL_TIME_SEARCH, enabled); };

    bool wholeWordsOnlyEnabled() { return value(KEY_WHOLE_WORDS_ONLY, DEFAULT_WHOLE_WORDS_ONLY).toBool(); };
    void setWholeWordsOnlyEnabled(bool enabled) { setValue(KEY_WHOLE_WORDS_ONLY, enabled); };

    QStringList categories() { return value(KEY_CATEGORY_LIST).toStringList(); };
    void setCategories(QStringList categories)
    {
        categories.sort(Qt::CaseInsensitive);
        setValue(KEY_CATEGORY_LIST, categories);
    };
    void resetCategories() { remove(KEY_CATEGORY_LIST); };

    QSize mainWindowSize() { return value(KEY_MAIN_WINDOW_SIZE, DEFAULT_MAIN_WINDOW_SIZE).toSize(); };
    void setMainWindowSize(QSize size) { setValue(KEY_MAIN_WINDOW_SIZE, size); };

  private:
    static Settings* settings;

    Settings(QString organization, QString application)
        : QSettings(organization, application){};
    };

#endif // SETTINGS_HPP
