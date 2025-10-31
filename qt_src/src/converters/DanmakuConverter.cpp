#include "DanmakuConverter.h"
#include "qtbilimerge.h"

DanmakuConverter::DanmakuConverter(QObject *parent)
    : QObject(parent)
    , m_fontSize(DEFAULT_FONT_SIZE)
    , m_textOpacity(DEFAULT_TEXT_OPACITY)
    , m_reverseBlank(DEFAULT_REVERSE_BLANK)
    , m_durationMarquee(DEFAULT_DURATION_MARQUEE)
    , m_durationStill(DEFAULT_DURATION_STILL)
    , m_reduceComments(false)
{
}

bool DanmakuConverter::convertXmlToAss(const QString &xmlPath, const QString &assPath)
{
    return convertXmlToAss(xmlPath, assPath, m_fontSize, m_textOpacity, m_reverseBlank,
                           m_durationMarquee, m_durationStill, m_reduceComments);
}

bool DanmakuConverter::convertXmlToAss(const QString &xmlPath, const QString &assPath,
                                       int fontSize, double textOpacity, double reverseBlank,
                                       int durationMarquee, int durationStill,
                                       bool reduceComments)
{
    Q_UNUSED(xmlPath)
    Q_UNUSED(assPath)
    Q_UNUSED(fontSize)
    Q_UNUSED(textOpacity)
    Q_UNUSED(reverseBlank)
    Q_UNUSED(durationMarquee)
    Q_UNUSED(durationStill)
    Q_UNUSED(reduceComments)
    return false;
}

void DanmakuConverter::setFontSize(int size)
{
    m_fontSize = size;
}

void DanmakuConverter::setTextOpacity(double opacity)
{
    m_textOpacity = opacity;
}

void DanmakuConverter::setReverseBlank(double blank)
{
    m_reverseBlank = blank;
}

void DanmakuConverter::setDurationMarquee(int duration)
{
    m_durationMarquee = duration;
}

void DanmakuConverter::setDurationStill(int duration)
{
    m_durationStill = duration;
}

void DanmakuConverter::setReduceComments(bool reduce)
{
    m_reduceComments = reduce;
}

QString DanmakuConverter::generateAssHeader() const
{
    return QString();
}

QString DanmakuConverter::processDanmakuElement(const QString &xmlElement) const
{
    Q_UNUSED(xmlElement)
    return QString();
}

