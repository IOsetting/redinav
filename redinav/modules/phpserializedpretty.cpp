#include "phpserializedpretty.h"
#include <easylogging++.h>

PhpSerializedPretty::PhpSerializedPretty(const QString string) :
    m_string(string),
    m_index(0),
    m_p(0)
{
    m_mapTypes["n"] = type_n;
    m_mapTypes["N"] = type_N;
    m_mapTypes["o"] = type_o;
    m_mapTypes["O"] = type_O;
    m_mapTypes["s"] = type_s;
    m_mapTypes["S"] = type_S;
    m_mapTypes["a"] = type_a;
    m_mapTypes["A"] = type_A;
    m_mapTypes["c"] = type_c;
    m_mapTypes["C"] = type_C;
    m_mapTypes["i"] = type_i;
    m_mapTypes["I"] = type_I;
    m_mapTypes["b"] = type_b;
    m_mapTypes["B"] = type_B;
    m_mapTypes["d"] = type_d;
    m_mapTypes["D"] = type_D;
    m_mapTypes["f"] = type_f;
    m_mapTypes["F"] = type_D;
    m_mapTypes["r"] = type_r;
    m_mapTypes["R"] = type_R;

}

QString PhpSerializedPretty::pretty(const QString input)
{
    QString localInput(input);
    localInput.replace("\\x00", " ");

    PhpSerializedPretty parser(localInput);
    QString output("");


    try {
        output = parser.parse();
    }
    catch (Exception e)
    {
        //LOG(ERROR) << "Unable to parse PHP serialized string";
        //LOG(ERROR) << e.what();
        //output = input;
        output = "***INVALID PHP SERIALIZATION***";
    }

    return output;
}


QString PhpSerializedPretty::read(const int n) {
    QString o = m_string.mid(m_p, n);
    m_p += n;
    return o;
}

QString PhpSerializedPretty::readTo(const QString s) {
    int p = m_string.indexOf(s, m_p);
    if (p == -1) {
        throw Exception(QString("Could not find '%1' from offset %2 in '%3'").arg(s).arg(m_p).arg(m_string.mid(0, 50) + "..."));
    }
    else
    {
        return read(p - m_p);
    }
}


QString PhpSerializedPretty::assertString(const QString s) {
    int p = m_p;
    QString r = read(s.length());
    if (r != s) {
        throw Exception(QString("Got '%1' but expected '%2' at offset %3 in '%4'").arg(r).arg(s).arg(p).arg(m_string.mid(0, 50) + "..."));
    }
    return r;
}


QString PhpSerializedPretty::parseArray(const QString nl) {

    QString s("");
    QString n("");

    assertString(":");
    n = readTo(":");
    assertString(":{");
    s = "[" + n + "] {" + nl;
    QString k("");
    QString v("");
    for (int i=0; i < n.toInt(); i++)
    {
        k = parse(nl + "    ");
        v = parse(nl + "    ");
        s += "    " + k + " => " + v + nl;
    }
    return s + assertString("}");
}


QString PhpSerializedPretty::parseString() {

    QString s("");
    QString l("");

    assertString(":");
    l = readTo(":");
    assertString(":");
    s += assertString("\"");
    s += read(l.toInt());
    s += assertString("\"");
    return s;
}


QString PhpSerializedPretty::parseInt() {

    QString s("");
    QString l("");

    assertString(":");
    s += (l = readTo(";"));
    return s;
}

QString PhpSerializedPretty::parseDouble() {

    QString s("");
    QString l("");

    assertString(":");
    s += (l = readTo(";"));
    return s;
}

QString PhpSerializedPretty::parse(const QString nl) {


    QString l;
    QString s = read();


    if (!m_mapTypes.contains(s)) {
        s += assertString(":");
        s += readTo(";");
        s += assertString(";");
        return s;
    }

    switch (m_mapTypes[s]) {

    case type_i:
    case type_I:
        s = parseInt();
        assertString(";");
        return s;
        break;

    case type_r:
    case type_R:
        s = parseInt();
        assertString(";");
        return s;
        break;

    case type_d:
    case type_D:
    case type_f:
    case type_F:
        s = parseDouble();
        assertString(";");
        return s;
        break;

    case type_b:
    case type_B:
        s = parseInt();
        assertString(";");
        s = (s == "1" ? "true" : "false");
        return s;
        break;

    case type_n:
    case type_N:
        s = assertString(";");
        return "null";
        break;

    case type_o:
    case type_O:
        s = "(Object) " + parseString() + " => ";
        s += "props" + parseArray(nl);
        return s;
        break;

    case type_s:
    case type_S:
        s = parseString();
        assertString(";");
        return s;
        break;

    case type_a:
    case type_A:
        s = parseArray(nl);
        return "Array" + s;
        break;

    case type_c:
    case type_C:
        s = "(Class) " + parseString();
        assertString(":");
        l = readTo(":");
        assertString(":{");
        s += " {" + nl + "    " + read(l.toInt());
        assertString("}");
        s += nl + "}";
        return s;
        break;

    default:
        break;
        return "[unknown type]";

    }

    return "";

}
