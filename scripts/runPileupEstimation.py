import optparse
import os,sys
import pickle
import commands
import ROOT
from SimGeneral.MixingModule.mix_2015_25ns_Startup_PoissonOOTPU_cfi import *

MBXSEC=69000.
PUSCENARIOS={'nom':MBXSEC,'up':MBXSEC*1.1,'down':MBXSEC*0.9}

"""
steer the script
"""
def main():

    #configuration
    usage = 'usage: %prog [options]'
    parser = optparse.OptionParser(usage)
    parser.add_option('--json',      
                      dest='inJson'  ,      
                      help='json file with processed runs',      
                      default='/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions15/13TeV/Cert_246908-260627_13TeV_PromptReco_Collisions15_25ns_JSON_Silver.txt',
                      type='string')
    parser.add_option('--puJson',    
                      dest='puJson'  ,
                      help='pileup json file',      
                      default='/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions15/13TeV/PileUp/pileup_latest.txt',    
                      type='string')
    (opt, args) = parser.parse_args()
    
    #simulated pileup
    NPUBINS=len(mix.input.nbPileupEvents.probValue)
    simPuH=ROOT.TH1F('simPuH','',NPUBINS,float(0),float(NPUBINS))
    for xbin in xrange(0,NPUBINS):
        probVal=mix.input.nbPileupEvents.probValue[xbin]
        simPuH.SetBinContent(xbin,probVal)
    simPuH.Scale(1./simPuH.Integral())

    #compute pileup in data assuming different xsec
    puWgts,puDists={},{}
    for scenario in PUSCENARIOS:
        print scenario, 'xsec=',PUSCENARIOS[scenario]
        cmd='pileupCalc.py -i %s --inputLumiJSON %s --calcMode true --minBiasXsec %f --maxPileupBin %d --numPileupBins %s Pileup.root'%(opt.inJson,opt.puJson,PUSCENARIOS[scenario],NPUBINS,NPUBINS)
        commands.getstatusoutput(cmd)

        fIn=ROOT.TFile.Open('Pileup.root')
        pileupH=fIn.Get('pileup')
        pileupH.Scale(1./pileupH.Integral())
        puDists[scenario]=ROOT.TGraph(pileupH)
        puDists[scenario].SetName('pu_'+scenario)

        pileupH.Divide(simPuH)
        puWgts[scenario]=ROOT.TGraph(pileupH)
        puWgts[scenario].SetName('puwgts_'+scenario)
        fIn.Close()
    commands.getstatusoutput('rm Pileup.root')

    #dump to pickle file                                                                                                                                                                                          
    cache='%s/src/UserCode/BJetEnergyPeak/data/puweights.root' % os.environ['CMSSW_BASE']
    cachefile=ROOT.TFile.Open(cache,'RECREATE')
    for scenario in puWgts:
        puWgts[scenario].Write()
        puDists[scenario].Write()
    cachefile.Close()
    print 'Produced normalization cache for pileup weights @ %s'%cache

"""
for execution from another script
"""
if __name__ == "__main__":
    sys.exit(main())
