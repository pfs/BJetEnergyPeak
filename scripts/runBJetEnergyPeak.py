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
def runBJetEnergyPeak(inFileURL, outFileURL, wgtH):

    print '...analysing %s' % inFileURL

    #book some histograms
    histos={ 
        'nbtags':ROOT.TH1F('nbtags',';b-tag multiplicity; Events',5,0,5),
        'bjeten':ROOT.TH1F('bjeten',';Energy [GeV]; Jets',100,0,200)
        }
    for key in histos:
        histos[key].Sumw2()
        histos[key].SetDirectory(0)

    #open file and loop over events tree
    fIn=ROOT.TFile.Open(inFileURL)
    tree=fIn.Get('btagana/ttree')
    for i in xrange(0,tree.GetEntriesFast()):

        tree.GetEntry(i)

        #require trigger bits (first two for e-mu)
        failTrigger = False if (((tree.ttbar_trigWord >>0) &1) or ((tree.ttbar_trigWord >>1) &1)) else True
        if failTrigger : continue

        #require at least 2 leptons
        if tree.ttbar_nl<2: continue

        #require events in the e-mu channel
        if tree.ttbar_lid[0]*tree.ttbar_lid[1]!=11*13 : continue

        #require op. charge
        if tree.ttbar_lch[0]*tree.ttbar_lch[1]>0 : continue

        #require at least two jets
        nJets, nBtags = 0, 0
        taggedJetsP4=[]
        for ij in xrange(0,tree.nJet):

            #get the kinematics and select the jet
            jp4=ROOT.TLorentzVector()
            jp4.SetPtEtaPhiM(tree.Jet_pt[ij],tree.Jet_eta[ij],tree.Jet_phi[ij],tree.Jet_mass[ij])
            if jp4.Pt()<30 or ROOT.TMath.Abs(jp4.Eta())>2.4 : continue

            #count selected jet
            nJets +=1

            #save P4 for b-tagged jet
            if tree.Jet_CombIVF[ij]>0.605:
                nBtags+=1
                taggedJetsP4.append(jp4)
        
        if nJets<2 : continue

        #generator level weight
        evWgt=1.0
        if wgtH : evWgt=tree.ttbar_w[0]*wgtH.GetBinContent(1)

        #ready to fill the histograms
        histos['nbtags'].Fill(nBtags,evWgt)
        for jp4 in taggedJetsP4: 
            histos['bjeten'].Fill(jp4.E(),evWgt)
        
    #all done with this file
    fIn.Close()

    #save histograms to file
    fOut=ROOT.TFile.Open(outFileURL,'RECREATE')
    for key in histos: histos[key].Write()
    fOut.Close()


"""
Wrapper to be used when run in parallel
"""
def runBJetEnergyPeakPacked(args):
    
    try:
        return runBJetEnergyPeak(inFileURL=args[0],
                                 outFileURL=args[1],
                                 wgtH=args[2])
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
    parser.add_option('-j', '--json',        dest='json'  ,      help='json with list of files',      default=None,        type='string')
    parser.add_option('-i', '--inDir',       dest='inDir',       help='input directory with files',   default=None,        type='string')
    parser.add_option('-o', '--outDir',      dest='outDir',      help='output directory',             default='analysis',  type='string')
    parser.add_option('-n', '--njobs',       dest='njobs',       help='# jobs to run in parallel',    default=0,           type='int')
    (opt, args) = parser.parse_args()

    #read list of samples
    jsonFile = open(opt.json,'r')
    samplesList=json.load(jsonFile,encoding='utf-8').items()
    jsonFile.close()

    #read normalization from cache
    xsecWgts, pileupWgts = {}, {}
    cachefile = open('%s/src/UserCode/BJetEnergyPeak/data/xsecweights.pck' % os.environ['CMSSW_BASE'], 'r')
    xsecWgts   = pickle.load(cachefile)
    pileupWgts = pickle.load(cachefile)
    cachefile.close()        

    #mount locally EOS
    eos_cmd = '/afs/cern.ch/project/eos/installation/0.2.41/bin/eos.select'
    Popen([eos_cmd, ' -b fuse mount', 'eos'],stdout=PIPE).communicate()

    #prepare output
    if len(opt.outDir) :  os.system('mkdir -p %s' % opt.outDir)
        
    #create the analysis jobs
    taskList = []
    for tag,sample in samplesList:

        fileList=os.listdir( 'eos/cms/%s/%s' % (opt.inDir,tag) )
        wgtH = xsecWgts[tag]
        for ifile in xrange(0,len(fileList)):
            #inFileURL  = 'root://eoscms//eos/cms/%s/%s/%s' % (opt.inDir, tag, fileList[ifile])
            inFileURL  = 'eos/cms/%s/%s/%s' % (opt.inDir, tag, fileList[ifile])
            outFileURL = '%s/%s_%d.root' % (opt.outDir,tag,ifile)
            taskList.append( (inFileURL,outFileURL,wgtH) )

    #run the analysis jobs
    if opt.njobs == 0:
        for inFileURL, outFileURL, wgtH in taskList:
            runBJetEnergyPeak(inFileURL=inFileURL, outFileURL=outFileURL, wgtH=wgtH)
    else:
        from multiprocessing import Pool
        pool = Pool(opt.njobs)
        pool.map(runBJetEnergyPeakPacked,taskList)

    #unmount locally EOS
    Popen([eos_cmd, ' -b fuse umount', 'eos'],stdout=PIPE).communicate()

    #merge the outputs
    for tag,_ in samplesList:
        os.system('hadd -f %s/%s.root %s/%s_*.root' % (opt.outDir,tag,opt.outDir,tag) )
        os.system('rm %s/%s_*.root' % (opt.outDir,tag) )
    print 'Analysis results are available in %s' % opt.outDir

    #all done here
    exit(0)



"""
for execution from another script
"""
if __name__ == "__main__":
    sys.exit(main())
