#ifndef ALGORITHMS_DETECTION_COMMON_CONTEXT_H
#define ALGORITHMS_DETECTION_COMMON_CONTEXT_H

#include "C:\Git Workspace\ComputerVision-master\Algorithms\Common/Algorithms.h"

#include <QMultiMap>
#include <QMap>
#include <QPoint>
#include <QRect>
#include <QDataStream>

namespace algorithms
{
    namespace detection
    {
        struct Input
        {
            QString HaarCascadeClassifierFileName;
            qreal ScaleFactor;
            QSize MinObjectSize;
        };

        enum PointType
        {
            LeftEyePoint,
            RightEyePoint
        };
        Q_DECLARE_FLAGS(PointsType, PointType)

            typedef QMap<PointsType, QPoint> PointsMap;

        struct Output
        {
            QRect Rectangle;
            PointsMap Points;

            Output& operator/=(const qreal& divisor);
        };
    }

    struct Detection
    {
        detection::Input In;
        QMultiMap<AlgType, detection::Output> Out;
    };
}

typedef algorithms::detection::Input AlgDetInput;
typedef algorithms::detection::Output AlgDetOutput;
typedef QMultiMap<AlgType, AlgDetOutput> AlgDetOutputMap;
typedef algorithms::Detection AlgDetInOut;

AlgDetOutputMap getMirroredDetectors(const AlgDetOutputMap& Detectors, const int& ImageWidth);

QDataStream& operator>>(QDataStream& Stream, AlgDetInOut& Value);
QDataStream& operator<<(QDataStream& Stream, const AlgDetInOut& Value);

#endif // ALGORITHMS_DETECTION_COMMON_CONTEXT_H
