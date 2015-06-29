#include "EcalTiming/EcalTiming/interface/EcalCrystalTimingCalibration.h"
#include <cassert>

float EcalCrystalTimingCalibration::getMeanWithinNSigma(float n_sigma, float maxRange)
{

	if(_numWithinNSigma.count(n_sigma) == 0) calcAllWithinNSigma(n_sigma);
	return _sumWithinNSigma[n_sigma] / _numWithinNSigma[n_sigma];
}

float EcalCrystalTimingCalibration::getStdDevWithinNSigma(float n_sigma, float maxRange)
{

	float mean = getMeanWithinNSigma(n_sigma, maxRange); //variables are calculated by that
	return sqrt(_sum2WithinNSigma[n_sigma] / _numWithinNSigma[n_sigma] - mean * mean);
}

float EcalCrystalTimingCalibration::getSkewnessWithinNSigma(float n_sigma, float maxRange)
{

	float mean = getMeanWithinNSigma(n_sigma, maxRange); //variables are calculated by that
	float stdDev = getStdDevWithinNSigma(n_sigma, maxRange);
	return (_sum3WithinNSigma[n_sigma] / _numWithinNSigma[n_sigma] - 3 * mean * stdDev * stdDev - mean * mean * mean) / (stdDev * stdDev * stdDev);

}


// store the results such that you do only one loop over the events
void EcalCrystalTimingCalibration::calcAllWithinNSigma(float n_sigma, float maxRange)
{
	float range = std::min(maxRange, stdDev() * n_sigma);

	_sumWithinNSigma[n_sigma] = 0.;
	_sum2WithinNSigma[n_sigma] = 0;
	_sum3WithinNSigma[n_sigma] = 0.;
	_numWithinNSigma[n_sigma] = 0.;

	for(auto te : timingEvents) {
		if(fabs(te.time() - mean()) < range) {
			_sumWithinNSigma[n_sigma] += te.time();
			_sum2WithinNSigma[n_sigma] += te.time() * te.time();
			_sum3WithinNSigma[n_sigma] += te.time() * te.time() * te.time();
			_numWithinNSigma[n_sigma]++;
		}
	}
	return;
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

		float mean_ = sum[index] / num[index];
		float stdDev_ = sqrt(sum2[index] / num[index] - mean_ * mean_);
		if(stdDev_ / sqrt(num[index]) > 1.41 * meanError() || num[index] < 30) break; // does not make any sense to continue if the error is too high

		if(abs(mean() - mean_) > meanError() ) return false; /// \todo define a better criterium
		// now requires only that the mean with higher energy threshold is within the calibration statistical uncertainty
	}
	return true;

}

void EcalCrystalTimingCalibration::dumpToTree(TTree *tree, int ix_, int iy_, int iz_, unsigned int status_, unsigned int elecID_)
{
	//assert(tree->GetEntries() == 0);
	Float_t time, timeError, energy;
	UShort_t ix(ix_), iy(iy_); 
	Char_t iz(iz_);
	UShort_t elecID(elecID_);
	UChar_t status(status_);

	if(tree->GetBranch("ix") == NULL) tree->Branch("ix", &ix, "ix/s");
	else tree->SetBranchAddress("ix", &ix);

	if(tree->GetBranch("iy") == NULL) tree->Branch("iy", &iy, "iy/s");
	else tree->SetBranchAddress("iy", &iy);

	if(tree->GetBranch("iz") == NULL) tree->Branch("iz", &iz, "iz/B");
	else tree->SetBranchAddress("iz", &iz);

	if(tree->GetBranch("elecID") == NULL) tree->Branch("elecID", &elecID, "elecID/s");
	else tree->SetBranchAddress("elecID", &elecID);

	if(tree->GetBranch("status") == NULL) tree->Branch("status", &status, "status/b");
	else tree->SetBranchAddress("status", &status);

	if(tree->GetBranch("time") == NULL) tree->Branch("time", &time, "time/F");
	else tree->SetBranchAddress("time", &time);

	if(tree->GetBranch("timeError") == NULL) tree->Branch("timeError", &timeError, "timeError/F");
	tree->SetBranchAddress("timeError", &timeError);

	if(tree->GetBranch("energy") == NULL) tree->Branch("energy", &energy, "energy/F");
	tree->SetBranchAddress("energy", &energy);

	for(auto te : timingEvents) {
		time = te.time();
		energy = te.energy();
		timeError = te.timeError();
		tree->Fill();
	}
}

