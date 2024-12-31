#include "Logger.hpp"
#include <QTime>

Logger* Logger::logger = nullptr;

Logger* Logger::instance()
{
    if (logger == nullptr) {
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
        this->Log.append('\n');
    }
    this->Log.append(QString("[%1] ").arg(QTime::currentTime().toString("HH:mm:ss:zzz"))).append(text);
    emit textAdded(this->Log);
}

void Logger::append(QString text)
{
    this->Log.append(text);
    emit textAdded(this->Log);
}

void Logger::startTimer()
{
    this->Timer.start();
}

QString Logger::elapsedTime()
{
    int   ms = this->Timer.elapsed();
    QTime Duration(0, 0);
    return Duration.addMSecs(ms).toString("HH:mm:ss.zzz");
}
