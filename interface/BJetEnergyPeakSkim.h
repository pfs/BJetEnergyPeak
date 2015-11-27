#ifndef _BJetEnergyPeakSkim_h_
#define _BJetEnergyPeakSkim_h_

#include "TFile.h"
#include "TH1F.h"
#include "TString.h"
#include "TTree.h"
#include "TSystem.h"
#include "TGraph.h"
#include "TLorentzVector.h"

#include <iostream>
#include <map>
#include <vector>

#include "CondFormats/JetMETObjects/interface/JetCorrectionUncertainty.h"
#include "CondFormats/JetMETObjects/interface/JetCorrectorParameters.h"

#include "UserCode/BJetEnergyPeak/interface/BTagSummaryEvent.h"


/**
   @class BJetEnergyPeakSkim
   @short wrapper to skim events for the b-jet energy peak analysis
   @author psilva@cern.ch
 */

class BJetEnergyPeakSkim
{
 public:
  /**
     @short CTOR
   */
  BJetEnergyPeakSkim();

  /**
     @short read a file and dump the output 
   */
  void processFile(TString inFile, TString outFile,TH1F *xsecWgt,Bool_t isData);

  /**
     @short DTOR
   */
  ~BJetEnergyPeakSkim()
    {
      for(size_t i=0; i<jecUncs_.size(); i++) delete jecUncs_[i];
    }

 private:

  /**
     @short hardcoded lepton selection efficiencies
   */
  std::pair<float,float> getLeptonSelectionEfficiencyScaleFactor(int id,float pt,float eta);

  /**
     @short hardocded jet energy resolutions
   */
  std::vector<float> getJetResolutionScales(float pt, float eta, float genjpt);

  //pileup weighting graphs
  std::vector<TGraph *>puWgtGr_;

  //jet energy scale uncertainty sources
  std::vector<TString> jecUncSrcs_;
  std::vector<JetCorrectionUncertainty *> jecUncs_;
};

#endif
