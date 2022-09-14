/*
    -- MAGMA (version 2.6.2) --
       Univ. of Tennessee, Knoxville
       Univ. of California, Berkeley
       Univ. of Colorado, Denver
       @date April 2022

       @generated from magmablas/zgerbt.h, normal z -> c, Wed Apr 20 17:38:08 2022

       @author Adrien Remy
       @author Azzam Haidar
       
       Definitions used in cgerbt.cu cgerbt_batched.cu
*/

#ifndef CGERBT_H
#define CGERBT_H

// =============================================================================
// classical prototypes

__global__ void 
magmablas_celementary_multiplication_kernel(
    magma_int_t n,
    magmaFloatComplex *dA, magma_int_t offsetA, magma_int_t ldda, 
    magmaFloatComplex *du, magma_int_t offsetu, 
    magmaFloatComplex *dv, magma_int_t offsetv);

__global__ void 
magmablas_capply_vector_kernel(
    magma_int_t n,
    magmaFloatComplex *du, magma_int_t offsetu,  magmaFloatComplex *db, magma_int_t offsetb );

__global__ void 
magmablas_capply_transpose_vector_kernel(
    magma_int_t n,
    magmaFloatComplex *du, magma_int_t offsetu, magmaFloatComplex *db, magma_int_t offsetb );

// =============================================================================
// batched prototypes

__global__ void 
magmablas_celementary_multiplication_kernel_batched(
    magma_int_t n,
    magmaFloatComplex **dA_array, magma_int_t offsetA, magma_int_t ldda, 
    magmaFloatComplex *du, magma_int_t offsetu, 
    magmaFloatComplex *dv, magma_int_t offsetv);

__global__ void 
magmablas_capply_vector_kernel_batched(
    magma_int_t n,
    magmaFloatComplex *du, magma_int_t offsetu, magmaFloatComplex **db_array, magma_int_t offsetb );

__global__ void 
magmablas_capply_transpose_vector_kernel_batched(
    magma_int_t n,
    magmaFloatComplex *du, magma_int_t offsetu, magmaFloatComplex **db_array, magma_int_t offsetb );

#endif // CGERBT_H
