#ifndef QBLUEZ_LOADADAPTERSJOB_H
#define QBLUEZ_LOADADAPTERSJOB_H

#include <QObject>

#include "job.h"
#include "qbluez_export.h"

namespace QBluez
{

class ManagerPrivate;

class QBLUEZ_EXPORT LoadAdaptersJob : public Job
{
    Q_OBJECT

public:
    LoadAdaptersJob(ManagerPrivate *manager, QObject *parent = 0);

Q_SIGNALS:
    void result(LoadAdaptersJob *job);

private:
    void doStart() Q_DECL_OVERRIDE;
    void doEmitResult() Q_DECL_OVERRIDE;

    class LoadAdaptersJobPrivate *d;
    friend class LoadAdaptersJobPrivate;
};

} // namespace QBluez

#endif // QBLUEZ_LOADADAPTERSJOB_H
