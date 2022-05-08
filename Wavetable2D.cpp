/*
 ____  _____ _        _    
| __ )| ____| |      / \   
|  _ \|  _| | |     / _ \  
| |_) | |___| |___ / ___ \ 
|____/|_____|_____/_/   \_\

http://bela.io

C++ Real-Time Audio Programming with Bela - Lecture 5: Classes and Objects
wavetable-class: an example that implements a wavetable oscillator as a C++ class
*/

// adapted from wavetable.cpp

#include <cmath>
#include "Wavetable2D.h"

// Constructor taking arguments for sample rate and table data
Wavetable2D::Wavetable2D(float sampleRate, 
						 std::vector<float>& wave1, 
						 std::vector<float>& wave2, 
						 int num_tables, 
						 unsigned int voices,
						 std::vector<float> detuneRatios,						 
						 bool useInterpolation) 
{
	setup(sampleRate, wave1, wave2, num_tables, voices, detuneRatios, useInterpolation);
} 

void Wavetable2D::setup(float sampleRate, 
						std::vector<float>& wave1, 
						std::vector<float>& wave2, 
						int num_tables, 
						unsigned int voices,
						std::vector<float> detuneRatios,		
						bool useInterpolation)
{
	// It's faster to multiply than to divide on most platforms, so we save the inverse
	// of the sample rate for use in the phase calculation later
	inverseSampleRate_ = 1.0 / sampleRate;

	// Copy other parameters
	wave1_ = wave1;
	wave2_ = wave2;
	useInterpolation_ = useInterpolation;
	
	// build 2D Wavetable
	table_.resize(num_tables);
	for (unsigned int n = 0; n < table_.size(); n++) {
		table_[n].resize(wave1_.size());
		float mix = (float)n / (float)(table_.size() - 1);
		for (unsigned int i = 0; i < wave1_.size(); i++) {
			table_[n][i] = (1 - mix) * wave1_[i] + mix * wave2_[i];
		} 
	}
	
	// set up voices
	voices_ = voices;
	
	// Initialise the starting state
	readPointers_.resize(voices);
	for (unsigned int i = 0; i < voices; i++) {
		readPointers_[i] = 0;
	}

	detuneRatios_ = detuneRatios;
	
}

// Set the oscillator frequency
void Wavetable2D::setFrequency(float f) 
{
	frequency_ = f;
}

// Get the oscillator frequency
float Wavetable2D::getFrequency() 
{
	return frequency_;
}		

// set the detune ratio
void Wavetable2D::setDetune(float detune) 
{
	detune_ = detune;
}

// set the detune ratios for each voice
void Wavetable2D::setDetuneRatios(std::vector<float> ratios) 
{
	detuneRatios_ = ratios;
}

// interpolate between the two waveforms
void Wavetable2D::setTable(float mix) {
	tablePosition_ = (int)(table_.size() * mix);
}
	
// Get the next sample and update the phase
float Wavetable2D::process() {
	float out = 0;
	
	// Make sure we have a valid table
	if(table_.size() == 0 || table_[tablePosition_].size() == 0)
		return out;
	
	// Increment and wrap the phase
	for (unsigned int i = 0; i < voices_; i++) {
		float frequency = frequency_ * (1.0 + detuneRatios_[i] * detune_);
		readPointers_[i] += table_[tablePosition_].size() * frequency * inverseSampleRate_;
		while (readPointers_[i] >= table_[tablePosition_].size())
			readPointers_[i] -= table_[tablePosition_].size();
	}
	
	if(useInterpolation_) {
		for (unsigned int i = 0; i < voices_; i++) {
			// The pointer will take a fractional index. Look for the sample on
			// either side which are indices we can actually read into the buffer.
			// If we get to the end of the buffer, wrap around to 0.
			int indexBelow = floorf(readPointers_[i]);
			int indexAbove = indexBelow + 1;
			if(indexAbove >= table_[tablePosition_].size())
				indexAbove = 0;
		
			// For linear interpolation, we need to decide how much to weigh each
			// sample. The closer the fractional part of the index is to 0, the
			// more weight we give to the "below" sample. The closer the fractional
			// part is to 1, the more weight we give to the "above" sample.
			float fractionAbove = readPointers_[i] - indexBelow;
			float fractionBelow = 1.0 - fractionAbove;
		
			// Calculate the weighted average of the "below" and "above" samples
		    out += fractionBelow * table_[tablePosition_][indexBelow] +
		    	   fractionAbove * table_[tablePosition_][indexAbove];
		}
	}
	else {
		for (unsigned int i = 0; i < voices_; i++) {
			// Read the table without interpolation
			out += table_[tablePosition_][(int)readPointers_[i]];
		}
	}
	
	// reduce amplitude to compensate for multiple voices
	out /= (float)voices_;
	
	return out;
}			