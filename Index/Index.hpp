#ifndef INDEX_HPP
#define INDEX_HPP

#include "TechnicalBulletin.hpp"
#include <QList>
#include <QObject>

class Index : public QObject
{
    Q_OBJECT
  public:
    static Index* instance();
    static void   release();

    QList<TechnicalBulletin*> bulletins() const;

  private:
    ~Index();
    static Index* index;

    QList<TechnicalBulletin*> Bulletins;
};

#endif // INDEX_HPP
