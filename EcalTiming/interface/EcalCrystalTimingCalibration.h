#include "EcalTiming/EcalTiming/interface/EcalTimingEvent.h"
#include <cassert>
#include <vector>
#include <TTree.h>

/** \class EcalCrystalTimingCalibration EcalCrystalTimingCalibration.h EcalTiming/EcalTiming/interface/EcalCrystalTimingCalibration.h
 *
 * Description: add a description here
 * This class contains all the timing information for a single crystal
 */


class EcalCrystalTimingCalibration
{
public:
//	DetId _detId; ///< detId of the channel


private:

	float _sum; ///< scalar sum of the time of each timingEvent
	float _sum2; ///< scalar sum of the square of the time of each timingEvent
	unsigned long int _num; ///< number of timingEvents;

	float _sumE;

	std::vector<EcalTimingEvent> timingEvents; ///< vector containing  all the events for this crystal
	std::vector<EcalTimingEvent>::iterator maxChi2Itr;


public:
	/// default constructor
	EcalCrystalTimingCalibration(bool weightMean = true) :
		//_detId(),
		_sum(0), _sum2(0), _num(0), _sumE(0)
		//totalChi2(-1),
		//useWeightedMean(weightMean)
	{
	}

	inline unsigned int num() const
	{
		return _num;
	};
	inline float mean() const
	{
		return _sum / _num;
	}; ///< average time (mean of the time distribution)
	inline float stdDev() const  ///< standard deviation of the time distribution
	{
		float mean_ = mean();
		return sqrt(_sum2 / _num - mean_ * mean_);
	};
	inline float meanError() const
	{
		return stdDev() / sqrt(_num);
	};
	inline float meanE() const
	{
		return _sumE / _num;
	}; ///< average Energy (mean of the Energy  distribution)
	/* bool operator<( EcalCrystalTimingCalibration& rhs) */
	/* { */
	/* 	if(_detId < rhs._detId) return true; */
	/* 	else return false; */
	/* } */
	//float totalChi2;


	friend std::ostream& operator<< (std::ostream& os, const EcalCrystalTimingCalibration& s)
	{
		os << s.mean() << "\t" << s.stdDev() << "\t" << s.num();
		return os;
	}

	/// add new event for this crystal
	inline bool add(EcalTimingEvent te_)
	{
		return insertEvent(te_);
	}
	inline void clear()
	{
	_sum = 0.0f;
	_sum2 = 0.0f;
	_num = 0;
	_sumE = 0.0f;

	timingEvents.clear();
	}

	/// need an empty tree
	void dumpToTree(TTree *tree, int ix_, int iy_, int iz_){
		assert(tree->GetEntries()==0);
		Float_t time, timeError, energy;
		Int_t ix=ix_, iy=iy_, iz=iz_;
		tree->Branch("ix", &ix, "ix/I");
		tree->Branch("iy", &iy, "iy/I");
		tree->Branch("iz", &iz, "iz/I");
		tree->Branch("time", &time, "time/F");
		tree->Branch("timeError", &timeError, "timeError/F");
		tree->Branch("energy", &energy, "energy/F");

		for(auto te : timingEvents){
			time = te.time();
			energy = te.energy();
			timeError = te.timeError();
			tree->Fill();
		}
	}		

private:
	/// \todo weighted average by timeError
	bool insertEvent(EcalTimingEvent te_)
	{
		if(true || te_.timeError() > 0) {
			_sum += te_.time();
			_sum2 += te_.time() * te_.time();
			_sumE += te_.energy();
			_num++;
			timingEvents.push_back(te_);
			//updateChi2();
			return true;
		} else {
			return false;
		}
	}

	/* int filterOutliers(float threshold = 0.5) */
	/* { */
	/* 	int numPointsErased = 0; */

	/* 	while(timingEvents.size() > 4) { */
	/* 		updateChi2(); */
	/* 		float oldMean = mean; */
	/* 		// Erase largest chi2 event */
	/* 		EcalTimingEvent toRemove = *maxChi2Itr; */
	/* 		timingEvents.erase(maxChi2Itr); */
	/* 		//Calculate new mean/error */
	/* 		updateChi2(); */

	/* 		//Compare to old mean and break if |(newMean-oldMean)| < newSigma */
	/* 		//TODO: study acceptance threshold */
	/* 		if(fabs(mean - oldMean) < threshold * meanE) { */
	/* 			insertEvent(toRemove); */
	/* 			break; */
	/* 		} else { */
	/* 			numPointsErased++; */
	/* 		} */
	/* 	} */

	/* 	return numPointsErased; */
	/* } */

private:
	/* bool useWeightedMean; */

	/* //calculate chi2: assume a gaussian distribution */
	/* void updateChi2() // update individual, total, maxChi2s */
	/* { */
	/* 	if(useWeightedMean) { */
	/* 		updateChi2Weighted(); */
	/* 	} else { */
	/* 		updateChi2Unweighted(); */
	/* 	} */
	/* } */

	/* void updateChi2Weighted() */
	/* { */
	/* 	updateMeanWeighted(); */
	/* 	float chi2 = 0; */
	/* 	maxChi2Itr = timingEvents.begin(); */

	/* 	for(std::vector<EcalTimingEvent>::iterator itr = timingEvents.begin(); */
	/* 	        itr != timingEvents.end(); ++itr) { */
	/* 		float singleChi = (itr->time - mean) / itr->sigmaTime; */
	/* 		itr->chi2 = singleChi * singleChi; */
	/* 		chi2 += singleChi * singleChi; */

	/* 		if(itr->chi2 > maxChi2Itr->chi2) { */
	/* 			maxChi2Itr = itr; */
	/* 		} */
	/* 	} */

	/* 	totalChi2 = chi2; */
	/* } */

	/* void updateChi2Unweighted() */
	/* { */
	/* 	updateMeanUnweighted(); */
	/* 	float chi2 = 0; */
	/* 	maxChi2Itr = timingEvents.begin(); */

	/* 	for(std::vector<EcalTimingEvent>::iterator itr = timingEvents.begin(); */
	/* 	        itr != timingEvents.end(); ++itr) { */
	/* 		float singleChi = (itr->time - mean); */
	/* 		itr->chi2 = singleChi * singleChi; */
	/* 		chi2 += singleChi * singleChi; */

	/* 		if(itr->chi2 > maxChi2Itr->chi2) { */
	/* 			maxChi2Itr = itr; */
	/* 		} */
	/* 	} */

	/* 	totalChi2 = chi2; */
	/* } */

	/* void updateMeanWeighted() */
	/* { */
	/* 	float meanTmp = 0; */
	/* 	float mean2Tmp = 0; */
	/* 	float sigmaTmp = 0; */

	/* 	for(std::vector<EcalTimingEvent>::const_iterator itr = timingEvents.begin(); */
	/* 	        itr != timingEvents.end(); ++itr) { */
	/* 		float sigmaT2 = itr->sigmaTime; */
	/* 		sigmaT2 *= sigmaT2; */
	/* 		sigmaTmp += 1 / (sigmaT2); */
	/* 		meanTmp += (itr->time) / (sigmaT2); */
	/* 		mean2Tmp += ((itr->time) * (itr->time)) / (sigmaT2); */
	/* 	} */

	/* 	meanE = sqrt(1 / sigmaTmp); */
	/* 	mean = meanTmp / sigmaTmp; */
	/* 	rms = sqrt(mean2Tmp / sigmaTmp); */
	/* 	stdDev = sqrt(rms * rms - mean * mean); */
	/* } */

	/* void updateMeanUnweighted() */
	/* { */
	/* 	float meanTmp = 0; */
	/* 	float mean2Tmp = 0; */

	/* 	for(std::vector<EcalTimingEvent>::const_iterator itr = timingEvents.begin(); */
	/* 	        itr != timingEvents.end(); ++itr) { */
	/* 		meanTmp += itr->time; */
	/* 		mean2Tmp += (itr->time) * (itr->time); */
	/* 	} */

	/* 	mean = meanTmp / timingEvents.size(); */
	/* 	rms = sqrt(mean2Tmp / timingEvents.size()); */
	/* 	stdDev = sqrt(rms * rms - mean * mean); */
	/* 	meanE = stdDev / sqrt(timingEvents.size()); // stdDev/sqrt(n) */
	/* } */

};
