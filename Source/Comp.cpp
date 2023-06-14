/*
  ==============================================================================
    Comp.cpp
    Author:  Quentin Prost
  ==============================================================================
*/

#include "Comp.h"

template <typename SampleType>
Comp<SampleType>::Comp(int sampleRate, int maxBlockSize) :
                                            mAhr(sampleRate, maxBlockSize),
                                            mControlGain(1, maxBlockSize),
                                            mSignalLevel(1, maxBlockSize),
                                            mSideChain(),
                                            ballistic()
{
    mSampleRate = sampleRate;
    mMaxBlockSize = maxBlockSize;
}

template <typename SampleType>
Comp<SampleType>::~Comp() {
    
}

template <typename SampleType>
void Comp<SampleType>::prepare(const juce::dsp::ProcessSpec &spec) {
    mSampleRate = spec.sampleRate;
    mMaxBlockSize = spec.maximumBlockSize;
    mNumChannels = spec.numChannels;
    mAhr.prepare(spec);
    mControlGain.setSize(1, mMaxBlockSize);
    mSignalLevel.setSize(1, mSignalLevel);
    ballistic.prepare(spec);
}

template <typename SampleType>
void Comp<SampleType>::setAttack(SampleType attack) {
    mParams.attack = attack;
    mAhr.setAttack(attack);
}

template <typename SampleType>
void Comp<SampleType>::setHold(SampleType hold) {
    mParams.hold = hold;
    mAhr.setHold(hold);
}

template <typename SampleType>
void Comp<SampleType>::setRelease(SampleType release) {
    mParams.release = release;
    mAhr.setRelease(release);
}

template <typename SampleType>
void Comp<SampleType>::setThreshold(SampleType threshold) {
    mParams.threshold = threshold;
    mAhr.setThreshold(threshold);
}

template <typename SampleType>
void Comp<SampleType>::setRatio(SampleType ratio) {
    mParams.ratio = ratio;
    mAhr.setRatio(ratio);
}

template <typename SampleType>
void Comp<SampleType>::setKnee(SampleType knee) {
    mParams.knee = knee;
    mAhr.setKnee(knee);
}

template <typename SampleType>
void Comp<SampleType>::setBypass(bool active) {
    bypass = active;
}

template <typename SampleType>
void Comp<SampleType>::setLookAhead(bool active, SampleType lookAheadTime) {
    mLookAheadTime = lookAheadTime;
}

template <typename SampleType>
void Comp<SampleType>::setEstimationType(EstimationType type) {
    mParams.estimationType = type;
    ballistic.setLevelCalculationType(type);
    switch (type) {
        case EstimationType::RMS:
            ballistic.setAttackTime(300.0);
            ballistic.setReleaseTime(300.0);
            break;
        case EstimationType::peak:
            ballistic.setAttackTime(0.001);
            ballistic.setReleaseTime(0.01);
        default:
            break;
    }
}

template <typename SampleType>
void Comp<SampleType>::processBlock(juce::dsp::ProcessContextNonReplacing<SampleType> &context) {
        
    auto& input = context.getInputBlock();
    auto& output = context.getOutputBlock();
    size_t blockSize = input.getNumSamples();
    
    jassert(blockSize < mMaxBlockSize);
    juce::dsp::AudioBlock<SampleType> signalLevel(mSignalLevel);
    juce::dsp::AudioBlock<SampleType> gains(mControlGain);
    mSideChain.copyFrom(input.getSingleChannelBlock(0));
    mSideChain.add(input.getSingleChannelBlock(1));
    mSideChain.multiplyBy(0.5);
    
    juce::dsp::ProcessContextNonReplacing<SampleType> context_ballistic(mSideChain, signalLevel);
    juce::dsp::ProcessContextNonReplacing<SampleType> context_ahr(signalLevel, gains);
    ballistic.process(context_ballistic);
    mAhr.processBlock(context_ahr);
    
    for (int n = 0; n < blockSize; n++) {
        for (int channel = 0; channel < mNumChannels; channel++) {
            SampleType output_value = output.getSample(channel, n) * gains.getSample(channel, 0);
            output.setSample(channel, n, output_value);
        }
    }
}

template class Comp<float>;
template class Comp<double>;
