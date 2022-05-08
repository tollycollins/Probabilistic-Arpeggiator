/***** ProbabilisticArp.cpp *****/

#include "ProbabilisticArp.h"

#include <vector>
#include <utility>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <cmath>
#include <stdexcept>


ProbabilisticArp::ProbabilisticArp(unsigned int subBeatsPerBeat, unsigned int beatsPerBar, unsigned int barsPerPattern, 		// constructor
								   unsigned int lowestNote, unsigned int octaves, int seed1, int seed2, float seedBalance, 
								   unsigned int tempDist)		
	: subBeatsPerBeat_(subBeatsPerBeat), beatsPerBar_(beatsPerBar), barsPerPattern_(barsPerPattern), 
	  lowestNote_(lowestNote), octaves_(octaves)
{
	// flag for generation of new notes
	isPlaying_ = false;
	
	pointer_ = -1;
	// holds current bass key
	key_ = 0;
	// major = 0, minor = 1
	mode_ = 0;
	
	// temperature controls from randomness behaviour
	harmonicTemp_ = 0;
	intervalTemp_ = 0;
	rhythmicTemp_ = 0;
	sparsity_ = 0;
	contourTemp_ = 0;
	movement_ = 0;
	dynamicTemp_ = 0;
	dynamicContourTemp_ = 0;
	consistency_ = 0;
	pitchTemp_ = 0;

	// distribution for possible next note relative to key
	distribution_ = std::vector<float>(lowTempDists_[0].size(), 0);
	startDistribution_ = std::vector<float>(lowTempDists_[0].size(), 0);	
	// set tempDistChoice_
	setTempDistChoice(tempDist);
	// distribution for next note among octave equivalents
	noteOptions_ = std::vector<float>(octaves, 0);
	
	// RNG
	std::random_device rd;
	rng_ = std::mt19937(rd());
	// uniform distribution
	uniform_ = std::uniform_real_distribution<float>(0.0, 1.0);
	
	// distribution for seed picking
	seedDist_ = std::uniform_int_distribution<int>(0, seedSequences_.size() - 1);
	
	// resize seed_
	seed_.resize(subBeatsPerBeat * beatsPerBar * barsPerPattern);
	// set seed balance
	setSeedBalance(seedBalance);
	// assign a seed sequence 
	setSeed(seed1, seed2);
	// put the seed sequence in the prevSequence_ buffer
	prevSequence_ = seed_;
	// initialise the prevNote_
	prevNote_ = prevSequence_.back();
}


void ProbabilisticArp::beat() 
{
	// update sequence pointer position [metrical position]
	if (++pointer_ >= subBeatsPerBeat_ * beatsPerBar_ * barsPerPattern_) {
		pointer_ = 0;
	}	
}

void ProbabilisticArp::play() {isPlaying_ = true; }
void ProbabilisticArp::stop() {isPlaying_ = false; }
bool ProbabilisticArp::isPlaying() {return isPlaying_; }

void ProbabilisticArp::setMetre(unsigned int subBeatsPerBeat, unsigned int beatsPerBar, unsigned int barsPerPattern)
{
	subBeatsPerBeat_ = subBeatsPerBeat;
	beatsPerBar_ = beatsPerBar;
	barsPerPattern_ = barsPerPattern;
}

void ProbabilisticArp::keyChange(unsigned int key) {key_ = key; }
void ProbabilisticArp::modeChange(unsigned int mode) {mode_ = mode;	}

unsigned int ProbabilisticArp::getKey() const {return key_; }
unsigned int ProbabilisticArp::getMode() const {return mode_; }

void ProbabilisticArp::setSequencePosition(unsigned int position) {pointer_ = position; }
unsigned int ProbabilisticArp::getSequencePosition() const {return pointer_; }

void ProbabilisticArp::setIntervalTemp(float intervalTemp) {intervalTemp_ = intervalTemp; }
void ProbabilisticArp::setContourTemp(float contourTemp) {contourTemp_ = contourTemp; }
void ProbabilisticArp::setRhythmicTemp(float rhythmicTemp) {rhythmicTemp_ = rhythmicTemp; }
void ProbabilisticArp::setSparsity(float sparsity) {sparsity_ = sparsity; }
void ProbabilisticArp::setConsistency(float consistency) {consistency_ = consistency; }
void ProbabilisticArp::setMovement(float movement) {movement_ = movement; }

void ProbabilisticArp::setHarmonicTemp(float harmonicTemp) 
{
	harmonicTemp_ = harmonicTemp; 
	
	// re-interpolate for the starting distribution
	for (unsigned int i = 0; i < startDistribution_.size(); i++) {
		startDistribution_[i] = lowTempDists_[tempDistChoice_][i] * (1 - harmonicTemp) + highTempDists_[tempDistChoice_][i] * harmonicTemp;
	}	
}

void ProbabilisticArp::setDynamicTemp(float dynamicTemp) {dynamicTemp_ = dynamicTemp; }
void ProbabilisticArp::setDynamicContourTemp(float dynamicContourTemp) {dynamicContourTemp_ = dynamicContourTemp; };
void ProbabilisticArp::setPitchTemp(float pitchTemp) {pitchTemp_ = pitchTemp; }

void ProbabilisticArp::changeAllTempsByProportion(float proportion)
{
	if (proportion >= 0) {
		intervalTemp_ += proportion * (1 - intervalTemp_);
		contourTemp_ += proportion * (1 - contourTemp_);
		rhythmicTemp_ += proportion * (1 - rhythmicTemp_);
		sparsity_ += proportion * (1 - sparsity_);
		consistency_ += proportion * (1 - consistency_);
		movement_ += proportion * (1 - movement_);
		harmonicTemp_ += proportion * (1 - harmonicTemp_);
		dynamicTemp_ += proportion * (1 - dynamicTemp_);
		dynamicContourTemp_ += proportion * (1 - dynamicContourTemp_);
		pitchTemp_ += proportion * (1 - pitchTemp_);
	}
	else {
		intervalTemp_ *= 1 + proportion;
		contourTemp_ *= 1 + proportion;
		rhythmicTemp_ *= 1 + proportion;
		sparsity_ *= 1 + proportion;
		consistency_ *= 1 + proportion;
		movement_ *= 1 + proportion;
		harmonicTemp_ *= 1 + proportion;
		dynamicTemp_ *= 1 + proportion;
		dynamicContourTemp_ *= 1 + proportion;
		pitchTemp_ *= 1 + proportion;
	}
}

float ProbabilisticArp::getIntervalTemp() {return intervalTemp_; };
float ProbabilisticArp::getContourTemp() {return contourTemp_; };
float ProbabilisticArp::getRhythmicTemp() {return rhythmicTemp_; };
float ProbabilisticArp::getSparsity() {return sparsity_; };
float ProbabilisticArp::getConsistency() {return consistency_; };
float ProbabilisticArp::getMovement() {return movement_; };
float ProbabilisticArp::getHarmonicTemp() {return harmonicTemp_; };
float ProbabilisticArp::getDynamicTemp() {return dynamicTemp_; };
float ProbabilisticArp::getDynamicContourTemp() {return dynamicContourTemp_; };
float ProbabilisticArp::getPitchTemp() {return pitchTemp_; };

float ProbabilisticArp::getOverallTemp() 
{
	float totalTemp = intervalTemp_ + contourTemp_ + rhythmicTemp_ + sparsity_ + consistency_ + 
					  movement_ + harmonicTemp_ + dynamicTemp_ + dynamicContourTemp_ + pitchTemp_;	
	return totalTemp / 9.0;
}


std::pair<int, float> ProbabilisticArp::generate()
{
	if (isPlaying_) {
		// initialise note and output
		int note = -1;
		int outputNote = -1;
		
		// reset distribution
		distribution_ = startDistribution_;
		
		// rt_printf("after harmonic: {%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f}\n", distribution_[0], distribution_[1], distribution_[2], distribution_[3], 
		// 		  distribution_[4], distribution_[5], distribution_[6], distribution_[7], distribution_[8], distribution_[9], distribution_[10], distribution_[11], distribution_[12]);
		
		// update probabilities based on position in sequence (rhythmic, contour temperatures and 'sparsity' for whether or not a note shuld be played)
		// bias towards no note if there was no note previously with low rhythm temperature
		// deal with initialisation of seed sequence
		int prevSeqNote = std::get<0>(prevSequence_[pointer_]);		// note at this metrical position in last pattern played
		
		// rt_printf("Pointer: %d, prevSeqNote: %d\n", pointer_, prevSeqNote);
		
		// get note chroma class for this position in previous sequence
		if (prevSeqNote != -1) {			// check it is not a non-note
			// convert to chroma class
			int prevSeqChroma = prevSeqNote % 12;						
			for (unsigned int i = 0; i < distribution_.size(); i++) {
				// weight options by proximity to previous sequence
				if (notes_[mode_][i] != -1) {			// check it is not a non-note (this will be dealt with separately)
					distribution_[i] *= powf((13.0 - (float)std::abs(notes_[mode_][i] - prevSeqChroma)) / 9.0, 8.0 * (1 - consistency_));	// low consistency slider position pulls generated note towards that in previous pattern
				}
				else {
					// update probability of no new note based on sparsity
					distribution_[i] += sparsity_ * 24.0;
					// update based on rhythmic temperature - a low temperature means that there should not be a no-note here
					distribution_[i] *= rhythmicTemp_;
				}
			}
		}
		// if there was no note in the previous sequence
		else {
			// only update non-note weighting
			for (unsigned int i = 0; i < distribution_.size(); i++)	{
				if (notes_[mode_][i] == -1) {
					// also update probability of no new note based on sparsity
					distribution_[i] += sparsity_ * 24.0;
				}
				else {
					// the probability of a new note in a previously no-note position is governed by the rhythmic temperature
					distribution_[i] *= rhythmicTemp_;
				}
			}		
		}

		// rt_printf("after consistency, sparsity, rhythmic: {%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f}\n", distribution_[0], distribution_[1], distribution_[2], distribution_[3], 
		// 		  distribution_[4], distribution_[5], distribution_[6], distribution_[7], distribution_[8], distribution_[9], distribution_[10], distribution_[11], distribution_[12]);
		
		// update probabilities based on previous note (movement)
		int prevNote = -1;
		unsigned int prevPosition = (prevSequence_.size() + pointer_ - 1) % prevSequence_.size();
		while (prevNote == -1) {
			// decrement pointer
			prevPosition = (prevSequence_.size() + prevPosition - 1) % prevSequence_.size();
			prevNote = std::get<0>(prevSequence_[prevPosition]);
			// protect against the improbable degeneration to all non-notes 
			if (prevPosition == pointer_) {
				prevNote = lowestNote_;
			}
		}
		// convert to a chroma class value
		int prevNoteChroma = prevNote % 12;
		// update distributuion weights according to proximity
		for (unsigned int i = 0; i < distribution_.size(); i++) {
			if (notes_[mode_][i] != -1) {			// check it is not a non-note 
				distribution_[i] *= powf((12.0 - std::abs(notes_[mode_][i] - prevNoteChroma)) / 3.5, 1.0 * (movement_));	// high movement pulls generated note towards that of previous note played
			}
		}

		// rt_printf("after movement: {%f, %f, %f, %f, %f, %f, %F, %f, %f, %f, %f, %f, %f}\n", distribution_[0], distribution_[1], distribution_[2], distribution_[3], 
		// 		  distribution_[4], distribution_[5], distribution_[6], distribution_[7], distribution_[8], distribution_[9], distribution_[10], distribution_[11], distribution_[12]);		

		// sample from distribution
		unsigned int position = sampleFrom(distribution_);
		
		// rt_printf("Chosen position: %d\n", position);
		
		// get note number
		note = notes_[mode_][position];
		
	
		// if note is '-1', this represents no note, so just return it
		// otherwise:
		if (note >= 0) {
			
			// find contour from seed sequence at this position
			Contour prevContour;
			prevPosition = pointer_;
			int seedNote = std::get<0>(seed_[prevPosition]);
			if (seedNote == -1) {
				prevContour = Contour::noNote;
			}
			else {
				prevPosition = (seed_.size() + prevPosition - 1) % prevSequence_.size();
				int prevSeedNote = std::get<0>(seed_[prevPosition]);
				while (prevSeedNote == -1) {
					// decrement pointer
					prevPosition = (seed_.size() + prevPosition - 1) % seed_.size();
					prevSeedNote = std::get<0>(seed_[prevPosition]);
					// protect against the improbable degeneration to all non-notes 
					if (prevPosition == pointer_) {
						prevSeedNote = lowestNote_;
					}
				}
				if (seedNote - prevSeedNote > 0) {
					prevContour = Contour::positive;
				}
				else if (seedNote - prevSeedNote < 0) {
					prevContour = Contour::negative;
				}
				else {
					prevContour = Contour::repeatedNote;
				}
			}
			
			// determine MIDI note (relative to harmonic root)
			// get MIDI chroma class of new note
			note = (note + key_) % 12;	
			
			int tempNote = 0;									// to hold the absolute MIDI pitch of the proposed new note
			unsigned int difference = octaves_ * 12;			// difference in semitones between proposed new note and comparison note
			unsigned int bestDifferenceInt = octaves_ * 12;		// closest interval to previous sequence found so far
			unsigned int bestDifferencePitch = octaves_ * 12;	// closest to seed pitch found so far
			unsigned int closestPosInt = 0;						// position in vector of closest interval proposed note
			unsigned int closestPosPitch = 0;					// position in vector of closest seed pitch proposed note
			
			// get options for note over possible octaves
			// if there was a new note at this point in the previous sequence - weight proposed notes by proximity to most recent note played
			if (prevSeqNote == -1) {
				prevSeqNote = prevNote; 						// previously calculated and guaranteed not to be = -1
			}

			// loop through octaves
			for (unsigned int i = 0; i < noteOptions_.size(); i++) {
				// set note to i octaves above lowest allowed
				tempNote = lowestNote_ + (note + 12 * i);
				difference = std::abs(tempNote - prevSeqNote);
				noteOptions_[i] = octaves_ * 12 - difference;
				// track closest option
				if (difference <= bestDifferenceInt) {
					closestPosInt = i;
					bestDifferenceInt = difference;
				}
				
				// take into account seed contour weighting
				if (tempNote - prevNote < 0 && prevContour == Contour::negative) {
					noteOptions_[i] += octaves_ * 12 * (1 - contourTemp_);
				}
				else if (tempNote - prevNote > 0 && prevContour == Contour::positive) {
					noteOptions_[i] += octaves_ * 12 * (1 - contourTemp_);
				}
				else if (tempNote - prevNote == 0 && prevContour == Contour::repeatedNote) {
					noteOptions_[i] += octaves_ * 12 * (1 - contourTemp_);
				}
				
				// adjust weights by interval relative to the seed sequence
				difference = std::abs(tempNote - seedNote);
				
				// track closest option
				if (difference <= bestDifferencePitch) {
					closestPosPitch = i;
					bestDifferencePitch = difference;
				}
			}
			
			// rt_printf("after contour: {%f, %f, %f, %f}\n", noteOptions_[0], noteOptions_[1], noteOptions_[2], noteOptions_[3]);			
			
			// adjust weights of octave positions which are not closest to current chosen note
			for (unsigned int i = 0; i < noteOptions_.size(); i++) {
				if (i != closestPosInt) {
					noteOptions_[i] *= (intervalTemp_ * 1.9) + 0.1;		// multiply by 2 to allow non-closest to be most likely option for high temperatures
				}
			}
			
			// rt_printf("after interval: {%f, %f, %f, %f}\n", noteOptions_[0], noteOptions_[1], noteOptions_[2], noteOptions_[3]);				

			// adjust weights by interval relative to the seed sequence
			for (unsigned int i = 0; i < noteOptions_.size(); i++) {
				if (i != closestPosPitch) {
					noteOptions_[i] *= pitchTemp_ + 0.001;
				}
			}

			// sample from distribution
			position = sampleFrom(noteOptions_);
			
			// get note
			outputNote = lowestNote_ + note + position * 12;
			// outputNote = note + position * 12;
		}
		else {
			outputNote = note;
		}
		
		// get amplitude
		float outputAmp = std::get<1>(prevSequence_[pointer_]);
		
		// vary amplitude according to dynamicTemp
		if (dynamicTemp_ > 0) {
			outputAmp *= (1 + (0.5 - uniform_(rng_)) * dynamicTemp_ / 4.0);
		}

		// rt_printf("after dynamic: %f\n", outputAmp);	
		
		// pull dynamics back toward seed sequence dynamics
		outputAmp += (std::get<1>(seed_[pointer_]) - outputAmp) * (1 - dynamicContourTemp_);
				
		// rt_printf("after dynamicContour: %f\n", outputAmp);	
		
		// put note into previous note variable
		prevNote_ = {outputNote, outputAmp};
		
		// put note into last seqeunce buffer
		prevSequence_[pointer_] = prevNote_;
	}
	
	return prevNote_;
}


unsigned int ProbabilisticArp::sampleFrom(std::vector<float>& distribution)
{
	// normalise
	float sum = 0;
	for (unsigned int i = 0; i < distribution.size(); i++) {
		sum += distribution[i];
	}
	// make cumulative along interval [0, 1]
	for (unsigned int i = 0; i < distribution.size(); i++) {
		distribution[i] /= sum;
		if (i > 0) {
			distribution[i] += distribution[i - 1];
		}
	}
	
	assert (distribution.back() == 1.0);
	
	// generate new random number
	float randNum = uniform_(rng_);
	
	// rt_printf("Random number: %f\n", randNum);
	
	assert (randNum <= 1.0);
	
	int position = -1;
	
	// get position
	for (unsigned int i = 0; i < distribution.size(); i++) {
		if (distribution[i] >= randNum) {
			// select the note number from the list according to randNum
			position = i;
			break;
		}
	}
	
	assert (position >= 0);
	
	return position;
}


// get the number of available seeds
unsigned int ProbabilisticArp::numSeeds() {return seedSequences_.size(); }

void ProbabilisticArp::setSeed(int seed1, int seed2)
{
	// check seed1 and seed2
	if (seed1 < -1 || seed1 >= seedSequences_.size()) {
		seed1 = -1;
	}
	if (seed2 < -1 || seed2 >= seedSequences_.size()) {
		seed2 = -1;
	}
	
	if (seed1 != seed1num_ || seed2 != seed2num_) {			// only proceed if necessary
		if (seed1 == -1) {
			// randomly pick a seed number
			seed1 = seedDist_(rng_);
		}
		if (seed2 == -1) {
			seed2 = seed1;
			while (seed2 == seed1) {
				// randomly pick a seed number
				seed2 = seedDist_(rng_);
			}
		}
	}
	
	// set seed numbers
	seed1num_ = seed1;
	seed2num_ = seed2;
	
	// interopolate between the two see sequences acconrding to seedBalance
	setSeedBalance(seedBalance_);
}

void ProbabilisticArp::setSeedBalance(float balance) 
{
	// check balance
	if (balance < 0 || balance > 1) {
		balance = 0;
		rt_printf("Invalid seed balance given: %f\n", balance);
	}
	
	// set seedBalance_
	seedBalance_ = balance;

	// interpolate between the two seed sequences
	for (unsigned int i = 0; i < seed_.size(); i++) {
		float noteinterp = round((1 - balance) * std::get<0>(seedSequences_[seed1num_][i]) + 
								  balance * std::get<0>(seedSequences_[seed2num_][i]));
		int note = noteinterp;
		float amplitude = (1 - balance) * std::get<1>(seedSequences_[seed1num_][i]) + 
						  balance * std::get<1>(seedSequences_[seed2num_][i]);
		seed_[i] = {note, amplitude};
	}
}

float ProbabilisticArp::getSeedBalance() {return seedBalance_; }

std::vector<int> ProbabilisticArp::getSeeds() {return std::vector<int> {seed1num_, seed2num_}; }

void ProbabilisticArp::resetToSeed() {prevSequence_ = seed_; }

void ProbabilisticArp::setTempDistChoice(unsigned int choice)
{
	if (choice >= lowTempDists_.size()) {
		throw std::invalid_argument("Invalid argument to 'setTempDistChoice': choice");
	}
	// set tempDistChoice_
	tempDistChoice_ = choice;
	
	// re-calculate start distribution for note chroma generation
	setHarmonicTemp(harmonicTemp_);
}

unsigned int ProbabilisticArp::getNumTempDists() {return lowTempDists_.size(); }


// optional alternative random number generator
// uint64_t ProbabilisticArp::xorshift64(struct xorshift64_state *state)
// 	{
// 		uint64_t x = state->a;
// 		x ^= x << 13;
// 		x ^= x >> 7;
// 		x ^= x << 17;
// 		return state->a = x;
// 	}

