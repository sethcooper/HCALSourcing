import FWCore.ParameterSet.Config as cms

process = cms.Process("HCALSourceDataMonitorPlots")

#process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger = cms.Service("MessageLogger",
     #suppressInfo = cms.untracked.vstring(),
     cout = cms.untracked.PSet(
               threshold = cms.untracked.string('WARNING')
           ),
     categories = cms.untracked.vstring('*'),
     destinations = cms.untracked.vstring('cout')
)

process.source = cms.Source("EmptySource")
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(1) )

process.hcalSourceDataMonPlots = cms.EDAnalyzer('HCALSourceDataMonitorPlots',
    RootInputFileName = cms.untracked.string('hcalSourceDataMon.test1000evts.214756.oct9.root'),
    RootOutputFileName = cms.untracked.string('hcalSourceDataMonPlots.test1000evts.214756.oct10.root'),
    NewRowEvery = cms.untracked.int32(4),
    ThumbnailSize = cms.untracked.int32(350),
    OutputRawHistograms = cms.untracked.bool(True),
    SelectDigiBasedOnTubeName = cms.untracked.bool(False), # unimplemented for now
    MaxEvents = cms.untracked.int32(100)
)

process.p = cms.Path(
                     process.hcalSourceDataMonPlots
                    )


