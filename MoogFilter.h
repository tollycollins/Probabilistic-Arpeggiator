/***** MoogFilter.h *****/

/*
Emulation of Moog Ladder Filter, according to 
Valimaki & Huovilainen (2006) - Oscillator and Filter Algorithms for Virtual Analog Synthesis
*/

#pragma once

#include "FirstOrderFilter.h"
#include <vector>

class MoogFilter {
public:
	MoogFilter();
	
	MoogFilter(float sampleRate, int type = 0);
	
	void setup(float sampleRate, int type = 0, float compensation = 0.5);
	
	void set_type(int type);
	
	void set_params(float frequencyHz, float resonance, float compensation = 0.5);
	
	float process(float in);
	
	~MoogFilter() { };
	
private:
	float sampleRate_;

	std::vector<FirstOrderFilter> filterSections_;
	
	std::vector<float> sectionOuts_;
	
	float res_;
	float comp_;
	
	int type_;
	
	// coefficients for filter types
	const std::vector<std::vector<float>> filterOutCoeffs_ {{0, 0, 1, 0, 0},	// Lowpass 6
											    		    {0, 0, 0, 0, 1},	// Lowpass 12
											    		    {0, 2, -2, 0, 0},	// Bandpass 6
											    		    {0, 0, 4, -8, 0},	// Bandpass 12
											    		    {1, -2, 1, 0, 0},	// Highpass 6
											    		    {1, -4, 6, -4, 1}};	// Highpass 12	
};
