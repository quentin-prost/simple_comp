/*
  ==============================================================================
    Comp.h
    Author:  Quentin Prost
  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"
#include "CompAhr.h"
#include "RingBuffer.h"

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
    EstimationType estimationType;
    bool externalSideChain;
};

template <typename SampleType>
class Comp {
public:
    Comp();
    Comp(int sampleRate, int maxBlockSize);
    ~Comp();
    
    void prepare(const juce::dsp::ProcessSpec& spec);
    void setAttack(SampleType attack);
    void setHold(SampleType hold);
    void setRelease(SampleType release);
    void setThreshold(SampleType threshold);
    void setRatio(SampleType ratio);
    void setKnee(SampleType knee);
    void setMakeUpGain(SampleType makeUpGain);
    void setLookAhead(bool active, SampleType lookAheadTime);
    void setBypass(bool active);
    void setEstimationType(EstimationType type);
    
    void processBlock(juce::dsp::ProcessContextNonReplacing<SampleType>& inputContext, juce::dsp::ProcessContextReplacing<SampleType>& sideChainContext);
    SampleType processSample(SampleType input);
public:
    CompAhr<SampleType> mAhr;
    juce::AudioBuffer<SampleType> mControlGain, mSignalLevel, mSideChain;
    juce::dsp::BallisticsFilter<SampleType> ballistic;
    RingBuffer<SampleType> lookAhead;
    CompParams<SampleType> mParams = {0.01, 0.0, 0.1, -6.0, 2.0, 5.0, 0.0, EstimationType::peak};
    int mSampleRate = 44100, mMaxBlockSize = 2048, mNumChannels = 2;
    bool bypass, lookAheadActive;
    bool externalSideChain;
    SampleType mLookAheadTime, mDryWetMix;
};
