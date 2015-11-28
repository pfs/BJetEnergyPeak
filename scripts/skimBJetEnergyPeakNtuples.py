#!/usr/bin/env python

import optparse
import os,sys
import json
import pickle
import ROOT
from subprocess import Popen, PIPE

"""
Perform the analysis on a single file
"""
def skimBJetEnergyPeakNtuples(inFile,outFile,wgtH,isData):

    skim=ROOT.BJetEnergyPeakSkim();
    skim.processFile(inFile,outFile,wgtH,isData)
    return True


"""
Wrapper to be used when run in parallel
"""
def skimBJetEnergyPeakNtuplesPacked(args):    
    try:
        return skimBJetEnergyPeakNtuples(args[0],args[1],args[2],args[3])
    except :
        print 50*'<'
        print "  Problem  (%s) with %s continuing without"%(sys.exc_info()[1],args[0])
        print 50*'<'
        return False


"""
steer the script
"""
def main():

    #configuration
    usage = 'usage: %prog [options]'
    parser = optparse.OptionParser(usage)
    parser.add_option('-i', '--inDir',       dest='inDir',       help='input directory with files',   default=None,        type='string')
    parser.add_option('-o', '--outDir',      dest='outDir',      help='output directory',             default='analysis',  type='string')
    parser.add_option('-n', '--njobs',      dest='njobs',        help='number of parallel jobs',      default=0,           type=int)
    (opt, args) = parser.parse_args()

    ROOT.AutoLibraryLoader.enable()
    ROOT.gROOT.LoadMacro('src/BJetEnergyPeakSkim.cc+')

    #prepare output
    os.system('mkdir -p %s/Chunks'%opt.outDir)

    #read normalization from cache   
    cachefile = open('%s/src/UserCode/BJetEnergyPeak/data/genweights.pck' % os.environ['CMSSW_BASE'], 'r')
    genweights   = pickle.load(cachefile)
    cachefile.close()        

    #mount locally EOS
    eos_cmd = '/afs/cern.ch/project/eos/installation/0.2.41/bin/eos.select'
    Popen([eos_cmd, ' -b fuse mount', 'eos'],stdout=PIPE).communicate()

    
    #create the analysis jobs
    taskList = []
    samplesList={}
    for sample in os.listdir('eos/cms/%s' % opt.inDir):

        samplesList[sample]=''

        isData=True if 'Data13TeV' in sample else False
        wgtH=genweights[sample] if (sample in genweights and not isData) else None

        sampleFiles=os.listdir('eos/cms/%s/%s' % (opt.inDir,sample ) )
        for ifile in xrange(0,len(sampleFiles)):
            
            inFile  = 'root://eoscms//eos/cms/%s/%s/%s' % (opt.inDir, sample, sampleFiles[ifile])
            outFile = '%s/Chunks/%s_%d.root' % (opt.outDir,sample,ifile)
            samplesList[sample]+= outFile + ' '
            taskList.append( (inFile,outFile,wgtH,isData) )

    #run the analysis jobs
    if opt.njobs == 1:
        for inFile,outFile,wgtH,isData in taskList:
            skimBJetEnergyPeakNtuples(inFile,outFile,wgtH,isData)
    elif opt.njobs>1:
        from multiprocessing import Pool
        pool = Pool(opt.njobs)
        pool.map(skimBJetEnergyPeakNtuplesPacked,taskList)

    #unmount locally EOS
    Popen([eos_cmd, ' -b fuse umount', 'eos'],stdout=PIPE).communicate()

    #merge the outputs
    for sample in samplesList:
        os.system('hadd -f %s/%s.root %s' % (opt.outDir,sample,samplesList[sample]) )
    print 'Skimmed ntuples (chunks) are available in %s(/Chunks)' % opt.outDir

    #all done here
    exit(0)



"""
for execution from another script
"""
if __name__ == "__main__":
    sys.exit(main())
