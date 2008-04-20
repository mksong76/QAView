/**
 * Time stamping class.
 *
 * vim:sw=2:et
 */
#ifndef __TIME_STAMP_H__
#define __TIME_STAMP_H__

class   TimeStamp
{
    long    time_buf[2];
  public:
    TimeStamp();
    TimeStamp(const TimeStamp&);

    void setToNow();
    long passed();
    long diff(const TimeStamp &s);
};

#endif
