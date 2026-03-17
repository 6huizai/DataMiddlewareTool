#ifndef HPMESSAGE_H
#define HPMESSAGE_H
#include <QObject>
#include <QDataStream>

class HPTrackMessage
{
public:
    struct Point {
        qint32 id;
        qint64 timeStamp;
        float distance;
        float azimuth;
        float pitch;
        float speed;
        float heading;
        double lon;
        double lat;
        double height;
        float targetStrength;
        qint32 trackingCount;
        qint32 lossCount;
        qint32 deviceId;
        uchar type;
        float rcs;
        char reserve[10];
        char check;
    };
public:
    quint16 head1 = 0xFF77;
    quint16 head2 = 0xCCCC;
    quint16 head3 = 0x77FF;
    std::vector<Point> points;

    QByteArray dumpBody()
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream << head1;
        stream << head2;
        stream << head3;
        stream << (int32_t)points.size();

        for (int i=0; i<points.size(); i++)
        {
            Point p = points[i];
            stream << p.id;
            stream << p.timeStamp;
            stream << p.distance;
            stream << p.azimuth;
            stream << p.pitch;
            stream << p.speed;
            stream << p.heading;
            stream << p.lon;
            stream << p.lat;
            stream << p.height;
            stream << p.targetStrength;
            stream << p.trackingCount;
            stream << p.lossCount;
            stream << p.deviceId;
            stream << p.type;
            stream << p.rcs;
            stream.writeRawData(p.reserve, 10);
            stream << p.check;
        }
        return data;
    }
};


#endif // HPMESSAGE_H
