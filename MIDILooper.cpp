/***** Looper.cpp *****/

#include "MIDILooper.h"

#include <vector>

// default constructor
MIDILooper::MIDILooper() 
	: sRateRecip_(1), bps_(0), beatsPerBar_(4), barsPerCycle_(4), ticksPerBeat_(960), 
	  pointer_(0), fractionCounter_(0), overwrite_(true)
{
	
}

// overloaded constructor
MIDILooper::MIDILooper(float sampleRate,				// to set up with the audio sample rate
					   float tempo,
					   unsigned int beatsPerBar,
					   unsigned int barsPerCycle, 
					   unsigned int ticksPerBeat, 
					   std::vector<int> noMessage) 
	: beatsPerBar_(beatsPerBar), barsPerCycle_(barsPerCycle), ticksPerBeat_(ticksPerBeat),
	  pointer_(0), fractionCounter_(0), overwrite_(true), noMessage_(noMessage)
{
	sRateRecip_ = 1.0 / sampleRate;
	bps_ = tempo / 60.0;
	
	reset();
	
   writeTemp_ = noMessage_;
   readTemp_ = noMessage_;
   output_ = noMessage_;
}


// to initialise with the sample rate
void MIDILooper::setup(float sampleRate, 
					   float tempo,
					   unsigned int beatsPerBar,
					   unsigned int barsPerCycle, 
					   unsigned int ticksPerBeat, 
					   std::vector<int> noMessage)
{
	noMessage_ = noMessage;
	
	sRateRecip_ = 1.0 / sampleRate;
	setTempo(tempo);
	beatsPerBar_ = beatsPerBar;
	barsPerCycle_ = barsPerCycle;
	ticksPerBeat_ = ticksPerBeat;
	
	reset();								// resets the buffer
	
	writeTemp_ = noMessage_;
	readTemp_ = noMessage_;
	output_ = noMessage_;
}

void MIDILooper::reset() {
	buffer_ = std::vector<std::vector<int>>(ticksPerBeat_ * beatsPerBar_ * barsPerCycle_, noMessage_);
}

void MIDILooper::process()
{
	// increment fraction counter
	fractionCounter_ += bps_ * sRateRecip_ * ticksPerBeat_;
	
	// increment buffer pointer if a whole number has been reached
	while (fractionCounter_ >= 1.0) {
		// check for a write
		if (writeTemp_ != noMessage_) {
			buffer_[pointer_] = writeTemp_;
			// clear temp write buffer
			writeTemp_ = noMessage_;
		}
		// check for overwrite flag
		else if (overwrite_) {
			buffer_[pointer_] = noMessage_;
		}
		
		// increment pointer and check for falling off end of buffer
		if (++pointer_ >= buffer_.size()) {
			pointer_ -= buffer_.size();
		}
		// decrement fraction counter
		fractionCounter_ -= 1;
		
		// place value of next main buffer entry into the read buffer
		readTemp_ = buffer_[pointer_];
	}
}

// get / set the overwrite flag (determines whether or not to wipe the previous loop as it plays)
void MIDILooper::setOverwrite(bool flag) {overwrite_ = flag; }
bool MIDILooper::getOverwrite() {return overwrite_; }

// write a value pair to the buffer 
void MIDILooper::write(std::vector<int> value) {
	// check size and add to write buffer
	if (value.size() == noMessage_.size()) {
		writeTemp_ = value; 
	}
}

// read a value pair from the buffer 
std::vector<int> MIDILooper::read() 
{
	// transfer contents of read buffer
	output_ = readTemp_;
	// clear read buffer
	readTemp_ = noMessage_;
	
	return output_;
}

// // add mode MIDI value
// void MIDILooper::addModeValue(int mode) {writeTemp_[2] = mode; }

// change the metrical structure
void MIDILooper::setMetre(unsigned int beatsPerBar, unsigned int barsPerCycle)
{
   beatsPerBar_ = beatsPerBar;
   barsPerCycle_ = barsPerCycle;	
   
   // reset the buffer
   reset();
}

// change the tempo
void MIDILooper::setTempo(float tempo) {bps_ = tempo / 60.0; }

