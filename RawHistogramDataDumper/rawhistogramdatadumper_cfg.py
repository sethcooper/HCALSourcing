import FWCore.ParameterSet.Config as cms

process = cms.Process("Histo")

process.load("FWCore.MessageService.MessageLogger_cfi")

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(25) )

process.source = cms.Source("HcalTBSource",
    fileNames = cms.untracked.vstring(
        'file:/afs/cern.ch/user/s/scooper/work/private/cmssw/525/HCALFWAnalysis/src/HCALSourcing/HCALSourceDataMonitor/HTB_006675.root'
    )
)

process.Histo = cms.EDAnalyzer('RawHistogramDataDumper'
)


process.p = cms.Path(process.Histo)
