      SUBROUTINE ZTBT02( UPLO, TRANS, DIAG, N, KD, NRHS, AB, LDAB, X,
     $                   LDX, B, LDB, WORK, RWORK, RESID )
*
*  -- LAPACK test routine (version 3.1) --
*     Univ. of Tennessee, Univ. of California Berkeley and NAG Ltd..
*     November 2006
*
*     .. Scalar Arguments ..
      CHARACTER          DIAG, TRANS, UPLO
      INTEGER            KD, LDAB, LDB, LDX, N, NRHS
      DOUBLE PRECISION   RESID
*     ..
*     .. Array Arguments ..
      DOUBLE PRECISION   RWORK( * )
      COMPLEX*16         AB( LDAB, * ), B( LDB, * ), WORK( * ),
     $                   X( LDX, * )
*     ..
*
*  Purpose
*  =======
*
*  ZTBT02 computes the residual for the computed solution to a
*  triangular system of linear equations  A*x = b,  A**T *x = b,  or
*  A**H *x = b  when A is a triangular band matrix.  Here A**T denotes
*  the transpose of A, A**H denotes the conjugate transpose of A, and
*  x and b are N by NRHS matrices.  The test ratio is the maximum over
*  the number of right hand sides of
*     norm(b - op(A)*x) / ( norm(op(A)) * norm(x) * EPS ),
*  where op(A) denotes A, A**T, or A**H, and EPS is the machine epsilon.
*
*  Arguments
*  =========
*
*  UPLO    (input) CHARACTER*1
*          Specifies whether the matrix A is upper or lower triangular.
*          = 'U':  Upper triangular
*          = 'L':  Lower triangular
*
*  TRANS   (input) CHARACTER*1
*          Specifies the operation applied to A.
*          = 'N':  A *x = b     (No transpose)
*          = 'T':  A**T *x = b  (Transpose)
*          = 'C':  A**H *x = b  (Conjugate transpose)
*
*  DIAG    (input) CHARACTER*1
*          Specifies whether or not the matrix A is unit triangular.
*          = 'N':  Non-unit triangular
*          = 'U':  Unit triangular
*
*  N       (input) INTEGER
*          The order of the matrix A.  N >= 0.
*
*  KD      (input) INTEGER
*          The number of superdiagonals or subdiagonals of the
*          triangular band matrix A.  KD >= 0.
*
*  NRHS    (input) INTEGER
*          The number of right hand sides, i.e., the number of columns
*          of the matrices X and B.  NRHS >= 0.
*
*  AB      (input) COMPLEX*16 array, dimension (LDA,N)
*          The upper or lower triangular band matrix A, stored in the
*          first kd+1 rows of the array. The j-th column of A is stored
*          in the j-th column of the array AB as follows:
*          if UPLO = 'U', AB(kd+1+i-j,j) = A(i,j) for max(1,j-kd)<=i<=j;
*          if UPLO = 'L', AB(1+i-j,j)    = A(i,j) for j<=i<=min(n,j+kd).
*
*  LDAB    (input) INTEGER
*          The leading dimension of the array AB.  LDAB >= max(1,KD+1).
*
*  X       (input) COMPLEX*16 array, dimension (LDX,NRHS)
*          The computed solution vectors for the system of linear
*          equations.
*
*  LDX     (input) INTEGER
*          The leading dimension of the array X.  LDX >= max(1,N).
*
*  B       (input) COMPLEX*16 array, dimension (LDB,NRHS)
*          The right hand side vectors for the system of linear
*          equations.
*
*  LDB     (input) INTEGER
*          The leading dimension of the array B.  LDB >= max(1,N).
*
*  WORK    (workspace) COMPLEX*16 array, dimension (N)
*
*  RWORK   (workspace) DOUBLE PRECISION array, dimension (N)
*
*  RESID   (output) DOUBLE PRECISION
*          The maximum over the number of right hand sides of
*          norm(op(A)*x - b) / ( norm(op(A)) * norm(x) * EPS ).
*
*  =====================================================================
*
*     .. Parameters ..
      DOUBLE PRECISION   ZERO, ONE
      PARAMETER          ( ZERO = 0.0D+0, ONE = 1.0D+0 )
*     ..
*     .. Local Scalars ..
      INTEGER            J
      DOUBLE PRECISION   ANORM, BNORM, EPS, XNORM
*     ..
*     .. External Functions ..
      LOGICAL            LSAME
      DOUBLE PRECISION   DLAMCH, DZASUM, ZLANTB
      EXTERNAL           LSAME, DLAMCH, DZASUM, ZLANTB
*     ..
*     .. External Subroutines ..
      EXTERNAL           ZAXPY, ZCOPY, ZTBMV
*     ..
*     .. Intrinsic Functions ..
      INTRINSIC          DCMPLX, MAX
*     ..
*     .. Executable Statements ..
*
*     Quick exit if N = 0 or NRHS = 0
*
      IF( N.LE.0 .OR. NRHS.LE.0 ) THEN
         RESID = ZERO
         RETURN
      END IF
*
*     Compute the 1-norm of A or A'.
*
      IF( LSAME( TRANS, 'N' ) ) THEN
         ANORM = ZLANTB( '1', UPLO, DIAG, N, KD, AB, LDAB, RWORK )
      ELSE
         ANORM = ZLANTB( 'I', UPLO, DIAG, N, KD, AB, LDAB, RWORK )
      END IF
*
*     Exit with RESID = 1/EPS if ANORM = 0.
*
      EPS = DLAMCH( 'Epsilon' )
      IF( ANORM.LE.ZERO ) THEN
         RESID = ONE / EPS
         RETURN
      END IF
*
*     Compute the maximum over the number of right hand sides of
*        norm(op(A)*x - b) / ( norm(op(A)) * norm(x) * EPS ).
*
      RESID = ZERO
      DO 10 J = 1, NRHS
         CALL ZCOPY( N, X( 1, J ), 1, WORK, 1 )
         CALL ZTBMV( UPLO, TRANS, DIAG, N, KD, AB, LDAB, WORK, 1 )
         CALL ZAXPY( N, DCMPLX( -ONE ), B( 1, J ), 1, WORK, 1 )
         BNORM = DZASUM( N, WORK, 1 )
         XNORM = DZASUM( N, X( 1, J ), 1 )
         IF( XNORM.LE.ZERO ) THEN
            RESID = ONE / EPS
         ELSE
            RESID = MAX( RESID, ( ( BNORM / ANORM ) / XNORM ) / EPS )
         END IF
   10 CONTINUE
*
      RETURN
*
*     End of ZTBT02
*
      END
