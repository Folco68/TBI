#include "Logger.hpp"
#include <QTime>

Logger* Logger::logger = nullptr;

Logger* Logger::instance()
{
    if (logger != nullptr) {
        logger = new Logger;
    }
    return logger;
}

void Logger::release()
{
    if (logger != nullptr) {
        delete logger;
        logger = nullptr;
    }
}

void Logger::newEntry(QString text)
{
    if (!this->Log.isEmpty()) {
        text.prepend('\n');
    }
    this->Log.append(text);
    emit textAdded(text);
}

void Logger::append(QString text)
{
    this->Log.append(text);
    emit textAdded(text);
}

void Logger::startTimer()
{
    this->Timer.start();
}

void Logger::displayTimer()
{
    int   ms = this->Timer.elapsed();
    QTime Time(0, 0, 0, ms);
    this->Log.append(Time.toString("HH:mm:ss.zzz"));
}
