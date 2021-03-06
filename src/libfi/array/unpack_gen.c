/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

   Path64 is free software; you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation version 2.1

   Path64 is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Path64; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

   Special thanks goes to SGI for their continued support to open source

*/

/* automatically generated file, do not edit */

#include "f90_intrinsic.h"

void 
_UNPACK(
DopeVectorType	*result,
DopeVectorType	*vector,
DopeVectorType	*mask,
DopeVectorType	*field)
{
  char  * result_p, * result_b ;
  char  * vector_p, * vector_b ;
  char * mask_p, * mask_b ;
  char * field_p, * field_b ;

  size_t src_extent [MAX_NARY_DIMS] ;
  size_t src_stride [MAX_NARY_DIMS] ;
  size_t src_offset [MAX_NARY_DIMS] ;
  size_t counter[MAX_NARY_DIMS] ;

  size_t res_stride [MAX_NARY_DIMS] ;
  size_t res_extent [MAX_NARY_DIMS] ;
  size_t res_offset [MAX_NARY_DIMS] ;

  size_t msk_stride [MAX_NARY_DIMS] ;
  size_t msk_extent [MAX_NARY_DIMS] ;
  size_t msk_offset [MAX_NARY_DIMS] ;

  int32_t j,ii;
  char *rp, *ap ;
  int32_t res_rank ;
  int32_t src_rank = GET_RANK_FROM_DESC(vector) - 1;

  size_t typ_sz   = GET_ELEMENT_SZ_FROM_DESC(vector);

  size_t a_size,a_stride,r_stride, i,k ;
  int8_t  zero_szd_source = FALSE;
  int8_t  byte_aligned = FALSE;

  int32_t ddim ;

  size_t fill_stride [MAX_NARY_DIMS] ;
  size_t fill_offset [MAX_NARY_DIMS] ;
  size_t f_stride ;
  int32_t fill_rank  ;
  size_t  num_trues ;
  int32_t local_alloc ;
  size_t  tot_ext ;
  size_t  str_sz  ;

  size_t src_size ;
  size_t m_stride ;
  int32_t msk_rank ;

  size_t  res_sz;
  size_t  xfer_sz;
  size_t  tot_sz;

  src_stride[0]     = GET_STRIDE_FROM_DESC(vector,0) ;
  zero_szd_source   = (GET_EXTENT_FROM_DESC(vector,0) == 0);
  byte_aligned      = GET_BYTEALIGNED_FROM_DESC(vector) ;

  {
    size_t msk_typ_sz ;
    msk_typ_sz = GET_ELEMENT_SZ_FROM_DESC(mask);
    mask_b   = (char *) GET_ADDRESS_FROM_DESC(mask) + OFFSET_TO_TF_BYTE(msk_typ_sz);
    msk_rank = GET_RANK_FROM_DESC(mask) - 1 ;

    for ( j = 0 ; j <= msk_rank ; j ++  ) {
      msk_stride[j] = GET_STRIDE_FROM_DESC(mask,j) ;
      src_extent[j]  = GET_EXTENT_FROM_DESC(mask,j) ;
    }

    for ( j = 1 ; j <= msk_rank ; j ++  )
      msk_offset[j-1] = msk_stride[j] - (msk_stride [j-1] * (src_extent[j-1]))  ;
  }

  if (!GET_ASSOCIATED_FROM_DESC(result)) {

    size_t  nbytes  ;
    char    *p      ;

    SET_ADDRESS_IN_DESC(result,NULL);
    SET_ORIG_BS_IN_DESC(result,NULL) ;
    SET_ORIG_SZ_IN_DESC(result,0) ;

    p = NULL ;
    tot_ext = 1 ;
    nbytes  = typ_sz ;
    str_sz  = MK_STRIDE(byte_aligned,typ_sz);

    for ( i = 0 ; i <= msk_rank ; i ++) {
      SET_LBOUND_IN_DESC(result,i,1);
      SET_EXTENT_IN_DESC(result,i,src_extent[i]);
      SET_STRMULT_IN_DESC(result,i,tot_ext * str_sz );
      tot_ext *= src_extent[i];
      nbytes  *= src_extent[i];
    }

    if (nbytes > 0) {
      p = malloc (nbytes);
      if (p == NULL)
	ERROR(_LELVL_ABORT, FENOMEMY);

      SET_ADDRESS_IN_DESC(result,p);
    }
    SET_CONTIG_IN_DESC(result);
    SET_ASSOCIATED_IN_DESC(result);
    if (GET_DV_ASCII_FROM_DESC(vector)) {
      SET_CHARPTR_IN_DESC(result,p,typ_sz);
    }
    SET_ORIG_BS_IN_DESC(result,p) ;
    SET_ORIG_SZ_IN_DESC(result,nbytes * 8) ;
  }

  for ( j = 0 ; j <= msk_rank ; j ++  ) {
    res_stride[j]  = GET_STRIDE_FROM_DESC(result,j) ;
    counter[j] = 0 ;
    src_offset[j]  = 0 ;
  }

  for ( j = 1 ; j <= msk_rank ; j ++  )
    res_offset[j-1] = res_stride[j] - (res_stride [j-1] * (src_extent[j-1]))  ;

  field_p = GET_ADDRESS_FROM_DESC(field);
  fill_rank = GET_RANK_FROM_DESC(field) ;

  for ( j = 0 ; j <= msk_rank ; j ++  ) {
    fill_stride[j] = 0;
    fill_offset[j] = 0;
  }
  for ( j = 0 ; j < fill_rank ; j ++  ) {
    fill_stride[j] = GET_STRIDE_FROM_DESC(field,j) ;
  }
  for ( j = 1 ; j < fill_rank ; j ++  )  {
    fill_offset[j] = fill_stride[j] - (fill_stride [j-1] * (src_extent[j]))  ;
  }

  src_rank = msk_rank ;

  if (zero_szd_source)
    return ;

  a_size   = src_extent[0] ;
  a_stride = src_stride[0] ;
  r_stride = res_stride[0] ;
  m_stride = msk_stride[0] ;
  f_stride = fill_stride[0] ;
  vector_p = GET_ADDRESS_FROM_DESC(vector);
  result_p = GET_ADDRESS_FROM_DESC(result);
  mask_p   = mask_b ;

  {
    while (counter[src_rank] < src_extent[src_rank] ) {
      char * lfill ;

      for ( i = 0 ; i < a_size ; i ++ ) {

	lfill = field_p ;
	if (*mask_p) {
	  ap = vector_p ;
	  vector_p += a_stride ;
	} else {
	  ap = lfill ;
	}

	rp = result_p ;
	if (typ_sz > BIGDEFAULTSZ)
	  (void) memcpy (rp, ap, typ_sz);
	else
	  for (j = 0 ; j < typ_sz ; j ++)  *rp++ = *ap ++ ;

	result_p += r_stride ;
	mask_p += m_stride ;
	field_p += f_stride ;
      }
      counter[0] = a_size  ;
      j = 0 ;
      while ((counter[j] == src_extent[j]) && (j < src_rank)) {
	vector_p += src_offset[j] ;
	result_p += res_offset[j] ;
	mask_p   += msk_offset[j] ;
	field_p   += fill_offset[j] ;
	counter[j+1]++ ;
	counter[j] = 0 ;
	j ++ ;
      }

    }
  }
}
