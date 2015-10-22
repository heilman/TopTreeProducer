# Starting with a skeleton process which gets imported with the following line
from PhysicsTools.PatAlgos.patTemplate_cfg import *

from PhysicsTools.PatAlgos.tools.coreTools import *

#from TopBrussels.TopTreeProducer.datasetToSource import *
## This is used to get the correct global tag below, and to find the files
## It is *reset* automatically by ProductionTasks, so you can use it after the ProductionTasksHook
#datasetInfo = ('cmgtools_group', '/VBF_HToTauTau_M-125_8TeV-powheg-pythia6/Summer12_DR53X-PU_S10_START53_V7A-v1/AODSIM/V5_B','.*root')
#datasetInfo = ('cmgtools_group', '/TTJets_MassiveBinDECAY_TuneZ2star_8TeV-madgraph-tauola/Summer12_DR53X-PU_S10_START53_V7A-v1/AODSIM/V5_B','.*root')
#process.source = datasetToSource(
#    *datasetInfo
#    )

#process.source.fileNames = process.source.fileNames[:20]
#import Run2012AData as InputFiles1
#import Run2012BData as InputFiles2
#process.source = InputFiles1.source
#process.source.fileNames.extend(InputFiles2.readFiles)

process.source.fileNames = [
   'file://DYTOLL.root'
]

# load the PAT config
process.load("PhysicsTools.PatAlgos.patSequences_cff")


runOnMC = True 
runOnFastSim = True 

###############################
####### Global Setup ##########
###############################

process.load("FWCore.Framework.test.cmsExceptionsFatal_cff")
process.load("SimGeneral.HepPDTESSource.pythiapdt_cfi")
process.load("PhysicsTools.HepMCCandAlgos.genParticles_cfi")
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.destinations = ['cerr']
process.MessageLogger.statistics = []
process.MessageLogger.fwkJobReports = []
process.MessageLogger.categories=cms.untracked.vstring('FwkJob'
							,'FwkReport'
							,'FwkSummary'
			                               )

process.MessageLogger.cerr.INFO = cms.untracked.PSet(limit = cms.untracked.int32(0))
process.MessageLogger.cerr.FwkReport.reportEvery = cms.untracked.int32(100)
process.options = cms.untracked.PSet(
		                 wantSummary = cms.untracked.bool(True)
	         	 	)

### Set the global tag from the dataset name
if runOnFastSim is False:
  from TopBrussels.TopTreeProducer.Tools.getGlobalTag import getGlobalTagByDataset
  process.GlobalTag.globaltag = getGlobalTagByDataset( runOnMC, process.source.fileNames[0])
else:
  process.GlobalTag.globaltag = cms.string('START53_V27::All')

##-------------------- Import the Jet RECO modules ----------------------- ## this makes cmsRun crash
##
process.load('RecoJets.Configuration.RecoPFJets_cff')
##-------------------- Turn-on the FastJet density calculation -----------------------
process.kt6PFJets.doRhoFastjet = True

process.primaryVertexFilter = cms.EDFilter("GoodVertexFilter",
                                           vertexCollection = cms.InputTag('offlinePrimaryVertices'),
                                           minimumNDOF = cms.uint32(4) ,
                                           maxAbsZ = cms.double(24), 
                                           maxd0 = cms.double(2) 
                                           )

from PhysicsTools.SelectorUtils.pvSelector_cfi import pvSelector

process.goodOfflinePrimaryVertices = cms.EDFilter(
    "PrimaryVertexObjectFilter",
    filterParams = pvSelector.clone( maxZ = cms.double(24.0) ),
    src=cms.InputTag('offlinePrimaryVertices')
    )

###############################
#### Load MVA electron Id #####
###############################

### UserCode area is not supported anymore ### 
#process.load('EGamma.EGammaAnalysisTools.electronIdMVAProducer_cfi')
### Use CMSSW area 
process.load('EgammaAnalysis/ElectronTools.electronIdMVAProducer_cfi')
process.eidMVASequence = cms.Sequence( process.mvaTrigV0 + process.mvaNonTrigV0 )

###############################
####### PF2PAT Setup ##########
###############################

# Default PF2PAT with AK5 jets. Make sure to turn ON the L1fastjet stuff. 
from PhysicsTools.PatAlgos.tools.pfTools import *
postfix = "PF2PAT"
usePF2PAT(process,runPF2PAT=True, jetAlgo="AK5", runOnMC=runOnMC, postfix=postfix, pvCollection=cms.InputTag('goodOfflinePrimaryVertices'), typeIMetCorrections=True)


# TOP projection

###Change this to 999 to remove iso cut
#process.pfIsolatedMuonsPF2PAT.isolationCut = cms.double(0.2)
process.pfIsolatedMuonsPF2PAT.isolationCut = 999
getattr( process, 'pfMuonsFromVertex' + postfix ).d0Cut = cms.double(-1.0)
getattr( process, 'pfMuonsFromVertex' + postfix ).dzCut = cms.double(-1.0)
getattr( process, 'pfMuonsFromVertex' + postfix ).d0SigCut = cms.double(-1.0)
getattr( process, 'pfMuonsFromVertex' + postfix ).dzSigCut = cms.double(-1.0)

process.pfIsolatedMuonsPF2PAT.doDeltaBetaCorrection = True
process.pfSelectedMuonsPF2PAT.cut = cms.string('pt > 10. && abs(eta) < 2.5')
process.pfIsolatedMuonsPF2PAT.isolationValueMapsCharged = cms.VInputTag(cms.InputTag("muPFIsoValueCharged04PF2PAT"))
process.pfIsolatedMuonsPF2PAT.deltaBetaIsolationValueMap = cms.InputTag("muPFIsoValuePU04PF2PAT")
process.pfIsolatedMuonsPF2PAT.isolationValueMapsNeutral = cms.VInputTag(cms.InputTag("muPFIsoValueNeutral04PF2PAT"), cms.InputTag("muPFIsoValueGamma04PF2PAT"))
# leptons for top tree: no isolation requirement 
# "pfMuons" is cloned from "pfIsolatedMuons" but the isolation cut is removed in the main PFBRECO sequence
process.patMuonsPF2PAT.pfMuonSource = "pfMuonsPF2PAT"
process.patMuonsPF2PAT.embedCaloMETMuonCorrs = False
process.patMuonsPF2PAT.embedTcMETMuonCorrs= False

print "process.pfIsolatedMuonsPF2PAT.isolationCut -> "+str(process.pfIsolatedMuonsPF2PAT.isolationCut)
print "process.patMuonsPF2PAT.pfMuonSource -> "+str(process.patMuonsPF2PAT.pfMuonSource)


# to use GsfElectrons instead of PF electrons
# this will destory the feature of top projection which solves the ambiguity between leptons and jets because
# there will be overlap between non-PF electrons and jets even though top projection is ON!
#useGsfElectrons(process,postfix,"03") # to change isolation cone size to 0.3 as it is recommended by EGM POG, use "04" for cone size 0.4

#disable useRecoMuon and use new function
from TopBrussels.TopTreeProducer.Tools.tools import *
#useRecoMuon(process,postfix,"04")

#actually it appears that pfMuons (i.e. pfIsolatedMuons clone without isolation) can be  used. See comment above
muonSource = "pfMuonsPF2PAT"

#use new function for muons in TopTreeProducer.Tools.tools
useOtherMuonCollection(process,postfix,sourceMuons=muonSource,useParticleFlow=True, dR="04")


#Anyway, print some info to check
print "XXXX Some debug info"
print "process.pfIsolatedMuonsPF2PAT.isolationCut -> "+str(process.pfIsolatedMuonsPF2PAT.isolationCut)
print "process.patMuonsPF2PAT.pfMuonSource -> "+str(process.patMuonsPF2PAT.pfMuonSource)
print "process.pfMuonsPF2PAT.isolationCut -> "+str(process.pfMuonsPF2PAT.isolationCut)
print "process.muonmatchPF2PAT.src -> "+str(getattr(process,"muonMatch"+postfix).src)

#process.pfIsolatedElectronsPF2PAT.isolationCut = cms.double(0.2)
#process.pfIsolatedElectronsPF2PAT.doDeltaBetaCorrection = False
#process.pfSelectedElectronsPF2PAT.cut = cms.string('pt > 10. && abs(eta) < 2.5 && gsfTrackRef.isNonnull && gsfTrackRef.trackerExpectedHitsInner.numberOfLostHits<2')
#process.pfIsolatedElectronsPF2PAT.isolationValueMapsCharged = cms.VInputTag(cms.InputTag("elPFIsoValueCharged03PFIdPF2PAT"))
#process.pfIsolatedElectronsPF2PAT.deltaBetaIsolationValueMap = cms.InputTag("elPFIsoValuePU03PFIdPF2PAT")
#process.pfIsolatedElectronsPF2PAT.isolationValueMapsNeutral = cms.VInputTag(cms.InputTag("elPFIsoValueNeutral03PFIdPF2PAT"), cms.InputTag("elPFIsoValueGamma03PFIdPF2PAT"))

#process.patElectronsPF2PAT.isolationValues = cms.PSet(
#    pfChargedHadrons = cms.InputTag("elPFIsoValueCharged03PFIdPF2PAT"),
#    pfChargedAll = cms.InputTag("elPFIsoValueChargedAll03PFIdPF2PAT"),
#    pfPUChargedHadrons = cms.InputTag("elPFIsoValuePU03PFIdPF2PAT"),
#    pfNeutralHadrons = cms.InputTag("elPFIsoValueNeutral03PFIdPF2PAT"),
#    pfPhotons = cms.InputTag("elPFIsoValueGamma03PFIdPF2PAT")
#    )


#Commented out use of gsf electrons. Adapting pf electrons:

#from CommonTools.ParticleFlow.pfElectrons_cff  import *

#process.patElectronsPF2PAT.pfElectronSource=cms.InputTag("pfElectrons" + postfix)

#or could access isolation and disable it
getattr(process,"pfIsolatedElectrons"+postfix).isolationCut = 999

print "XXXX electron source:"
print process.patElectronsPF2PAT.pfElectronSource
print "YYYY isolation cut:"
print process.pfElectronsPF2PAT.isolationCut

getattr( process, 'pfElectronsFromVertex' + postfix ).d0Cut = cms.double(-1.0)
getattr( process, 'pfElectronsFromVertex' + postfix ).dzCut = cms.double(-1.0)
getattr( process, 'pfElectronsFromVertex' + postfix ).d0SigCut = cms.double(-1.0)
getattr( process, 'pfElectronsFromVertex' + postfix ).dzSigCut = cms.double(-1.0)

process.patElectronsPF2PAT.electronIDSources.mvaTrigV0    = cms.InputTag("mvaTrigV0")
process.patElectronsPF2PAT.electronIDSources.mvaNonTrigV0 = cms.InputTag("mvaNonTrigV0") 
process.patPF2PATSequencePF2PAT.replace( process.patElectronsPF2PAT, process.eidMVASequence * process.patElectronsPF2PAT )




process.patJetCorrFactorsPF2PAT.payload = 'AK5PFchs'
process.patJetCorrFactorsPF2PAT.levels = cms.vstring(['L1FastJet', 'L2Relative', 'L3Absolute'])
process.pfPileUpPF2PAT.checkClosestZVertex = False

# top projections in PF2PAT:
getattr(process,"pfNoPileUp"+postfix).enable = True
getattr(process,"pfNoMuon"+postfix).enable = True
getattr(process,"pfNoElectron"+postfix).enable = True
getattr(process,"pfNoTau"+postfix).enable = False
getattr(process,"pfNoJet"+postfix).enable = False

#####################################################################################################
#### Clone the PF2PAT sequence for data-driven QCD estimate, and for Stijn's JetMET service work ####
#####################################################################################################

from PhysicsTools.PatAlgos.tools.helpers import cloneProcessingSnippet
postfixNoLeptonCleaning = 'NoLeptonCleaning'

# just cloning the first sequence, and enabling lepton cleaning 
cloneProcessingSnippet(process, getattr(process, 'patPF2PATSequencePF2PAT'), postfixNoLeptonCleaning)

getattr(process,"pfNoMuonPF2PATNoLeptonCleaning").enable = False
getattr(process,"pfNoElectronPF2PATNoLeptonCleaning").enable = False 
getattr(process,"pfIsolatedMuonsPF2PATNoLeptonCleaning").combinedIsolationCut = cms.double(999999)
getattr(process,"pfIsolatedMuonsPF2PATNoLeptonCleaning").isolationCut = cms.double(999999)
getattr(process,"pfIsolatedElectronsPF2PATNoLeptonCleaning").combinedIsolationCut = cms.double(999999)
getattr(process,"pfIsolatedElectronsPF2PATNoLeptonCleaning").isolationCut = cms.double(999999)

###############################
###### Bare KT 0.6 jets #######
###############################

#from RecoJets.JetProducers.kt4PFJets_cfi import kt4PFJets
# For electron (effective area) isolation
#process.kt6PFJetsForIsolation = kt4PFJets.clone( rParam = 0.6, doRhoFastjet = True )
#process.kt6PFJetsForIsolation.Rho_EtaMax = cms.double(2.5)

###############################
### Add AK5GenJetsNoMuNoNu ####
###############################

from RecoJets.Configuration.GenJetParticles_cff import genParticlesForJetsNoMuNoNu
from RecoJets.JetProducers.ak5GenJets_cfi import ak5GenJets

process.ak5GenJetsNoMuNoNu = ak5GenJets.clone( src = cms.InputTag('genParticlesForJetsNoMuNoNu') )
process.ak5GenJetsSeq = cms.Sequence(genParticlesForJetsNoMuNoNu*process.ak5GenJetsNoMuNoNu)

###############################
#### Selections Setup #########
###############################




# AK5 Jets
#   PF
process.selectedPatJetsPF2PAT.cut = cms.string("pt > 10")
process.selectedPatJetsPF2PATNoLeptonCleaning.cut = cms.string("pt > 10")
#process.selectedPatJetsPF2PATNoPFnoPU.cut = cms.string("pt > 10")
#process.selectedPatJetsAK5Calo.cut = cms.string("pt > 15")

# Flavor history stuff
process.load("PhysicsTools.HepMCCandAlgos.flavorHistoryPaths_cfi")
process.flavorHistoryFilter.pathToSelect = cms.int32(-1)
process.cFlavorHistoryProducer.matchedSrc = cms.InputTag("ak5GenJetsNoNu")
process.bFlavorHistoryProducer.matchedSrc = cms.InputTag("ak5GenJetsNoNu")

process.prePathCounter = cms.EDProducer("EventCountProducer")
process.postPathCounter = cms.EDProducer("EventCountProducer")

process.load('TopBrussels.TopTreeProducer.eventCleaning.eventCleaning_cff')

### photon sequence ### 
from CommonTools.ParticleFlow.Tools.pfIsolation import setupPFElectronIso, setupPFMuonIso, setupPFPhotonIso
process.phoIsoSequence = setupPFPhotonIso(process, 'selectedPatPhotons')

if runOnMC is False:
  process.makePatPhotons.remove(process.photonMatch)

process.photonSequence = cms.Sequence (
    process.makePatPhotons+
    process.selectedPatPhotons+
    process.phoIsoSequence
)

# let it run
process.patseq = cms.Sequence(
    process.prePathCounter*
#    process.kt6PFJetsForIsolation*
    process.goodOfflinePrimaryVertices*
#    process.ak5GenJetsSeq*
    process.primaryVertexFilter * #removes events with no good pv (but if cuts to determine good pv change...)
    process.eventCleaning*
    getattr(process,"patPF2PATSequence"+postfix)* # main PF2PAT
#    getattr(process,"patPF2PATSequence"+postfix+postfixNoLeptonCleaning)* # PF2PAT FOR DATA_DRIVEN QCD
#    getattr(process,"patPF2PATSequence"+postfix+postfixNoPFnoPU)* # PF2PAT FOR JETS WITHOUT PFnoPU
#    process.patDefaultSequence*
    process.flavorHistorySeq*
    process.photonSequence
    )

if runOnMC is False:
    
    process.patseq.remove( process.flavorHistorySeq )
    process.patJetCorrFactorsPF2PAT.levels = cms.vstring(['L1FastJet', 'L2Relative', 'L3Absolute','L2L3Residual']) 

##if runOnFastSim is True:
##normally this option is only for fastsim simulation (and not full sim) but 
## this isn't implemented yet 
if runOnMC is True:
    process.eventCleaning.remove(process.HBHENoiseFilter)

#################
#### ENDPATH ####
#################

nEventsInit = cms.EDProducer("EventCountProducer")

process.p = cms.Path(
    process.patseq+
    process.postPathCounter
    )

process.out.SelectEvents.SelectEvents = cms.vstring('p')

# rename output file
process.out.fileName = "PAT.root"

# process all the events
process.maxEvents.input = 100 #changed

process.options.wantSummary = True 
process.out.dropMetaData = cms.untracked.string("DROPPED")

process.source.inputCommands = cms.untracked.vstring("keep *", "drop *_MEtoEDMConverter_*_*")

from TopBrussels.TopTreeProducer.patEventContentTopTree_cff import patEventContentTopTree
process.out.outputCommands = patEventContentTopTree  
#process.out.outputCommands.append('keep *_selectedPatPhotons*_*_*')
#process.out.outputCommands.append('keep *_pfSelectedElectronsLoose*_*_*')
