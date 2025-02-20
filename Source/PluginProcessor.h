/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Comp.h"
#include "../Utilities/Utils.h"

#define NUM_EQ_BANDS 4

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
    PARAMETER_ID(eqBandFreq1)
    PARAMETER_ID(eqBandQuality1)
    PARAMETER_ID(eqBandGain1)
    PARAMETER_ID(eqBandActive1)
    PARAMETER_ID(eqBandSlope1)
    PARAMETER_ID(eqBandType1)
    PARAMETER_ID(eqBandFreq2)
    PARAMETER_ID(eqBandQuality2)
    PARAMETER_ID(eqBandGain2)
    PARAMETER_ID(eqBandActive2)
    PARAMETER_ID(eqBandSlope2)
    PARAMETER_ID(eqBandType2)
    PARAMETER_ID(eqBandFreq3)
    PARAMETER_ID(eqBandQuality3)
    PARAMETER_ID(eqBandGain3)
    PARAMETER_ID(eqBandActive3)
    PARAMETER_ID(eqBandSlope3)
    PARAMETER_ID(eqBandType3)
    PARAMETER_ID(eqBandFreq4)
    PARAMETER_ID(eqBandQuality4)
    PARAMETER_ID(eqBandGain4)
    PARAMETER_ID(eqBandActive4)
    PARAMETER_ID(eqBandSlope4)
    PARAMETER_ID(eqBandType4)
}

template <typename T>
inline static void castParameter(juce::AudioProcessorValueTreeState& apvts, const juce::ParameterID& Id, T& dest) {
    dest = dynamic_cast<T>(apvts.getParameter(Id.getParamID()));
    jassert(dest);
}

//==============================================================================
/**
*/

struct compEqBandParams {
    juce::AudioParameterFloat* freq;
    juce::AudioParameterFloat* quality;
    juce::AudioParameterFloat* gain;
    juce::AudioParameterChoice* slope;
    juce::AudioParameterChoice* type;
    juce::AudioParameterBool* active;
};

struct compEqParams {
    compEqBandParams bands[NUM_EQ_BANDS];
};

struct compAudioProcessorParams {
    juce::AudioParameterFloat* attack;
    juce::AudioParameterFloat* hold;
    juce::AudioParameterFloat* release;
    juce::AudioParameterFloat* threshold;
    juce::AudioParameterFloat* knee;
    juce::AudioParameterFloat* ratio;
    juce::AudioParameterFloat* makeUpGain;
    juce::AudioParameterChoice* estimationType;
    juce::AudioParameterBool* externalSideChain;
    juce::AudioParameterBool* bypass;
    compEqParams eq;
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
    juce::AudioBuffer<float> inputBuffer, outputBuffer, sideChainBuffer;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    bool externalSideChain = false, is_bypass = false;
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Simple_compAudioProcessor)
};
