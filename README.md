# BJetEnergyPeak

## Installation

To install the code do
```
cmsrel CMSSW_7_4_14
cd CMSSW_7_4_14/src
cmsenv
git clone git@github.com:pfs/BJetEnergyPeak.git UserCode/BJetEnergyPeak
scram b
cd UserCode/BJetEnergyPeak
```

## Prepare ntuples for local analysis

This step should be done only once and it's only commissioned to be run at CERN in lxplus. 
It skims the information needed for the analysis in simplified ntuples.
The starting point are the BTV performance ntuples. Further details on how to produce them can be found in 
https://twiki.cern.ch/twiki/bin/view/CMS/BTagCommissioning2015. Before skimming three steps must be done:

* Determine the pileup weights: the output is a ROOT file: data/puweights.root. Run the following script
```
python scripts/runPileupEstimation.py 
```
* Determine the normalization weights for all the samples available. The output is a pickle file: data/genweights.pck. Run the following script
```
python scripts/produceNormalizationCache.py -i /store/group/phys_btag/performance/TTbar/2015_25ns/8622ee3
```
* Project the expected b-tagging efficiency for the TTbar sample. The output is a pickle file: btagefficiencies. Run the following script
```
python scripts/projectExpectedBtagEff.py -i /store/group/phys_btag/performance/TTbar/2015_25ns/8622ee3/MC13TeV_TTJets
```
The files above will be stored by default in github, but if a reprocessing of the original ntuples occurs,
they need to be run again. For the pileup estimation one needs to use the original json file (check with BTV experts).
With the files updated you're now ready to skim the info needed for the ntuples. You can run the following command
```
python scripts/skimBJetEnergyPeakNtuples.py -i /store/group/phys_btag/performance/TTbar/2015_25ns/8622ee3 -o analysis -n 8
```

## Ntuple content

| Variable | Type | Comments |
| :------------ | :------------ | :------------ | 
| Run | I | run number |
| Evt | I | event number |
| LumiBlock | I | lumi section |
| nPV | I | no. primary vertices |
| nPUtrue | F | no. pileup generated |
| PUWeights | [3]/F | pileup weights: nominal/down/up |
| LepSelEffWeights | [3]/F | lepton selection scale factors : nominal/down/up |
| TrigWord | I | each bit is a trigger fired |
| nLepton | I | no. selected leptons |
| Lepton_{pt,eta,phi} | []/F | Lepton kinematics |
| Lepton_{id,gid,ch} | []/I | Lepton id, matched id at gen. level and charge |
| MET_{pt,phi} | F | Missing transverse energy pT and phi |
| nGenWeight | I | no. generator level weights |
| GenWeights | []/I | generator level weights |
| nJet | I | no. jets |
| Jet_{pt,eta,phi,mass} | []/F | jet kinematics |
| Jet_flavour | I | jet flavour at gen. level |
| Jet_genpt | F | jet pt at gen. level |
| Jet_CombIVF  | F| CSV discriminator |
| Jet_uncs | [][29]/F | Jet energy resolution and scale uncertainties |


## Producing and plotting a simple b-jet energy peak distribution

The following is an example of how to run locally over the skimmed ntuples to produce the b-jet energy peak.
To run the event selection and basic filling of histograms using a pre-defined list of samples and cross sections one can use the following script 
```
python scripts/runBJetEnergyPeak.py -i analysis -j data/samples_Run2015_25ns.json -o analysis/plots -n 8
```
The results are stored in ROOT files and can be plotted together and compared to data using
```
python scripts/plotter.py -i analysis/ -j data/samples_Run2015_25ns.json  -l 2444.
```
Under analysis/plots you'll find the files with the plots and also a file called plotter.root containing the histograms with the distributions
normalized by integrated luminosity.
