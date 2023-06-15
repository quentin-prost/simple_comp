/*
  ==============================================================================
    Ahr.h
    Created: 13 Jun 2023 12:01:04pm
    Author:  Quentin Prost

  ==============================================================================
*/
#pragma once

#include "JuceHeader.h"

enum CompKneeType {
    COMP_HARD_KNEE,
    COMP_SOFT_KNEE
};

enum CompAhrState {
    STATE_ATTACK,
    STATE_HOLD,
    STATE_RELEASE
};

template <typename SampleType>
struct CompAhrParams {
    SampleType attack; // in seconds
    SampleType hold; // in seconds
    SampleType release; // in seconds
    SampleType threshold; // in dB
    SampleType knee; // in dB
    SampleType ratio; // in dB
    SampleType makeUpGain; // in dB
};

template <typename SampleType>
struct CompAhrKnee {
    SampleType width;
    SampleType top;
    SampleType bottom;
    CompKneeType type;
};

template <typename SampleType>
struct CompAhrScaleType {
    SampleType db;
    SampleType linear;
};

template <typename SampleType>
struct CompAhrParamState {
    SampleType time;
    unsigned int counter;
    unsigned int samples;
    SampleType value;
    SampleType coefs[2];
};

template <typename SampleType>
struct CompAhrRatio {
    SampleType value;
    SampleType slope;
};

template <typename SampleType>
struct CompAhrEnvelope {
    SampleType target;
    SampleType dynamic;
};

template <typename SampleType>
class CompAhr {
    
public:
    CompAhr() {
        setParams(&mParams);
    }
    CompAhr(int sampleRate, int maxBlockSize) {
        mSampleRate = sampleRate;
        mEnvelope.resize(maxBlockSize);
        setParams(&mParams);
    }
    ~CompAhr() {};
    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();
    void setAttack(SampleType attack);
    void setHold(SampleType hold);
    void setRelease(SampleType release);
    void setThreshold(SampleType threshold);
    void setKnee(SampleType knee);
    void setRatio(SampleType ratio);
    void setMakeUpGain(SampleType makeUpGain);
    void setParams(CompAhrParams<SampleType> *params);
    
    void processBlock(const juce::dsp::ProcessContextNonReplacing<SampleType>& context);
    SampleType processSample(SampleType input);
private:
    
    void applyHardKnee(const juce::dsp::ProcessContextNonReplacing<SampleType>& context);
    void applySoftKnee(const juce::dsp::ProcessContextNonReplacing<SampleType>& context);
    SampleType applyHardKneeSample(SampleType envelopeDb);
    SampleType applySoftKneeSample(SampleType envelopeDb);
    
    CompAhrParams<SampleType> mParams = {0.001, 0.0, 0.1, -6.0, 6.0, 2.0, 0.0};
    CompAhrState mState = STATE_RELEASE;
    CompAhrParamState<SampleType> mAttack, mHold, mRelease;
    CompAhrScaleType<SampleType> mThreshold, mMakeUpGain;
    CompAhrRatio<SampleType> mRatio;
    CompAhrKnee<SampleType> mKnee;
    SampleType current_envelope;
    std::vector<SampleType> mEnvelope;
    int mSampleRate = 44100;
};
