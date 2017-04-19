/// \ingroup newran
///@{

/// \file test_out.cpp
/// Statistical tests - bodies.


// *************** classes for carrying out tests *********************** //

// definitions of library classes and function

#define WANT_MATH
#define WANT_STREAM

#include "include.h"
#include "str.h"
#include "utility.h"
#include "test_out.h"
#include "format.h"

#ifdef use_namespace
using namespace NEWRAN;
using namespace RBD_STRING;
using namespace RBD_COMMON;
#endif


const char* BaseTest::Header =
"Test          distrib.    param. 1   param. 2  statistic   n. stat. -lg sig";

void BaseTest::WrapUp(double sig_prob)
{
   SigProb = sig_prob;

   if (sig_prob == 0.0) cout << "  9999 ";
   
   else if (sig_prob <= 0.1)
   {
      
      Format F6; F6.FormatType(Format::DEC_FIGS);
      F6.Width(6); F6.Precision(2); F6.Suffix(" ");
      cout  << F6 << -log10(sig_prob);
   }
   else cout << "   < 1 ";

   if (sig_prob <= 0.001) cout << "***";
   else if (sig_prob <= 0.01) cout << "** ";
   else if (sig_prob <= 0.05) cout << "*  ";
   else cout << "   ";

   cout << endl;
}

void NormalTestUpper::DoTest()
{
   double sd = sqrt(Var);
   double nts = (Statistic - Mean) / sd;
   Format S12; S12.Width(12); S12.Suffix(" "); S12.Alignment(Format::LEFT);
   Format F10;
   F10.FormatType(Format::SIG_FIGS);
   F10.Precision(5); F10.Width(10); F10.Suffix(" ");
   Format D10;
   D10.FormatType(Format::DEC_FIGS);
   D10.Precision(5); D10.Width(10); D10.Suffix(" ");
   cout << S12 << LongName;
   cout << "Norm. (up) ";
   cout << F10 << Mean << sd << Statistic << D10 << nts;

   WrapUp(cdfnml(-nts));
}

void NormalTestTwoSided::DoTest()
{
   double sd = sqrt(Var);
   double nts = (Statistic - Mean) / sd;
   Format S12; S12.Width(12); S12.Suffix(" "); S12.Alignment(Format::LEFT);
   Format F10;
   F10.FormatType(Format::SIG_FIGS);
   F10.Precision(5); F10.Width(10); F10.Suffix(" ");
   Format D10;
   D10.FormatType(Format::DEC_FIGS);
   D10.Precision(5); D10.Width(10); D10.Suffix(" ");
   cout << S12 << LongName;
   cout << "Norm. (2s) ";
   cout << F10 << Mean << sd << Statistic << D10 << nts;
   WrapUp(2.0 * cdfnml(-fabs(nts)));
}

void ChiSquaredTestUpper::DoTest()
{
   double nts = (Statistic - DF) / sqrt((Real)(2 * DF));
   Format S12; S12.Width(12); S12.Suffix(" "); S12.Alignment(Format::LEFT);
   Format F10;
   F10.FormatType(Format::SIG_FIGS);
   F10.Precision(5); F10.Width(10); F10.Suffix(" ");
   Format D10;
   D10.FormatType(Format::DEC_FIGS);
   D10.Precision(5); D10.Width(10); D10.Suffix(" ");
   Format I10;
   I10.FormatType(Format::INTEGER);
   I10.Precision(5); I10.Width(10); I10.Suffix(" ");
   cout << S12 << LongName;
   cout << "Chisq (up) ";
   cout << I10 << DF << "" << F10 << Statistic << D10 << nts;
   WrapUp(cdf_chisq(Statistic, DF, true));
}

void ChiSquaredTestLower::DoTest()
{
   double nts = (Statistic - DF) / sqrt((Real)(2 * DF));
   Format S12; S12.Width(12); S12.Suffix(" "); S12.Alignment(Format::LEFT);
   Format F10;
   F10.FormatType(Format::SIG_FIGS);
   F10.Precision(5); F10.Width(10); F10.Suffix(" ");
   Format D10;
   D10.FormatType(Format::DEC_FIGS);
   D10.Precision(5); D10.Width(10); D10.Suffix(" ");
   Format I10;
   I10.FormatType(Format::INTEGER);
   I10.Precision(5); I10.Width(10); I10.Suffix(" ");
   cout << S12 << LongName;
   cout << "Chisq (up) ";
   cout << I10 << DF << "" << F10 << Statistic << D10 << nts;
   WrapUp(cdf_chisq(Statistic, DF, false));
}

void ChiSquaredTestTwoSided::DoTest()
{
   double nts = (Statistic - DF) / sqrt((Real)(2 * DF));
   Format S12; S12.Width(12); S12.Suffix(" "); S12.Alignment(Format::LEFT);
   Format F10;
   F10.FormatType(Format::SIG_FIGS);
   F10.Precision(5); F10.Width(10); F10.Suffix(" ");
   Format D10;
   D10.FormatType(Format::DEC_FIGS);
   D10.Precision(5); D10.Width(10); D10.Suffix(" ");
   Format I10;
   I10.FormatType(Format::INTEGER);
   I10.Precision(5); I10.Width(10); I10.Suffix(" ");
   cout << S12 << LongName;
   cout << "Chisq (2s) ";
   cout << I10 << DF << "" << F10 << Statistic << D10 << nts;
   double Up = cdf_chisq(Statistic, DF, true);
   double Lo = cdf_chisq(Statistic, DF, false);
   WrapUp(2.0 * (Lo < Up ? Lo : Up));
}

void QuasiUniformLower::DoTest()
{
   Format S12; S12.Width(12); S12.Suffix(" "); S12.Alignment(Format::LEFT);
   Format F10;
   F10.FormatType(Format::SIG_FIGS);
   F10.Precision(5); F10.Width(10); F10.Suffix(" ");
   cout << S12 << LongName;
   cout << "Q.U.  (lo) ";
   cout << F10 << "" << "" << Statistic << "";

   WrapUp(Statistic);
}

void UniformLower::DoTest()
{
   Format S12; S12.Width(12); S12.Suffix(" "); S12.Alignment(Format::LEFT);
   Format D10;
   D10.FormatType(Format::DEC_FIGS);
   D10.Precision(5); D10.Width(10); D10.Suffix(" ");
   cout << S12 << LongName;
   cout << "Unif. (lo) ";
   cout << D10 << "" << "" << Statistic << "";

   WrapUp(Statistic);
}

void KS_Upper::DoTest()
{
   Format S12; S12.Width(12); S12.Suffix(" "); S12.Alignment(Format::LEFT);
   Format D10;
   D10.FormatType(Format::DEC_FIGS);
   D10.Precision(5); D10.Width(10); D10.Suffix(" ");
   cout << S12 << LongName;
   cout << "K.S.  (up) ";
   cout << D10 << "" << "" << Statistic << "";

   WrapUp(KS_probabilities(Statistic));
}

///@}

