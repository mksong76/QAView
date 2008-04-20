#include "filelister.h"
#include "qzdir.h"
#include <stdio.h>
#include <stdio.h>

int
FileLister::getType(const QString &path)
{
    int s_idx, p_idx;
    QString suffix;
    s_idx = path.findRev('.');
    if (s_idx<0)
        return FLT_UNKNOWN;
    p_idx = path.findRev('/');
    if (p_idx>s_idx)
        return FLT_UNKNOWN;
    suffix = path.mid(s_idx).lower();
    if (suffix==".jpg" || suffix==".jpeg" ||
            suffix==".gif" || suffix==".png")
        return FLT_IMAGE;
    if (suffix==".cap" || suffix==".txt")
        return FLT_TEXT;
    return FLT_UNKNOWN;
}

FileLister::FileLister()
{
    m_type = FLT_UNKNOWN;
}

int
FileLister::find(const QString &fname)
{
    if (FLT_UNKNOWN==m_type)
        return -1;
    for (int idx=0 ; idx<m_list.count() ; idx++) {
        if (m_list[idx]==fname)
            return idx;
    }
    return -1;
}

int
FileLister::findPath(const QString &path)
{
    QString fname, dname;
    dname = QZDir::getDir(path, &fname);
    if (dname!=m_dir.path()) {
        return -1;
    }
    return find(fname);
}

int
FileLister::setFile(const QString &path)
{
    QString fname, dname;
    QZDir   dir;
    int     new_type;

    new_type = FileLister::getType(path);
    if (FLT_UNKNOWN==new_type) return -1;

    dname = QZDir::getDir(path, &fname);
    if (dname==dir.path())
        return find(fname);
    dir.setPath(dname);
    if (!dir.isDir()) return -1;

    QStringList flist;
    flist = dir.getList("*.*", QZFL_FILE, QZFS_NAME);

    int idx = 0, fname_type, file_idx = -1;
    QString     fname_ptr;
    QStringList::Iterator    itr;
    itr = flist.begin();
    while (itr!=flist.end()) {
        fname_type = FileLister::getType(dname+"/"+*itr);
        if (fname_type==new_type) {
            itr++;
        } else {
            itr = flist.remove(itr);
        }
    }
    m_list = flist;
    m_dir = dir;
    m_type = new_type;

    m_list.sort();

    return find(fname);
}

QString
FileLister::getFile(int idx)
{
    if (m_type==FLT_UNKNOWN) {
        printf("FileLister::getFile(%d) is UNKNOWN\n", idx);
        return NULL;
    }
    if (idx<0 || idx>=m_list.count()) {
        printf("FileLister::getFile(%d) is OUT OF RANGE(0,%d)\n",
                idx, m_list.count()-1);
        return NULL;
    }
    /*
    printf("FileLister::getFile(%d) -> %s\n",
            idx, (const char*)((QString)m_list[idx]).utf8());
            */
    return m_dir.absPath()+"/"+(QString)m_list[idx];
}

int
FileLister::count()
{
    return m_list.count();
}
