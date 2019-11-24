#include "context.h"

QDataStream& operator>>(QDataStream& Stream, AlgContext& Value)
{
    Stream >> Value.Detection;
    Stream >> Value.Vectorization;
    return Stream;
}

QDataStream& operator<<(QDataStream& Stream, const AlgContext& Value)
{
    Stream << Value.Detection;
    Stream << Value.Vectorization;
    return Stream;
}
