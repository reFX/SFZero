#ifndef SFZVoice_h
#define SFZVoice_h

#include "SFZEG.h"

namespace SFZero {

class SFZRegion;


class JUCE_API SFZVoice : public juce::SynthesiserVoice {
	public:
		SFZVoice();
		~SFZVoice() override;

    bool	canPlaySound(juce::SynthesiserSound* sound) override;
    void	startNote(
			const int midiNoteNumber,
			const float velocity,
			juce::SynthesiserSound* sound,
			const int currentPitchWheelPosition) override;
    void	stopNote(float velocity, const bool allowTailOff) override;
		void	stopNoteForGroup();
		void	stopNoteQuick();
    void	pitchWheelMoved(const int newValue) override;
    void	controllerMoved(
			const int controllerNumber,
			const int newValue) override;
    void	renderNextBlock(
			juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override;
		bool	isPlayingNoteDown();
		bool	isPlayingOneShot();

		int	getGroup();
		int	getOffBy();

		// Set the region to be used by the next startNote().
		void	setRegion(SFZRegion* nextRegion);

		juce::String	infoString();

	protected:
		int       	trigger;
		SFZRegion*	region;
		int       	curMidiNote, curPitchWheel;
		double    	pitchRatio;
		float     	noteGainLeft, noteGainRight;
		double    	sourceSamplePosition;
		SFZEG     	ampeg;
		unsigned long	sampleEnd;
		unsigned long	loopStart, loopEnd;

		// Info only.
		unsigned long	numLoops;
		int	curVelocity;

		void	calcPitchRatio();
		void	killNote();
		double	noteHz(double note, const double freqOfA = 440.0);
	};

}

#endif 	// SFZVoice_h

