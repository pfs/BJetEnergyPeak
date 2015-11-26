#!/usr/bin/env python

import os
import ROOT
from array import array
import sys
import optparse
import pickle
from subprocess import Popen, PIPE

CSVWPS={'loose':0.605, 'medium':0.890, 'tight':0.970}

"""
steer the script
"""
def main():
    
    #configuration
    usage = 'usage: %prog [options]'
    parser = optparse.OptionParser(usage)
    parser.add_option('-i', 
                      '--inDir', 
                      dest='inDir',
                      help='input directory',                   
                      default='/store/group/phys_btag/performance/TTbar/2015_25ns/8622ee3/MC13TeV_TTJets',
                      type='string')
    (opt, args) = parser.parse_args()

    #mount locally EOS
    eos_cmd = '/afs/cern.ch/project/eos/installation/0.2.41/bin/eos.select'
    Popen([eos_cmd, ' -b fuse mount', 'eos'],stdout=PIPE).communicate()

    #create the chain (no need to use all the available files)
    chain=ROOT.TChain('btagana/ttree')
    allFiles=os.listdir('eos/cms/%s' % opt.inDir)
    for i in xrange(0,5): chain.Add('eos/cms/%s/%s' % (opt.inDir,allFiles[i]))

    effgrs={}
    ptBins = [0,20,25,30,35,40,45,50,60,70,80,90,100,120,140,160,180,200,250,300,400,500,600,800,1000]
    preTagH=ROOT.TH1F('preTagH',';p_{T} [GeV/c];',len(ptBins)-1,array('d',ptBins))
    preTagH.Sumw2()
    tagH=preTagH.Clone('tagH')
    for flav,cond in [('b',   'abs(Jet_flavour)==5'),
                      ('c',   'abs(Jet_flavour)==4'),
                      ('udsg','abs(Jet_flavour)!=5 && abs(Jet_flavour)!=4')]:

        #pre-tagged jet pT 
        preTagH.Reset('ICE')
        chain.Draw("Jet_pt >> preTagH",'ttbar_w[0]*(%s)'%cond,'goff')

        #compute the efficiency for the different working points
        effgrs[flav]={}
        for wp in CSVWPS:

            print '...starting',flav,wp

            #tagged jet pT
            csvWP='Jet_CombIVF>%f' % CSVWPS[wp]
            tagH.Reset('ICE')
            chain.Draw('Jet_pt >> tagH',   'ttbar_w[0]*(%s && %s)'%(cond,csvWP),'goff')
            
            #efficiency            
            effgrs[flav][wp]=ROOT.TGraphAsymmErrors()
            effgrs[flav][wp].SetName('%s_eff%s'%(flav,wp))
            effgrs[flav][wp].Divide(tagH,preTagH,'norm')

    #unmount locally EOS                                                                                                                                                                                          
    Popen([eos_cmd, ' -b fuse umount', 'eos'],stdout=PIPE).communicate()

    #save all in a cache
    cache='%s/src/UserCode/BJetEnergyPeak/data/btagefficiencies.pck' % os.environ['CMSSW_BASE']
    cachefile = open(cache,'w')
    pickle.dump(effgrs, cachefile,pickle.HIGHEST_PROTOCOL)
    cachefile.close()
    print 'Expected tagger efficiencies have been saved to %s'%cache


    #all done here
    exit(0)


"""
for execution from another script
"""
if __name__ == "__main__":
    sys.exit(main())

