/// \ingroup newran
///@{

/// \file tryrand2.cpp
/// Test of newran - more histograms.

#define WANT_STREAM
#define WANT_MATH

#include "include.h"
#include "newran.h"
#include "format.h"
#include "utility.h"
#include "tryrand.h"


void test2(int n)
{
   {
      Uniform u;
      SumRandom sr1 = -u;
      SumRandom sr2 = 5.0-u;
      SumRandom sr3 = 5.0-2*u;
      MixedRandom sr4 = u(0.5) + (-u)(0.5);
      Histogram(&sr1,n);
      cout << "Mean and variance should be -0.5 and 0.083333" << endl;
      Histogram(&sr2,n);
      cout << "Mean and variance should be 4.5 and 0.083333" << endl;
      Histogram(&sr3,n);
      cout << "Mean and variance should be 4.0 and 0.33333" << endl;
      Histogram(&sr4,n);
      cout << "Mean and variance should be 0.0 and 0.33333" << endl;
   }
   {
      Uniform u;
      SumRandom sr1 = u*u;
      SumRandom sr2 = (u-0.5)*(u-0.5);
      Histogram(&sr1,n);
      cout << "Mean and variance should be 0.25 and 0.048611" << endl;
      Histogram(&sr2,n);
      cout << "Mean and variance should be 0.0 and 0.006944" << endl;
   }
   {
      static Real probs[]={.4,.2,.4};
      DiscreteGen discrete(3,probs); Normal nn;
      SumRandom sr=discrete+(nn*0.25)(2)+10.0;
      Histogram(&discrete,n);
      Histogram(&sr,n);
   }
   {
      static Real probs[]={.2,.8};
      Random* rv[2];
      Normal nn; SumRandom snn=nn*10.0;
      rv[0]=&snn; rv[1]=&nn;
      MixedRandom mr(2,probs,rv);
      MixedRandom mr2=snn(.2)+nn(.8);
      Histogram(&mr2,n);
      Histogram(&mr,n);
   }

   {
      Normal nn; Constant c1(0.0); Constant c2(10.0);
      MixedRandom mr=c1(0.25)+(nn+5.0)(0.5)+c2(0.25);
      Histogram(&mr,n);
   }
   {
      Cauchy cy; Normal nn; SumRandom sr = cy*.01+nn+2.0;
      MixedRandom mr=sr(0.1)+nn(0.9);
      Histogram(&sr,n);
      Histogram(&mr,n);
   }
   {
      Constant c0(0.0); Constant c1(1.0); Constant c2(2.0);
      Constant c3(3.0); Constant c4(4.0); Constant c5(5.0);
      MixedRandom mr=c0(.1)+c1(.2)+c2(.2)+c3(.2)+c4(.2)+c5(.1);
      Histogram(&mr,n);
   }
   {
      Uniform u; Normal nl;
      MixedRandom m=( u(3)-1.5 )(0.5)+( nl*0.5+10.0 )(0.5);
      Histogram(&m,n);
   }
   {
      Real prob[] = { .25, .25, .25, .25 };
      Real val[] = { 3, 1.5, 1, 0.75 };
      DiscreteGen X(4, prob, val);
      SumRandom Y = 1/X;
      Histogram(&Y,n);  // mean should be 0.83333, variance should be 0.13889
      cout << "Mean and variance should be 0.83333 and 0.13889" << endl;
      Uniform U;
      SumRandom Z = U/X;
      Histogram(&Z,n);  // mean should be 0.41667, variance should be 0.10417
      cout << "Mean and variance should be 0.41667 and 0.10417" << endl;
   }
   {
      int M = 5, N = 9;
      ChiSq Num(M); ChiSq Den(N);
      SumRandom F_dist = (double)N/(double)M * Num / Den;
      Histogram(&F_dist,n);
      Format F; F.FormatType(Format::SIG_FIGS); F.Precision(5);
      F.MaxWidth(10); F.MinWidth(1);
      cout << "Mean and variance should be ";
      cout << F << N / (double)(N-2);
      cout << " and ";
      cout << F << 2 * N * N * (M+N-2) / (double)(M * (N-2) * (N-2) * (N-4));
      cout << endl;
   }
   {
      Real rho[117];
      rho[0] = 0;
      rho[1] = 0.0909091;
      rho[2] = 0.458498;
      rho[3] = 0.152833;
      rho[4] = 0.0764164;
      rho[5] = 0.0458498;
      rho[6] = 0.0305665;
      rho[7] = 0.0218332;
      rho[8] = 0.0163749;
      rho[9] = 0.0127361;
      rho[10] = 0.0101888;
      rho[11] = 0.00833633;
      rho[12] = 0.00694694;
      rho[13] = 0.00587818;
      rho[14] = 0.00503844;
      rho[15] = 0.00436665;
      rho[16] = 0.00382082;
      rho[17] = 0.00337131;
      rho[18] = 0.00299672;
      rho[19] = 0.00268128;
      rho[20] = 0.00241315;
      rho[21] = 0.00218332;
      rho[22] = 0.00198484;
      rho[23] = 0.00181225;
      rho[24] = 0.00166122;
      rho[25] = 0.00152833;
      rho[26] = 0.00141076;
      rho[27] = 0.00130626;
      rho[28] = 0.00121296;
      rho[29] = 0.00112931;
      rho[30] = 0.00105402;
      rho[31] = 0.000986017;
      rho[32] = 0.000924391;
      rho[33] = 0.000868368;
      rho[34] = 0.000817287;
      rho[35] = 0.000770585;
      rho[36] = 0.000727775;
      rho[37] = 0.000688436;
      rho[38] = 0.000652202;
      rho[39] = 0.000618756;
      rho[40] = 0.000587818;
      rho[41] = 0.000559144;
      rho[42] = 0.000532518;
      rho[43] = 0.00050775;
      rho[44] = 0.00048467;
      rho[45] = 0.000463129;
      rho[46] = 0.000442993;
      rho[47] = 0.000424143;
      rho[48] = 0.00040647;
      rho[49] = 0.000389879;
      rho[50] = 0.000374284;
      rho[51] = 0.000359606;
      rho[52] = 0.000345775;
      rho[53] = 0.000332727;
      rho[54] = 0.000320404;
      rho[55] = 0.000308753;
      rho[56] = 0.000297726;
      rho[57] = 0.00028728;
      rho[58] = 0.000277373;
      rho[59] = 0.000267971;
      rho[60] = 0.000259038;
      rho[61] = 0.000250545;
      rho[62] = 0.000242463;
      rho[63] = 0.000234766;
      rho[64] = 0.00022743;
      rho[65] = 0.000220432;
      rho[66] = 0.000213752;
      rho[67] = 0.000207371;
      rho[68] = 0.000201272;
      rho[69] = 0.000195438;
      rho[70] = 0.000189854;
      rho[71] = 0.000184506;
      rho[72] = 0.000179381;
      rho[73] = 0.000174467;
      rho[74] = 0.000169751;
      rho[75] = 0.000165225;
      rho[76] = 0.000160877;
      rho[77] = 0.000156698;
      rho[78] = 0.00015268;
      rho[79] = 0.000148815;
      rho[80] = 0.000145094;
      rho[81] = 0.000141512;
      rho[82] = 0.00013806;
      rho[83] = 0.000134734;
      rho[84] = 0.000131526;
      rho[85] = 0.000128431;
      rho[86] = 0.000125444;
      rho[87] = 0.00012256;
      rho[88] = 0.000119775;
      rho[89] = 0.000117083;
      rho[90] = 0.000114481;
      rho[91] = 0.000111965;
      rho[92] = 0.000109531;
      rho[93] = 0.000107176;
      rho[94] = 0.000104895;
      rho[95] = 0.000102687;
      rho[96] = 0.000100548;
      rho[97] = 9.84747e-05;
      rho[98] = 9.6465e-05;
      rho[99] = 9.45162e-05;
      rho[100] = 9.26259e-05;
      rho[101] = 9.07917e-05;
      rho[102] = 8.90115e-05;
      rho[103] = 8.72831e-05;
      rho[104] = 8.56046e-05;
      rho[105] = 8.3974e-05;
      rho[106] = 8.23896e-05;
      rho[107] = 8.08496e-05;
      rho[108] = 7.93524e-05;
      rho[109] = 7.78964e-05;
      rho[110] = 7.64801e-05;
      rho[111] = 7.51021e-05;
      rho[112] = 7.3761e-05;
      rho[113] = 7.24554e-05;
      rho[114] = 7.11843e-05;
      rho[115] = 6.99463e-05;
      rho[116] = 6.87403e-05;

      Real sum = 0.0, s1 = 0.0, s2 = 0.0; int i;
      for (i = 0; i < 117; ++i) sum += rho[i]; 
      for (i = 0; i < 117; ++i) rho[i] /= sum;
      for (i = 0; i < 117; ++i) { s1 += i*rho[i]; s2 += i*i*rho[i]; }
      
      DiscreteGen X(117, rho);
      Histogram(&X,n);
      Format F; F.FormatType(Format::SIG_FIGS); F.Precision(5);
      Format F1; F1.FormatType(Format::SIG_FIGS); F1.Precision(10);
      cout << "Mean and variance should be ";
      cout << F << s1;
      cout << " and ";
      cout << F << s2 - s1 * s1;
      cout << endl;
      cout << "Renormalising sum = " << F1 << sum << endl;
   }

}

///@}

