#include "SF2Sound.h"
#include "SF2Reader.h"
#include "SFZSample.h"
#include "SFZDebug.h"

using namespace SFZero;


SF2Sound::SF2Sound(const juce::File& file_)
	: SFZSound(file_)
{
}


SF2Sound::~SF2Sound()
{
	// "presets" owns the regions, so clear them out of "regions" so ~SFZSound()
	// doesn't try to delete them.
	regions.clear();

	// The samples all share a single buffer, so make sure they don't all delete it.
	juce::AudioSampleBuffer* buffer = NULL;
	for (juce::HashMap<int64_t, SFZSample*>::Iterator i(samplesByRate); i.next();)
		buffer = i.getValue()->detachBuffer();
	delete buffer;
}


class PresetComparator {
	public:
		static int compareElements(const SF2Sound::Preset* first, const SF2Sound::Preset* second) {
			int cmp = first->bank - second->bank;
			if (cmp != 0)
				return cmp;
			return first->preset - second->preset;
			}
	};

void SF2Sound::loadRegions()
{
	SF2Reader reader(this, file);
	reader.read();

	// Sort the presets.
	PresetComparator comparator;
	presets.sort(comparator);

	useSubsound(0);
}


void SF2Sound::loadSamples(juce::AudioFormatManager* /*formatManager*/, double* progressVar, juce::Thread* thread)
{
	SF2Reader reader(this, file);
	juce::AudioSampleBuffer* buffer = reader.readSamples(progressVar, thread);
	if (buffer) {
		// All the SFZSamples will share the buffer.
		for (juce::HashMap<int64_t, SFZSample*>::Iterator i(samplesByRate); i.next();)
			i.getValue()->setBuffer(buffer);
		}

	if (progressVar)
		*progressVar = 1.0;
}


void SF2Sound::addPreset(SF2Sound::Preset* preset)
{
	presets.add(preset);
}


int SF2Sound::numSubsounds()
{
	return presets.size();
}


juce::String SF2Sound::subsoundName(int whichSubsound)
{
	Preset* preset = presets[whichSubsound];
	juce::String result;
	if (preset->bank != 0) {
		result += preset->bank;
		result += "/";
		}
	result += preset->preset;
	result += ": ";
	result += preset->name;
	return result;
}


void SF2Sound::useSubsound(int whichSubsound)
{
	selectedPreset = whichSubsound;
	regions.clear();
	regions.addArray(presets[whichSubsound]->regions);
}


int SF2Sound::selectedSubsound()
{
	return selectedPreset;
}


SFZSample* SF2Sound::sampleFor(unsigned long sampleRate)
{
	SFZSample* sample = samplesByRate[sampleRate];
	if (sample == NULL) {
		sample = new SFZSample(sampleRate);
		samplesByRate.set(sampleRate, sample);
		}
	return sample;
}


void SF2Sound::setSamplesBuffer(juce::AudioSampleBuffer* buffer)
{
	for (juce::HashMap<int64_t, SFZSample*>::Iterator i(samplesByRate); i.next();)
		i.getValue()->setBuffer(buffer);
}



