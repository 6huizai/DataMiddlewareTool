#ifndef QGVC_PROJECTIONUTILS_H
#define QGVC_PROJECTIONUTILS_H

#include <QtPositioning/QGeoCoordinate>
#include <qmath.h>
#include <cmath>


class ProjectionUtils
{
public:

    static inline QGeoCoordinate geoDirect(
            const QGeoCoordinate &pos, double azi1, double s12)
    {
        QGeoCoordinate coord1(pos.latitude(), pos.longitude());
        QGeoCoordinate coord2 = coord1.atDistanceAndAzimuth(s12, azi1,0);
        return coord2;
    }


};


#endif // QGVC_PROJECTIONUTILS_H
