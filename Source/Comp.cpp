/*
  ==============================================================================
    Comp.cpp
    Author:  Quentin Prost
  ==============================================================================
*/

#include "Comp.h"

template <typename SampleType>
Comp<SampleType>::Comp() : mAhr(), mControlGain(), mSignalLevel(), mSideChain(), ballistic(), lookAhead(44100, 0.01, 2)
{
    mSampleRate = 44100;
    mMaxBlockSize = 2048;
}

template <typename SampleType>
Comp<SampleType>::Comp(int sampleRate, int maxBlockSize) :
                                            mAhr(sampleRate, maxBlockSize),
                                            mControlGain(1, maxBlockSize),
                                            mSignalLevel(1, maxBlockSize),
                                            mSideChain(1, maxBlockSize),
                                            ballistic(),
                                            lookAhead(sampleRate, static_cast<SampleType>(0.01), 2)
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
    mSignalLevel.setSize(1, mMaxBlockSize);
    mSideChain.setSize(1, mMaxBlockSize);
    ballistic.prepare(spec);
    setEstimationType(EstimationType::peak);
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
void Comp<SampleType>::setMakeUpGain(SampleType makeUpGain) {
    mParams.makeUpGain = makeUpGain;
    mAhr.setMakeUpGain(makeUpGain);
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
void Comp<SampleType>::processBlock(juce::dsp::ProcessContextNonReplacing<SampleType>& inputContext,
                                    juce::dsp::ProcessContextReplacing<SampleType>& sideChainContext) {
        
    auto const& input = inputContext.getInputBlock();
    auto& sideChainInput = sideChainContext.getInputBlock();
    auto& output = inputContext.getOutputBlock();
    size_t blockSize = input.getNumSamples();
    
    juce::dsp::AudioBlock<SampleType> signalLevel(mSignalLevel);
    juce::dsp::AudioBlock<SampleType> gains(mControlGain);
    juce::dsp::AudioBlock<SampleType> sideChain(mSideChain);
    sideChain.copyFrom(sideChainInput.getSingleChannelBlock(0));
    sideChain.add(sideChainInput.getSingleChannelBlock(1));
    sideChain.multiplyBy(0.5);
    
    juce::dsp::ProcessContextNonReplacing<SampleType> context_ballistic(mSideChain, signalLevel);
    juce::dsp::ProcessContextNonReplacing<SampleType> context_ahr(signalLevel, gains);
    ballistic.process(context_ballistic);
    mAhr.processBlock(context_ahr);
    
    for (int n = 0; n < blockSize; n++) {
        for (int channel = 0; channel < mNumChannels; channel++) {
            SampleType output_value = input.getSample(channel, n) * gains.getSample(0, n);
            output.setSample(channel, n, output_value);
        }
    }
    
}

template class Comp<float>;
template class Comp<double>;
