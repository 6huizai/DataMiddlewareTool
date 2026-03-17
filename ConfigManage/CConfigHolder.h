#ifndef CCONFIGHOLDER_H
#define CCONFIGHOLDER_H

#include <QObject>
#include "CConfigData.h"
#include "FileHolder.h"
#include "JsonHolder.h"
#include "Macros.h"

#define qConfig CConfigHolder::instance()
#define qConfigCData qConfig->constData()
#define qConfigData qConfig->data()

class CConfigHolder : public FileHolder, public JsonHolder<CConfigData>
{
    Q_OBJECT
    Q_SINGLETON(CConfigHolder)

private:
    using Super = JsonHolder<CConfigData>;

public:
    explicit CConfigHolder(QObject *parent = nullptr);
    explicit CConfigHolder(const QString &path, QObject *parent = nullptr);



    bool load() override;
    void reset() override;
};

#endif // CCONFIGHOLDER_H
