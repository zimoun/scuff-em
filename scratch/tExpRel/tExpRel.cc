/*
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <complex>

#include <libhrutil.h>

#define EXPRELTOL  1.0e-8
#define EXPRELTOL2 EXPRELTOL*EXPRELTOL

#define II cdouble(0.0,1.0)

/***************************************************************/
/***************************************************************/
/***************************************************************/
cdouble In_EMKROverR_libRWG(int n, cdouble GParam, double X);
cdouble In_GradEMKROverR_libRWG(int n, cdouble GParam, double R);
cdouble In_EIKROverR_libRWG(int n, cdouble K, double R);
cdouble In_GradEIKROverR_libRWG(int n, cdouble K, double R);

/***************************************************************/
/* quick factorial function needed below                       */
/***************************************************************/
static inline double factorial(int n) 
{ 
  static double FactTable[7]={1.0, 1.0, 2.0, 6.0, 24.0, 120.0, 620.0}; 

  if (n<=6)
   return FactTable[n];
  else
   return ((double)n)*factorial(n-1);
}

/***************************************************************/
/***************************************************************/
/***************************************************************/
cdouble ExpRel(int n, cdouble Z)
{
  int m;

  /*--------------------------------------------------------------*/
  /*- purely real case                                           -*/
  /*--------------------------------------------------------------*/
  if ( imag(Z)==0.0 )
   { 
     double Term, Sum;
     double X=real(Z);

     //////////////////////////////////////////////////
     // small-Z expansion
     //////////////////////////////////////////////////
     if ( fabs(X) < 0.1 )
      { Sum=1.0;
        for(Term=1.0, m=1; m<100; m++)
         { Term*=X/((double)(m+n));
           Sum+=Term;
           if ( fabs(Term) < EXPRELTOL*fabs(Sum) )
            break;
         };
        return Sum;
      }
     else
      { Sum=exp(X);
        for(Term=1.0, m=0; m<n; m++)
         { 
          Sum-=Term;
           Term*=X/((double)(m+1));
         };
        return Sum / Term;
      };
   }
  else
   { 
     cdouble Term, Sum;
     /*--------------------------------------------------------------*/
     /*- small-Z expansion                                          -*/
     /*--------------------------------------------------------------*/
     if ( abs(Z) < 0.1 )
      { cdouble Term, Sum;
        double mag2Term, mag2Sum;
        Sum=1.0;
        for(Term=1.0, m=1; m<100; m++)
         { Term*=Z/((double)(m+n));
           Sum+=Term;
           if ( norm(Term) < EXPRELTOL2*norm(Sum) )
            break;
         };
        return Sum;
      }
     else
      {
        Sum=exp(Z);
        for(Term=1.0, m=0; m<n; m++)
         { 
           Sum-=Term;
           Term*=Z/((double)(m+1));
         };
        return Sum / Term;
      };
   };

} 

/***************************************************************/
/***************************************************************/
/***************************************************************/
cdouble In_EMKROverR(int n, double K, double R)
{
  double KR=K*R;

  if ( abs(KR) < 0.1 )
   return exp(-KR)*ExpRel(n,KR) / ((double)(n)*R);
  else 
   { cdouble Sum, Term;
     int m;
     for(Sum=0.0, Term=1.0, m=0; m<n; m++)
      { 
        Sum+=Term;
        Term *= KR/((double)(m+1));
      };
     return (1.0 - exp(-K*R)*Sum) / (n*R*Term);
   };
}

/***************************************************************/
/***************************************************************/
/***************************************************************/
// g(r) = exp(I*K*r)/r
cdouble In_EIKROverR(int n, cdouble K, double R)
{ 
  cdouble KR=K*R;

  if ( (imag(KR)) > 40.0  )
   return factorial(n-1) / (R*pow(-II*K*R,n));
  else if ( real(K)==0.0 )
   return In_EMKROverR(n, imag(K), R);
  else if ( abs(KR) < 0.1 )
   return exp(II*K*R)*ExpRel(n,-II*K*R) / ((double)(n)*R);
  else 
   { cdouble Sum, Term, miKR=-II*KR;
     int m;
     for(Sum=0.0, Term=1.0, m=0; m<n; m++)
      { 
        Sum+=Term;
        Term *= miKR/((double)(m+1));
      };
     return (1.0 - exp(II*K*R)*Sum) / (n*R*Term);
   };
}

cdouble In_GradEMKROverR(int n, double K, double R)
{
  double KR=K*R;

  if ( abs(KR) < 0.1 )
   return exp(-KR)*ExpRel(n,KR) / ((double)(n)*R);
  else 
   { cdouble Sum, Term;
     int m;
     for(Sum=0.0, Term=1.0, m=0; m<n; m++)
      { 
        Sum+=Term;
        Term *= KR/((double)(m+1));
      };
     return (1.0 - exp(-K*R)*Sum) / (n*R*Term);
   };
}

// g(r) = (ikr-1.0)*exp(ikr)/r^3
cdouble In_GradEIKROverR(int n, cdouble K, double R)
{ 

  double PreFac=((double)(n-1))/((double)(n-2));
  cdouble KR=K*R;

  if ( (imag(KR)) > 40.0  )
   return factorial(n-1) / (R*pow(-II*KR,n));
  else if ( real(K)==0.0 )
   return In_GradEMKROverR(n, imag(K), R);
  else if ( abs(KR) < 0.1 )
   return exp(II*KR)*( PreFac*ExpRel(n-2,-II*KR) - 1.0 ) / (R*R*R); 
  else 
   { cdouble Sum, Term, miKR=-II*KR;
     int m;
     for(Sum=0.0, Term=1.0, m=0; m<n-2; m++)
      { 
        Sum+=Term;
        Term *= miKR/((double)(m+1));
      };
     Sum+=Term/PreFac;
     return PreFac*(1.0 - exp(II*KR)*Sum) / (((double)n)*Term*R*R*R);
   };
}

/***************************************************************/
/***************************************************************/
/***************************************************************/
int main(int argc, char *argv[])
{ 
  /*--------------------------------------------------------------*/
  /*- process options  -------------------------------------------*/
  /*--------------------------------------------------------------*/
  cdouble K=0.0;
  double R=0.2;
  int n=0;
  int NTimes=1000;
  /* name               type    #args  max_instances  storage           count         description*/
  OptStruct OSArray[]=
   { {"K",      PA_CDOUBLE, 1, 1, (void *)&K,       0,  "K"},
     {"R",      PA_DOUBLE,  1, 1, (void *)&R,       0,  "R"},
     {"n",      PA_INT,     1, 1, (void *)&n,       0,  "n"},
     {"NTimes", PA_INT,     1, 1, (void *)&NTimes,  0,  "number of times"},
     {0,0,0,0,0,0,0}
   };
  ProcessOptions(argc, argv, OSArray);

  /*--------------------------------------------------------------*/
  /*--------------------------------------------------------------*/
  /*--------------------------------------------------------------*/
  cdouble HRNew, HROld;
  double ElapsedOld, ElapsedNew;
  int nTimes;

  Tic();
  for(nTimes=0; nTimes<NTimes; nTimes++)
   HROld=In_EIKROverR_libRWG(n, K, R);
  ElapsedOld=Toc();

  Tic();
  for(nTimes=0; nTimes<NTimes; nTimes++)
   HRNew=In_EIKROverR(n, K, R);
  ElapsedNew=Toc();

  printf("\n\nIn_EIKROverR: \n");
  printf("Old: %.3f us\n",ElapsedOld*1.0e6);
  printf("New: %.3f us\n",ElapsedNew*1.0e6);

  SetDefaultCD2SFormat("(%+16.8e,%+16.8e)");
  printf("%35s %35s %s\n","                OLD                ","                NEW                "," RD "); 
  printf("%35s-%35s-%s\n","-----------------------------------","-----------------------------------","-----"); 
  printf("%s %s %e\n",CD2S(HROld),CD2S(HRNew),RD(HROld,HRNew));

  /*--------------------------------------------------------------*/
  /*--------------------------------------------------------------*/
  /*--------------------------------------------------------------*/
  Tic();
  for(nTimes=0; nTimes<NTimes; nTimes++)
   HROld=In_GradEIKROverR_libRWG(n, K, R);
  ElapsedOld=Toc();

  Tic();
  for(nTimes=0; nTimes<NTimes; nTimes++)
   HRNew=In_GradEIKROverR(n, K, R);
  ElapsedNew=Toc();

  printf("\n\nIn_GradEIKROverR: \n");
  printf("Old: %.3f us\n",ElapsedOld*1.0e6);
  printf("New: %.3f us\n",ElapsedNew*1.0e6);

  SetDefaultCD2SFormat("(%+16.8e,%+16.8e)");
  printf("%35s %35s %s\n","                OLD                ","                NEW                "," RD "); 
  printf("%35s-%35s-%s\n","-----------------------------------","-----------------------------------","-----"); 
  printf("%s %s %e\n\n",CD2S(HROld),CD2S(HRNew),RD(HROld,HRNew));

}
