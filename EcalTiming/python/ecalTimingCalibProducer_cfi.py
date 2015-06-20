import FWCore.ParameterSet.Config as cms

looper = cms.Looper("EcalTimingCalibProducer",
                    maxLoop = cms.uint32(2),
                    isSplash = cms.bool(False),
                    makeEventPlots = cms.bool(False),
                    recHitEBCollection = cms.InputTag("ecalRecHit","EcalRecHitsEB"),
                    recHitEECollection = cms.InputTag("ecalRecHit","EcalRecHitsEE"),
                    recHitFlags = cms.vint32([0]), # only recHits with these flags are accepted for calibration
                    #recHitMinimumN = cms.uint32(10),
                    recHitMinimumN = cms.uint32(2),
                    minRecHitEnergy = cms.double(1),
                    minRecHitEnergyStep = cms.double(0.5),
                    minEntries = cms.uint32(1),
                    globalOffset = cms.double(0.),
                    produceNewCalib = cms.bool(True),
                    outputDumpFile = cms.string('output.dat'),
                    noiseRMSThreshold = cms.double(0.5),
                    noiseTimeThreshold = cms.double(6.0),
                    )
