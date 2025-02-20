/*
  ==============================================================================
    Comp.cpp
    Author:  Quentin Prost
  ==============================================================================
*/

#include "Comp.h"

template <typename SampleType>
Comp<SampleType>::Comp() : mAhr(), mControlGainBuffer(), mSignalLevelBuffer(), mSideChainBuffer(), ballistic(), eq()
{
    mSampleRate = 44100;
    mMaxBlockSize = 2048;
}

template <typename SampleType>
Comp<SampleType>::Comp(int sampleRate, int maxBlockSize) :
                                            mAhr(sampleRate, maxBlockSize),
                                            mControlGainBuffer(1, maxBlockSize),
                                            mSignalLevelBuffer(1, maxBlockSize),
                                            mSideChainBuffer(1, maxBlockSize),
                                            ballistic(),
                                            eq(sampleRate)
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
    mControlGainBuffer.setSize(1, mMaxBlockSize);
    mSignalLevelBuffer.setSize(1, mMaxBlockSize);
    mSideChainBuffer.setSize(1, mMaxBlockSize);
    ballistic.setLevelCalculationType(mParams.estimationType);
    ballistic.prepare(spec);
    eq.prepare(spec);
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
void Comp<SampleType>::setExternalSideChain(bool value) {
    mExternalSideChain = value;
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
void Comp<SampleType>::setEqSideChainBypass(bool bypass) {
    mEqSideChainBypass = bypass;
}

template <typename SampleType>
void Comp<SampleType>::setEqBandBypass(size_t index, bool bypass) {
    eq.setBandBypass(index, bypass);
}

template <typename SampleType>
void Comp<SampleType>::setEqBandParams(size_t index, FilterParams& params) {
    eq.setBandParams(index, params);
}

template <typename SampleType>
void Comp<SampleType>::processBlock(juce::dsp::ProcessContextNonReplacing<SampleType>& inputContext,
                                    juce::dsp::ProcessContextNonReplacing<SampleType>& extSideChainContext) {
        
    
    auto const& input = inputContext.getInputBlock();
    auto& output = inputContext.getOutputBlock();
    auto& sideChainInputContext = mExternalSideChain ? extSideChainContext : inputContext;
    size_t blockSize = input.getNumSamples();
    
    juce::dsp::AudioBlock<SampleType> signalLevelBlock;
    juce::dsp::AudioBlock<SampleType> outputGainsBlock(mControlGainBuffer);
    juce::dsp::AudioBlock<SampleType> sideChainBlock;
    
    /*sideChainBlock.copyFrom(sideChainInput.getSingleChannelBlock(0));
    sideChainBlock.add(sideChainInput.getSingleChannelBlock(1));
    sideChainBlock.multiplyBy(0.5);*/
    
    if (!mEqSideChainBypass)
        eq.processBlock(sideChainInputContext);
    
    /* Side Chain signal for ballistic filter is only mono at the moment (i.e fully linked stereo compression)
     To do : Add a new parameters that can gradually unlinked the gain reduction for left & right channels */
    sideChainBlock.copyFrom(sideChainInputContext.getOutputBlock().getSingleChannelBlock(0));
    sideChainBlock.add(sideChainInputContext.getOutputBlock().getSingleChannelBlock(1));
    sideChainBlock.multiplyBy(0.5);
    
    juce::dsp::ProcessContextNonReplacing<SampleType> context_ballistic(sideChainBlock, signalLevelBlock);
    juce::dsp::ProcessContextNonReplacing<SampleType> context_ahr(signalLevelBlock, outputGainsBlock);
    ballistic.process(context_ballistic);
    mAhr.processBlock(context_ahr);
    
    for (int n = 0; n < blockSize; n++) {
        for (int channel = 0; channel < mNumChannels; channel++) {
            SampleType output_value = input.getSample(channel, n) * outputGainsBlock.getSample(0, n);
            output.setSample(channel, n, output_value);
        }
    }
}

template class Comp<float>;
template class Comp<double>;
