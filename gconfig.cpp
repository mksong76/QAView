/**
 * General Configuration Manager for Qtopia
 *
 * vim:sw=2:ts=8:et
 */
#include "gconfig.h"
#include <qcolor.h>
#include <qfont.h>
#include <qstring.h>
#include <stdio.h>

struct CfgItem {
  union {
    QString *p_str;
    QColor  *p_color;
    int     v_int;
    bool    v_bool;
  }v;

  bool apply(CfgItem *cfg, int type);
};

bool
CfgItem::apply(CfgItem *cfg, int type)
{
  switch (type) {
    case GCT_STRING:
      if ((*v.p_str) == (*cfg->v.p_str)) return false;
      (*v.p_str) = (*cfg->v.p_str);
      break;
    case GCT_INT:
      if (v.v_int == cfg->v.v_int) return false;
      v.v_int = cfg->v.v_int;
      break;
    case GCT_BOOL:
      if (v.v_bool == cfg->v.v_bool) return false;
      v.v_bool = cfg->v.v_bool;
      break;
    case GCT_COLOR:
      if ((*v.p_color) == (*cfg->v.p_color)) return false;
      (*v.p_color) = (*cfg->v.p_color);
      break;
    default:
      return false;
  }
  return true;
}

GenConfig::GenConfig(const QString &name, CfgType *items)
{
  m_file = new Config(name);

  for (m_nameDB=items, m_count=0 ;
      items->type!=GCT_END ;
      items++, m_count++ );

  m_list = new CfgItem[m_count];

  int       idx;
  CfgType   *ptr = m_nameDB;
  for (idx=0 ; idx<m_count ; idx++, ptr++) {
    switch (ptr->type) {
      case GCT_GROUP:
        m_file->setGroup(ptr->name);
        break;
      case GCT_STRING:
        m_list[idx].v.p_str = new QString(m_file->readEntry(ptr->name,
                            QString(ptr->def)));
        break;
      case GCT_BOOL:
        m_list[idx].v.v_bool = m_file->readBoolEntry(ptr->name,
                            QString(ptr->def).toInt());
        break;
      case GCT_INT:
        m_list[idx].v.v_int = m_file->readNumEntry(ptr->name,
            QString(ptr->def).toInt());
        break;
      case GCT_COLOR:
        m_list[idx].v.p_color = new QColor(m_file->readEntry(ptr->name,
                            QString(ptr->def)));
        break;
    }
  }
}

static QColor   dummy_color;

QColor &
GenConfig::getColor(int id)
{
  CfgType   *ptr = m_nameDB;
  int       idx;
  for (idx=0 ; idx<m_count ; idx++, ptr++)
    if (ptr->id==id && ptr->type==GCT_COLOR)
      return *m_list[idx].v.p_color;
  return dummy_color;
}

static QString dummy_str;

QString &
GenConfig::getString(int id)
{
  CfgType   *ptr = m_nameDB;
  int       idx;
  for (idx=0 ; idx<m_count ; idx++, ptr++)
    if (ptr->id==id && ptr->type==GCT_STRING)
      return *m_list[idx].v.p_str;
  return dummy_str;
}

static int dummy_int = 0;

int &
GenConfig::getInt(int id)
{
  CfgType   *ptr = m_nameDB;
  int       idx;
  for (idx=0 ; idx<m_count ; idx++, ptr++)
    if (ptr->id==id && ptr->type==GCT_INT)
      return m_list[idx].v.v_int;
  return dummy_int;
}

static bool dummy_bool = false;

bool &
GenConfig::getBool(int id)
{
  CfgType   *ptr = m_nameDB;
  int       idx;
  for (idx=0 ; idx<m_count ; idx++, ptr++)
    if (ptr->id==id && ptr->type==GCT_BOOL)
      return m_list[idx].v.v_bool;
  return dummy_bool;
}

GenConfig *
GenConfig::copy()
{
  GenConfig *cfg = new GenConfig();

  cfg->m_name = m_name;
  cfg->m_file = NULL;
  cfg->m_nameDB = m_nameDB;
  cfg->m_count = m_count;
  cfg->m_list = new CfgItem[m_count];

  for (int idx=0 ; idx<m_count ; idx++) {
    switch (m_nameDB[idx].type) {
      case GCT_COLOR:
        cfg->m_list[idx].v.p_color = new QColor(*m_list[idx].v.p_color);
        break;
      case GCT_STRING:
        cfg->m_list[idx].v.p_str = new QString(*m_list[idx].v.p_str);
        break;
      default:
        cfg->m_list[idx].v.v_int = m_list[idx].v.v_int;
    }
  }

  return cfg;
}

bool
GenConfig::write(int id)
{
  CfgItem   *ptr1;
  CfgType   *ptr3;

  ptr1 = m_list;
  ptr3 = m_nameDB;

  int   idx;
  int   group_idx=-1;
  for (idx=0 ; idx<m_count ; idx++) {
    if (ptr3->type==GCT_GROUP) {
      group_idx = idx;
      ptr3++;
      ptr1++;
      continue;
    }

    if (ptr3->id==id) {
      if (group_idx<0) {
        return false;
      }

      m_file->setGroup(m_nameDB[group_idx].name);

      switch (ptr3->type) {
        case GCT_STRING :
          m_file->writeEntry(ptr3->name, *ptr1->v.p_str);
          break;
        case GCT_COLOR :
          m_file->writeEntry(ptr3->name, ptr1->v.p_color->name());
          break;
        case GCT_BOOL :
          m_file->writeEntry(ptr3->name, ptr1->v.v_bool);
          break;
        case GCT_INT :
          m_file->writeEntry(ptr3->name, ptr1->v.v_int);
          break;
        default:
          return false;
      }
      return true;
    }
    ptr3++;
    ptr1++;
  }
  return false;
}

void
GenConfig::apply(const GenConfig *cfg)
{
  CfgItem   *ptr1, *ptr2;
  CfgType   *ptr3;

  if (m_name!=cfg->m_name || m_nameDB!=cfg->m_nameDB)
    return;

  ptr1 = cfg->m_list;
  ptr2 = m_list;
  ptr3 = m_nameDB;

  int idx = 0;

  for (idx=0 ; idx<m_count ; idx++) {
    if (ptr3->type==GCT_GROUP) {
      m_file->setGroup(ptr3->name);
    } else if (ptr2->apply(ptr1, ptr3->type) && m_file!=NULL) {
      //printf("writeEntry(%d, %s)\n", ptr3->type, (const char*)ptr3->name);
      switch (ptr3->type) {
        case GCT_STRING :
          m_file->writeEntry(ptr3->name, *ptr1->v.p_str);
          break;
        case GCT_COLOR :
          m_file->writeEntry(ptr3->name, ptr1->v.p_color->name());
          break;
        case GCT_BOOL :
          m_file->writeEntry(ptr3->name, ptr1->v.v_bool);
          break;
        case GCT_INT :
          m_file->writeEntry(ptr3->name, ptr1->v.v_int);
          break;
      }
    }
    ptr1++;
    ptr2++;
    ptr3++;
  }
}
