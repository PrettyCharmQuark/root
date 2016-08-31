// @(#)root/gpad:$Id$
// Author: Paul Gessinger   25/08/2016

/*************************************************************************
 * Copyright (C) 1995-2016, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TRatioPlot
#define ROOT_TRatioPlot


//////////////////////////////////////////////////////////////////////////
//                                                                      //
// ROOT_TRatioPlot                                                      //
//                                                                      //
// A collection of histograms                                           //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TH1.h"
#include "TPad.h"
#include "TGraphAsymmErrors.h"
#include "TGraphErrors.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TGaxis.h"
#include "TH1F.h"
#include "TFitResultPtr.h"
#include "THStack.h"

class TBrowser;
class TFileMergeInfo;

class TRatioPlot : public TObject {

private:
   TRatioPlot& operator=(const TRatioPlot&); // Not implemented
   TRatioPlot(const TRatioPlot &hrp);

   enum CalculationMode {
      kDivideHist = 1, ///< Use `TH1::Divide` to create the ratio.
      kDivideGraph = 2, ///< Use `TGraphAsymmErrors::Divide` to create the ratio.
      kDifference = 3, ///< Calculate the difference between the histograms.
      kFitResidual = 4, ///< Calculate the fit residual between the histogram and a fit stored within it.
      kDifferenceSign = 5 ///< Calculate the difference divided by the error.
   };

   enum ErrorMode {
      kErrorSymmetric = 1, ///< Use the regular `TH1::GetBinError` as the error
      kErrorAsymmetric = 2, ///< Use `TH1::GetBinErrorUp` and `TH1::GetBinErrorLow` for the error, depending on y values.
      kErrorFunc = 3 ///< Use the square root of the function value as the error.
   };

   enum HideLabelMode {
      kHideUp = 1, ///< Hide the first label of the upper y axis when there is low space.
      kHideLow = 2, ///< Hide the last label of the lower y axis when there is low space.
      kNoHide = 3, ///< Do not hide labels when there is low space.
      kForceHideUp = 4, ///< Always hide the first label of the upper y axis
      kForceHideLow = 5 ///< Always hide the last label of the lower y axis
   };


protected:

   TVirtualPad *fParentPad = 0; ///< Stores the pad the ratio plot was created in
   TPad *fUpperPad = 0; ///< The pad which contains the upper plot part
   TPad *fLowerPad = 0; ///< The pad which contains the calculated lower plot part
   TPad *fTopPad = 0; ///< The Pad that drawn on top on the others to have consistent coordinates

   TH1 *fH1 = 0; ///< Stores the primary histogram
   TH1 *fH2 = 0; ///< Stores the secondary histogram, if there is one
   TObject *fHistDrawProxy = 0; ///< The object which is actually drawn, this might be TH1 or THStack

   Int_t fMode = 0; ///< Stores which calculation is supposed to be performed as specified by user option
   Int_t fErrorMode = TRatioPlot::ErrorMode::kErrorSymmetric; ///< Stores the error mode, sym, asym or func
   TString fOption = ""; ///< Stores the option which is given in the constructor as a string
   TString fH1DrawOpt = ""; ///< Stores draw option for h1 given in constructor
   TString fH2DrawOpt = ""; ///< Stores draw option for h2 given in constructor
   TString fGraphDrawOpt = ""; ///< Stores draw option for the lower plot graph given in constructor

   Float_t fSplitFraction = 0.3; ///< Stores the fraction at which the upper and lower pads meet

   TGraph *fRatioGraph = 0; ///< Stores the lower plot's graph
   TGraphErrors *fConfidenceInterval1 = 0; ///< Stores the graph for the 1 sigma band
   TGraphErrors *fConfidenceInterval2 = 0; ///< Stores the graph for the 2 sigma band
   Color_t fCi1Color = kGreen; ///< Stores the color for the 1 sigma band
   Color_t fCi2Color = kYellow; ///< Stores the color for the 2 sigma band

   Bool_t fShowConfidenceIntervals = kTRUE; ///< Stores whether to show the confidence interval bands. From Draw option

   Double_t fCl1 = 0.6827; ///< Stores the confidence level for the inner confidence interval band
   Double_t fCl2 = 0.9545; ///< Stores the confidence level for the outer confidence interval band

   Double_t fC1 = 1.; ///< Stores the scale factor for h1 (or THStack sum)
   Double_t fC2 = 1.; ///< Stores the scale factor for h2

   TFitResult *fFitResult = 0; ///< Stores the explicit fit result given in the fit residual case. Can be 0

   TAxis *fSharedXAxis = 0; ///< X axis that stores the range for both plots
   TGaxis *fUpperGXaxis = 0; ///< Upper graphical x axis
   TGaxis *fLowerGXaxis = 0; ///< Lower graphical x axis
   TGaxis *fUpperGYaxis = 0; ///< Upper graphical y axis
   TGaxis *fLowerGYaxis = 0; ///< Lower graphical y axis
   TGaxis *fUpperGXaxisMirror = 0; ///< Upper mirror of the x axis
   TGaxis *fLowerGXaxisMirror = 0; ///< Lower mirror of the x axis
   TGaxis *fUpperGYaxisMirror = 0; ///< Upper mirror of the y axis
   TGaxis *fLowerGYaxisMirror = 0; ///< Lower mirror of the y axis

   TAxis *fUpYaxis = 0; ///< Clone of the upper y axis
   TAxis *fLowYaxis = 0; ///< Clone of the lower y axis

   std::vector<TLine*> fGridlines; ///< Keeps TLine objects for the gridlines
   std::vector<double> fGridlinePositions; ///< Stores the y positions for the gridlines
   Bool_t fShowGridlines = kTRUE; ///< Stores whether to show the gridlines at all
   Int_t fHideLabelMode = TRatioPlot::HideLabelMode::kHideLow; ///< Stores which label to hide if the margin is to narrow, if at all

   // store margins to be able do determine
   // what has changed when user drags
   Float_t fUpTopMargin = 0.1; ///< Stores the top margin of the upper pad
   Float_t fUpBottomMargin = 0.05; ///< Stores the bottom margin of the upper pad
   Float_t fLowTopMargin = 0.05; ///< Stores the top margin of the lower pad
   Float_t fLowBottomMargin = 0.3; ///< Stores the bottom margin of the lower pad

   Float_t fLeftMargin = 0.1; ///< Stores the common left margin of both pads
   Float_t fRightMargin = 0.1; ///< Stores the common right margin of both pads

   Bool_t fIsUpdating = kFALSE; ///< Keeps track of whether its currently updating to reject other calls until done
   Bool_t fIsPadUpdating = kFALSE; ///< Keeps track whether pads are updating during resizing

   virtual void SyncAxesRanges();
   virtual void SetupPads();
   virtual void CreateVisualAxes();
   virtual Bool_t SyncPadMargins();
   virtual void SetPadMargins();
   virtual void CreateGridline();

   virtual void ImportAxisAttributes(TGaxis* gaxis, TAxis* axis);

   virtual Bool_t IsDrawn();

   virtual void Init(TH1* h1, TH1* h2, Option_t *option = "", Option_t *h1DrawOpt = "hist", Option_t *h2DrawOpt = "E", Option_t *graphDrawOpt = "AP");

public:

   TRatioPlot();
   virtual ~TRatioPlot();
   TRatioPlot(TH1* h1, TH1* h2, Option_t *option = "pois", Option_t *h1DrawOpt = "hist", Option_t *h2DrawOpt = "E", Option_t *graphDrawOpt = "AP");

   TRatioPlot(THStack* st, TH1* h2, Option_t *option = "pois", Option_t *h1DrawOpt = "hist", Option_t *h2DrawOpt = "E", Option_t *graphDrawOpt = "AP");

   TRatioPlot(TH1* h1, Option_t *option = "", Option_t *h1DrawOpt = "", Option_t *graphDrawOpt = "LX", TFitResult *fitres = 0);



   virtual void Draw(Option_t *chopt="");
   virtual void Browse(TBrowser *b);

   virtual void BuildLowerPlot();

   virtual void Paint(Option_t *opt = "");

   // Slots for signal receiving
   virtual void UnZoomed();
   virtual void RangeAxisChanged();
   virtual void SubPadResized();

   // Getters
   virtual TAxis *GetXaxis() { return fSharedXAxis; }
   virtual TAxis *GetUpYaxis() { return fUpYaxis; }
   virtual TAxis *GetLowYaxis() { return fLowYaxis; }

   virtual TGraph *GetLowerRefGraph();
   virtual TAxis *GetLowerRefXaxis();
   virtual TAxis *GetLowerRefYaxis();

   virtual TObject *GetUpperRefObject();
   virtual TAxis *GetUpperRefXaxis();
   virtual TAxis *GetUpperRefYaxis();

   virtual TPad * GetUpperPad() { return fUpperPad; }
   virtual TPad * GetLowerPad() { return fLowerPad; }

   // Setters
   virtual void SetFitResult(TFitResultPtr fitres);
   virtual void SetFitResult(TFitResult *fitres); 

   // Setters for margins
   void SetUpTopMargin(Float_t margin);
   void SetUpBottomMargin(Float_t margin);
   void SetLowTopMargin(Float_t margin);
   void SetLowBottomMargin(Float_t margin);
   void SetLeftMargin(Float_t margin);
   void SetRightMargin(Float_t margin);

   virtual void SetSeparationMargin(Float_t);
   virtual Float_t GetSeparationMargin();
   virtual void SetSplitFraction(Float_t sf);
   virtual void SetConfidenceLevels(Double_t cl1, Double_t cl2);

   virtual void SetGridlines(Double_t *gridlines, Int_t numGridlines); 
   virtual void SetGridlines(std::vector<double> gridlines); 

   virtual void SetConfidenceIntervalColors(Color_t ci1 = kGreen, Color_t ci2 = kYellow);

   virtual void SetC1(Double_t c1) { fC1 = c1; }
   virtual void SetC2(Double_t c2) { fC2 = c2; }

   ClassDef(TRatioPlot, 1)  //A ratio of histograms
};

#endif