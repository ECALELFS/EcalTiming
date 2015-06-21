#include "EcalTiming/EcalTiming/interface/EcalCrystalTimingCalibration.h"
#include <cassert>

float EcalCrystalTimingCalibration::getMeanWithinNSigma(float n_sigma)
{

	if(_numWithinNSigma.count(n_sigma) == 0) calcAllWithinNSigma(n_sigma);
	return _sumWithinNSigma[n_sigma] / _numWithinNSigma[n_sigma];
}

float EcalCrystalTimingCalibration::getStdDevWithinNSigma(float n_sigma)
{

	float mean = getMeanWithinNSigma(n_sigma); //variables are calculated by that
	return sqrt(_sum2WithinNSigma[n_sigma] / _numWithinNSigma[n_sigma] - mean * mean);
}

float EcalCrystalTimingCalibration::getSkewnessWithinNSigma(float n_sigma)
{

	float mean = getMeanWithinNSigma(n_sigma); //variables are calculated by that
	return pow( (_sum3WithinNSigma[n_sigma] - 3 * mean * stdDev * stdDev - mean * mean * mean) / (stdDev * stdDev * stdDev), 1. / 3);

}


// store the results such that you do only one loop over the events
float EcalCrystalTimingCalibration::calcAllWithinNSigma(float n_sigma)
{
	float range = stdDev() * n_sigma;

	_sumWithinNSigma[n_sigma] = 0.;
	_sum2WithinNSigma[n_sigma] = 0;
	_sum3WithinNSigma[n_sigma] = 0.;
	_numWithinNSigma[n_sigma] = 0.;

	for(auto te : timingEvents) {
		if(fabs(te.time() - te.mean()) < range) {
			_sumWithinNSigma[n_sigma] += te.time();
			_sum2WithinNSigma[n_sigma] += te.time() * te.time();
			_sum3WithinNSigma[n_sigma] += te.time() * te.time() * te.time();
			_numWithinNSigma[n_sigma]++;
		}
	}
	return sum / num;
}



bool EcalCrystalTimingCalibration::isStableInEnergy(float min, float max, float step)
{
	unsigned int nSteps = (max - min) / step;

	assert(nSteps < 100);
	assert(nSteps > 1);
	float sum[100], sum2[100];
	unsigned int num[100];

	for(auto te : timingEvents) {
		float energy = min;
		for(unsigned int index = 0; index < nSteps; ++index, energy += step) {

			if(te.energy() > energy) {
				sum[index] += te.time();
				sum2[index] += te.time() * te.time();
				//sum3[index]+=te.time() * te.time() * te.time();
				num[index]++;
			}

		}
	}


	for(unsigned int index = 0; index < nSteps; ++index) {
//	  if(num[index]<50) break; // do the test only if there are at least 50 events

		float mean = sum[index] / num[index];
		float stdDev = sqrt(sum2[index] / num[index] - mean * mean);
		if(stdDev / sqrt(num[index]) > meanError()) break; // does not make any sense to continue if the error is too high

		if(abs(mean() - mean) > meanError() ) return false; /// \todo define a better criterium
		// now requires only that the mean with higher energy threshold is within the calibration statistical uncertainty
	}
	return true;

}

void EcalCrystalTimingCalibration::dumpToTree(TTree *tree, int ix_, int iy_, int iz_)
{
	assert(tree->GetEntries() == 0);
	Float_t time, timeError, energy;
	Int_t ix = ix_, iy = iy_, iz = iz_;
	tree->Branch("ix", &ix, "ix/I");
	tree->Branch("iy", &iy, "iy/I");
	tree->Branch("iz", &iz, "iz/I");
	tree->Branch("time", &time, "time/F");
	tree->Branch("timeError", &timeError, "timeError/F");
	tree->Branch("energy", &energy, "energy/F");

	for(auto te : timingEvents) {
		time = te.time();
		energy = te.energy();
		timeError = te.timeError();
		tree->Fill();
	}
}

