#ifndef __HCODE_H__
#define __HCODE_H__

#ifdef  __cplusplus
extern "C" {
#endif
    int isKS( const char *mbs, int mbslen,
              int *ks_count, int *ksm_count, int *eng_count );
#ifdef  __cplusplus
};
#endif

#endif
