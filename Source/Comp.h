/*
  ==============================================================================
    Comp.h
    Author:  Quentin Prost
  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"
#include "CompAhr.h"
#include "Equaliser.h"

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
    void setExternalSideChain(bool value);
    void setEstimationType(EstimationType type);
    void setEqSideChainBypass(bool bypass);
    void setEqBandBypass(size_t index, bool bypass);
    void setEqBandParams(size_t index, FilterParams& params);
    void processBlock(juce::dsp::ProcessContextNonReplacing<SampleType>& inputContext, juce::dsp::ProcessContextNonReplacing<SampleType>& sideChainContext);
    SampleType processSample(SampleType input);
public:
    CompAhr<SampleType> mAhr;
    juce::AudioBuffer<SampleType> mControlGainBuffer, mSignalLevelBuffer, mSideChainBuffer;
    juce::dsp::BallisticsFilter<SampleType> ballistic;
    Equaliser<SampleType> eq;
    CompParams<SampleType> mParams = {0.01, 0.0, 0.1, -6.0, 2.0, 5.0, 0.0, EstimationType::peak};
    int mSampleRate = 44100, mMaxBlockSize = 2048, mNumChannels = 2;
    bool mEqSideChainBypass = true;
    bool mExternalSideChain = false;
};
