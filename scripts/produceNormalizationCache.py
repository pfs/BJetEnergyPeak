#!/usr/bin/env python

import optparse
import os,sys
import pickle
import ROOT
from subprocess import Popen, PIPE

"""
steer the script
"""
def main():

    #configuration
    usage = 'usage: %prog [options]'
    parser = optparse.OptionParser(usage)
    parser.add_option('-i', '--inDir',       dest='inDir',       help='input directory with files',   default='/store/group/phys_btag/performance/TTbar/2015_25ns/8622ee3',        type='string')
    (opt, args) = parser.parse_args()

    #mount locally EOS
    eos_cmd = '/afs/cern.ch/project/eos/installation/0.2.41/bin/eos.select'
    Popen([eos_cmd, ' -b fuse mount', 'eos'],stdout=PIPE).communicate()

    #loop over samples available
    genweights={}
    for sample in os.listdir('eos/cms/%s' % opt.inDir):

        #sum weight generator level weights
        wgtCounter=None
        for f in os.listdir('eos/cms/%s/%s' % (opt.inDir,sample ) ):
            fIn=ROOT.TFile.Open('eos/cms/%s/%s/%s' % (opt.inDir,sample,f ) )
            if wgtCounter is None:
                wgtCounter=fIn.Get('ttbarselectionproducer/wgtcounter').Clone('genwgts')
                wgtCounter.SetDirectory(0)
                wgtCounter.Reset('ICE')
            wgtCounter.Add(fIn.Get('ttbarselectionproducer/wgtcounter'))
            fIn.Close()

        if wgtCounter is None: continue

        #invert to set normalization
        print sample,' initial sum of weights=',wgtCounter.GetBinContent(1)
        for xbin in xrange(1,wgtCounter.GetNbinsX()+1):
            val=wgtCounter.GetBinContent(xbin)
            if val==0: continue
            wgtCounter.SetBinContent(xbin,1./val)
            wgtCounter.SetBinError(xbin,0.)
       
        genweights[sample]=wgtCounter

    #unmount locally EOS
    Popen([eos_cmd, ' -b fuse umount', 'eos'],stdout=PIPE).communicate()

    #dump to pickle file
    cache='%s/src/UserCode/BJetEnergyPeak/data/genweights.pck' % os.environ['CMSSW_BASE']
    cachefile=open(cache,'w')
    pickle.dump(genweights, cachefile, pickle.HIGHEST_PROTOCOL)
    cachefile.close()
    print 'Produced normalization cache @ %s'%cache

    #all done here
    exit(0)



"""
for execution from another script
"""
if __name__ == "__main__":
    sys.exit(main())
