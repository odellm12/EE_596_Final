#ifndef FRAME_H
#define FRAME_H

#include <types_c.h>
#include <core.hpp>

#include <QImage>
#include <QPixmap>

namespace common
{
    class Frame;

    void convertColor(Frame& Dst, const Frame& Src, int ConversionType);

    class Frame : public cv::Mat
    {
        friend void convertColor(Frame&, const Frame&, int);
    public:
        Frame();
        Frame(const QString& FileName);
        Frame(const Frame& Other);
        Frame(const cv::Mat& Other, const QImage::Format& OtherFormat);
        Frame(const Frame& Other, const QRect& ROI);
        Frame(const IplImage* Image, bool CopyData = false);
        Frame(const QImage& Other);
        ~Frame();

        Frame& operator=(const Frame& Other);
        Frame& operator=(const IplImage* Other);
        Frame& operator=(const QImage& Other);

        Frame operator()(const QRect& ROI) const;

        operator QPixmap() const;
        operator QImage() const;
        operator QByteArray() const;

        int width() const;
        int height() const;

        int bytesPerLine() const;

        int byteCount() const;

        bool isNull() const;

        qint64 crc() const;

        void flipLR();

        bool loadFromData(const QByteArray& Data);

        quint8 getColorDepth() const;
        quint8 getMiddleBrightness() const;

        bool valid(const QPoint& Position) const;
        bool valid(int X, int Y) const;

        QRgb pixel(const QPoint& Position) const;
        QRgb pixel(int X, int Y) const;

        Frame scaled(const qreal& ScaleFactor) const;
        Frame mirrored() const;

        QImage::Format format() const;

        uchar* scanLine(const int Line);
        const uchar* constScanLine(const int Line) const;

    private:
        int getCVType() const;

    private:
        QImage::Format Format;
    };
}

#endif // COMMON_FRAME_H
