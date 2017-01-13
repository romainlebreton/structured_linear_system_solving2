#include <NTL/vec_lzz_p.h>
#include <assert.h>

#include "vec_lzz_p_extra.h"
#include "lzz_p_cauchy_geometric.h"

NTL_CLIENT

/*------------------------------------------------------------*/
/* if opt = 1, runs a check                                   */
/* else, runs timings                                         */
/*------------------------------------------------------------*/
void check(int opt){
   zz_p::FFTInit(0);
   zz_p::UserFFTInit(65537);

   for (long i = 500; i < 10000; i += 100){

     zz_p a = power(random_zz_p(), 2);
     long j = i+1;
     long alpha = (i/10) + 3;
     mat_zz_p A, B;
     random(A, i, alpha);
     random(B, j, alpha);
     lzz_p_cauchy_like_geometric M(A, B, to_zz_p(1), power(a, i), a);
     mat_zz_p Z;

     Mat<zz_p> in, out, check;
     random(in, j, alpha);

     if (opt == 1){
       M.mul_right_sigma_UL(out, in);
       M.to_dense(Z);
       mul(check, Z, in);
       assert (check == out);
       cout << i << endl;
     }
     else{
       cout << i << " " << alpha << " ";

       double t;

       t = GetTime();
       for (long j = 0; j < 1; j++)
	 M.mul_right(out, in);
       t = GetTime() - t;
       cout << t << " ";

       t = GetTime();
       for (long j = 0; j < 1; j++)
	 M.mul_right_sigma_UL(out, in);
       t = GetTime() - t;
       cout << t << " ";

       cout << endl;
     }
   }
}

/*------------------------------------------------------------*/
/* main just calls check()                                    */
/* if not argument is given, runs timings                     */
/* if the argument 1 is given, runs check                     */
/*------------------------------------------------------------*/
int main(int argc, char** argv){
  int opt = 0;
  if (argc > 1)
    opt = atoi(argv[1]);
  check(opt);

  return 0;
}
