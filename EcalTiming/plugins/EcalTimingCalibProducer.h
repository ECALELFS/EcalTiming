#ifndef ecaltimingcalibproducer_h
#define ecaltimingcalibproducer_h
/** \class EcalTimingCalibProducer EcalTimingCalibProducer.h EcalTiming/EcalTiming/plugins/EcalTimingCalibProducer.h
    \brief Plugin that derives the calibration constants

 This plugin runs over the events, selects the recHits according to the criteria defined in addRecHit

 \todo Exit condition based on convergence

*/


/**
   Module description:
 - digi to calibrated recHit reconstruction
 - selection of events with reasonable activity: minimum number of recHits
 - select recHits based on:
   - recoFlag -> only good recHits (exclude OOT pileup contribution with MultiFit)
   - minimum energy, ring based threshold
 - save all the time events (recHits) passing the selection
 - discard those not within 2 sigma of the stdDev distribution for the single channel (then excluding OOT spurious events)
 - exclude events with large time error (> 3ns) or null time error (==0 ns)
 - verify that the distribution is symmetric
 - check stability vs energy of the single channel within uncertainty
 - save full dump of time events in TTree for further checks if:
   - distribution not symmetric
   - time calibration not stable vs energy
 - calculate global time for EB, EE


 */
//#define DEBUG
#define RAWIDCRY 838904321

/// The entire set of events are saved for one channel in EB and one channel in EE in order to debug
#define EBCRYex 838861346
#define EECRYex 872422180

/// For one ring in EB, one in EE+ and one in EE- the entire set of events are saved to debug
#define EBRING 1
#define EEmRING 20
#define EEpRING 20

#define SPEEDOFLIGHT 30.0 // (cm/ns)

// system include files
#include <memory>
#include <iostream>
#include <fstream>

// Make Histograms the way!!
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDConsumerBase.h"

#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
//#include "FWCore/Framework/interface/Handle.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/LooperFactory.h"
#include "FWCore/Framework/interface/ESProducerLooper.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/ESProducts.h"
#include "FWCore/Framework/interface/Event.h"
// input collections
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
//RingTools
#include "Calibration/Tools/interface/EcalRingCalibrationTools.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloSubdetectorGeometry.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"

// record to be produced:
#include "CondFormats/DataRecord/interface/EcalTimeCalibConstantsRcd.h"
#include "CondFormats/DataRecord/interface/EcalTimeCalibErrorsRcd.h"
#include "CondFormats/DataRecord/interface/EcalTimeOffsetConstantRcd.h"
#include "CondTools/Ecal/interface/EcalTimeCalibConstantsXMLTranslator.h"
#include "CondTools/Ecal/interface/EcalTimeCalibErrorsXMLTranslator.h"
#include "CondTools/Ecal/interface/EcalTimeOffsetXMLTranslator.h"
#include "CondTools/Ecal/interface/EcalCondHeader.h"

#include "CondFormats/EcalObjects/interface/EcalTimeCalibConstants.h"
#include "CondFormats/EcalObjects/interface/EcalTimeCalibErrors.h"
#include "CondFormats/EcalObjects/interface/EcalTimeOffsetConstant.h"

#include "EcalTiming/EcalTiming/interface/EcalTimingEvent.h"
#include "EcalTiming/EcalTiming/interface/EcalCrystalTimingCalibration.h"

#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/EcalDetId/interface/EEDetId.h"

#include "TProfile.h"
#include "TGraphErrors.h"
#include "TGraph.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TFile.h"
#include "TProfile2D.h"

#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <functional>
#include <set>
#include <assert.h>
#include <time.h>

#include <TMath.h>
#include <Math/VectorUtil.h>
//#include <boost/tokenizer.hpp>

#include "EcalTiming/EcalTiming/interface/EcalTimeCalibrationMapFwd.h"

class EcalTimingCalibProducer : public edm::ESProducerLooper
{

private:
	EcalTimeCalibrationMap _timeCalibMap; ///< calibration map: contains the time shift for each crystal
	EventTimeMap _eventTimeMap;           ///< container of recHits passing selection in the event (reset at each event)

	// For finding averages for specific eta ring
	EcalCrystalTimingCalibration timeEEP; ///< global time calibration of EE+
	EcalCrystalTimingCalibration timeEEM; ///< global time calibration of EE-
	EcalCrystalTimingCalibration timeEB; ///< global time calibration of EB
	EcalCrystalTimingCalibration timeEBRing; ///< global time calibration of one EB ring
	EcalCrystalTimingCalibration timeEEmRing; ///< global time calibration of one EE- ring
	EcalCrystalTimingCalibration timeEEpRing; ///< global time calibration of one EE+ ring
	EcalCrystalTimingCalibration timeEBCRYex; ///< global time calibration of one EB channel
	EcalCrystalTimingCalibration timeEECRYex; ///< global time calibration of one EE channel

	float nearEndcapTime;
	float farEndcapTime;

public:
	EcalTimingCalibProducer(const edm::ParameterSet&); // default constructor
	~EcalTimingCalibProducer();                        // default destructor


	virtual void beginOfJob(const edm::EventSetup&);
	virtual void startingNewLoop(unsigned int ) ;
	virtual Status duringLoop(const edm::Event&, const edm::EventSetup&) ;
	virtual Status endOfLoop(const edm::EventSetup&, unsigned int);
	virtual void endOfJob();
private:
	// ----------member data ---------------------------
	/** @name Input Parameters
	 * Parameters defined in the config file _cfi,py
	 */
	///@{

	unsigned int _maxLoop; ///< maximum number of loops for intercalibration
	bool _isSplash; ///< flag to activate for splash analysis
	bool _makeEventPlots; ///< flag for making plots for each event (meant for splashes)
	edm::InputTag _ecalRecHitsEBTAG; ///< input collection
	edm::InputTag _ecalRecHitsEETAG;///< input collection
	std::vector<int> _recHitFlags; ///< vector containing list of valid rec hit flags for calibration
	unsigned int _recHitMin; ///< require at least this many rec hits to count the event
	double       _minRecHitEnergy; ///< minimum energy for the recHit to be considered for timing studies
	double _minRecHitEnergyStep; ///< at each iteration the code increases the minimum energy required for the calibration
	unsigned int _minEntries; ///< require a minimum number of entries in a ring to do averages
	float        _globalOffset;    ///< time to subtract from every event
	bool _produceNewCalib; ///< true if you don't want to use the values in DB and what to extract new absolute calibrations, if false iteration does not work
	std::string _outputDumpFileName; ///< name of the output file for the calibration constants' dump
	float _noiseRMSThreshold;
	float _noiseTimeThreshold;
	float _maxSkewnessForDump;
/// @}

	void dumpCalibration(std::string filename);
	void dumpCorrections(std::string filename);

// plotting
///fill histograms with the measured shifts (that will become -corrections for the next step)
	void FillCalibrationCorrectionHists(EcalTimeCalibrationMap::const_iterator cal_itr);
	void initHists(TFileDirectory dir);
	void initEventHists(TFileDirectory dir);
	void initTree(TFileDirectory dir);

	// Create calibration container objects -> to be used in beginOfJob
	void createConstants(const edm::EventSetup& iSetup)
	{

		// time calib constants
		edm::ESHandle<EcalTimeCalibConstants> ecalTimeCalibConstantsHandle;
		iSetup.get<EcalTimeCalibConstantsRcd>().get( ecalTimeCalibConstantsHandle);
		_timeCalibConstants = *ecalTimeCalibConstantsHandle;
#ifdef DEBUG
		// why they are 0 when setWhatProduced is uncommented? -> I've not found a way to read the default DB at iter 0
		for(auto t_itr = _timeCalibConstants.begin(); t_itr != _timeCalibConstants.end() && t_itr - _timeCalibConstants.begin() < 10; ++t_itr) {
			std::cout << "t_itr = " << *t_itr << std::endl;
		}
#endif
		produceCalibConstants(iSetup.get<EcalTimeCalibConstantsRcd>());

		// time calib errors
		edm::ESHandle<EcalTimeCalibErrors> ecalTimeCalibErrorsHandle;
		//iSetup.get<EcalTimeCalibErrorsRcd>().get( ecalTimeCalibErrorsHandle);
		//_timeCalibErrors = *ecalTimeCalibErrorsHandle;
		//produceCalibErrors(iSetup.get<EcalTimeCalibErrorsRcd>());

		// time offset constant
		edm::ESHandle<EcalTimeOffsetConstant> ecalTimeOffsetConstantHandle;
		iSetup.get<EcalTimeOffsetConstantRcd>().get( ecalTimeOffsetConstantHandle);
		_timeOffsetConstant = *ecalTimeOffsetConstantHandle;
		produceOffsetConstant(iSetup.get<EcalTimeOffsetConstantRcd>());

	}

	EcalTimeCalibConstants _timeCalibConstants; ///< container of calibrations updated iter by iter
	boost::shared_ptr<EcalTimeCalibConstants> _calibConstants; // corrections used during this iter
	inline boost::shared_ptr<EcalTimeCalibConstants> produceCalibConstants(const EcalTimeCalibConstantsRcd& iRecord)
	{
		// new shared_ptr initialized with previous iter: _timeCalibConstants
		_calibConstants = boost::shared_ptr<EcalTimeCalibConstants>( new EcalTimeCalibConstants(_timeCalibConstants) );
		return  _calibConstants;
	}

	EcalTimeCalibErrors    _timeCalibErrors;
	boost::shared_ptr<EcalTimeCalibErrors> _calibErrors;
	inline boost::shared_ptr<EcalTimeCalibErrors>& produceCalibErrors(const EcalTimeCalibErrorsRcd& iRecord)
	{
		_calibErrors = boost::shared_ptr<EcalTimeCalibErrors>( new EcalTimeCalibErrors(_timeCalibErrors) );
		return _calibErrors;
	}

	EcalTimeOffsetConstant _timeOffsetConstant;
	boost::shared_ptr<EcalTimeOffsetConstant> _offsetConstant;
	inline boost::shared_ptr<EcalTimeOffsetConstant> produceOffsetConstant(const EcalTimeOffsetConstantRcd& iRecord)
	{
		_offsetConstant = boost::shared_ptr<EcalTimeOffsetConstant>( new EcalTimeOffsetConstant(_timeOffsetConstant) );
		return _offsetConstant;
	}



	/**
	  \brief  If recHit passes the selection it is added to the list of recHits to be used for calibration

	  The recHit is used (accepted) if:
	    - recHit flag in the list of _recHitFlags defined in the config file
	    - recHit energy in EB > _minRecHitEnergy defined in the config file
	    - recHit energy in EE is x2 the threshold of EB
	    - at each iteration the recHit threshold is raised by _minRecHitEnergyStep
	    If the recHit is used, the time information is added to _eventTimeMap
	*/
	bool addRecHit(const EcalRecHit& recHit, EventTimeMap& eventTimeMap_);


	/// Adds the recHit to the per Event histograms
	void plotRecHit(const EcalTimingEvent& recHit);
	///
	/// Returns an EcalTimingEvent with a new time, which has been adjusted
	/// so that the upstream endcap is 0.
	///
	EcalTimingEvent correctGlobalOffset(const EcalTimingEvent& ev, int splashDir, float bunchCorr);

	std::map<DetId, float>  _CrysEnergyMap;

	edm::Service<TFileService> fileService_;
	TFileDirectory histDir_;
	// Tree
	TTree *timeEBCRYexTree, *timeEECRYexTree, *timeEEpRingTree, *timeEEmRingTree, *timeEBRingTree;
	TTree *_unstableEnergyTree, *_highSkewnessTree;

	// Mean Histograms
	TProfile2D* EneMapEEP_; /// Using TProfile2D so we don't paint empty bins.
	TProfile2D* EneMapEEM_;
	TProfile2D* TimeMapEEP_;
	TProfile2D* TimeMapEEM_;

	TProfile2D* EneMapEB_;
	TProfile2D* TimeMapEB_;

	// Error Histograms
	TProfile2D* TimeErrorMapEEP_;
	TProfile2D* TimeErrorMapEEM_;

	TProfile2D* TimeErrorMapEB_;

	// Event Based Plots
	TProfile2D * Event_EneMapEEP_;
	TProfile2D * Event_EneMapEEM_;
	TProfile2D * Event_EneMapEB_;

	TProfile2D* Event_TimeMapEEP_;
	TProfile2D* Event_TimeMapEEM_;
	TProfile2D* Event_TimeMapEB_;

	TProfile2D* Event_TimeMapEEP_OOT;
	TProfile2D* Event_TimeMapEEM_OOT;
	TProfile2D* Event_TimeMapEB_OOT;

	TH1F* RechitEneEB_;
	TH1F* RechitTimeEB_;
	TH1F* RechitEneEEM_;
	TH1F* RechitTimeEEM_;
	TH1F* RechitEneEEP_;
	TH1F* RechitTimeEEP_;

	TH2F* RechitEnergyTimeEB;
	TH2F* RechitEnergyTimeEEM;
	TH2F* RechitEnergyTimeEEP;

	EcalRingCalibrationTools _ringTools;
	const CaloSubdetectorGeometry * endcapGeometry_;
	const CaloSubdetectorGeometry * barrelGeometry_;

	std::vector<int> _noisyXtals;
	std::vector<TH1F*> _noisyXtalsHists;
	unsigned int _iter;
};

#endif
