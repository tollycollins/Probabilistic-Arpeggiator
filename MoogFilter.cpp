/***** MoogFilter.cpp *****/

#include "MoogFilter.h"

#include <cmath>
#include <vector>
#include <libraries/math_neon/math_neon.h>

#include "FirstOrderFilter.h"

MoogFilter::MoogFilter() 
{
	setup(1, 0);
}

MoogFilter::MoogFilter(float sampleRate, int type) 
{
	setup(sampleRate, type);
}

void MoogFilter::setup(float sampleRate, int type, float compensation)
{
	sampleRate_ = sampleRate;
	type_ = type;
	
	res_ = 0.75;
	comp_ = compensation;
	
	// initialise first order sections
	for (unsigned int n = 0; n < 4; n++) {
		filterSections_.push_back(FirstOrderFilter());
	}
	// initialise section outputs (including input after non-linearity)
	for (unsigned int n = 0; n < 5; n++) {
		sectionOuts_.push_back(0);
	}	
}

void MoogFilter::set_type(int type) 
{
	type_ = type;
}

void MoogFilter::set_params(float frequencyHz, float resonance, float compensation) 
{
	// calculate g (polynomial approximation of cutoff frequency)
	float omega = 2 * M_PI * frequencyHz  / sampleRate_;			// normalised angular frequency in radians
	float g = 0.9892 * omega - 0.4342 * powf(omega, 2) + 0.1381 * powf(omega, 3) - 0.0202 * powf(omega, 4);
	
	// update Gres (resonance coefficient)
	res_ = resonance * (1.0029 + 0.0526 * omega - 0.0926 * powf(omega, 2) + 0.0218 * powf(omega, 3));
	
	// calculate new coefficients for each first order section		
	float a1 = g - 1;
	float b0 = g / 1.3;
	float b1 = 0.3 * g / 1.3;
	
	// update filter coefficients
	for (unsigned int n = 0; n < filterSections_.size(); n++) {
		filterSections_[n].set_coefficients(a1, b0, b1);
	}
	
	// set compensation coefficient
	comp_ = compensation;
}

float MoogFilter::process(float in) 
{
	// add feedback to input signal
	in = (1 + 4 * res_ * comp_) * in - 4 * res_ * sectionOuts_[4];
	
	// apply nonlinearity
	in = tanhf_neon(in);
	
	// store value of 'in'
	sectionOuts_[0] = in;
	
	// loop through filter sections, applying each section to the signal
	for (unsigned int n = 0; n < 4; n++) {
		in = filterSections_[n].process(in);
		// store signal value
		sectionOuts_[n + 1] = in;
	}
	
	// initialise final output y[n]
	float out = 0;	
	// multiply the outputs from each section by the corresponding coefficients
	for (unsigned int n = 0; n < 5; n++) {
		out += sectionOuts_[n] * filterOutCoeffs_[type_][n];
	}
	
	return out;	
}
	