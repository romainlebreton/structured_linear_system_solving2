#ifndef __ZZ_HERMITE_PADE_H__
#define __ZZ_HERMITE_PADE_H__

#include <NTL/tools.h>
#include <NTL/matrix.h>
#include <NTL/vector.h>
#include <NTL/SmartPtr.h>
#include <NTL/ZZ.h>
#include <NTL/ZZ_pX.h>
#include <NTL/lzz_pX.h>

#include <memory>

#include "ZZ_p_block_sylvester.h"
#include "lzz_p_cauchy_geometric.h"
#include "ZZ_p_cauchy_geometric.h"
#include "lzz_pX_mosaic_hankel.h"
#include "ZZ_pX_mosaic_hankel.h"
#include "ZZX_mosaic_hankel.h"
#include "ZZ_pX_CRT.h"

NTL_CLIENT

/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
/* abstract base class for hermite pade classes                   */
/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/

class hermite_pade {
 protected:
  
  Vec<long> type;  // the type of the approximant
  long rank;       // the rank of M
  long p, p2;      // our main prime, and the prime for checking
  long w;          // w mod p
  long order;      // order of w
  long prec;       // precision of the approximation
  long added;      // number of rows added
  long sizeX;      // number of rows = prec + added
  long sizeY;      // numbers of columns
  long level;      // current level 
  zz_pContext ctx; // zz_p
  ZZ_pContext ctx2;// zz_p2

  Vec<SmartPtr<ZZ_p_block_sylvester>> vec_M;      // stores the block-sylvester matrices mod powers of p
  Vec<ZZ_pX_Multipoint_FFT> vec_X_int, vec_Y_int; // stores regularization matrices mod powers of p
  Vec<ZZ> p_powers;                               // p_powers[i] = p^(2^i)
  Vec<ZZ> e, f;                                   // diagonal preconditioners for the Cauchy matrix
  ZZ c, d;                                        // constants for the preconditioners
  Vec<ZZ> vec_w;                                  // the lifts of the root of 1 for X, Y
  lzz_p_cauchy_like_geometric invA;
  Vec<ZZ_p_cauchy_like_geometric> lift_invA;      // for Newton
  Vec<long> witness;                              // for stop criterion mode 3

  
  long mode = 0;                                  // determines the subroutine to solve Mx = b mod p^2^n, 0-DAC, 1-Dixon, 2-Newton
  long stop_criterion = 0;                        // how we stop the lifting. 0 = test another prime, 1 = twice the same result

  double time_mulA = 0; // time spent on just multiplications
  double time_recon_all = 0; // time spent on the whole recon
  double time_recon = 0; // time spent on just reconstructing entries
  double time_check_p2 = 0; // time spent checking p2
  double div_in_recon = 0;
  double time_mul_m_right = 0;
  double time_mul_x_right = 0;
  double time_mul_y_right = 0;
  double time_mul_x_left = 0;
  double time_mul_y_left = 0;
  double time_mul_m_left = 0;
  double time_cauchy_right = 0;
  double time_cauchy_left = 0;
  double time_spent_lower = 0;
  
  /*----------------------------------------------------------------*/
  /* applies a block reversal to v                                  */
  /* e.g., type = [1,2] v = [v1,v2,v3,v4,v5]                        */
  /* returns [v2,v1,v5,v4,v3] (blocks have length type[i]+1)        */
  /*----------------------------------------------------------------*/
  Vec<ZZ_p> flip_on_type (const Vec<ZZ_p> &v);
  Vec<long> flip_on_type (const Vec<long> &v);
  Vec<Vec<ZZ>> flip_on_type(const Vec<Vec<ZZ>> &v);

  /*----------------------------------------------------------------*/
  /* splits v according to the current type                         */
  /*----------------------------------------------------------------*/
  Vec<Vec<ZZ_p>> split_on_type(const Vec<ZZ_p> &v);

  /*----------------------------------------------------------------*/
  /* splits v according to the current type                         */
  /*----------------------------------------------------------------*/
   Vec<zz_pX> split_on_type(const Vec<zz_p> &v);

  /*----------------------------------------------------------------*/
  /* splits v according to the current type                         */
  /*----------------------------------------------------------------*/
  Vec<ZZX> split_on_type(const Vec<ZZ> &v);
  
  /*----------------------------------------------------------------*/
  /* switches ZZ_p to ZZ mod p^(2^n)                                */
  /* does not save the current context                              */
  /*----------------------------------------------------------------*/
  void switch_context(long n);
  
  /*----------------------------------------------------------------*/
  /* calls create_bmc and appends the result to vec_M               */
  /*----------------------------------------------------------------*/
  void set_up_bmc();

  /*----------------------------------------------------------------*/
  /* creates a new ZZ_p_block_sylvester                             */
  /*----------------------------------------------------------------*/
  virtual SmartPtr<ZZ_p_block_sylvester> create_bmc() = 0;

  /*------------------------------------------------------------------*/
  /*------------------------------------------------------------------*/
  virtual void generators_hankel_last_row_last_column(Mat<ZZ_p> & X, Mat<ZZ_p> & Y, Vec<ZZ_p> & r, Vec<ZZ_p> & c) = 0;

  /*------------------------------------------------------------------*/
  /*------------------------------------------------------------------*/
  void generators_cauchy(Mat<ZZ_p> & X, Mat<ZZ_p> & Y);

  /*----------------------------------------------------------------*/
  /* multiplies b by the matrix Y_int^t D_f                         */
  /*----------------------------------------------------------------*/
  Vec<ZZ_p> mul_Y_right(const Vec<ZZ_p> &b);
  /*----------------------------------------------------------------*/
  /* multiplies b by the matrix D_f Y_int                           */
  /*----------------------------------------------------------------*/
  Vec<ZZ_p> mul_Y_left(const Vec<ZZ_p> &b);
  

  /*----------------------------------------------------------------*/
  /* multiplies b by the matrix D_e X_int                           */
  /*----------------------------------------------------------------*/
  Vec<ZZ_p> mul_X_right(const Vec<ZZ_p> &b);
  /*----------------------------------------------------------------*/
  /* multiplies b by the matrix X_int^t D_e                         */
  /*----------------------------------------------------------------*/
  Vec<ZZ_p> mul_X_left(const Vec<ZZ_p> &b);


  /*----------------------------------------------------------------*/
  /* multiplies b by the matrix M                                   */
  /*----------------------------------------------------------------*/
  virtual Vec<ZZ_p> mul_M_right(const Vec<ZZ_p> &b);
  /*----------------------------------------------------------------*/
  /* left-multiplies b by the matrix M                              */
  /*----------------------------------------------------------------*/
  virtual Vec<ZZ_p> mul_M_left(const Vec<ZZ_p> &b);

  /*----------------------------------------------------------------*/
  /* multiplies b by the matrix CL =  D_e X_int M Y_int^t D_f       */
  /* (CL is cauchy-geometric-like)                                  */
  /* b need not have size CL.NumCols()                              */
  /*----------------------------------------------------------------*/
  Vec<ZZ_p> mulA_right(const Vec<ZZ_p> &b);
  Mat<ZZ_p> mulA_right(const Mat<ZZ_p> &b);

  /*----------------------------------------------------------------*/
  /* left-multiplies b by the matrix CL =  D_e X_int M Y_int^t D_f  */
  /* (CL is cauchy-geometric-like)                                  */
  /*----------------------------------------------------------------*/
  Vec<ZZ_p> mulA_left(const Vec<ZZ_p> &b);
  Mat<ZZ_p> mulA_left(const Mat<ZZ_p> &b);

  /*----------------------------------------------------------------*/
  /* if Mx = b mod p^(2^{n-1}), updates x so that Mx = b mod p^(2^n)*/
  /*----------------------------------------------------------------*/
  void update_solution(Vec<ZZ>& x, const Vec<ZZ_p> &b, long n);

  /*----------------------------------------------------------------*/
  /* computes the inverse of A (cauchy matrix) mod p^(2^n)          */
  /* assumes the inverse mod p^(2^(n-1)) is known                   */
  /*----------------------------------------------------------------*/
  void Newton(long n);

  /*----------------------------------------------------------------*/
  /* solves Mx = b mod p^(2^n) by Newton iteration                  */
  /*----------------------------------------------------------------*/
  void solve_newton(Vec<ZZ>& x, const Vec<ZZ> &b, long n);
  
  /*----------------------------------------------------------------*/
  /* solves for Mx = b mod p^(2^n) using DAC                        */
  /*----------------------------------------------------------------*/
  void DAC (Vec<ZZ> &x, const Vec<ZZ> &b, long n);
  
  /*----------------------------------------------------------------*/
  /* solves for Mx = b mod p^(2^n) using Dixon's algorithm          */
  /*----------------------------------------------------------------*/
  void Dixon (Vec<ZZ> &x, Vec<ZZ> & b_in, long n);
  
  /*----------------------------------------------------------------*/
  /* checks if every entry can be reconstructed                     */
  /* if so, check whether the solution cancels the system mod p2    */
  /*----------------------------------------------------------------*/
  bool reconstruct_and_check(Vec<ZZX> & sol, const Vec<ZZ_p> &v, long n);

  /*----------------------------------------------------------------*/
  /* sets up the field, contexts, ..                                */
  /*----------------------------------------------------------------*/
  hermite_pade(long fft_index);

  /*----------------------------------------------------------------*/
  /* does nothing                                                   */
  /*----------------------------------------------------------------*/
  hermite_pade();

 public:

  /*----------------------------------------------------------------*/
  /* computes a random solution to the system modulo p              */
  /*----------------------------------------------------------------*/
  void random_solution_mod_p(Vec<zz_pX> &v);

  /*----------------------------------------------------------------*/
  /* computes a random solution to the system                       */
  /*----------------------------------------------------------------*/
  void random_solution (Vec<ZZX> &sol);

  /*----------------------------------------------------------------*/
  /* Rank                                                           */
  /*----------------------------------------------------------------*/
  long Rank() const;

  /*----------------------------------------------------------------*/
  /* Dimensions                                                     */
  /*----------------------------------------------------------------*/
  long NumRows() const;
  long NumCols() const;
  
  /*----------------------------------------------------------------*/
  /* mode switch. 0-DAC, 1-Dixon, 2-Newton                          */
  /*----------------------------------------------------------------*/
  void switch_mode(long i);
};


/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
/* hermite pade for general polynomials                           */
/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/

class hermite_pade_general : public hermite_pade{
 protected:
  Vec<ZZX> vec_fs;
  Vec<Vec<long>> vec_added;
  long rows_added;
  long original_sizeX;

  Mat<ZZ> G, H;                                   // the generators of the mosaic hankel matrix over ZZ
  ZZ_mosaic_hankel MH_ZZ;                         // the mosaic hankel matrix over ZZ  

  /*---------------------------------------------------------*/
  /* tries to increase the rank of M by adding random blocks */
  /*---------------------------------------------------------*/
  void increase_rank(Vec<hankel> & vh_zz_p, Vec<ZZ_hankel> & vh_ZZ, long add);
  
  /*----------------------------------------------------------------*/
  /* creates a new block Sylvester matrix                           */
  /*----------------------------------------------------------------*/
  SmartPtr<ZZ_p_block_sylvester> create_bmc() override;

  /*----------------------------------------------------------------*/
  /* does the product by M and the extra rows                       */
  /*----------------------------------------------------------------*/
  Vec<ZZ_p> mul_M_right(const Vec<ZZ_p> &) override;

  /*------------------------------------------------------------------*/
  /*------------------------------------------------------------------*/
  void generators_hankel_last_row_last_column(Mat<ZZ_p> & X, Mat<ZZ_p> & Y, Vec<ZZ_p> & r, Vec<ZZ_p> & c);

  /*----------------------------------------------------------------*/
  /* initializes everything                                         */
  /*----------------------------------------------------------------*/
  void init();
  
 public:


  /*----------------------------------------------------------------*/
  /* does nothing                                                   */
  /*----------------------------------------------------------------*/
  hermite_pade_general();

  /*----------------------------------------------------------------*/
  /* fs: the power series                                           */
  /* type: the type of the approximant                              */
  /* sigma: requested precision                                     */
  /*----------------------------------------------------------------*/
  hermite_pade_general(const Vec<ZZX> &fs, const Vec<long>& type, long sigma, long fft_init = 0);

  /*----------------------------------------------------------------*/
  /* fs: the power series                                           */
  /* type: the type of the approximant                              */
  /* sigma: requested precision                                     */
  /* w: solution modulo the next fft prime                          */
  /*----------------------------------------------------------------*/
  hermite_pade_general(const Vec<ZZX> &fs, const Vec<long> &type, long sigma, Vec<long>& w, long fft_init = 0);

};


/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
/* hermite pade for algebraic approximants                        */
/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/

class hermite_pade_algebraic : public hermite_pade {

  ZZX f_full_prec; // the polynomial in full precision
  ZZ denom = ZZ{1}; // the denominator for every entry in f

  /*----------------------------------------------------------------*/
  /* creates a new bivariate modular composition                    */
  /*----------------------------------------------------------------*/
  SmartPtr<ZZ_p_block_sylvester> create_bmc() override;

  /*------------------------------------------------------------------*/
  /*------------------------------------------------------------------*/
  void generators_hankel_last_row_last_column(Mat<ZZ_p> & X, Mat<ZZ_p> & Y, Vec<ZZ_p> & r, Vec<ZZ_p> & c);
  
  public:
  /*----------------------------------------------------------------*/
  /* gives a candidate for a reduced type by GCD techniques         */
  /*----------------------------------------------------------------*/
  Vec<long> find_new_type();

  /*----------------------------------------------------------------*/
  /* initializes everything                                         */
  /*----------------------------------------------------------------*/
  void init(const Vec<long>& type);

  /*----------------------------------------------------------------*/
  /* does nothing                                                   */
  /*----------------------------------------------------------------*/
  hermite_pade_algebraic();

  /*----------------------------------------------------------------*/
  /* f: the power series                                            */
  /* type: the type of the approximant                              */
  /* sigma: requested precision                                     */
  /*----------------------------------------------------------------*/
  hermite_pade_algebraic(const ZZX &f, const Vec<long>& type, long sigma, long fft_init = 0);

  /*----------------------------------------------------------------*/
  /* f: the power series                                            */
  /* type: the type of the approximant                              */
  /* sigma: requested precision                                     */
  /* w: solution modulo the next fft prime                          */
  /*----------------------------------------------------------------*/
  hermite_pade_algebraic(const ZZX &f, const Vec<long>& type, long sigma, Vec<long>& w, long fft_index = 0);

  /*----------------------------------------------------------------*/
  /* f: numerators of the power series                              */
  /* denom: their denominators                                      */
  /* type: the type of the approximant                              */
  /* sigma: requested precision                                     */
  /*----------------------------------------------------------------*/
  hermite_pade_algebraic(const ZZX &f, const ZZ &denom, const Vec<long>& type, long sigma, long fft_index = 0);
};


#endif
