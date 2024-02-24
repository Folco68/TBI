#include "LineEditNoFocus.hpp"

LineEditNoFocus::LineEditNoFocus(QWidget* parent)
    : QLineEdit(parent)
{
}

LineEditNoFocus::~LineEditNoFocus()
{
}

void LineEditNoFocus::focusInEvent(QFocusEvent* event)
{
    QLineEdit::focusInEvent(event);
    deselect();
}
