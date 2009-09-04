#include "filelister.h"
#include "qzdir.h"
#include "qzfile.h"
#include <stdio.h>
#include <stdio.h>

FILE_TYPE
FileLister::getType(const QString &path, FILE_ENCODING &enc)
{
    QZFile  fd(path);
    char buf[10];
    int readed, aword;
    if (!fd.open()) return FT_UNKNOWN;
    memset(buf, 0, sizeof(buf));
    readed = fd.read(buf, 9);
    if (readed<0) return FT_UNKNOWN;

    aword = (buf[0]<<24) | (buf[1]<<16) | (buf[2]<<8) | buf[3];
    switch (aword) {
        case 0x89504e47:    // PNG
        case 0xffd8ffe0:    // JPG
        case 0x47494638:    // GIF
            enc = FE_AUTO;
            return FT_IMAGE;
    }

    int s_idx, p_idx;
    QString suffix;
    s_idx = path.findRev('.');
    if (s_idx<0)
        return FT_UNKNOWN;
    p_idx = path.findRev('/');
    if (p_idx>s_idx)
        return FT_UNKNOWN;
    suffix = path.mid(s_idx).lower();
    if (suffix==".jpg" || suffix==".jpeg" ||
            suffix==".gif" || suffix==".png") {
        enc = FE_AUTO;
        return FT_IMAGE;
    }
    if (suffix==".cap" || suffix==".txt") {
        enc = FE_AUTO;
        return FT_TEXT;
    }
    return FT_UNKNOWN;
}

FileLister::FileLister()
{
    m_type = FT_UNKNOWN;
}

int
FileLister::find(const QString &fname)
{
    if (FT_AUTO==m_type)
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
    FILE_TYPE       new_type;
    FILE_ENCODING   new_enc;

    new_type = FileLister::getType(path, new_enc);
    if (FT_UNKNOWN==new_type) return -1;

    dname = QZDir::getDir(path, &fname);
    if (dname==dir.path())
        return find(fname);
    dir.setPath(dname);
    if (!dir.isDir()) return -1;

    QStringList flist;
    flist = dir.getList("*.*", QZFL_FILE, QZFS_NAME);

    int idx = 0, file_idx = -1;
    FILE_TYPE       fname_type;
    FILE_ENCODING   fname_enc;
    QString     fname_ptr;
    QStringList::Iterator    itr;
    itr = flist.begin();
    while (itr!=flist.end()) {
        fname_type = FileLister::getType(dname+"/"+*itr, fname_enc);
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
    if (m_type==FT_UNKNOWN) {
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
