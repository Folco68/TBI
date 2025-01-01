/***********************************************************************************************************************
 *                                                                                                                     *
 *                        TBI - Technical Bulletin Indexer - Save and index Technical Bulletins                        *
 *                                    allowing to use keywords to find them easily                                     *
 *                                 Copyright (C) 2020-2025 Martial Demolins AKA Folco                                  *
 *                                                                                                                     *
 *                        This program is free software: you can redistribute it and/or modify                         *
 *                        it under the terms of the GNU General Public License as published by                         *
 *                          the Free Software Foundation, either version 3 of the License, or                          *
 *                                         (at your option) any later version                                          *
 *                                                                                                                     *
 *                           This program is distributed in the hope that it will be useful                            *
 *                           but WITHOUT ANY WARRANTY; without even the implied warranty of                            *
 *                            MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                            *
 *                                     GNU General Public License for more details                                     *
 *                                                                                                                     *
 *                          You should have received a copy of the GNU General Public License                          *
 *                         along with this program.  If not, see <https://www.gnu.org/licenses                         *
 *                                                                                                                     *
 *                                  mail: martial <dot> demolins <at> gmail <dot> com                                  *
 *                                                                                                                     *
 **********************************************************************************************************************/

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
