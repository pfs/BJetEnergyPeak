# BJetEnergyPeak

To install the code do
```
cmsrel CMSSW_7_4_14
cd CMSSW_7_4_14/src
cmsenv
git clone git@github.com:pfs/BJetEnergyPeak.git UserCode/BJetEnergyPeak
scram b
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
