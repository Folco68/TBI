#include "Settings.hpp"
#include "Global.hpp"
#
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
