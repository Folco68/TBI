#ifndef INDEX_HPP
#define INDEX_HPP

#include "TechnicalBulletin.hpp"
#include <QEvent>
#include <QList>
#include <QObject>
#include <QString>

class Index : public QObject
{
    Q_OBJECT
  public:
    static Index* instance();
    static void   release();

    QList<TechnicalBulletin*> bulletins() const;

  signals:
    // Opening on its normal way
    void openingStarting();
    void openingHeader(int version, qint32 count);
    void openingProgress(int count);
    void openingSuccessful(int count); // Mark end of opening
    void noIndexFound();               // Mark end of opening

    // Opening failing
    void indexTooRecent(qint32 version);
    void invalidMagic(QString magic);
    void failedToOpenIndex();
    void failedToReadFileContent();
    void openingFailed(int count); // Mark end of opening

    // Save failure
    void failedToCreateBackup();
    void failedToOpenFileForSaving();
    void failedToWriteContent(int count);
    void savingSuccessful(int count);

  private:
    Index();
    ~Index();
    static Index* index;

    bool event(QEvent* event) override;
    void open(bool ForceIndexCheck);
    void openIndexVersion0(qint32 count, QDataStream& stream, bool ForceIndexCheck);
    void openIndexVersion1(QDataStream& stream, bool ForceIndexCheck);
    void save(bool backup);

    QList<TechnicalBulletin*> Bulletins;
    bool                      OpeningSuccessful;
};

#endif // INDEX_HPP
