#include "prjmessage.h"

namespace radar {
namespace net {

bool UploadTargetPointsMessage::parseBody(QByteArray& array)
{

    int32_t pointNum = 0;
    QDataStream stream(&array, QIODevice::ReadOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream >> timestamp;
    stream >> frame;
    stream >> pointNum;
    points.clear();
    for (int i=0; i<pointNum; i++)
    {
        Point p;
        stream >> p.distance;
        stream >> p.angle;
        stream >> p.doppler;
        stream >> p.snr;
        stream >> p.beam;
        stream >> p.pluse;
        points.push_back(p);
    }
    return true;
}

bool UploadTargetPointsMessageV3::parseBody(QByteArray& array)
{

    int32_t pointNum = 0;
    QDataStream stream(&array, QIODevice::ReadOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream >> timestamp;
    stream >> frame;
    stream >> scanBoundaryA;
    stream >> scanBoundaryB;
    stream >> reserve;
    stream >> orientation;
    stream >> scanningDirection;
    stream >> pointNum;
    points.clear();
    for (int i=0; i<pointNum; i++)
    {
        Point p;
        stream >> p.x;
        stream >> p.y;
        stream >> p.z;
        stream >> p.di;
        stream >> p.ri;
        stream >> p.snr;
        stream >> p.beam;
        stream >> p.dump0;
        points.push_back(p);
    }
    return true;
}

bool UploadTrackPointsMessage::parseBody(QByteArray& array)
{
    int32_t pointNum = 0;
    QDataStream stream(&array, QIODevice::ReadOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream >> timestamp;
    stream >> frame;
    stream >> pointNum;
    points.clear();
    for (int i=0; i<pointNum; i++)
    {
        Point p;
        stream >> p.distance;
        stream >> p.angle;
        stream >> p.speed;
        stream >> p.snr;
        stream >> p.id;
        stream >> p.object;
        stream >> p.pluse;
        stream >> p.beam;
        stream >> p.doppler;
        stream >> p.dopplerWidth;
        points.push_back(p);
    }
    return true;
}

bool UploadTrackPointsMessageV2::parseBody(QByteArray& array)
{
    int32_t pointNum = 0;
    QDataStream stream(&array, QIODevice::ReadOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream >> timestamp;
    stream >> frame;
    stream >> pointNum;
    points.clear();
    for (int i=0; i<pointNum; i++)
    {
        Point p;
        stream >> p.distance;
        stream >> p.angle;
        stream >> p.speed;
        stream >> p.speedDirection;
        stream >> p.snr;
        stream >> p.id;
        stream >> p.object;
        stream >> p.pluse;
        stream >> p.beam;
        stream >> p.doppler;
        stream >> p.dopplerWidth;
        stream >> p.privDistance;
        stream >> p.privAngle;
        stream >> p.privSpeed;
        stream >> p.privSpeedDirection;
        points.push_back(p);
    }
    return true;
}


bool UploadTrackPointsMessageV3::parseBody(QByteArray& array)
{
    int32_t pointNum = 0;
    QDataStream stream(&array, QIODevice::ReadOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream >> timestamp;
    stream >> frame;
    stream >> frameTimestamp;
    stream >> scanBoundaryA;
    stream >> scanBoundaryB;
    stream >> reserve;
    stream >> orientation;
    stream >> scanningDirection;
    stream >> pointNum;
    points.clear();
    for (int i=0; i<pointNum; i++)
    {
        Point p;
        stream >> p.x;
        stream >> p.y;
        stream >> p.z;
        stream >> p.vx;
        stream >> p.vy;
        stream >> p.vz;
        stream >> p.id;
        stream >> p.snr;
        stream >> p.rcs2;
        stream >> p.beam;
        stream >> p.di;
        stream >> p.ri;
        stream >> p.object;
        stream >> p.pluse;
        stream >> p.timestamp;
        stream >> p.dump0;
        stream >> p.dump1;
        stream >> p.rcs6;
        stream >> p.selected;

        points.push_back(p);
    }
    return true;
}

QByteArray UploadTrackPointsMessageV3::dumpBody()
{
    QDataStream stream(&m_data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << timestamp;
    stream << frame;
    stream << frameTimestamp;
    stream << scanBoundaryA;
    stream << scanBoundaryB;
    stream << reserve;
    stream << orientation;
    stream << scanningDirection;
    stream << (int32_t)points.size();
    for (int i=0; i<points.size(); i++)
    {
        stream << points[i].x;
        stream << points[i].y;
        stream << points[i].z;
        stream << points[i].vx;
        stream << points[i].vy;
        stream << points[i].vz;
        stream << points[i].id;
        stream << points[i].snr;
        stream << points[i].rcs2;
        stream << points[i].beam;
        stream << points[i].di;
        stream << points[i].ri;
        stream << points[i].object;
        stream << points[i].pluse;
        stream << points[i].timestamp;
        stream << points[i].dump0;
        stream << points[i].dump1;
        stream << points[i].rcs6;
        stream << points[i].selected;
    }
    return m_data;
}

bool UploadClutterGridMessage::parseBody(QByteArray& array)
{
    // QLOG_INFO() << "UploadClutterGridMessage array: " << array.toHex(' ');
    int32_t pointNum = 0;
    QDataStream stream(&array, QIODevice::ReadOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream >> timestamp;
    stream >> frame;
    stream >> pointNum;
    grids.clear();
    for (int i=0; i<pointNum; i++)
    {
        Grid g;
        stream >> g.distance;
        stream >> g.distanceRange;
        stream >> g.angle;
        stream >> g.angleRange;
        stream >> g.mean;
        stream >> g.level;
        grids.push_back(g);
        // QLOG_INFO() << "grid: " << g.distance << ", " << g.distanceRange;
    }
    return true;
}


bool UploadClutterGridMessageV3::parseBody(QByteArray& array)
{
    // QLOG_INFO() << "UploadClutterGridMessage array: " << array.toHex(' ');
    int32_t pointNum = 0;
    QDataStream stream(&array, QIODevice::ReadOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream >> timestamp;
    stream >> frame;
    stream >> scanningBoundaryA;
    stream >> scanningBoundaryB;
    stream >> pointNum;
    grids.clear();
    for (int i=0; i<pointNum; i++)
    {
        Grid g;
        stream >> g.range;
        stream >> g.rangeSpan;
        stream >> g.azimuth;
        stream >> g.azimuthSpan;
        stream >> g.pitch;
        stream >> g.pitchSpan;
        stream >> g.density;
        stream >> g.threshold;
        stream >> g.beam;
        grids.push_back(g);
        // QLOG_INFO() << "grid: " << g.distance << ", " << g.distanceRange;
    }
    return true;
}



void RequestRtkMessage::dumpBody(QDataStream& stream)
{
    stream << enable;
    stream << kind;
}

bool UploadRtkMessage::parseBody(QByteArray& array)
{
    QDataStream stream(&array, QIODevice::ReadOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream >> lat;
    stream >> lon;
    stream >> heading;
    stream >> stars;
    stream >> alt;
    return true;
}

void UploadRtkMessage::dumpBody(QDataStream& stream)
{
    stream << lat;
    stream << lon;
    stream << heading;
    stream << stars;
    stream << alt;
}

bool UploadStatusMessage::parseBody(QByteArray& array)
{
    QDataStream stream(&array, QIODevice::ReadOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream >> power.voltage;
    stream >> power.current;
    stream >> power.soc;
    stream >> power.remainTime;

    return true;
}


}
}
