#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <QElapsedTimer>
#include <QObject>
#include <QString>

class Logger : public QObject
{
    Q_OBJECT
  public:
    static Logger* instance();
    static void    release();

    void newEntry(QString text);
    void append(QString text);
    void startTimer();
    void displayTimer();

  private:
    static Logger* logger;
    QString        Log;
    QElapsedTimer  Timer;

  signals:
    void textAdded(QString text);
};

#endif // LOGGER_HPP
