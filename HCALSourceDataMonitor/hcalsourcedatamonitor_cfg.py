import FWCore.ParameterSet.Config as cms

process = cms.Process("HCALSourceDataMonitor")

#process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger = cms.Service("MessageLogger",
     #suppressInfo = cms.untracked.vstring(),
     cout = cms.untracked.PSet(
               threshold = cms.untracked.string('WARNING')
           ),
     categories = cms.untracked.vstring('*'),
     destinations = cms.untracked.vstring('cout')
)


##-- GT conditions for all
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
from Configuration.AlCa.autoCond import autoCond
process.GlobalTag.globaltag = autoCond['com10'] ## == GR_R_53_V16::All in 5_3_7
###-- Customized particular conditions
from CondCore.DBCommon.CondDBSetup_cfi import *


process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )
#process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(100) )

process.source = cms.Source("HcalTBSource",
    # replace 'myfile.root' with the source file you want to use
    fileNames = cms.untracked.vstring(
        'file:/afs/cern.ch/user/s/scooper/work/private/cmssw/HCALSourcingWork/src/HCALSourcing/HCALSourceDataMonitor/USC_215595.root'
    )
)

# TB data unpacker
#process.load('RecoTBCalo.HcalTBObjectUnpacker.HcalTBObjectUnpacker_SourceCal_cfi')
#process.tbunpack = cms.EDProducer("HcalTBObjectUnpacker",
#    HcalSlowDataFED = cms.untracked.int32(12),
#    HcalSourcePositionFED = cms.untracked.int32(-1),
#    HcalTriggerFED = cms.untracked.int32(1)
#)

process.tbunpack = cms.EDProducer("HcalTBObjectUnpacker",
    HcalSlowDataFED = cms.untracked.int32(-1),
    HcalSourcePositionFED = cms.untracked.int32(12),
    HcalTriggerFED = cms.untracked.int32(1),
    fedRawDataCollectionTag = cms.InputTag('rawDataCollector')
)

# histo unpacker
process.load("EventFilter.HcalRawToDigi.HcalHistogramRawToDigi_cfi")
process.hcalhistos.HcalFirstFED = cms.untracked.int32(700)
#process.hcalhistos.FEDs = cms.untracked.vint32(700)
#process.hcalhistos.FEDs = cms.untracked.vint32(718,722)
process.hcalhistos.FEDs = cms.untracked.vint32(718,719,720,721,722,723)

#process.TFileService = cms.Service("TFileService", 
#    fileName = cms.string("hcalFWAnalyzer.6096.SDMovingH2.Apr9.newEmap.root"),
#)

process.hcalSourceDataMon = cms.EDAnalyzer('HCALSourceDataMonitor',
    RootFileName = cms.untracked.string('hcalSourceDataMon.test.215595.root'),
    PrintRawHistograms = cms.untracked.bool(False),
    SelectDigiBasedOnTubeName = cms.untracked.bool(True)
)

process.p = cms.Path(process.tbunpack
                     *process.hcalhistos
                     *process.hcalSourceDataMon
                    )

