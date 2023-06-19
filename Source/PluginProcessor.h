/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Comp.h"
#include "../Utilities/Utils.h"

namespace ParameterID
{
#define PARAMETER_ID(str) const juce::ParameterID str(#str, 1);
    PARAMETER_ID(attackValue)
    PARAMETER_ID(holdValue)
    PARAMETER_ID(releaseValue)
    PARAMETER_ID(thresholdValue)
    PARAMETER_ID(kneeValue)
    PARAMETER_ID(ratioValue)
    PARAMETER_ID(makeUpGainValue)
    PARAMETER_ID(estimationTypeValue)
    PARAMETER_ID(externalSideChain)
    PARAMETER_ID(bypassValue)
}

template <typename T>
inline static void castParameter(juce::AudioProcessorValueTreeState& apvts, const juce::ParameterID& Id, T& dest) {
    dest = dynamic_cast<T>(apvts.getParameter(Id.getParamID()));
    jassert(dest);
}

//==============================================================================
/**
*/

struct compAudioProcessorParams {
    juce::AudioParameterFloat* attack;
    juce::AudioParameterFloat* hold;
    juce::AudioParameterFloat* release;
    juce::AudioParameterFloat* threshold;
    juce::AudioParameterFloat* knee;
    juce::AudioParameterFloat* ratio;
    juce::AudioParameterFloat* makeUpGain;
    juce::AudioParameterChoice* estimationType;
    juce::AudioParameterBool* bypass;
};

class Simple_compAudioProcessor  : public juce::AudioProcessor,
                                   public juce::AudioProcessorValueTreeState::Listener
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    Simple_compAudioProcessor();
    ~Simple_compAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void setExternalSideChain(bool active);
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    juce::AudioProcessorValueTreeState apvts;
private:
    Comp<float> comp;
    compAudioProcessorParams params;
    juce::AudioBuffer<float> inputBuffer;
    juce::AudioBuffer<float> sideChainBuffer;
    juce::AudioBuffer<float> outputBuffer;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    bool externalSideChain;
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Simple_compAudioProcessor)
};
