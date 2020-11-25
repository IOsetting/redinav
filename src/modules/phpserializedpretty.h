#include <QMap>
#include <QString>
#include <QDebug>
#include <easylogging++.h>
#include <qredisclient/connection.h>


#pragma once

class PhpSerializedPretty
{

    ADD_EXCEPTION

public:
    PhpSerializedPretty(const QString string);
    static QString pretty(const QString input);
private:

    enum PhpSerializeTypes {
        type_n,
        type_N,
        type_o,
        type_O,
        type_s,
        type_S,
        type_a,
        type_A,
        type_c,
        type_C,
        type_i,
        type_I,
        type_b,
        type_B,
        type_d,
        type_D,
        type_f,
        type_F,
        type_r,
        type_R
    };

    QString m_string;
    int m_index;
    int m_p;

    QString read(const int n = 1);
    QString readTo(const QString s);
    QString assertString(const QString s);
    QString parseArray(const QString nl);
    QString parseString();
    QString parseInt();
    QString parse(const QString nl = "\n");

    QMap<QString, PhpSerializeTypes> m_mapTypes;

    QString parseDouble();
};

