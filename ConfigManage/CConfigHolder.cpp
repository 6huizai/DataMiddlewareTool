#include "CConfigHolder.h"


Q_SINGLETON_DECLARE(CConfigHolder)

CConfigHolder::CConfigHolder(QObject *parent)
{
    construct();
}

CConfigHolder::CConfigHolder(const QString &filename, QObject *parent)
    : Super(filename)
{
    construct();
}

bool CConfigHolder::load()
{
    if (!Super::load()) {
        m_data.reset();
    }
    return true;
}

void CConfigHolder::reset()
{
    m_data.reset();
}



