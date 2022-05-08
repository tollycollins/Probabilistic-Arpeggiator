/*
Music and Audio Programming - final project
Tolly Collins
*/

#include <Bela.h>
#include <libraries/Gui/Gui.h>
#include <libraries/GuiController/GuiController.h>
#include <libraries/Scope/Scope.h>
#include <libraries/Midi/Midi.h>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <utility>
#include "Wavetable1D.h"
#include "Wavetable2D.h"
#include "ADSR.h"
#include "MoogFilter.h"
#include "ProbabilisticArp.h"
#include "MonoFilePlayer.h"
#include "MIDILooper.h"


// global constants and variables
// oscillators
const unsigned int kWavetableSize = 512;
const unsigned int kWavetable2DSize = 128;

unsigned int gBassDetuneVoices = 3;
Wavetable2D gBassOsc;
std::vector<float> gBassDetuneVoicePositions {-1, 0, 1};
float gBassAmp = 0.4;
unsigned int gBassNote = 0;
// flag to determine whether bass should be played
int gPlayBass = 0;

unsigned int gSubBassDetuneVoices = 1;
Wavetable1D gSubBassOsc;
std::vector<float> gSubBassDetuneVoicePositions {-1};
float gSubBassAmp = 0.1;

unsigned int gLeadDetuneVoices = 4;
Wavetable2D gLeadOsc;
std::vector<float> gLeadDetuneVoicePositions {-1, -0.3, 0.3, 1};
std::pair<int, float> gLeadNoteAmp;
// float gLeadAmp = 0.8;
// unsigned int gLeadNote = 0;
// flag to determine whether lead should be played
// int gPlayLead = 0;

// filters
MoogFilter gBassFilt;
MoogFilter gLeadFilt;
// lead filter resonance
float gLeadFiltCutoff = 2030;

// Metronome and sequencer values
// tempo values
const float kMinTempo = 90.0;
const float kMaxTempo = 150.0;
float gTempo = 120.0;
bool gIntTempo = true;
// metre
unsigned int ksubBeatsPerBeat = 4;
unsigned int kBeatsPerBar = 4;
unsigned int kBarsPerPattern = 4;
// sample counter
unsigned int gCounter = 0;

// lead note event
void nextEvent();

// // patterns
// const std::vector<int> gPatterns {{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

// ADSR
ADSR gBassAmpADSR;
ADSR gBassFiltADSR;
ADSR gLeadAmpADSR;
ADSR gLeadFiltADSR;

// Device for handling MIDI messages
Midi gMidi;
// MIDI export
const char* gMidiPort0 = "hw:1,0,0";

// MIDI Handler Function Prototypes
void noteOn(int noteNumber, int velocity);
void noteOff(int noteNumber);
void controlChange(int controller, int value);

// MIDI callback function
void midiEvent(MidiChannelMessage message, void *arg);

// MIDI Controller numbers for different parameters
enum {
	// instrument on/off buttons
	kMIDIControllerKick = 85,
	kMIDIControllerBass = 86,
	kMIDIControllerLead = 87,
	
	// looper button (rhombus)
	kMIDIControllerLoop = 88,
	
	// tempo slider (long slider)
	kMIDIControllerTempo = 89,
	
	// 'temperature' controls (control arpeggiator generation behaviour)
	// === 1st horizontal slider bank - top to bottom ===
	kMIDIControllerPitchTemp = 102,				// how closely the generation follows the pitch of the seed sequence
	kMIDIControllerHarmonicTemp = 103,			// degree of harmonic freedom allowed (degree to which unexpected notes within a given key are allowed)
	kMIDIControllerRhythmicTemp = 104,			// degree of rhythmic freedom allowed relative to preceeding sequence (how closely the rhythm should match previous rhythm)
	kMIDIControllerDynamicContourTemp = 105,	// how far the dynamics can stray from the seed seqeunce
	
	// === 2nd horizontal slider bank - top to bottom ===
	kMIDIControllerContourTemp = 106,			// how closely the generation follows the contour of the seed sequence
	kMIDIControllerSparsity = 107,				// affects the probability of a new note sounding at this metrical position (as opposed to no note sounding)
	kMIDIControllerMovement = 108,				// degree of variation allowed from previous note (high value will result in more repeated notes and smaller intervals)	
	kMIDIControllerDynamicTemp = 109,			// how much the dynamics can vary by for each generated note

	// === 3rd horizontal slider bank - top to bottom ===
	kMIDIControllerIntervalTemp = 110,			// how far the generative process can stray (in terms of interval) from the previous sequence / note	
	
	kMIDIControllerConsistency = 112,			// degree of contour freedom allowed relative to sequence from preceeding pattern 
	
	// balance between arpeggiator seed sequences
	kMIDIControllerArpOverallTemperature = 14,	// proportionally raise or lower all arpeggiator temperature levels
	kMIDIControllerArpSeedBalance = 15,			// interpolation ratio between 2 seed arpeggiator sequences 

	// 'flavour' controls - control sound characteristics 
	kMIDIControllerBassAmp = 20,
	kMIDIControllerBassTablePos = 21,
	kMIDIControllerBassDetune = 22,
	
	kMIDIControllerLeadWavetableMix = 24,
	kMIDIControllerLeadADSRa = 25,
	kMIDIControllerLeadADSRd = 26,
	kMIDIControllerLeadADSRs = 27,
	
	kMIDIControllerLeadFiltCutoff = 28,
	kMIDIControllerLeadFiltADSRa = 29,
	kMIDIControllerLeadFiltADSRd = 30,
	kMIDIControllerLeadFiltADSRs = 31,
	
	// for messages from QuNeo
	kMIDIControllerMode = 3,
	kMIDIControllerLED = 9
};

// updates QuNeo slider representing overall temperature
void overallTempLEDMidiMessage();

// default non-message for MIDI looper
const std::vector<int> kNoMessage {-1, -1, -1, -1};

// MIDI Looping for bass notes
MIDILooper gBassLoop;
// resolution for MIDI information (samples per beat)
const unsigned int kLooperMIDIRes = 960;
// flag for read/write behaviour of bass looper
bool gBassLoopRead = false;
// to hold a loop note message for reading or writing
std::vector<int> gLoopNoteReadMessage {kNoMessage};
std::vector<int> gLoopNoteWriteMessage {kNoMessage};
// allows bass loop reading to be suspended while a note is pressed
bool gBassLoopReadOverride = false;

// for LED Control
unsigned int gBassLED1;
unsigned int gBassLED2;

// MIDI CC values for control change LEDs
enum {
	kLEDTemperature1 = 11,
	kLEDTemperature2 = 10,
	kLEDTemperature3 = 9,
	kLEDTemperature4 = 8,
	kLEDTempOverall = 7,
	kLEDSeedBalance = 6,
	kLEDTempo = 5,
	kLEDSound1 = 1,
	kLEDSound2 = 2,
	kLEDSound3 = 3,
	kLEDSound4 = 4,
};

// // to keep track of LED bank changes
// unsigned int gLEDTemp1LastCCVal = 0;
// unsigned int gLEDTemp2LastCCVal = 0;
// unsigned int gLEDTemp3LastCCVal = 0;
// unsigned int gLEDTemp4LastCCVal = 0;

// MIDI CC note numbers for LEDs with note number controls
enum {
	kLEDKick = 33,
	kLEDBass = 34,
	kLEDArp = 35,
	kLEDLoop1 = 44,
	kLEDLoop2 = 45,
};


// Lead Arpeggiator
// arpeggiator parameters
const unsigned int kLowestArpNote = 48;		// MIDI value of the lowest allowed output note	
const unsigned int kArpOctaveRange = 4;		// octave range above lowest note for possible outputs
int gArpSeed1 = 1;					// integer seed value (note: a seed of -1 will result in a random pick)
int gArpSeed2 = 0;					// integer seed value (note: a seed of -1 will result in a random pick)
float gArpSeedBalance = 0;			// sets interpolation ration between 2 seed sequences
unsigned int gArpTempDist = 0;		// choice of distribution for harmonic note options
// initialise arpeggiator
ProbabilisticArp gArp(ksubBeatsPerBeat, kBeatsPerBar, kBarsPerPattern, kLowestArpNote, 
					  kArpOctaveRange, gArpSeed1, gArpSeed2, gArpSeedBalance, gArpTempDist);
// for updating temperature controls via MIDI
float gArpOverallTemperature = 0;
// get useful values from gArp
const unsigned int kArpNumSeeds = gArp.numSeeds();
const unsigned int kArpNumTempDists = gArp.getNumTempDists();

// Object that handles playing sound from a file
MonoFilePlayer gPlayer;
// name of the sound file for the gPlayer
std::string gFilename = "171104__dwsd__kick-gettinglaid.wav"; 
// flag for playing Kick Drum
unsigned int gPlayKick = 0;
unsigned int gKickCounter = 0;
// kick Amplitude
float gKickAmp = 0.65;
// factor for reducing amplitude on non-downbeats
float gKickAmpRed = 1.0;
const float kKickAmpRedRatio = 0.7;

// Browser-based GUI to adjust parameters
Gui gGui;
GuiController gGuiController;

// oscilloscope
Scope gScope;


bool setup(BelaContext *context, void *userData)
{
	// initialise wavetable buffer
	std::vector<float> wavetableSaw(kWavetableSize);
	// populate buffer with basic waveform
	for (unsigned int n = 0; n < wavetableSaw.size(); n++) {
		wavetableSaw[n] = powf(2.0 * (float)n / (float)wavetableSaw.size() - 1, 2);
	} 
	// apply filter [H(z) = (1 - Z^-2) / 2]
	float first_val = wavetableSaw[0] - wavetableSaw[wavetableSaw.size() - 1];
	float second_val = wavetableSaw[1] - wavetableSaw.back();
	for (unsigned int n = wavetableSaw.size() - 1; n >= 2; n--) {
		wavetableSaw[n] = (wavetableSaw[n] - wavetableSaw[n-2]) / 2.0;
	}
	wavetableSaw[0] = first_val;
	wavetableSaw[1] = second_val;
	// 
	// normalise saw wavetable
	float max_elem = *std::max_element(wavetableSaw.begin(), wavetableSaw.end());
	for (unsigned int n = 0; n < wavetableSaw.size(); n++) {
		wavetableSaw[n] /= max_elem;
	} 
	// generate square wave buffer
	std::vector<float> wavetableSquare(kWavetableSize);
	for (unsigned int n = 0; n < wavetableSquare.size(); n++) {
		unsigned int shiftPos = (n + (wavetableSquare.size() / 2)) % wavetableSquare.size();
		wavetableSquare[n] = wavetableSaw[n] - wavetableSaw[shiftPos];
	} 
	// normalise square wavetable
	max_elem = *std::max_element(wavetableSquare.begin(), wavetableSquare.end());
	for (unsigned int n = 0; n < wavetableSquare.size(); n++) {
		wavetableSquare[n] /= max_elem;
	} 	
	
	// initialise oscillator wavetables
	gBassOsc.setup(context->audioSampleRate, wavetableSaw, wavetableSquare, kWavetable2DSize, gBassDetuneVoices, gBassDetuneVoicePositions);
	gSubBassOsc.setup(context->audioSampleRate, wavetableSquare, gSubBassDetuneVoices, gSubBassDetuneVoicePositions);
	gLeadOsc.setup(context->audioSampleRate, wavetableSaw, wavetableSquare, kWavetable2DSize, gLeadDetuneVoices, gLeadDetuneVoicePositions);
	// setup wavetable poisition
	gLeadOsc.setTable(0.1);

	// initialise filters
	gBassFilt.setup(context->audioSampleRate, 1);
	gLeadFilt.setup(context->audioSampleRate, 1);

	// initialise the ADSR objects
	gBassAmpADSR.setSampleRate(context->audioSampleRate);
	gBassFiltADSR.setSampleRate(context->audioSampleRate);
	gLeadAmpADSR.setSampleRate(context->audioSampleRate);
	gLeadFiltADSR.setSampleRate(context->audioSampleRate);
	
	// initialise with default values when GUI is not used
	gLeadAmpADSR.setAttackTime(0.012);
	gLeadAmpADSR.setDecayTime(0.03);
	gLeadAmpADSR.setSustainLevel(0.03);
	gLeadFiltADSR.setAttackTime(0.006);
	gLeadFiltADSR.setDecayTime(0.0035);
	gLeadFiltADSR.setSustainLevel(0.2);
	
	// Initialise the MIDI device
	if(gMidi.readFrom(gMidiPort0) < 0) {
		rt_printf("Unable to read from MIDI port %s\n", gMidiPort0);
		return false;
	}
	gMidi.writeTo(gMidiPort0);
	gMidi.enableParser(true);	
	gMidi.setParserCallback(midiEvent, (void *)gMidiPort0);
	
	// set up MIDI Looper
	gBassLoop.setup(context->audioSampleRate, gTempo, kBeatsPerBar, kBarsPerPattern, kLooperMIDIRes, kNoMessage);
	
	// wipe LEDs on QuNeo
	for (unsigned int i = 0; i <= kLEDArp; i++) {
		// send MIDI out message with velocity 0
		int message = gMidi.writeNoteOn(0, i, 0);		// pads and play buttons
		gMidi.writeOutput(message);
	}
	int message = gMidi.writeNoteOff(0, kLEDLoop1, 0);			// looper button
	gMidi.writeOutput(message);
	message = gMidi.writeNoteOff(0, kLEDLoop2, 0);				// looper button
	gMidi.writeOutput(message);
	// turn off slider LEDs
	for (unsigned int i = kLEDSound1; i <= kLEDSound4; i++) {
		message = gMidi.writeControlChange(0, i, 0);
		gMidi.writeOutput(message);		
	}
	for (unsigned int i = kLEDTemperature4; i <= kLEDTemperature1; i++) {
		message = gMidi.writeControlChange(0, i, 0);
		gMidi.writeOutput(message);		
	}
	// light up tempo LED to starting tempo
	int tempoCC = map(gTempo, kMinTempo, kMaxTempo, 0, 127);
	message = gMidi.writeControlChange(0, kLEDTempo, tempoCC);
	gMidi.writeOutput(message);
	// set everall temperature LED
	int tempCC = map(gArpOverallTemperature, 0.0, 1.0, 0, 127);
	message = gMidi.writeControlChange(0, kLEDTempOverall, tempCC);
	gMidi.writeOutput(message);	
	// set arpeggiator seed balance LED
	int balanceCC = map(gArpSeedBalance, 0.0, 1.0, 0, 127);
	message = gMidi.writeControlChange(0, kLEDSeedBalance, balanceCC);
	gMidi.writeOutput(message);
	
	// Load the audio file
	if(!gPlayer.setup(gFilename, false, false)) {
    	rt_printf("Error loading audio file '%s'\n", gFilename.c_str());
    	return false;
	}
	// Print some useful info
    rt_printf("Loaded the audio file '%s' with %d frames (%.1f seconds)\n", 
    			gFilename.c_str(), gPlayer.size(),
    			gPlayer.size() / context->audioSampleRate);
    
    // set up the Probabilistic ArpSynth
    gArp.setMetre(ksubBeatsPerBeat, kBeatsPerBar, kBarsPerPattern);

	// Set up the GUI
	float table_inc = 1.0 / (float)(kWavetable2DSize - 1);
	gGui.setup(context->projectName);
	gGuiController.setup(&gGui, "ArpSynth Controller");	
	gGuiController.addSlider("Global Amplitude", 0.6, 0, 1.0, 0);
	gGuiController.addSlider("Bass Saw-Square mix", 0, 0, 1.0, table_inc);
	gGuiController.addSlider("Bass Pitch", 0, 0, 12, 1);
	gGuiController.addSlider("Bass Detune",  0.0002, 0, 0.01, 0);
	gGuiController.addSlider("Bass Amplitude", -9, -40, -6, 0);
	gGuiController.addSlider("Bass Filt cutoff", 1160, 100, 5000, 0);
	gGuiController.addSlider("Bass Filt Resonance", 0, 0, 1.1, 0);
	gGuiController.addSlider("Sub Bass Amplitude", -25, -40, -6, 0);
	gGuiController.addSlider("Lead Saw-Square mix", 0.1, 0, 1.0, table_inc);
	gGuiController.addSlider("Lead Pitch", 0, 0, 36, 1);
	gGuiController.addSlider("Lead Detune",  0.0004, 0, 0.01, 0);
	gGuiController.addSlider("Lead Amplitude", -0.1, -40, 0, 0);
	gGuiController.addSlider("Lead Filt cutoff", 2030, 100, 10000, 0);
	gGuiController.addSlider("Lead Filt sens", 2660, 0, 5000, 0);
	gGuiController.addSlider("Lead Filt Resonance", 0.18, 0, 1.1, 0);
	gGuiController.addSlider("Lead ADSR A", 0.012, 0, 0.1, 0);
	gGuiController.addSlider("Lead ADSR D", 0.03, 0, 0.1, 0);
	gGuiController.addSlider("Lead ADSR S", 0.03, 0, 1.0, 0);
	gGuiController.addSlider("Lead ADSR R", 0.01, 0, 0.1, 0);
	gGuiController.addSlider("Lead Filt ADSR A", 0.006, 0, 0.1, 0);
	gGuiController.addSlider("Lead Filt ADSR D", 0.0035, 0, 0.1, 0);
	gGuiController.addSlider("Lead Filt ADSR S", 0.2, 0, 1.0, 0);
	gGuiController.addSlider("Lead Filt ADSR R", 0.01, 0, 0.1, 0);
	gGuiController.addSlider("Kick Amplitude", 0.6, 0, 1.0, 0);
	gGuiController.addSlider("Tempo", 120.0, kMinTempo, kMaxTempo, 1);
	gGuiController.addSlider("Arp Seed1", gArpSeed1, 0, kArpNumSeeds - 1, 1);
	gGuiController.addSlider("Arp Seed2", gArpSeed2, 0, kArpNumSeeds - 1, 1);
	gGuiController.addSlider("Temperature Dist", gArpTempDist, 0, kArpNumTempDists - 1, 1);
	
	// Set up the oscilloscope
	gScope.setup(1, context->audioSampleRate);

	return true;
}


void render(BelaContext *context, void *userData)
{
	// read GUI slider values
	float globalAmplitude = gGuiController.getSliderValue(0);	
	// float bassWavetablePos = gGuiController.getSliderValue(1);
	// float bassPitch = gGuiController.getSliderValue(2);	
	// float bassDetune = gGuiController.getSliderValue(3);
	// float bassAmplitudeDB = gGuiController.getSliderValue(4);
	float bassFiltCutoff = gGuiController.getSliderValue(5);
	float bassFiltRes = gGuiController.getSliderValue(6);
	float subBassAmplitudeDB = gGuiController.getSliderValue(7);
	// float leadWavetablePos = gGuiController.getSliderValue(8);
	// float leadPitch = gGuiController.getSliderValue(9);
	float leadDetune = gGuiController.getSliderValue(10);
	// float leadAmplitudeDB = gGuiController.getSliderValue(11);
	// float leadFiltCutoff = gGuiController.getSliderValue(12);
	float leadFiltSensitivity = gGuiController.getSliderValue(13);
	float leadFiltRes = gGuiController.getSliderValue(14);
	// float leadADSRa = gGuiController.getSliderValue(15);
	// float leadADSRd = gGuiController.getSliderValue(16);
	// float leadADSRs = gGuiController.getSliderValue(17);
	float leadADSRr = gGuiController.getSliderValue(18);
	// float leadFiltADSRa = gGuiController.getSliderValue(19);
	// float leadFiltADSRd = gGuiController.getSliderValue(20);
	// float leadFiltADSRs = gGuiController.getSliderValue(21);
	float leadFiltADSRr = gGuiController.getSliderValue(22);
	float kickAmplitudeDB = gGuiController.getSliderValue(23);
	// float tempo = gGuiController.getSliderValue(24);
	unsigned int arpSeed1 = gGuiController.getSliderValue(25);
	unsigned int arpSeed2 = gGuiController.getSliderValue(26);
	unsigned int arpTempDist = gGuiController.getSliderValue(27);
	
	// set tempo
	// gTempo = tempo;

	// convert amplitudes to linear
	// float bassAmplitude = powf(10.0, bassAmplitudeDB / 20.0);

	// Set the bass oscillator parameters
	// float bassCentreFreq = 65.41 * powf(2.0, bassPitch / 12.0);		// convert semitones (above C2) to Hertz
	// gBassOsc.setTable(bassWavetablePos);
	// gBassOsc.setFrequency(bassCentreFreq);
	// gBassOsc.setDetune(bassDetune);
	
	// kick
	gKickAmp = powf(10.0, kickAmplitudeDB / 20.0) * 2.0;
	
	// set the sub bass parameters
	gSubBassAmp = powf(10.0, subBassAmplitudeDB / 20.0);
	
	// set the lead parameters
	// gLeadNote = leadPitch;		// convert semitones (above C2) to Hertz
	// gLeadOsc.setTable(leadWavetablePos);
	gLeadOsc.setDetune(leadDetune);
	// convert amplitudes to linear
	// gLeadAmp = powf(10.0, leadAmplitudeDB / 20.0);
	
	// calculate filter coefficients
	gBassFilt.set_params(bassFiltCutoff, bassFiltRes);
	gLeadFilt.set_params(gLeadFiltCutoff, leadFiltRes);

	// set ADSR parameters
	gBassAmpADSR.setAttackTime(0.01);
	gBassAmpADSR.setDecayTime(0.005);
	gBassAmpADSR.setSustainLevel(0.8);
	gBassAmpADSR.setReleaseTime(0.005);
	
	gBassFiltADSR.setAttackTime(0.006);
	gBassFiltADSR.setDecayTime(0.1);
	gBassFiltADSR.setSustainLevel(0.6);
	gBassFiltADSR.setReleaseTime(0.3);
	
	// gLeadAmpADSR.setAttackTime(leadADSRa);
	// gLeadAmpADSR.setDecayTime(leadADSRd);
	// gLeadAmpADSR.setSustainLevel(leadADSRs);
	gLeadAmpADSR.setReleaseTime(leadADSRr);
	
	// gLeadFiltADSR.setAttackTime(leadFiltADSRa);
	// gLeadFiltADSR.setDecayTime(leadFiltADSRd);
	// gLeadFiltADSR.setSustainLevel(leadFiltADSRs);
	gLeadFiltADSR.setReleaseTime(leadFiltADSRr);
	
	// update arpeggiator seeds
	gArp.setSeed(arpSeed1, arpSeed2);
	
	// set arpeggiator temperature distribution choice
	gArp.setTempDistChoice(arpTempDist);
	

	// Audio loop
    for(unsigned int n = 0; n < context->audioFrames; n++) {
    	float out = 0;
    	
    	// process bass loop
    	gBassLoop.process();
    	// read from bass loop
    	if (gPlayBass && gBassLoopRead && !gBassLoopReadOverride) {
    		gLoopNoteReadMessage = gBassLoop.read();
    		
			// rt_printf("Message from looper: {%d, %d, %d, %d}\n", gLoopNoteReadMessage[0], gLoopNoteReadMessage[1], gLoopNoteReadMessage[2], gLoopNoteReadMessage[3]);
    		
      		if (gLoopNoteReadMessage[2] != kNoMessage[2]) {
    			gArp.modeChange(gLoopNoteReadMessage[2]);
    		}
    		if (gLoopNoteReadMessage[3] != kNoMessage[3]) {
    			controlChange(kMIDIControllerLED, gLoopNoteReadMessage[3]);
    		}
    		if (gLoopNoteReadMessage[0] != kNoMessage[0]) {
    			// apply note on function
    			noteOn(gLoopNoteReadMessage[0], gLoopNoteReadMessage[1]);
    			// also apply note off
    			noteOff(gLoopNoteReadMessage[0]);
    		}
    	}
    	
    	// get bass sample value from wavetable
    	float bassADSR = gBassAmpADSR.process();
    	float bassAmp = gBassAmp * bassADSR;
    	float bassOut = gBassOsc.process() * bassAmp;
    	// play sub bassOut
    	float subBassAmp = gSubBassAmp * bassADSR;    
    	bassOut += gSubBassOsc.process() * subBassAmp;
    	// apply filter
    	float bassfiltercontrol = gBassFiltADSR.process();
    	gBassFilt.set_params(bassFiltCutoff + bassfiltercontrol * 400, bassFiltRes);
    	bassOut = gBassFilt.process(bassOut);
    	
    	// play lead
    	// play next event
		if (++gCounter >= 1.0 / (float)gTempo * 60.0 * context->audioSampleRate / (float)kBeatsPerBar) {
			nextEvent();

			// reset counter
			gCounter = 0;
		}
		
		// get lead output
		float leadAmp = std::get<1>(gLeadNoteAmp) * gLeadAmpADSR.process();		// output amplitude from the arpeggiator * envelope value
		float leadOut = gLeadOsc.process() * leadAmp;
		// apply filter
    	float leadfiltercontrol = gLeadFiltADSR.process();
    	gLeadFilt.set_params(gLeadFiltCutoff + leadfiltercontrol * leadFiltSensitivity, leadFiltRes);
    	leadOut = gLeadFilt.process(leadOut);
    	
    	// add kick
    	float kick = 0;
		if (gPlayKick) {
			kick = gPlayer.process() * gKickAmp * gKickAmpRed;
		}
		    	
    	// set audio output
    	out = bassOut + leadOut + kick / 2.0;
    	out *= globalAmplitude;
    	
		// Write the sample to every audio output channel            
    	for(unsigned int channel = 0; channel < context->audioOutChannels; channel++) {
    		audioWrite(context, n, channel, out);
    	}
    	
    	// Log the audio output and the envelope to the scope
    	gScope.log(out);    	
    }
}


void midiEvent(MidiChannelMessage message, void *arg) {
	// Display the port, if available
	if(arg != NULL) {
		rt_printf("Message from midi port %s ", (const char*) arg);
	}
	
	// Display the message
	message.prettyPrint();
		
	// A MIDI "note on" message type might actually hold a real
	// note onset (e.g. key press), or it might hold a note off (key release).
	// The latter is signified by a velocity of 0.
	if(message.getType() == kmmNoteOn) {
		int noteNumber = message.getDataByte(0);
		int velocity = message.getDataByte(1);
		
		// Velocity of 0 is really a note off
		if(velocity != 0) {
			noteOn(noteNumber, velocity);
		}
		else {
			
		}
	}
	else if(message.getType() == kmmNoteOff) {
		// We can also encounter the "note off" message type which is the same
		// as "note on" with a velocity of 0.
		int noteNumber = message.getDataByte(0);
		
		noteOff(noteNumber);
	}
	// else if(message.getType() == kmmPitchBend) {
	// 	int lsb = message.getDataByte(0);
	// 	int msb = message.getDataByte(1);
	
	// 	pitchWheel(lsb, msb);
	// }
	else if(message.getType() == kmmControlChange) {
		int controller = message.getDataByte(0);
		int value = message.getDataByte(1);
		
		controlChange(controller, value);
	}
}


// MIDI note on received
void noteOn(int noteNumber, int velocity) 
{
	// set playing flag
	if (!gPlayBass) {
		gPlayBass = 1;
		
		// send an LED on message to QuNeo
		int message = gMidi.writeNoteOn(0, 34, 127);
		gMidi.writeOutput(message);
		
		rt_printf("Playing Audio\n");
	}
	
	rt_printf("Note on message received: %d\n", noteNumber);
	
	// Map note number to frequency
	float bassCentreFreq = 65.41 * powf(2.0, (noteNumber - 36) / 12.0);
	gBassOsc.setFrequency(bassCentreFreq);
	
	// set sub bass oscillator
	gSubBassOsc.setFrequency(bassCentreFreq / 2.0);
	
	// Map velocity to amplitude on a decibel scale
	// float decibels = map(velocity, 1, 127, -40, 0);
	// gBassAmp = powf(10.0, decibels / 20.0);

	// trigger envelopes
	gBassAmpADSR.trigger();
	gBassFiltADSR.trigger();
	
	// set bass note
	gBassNote = noteNumber;
	
	// set key 
	gArp.keyChange(noteNumber % 12);
	
	// write message to bass Looper
	gLoopNoteWriteMessage[0] = noteNumber;
	gLoopNoteWriteMessage[1] = velocity;
	gLoopNoteWriteMessage[2] = gArp.getMode();
	gBassLoop.write(gLoopNoteWriteMessage);
	
	rt_printf("Wrote message to looper: {%d, %d, %d, %d}\n", gLoopNoteWriteMessage[0], gLoopNoteWriteMessage[1], gLoopNoteWriteMessage[2], gLoopNoteWriteMessage[3]);
	
	// set bass loop read flag to false while note is pressed (to allow overwriting of notes in the loop)
	gBassLoopReadOverride = true;
	// set bass loop to overwrite mode
	gBassLoop.setOverwrite(true);
}


// MIDI note off received
void noteOff(int noteNumber)
{
	rt_printf("Note off message received: %d\n", noteNumber);
	
	// remove bass looper read override
	gBassLoopReadOverride = false;
	// remove bass loop overwrite if we are reading from the loop
	if (gBassLoopRead) {
		gBassLoop.setOverwrite(false);
	}	
}

// Handle control change messages
void controlChange(int controller, int value)
{
	if(controller == kMIDIControllerKick) { 
		// rt_printf("Kick Play CC received, value %d\n", value);
		
		if (value > 0) {
			if (!gPlayKick) {
				gPlayKick = 1;
	
				// send an LED on message to QuNeo
				int message = gMidi.writeNoteOn(0, kLEDKick, 127);
				gMidi.writeOutput(message);
			}
			else {
				gPlayKick = 0;
	
				// send an LED off message to QuNeo
				int message = gMidi.writeNoteOff(0, kLEDKick, 0);
				gMidi.writeOutput(message);
			}
		}
	}
	else if(controller == kMIDIControllerBass) { 
		// rt_printf("Bass Play CC received, value %d\n", value);
		
		if (value > 0) {
			if (!gPlayBass) {
				gPlayBass = 1;
				
				// send an LED on message to QuNeo
				int message = gMidi.writeNoteOn(0, kLEDBass, 127);
				gMidi.writeOutput(message);
			}
			else {
				gBassAmpADSR.release();
				gBassFiltADSR.release();
				
				gPlayBass = 0;
				
				// send an LED off message to QuNeo
				int message = gMidi.writeNoteOff(0, kLEDBass, 0);
				gMidi.writeOutput(message);
			}
		}
	}
	else if(controller == kMIDIControllerLead) { 
		// rt_printf("Lead Play CC received, value %d\n", value);
		
		if (value > 0) {
			if (!gArp.isPlaying()) {
				gArp.play();
	
				// send an LED on message to QuNeo
				int message = gMidi.writeNoteOn(0, kLEDArp, 127);
				gMidi.writeOutput(message);
			}
			else {
				// stop playing Arpeggiator
				gArp.stop();
				
				// reset previous sequence buffer to the seed sequence
				gArp.resetToSeed();
				
				gLeadAmpADSR.release();
				gLeadFiltADSR.release();
	
				// send an LED off message to QuNeo
				int message = gMidi.writeNoteOff(0, kLEDArp, 0);
				gMidi.writeOutput(message);
			}
		}
	}
	else if(controller == kMIDIControllerLoop) { 
		// rt_printf("Lead Play CC received, value %d\n", value);
		
		if (value > 0) {
			if (gBassLoopRead) {
				gBassLoopRead = false;
				// set loop to overwrite each cycle
				gBassLoop.setOverwrite(true);			
	
				// send an LED off message to QuNeo
				int message = gMidi.writeNoteOff(0, kLEDLoop1, 0);
				gMidi.writeOutput(message);
				message = gMidi.writeNoteOff(0, kLEDLoop2, 0);
				gMidi.writeOutput(message);
			}
			else {
				gBassLoopRead = true;
				// stop loop overwriting
				gBassLoop.setOverwrite(false);			
	
				// send an LED on message to QuNeo
				int message = gMidi.writeNoteOn(0, kLEDLoop1, 127);
				gMidi.writeOutput(message);
				message = gMidi.writeNoteOn(0, kLEDLoop2, 127);
				gMidi.writeOutput(message);
			}
		}
	}
	else if(controller == kMIDIControllerTempo) { 
		// map value to tempo
		float tempo = map(value, 0, 127, kMinTempo, kMaxTempo);
		// snap to integer
		if (gIntTempo) {
			tempo = std::round(tempo);
		}
		// change global tempo
		gTempo = tempo;
		// send to MIDI Looper
		gBassLoop.setTempo(tempo);
		
		rt_printf("Tempo changed to %f\n", tempo);
	}
	else if(controller == kMIDIControllerContourTemp) {
		// map value to [0, 1]
		float temp = map(value, 0, 127, 0.0, 1.0);

		// update Arpeggiator control
		gArp.setContourTemp(temp);
		
		// adjust overall temp LED
		overallTempLEDMidiMessage();
		
		rt_printf("Arpeggiator contour temperature set to %f\n", temp);
	}	
	else if(controller == kMIDIControllerHarmonicTemp) {
		// map value to [0, 1]
		float temp = map(value, 0, 127, 0.0, 1.0);
		
		// update Arpeggiator control
		gArp.setHarmonicTemp(temp);
		
		// adjust overall temp LED
		overallTempLEDMidiMessage();
		
		rt_printf("Arpeggiator harmonic temperature set to %f\n", temp);
	}
	else if(controller == kMIDIControllerRhythmicTemp) {
		// map value to [0, 1]
		float temp = map(value, 0, 127, 0.0, 1.0);
		
		// update Arpeggiator control
		gArp.setRhythmicTemp(temp);
		
		// adjust overall temp LED
		overallTempLEDMidiMessage();
			
		rt_printf("Arpeggiator rhythmic temperature set to %f\n", temp);
	}
	else if(controller == kMIDIControllerDynamicContourTemp) {
		// map value to [0, 1]
		float temp = map(value, 0, 127, 0.0, 1.0);
		
		// update Arpeggiator control
		gArp.setDynamicContourTemp(temp);
		
		// adjust overall temp LED
		overallTempLEDMidiMessage();
			
		rt_printf("Arpeggiator dynamic contour temperature set to %f\n", temp);
	}
	else if(controller == kMIDIControllerIntervalTemp) {
		// map value to [0, 1]
		float temp = map(value, 0, 127, 0.0, 1.0);
		
		// update Arpeggiator control
		gArp.setIntervalTemp(temp);
		
		// adjust overall temp LED
		overallTempLEDMidiMessage();
				
		rt_printf("Arpeggiator interval temperature set to %f\n", temp);
	}
	else if(controller == kMIDIControllerSparsity) {
		// map value to [0, 1]
		float temp = map(value, 0, 127, 0.0, 1.0);
		
		// update Arpeggiator control
		gArp.setSparsity(temp);
		
		// adjust overall temp LED
		overallTempLEDMidiMessage();
				
		rt_printf("Arpeggiator sparsity set to %f\n", temp);
	}
	else if(controller == kMIDIControllerMovement) {
		// map value to [0, 1]
		float temp = map(value, 0, 127, 0.0, 1.0);
		
		// update Arpeggiator control
		gArp.setMovement(temp);
		
		// adjust overall temp LED
		overallTempLEDMidiMessage();
				
		rt_printf("Arpeggiator movement set to %f\n", temp);
	}
	else if(controller == kMIDIControllerDynamicTemp) {
		// map value to [0, 1]
		float temp = map(value, 0, 127, 0.0, 1.0);
		
		// update Arpeggiator control
		gArp.setDynamicTemp(temp);
		
		// adjust overall temp LED
		overallTempLEDMidiMessage();
				
		rt_printf("Arpeggiator dynamic temperature set to %f\n", temp);
	}
	else if(controller == kMIDIControllerConsistency) {
		// map value to [0, 1]
		float temp = map(value, 0, 127, 0.0, 1.0);
		
		// update Arpeggiator control
		gArp.setConsistency(temp);
		
		// adjust overall temp LED
		overallTempLEDMidiMessage();
				
		rt_printf("Arpeggiator consistency set to %f\n", temp);
	}
	else if(controller == kMIDIControllerPitchTemp) {
		// map value to [0, 1]
		float temp = map(value, 0, 127, 0.0, 1.0);
		
		// update Arpeggiator control
		gArp.setPitchTemp(temp);
		
		// adjust overall temp LED
		overallTempLEDMidiMessage();
				
		rt_printf("Arpeggiator pitch temperature set to %f\n", temp);
	}
	else if(controller == kMIDIControllerArpSeedBalance) {
		// map value to [0, 1]
		float balance = map(value, 0, 127, 0.0, 1.0);
		
		// update Arpeggiator control
		gArp.setSeedBalance(balance);
		
		// send an LED message to QuNeo
		int message = gMidi.writeControlChange(0, kLEDSeedBalance, value);
		gMidi.writeOutput(message);		// update LEDs
				
		rt_printf("Arpeggiator seed balance set to %f\n", balance);
	}
	else if(controller == kMIDIControllerArpOverallTemperature) {
		// map value to [0, 1]
		float position = map(value, 0, 127, 0.0, 1.0);

		float proportion = 0;
		if (position > gArpOverallTemperature) {
			// get proportional increase relative to the maximum possible
			proportion = (position - gArpOverallTemperature) / (1 - gArpOverallTemperature);	
		}
		else if (position < gArpOverallTemperature) {
			proportion = (position - gArpOverallTemperature) / (gArpOverallTemperature);
		}

		// update Arpeggiator controls by the change in ratio
		// Note: a negative value results in a decrease by that proportion
		gArp.changeAllTempsByProportion(proportion);
		
		// send an LED message to QuNeo
		int message = gMidi.writeControlChange(0, kLEDTempOverall, value);
		gMidi.writeOutput(message);		// update LEDs		
		
		rt_printf("Arpeggiator temps proportional change of %f\n", proportion);
	}
	else if(controller == kMIDIControllerBassAmp) {
		float decibels = map(value, 0, 127, -40, 0);
		gBassAmp = powf(10.0, decibels / 20.0);	
		
		// rt_printf("Set Bass amplitude to %f\n", gBassAmp);
	}	
	else if(controller == kMIDIControllerBassTablePos) {
		float bassWavetablePos = map(value, 0, 127, 0.0, 1.0);
		gBassOsc.setTable(bassWavetablePos);
		
		// rt_printf("Set Bass Wavetable Position to %f\n", bassWavetablePos);
	}
	else if(controller == kMIDIControllerBassDetune) {
		float bassDetune = map(value, 0, 127, 0, 0.01);
		gBassOsc.setDetune(bassDetune);
		
		// rt_printf("Set Bass detune to %f\n", bassDetune);
	}
	else if(controller == kMIDIControllerLeadWavetableMix) {
		float mix = map(value, 0, 127, 0, 1.0);
		gLeadOsc.setTable(mix);
	}
	else if(controller == kMIDIControllerLeadADSRa) {
		float val = map(value, 0, 127, 0, 0.1);
		gLeadAmpADSR.setAttackTime(val);
	}
	else if(controller == kMIDIControllerLeadADSRd) {
		float val = map(value, 0, 127, 0, 0.1);
		gLeadAmpADSR.setDecayTime(val);
	}
	else if(controller == kMIDIControllerLeadADSRs) {
		float val = map(value, 0, 127, 0, 1.0);
		gLeadAmpADSR.setSustainLevel(val);
	}
	else if(controller == kMIDIControllerLeadFiltCutoff) {
		gLeadFiltCutoff = map(value, 0, 127, 1000, 10000);
	}
	else if(controller == kMIDIControllerLeadFiltADSRa) {
		float val = map(value, 0, 127, 0, 0.1);
		gLeadFiltADSR.setAttackTime(val);
	}
	else if(controller == kMIDIControllerLeadFiltADSRd) {
		float val = map(value, 0, 127, 0, 0.1);
		gLeadFiltADSR.setDecayTime(val);
	}
	else if(controller == kMIDIControllerLeadFiltADSRs) {
		float val = map(value, 0, 127, 0, 1.0);
		gLeadFiltADSR.setSustainLevel(val);
	}
	else if(controller == kMIDIControllerMode) { 
		// left side pad hits give mode 0 (major key)
		unsigned int mode = 0;
		if (value >= 64) {
			// right side pad hits give mode 1 (minor key)
			mode = 1;
		}
		
		// update Arpeggiator
		gArp.modeChange(mode);
		
		// rt_printf("Mode changed to %d\n", mode);
	}
	else if(controller == kMIDIControllerLED) {
		// rt_printf("LED base value: %d\n", value);

		// get correct notes
		if (value > 0) {
			// turn off previous LED
			int message1 = gMidi.writeNoteOff(1, gBassLED1, 0);
			int message2 = gMidi.writeNoteOff(1, gBassLED2, 0);
			gMidi.writeOutput(message1);
			gMidi.writeOutput(message2);
			
			gBassLED1 = value;		// this corresponds to the positions of NoteOn values for the controller pads (bottom right LEDs)
			gBassLED2 = value + 16;
			int mode = gArp.getMode();
			if (mode == 0) {
				gBassLED1 -= 2;
				gBassLED2 -= 2;
			}
			message1 = gMidi.writeNoteOn(1, gBassLED1, 127);
			message2 = gMidi.writeNoteOn(1, gBassLED2, 127);
			gMidi.writeOutput(message1);
			gMidi.writeOutput(message2);
			
			// write LED value to MIDI looper
			gLoopNoteWriteMessage[3] = value;
		}
	}
}


void nextEvent() {
	
	// move on the metre counter
	gArp.beat();
	
	// Map velocity to amplitude on a decibel scale
	// float decibels = map(velocity, 1, 127, -40, 0);
	// gBassAmp = powf(10.0, decibels / 20.0);

	if (gArp.isPlaying()) {
		// get note and amplitude pair
		gLeadNoteAmp = gArp.generate();	
		// check for a 'no note'
		if (std::get<0>(gLeadNoteAmp) != -1) {
			// set lead oscillator frequency
			float leadCentreFreq = 130.81 * powf(2.0, (std::get<0>(gLeadNoteAmp) - 48) / 12.0);
			gLeadOsc.setFrequency(leadCentreFreq);
			
			// trigger envelopes
			gLeadAmpADSR.trigger();
			gLeadFiltADSR.trigger();
		}
	}
	
	int beat = gArp.getSequencePosition();
	
	// add kick
	if (beat % ksubBeatsPerBeat == 0) {
		if (gPlayKick) {
			gPlayer.trigger();	
			
			// alter amplitude depending on metrical beat position
			if (beat % (ksubBeatsPerBeat * kBeatsPerBar) == 0) {
				gKickAmpRed = 1.0;
			}
			else if (beat % (ksubBeatsPerBeat * kBeatsPerBar / 2) == 0) {
				gKickAmpRed = (1.0 + 2.0 * kKickAmpRedRatio) / 3.0;
			}
			else {
				gKickAmpRed = kKickAmpRedRatio;
			}
		}
		
		// unlight previous beat LED
		int message = gMidi.writeNoteOff(0, ((16 + (beat / 4 - 1)) % 16) * 2, 0);
		gMidi.writeOutput(message);
		// light pad LED on beats
		message = gMidi.writeNoteOn(0, beat / 2, 127);
		gMidi.writeOutput(message);
	}
}


// utility function to send an LED message to the Overall temerature slider
void overallTempLEDMidiMessage()
{
	// get overall temperature proportion
	float temp = gArp.getOverallTemp();
	// send an LED message to QuNeo
	int tempCC = map(temp, 0.0, 1.0, 0, 127);
	int message = gMidi.writeControlChange(0, kLEDTempOverall, tempCC);
	gMidi.writeOutput(message);
}


void cleanup(BelaContext *context, void *userData)
{

}
