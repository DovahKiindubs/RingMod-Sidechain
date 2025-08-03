/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RMSCAudioProcessor::RMSCAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
		               .withInput("Sidechain", juce::AudioChannelSet::stereo(), true)    
                       ),
	parameters(*this, nullptr, "Parameters",
		{
			std::make_unique<juce::AudioParameterFloat>("mix", "Mix", 0.0f, 1.0f, 1.0f),
			std::make_unique<juce::AudioParameterBool>("bypass", "Bypass", true)
		}),
    mainvisual(1),
	sidechainvisual(1)
#endif
{

    mainvisual.setRepaintRate(60);  
    mainvisual.setBufferSize(256); 
    sidechainvisual.setRepaintRate(60);
    sidechainvisual.setBufferSize(256);
}

RMSCAudioProcessor::~RMSCAudioProcessor()
{
}

//==============================================================================
const juce::String RMSCAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RMSCAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool RMSCAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool RMSCAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double RMSCAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RMSCAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int RMSCAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RMSCAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String RMSCAudioProcessor::getProgramName (int index)
{
    return {};
}

void RMSCAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void RMSCAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    mainvisual.clear();
    sidechainvisual.clear();
}

void RMSCAudioProcessor::releaseResources()
{
    mainvisual.clear();
    sidechainvisual.clear();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RMSCAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.inputBuses.size() > 1) //If there is a sidechain, OK if it is mono or stereo
    {
        auto sideChainLayout = layouts.getChannelSet(true, 1);
        if (sideChainLayout != juce::AudioChannelSet::mono()
            && sideChainLayout != juce::AudioChannelSet::stereo())
            return false;
}

    return true;
}
#endif

void RMSCAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    auto mainInput = getBusBuffer(buffer, true, 0); // Ö÷ÊäÈë
    auto sideInput = getBusBuffer(buffer, true, 1); // Sidechain ÊäÈë
    auto output = getBusBuffer(buffer, false, 0);

    const int numSamples = buffer.getNumSamples();
    const int numChannels = mainInput.getNumChannels();

    juce::AudioBuffer<float> scaledMainInput(mainInput.getNumChannels(), mainInput.getNumSamples());
    juce::AudioBuffer<float> scaledSideInput(sideInput.getNumChannels(), sideInput.getNumSamples());

    for (int ch = 0; ch < numChannels; ++ch)
    {
        scaledMainInput.copyFrom(ch, 0, mainInput, ch, 0, numSamples);
        scaledMainInput.applyGain(ch, 0, numSamples, 0.8f);

        scaledSideInput.copyFrom(ch, 0, sideInput, ch, 0, numSamples);
        scaledSideInput.applyGain(ch, 0, numSamples, 0.8f);
    }

    mainvisual.pushBuffer(scaledMainInput);
    sidechainvisual.pushBuffer(scaledSideInput);

    float mix = *parameters.getRawParameterValue("mix");
    bool bypass = (*parameters.getRawParameterValue("bypass") > 0.5f);

    if (!bypass)
        return; 


    for (int i = 0; i < numSamples; ++i)
    {

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float sc = sideInput.getReadPointer(ch)[i];
            float dry = mainInput.getReadPointer(ch)[i];
            float processed = dry - dry * std::abs(sc);
            float out = dry * (1.0f - mix) + processed * mix;
            output.getWritePointer(ch)[i] = out;
        }
    }
}

//==============================================================================
bool RMSCAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* RMSCAudioProcessor::createEditor()
{
    return new RMSCAudioProcessorEditor (*this);
}

//==============================================================================
void RMSCAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void RMSCAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RMSCAudioProcessor();
}
