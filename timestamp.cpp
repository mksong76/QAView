/**
 * vim:sw=2:et
 */
#include "timestamp.h"
#include <sys/time.h>
#include <stdio.h>


TimeStamp::TimeStamp()
{
  time_buf[0] = 0;
  time_buf[1] = 0;
}

TimeStamp::TimeStamp(const TimeStamp&x)
{
  time_buf[0] = x.time_buf[0];
  time_buf[1] = x.time_buf[1];
}

void
TimeStamp::setToNow()
{
  gettimeofday((timeval*)&time_buf, NULL);
}

long
TimeStamp::diff(const TimeStamp &s)
{
  return (time_buf[0]-s.time_buf[0])*1000+(time_buf[1]-s.time_buf[1])/1000;
}

long
TimeStamp::passed()
{
  timeval   t_temp;
  gettimeofday((timeval*)&t_temp, NULL);
  return (t_temp.tv_sec-time_buf[0])*1000+(t_temp.tv_usec-time_buf[1])/1000;
}
