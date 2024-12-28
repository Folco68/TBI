#include "Index.hpp"

Index* Index::index = nullptr;

Index* Index::instance()
{
    if (index == nullptr) {
        index = new Index;
    }
    return index;
}

void Index::release()
{
    if (index != nullptr) {
        delete index;
        index = nullptr;
    }
}

Index::~Index()
{
    // Delete every element of the list and resize it to 0
    while (!Bulletins.isEmpty()) {
        delete Bulletins.takeLast();
    }
}

QList<TechnicalBulletin*> Index::bulletins() const
{
    return this->Bulletins;
}
