/**
 * AView Configuration Manager.
 *
 *  vim:sw=2:et
 */
#include "aconfig.h"
#include <qcolor.h>
#include <qpe/config.h>
#include <qpe/qpeapplication.h>
#include <stdio.h>


AViewConfigManager  *g_cfg = NULL;

AViewConfigManager *
AViewConfigManager::getInstance()
{
  if (!g_cfg) {
    g_cfg = new AViewConfigManager();
  }
  return g_cfg;
}

#define CONFIG_NAME     "QAView2"

static CfgType  configs[] = {
  { GCT_GROUP,  -1,                 "Global",           NULL        },
  { GCT_STRING, ACFG_FontName,      "FontName",         "dinaru"    },
  { GCT_INT,    ACFG_FontSize,      "FontSize",         "16"        },
  { GCT_BOOL,   ACFG_FontBold,      "FontBold",         "0"         },
  { GCT_BOOL,   ACFG_FakeBold,      "FakeBold",         "1"         },
  { GCT_INT,    ACFG_LineMargin,    "LineMargin",       "5"         },
  { GCT_INT,    ACFG_Margin,        "Margin",           "10"        },
  { GCT_COLOR,  ACFG_Background,    "Background",       "#000000"   },
  { GCT_COLOR,  ACFG_TextColor,     "TextColor",        "#ffffff"   },
  { GCT_COLOR,  ACFG_HighLight1,    "HighLight1",       "#ff0000"   },
  { GCT_COLOR,  ACFG_HighLight2,    "HighLight2",       "#0000ff"   },
  { GCT_COLOR,  ACFG_HighLight3,    "HighLight3",       "#00ff00"   },
  { GCT_COLOR,  ACFG_HighLight4,    "HighLight4",       "#ffff00"   },
  { GCT_COLOR,  ACFG_HighLight5,    "HighLight5",       "#00ffff"   },
  { GCT_COLOR,  ACFG_HighLight6,    "HighLight6",       "#ff00ff"   },
  { GCT_COLOR,  ACFG_HighLight7,    "HighLight7",       "#888888"   },
  { GCT_COLOR,  ACFG_BarFG,         "BarFG",            "#225522"   },
  { GCT_COLOR,  ACFG_BarBG,         "BarBG",            "#333333"   },
  { GCT_COLOR,  ACFG_BarText,       "BarText",          "#ffffff"   },
  { GCT_INT,    ACFG_BarHeight,     "BarHeight",        "10"        },
  { GCT_BOOL,   ACFG_BarVisible,    "BarVisible",       "1"         },
  { GCT_INT,    ACFG_HistoryLimit,  "HistoryLimit",     "10"        },
  { GCT_BOOL,   ACFG_LoadLastDir,   "LoadLastDir",      "0"         },
  { GCT_BOOL,   ACFG_LoadLastFile,  "LoadLastFile",     "1"         },
  { GCT_INT,    ACFG_Rotation,      "Rotation",         "0"         },
  { GCT_INT,    ACFG_ScrollHeight,  "ScrollHeight",     "1"         },
  { GCT_INT,    ACFG_ScrollDelay,   "ScrollDelay",      "3"         },
  { GCT_INT,    ACFG_SlicingMethod, "SlicingMethod",    "2"         },
  { GCT_INT,    ACFG_HSlicing,      "HSlicing",         "1"         },
  { GCT_INT,    ACFG_VSlicing,      "VSlicing",         "1"         },
  { GCT_INT,    ACFG_ScalingMethod, "ScalingMethod",    "2"         },
  { GCT_INT,    ACFG_ScaleFactor,   "ScaleFactor",      "100"       },
  { GCT_INT,    ACFG_ScrollPolicy,  "ScrollPolicy",     "3"         },
  { GCT_INT,    ACFG_PagingPolicy,  "PagingPolicy",     "3"         },
  { GCT_BOOL,   ACFG_ScaleUp,       "ScaleUp",          "0"         },
  { GCT_BOOL,   ACFG_HoldDelay,     "HoldDelay",        "500"       },
  { GCT_END, -1, NULL, NULL }
};

AViewConfigManager::AViewConfigManager() :
  GenConfig(CONFIG_NAME, configs)
{
  m_change = false;

  m_file->setGroup("Global");
  m_lastDir = m_file->readEntry("LastDir", QPEApplication::documentDir());

  m_file->setGroup("History");

  PositionInfo  *last = NULL, *new_one;
  QStringList   list;
  QString       key;
  int           idx = 0, limit = getInt(ACFG_HistoryLimit);

  m_position = NULL;
  do {
    key.sprintf("File%d", idx++);
    list = m_file->readListEntry(key, ',');
    if (list.count()<1) break;
    new_one = new PositionInfo;
    new_one->file_name = list[0];
    new_one->para = list[1].toInt();
    new_one->offset = list[2].toInt();

    new_one->next = NULL;
    if (last)
      last->next = new_one;
    else
      m_position = new_one;
    last = new_one;
  } while(idx<limit);
}

AViewConfigManager::~AViewConfigManager()
{
  sync();
}
void
AViewConfigManager::writeHistory()
{
  if (!m_change) return;

  m_file->setGroup("Global");
  m_file->writeEntry("LastDir", m_lastDir);
  m_file->setGroup("History");
  m_file->clearGroup();

  PositionInfo  *ptr = m_position;
  int           idx = 0;
  QString       key, value;
  QStringList   lst;
  while (ptr) {
    key.sprintf("File%d", idx++);
    lst.clear();
    lst.append(ptr->file_name);
    lst.append(QString::number(ptr->para));
    lst.append(QString::number(ptr->offset));
    m_file->writeEntry(key, lst, ',');
    ptr = ptr->next;
  }
  m_change = false;
}

static QString
getDir(const QString &dir)
{
  int   idx;
  idx = dir.findRev('/');
  if (idx<0) {
    return (QString)"";
  } if (idx==0) {
    return (QString)"/";
  }
  return dir.left(idx);
}

void
AViewConfigManager::setLastPosition(const QString &f_name, int para, int offset)
{
  PositionInfo  *ptr = m_position, *pre=NULL;
  int       idx_count = 0;

  printf("setLastPosition:%s,%d,%d\n",
      (const char*)f_name.utf8(), para, offset);

  /* search for previous history */
  while (ptr) {
    if (getDir(f_name)==getDir(ptr->file_name)) {
      if (pre)
        pre->next = ptr->next;
      else
        m_position = ptr->next;
      ptr->file_name = f_name;
      ptr->para = para;
      ptr->offset = offset;
      ptr->next = NULL;
      break;
    }
    idx_count++;
    pre = ptr;
    ptr = ptr->next;
  }

  /* create if there is no history !! */
  if (!ptr) {
    ptr = new PositionInfo;
    ptr->file_name = f_name;
    ptr->para = para;
    ptr->offset = offset;
    idx_count++;
  }
  /* connect this to first!! */
  ptr->next = m_position;
  m_position = ptr;

  /* clean up limited... */
  int   history_limit = getInt(ACFG_HistoryLimit);
  if (idx_count>history_limit) {
    pre = NULL;
    ptr = m_position;
    for (idx_count=0 ; idx_count<history_limit ; idx_count++) {
      pre = ptr;
      ptr = ptr->next;
    }
    pre->next = NULL;
    while(ptr) {
      pre = ptr;
      ptr = ptr->next;
      delete pre;
    }
  }
  m_change = true;

  /* write configuration if it changed last position */
  sync();
}

QStringList
AViewConfigManager::getRescentFiles()
{
  PositionInfo  *ptr = m_position;
  QStringList   to_return;
  while (ptr) {
    to_return.append(ptr->file_name);
    ptr = ptr->next;
  }
  return to_return;
}

bool
AViewConfigManager::getLastPosition(const QString &f_name,
    int &para, int &offset)
{
  PositionInfo  *ptr = m_position;
  /* search for previous history */
  while (ptr) {
    if (f_name==ptr->file_name) {
      para = ptr->para;
      offset = ptr->offset;
      printf("getLastPosition:%s,%d,%d\n",
          (const char*)f_name.utf8(), para, offset);
      return true;
    }
    ptr = ptr->next;
  }
  para = 0;
  offset = 0;
  return false;
}

QString
AViewConfigManager::getLastFile()
{
  if (m_position) {
    return m_position->file_name;
  }
  return QString();
}

void
AViewConfigManager::setLastDir(const QString &d_path)
{
  m_lastDir = d_path;
  m_change = true;
}

QString
AViewConfigManager::getLastDir()
{
  return m_lastDir;
}

void
AViewConfigManager::sync()
{
  if (m_change) writeHistory();
  delete m_file;
  //printf("AViewConfigManager::sync~~\n");

  m_file = new Config(CONFIG_NAME);
}
