/***** Looper.h *****/

#pragma once

#include <vector>

class MIDILooper {
public:
	MIDILooper();								// default constructor
	
	MIDILooper(float sampleRate,				// construct with arguments
			   float tempo = 0,
			   unsigned int beatsPerBar = 4,
			   unsigned int barsPerCycle = 4, 
			   unsigned int ticksPerBeat = 960,
			   std::vector<int> noMessage = {-1, -1, -1, -1});
	
	void setup(float sampleRate,				// to set up with the audio sample rate
			   float tempo,
			   unsigned int beatsPerBar = 4,
			   unsigned int barsPerCycle = 4, 
			   unsigned int ticksPerBeat = 960, 
			   std::vector<int> noMessage = {-1, -1, -1, -1});		// 960 PPQ is a common industry standard
			   
	void reset();										// clears the buffer
	
	void process();										// move on the pointer_ and fractionCunter_ variables per audio sample		
	
	void setOverwrite(bool flag);						// if true, buffer elements will be set to noMessage once read
	bool getOverwrite();								// get value of overwrite flag
	
	void write(std::vector<int> value);					// write into the buffer
	std::vector<int> read();							// read from the buffer 
	
	// void addModeValue(int mode);						// add MIDI value of mode MIDI CC message
	
	void setMetre(unsigned int beatsPerBar, unsigned int barsPerCycle);		// define the metre
	void setTempo(float tempo);												// set the tempo
	
	~MIDILooper() = default;					// destructor
	
private:
	float sRateRecip_;							// (1 / audio sample rate) - multiplications are faster than divisions
	float bps_;									// current sequencer tempo in beats per second
	unsigned int beatsPerBar_;					// quarter notes per bar
	unsigned int barsPerCycle_;					// Bars per sequencer cycle
	
	unsigned int ticksPerBeat_;					// buffer uses ticks rather than absolute time to be tempo-agnostic
	
	std::vector<std::vector<int>> buffer_;		// stores MIDI messages of the form {noteNumber, velocity, LED number}
	unsigned int pointer_;						// track the read/write position in the buffer
	float fractionCounter_;						// keeps track of when to increment the pointer
	
	bool overwrite_;							// whether or not to overwrite the previous buffer iteration, or to add
	
	std::vector<int> writeTemp_;				// store a write message until the pointer moves on
	std::vector<int> readTemp_;					// store a read message until it is read
	std::vector<int> output_;					// transfer entry from readTemp_ for output
	
	std::vector<int> noMessage_;				// signifier of no MIDI message
};
