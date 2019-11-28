#ifndef CONTEXT_H
#define CONTEXT_H
#ifndef ALGORITHMS_COMMON_CONTEXT_H
#define ALGORITHMS_COMMON_CONTEXT_H

#include "context.h"
#include "../Vectorization/Common/Context.h"
#include "Algorithms.h"

#include <QSize>
#include <QString>

namespace algorithms
{
    struct ContextType
    {
        AlgDetInOut Detection;
        AlgVecInOut Vectorization;
    };
}

typedef algorithms::ContextType AlgContext;

QDataStream& operator>>(QDataStream& Stream, AlgContext& Value);
QDataStream& operator<<(QDataStream& Stream, const AlgContext& Value);

#endif // ALGORITHMS_COMMON_CONTEXT_H
