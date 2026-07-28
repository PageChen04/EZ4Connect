#include "consoleoutputdecoder.h"

#include <QTextCodec>

QString ConsoleOutputDecoder::decode(const QByteArray &bytes)
{
    static QString encoding;
    if (encoding == "UTF-8")
    {
        return QTextCodec::codecForName("UTF-8")->toUnicode(bytes);
    }
    if (encoding == "GBK")
    {
        return QTextCodec::codecForName("GBK")->toUnicode(bytes);
    }
    if (encoding == "locale")
    {
        return QString::fromLocal8Bit(bytes);
    }

    QTextCodec *utf8Codec = QTextCodec::codecForName("UTF-8");
    const QString utf8 = utf8Codec->toUnicode(bytes);
    const QByteArray utf8RoundTrip = utf8Codec->fromUnicode(utf8);

    QTextCodec *gbkCodec = QTextCodec::codecForName("GBK");
    const QString gbk = gbkCodec->toUnicode(bytes);
    const QByteArray gbkRoundTrip = gbkCodec->fromUnicode(gbk);

    if (utf8RoundTrip == bytes && gbkRoundTrip != bytes)
    {
        encoding = "UTF-8";
        return utf8;
    }
    if (gbkRoundTrip == bytes && utf8RoundTrip != bytes)
    {
        encoding = "GBK";
        return gbk;
    }
    if (gbkRoundTrip == bytes && utf8RoundTrip == bytes)
    {
        return utf8;
    }

    encoding = "locale";
    return QString::fromLocal8Bit(bytes);
}
