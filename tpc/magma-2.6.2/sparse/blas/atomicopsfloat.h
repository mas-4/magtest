/*
    -- MAGMA (version 2.6.2) --
       Univ. of Tennessee, Knoxville
       Univ. of California, Berkeley
       Univ. of Colorado, Denver
       @date April 2022

       @precisions normal z -> c d s
       @author Weifeng Liu

*/
#ifndef MAGMASPARSE_ATOMICOPS_FLOAT_H
#define MAGMASPARSE_ATOMICOPS_FLOAT_H

#include "magmasparse_internal.h"

extern __forceinline__ __device__ void 
atomicAddfloat(float *addr, float val)
{
    atomicAdd(addr, val);
}


#endif 
