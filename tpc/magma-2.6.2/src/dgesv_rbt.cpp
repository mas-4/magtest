/*
    -- MAGMA (version 2.6.2) --
       Univ. of Tennessee, Knoxville
       Univ. of California, Berkeley
       Univ. of Colorado, Denver
       @date April 2022

       @generated from src/zgesv_rbt.cpp, normal z -> d, Wed Apr 20 17:36:14 2022

*/
#include "magma_internal.h"

/***************************************************************************//**
    Purpose
    -------
    DGESV_RBT solves a system of linear equations
        A * X = B
    where A is a general N-by-N matrix and X and B are N-by-NRHS matrices.
    Random Butterfly Tranformation is applied on A and B, then
    the LU decomposition with no pivoting is
    used to factor A as
        A = L * U,
    where L is unit lower triangular, and U is
    upper triangular.  The factored form of A is then used to solve the
    system of equations A * X = B.
    The solution can then be improved using iterative refinement.

    Arguments
    ---------
    @param[in]
    refine  magma_bool_t
            Specifies if iterative refinement is to be applied to improve the solution.
      -     = MagmaTrue:   Iterative refinement is applied.
      -     = MagmaFalse:  Iterative refinement is not applied.

    @param[in]
    n       INTEGER
            The order of the matrix A.  N >= 0.

    @param[in]
    nrhs    INTEGER
            The number of right hand sides, i.e., the number of columns
            of the matrix B.  NRHS >= 0.

    @param[in,out]
    A       DOUBLE PRECISION array, dimension (LDA,N).
            On entry, the M-by-N matrix to be factored.
            On exit, the factors L and U from the factorization
            A = P*L*U; the unit diagonal elements of L are not stored.

    @param[in]
    lda     INTEGER
            The leading dimension of the array A.  LDA >= max(1,N).

    @param[in,out]
    B       DOUBLE PRECISION array, dimension (LDB,NRHS)
            On entry, the right hand side matrix B.
            On exit, the solution matrix X.
    
    @param[in]
    ldb     INTEGER
            The leading dimension of the array B.  LDB >= max(1,N).

    @param[out]
    info    INTEGER
      -     = 0:  successful exit
      -     < 0:  if INFO = -i, the i-th argument had an illegal value

    @ingroup magma_gesv_rbt
*******************************************************************************/
extern "C" magma_int_t
magma_dgesv_rbt(
    magma_bool_t refine, magma_int_t n, magma_int_t nrhs,
    double *A, magma_int_t lda,
    double *B, magma_int_t ldb,
    magma_int_t *info)
{
    /* Constants */
    const double c_zero = MAGMA_D_ZERO;
    const double c_one  = MAGMA_D_ONE;
    
    /* Local variables */
    magma_int_t nn = magma_roundup( n, 4 );  // n + ((4-(n % 4))%4);
    double *hu=NULL, *hv=NULL;
    magmaDouble_ptr dA=NULL, dB=NULL, dAo=NULL, dBo=NULL, dwork=NULL, dv=NULL;
    magma_int_t i, iter;
    magma_queue_t queue=NULL;
    
    /* Function Body */
    *info = 0;
    if ( ! (refine == MagmaTrue) &&
         ! (refine == MagmaFalse) ) {
        *info = -1;
    }
    else if (n < 0) {
        *info = -2;
    } else if (nrhs < 0) {
        *info = -3;
    } else if (lda < max(1,n)) {
        *info = -5;
    } else if (ldb < max(1,n)) {
        *info = -7;
    }
    if (*info != 0) {
        magma_xerbla( __func__, -(*info) );
        return *info;
    }

    /* Quick return if possible */
    if (nrhs == 0 || n == 0)
        return *info;

    if (MAGMA_SUCCESS != magma_dmalloc( &dA, nn*nn ) ||
        MAGMA_SUCCESS != magma_dmalloc( &dB, nn*nrhs ))
    {
        *info = MAGMA_ERR_DEVICE_ALLOC;
        goto cleanup;
    }

    if (refine == MagmaTrue) {
        if (MAGMA_SUCCESS != magma_dmalloc( &dAo,   nn*nn ) ||
            MAGMA_SUCCESS != magma_dmalloc( &dwork, nn*nrhs ) ||
            MAGMA_SUCCESS != magma_dmalloc( &dBo,   nn*nrhs ))
        {
            *info = MAGMA_ERR_DEVICE_ALLOC;
            goto cleanup;
        }
    }

    if (MAGMA_SUCCESS != magma_dmalloc_cpu( &hu, 2*nn ) ||
        MAGMA_SUCCESS != magma_dmalloc_cpu( &hv, 2*nn ))
    {
        *info = MAGMA_ERR_HOST_ALLOC;
        goto cleanup;
    }

    magma_device_t cdev;
    magma_getdevice( &cdev );
    magma_queue_create( cdev, &queue );
    
    magmablas_dlaset( MagmaFull, nn, nn, c_zero, c_one, dA, nn, queue );

    /* Send matrix to the GPU */
    magma_dsetmatrix( n, n, A, lda, dA, nn, queue );

    /* Send b to the GPU */
    magma_dsetmatrix( n, nrhs, B, ldb, dB, nn, queue );

    *info = magma_dgerbt_gpu( MagmaTrue, nn, nrhs, dA, nn, dB, nn, hu, hv, info );
    if (*info != MAGMA_SUCCESS)  {
        return *info;
    }

    if (refine == MagmaTrue) {
        magma_dcopymatrix( nn, nn, dA, nn, dAo, nn, queue );
        magma_dcopymatrix( nn, nrhs, dB, nn, dBo, nn, queue );
    }
    /* Solve the system U^TAV.y = U^T.b on the GPU */
    magma_dgesv_nopiv_gpu( nn, nrhs, dA, nn, dB, nn, info );

    /* Iterative refinement */
    if (refine == MagmaTrue) {
        magma_dgerfs_nopiv_gpu( MagmaNoTrans, nn, nrhs, dAo, nn, dBo, nn, dB, nn, dwork, dA, &iter, info );
    }
    //printf("iter = %lld\n", (long long) iter );

    /* The solution of A.x = b is Vy computed on the GPU */
    if (MAGMA_SUCCESS != magma_dmalloc( &dv, 2*nn )) {
        *info = MAGMA_ERR_DEVICE_ALLOC;
        goto cleanup;
    }

    magma_dsetvector( 2*nn, hv, 1, dv, 1, queue );
    
    for (i = 0; i < nrhs; i++) {
        magmablas_dprbt_mv( nn, dv, dB+(i*nn), queue );
    }

    magma_dgetmatrix( n, nrhs, dB, nn, B, ldb, queue );

cleanup:
    magma_queue_destroy( queue );
    
    magma_free_cpu( hu );
    magma_free_cpu( hv );

    magma_free( dA );
    magma_free( dv );
    magma_free( dB );
    
    if (refine == MagmaTrue) {
        magma_free( dAo );
        magma_free( dBo );
        magma_free( dwork );
    }
    
    return *info;
}
