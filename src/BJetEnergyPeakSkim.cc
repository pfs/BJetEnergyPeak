#include "UserCode/BJetEnergyPeak/interface/BJetEnergyPeakSkim.h"

using namespace std;

//
BJetEnergyPeakSkim::BJetEnergyPeakSkim(TString outDir) : outDir_(outDir)
{
  //read pileup weights
  TString puWgtUrl("${CMSSW_BASE}/src/TopLJets2015/UserCode/BJetEnergyPeak/pileupWgts.root");
  gSystem->ExpandPathName(puWgtUrl);
  TFile *fIn=TFile::Open(puWgtUrl);
  puWgtGr_.push_back( (TGraph *)fIn->Get("puwgts_nom") );
  puWgtGr_.push_back( (TGraph *)fIn->Get("puwgts_down") );
  puWgtGr_.push_back( (TGraph *)fIn->Get("puwgts_up") );
  fIn->Close();
  
  //JES uncertainties
  jecUncSrcs_.push_back("Absolute");        
  jecUncSrcs_.push_back("HighPtExtra");    
  jecUncSrcs_.push_back("SinglePionECAL");  
  jecUncSrcs_.push_back("SinglePionHCAL"); 
  jecUncSrcs_.push_back("Time");
  jecUncSrcs_.push_back("RelativeJEREC1");  
  jecUncSrcs_.push_back("RelativeJEREC2"); 
  jecUncSrcs_.push_back("RelativeJERHF");
  jecUncSrcs_.push_back("RelativePtBB");    
  jecUncSrcs_.push_back("RelativePtEC1");  
  jecUncSrcs_.push_back("RelativePtEC2");   
  jecUncSrcs_.push_back("RelativePtHF");  
  jecUncSrcs_.push_back("RelativeFSR");
  jecUncSrcs_.push_back("RelativeStatEC2"); 
  jecUncSrcs_.push_back("RelativeStatHF");
  jecUncSrcs_.push_back("PileUpDataMC");    
  jecUncSrcs_.push_back("PileUpPtBB");     
  jecUncSrcs_.push_back("PileUpPtEC");      
  jecUncSrcs_.push_back("PileUpPtHF");    
  jecUncSrcs_.push_back("PileUpBias");
  jecUncSrcs_.push_back("FlavorPureGluon"); 
  jecUncSrcs_.push_back("FlavorPureQuark");
  jecUncSrcs_.push_back("FlavorPureCharm"); 
  jecUncSrcs_.push_back("FlavorPureBottom");
  TString jecUncUrl("${CMSSW_BASE}/src/UserCode/BJetEnergyPeak/data/Summer15_25nsV6M3_DATA_UncertaintySources_AK4PFchs.txt");
  gSystem->ExpandPathName(jecUncUrl);
  for(size_t i=0; i<jecUncSrcs_.size(); i++)
    {
      JetCorrectorParameters *p = new JetCorrectorParameters(jecUncUrl.Data(), jecUncSrcs_[i].Data());
      jecUncs_.push_back( new JetCorrectionUncertainty(*p) );
    }
}

//
void BJetEnergyPeakSkim::processFile(TString inFile,TH1F *xsecWgt,Bool_t isData)
{
  BTagSummaryEvent_t ev;

  Float_t LepSelEffWeights[3]={1.0,1.0,1.0};
  Float_t PUWeights[3]={1.0,1.0,1.0};
  Float_t JetUncs[100][25];
  for(Int_t i=0; i<100; i++)
    for(Int_t j=0; j<25; j++)
      JetUncs[i][j]=1.0;

  //prepare output file
  TString outFile(Form("%s/%s",outDir_.Data(),gSystem->BaseName(inFile)));
  TFile *outF=TFile::Open(outFile);
  TTree *outT=new TTree("data","data");
  outT->Branch("Run"        ,    &ev.Run,            "Run/I");
  outT->Branch("Evt"        ,    &ev.Evt,            "Evt/I");
  outT->Branch("LumiBlock"  ,    &ev.LumiBlock,      "LumiBlock/I");
  outT->Branch("nPV"        ,    &ev.nPV,            "nPV/I");
  outT->Branch("nPUtrue",        &ev.nPUtrue,        "nPUtrue/F");
  outT->Branch("PUWeights",      PUWeights,          "PUWeights[3]/F");
  outT->Branch("LepSelEffWeights",  LepSelEffWeights,      "LepSelEffWeights[3]/F");
  outT->Branch("TrigWord",       &ev.ttbar_trigWord, "TrigWord/I");
  outT->Branch("nLepton"   ,     &ev.ttbar_nl,       "nLepton/I");
  outT->Branch("Lepton_pt"  ,     ev.ttbar_lpt,      "Lepton_pt[nLepton]/F"); 
  outT->Branch("Lepton_eta" ,     ev.ttbar_leta,     "Lepton_eta[nLepton]/F");
  outT->Branch("Lepton_phi" ,     ev.ttbar_lphi,     "Lepton_phi[nLepton]/F");
  outT->Branch("Lepton_id"  ,     ev.ttbar_lid,      "Lepton_id[nLepton]/F");
  outT->Branch("Lepton_gid" ,     ev.ttbar_lgid,     "Lepton_gid[nLepton]/F");
  outT->Branch("Lepton_ch"  ,     ev.ttbar_lch,      "Lepton_ch[nLepton]/F");
  outT->Branch("MET_pt",         &ev.ttbar_metpt,    "MET_pt");
  outT->Branch("MET_phi",        &ev.ttbar_metphi,   "MET_phi");
  outT->Branch("nGenWeight",     &ev.ttbar_nw,       "nGenWeight");
  outT->Branch("GenWeights",      ev.ttbar_w,        "GenWeights[nGenWeight]/F");
  outT->Branch("nJet",           &ev.nJet,           "nJet/I");
  outT->Branch("Jet_uncs",        JetUncs,           "Jet_uncs[nJet][25]/F");
  outT->Branch("Jet_pt",          ev.Jet_pt,         "Jet_pt[nJet]/F");
  outT->Branch("Jet_genpt",       ev.Jet_genpt,      "Jet_genpt[nJet]/F");
  outT->Branch("Jet_eta",         ev.Jet_eta,        "Jet_eta[nJet]/F");
  outT->Branch("Jet_phi",         ev.Jet_phi,        "Jet_phi[nJet]/F");
  outT->Branch("Jet_mass",        ev.Jet_mass,       "Jet_mass[nJet]/F");
  outT->Branch("Jet_CombIVF",     ev.Jet_CombIVF,    "Jet_CombIVF[nJet]/F");
  outT->Branch("Jet_flavour",     ev.Jet_flavour,    "Jet_flavour[nJet]/I");

  //open input file
  TFile *inF=TFile::Open(inFile);
  TTree *inT=(TTree *)inF->Get("btagana/ttree");
  inT->SetBranchAddress("Run"        ,    &ev.Run);
  inT->SetBranchAddress("Evt"        ,    &ev.Evt);
  inT->SetBranchAddress("LumiBlock"  ,    &ev.LumiBlock);
  inT->SetBranchAddress("nPV"        ,    &ev.nPV);
  inT->SetBranchAddress("nPUtrue",        &ev.nPUtrue );
  inT->SetBranchAddress("ttbar_chan" ,    &ev.ttbar_chan);
  inT->SetBranchAddress("ttbar_trigWord", &ev.ttbar_trigWord);
  inT->SetBranchAddress("ttbar_nl"   ,    &ev.ttbar_nl);
  inT->SetBranchAddress("ttbar_lpt"  ,     ev.ttbar_lpt); 
  inT->SetBranchAddress("ttbar_leta" ,     ev.ttbar_leta);
  inT->SetBranchAddress("ttbar_lphi" ,     ev.ttbar_lphi);
  inT->SetBranchAddress("ttbar_lm"   ,     ev.ttbar_lm);
  inT->SetBranchAddress("ttbar_lid"  ,     ev.ttbar_lid);
  inT->SetBranchAddress("ttbar_lgid" ,     ev.ttbar_lgid);
  inT->SetBranchAddress("ttbar_lch"  ,     ev.ttbar_lch);
  inT->SetBranchAddress("ttbar_metpt",    &ev.ttbar_metpt);
  inT->SetBranchAddress("ttbar_metphi",   &ev.ttbar_metphi);
  inT->SetBranchAddress("ttbar_nw",       &ev.ttbar_nw);
  inT->SetBranchAddress("ttbar_w",         ev.ttbar_w);
  inT->SetBranchAddress("nJet",           &ev.nJet);
  inT->SetBranchAddress("Jet_pt",          ev.Jet_pt);
  inT->SetBranchAddress("Jet_genpt",       ev.Jet_genpt);
  inT->SetBranchAddress("Jet_eta",         ev.Jet_eta);
  inT->SetBranchAddress("Jet_phi",         ev.Jet_phi);
  inT->SetBranchAddress("Jet_mass",        ev.Jet_mass);
  inT->SetBranchAddress("Jet_CombIVF",     ev.Jet_CombIVF);
  inT->SetBranchAddress("Jet_flavour",     ev.Jet_flavour);
  Int_t nentries=inT->GetEntriesFast();
  std::cout << "...opening " << inFile << " -> analysing " << nentries << " events -> " << outF->GetName();
  if(xsecWgt) std::cout << " xsec weight=" << xsecWgt->GetBinContent(1);
  if(isData)  std::cout << " is data";
  std::cout << std::endl;

  for(Int_t i=0; i<nentries; i++)
    {
      inT->GetEntry(i);
      
      //progress bar
      if(i%100==0) std::cout << "\r[ " << int(100.*i/nentries) << "/100 ] to completion" << std::flush;

      //
      //CHANNEL ASSIGNMENT 
      //
      if(ev.ttbar_nl<2 || ev.nJet<2) continue;
      ev.ttbar_chan=ev.ttbar_lid[0]*ev.ttbar_lch[0]*ev.ttbar_lid[1]*ev.ttbar_lch[1]; 
      if(ev.ttbar_chan != -143) continue;

      //check if e-mu trigger fired
      bool hasTrigger( ((ev.ttbar_trigWord>>0) & 1) );
      hasTrigger |= ((ev.ttbar_trigWord>>1) & 1);
      if(!hasTrigger) continue;
      
      //generator level weights
      if(xsecWgt)
	for(Int_t k=0; k<ev.ttbar_nw; k++) 
	  ev.ttbar_w[k] *= xsecWgt->GetBinContent(k+1);

      //pileup weights
      if(!isData && puWgtGr_.size()==3)
	{
	  PUWeights[0] = puWgtGr_[0]->Eval(ev.nPUtrue);
	  PUWeights[1] = puWgtGr_[1]->Eval(ev.nPUtrue);
	  PUWeights[2] = puWgtGr_[2]->Eval(ev.nPUtrue);
	}
        
      //efficiency weights
      if(!isData)
	{
	  std::pair<float,float> eff1SF=getLeptonSelectionEfficiencyScaleFactor(ev.ttbar_lid[0],ev.ttbar_lpt[0],ev.ttbar_leta[0]);
	  std::pair<float,float> eff2SF=getLeptonSelectionEfficiencyScaleFactor(ev.ttbar_lid[0],ev.ttbar_lpt[0],ev.ttbar_leta[0]);
	  LepSelEffWeights[0]=eff1SF.first*eff2SF.first;
	  float unc=sqrt( pow(eff1SF.first*eff2SF.second,2)+pow(eff2SF.first*eff1SF.second,2) );
	  LepSelEffWeights[1]=LepSelEffWeights[0]+unc;
	  LepSelEffWeights[2]=LepSelEffWeights[0]-unc;
	}


      //dilepton invariant mass requirement
      std::vector<TLorentzVector> lp4;
      for(Int_t il=0; il<2; il++)
	{
	  lp4.push_back( TLorentzVector(0,0,0,0) );
          lp4[il].SetPtEtaPhiM(ev.ttbar_lpt[il],ev.ttbar_leta[il],ev.ttbar_lphi[il],0.);
	}
      TLorentzVector dilepton(lp4[0]+lp4[1]);
      Float_t mll=dilepton.M();
      if(mll<12) continue;


      //
      //JET/MET SELECTION
      //
      int lastSelJetIdx=0;
      for(Int_t ij=0; ij<ev.nJet; ij++)
	{      
	  //convert to P4
	  TLorentzVector jp4(0,0,0,0);
	  jp4.SetPtEtaPhiM(ev.Jet_pt[ij],ev.Jet_eta[ij],ev.Jet_phi[ij],ev.Jet_mass[ij]);

	  //cross clean wrt to leptons
	  Float_t minDRlj(9999.);
	  for(size_t il=0; il<2; il++) minDRlj = TMath::Min( (Float_t)minDRlj, (Float_t)lp4[il].DeltaR(jp4) );
	  if(minDRlj<0.4) continue;
	
	  ev.Jet_pt[lastSelJetIdx]=ev.Jet_pt[ij];
	  ev.Jet_eta[lastSelJetIdx]=ev.Jet_eta[ij];
	  ev.Jet_phi[lastSelJetIdx]=ev.Jet_phi[ij];
	  ev.Jet_mass[lastSelJetIdx]=ev.Jet_mass[ij];
	  ev.Jet_genpt[lastSelJetIdx]=ev.Jet_genpt[ij];
	  ev.Jet_CombIVF[lastSelJetIdx]=ev.Jet_CombIVF[ij];
	  ev.Jet_flavour[lastSelJetIdx]=ev.Jet_flavour[ij];
  
	  //smear resolution 
	  std::vector<float> jerSF= getJetResolutionScales(jp4.Pt(), jp4.Eta(), ev.Jet_genpt[ij]);
	  jp4*=jerSF[0];
	  JetUncs[lastSelJetIdx][0]=fabs(1-jerSF[1]);
	  for(size_t iunc=0; iunc<jecUncs_.size(); iunc++)
	    {
	      jecUncs_[iunc]->setJetEta(fabs(jp4.Eta()));
	      jecUncs_[iunc]->setJetPt(jp4.Pt());
	      JetUncs[lastSelJetIdx][iunc+1]=jecUncs_[iunc]->getUncertainty(true);
	    }

	  //update last jet selected
	  lastSelJetIdx=ij;
	}

      if(lastSelJetIdx<1) continue;
      outT->Fill();
    }

  //all done
  inF->Close();
  outF->Close();
}

//Source (50ns) : CMS AN 022/2015 v15
std::pair<float,float> BJetEnergyPeakSkim::getLeptonSelectionEfficiencyScaleFactor(int id,float pt,float eta)
{
  std::pair<float,float>res(1.0,0.0);
 
  //electrons
  if(abs(id)==11)
    {
      if (fabs(eta)<0.8)
	{
	  if (pt<30)      { res.first=0.927; res.second=0.073; }
	  else if (pt<40) { res.first=0.975; res.second=0.018; }
	  else if (pt<50) { res.first=0.962; res.second=0.036; }
	  else            { res.first=0.955; res.second=0.022; }
	}
      else if (fabs(eta)<1.5)
	{
	  if (pt<30)      { res.first=0.891; res.second=0.074; }
	  else if (pt<40) { res.first=0.965; res.second=0.020; }
	  else if (pt<50) { res.first=0.968; res.second=0.018; }
	  else            { res.first=0.955; res.second=0.018; }
	}
      else
	{
	  if (pt<30)      { res.first=0.956; res.second=0.059; }
	  else if (pt<40) { res.first=0.995; res.second=0.018; }
	  else if (pt<50) { res.first=0.993; res.second=0.019; }
	  else            { res.first=0.985; res.second=0.023; }
	}
    }

  //muons
  if (abs(id)==13)
    {
      if (fabs(eta)<0.9)
	{
	  if (pt<30)      { res.first=1.003; res.second=0.019; }
	  else if (pt<40) { res.first=1.014; res.second=0.015; }
	  else if (pt<50) { res.first=1.001; res.second=0.014; }
	  else            { res.first=0.983; res.second=0.014; }
	}
      else if(fabs(eta)<1.2)
	{
	  if (pt<30)      { res.first=0.993; res.second=0.019; }
	  else if (pt<40) { res.first=0.994; res.second=0.015; }
	  else if (pt<50) { res.first=0.980; res.second=0.014; }
	  else            { res.first=0.987; res.second=0.015; }
	}
      else
	{
	  if (pt<30)      { res.first=1.023; res.second=0.028; }
	  else if (pt<40) { res.first=0.994; res.second=0.014; }
	  else if (pt<50) { res.first=0.996; res.second=0.014; }
	  else            { res.first=0.979; res.second=0.014; }
	}
    }

  return res;
}

//Source:  https://twiki.cern.ch/twiki/bin/viewauth/CMS/JetResolution
std::vector<float> BJetEnergyPeakSkim::getJetResolutionScales(float pt, float eta, float genjpt)
{
  std::vector<float> res(3,1.0);

  float ptSF(1.0), ptSF_err(0.0);
  if(TMath::Abs(eta)<0.8)       { ptSF=1.061; ptSF_err = 0.023; }
  else if(TMath::Abs(eta)<1.3)  { ptSF=1.088; ptSF_err = 0.029; }
  else if(TMath::Abs(eta)<1.9)  { ptSF=1.106; ptSF_err = 0.030; }
  else if(TMath::Abs(eta)<2.5)  { ptSF=1.126; ptSF_err = 0.094; }
  else if(TMath::Abs(eta)<3.0)  { ptSF=1.343; ptSF_err = 0.123; }
  else if(TMath::Abs(eta)<3.2)  { ptSF=1.303; ptSF_err = 0.111; }
  else if(TMath::Abs(eta)<5.0)  { ptSF=1.320; ptSF_err = 0.286; }

  res[0] = TMath::Max((Float_t)0.,(Float_t)(genjpt+(ptSF)*(pt-genjpt)))/pt;
  res[1] = TMath::Max((Float_t)0.,(Float_t)(genjpt+(ptSF-ptSF_err)*(pt-genjpt)))/pt;
  res[2] = TMath::Max((Float_t)0.,(Float_t)(genjpt+(ptSF+ptSF_err)*(pt-genjpt)))/pt;
  
  return res;
}

