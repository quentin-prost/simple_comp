/*
  ==============================================================================
    Comp.h
    Author:  Quentin Prost
  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"
#include "CompAhr.h"

using EstimationType = juce::dsp::BallisticsFilterLevelCalculationType;

template <typename SampleType>
struct CompParams {
    SampleType attack;
    SampleType hold;
    SampleType release;
    SampleType threshold;
    SampleType ratio;
    SampleType knee;
    SampleType makeUpGain;
    SampleType dryWetMix;
    SampleType lookaheadTime;
    EstimationType estimationType;
};

template <typename SampleType>
class Comp {
public:
    Comp();
    ~Comp();
    
    void prepare(const juce::dsp::ProcessSpec& spec);
    void setAttack(SampleType attack);
    void setHold(SampleType hold);
    void setRelease(SampleType release);
    void setThreshold(SampleType threshold);
    void setRatio(SampleType ratio);
    void setKnee(SampleType knee);
    void setBypass(bool active);
    void setLookAhead(bool active, float lookAheadTime);
    void setEstimationType(EstimationType type);
    
    void processBlock(const juce::dsp::ProcessContextNonReplacing<SampleType>& context);
    SampleType processSample(SampleType input);
public:
    juce::AudioBuffer<SampleType> mControlGain;
    juce::AudioBuffer<SampleType> mSideChain;
    CompParams<SampleType> mParams;
    CompAhr<SampleType> ahr;
    juce::dsp::BallisticsFilter<SampleType> ballistic;
    
};
