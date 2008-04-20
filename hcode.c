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
  Programmer  : 전북대학교 전자공학과 Cherie 채경삼
****************************************************************************
  Discription : 한글코드 인식 루틴
                한글코드는 조합형코드와 완성형 코드의 빈도수를 검사해서
                판단하게되는데, 이렇게 빈도수만가지고 판단하게 되면 정확
                한 판단이 힘들다.
                따라서 전후 코드를 참조하고, 때로는 글자순서에 따른 판단
                을 가미하여 좀더 정확한 판단을 할 수 있도록 했다.
****************************************************************************
  Update      : 97.12  하이텔 ddoch님의 소스를 사용
                98.02.21 완성형 선문자에 대한 인식 강화
          98.02.23 조합형과 완성형 둘다 사용되는 코드중, 완성형에서 자주
              사용되는 코드는 강제로 완성형 코드로 간주한다.
                98.02.26 조합형 선문자 체크루틴 추가
          98.04.28 조합형 모음일경우 무조건 조합형으로 인식하도록 했음
                98.07  문장의 글자순서에 따라서 한글코드를 판단하는 루틴 추가
                98.12.06 완성형 '루'가 조합형으로 인식되는 버그 수정
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
    0x8877,  //  `강`
    0x8969,  //  `결`
    0x89a1,  //  `고`
    0x8b71,  //  `금`
    0x8fa1,  //  `끼`
    0x9281,  //  `누`
    0x9c61,  //  `라`
    0x9e81,  // `루`
    0xa061,  //  `마`
    0xa0f5,  //  `멋`
    0xa2a1,  //  `뭐`
    0xa3a8,  //  `믿`
    0xa4e1,  //  `버`
    0xa4e5,  //  `번`
    0xa4f1,  // `범`
    0xa5c1,  //  `봐`
    0xa7a1,  //  `비`
    0xa8b1,  //  `뺨`
    0xaca1,  //  `샤`
    0xace1,  //  `서`
    0xace2,  //  `서`
    0xace3,  //  `서`
    0xace4,  //  `서`
    0xace5,  //  `선
    0xace6,  //  `서`
    0xace7,  //  `서`
    0xace8,  //  `서`
    0xace9,  //  `서`
    0xb3a2,  //  `씩`
    0xb4a1,  //  `야`
    0xb4a2,  //  `약`
    0xb4e1,  //  `어`
    0xb4f1,  //  `엄`
    0xb4f4,  //  `없`
    0xb541,  //  `에`
    0xb5e1,  //  `왜`
    0xb765,  //  `은`
    0xb8e5,  //  `전`
    0xb941,  //  `제`
    0xb9a1,  //  `조`
    0xb9a5,  //  `존`
    0xb9c1,  //  `좌`
    0xc561,  //  `켜`
    0xc7a1,  //  `키`
    0xcfa1,  //  `피`
    0xdbad,  //  `┃`
    0xdccf,  //  `ⓒ`
    0xe7e6,  //  `戊`
  };

  //  1998.2.23 새로 추가(Cherie)
  //  조합형 일어의 경우 완성형과 동일한 부분이 있는데,
  //  완성형에서는 거의 사용되지 않는 문자이므로,
  //  아래 코드가 있을경우 무조건 조합형으로 인식
  //  만일 조합형 일어 코드가 아니라면...
  if( hancode == 0xb7e7 ) return false;

  if( (int *)mybsearch(&hancode, kssmcode, MAX_CODE, sizeof(int), codecmp) != NULL ||
    ( hancode >= 0xd000 && hancode <= 0xd3ff ) ||
      ( hancode >= 0xdda1 && hancode <= 0xde98 ) ||
      ( hancode >= 0xdba1 && hancode <= 0xdbc8 ) ||

      //  1998.04.22 새로추가(Cherie)
      //  조합형 한글이 제대로 인식이 되지 않던 문제 해결
      //  아래 코드가 있을경우 무조건 조합형으로 인식
      //  1998.05.05 수정(Cherie)
      //  일부 완성형 코드가 조합형으로 인식하는 오류가 있어서
      //  검사 범위를 0xb400 - 0xb8ff 에서 0xb400 - 0xb7ff로 바꿈
      //  1998.07.28 수정(Cherie)
      //  조합형과 완성형 둘다 사용되는 코드중, 완성형에서 자주
      //  사용되지 않는 코드들은 조합형 코드로 간주한다.
      ( hancode >= 0xc500 && hancode <= 0xc5ff ) ||
      ( hancode >= 0xb700 && hancode <= 0xb7cd ) ||
      ( hancode >= 0xb7cf && hancode <= 0xb7ff ) ||

      //  1998.04.28 새로추가(Cherie)
      //  조합형 모음일경우 무조건 조합형으로 인식
      ( hancode >= 0xda00 && hancode <= 0xdaff ) ||

      //  1998.2.23 새로 추가(Cherie)
      //  조합형 선문자일경우도 무조건 조합형으로 인식
      //  만일 조합형 선문자 코드가 아니라면...
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
    if(first < 0x80)  continue; //  한글코드가 아니라면
    second = buf[count];
    if( count > 10240 ) break;
    if( count >= max )  break;
    count++;

    hancode = (first << 8) + second;  //  한글코드를 조합하기

    //  조합형과 완성형 둘다 자주 쓰이는 코드일경우에는
    //  카운트를 하지않는다.
    if( hancode >= 0xb500 && hancode < 0xb5ff &&
        hancode != 0xb5a5 && hancode != 0xb5e1 &&
        hancode != 0xb541 )
      continue;

    //  완성형에서 빨,래,닯,데,덧과 같은 글자는 조합형에서도 자주 쓰이는
    //  글자이므로 카운트를 하지 않는다.
    if( hancode == 0xbba1 ||
        hancode == 0xb7a1 ||
        hancode == 0xb4e5 ||
        hancode == 0xb7b6 ||
        hancode == 0xb4f6 ||
        hancode == 0xb8e1 ||  //  `저`
        hancode == 0xb1c1 ||  //  `쏴`
        hancode == 0xbaa1 ||  //  `줘`
        hancode == 0xbba5 ||  //  `진`
        hancode == 0xb6a1 ||  //  `워`/`땀`
        hancode == 0xb6e1 ||  //  `위`
        hancode == 0xa1a1 ) continue;

        // 띄엄띄엄 띄워진 KS5061의 특수문자 코드인가?
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

        //  1998.2.21 새로 추가(Cherie)
        hancode == 0xaec2 ||

        //  1998.8.31 새로 추가(Cherie)
        (hancode > 0xadd0 && hancode < 0xadff) ||

        // KS5061의 한글코드인가?
        Check_HanRange( hancode, 0xb0a0, 0xb0ff, 16 ) ||
        Check_HanRange( hancode, 0xc0a0, 0xc0ff, 9  ) ||

        ( hancode == 0xb8ae ) ||
        ( hancode == 0xb7e7 ) || // 완성형 '루'

        // KS5061의 한자코드인가?
        Check_HanRange( hancode, 0xcaa0, 0xcaff, 6  ) ||
        Check_HanRange( hancode, 0xd0a0, 0xd0ff, 16 ) ||
        Check_HanRange( hancode, 0xe0a0, 0xe0ff, 16 ) ||
        Check_HanRange( hancode, 0xf0a0, 0xf0ff, 14 ) ) &&

        //  1998.08.22 수정(Cherie)
        //  조합형과 완성형 둘다 사용되는 코드중, 조합형에서 자주
        //  사용되는 코드는 강제로 조합형 코드로 간주한다.
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

  //  대부분 조합형보다는 완성형 글이 많으므로 카운트 값이
  //  같을 경우 완성형으로 처리하도록 한다.
  if( ks_count >= kssm_count )
    return  1;
  else
    return  0;
}
