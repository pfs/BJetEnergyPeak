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

* Determine the pileup weights: the output is a ROOT file: data/pileupWgts.root. Run the following script
```
python scripts/runPileupEstimation.py 
```
* Determine the normalization weights for all the samples available. The output is a pickle file: data/genweights.pck. Run the following script
```
python scripts/produceNormalizationCache.py -i /store/group/phys_btag/performance/TTbar/2015_25ns/8622ee3
```
* Project the expected b-tagging efficiency for the TTbar sample
```
python scripts/saveExpectedBtagEff.py -i /store/group/phys_btag/performance/TTbar/2015_25ns/8622ee3
```

In the following I assume you run the code inside the UserCode/BJetEnergyPeak directory.



To run the event selection and basic filling of histograms do
```
runBJetEnergyPeak.py -i /store/group/phys_btag/performance/TTbar/2015_25ns/8622ee3 -j data/samples_Run2015_25ns.json -o analysis -n 8
```

The results are stored in ROOT files and can be plotted together and compared to data using
```
plotter.py -i analysis/ -j data/samples_Run2015_25ns.json  -l 1615 
```
