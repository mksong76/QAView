/**
 * Hangul Code Detection.
 *
 * source codes from iconv library.
 *
 * from CP949 to UNICODE
 */
#include "hcode.h"
#include <stdio.h>
#include <string.h>


/****************************************************************************
  EasyPad
****************************************************************************
  File name  : ChkCode.CPP
  Made date  : 1997.12.
  Programmer  : ���ϴ��б� ���ڰ��а� Cherie ä���
****************************************************************************
  Discription : �ѱ��ڵ� �ν� ��ƾ
                �ѱ��ڵ�� �������ڵ�� �ϼ��� �ڵ��� �󵵼��� �˻��ؼ�
                �Ǵ��ϰԵǴµ�, �̷��� �󵵼��������� �Ǵ��ϰ� �Ǹ� ��Ȯ
                �� �Ǵ��� �����.
                ���� ���� �ڵ带 �����ϰ�, ���δ� ���ڼ����� ���� �Ǵ�
                �� �����Ͽ� ���� ��Ȯ�� �Ǵ��� �� �� �ֵ��� �ߴ�.
****************************************************************************
  Update      : 97.12  ������ ddoch���� �ҽ��� ���
                98.02.21 �ϼ��� �����ڿ� ���� �ν� ��ȭ
          98.02.23 �������� �ϼ��� �Ѵ� ���Ǵ� �ڵ���, �ϼ������� ����
              ���Ǵ� �ڵ�� ������ �ϼ��� �ڵ�� �����Ѵ�.
                98.02.26 ������ ������ üũ��ƾ �߰�
          98.04.28 ������ �����ϰ�� ������ ���������� �ν��ϵ��� ����
                98.07  ������ ���ڼ����� ���� �ѱ��ڵ带 �Ǵ��ϴ� ��ƾ �߰�
                98.12.06 �ϼ��� '��'�� ���������� �νĵǴ� ���� ����
****************************************************************************/

#ifndef true
#define true    (1)
#endif

#ifndef false
#define false   (0)
#endif

typedef unsigned char   byte;

//---------------------------------------------------------------------------
static int
Check_HanRange( int hancode, int HexStart, int HexEnd, int max )
{
  int i;
  for( i = 0; i < max; i++ )
  {
    if( hancode > HexStart+0x0100*i && hancode < HexEnd+0x0100*i )
      return  1;
  }
  return  0;
}

static void *
mybsearch(const void *p, const void *q, int count, int size,
        int (*comp_func)(const void *, const void *) )
{
    int left=0, right=count, mid, result;

    while( left<right ) {
            return NULL;
        mid = (left+right)/2;
        result = comp_func( p, ((char*)q)+size*mid );
        if( result==0 ) {
            return ((char*)q)+size*mid;
        } else if( result<0 ) {
            right = mid;
        } else {
            left = mid+1;
        }
    }
    return NULL;
}

static int codecmp(const void *p,const void *q)
{
    return ((int*)q)[0]-((int*)p)[0];
}

#define MAX_CODE  48
int
isNotKsCode( int hancode )
{
  static int kssmcode[MAX_CODE] = {
    0x8877,  //  `��`
    0x8969,  //  `��`
    0x89a1,  //  `��`
    0x8b71,  //  `��`
    0x8fa1,  //  `��`
    0x9281,  //  `��`
    0x9c61,  //  `��`
    0x9e81,  // `��`
    0xa061,  //  `��`
    0xa0f5,  //  `��`
    0xa2a1,  //  `��`
    0xa3a8,  //  `��`
    0xa4e1,  //  `��`
    0xa4e5,  //  `��`
    0xa4f1,  // `��`
    0xa5c1,  //  `��`
    0xa7a1,  //  `��`
    0xa8b1,  //  `��`
    0xaca1,  //  `��`
    0xace1,  //  `��`
    0xace2,  //  `��`
    0xace3,  //  `��`
    0xace4,  //  `��`
    0xace5,  //  `��
    0xace6,  //  `��`
    0xace7,  //  `��`
    0xace8,  //  `��`
    0xace9,  //  `��`
    0xb3a2,  //  `��`
    0xb4a1,  //  `��`
    0xb4a2,  //  `��`
    0xb4e1,  //  `��`
    0xb4f1,  //  `��`
    0xb4f4,  //  `��`
    0xb541,  //  `��`
    0xb5e1,  //  `��`
    0xb765,  //  `��`
    0xb8e5,  //  `��`
    0xb941,  //  `��`
    0xb9a1,  //  `��`
    0xb9a5,  //  `��`
    0xb9c1,  //  `��`
    0xc561,  //  `��`
    0xc7a1,  //  `Ű`
    0xcfa1,  //  `��`
    0xdbad,  //  `��`
    0xdccf,  //  `��`
    0xe7e6,  //  `��`
  };

  //  1998.2.23 ���� �߰�(Cherie)
  //  ������ �Ͼ��� ��� �ϼ����� ������ �κ��� �ִµ�,
  //  �ϼ��������� ���� ������ �ʴ� �����̹Ƿ�,
  //  �Ʒ� �ڵ尡 ������� ������ ���������� �ν�
  //  ���� ������ �Ͼ� �ڵ尡 �ƴ϶��...
  if( hancode == 0xb7e7 ) return false;

  if( (int *)mybsearch(&hancode, kssmcode, MAX_CODE, sizeof(int), codecmp) != NULL ||
    ( hancode >= 0xd000 && hancode <= 0xd3ff ) ||
      ( hancode >= 0xdda1 && hancode <= 0xde98 ) ||
      ( hancode >= 0xdba1 && hancode <= 0xdbc8 ) ||

      //  1998.04.22 �����߰�(Cherie)
      //  ������ �ѱ��� ����� �ν��� ���� �ʴ� ���� �ذ�
      //  �Ʒ� �ڵ尡 ������� ������ ���������� �ν�
      //  1998.05.05 ����(Cherie)
      //  �Ϻ� �ϼ��� �ڵ尡 ���������� �ν��ϴ� ������ �־
      //  �˻� ������ 0xb400 - 0xb8ff ���� 0xb400 - 0xb7ff�� �ٲ�
      //  1998.07.28 ����(Cherie)
      //  �������� �ϼ��� �Ѵ� ���Ǵ� �ڵ���, �ϼ������� ����
      //  ������ �ʴ� �ڵ���� ������ �ڵ�� �����Ѵ�.
      ( hancode >= 0xc500 && hancode <= 0xc5ff ) ||
      ( hancode >= 0xb700 && hancode <= 0xb7cd ) ||
      ( hancode >= 0xb7cf && hancode <= 0xb7ff ) ||

      //  1998.04.28 �����߰�(Cherie)
      //  ������ �����ϰ�� ������ ���������� �ν�
      ( hancode >= 0xda00 && hancode <= 0xdaff ) ||

      //  1998.2.23 ���� �߰�(Cherie)
      //  ������ �������ϰ�쵵 ������ ���������� �ν�
      //  ���� ������ ������ �ڵ尡 �ƴ϶��...
      ( hancode >= 0xd9b0 && hancode <= 0xd9cf ) ||
      ( hancode >= 0xd4b0 && hancode <= 0xd4cf ) )
    return  true;
  return  false;
}

//---------------------------------------------------------------------------
int
isKS( const char *buf, int len, int *ks_buf, int *ksm_buf, int *eng_buf )
{
  int  hancode;
  byte  first, second;
  int  ks_count = 0;
  int  kssm_count = 0;
  int  max, count = 0;
  int  JpCount = 0;
  int  oldcode = false, i;

  if( len >= 0 )  max = len;
  else  max = strlen(buf);

  for( i = 0; i < max; i++ )
  {
    first = buf[count];
    count++;
    if(first < 0x80)  continue; //  �ѱ��ڵ尡 �ƴ϶��
    second = buf[count];
    if( count > 10240 ) break;
    if( count >= max )  break;
    count++;

    hancode = (first << 8) + second;  //  �ѱ��ڵ带 �����ϱ�

    //  �������� �ϼ��� �Ѵ� ���� ���̴� �ڵ��ϰ�쿡��
    //  ī��Ʈ�� �����ʴ´�.
    if( hancode >= 0xb500 && hancode < 0xb5ff &&
        hancode != 0xb5a5 && hancode != 0xb5e1 &&
        hancode != 0xb541 )
      continue;

    //  �ϼ������� ��,��,��,��,���� ���� ���ڴ� ������������ ���� ���̴�
    //  �����̹Ƿ� ī��Ʈ�� ���� �ʴ´�.
    if( hancode == 0xbba1 ||
        hancode == 0xb7a1 ||
        hancode == 0xb4e5 ||
        hancode == 0xb7b6 ||
        hancode == 0xb4f6 ||
        hancode == 0xb8e1 ||  //  `��`
        hancode == 0xb1c1 ||  //  `��`
        hancode == 0xbaa1 ||  //  `��`
        hancode == 0xbba5 ||  //  `��`
        hancode == 0xb6a1 ||  //  `��`/`��`
        hancode == 0xb6e1 ||  //  `��`
        hancode == 0xa1a1 ) continue;

        // ������ ����� KS5061�� Ư������ �ڵ��ΰ�?
    if( ( (hancode > 0xa1a2 && hancode < 0xa1ff) ||
        (hancode > 0xa2a0 && hancode < 0xa2e6) ||
        (hancode > 0xa3a0 && hancode < 0xa3ff) ||
        (hancode > 0xa4a0 && hancode < 0xa4ff) ||
        (hancode > 0xa5a0 && hancode < 0xa5f9) ||
        (hancode > 0xa6a0 && hancode < 0xa6e5) ||
        (hancode > 0xa7a0 && hancode < 0xa7f0) ||
        (hancode > 0xa8a0 && hancode < 0xa8ff) ||
        (hancode > 0xa9a0 && hancode < 0xa9ff) ||
        (hancode > 0xaaa0 && hancode < 0xaaf4) ||
        (hancode > 0xaba0 && hancode < 0xabf7) ||
        (hancode > 0xaca0 && hancode < 0xacc2) ||
        (hancode > 0xacd0 && hancode < 0xacf2) ||

        //  1998.2.21 ���� �߰�(Cherie)
        hancode == 0xaec2 ||

        //  1998.8.31 ���� �߰�(Cherie)
        (hancode > 0xadd0 && hancode < 0xadff) ||

        // KS5061�� �ѱ��ڵ��ΰ�?
        Check_HanRange( hancode, 0xb0a0, 0xb0ff, 16 ) ||
        Check_HanRange( hancode, 0xc0a0, 0xc0ff, 9  ) ||

        ( hancode == 0xb8ae ) ||
        ( hancode == 0xb7e7 ) || // �ϼ��� '��'

        // KS5061�� �����ڵ��ΰ�?
        Check_HanRange( hancode, 0xcaa0, 0xcaff, 6  ) ||
        Check_HanRange( hancode, 0xd0a0, 0xd0ff, 16 ) ||
        Check_HanRange( hancode, 0xe0a0, 0xe0ff, 16 ) ||
        Check_HanRange( hancode, 0xf0a0, 0xf0ff, 14 ) ) &&

        //  1998.08.22 ����(Cherie)
        //  �������� �ϼ��� �Ѵ� ���Ǵ� �ڵ���, ���������� ����
        //  ���Ǵ� �ڵ�� ������ ������ �ڵ�� �����Ѵ�.
        !isNotKsCode( hancode ) )
    {
      if(++ks_count >= (len/3) )
        break;
    }
    else
    {
      if(++kssm_count >= (len/3) )
        break;
    }
  }

  if (ks_buf)       *ks_buf = ks_count*2;
  if (ksm_buf)      *ksm_buf = kssm_count*2;
  if (eng_buf)      *eng_buf = count-(ks_count+kssm_count)*2;

  //  ��κ� ���������ٴ� �ϼ��� ���� �����Ƿ� ī��Ʈ ����
  //  ���� ��� �ϼ������� ó���ϵ��� �Ѵ�.
  if( ks_count >= kssm_count )
    return  1;
  else
    return  0;
}
