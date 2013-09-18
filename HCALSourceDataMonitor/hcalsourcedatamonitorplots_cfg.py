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
    RootInputFileName = cms.untracked.string('hcalSourceDataMon.test.6703.500kevts.sep17.root'),
    RootOutputFileName = cms.untracked.string('hcalSourceDataMonPlots.test.6703.500kevts.sep17.root'),
    HtmlFileName = cms.untracked.string('test.html'),
    NewRowEvery = cms.untracked.int32(3),
    ThumbnailSize = cms.untracked.int32(350),
    OutputRawHistograms = cms.untracked.bool(False),
    SelectDigiBasedOnTubeName = cms.untracked.bool(False),

)

process.p = cms.Path(
                     process.hcalSourceDataMonPlots
                    )


