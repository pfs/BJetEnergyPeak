#ifndef _BTagSummaryEvent_h_
#define _BTagSummaryEvent_h_

struct BTagSummaryEvent_t
{
  Int_t Run,Evt,LumiBlock;
  Int_t nPV;
  Int_t ttbar_chan, ttbar_trigWord;
  Int_t ttbar_nl, ttbar_lid[10], ttbar_lgid[10], ttbar_lch[10];
  Float_t ttbar_lpt[10], ttbar_leta[10], ttbar_lphi[10], ttbar_lm[10];
  Float_t ttbar_metpt,ttbar_metphi;
  Int_t   ttbar_nw;
  Float_t ttbar_w[500];
  Float_t nPUtrue;
  Int_t nJet;
  Float_t Jet_pt[100],Jet_eta[100],Jet_phi[100],Jet_mass[100], Jet_genpt[100], Jet_CombIVF[100];
  Int_t Jet_flavour[100];
};

#endif
