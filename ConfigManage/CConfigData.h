#ifndef CCONFIGDATA_H
#define CCONFIGDATA_H

#include <AIGCJson.hpp>
#include <QMetaType>

class CConfigData
{
public:
    class System {
    public:
        int translation;
        AIGC_JSON_HELPER(translation)
        AIGC_JSON_HELPER_DEFAULT(translation = 0)
    };

public:
    void reset();

public:
    System system;
    AIGC_JSON_HELPER(system)
};

Q_DECLARE_METATYPE(CConfigData);
Q_DECLARE_METATYPE(CConfigData::System);



#endif // CCONFIGDATA_H
