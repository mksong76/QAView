#ifndef __BATTERY_STATUS_H__
#define __BATTERY_STATUS_H__

#include <qpe/power.h>

class   MyBattery {
    private:
        int     m_acStatus;
        int     m_percent;

    public:

        MyBattery();
        const MyBattery &operator = (const MyBattery &bat);

        int     batteryPercentRemaining();
        int     acStatus();

        bool updateStatus();
};

#endif
