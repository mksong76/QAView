#include "para.h"
#include <stdio.h>

Link *
Link::findLink(int offset)
{
  Link *ptr = this;
  while (ptr) {
    if (ptr->sidx<=offset && ptr->eidx>offset) {
      break;
    }
    ptr = ptr->next;
  }
  return ptr;
}

ParaData::ParaData(int len)
{
  m_ref = 1;
  m_len = len;
  m_ind = 0;
  m_link = NULL;
  m_attr = new Attr[len];
  m_lines = 0;
  memset(m_attr, 0, len*sizeof(Attr));
}

ParaData::~ParaData()
{
  delete m_attr;
  delete m_link;
}


static QString empty_str = "";
static Attr    empty_attr[1];

Paragraph::Paragraph()
{
  m_para = NULL;
  m_attr = empty_attr;
  m_str = &empty_str;
}

Paragraph::Paragraph(const Paragraph&p)
{
  if (p.m_para) {
    m_para = p.m_para;
    m_str = &p.m_para->m_str;
    m_attr = p.m_para->m_attr;
    this->grab();
  } else {
    m_para = NULL;
    m_str = &empty_str;
    m_attr = empty_attr;
  }
}

Paragraph::Paragraph(int len)
{
  if (len<1) {
    m_para = NULL;
    m_attr = NULL;
    m_str = &empty_str;
    return;
  }

  m_para = new ParaData(len);
  m_str = &m_para->m_str;
  m_attr = m_para->m_attr;
}

/**
 * increase reference
 */
void
Paragraph::grab()
{
  if (m_para) m_para->m_ref++;
}

void
Paragraph::release()
{
  if (m_para){
    m_para->m_ref--;
    if (m_para->m_ref==0)
      delete m_para;
    m_para = NULL;
    m_str = &empty_str;
    m_attr = empty_attr;
  }
}


/**
 * Decrease reference.
 * it will automatically clean up the memory if it has no reference.
 */
Paragraph::~Paragraph()
{
  release();
}

void
Paragraph::addLink(int s, int e, QString st)
{
  if (m_para) {
    Link *n = new Link(s, e, st);
    n->next = m_para->m_link;
    m_para->m_link = n;
  }
}

Link *
Paragraph::findLink(int s)
{
  if (m_para && m_para->m_link)
    return m_para->m_link->findLink(s);
  return NULL;
}

bool
Paragraph::empty()
{
  return m_para==NULL;
}

bool
Paragraph::operator ()()
{
  return !empty();
}


Paragraph &
Paragraph::operator = (const Paragraph &p)
{
  this->release();
  if (p.m_para) {
    m_para = p.m_para;
    m_str = &m_para->m_str;
    m_attr = m_para->m_attr;
    this->grab();
  }

  return *this;
}

