#ifndef __AVIEW_CONFIG_H__
#define __AVIEW_CONFIG_H__

#include <qpe/config.h>
#include <qcolor.h>
#include "gconfig.h"

struct PositionInfo {
  QString       file_name;
  int           para, offset;
  PositionInfo  *next;
};

enum {
  ACFG_FontName,
  ACFG_FontSize,
  ACFG_FontBold,
  ACFG_FakeBold,
  ACFG_LineMargin,
  ACFG_Margin,
  ACFG_Rotation,
  ACFG_Background,
  ACFG_TextColor,
  ACFG_HighLight1,
  ACFG_HighLight2,
  ACFG_HighLight3,
  ACFG_HighLight4,
  ACFG_HighLight5,
  ACFG_HighLight6,
  ACFG_HighLight7,
  ACFG_BarFG,
  ACFG_BarBG,
  ACFG_BarText,
  ACFG_BarHeight,
  ACFG_BarVisible,
  ACFG_HistoryLimit,
  ACFG_LoadLastDir,
  ACFG_LoadLastFile,
  ACFG_ScrollHeight,
  ACFG_ScrollDelay,
  ACFG_SlicingMethod,
  ACFG_HSlicing,
  ACFG_VSlicing,
  ACFG_ScalingMethod,
  ACFG_ScaleFactor,
  ACFG_ScrollPolicy,
  ACFG_PagingPolicy,
  ACFG_ScaleUp,
  ACFG_HoldDelay,
  ACFG_Reserved
};

class   AViewConfigManager : public GenConfig {
  private:
    bool            m_change;

    int             m_historyLimit;
    PositionInfo    *m_position;
    QString         m_lastDir;

  private:
    AViewConfigManager();

    void writeHistory();

  public:
    ~AViewConfigManager();

    static AViewConfigManager *getInstance();

    void setLastPosition(const QString &f_name, int para, int offset);
    bool getLastPosition(const QString &f_name, int &para, int &offset);
    QString getLastFile();
    QStringList getRescentFiles();

    void setLastDir(const QString &d_path);
    QString getLastDir();

    void sync();
};

/**
 * AView configuration reference.
 */
extern AViewConfigManager   *g_cfg;

#endif
