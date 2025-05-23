#include "SFZSound.h"
#include "SFZRegion.h"
#include "SFZSample.h"
#include "SFZReader.h"
#include "SFZDebug.h"

using namespace SFZero;


SFZSound::SFZSound(const juce::File& fileIn)
	: file(fileIn)
{
}


SFZSound::~SFZSound()
{
	int numRegions = regions.size();
	for (int i = 0; i < numRegions; ++i)
	{
		delete regions[i];
		regions.set(i, NULL);
	}

	for (juce::HashMap<juce::String,SFZSample*>::Iterator i(samples); i.next();)
		delete i.getValue();
}


bool SFZSound::appliesToNote(const int /*midiNoteNumber*/)
{
	// Just say yes; we can't truly know unless we're told the velocity as well.
	return true;
}


bool SFZSound::appliesToChannel(const int /*midiChannel*/)
{
	return true;
}


void SFZSound::addRegion(SFZRegion* region)
{
	regions.add(region);
}


SFZSample* SFZSound::addSample(juce::String path, juce::String defaultPath)
{
	path = path.replaceCharacter('\\', '/');
	defaultPath = defaultPath.replaceCharacter('\\', '/');
	juce::File sampleFile;
	if (defaultPath.isEmpty())
		sampleFile = file.getSiblingFile(path);
	else {
		juce::File defaultDir = file.getSiblingFile(defaultPath);
		sampleFile = defaultDir.getChildFile(path);
		}
	juce::String samplePath = sampleFile.getFullPathName();
	SFZSample* sample = samples[samplePath];
	if (sample == NULL) {
		sample = new SFZSample(sampleFile);
		samples.set(samplePath, sample);
		}
	return sample;
}


void SFZSound::addError(const juce::String& message)
{
	errors.add(message);
}


void SFZSound::addUnsupportedOpcode(const juce::String& opcode)
{
	unsupportedOpcodes.set(opcode, opcode);
}


void SFZSound::loadRegions()
{
	SFZReader reader(this);
	reader.read(file);
}


void SFZSound::loadSamples (juce::AudioFormatManager* formatManager, double* progressVar, juce::Thread* thread)
{
	if (progressVar)
		*progressVar = 0.0;

	double numSamplesLoaded = 1.0, numSamples = samples.size();
	for (juce::HashMap<juce::String,SFZSample*>::Iterator i(samples); i.next();) {
		SFZSample* sample = i.getValue();
		bool ok = sample->load(formatManager);
		if (!ok)
			addError("Couldn't load sample \"" + sample->getShortName() + "\"");

		numSamplesLoaded += 1.0;
		if (progressVar)
			*progressVar = numSamplesLoaded / numSamples;
		if (thread && thread->threadShouldExit())
			return;
		}

	if (progressVar)
		*progressVar = 1.0;
}


SFZRegion* SFZSound::getRegionFor(
	int note, int velocity, SFZRegion::Trigger trigger)
{
	int numRegions = regions.size();
	for (int i = 0; i < numRegions; ++i) {
		SFZRegion* region = regions[i];
		if (region->matches((unsigned char)note, (unsigned char)velocity, trigger))
			return region;
		}

	return NULL;
}


int SFZSound::getNumRegions()
{
	return regions.size();
}


SFZRegion* SFZSound::regionAt(int index)
{
	return regions[index];
}


juce::String SFZSound::getErrorsString()
{
	juce::String result;
	int numErrors = errors.size();
	for (int i = 0; i < numErrors; ++i)
		result += errors[i] + "\n";

	if (unsupportedOpcodes.size() > 0) {
		result += "\nUnsupported opcodes:";
		bool shownOne = false;
		for (juce::HashMap<juce::String,juce::String>::Iterator i(unsupportedOpcodes); i.next();) {
			if (!shownOne) {
				result += " ";
				shownOne = true;
				}
			else
				result += ", ";
			result += i.getKey();
			}
		result += "\n";
		}
	return result;
}


int SFZSound::numSubsounds()
{
	return 1;
}


juce::String SFZSound::subsoundName(int /*whichSubsound*/)
{
	return {};
}


void SFZSound::useSubsound(int /*whichSubsound*/)
{
}


int SFZSound::selectedSubsound()
{
	return 0;
}


void SFZSound::dump()
{
	int i;

	int numErrors = errors.size();
	if (numErrors > 0) {
		printf("Errors:\n");
		for (i = 0; i < numErrors; ++i) {
			char message[256];
			errors[i].copyToUTF8(message, 256);
			printf("- %s\n", message);
			}
		printf("\n");
		}

	if (unsupportedOpcodes.size() > 0)
	{
		printf("Unused opcodes:\n");
		for (juce::HashMap<juce::String, juce::String>::Iterator itr(unsupportedOpcodes); itr.next();)
		{
			char opcode[64];
			itr.getKey().copyToUTF8(opcode, 64);
			printf("  %s\n", opcode);
		}
		printf("\n");
	}

	printf("Regions:\n");
	int numRegions = regions.size();
	for (i = 0; i < numRegions; ++i)
		regions[i]->dump();
	printf("\n");

	printf("Samples:\n");
	for (juce::HashMap<juce::String,SFZSample*>::Iterator itr(samples); itr.next();)
		itr.getValue()->dump();
}



