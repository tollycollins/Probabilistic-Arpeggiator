/***** ProbabilisticArp.h *****/

#pragma once

#include <vector>
#include <utility>
#include <random>
#include <stdint.h>

class ProbabilisticArp {
public:
	ProbabilisticArp(unsigned int subBeatsPerBeat = 4,			// constructor
					 unsigned int beatsPerBar = 4, 
					 unsigned int barsPerPattern = 4, 
					 unsigned int lowestNote = 36, 
					 unsigned int octaves = 5, 
					 int seed1 = -1,
					 int seed2 = -1, 
					 float seedBalance = 0, 
					 unsigned int tempDist = 0);
	
	void setMetre(unsigned int subBeatsPerBeat, unsigned int beatsPerBar, unsigned int barsPerPattern);		// define the metre
	
	void keyChange(unsigned int key);							// change the base key
	void modeChange (unsigned int mode);						// 0: major; 1: minor
	
	unsigned int getKey() const;
	unsigned int getMode() const;
	
	void setSequencePosition(unsigned int position);			// set position in metre (int)
	unsigned int getSequencePosition() const;					// get metrical position (int)
	
	// set the temperature controls (see attributes below for descriptions)
	void setIntervalTemp(float intervalTemp);
	void setContourTemp(float contourTemp);
	void setRhythmicTemp(float rhythmicTemp);
	void setSparsity(float sparsity);
	void setConsistency(float consistency);
	void setMovement(float movement);
	void setHarmonicTemp(float harmonicTemp);
	void setDynamicTemp(float dynamicTemp);
	void setDynamicContourTemp(float dynamicContourTemp);
	void setPitchTemp(float pitchTemp);
	
	// allows all temperatures to be altered by the same proportional increase or decrease
	// positive factor for increase, negative factor for decrease
	void changeAllTempsByProportion(float proportion);
	
	float getIntervalTemp();
	float getContourTemp();
	float getRhythmicTemp();
	float getSparsity();
	float getConsistency();
	float getMovement();
	float getHarmonicTemp();
	float getDynamicTemp();
	float getDynamicContourTemp();
	float getPitchTemp();
	
	// get the overall proportion of the sum of the temperature values compared to the total possible
	float getOverallTemp();
	
	void beat();			// move one the metrical position by 1
	void play();			// set isPlaying flag to true
	void stop();			// set isPlaying flag to false
	bool isPlaying();		// flag for whether or not to generate a new note
	
	std::pair<int, float> generate();		// generate a new note probabilistically, based on temperature controls
	
	void setSeed(int seed1 = -1, int seed2 = -1);				// set starting sequence seed
	std::vector<int> getSeeds();								// get the index numbers of the current seeds
	void setSeedBalance(float balance);							// change the interpolation weighting between the 2 seeds
	float getSeedBalance();										// return the interpolation ratio between the 2 seed sequences
	unsigned int numSeeds();									// return the number of available seed seqeunces
	void resetToSeed();											// resets previous sequence to match seed
	
	void setTempDistChoice(unsigned int choice = 0);			// set the choice for temperature distribution (distributions for note chroma)
	unsigned int getNumTempDists();								// return the number of choices for temperature distributions
	
	~ProbabilisticArp() = default;								// destructor
	
private:
	bool isPlaying_;

	// define metre
	unsigned int subBeatsPerBeat_;
	unsigned int beatsPerBar_;
	unsigned int barsPerPattern_;
	
	// pointer for the sequence buffer
	int pointer_;
	
	unsigned int key_;				// Underlying harmony
	// unsigned int prevKey_;
	unsigned int mode_;				// 0 major, 1 minor
	// unsigned int prevMode_;
	
	std::vector<std::pair<int, float>> prevSequence_;		// circular buffer for sequence
	std::pair<int, float> prevNote_;						// holds previous note
	
	unsigned int lowestNote_;								// MIDI pitch of lowest permitted note output - should be multiple of 12 [in the C chroma class]
	unsigned int octaves_;									// number of octaves above lowest note in range of possible output notes
	
	// random number generator
	// https://en.wikipedia.org/wiki/Xorshift
	// random number
	// struct xorshift64_state {
	//   uint64_t a;
	// };
	// // generate a new number
	// uint64_t xorshift64(struct xorshift64_state *state);
	
	// random number
	std::mt19937 rng_;
	
	// convert to float in the uniform distributions
	std::uniform_real_distribution<float> uniform_;
	// distribution for picking seeds a random
	std::uniform_int_distribution<int> seedDist_;
	
	// function to sample from a non-normalised distribution using the uniform distribution
	unsigned int sampleFrom(std::vector<float>& distribution);
	
	// determine the degree of randomness and unexpectedness in the generative process
	float pitchTemp_;				// the degree to which the pitch can vary from the seed pitch at that sequence position
	float intervalTemp_;			// how far the generative process can stray (in terms of interval) from the previous sequence / note
	float contourTemp_;				// how closely the generation follows the contour of the seed sequence
	float rhythmicTemp_;			// degree of rhythmic freedom allowed relative to preceeding sequence (how closely the rhythm should match previous rhythm)
	float sparsity_;				// affects the probability of a new note sounding at this metrical position (as opposed to no note sounding)
	float consistency_;				// degree of contour freedom allowed relative to sequence from preceeding pattern 
	float movement_;				// degree of variation allowed from previous note (low value will result in more repeated notes and smaller intervals)
	float harmonicTemp_;			// degree of harmonic freedom allowed (degree to which unexpected notes within a given key are allowed)
	float dynamicTemp_;				// how much the dynamics can vary by for each generated note
	float dynamicContourTemp_;		// how far the dynamics can stray from the seed seqeunce
	
	
	// earlier notes in the vector are more 'harmonically expected'
	// First row Major key, second row minor key
	// all notes relative to the harmonic root (0)
	std::vector<std::vector<int>> notes_ {{7, 0, 4, -1, 9, 2, 11, 5, 8, 1, 3, 10, 6}, 
										  {7, 0, 3, -1, 9, 2, 10, 5, 8, 1, 6, 11, 4}};
	
	// initial low- and high- temp distributions
	std::vector<std::vector<int>> lowTempDists_ {{8, 8, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
												 {12, 12, 12, 0, 8, 8, 2, 0, 0, 0, 0, 2, 0}, 
												 {12, 12, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
	std::vector<std::vector<int>> highTempDists_ {{8, 7, 6, 0, 4, 3, 2, 1, 0, 0, 0, 0, 0}, 
												  {10, 8, 10, 0, 8, 8, 4, 2, 2, 2, 1, 4, 1}, 
												  {8, 8, 8, 2, 12, 12, 8, 7, 6, 5, 4, 3, 2}};
												  
	int tempDistChoice_;
	std::vector<float> startDistribution_;			// holds the start point for calculating the distribution for note chroma choice (an interpolation between the 'low' and 'high' options)
	std::vector<float> distribution_;				// holds the distribution weightings for note chroma choice
	
	// std::vector<float> distWeightings_;
	
	std::vector<float> noteOptions_;				// holds distribution weightings for octave options
	
	// enum constants for tracking contour relative to seed sequence
	enum class Contour {
		noNote,
		negative,
		positive,
		repeatedNote
	};
	
	float seedBalance_;								// interpolation ratio between two chosen seeds
	int seed1num_;									// index (in seedSequences_) of first seed
	int seed2num_;									// index (in seedSequences_) of second seed
	std::vector<std::pair<int, float>> seed_;		// holds the current interpolation between the two chosen seed sequences
	
	std::vector<std::vector<std::pair<int, float>>> seedSequences_ {{{48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, 
																	 {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, 
																	 {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, 
																	 {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}, {48, 1.0}}, 
																	{{48, 1.0}, {51, 0.9}, {55, 0.9}, {48, 0.9}, {51, 1.0}, {55, 0.9}, {48, 0.9}, {51, 0.9}, {55, 1.0}, {48, 0.9}, {51, 0.9}, {55, 0.9}, {48, 1.0}, {51, 0.9}, {55, 0.9}, {48, 0.9}, 
																	 {51, 1.0}, {55, 0.9}, {48, 0.9}, {51, 0.9}, {55, 1.0}, {48, 0.9}, {51, 0.9}, {55, 0.9}, {48, 1.0}, {51, 0.9}, {55, 0.9}, {48, 0.9}, {51, 1.0}, {55, 0.9}, {48, 0.9}, {51, 0.9}, 
																	 {48, 1.0}, {51, 0.9}, {55, 0.9}, {48, 0.9}, {51, 1.0}, {55, 0.9}, {48, 0.9}, {51, 0.9}, {55, 1.0}, {48, 0.9}, {51, 0.9}, {55, 0.9}, {48, 1.0}, {51, 0.9}, {55, 0.9}, {48, 0.9}, 
																	 {51, 1.0}, {55, 0.9}, {48, 0.9}, {51, 0.9}, {55, 1.0}, {48, 0.9}, {51, 0.9}, {55, 0.9}, {48, 1.0}, {51, 0.9}, {55, 0.9}, {48, 0.9}, {51, 1.0}, {55, 0.9}, {48, 0.9}, {51, 0.9}}, 
																	{{48, 1.0}, {51, 0.9}, {55, 0.9}, {60, 0.9}, {62, 1.0}, {63, 0.9}, {67, 0.9}, {72, 0.9}, {74, 1.0}, {75, 0.9}, {79, 0.9}, {84, 0.9}, {86, 1.0}, {87, 0.9}, {93, 0.9}, {91, 0.9}, 
																	 {48, 1.0}, {51, 0.9}, {55, 0.9}, {60, 0.9}, {62, 1.0}, {63, 0.9}, {67, 0.9}, {72, 0.9}, {74, 1.0}, {75, 0.9}, {79, 0.9}, {84, 0.9}, {86, 1.0}, {87, 0.9}, {93, 0.9}, {91, 0.9}, 
																	 {48, 1.0}, {51, 0.9}, {55, 0.9}, {60, 0.9}, {62, 1.0}, {63, 0.9}, {67, 0.9}, {72, 0.9}, {74, 1.0}, {75, 0.9}, {79, 0.9}, {84, 0.9}, {86, 1.0}, {87, 0.9}, {93, 0.9}, {91, 0.9}, 
																	 {48, 1.0}, {51, 0.9}, {55, 0.9}, {60, 0.9}, {62, 1.0}, {63, 0.9}, {67, 0.9}, {72, 0.9}, {74, 1.0}, {75, 0.9}, {79, 0.9}, {84, 0.9}, {86, 1.0}, {87, 0.9}, {93, 0.9}, {91, 0.9}},
																	{{60, 1.0}, {72, 1.0}, {-1, 0.4}, {60, 1.0}, {72, 1.0}, {-1, 0.4}, {60, 1.0}, {72, 1.2}, {-1, 0.4}, {60, 1.0}, {72, 1.0}, {-1, 0.4}, {60, 1.0}, {72, 1.0}, {-1, 0.4}, {-1, 0.4}, 
																	 {60, 1.0}, {63, 1.0}, {-1, 0.4}, {60, 1.0}, {63, 1.0}, {-1, 0.4}, {60, 1.0}, {63, 1.2}, {-1, 0.4}, {60, 1.0}, {63, 1.0}, {-1, 0.4}, {60, 1.0}, {63, 1.0}, {-1, 0.4}, {-1, 0.4}, 
																	 {60, 1.0}, {67, 1.0}, {-1, 0.4}, {60, 1.0}, {67, 1.0}, {-1, 0.4}, {60, 1.0}, {67, 1.2}, {-1, 0.4}, {60, 1.0}, {67, 1.0}, {-1, 0.4}, {60, 1.0}, {67, 1.0}, {-1, 0.4}, {-1, 0.4}, 
																	 {60, 1.0}, {67, 1.0}, {-1, 0.4}, {60, 1.0}, {67, 1.0}, {-1, 0.4}, {60, 1.0}, {67, 1.2}, {-1, 0.4}, {60, 1.0}, {67, 1.0}, {-1, 0.4}, {60, 1.0}, {67, 1.0}, {-1, 0.4}, {-1, 0.4}}, 
	};
};

