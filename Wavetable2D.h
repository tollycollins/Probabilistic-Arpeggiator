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

class Wavetable2D {
public:
	Wavetable2D() {}													// Default constructor
	
	Wavetable2D(float sampleRate, std::vector<float>& wave1, 			// Constructor with arguments
				std::vector<float>& wave2,
				int num_tables,
				unsigned int voices,
				std::vector<float> voicePositions,
			    bool useInterpolation = true); 						
				
	void setup(float sampleRate,										// Set parameters
			   std::vector<float>& wave1,					
			   std::vector<float>& wave2,
			   int num_tables,
			   unsigned int voices = 1,
			   std::vector<float> detuneRatios = {1.0},			   
			   bool useInterpolation = true); 		
	
	void setFrequency(float f);									// Set the oscillator frequency
	float getFrequency();										// Get the oscillator frequency
	void setDetune(float detune);								// set detune ratio
	void setVoices(unsigned int voices);						// set number of detuned voices
	void setDetuneRatios(std::vector<float> ratios);		// set relative detune ratio for each voice
	
	void setTable(float mix);		// set the mix between the two waveforms
	
	float process();				// Get the next sample and update the phase
	
	~Wavetable2D() {}				// Destructor

private:
	std::vector<float> wave1_;				// Buffer holding waveform 1
	std::vector<float> wave2_;				// Buffer holding waveform 2
	std::vector<std::vector<float>> table_;	// Buffer holding the full 2D wavetable
	
	unsigned int tablePosition_;			// Current position in the 2D table

	float inverseSampleRate_;				// 1 divided by the audio sample rate	
	float frequency_;						// Frequency of the oscillator
	unsigned int voices_;					// number of voices for detuning
	std::vector<float> readPointers_;		// Location of the read pointer (phase of oscillator)
	float detune_;
	std::vector<float> detuneRatios_;	// detune ratios for each voice
	bool useInterpolation_;					// Whether to use linear interpolation
};