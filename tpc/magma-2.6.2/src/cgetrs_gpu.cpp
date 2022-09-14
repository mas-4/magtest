/*
    -- MAGMA (version 2.6.2) --
       Univ. of Tennessee, Knoxville
       Univ. of California, Berkeley
       Univ. of Colorado, Denver
       @date April 2022

       @generated from src/zgetrs_gpu.cpp, normal z -> c, Wed Apr 20 17:36:13 2022

*/
#include "magma_internal.h"

/***************************************************************************//**
    Purpose
    -------
    CGETRS solves a system of linear equations
        A * X = B,
        A**T * X = B,  or
        A**H * X = B
    with a general N-by-N matrix A using the LU factorization computed by CGETRF_GPU.

    Arguments
    ---------
    @param[in]
    trans   magma_trans_t
            Specifies the form of the system of equations:
      -     = MagmaNoTrans:    A    * X = B  (No transpose)
      -     = MagmaTrans:      A**T * X = B  (Transpose)
      -     = MagmaConjTrans:  A**H * X = B  (Conjugate transpose)

    @param[in]
    n       INTEGER
            The order of the matrix A.  N >= 0.

    @param[in]
    nrhs    INTEGER
            The number of right hand sides, i.e., the number of columns
            of the matrix B.  NRHS >= 0.

    @param[in]
    dA      COMPLEX array on the GPU, dimension (LDDA,N)
            The factors L and U from the factorization A = P*L*U as computed
            by CGETRF_GPU.

    @param[in]
    ldda    INTEGER
            The leading dimension of the array A.  LDDA >= max(1,N).

    @param[in]
    ipiv    INTEGER array, dimension (N)
            The pivot indices from CGETRF; for 1 <= i <= N, row i of the
            matrix was interchanged with row IPIV(i).

    @param[in,out]
    dB      COMPLEX array on the GPU, dimension (LDDB,NRHS)
            On entry, the right hand side matrix B.
            On exit, the solution matrix X.

    @param[in]
    lddb    INTEGER
            The leading dimension of the array B.  LDDB >= max(1,N).

    @param[out]
    info    INTEGER
      -     = 0:  successful exit
      -     < 0:  if INFO = -i, the i-th argument had an illegal value

    @ingroup magma_getrs
*******************************************************************************/
extern "C" magma_int_t
magma_cgetrs_gpu(
    magma_trans_t trans, magma_int_t n, magma_int_t nrhs,
    magmaFloatComplex_ptr dA, magma_int_t ldda, magma_int_t *ipiv,
    magmaFloatComplex_ptr dB, magma_int_t lddb,
    magma_int_t *info)
{
    // Constants
    const magmaFloatComplex c_one = MAGMA_C_ONE;
    
    // Local variables
    magmaFloatComplex *work = NULL;
    bool notran = (trans == MagmaNoTrans);
    magma_int_t i1, i2, inc;

    *info = 0;
    if ( (! notran) &&
         (trans != MagmaTrans) &&
         (trans != MagmaConjTrans) ) {
        *info = -1;
    } else if (n < 0) {
        *info = -2;
    } else if (nrhs < 0) {
        *info = -3;
    } else if (ldda < max(1,n)) {
        *info = -5;
    } else if (lddb < max(1,n)) {
        *info = -8;
    }
    if (*info != 0) {
        magma_xerbla( __func__, -(*info) );
        return *info;
    }

    /* Quick return if possible */
    if (n == 0 || nrhs == 0) {
        return *info;
    }

    magma_cmalloc_cpu( &work, n * nrhs );
    if ( work == NULL ) {
        *info = MAGMA_ERR_HOST_ALLOC;
        return *info;
    }
    
    magma_queue_t queue = NULL;
    magma_device_t cdev;
    magma_getdevice( &cdev );
    magma_queue_create( cdev, &queue );
    
    i1 = 1;
    i2 = n;
    if (notran) {
        inc = 1;

        /* Solve A * X = B. */
        magma_cgetmatrix( n, nrhs, dB, lddb, work, n, queue );
        lapackf77_claswp( &nrhs, work, &n, &i1, &i2, ipiv, &inc );
        magma_csetmatrix( n, nrhs, work, n, dB, lddb, queue );

        if ( nrhs == 1) {
            magma_ctrsv( MagmaLower, MagmaNoTrans, MagmaUnit,    n, dA, ldda, dB, 1, queue );
            magma_ctrsv( MagmaUpper, MagmaNoTrans, MagmaNonUnit, n, dA, ldda, dB, 1, queue );
        } else {
            magma_ctrsm( MagmaLeft, MagmaLower, MagmaNoTrans, MagmaUnit,    n, nrhs, c_one, dA, ldda, dB, lddb, queue );
            magma_ctrsm( MagmaLeft, MagmaUpper, MagmaNoTrans, MagmaNonUnit, n, nrhs, c_one, dA, ldda, dB, lddb, queue );
        }
    } else {
        inc = -1;

        /* Solve A**T * X = B  or  A**H * X = B. */
        if ( nrhs == 1) {
            magma_ctrsv( MagmaUpper, trans, MagmaNonUnit, n, dA, ldda, dB, 1, queue );
            magma_ctrsv( MagmaLower, trans, MagmaUnit,    n, dA, ldda, dB, 1, queue );
        } else {
            magma_ctrsm( MagmaLeft, MagmaUpper, trans, MagmaNonUnit, n, nrhs, c_one, dA, ldda, dB, lddb, queue );
            magma_ctrsm( MagmaLeft, MagmaLower, trans, MagmaUnit,    n, nrhs, c_one, dA, ldda, dB, lddb, queue );
        }

        magma_cgetmatrix( n, nrhs, dB, lddb, work, n, queue );
        lapackf77_claswp( &nrhs, work, &n, &i1, &i2, ipiv, &inc );
        magma_csetmatrix( n, nrhs, work, n, dB, lddb, queue );
    }
    
    magma_queue_destroy( queue );
    magma_free_cpu( work );

    return *info;
}
