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
      XXX_FILENAMES_XXX
    )
)

process.tbunpack = cms.EDProducer("HcalTBObjectUnpacker",
    HcalSlowDataFED = cms.untracked.int32(-1),
    HcalSourcePositionFED = cms.untracked.int32(12),
    HcalTriggerFED = cms.untracked.int32(1),
    fedRawDataCollectionTag = cms.InputTag('rawDataCollector')
)

# histo unpacker
process.load("EventFilter.HcalRawToDigi.HcalHistogramRawToDigi_cfi")
process.hcalhistos.HcalFirstFED = cms.untracked.int32(700)
process.hcalhistos.FEDs = cms.untracked.vint32(718,719,720,721,722,723)

process.hcalSourceDataMon = cms.EDAnalyzer('HCALSourceDataMonitor',
    RootFileName = cms.untracked.string('XXX_TFILENAME_XXX'),
    PrintRawHistograms = cms.untracked.bool(False),
    SelectDigiBasedOnTubeName = cms.untracked.bool(True)
)

process.p = cms.Path(process.tbunpack
                     *process.hcalhistos
                     *process.hcalSourceDataMon
                    )

