#include "battery.h"
#include <qfile.h>

MyBattery::MyBattery()
{
    m_acStatus = PowerStatus::Offline;
    m_percent = 0;
}

int
MyBattery::batteryPercentRemaining()
{
    return m_percent;
}

int
MyBattery::acStatus()
{
    return m_acStatus;
}

const MyBattery &
MyBattery::operator = (const MyBattery &bat)
{
    m_acStatus = bat.m_acStatus;
    m_percent = bat.m_percent;

    return *this;
}

bool
MyBattery::updateStatus()
{
    QFile   my_file("/proc/apm");

    if (!my_file.open(IO_ReadOnly))
        return false;

    QString str;
    if(my_file.readLine(str, 100)<1)
        return false;

    float   buf_float[2];
    int     buf_int[6];

    memset(buf_float, 0, sizeof(buf_float));
    memset(buf_int, 0, sizeof(buf_int));

    /*
    printf("STR:%s", (const char*)str);
    */
    sscanf(str, "%f %f 0x%02x 0x%02x 0x%02x 0x%02x %d%% %d",
            &buf_float[0], &buf_float[1],
            &buf_int[0], &buf_int[1], &buf_int[2], &buf_int[3],
            &buf_int[4], &buf_int[5] );
    /*
    printf("%f %f %d %d %d %d %d %d\n",
            buf_float[0], buf_float[1],
            buf_int[0], buf_int[1], buf_int[2], buf_int[3], buf_int[4],
            buf_int[5]);
            */

    m_acStatus = buf_int[1];
    if (m_acStatus!=0)
        m_percent = 100;
    else
        m_percent = buf_int[4];

    my_file.close();

    return true;
}
