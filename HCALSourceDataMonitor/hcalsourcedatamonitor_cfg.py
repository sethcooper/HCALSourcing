import FWCore.ParameterSet.Config as cms

process = cms.Process("HCALFWAnalyzer")

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

#----------------------------------- replacing conditions with txt ones
process.es_ascii = cms.ESSource("HcalTextCalibrations",
 input = cms.VPSet(
   cms.PSet(
     object = cms.string('ElectronicsMap'),
     #file = cms.FileInPath('HCALSourcing/HCALFWAnalyzer/emap_H2_25oct2012.txt')
     #file = cms.FileInPath('HCALSourcing/HCALFWAnalyzer/emap_HCAL_H2_BI_jan2013_1.txt')
     file = cms.FileInPath('HCALSourcing/HCALFWAnalyzer/emap_HCAL_H2_BI_modSIC_apr2013.txt')
   )
 )
)
process.es_prefer = cms.ESPrefer('HcalTextCalibrations','es_ascii')

#process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(100) )

process.source = cms.Source("HcalTBSource",
    # replace 'myfile.root' with the source file you want to use
    fileNames = cms.untracked.vstring(
        # run with source driver moving: http://cmsonline.cern.ch/cms-elog/804153
        'file:/afs/cern.ch/user/s/scooper/work/private/cmssw/525/HCALFWAnalysis/src/HCALSourcing/HCALFWAnalyzer/HTB_006096.root'
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
process.hcalhistos.FEDs = cms.untracked.vint32(700)

#process.TFileService = cms.Service("TFileService", 
#    fileName = cms.string("hcalFWAnalyzer.6096.SDMovingH2.Apr9.newEmap.root"),
#)

process.hcalSourceDataMon = cms.EDAnalyzer('HCALSourceDataMonitor',
    RootFileName = cms.untracked.string('hcalSourceDataMon.root'),
    HtmlFileName = cms.untracked.string('test.html'),
    NewRowEvery = cms.untracked.int32(3),
    ThumbnailSize = cms.untracked.int32(350)

)

process.load("FWCore.Modules.printContent_cfi")

process.p = cms.Path(process.tbunpack
                     *process.hcalhistos
                     #*process.printContent
                     *process.hcalSourceDataMon
                    )
