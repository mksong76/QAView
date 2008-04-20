/**
 * General configuration manager.
 *
 * vim:sw=2:ts=8:et
 */
#ifndef __GENERAL_CONFIG_H__
#define __GENERAL_CONFIG_H__

#include <qpe/config.h>

class QColor;
class QFont;
class QString;

enum { GCT_GROUP, GCT_STRING, GCT_BOOL, GCT_INT, GCT_COLOR, GCT_END };

struct CfgType {
  int           type;
  int           id;
  const char    *name;
  const char    *def;
};

struct CfgItem;

class   GenConfig
{
  protected:
    QString m_name;
    Config  *m_file;

    CfgItem *m_list;
    CfgType *m_nameDB;
    int     m_count;

    GenConfig() { }
  public:
    GenConfig(const QString &name, CfgType *);

    QColor &getColor(int id);
    QString &getString(int id);
    int &getInt(int id);
    bool &getBool(int id);
    bool write(int id);

    GenConfig *copy();
    void apply(const GenConfig *cfg);
};

#endif
