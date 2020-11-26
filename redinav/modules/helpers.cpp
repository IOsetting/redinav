#include "helpers.h"

#include <QDoubleValidator>
#include <QString>

Helpers::Helpers()
{
}

bool Helpers::isDouble(QString s)
{
    QDoubleValidator validator;
    int notused = 0;
    validator.setLocale(QLocale("C"));
    validator.setNotation(QDoubleValidator::StandardNotation);
    return validator.validate(s, notused);
}
