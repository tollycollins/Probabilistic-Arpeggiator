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

// Wavetable.h: header file for wavetable oscillator class

#pragma once

#include <vector>

class Wavetable1D {
public:
	Wavetable1D() {}													// Default constructor
	
	Wavetable1D(float sampleRate, std::vector<float>& wave1, 			// Constructor with arguments
				unsigned int voices,
				std::vector<float> voicePositions,
			    bool useInterpolation = true); 						
				
	void setup(float sampleRate,										// Set parameters
			   std::vector<float>& wave1,					
			   unsigned int voices = 1,
			   std::vector<float> detuneRatios = {1.0},			   
			   bool useInterpolation = true); 		
	
	void setFrequency(float f);									// Set the oscillator frequency
	float getFrequency();										// Get the oscillator frequency
	void setDetune(float detune);								// set detune ratio
	void setVoices(unsigned int voices);						// set number of detuned voices
	void setDetuneRatios(std::vector<float> ratios);			// set relative detune ratio for each voice
	
	float process();				// Get the next sample and update the phase
	
	~Wavetable1D() {}				// Destructor

private:
	std::vector<float> wave1_;				// Buffer holding waveform 1

	float inverseSampleRate_;				// 1 divided by the audio sample rate	
	float frequency_;						// Frequency of the oscillator
	unsigned int voices_;					// number of voices for detuning
	std::vector<float> readPointers_;		// Location of the read pointers (phase of oscillator)
	float detune_;
	std::vector<float> detuneRatios_;	// detune ratios for each voice
	bool useInterpolation_;					// Whether to use linear interpolation
};