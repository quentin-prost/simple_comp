#include "RingBuffer.h"

int nextPow2(int x) {
    return (int) pow(2.0, ceilf( log(x)/log(2.0) ) );
}

template <typename SampleType>
RingBuffer<SampleType>::RingBuffer(int sampleRate, SampleType maxDelaySeconds, int numChannels) : mRingBuffer() {
    int maxDelaySamples = ceil(sampleRate * maxDelaySeconds);
    mMaxDelaySamples = nextPow2(maxDelaySamples); // for optimization read index with & operation instead of %
    mDelaySample = 0; mNumChannels = numChannels;
    mRingBuffer.setSize(numChannels, mMaxDelaySamples);
    writeIndex = 0, readIndex = 0;
}

template <typename SampleType>
RingBuffer<SampleType>::~RingBuffer() {
}

template <typename SampleType>
int RingBuffer<SampleType>::setDelay(int delaySeconds) {
    int delaySample = ceil(mSampleRate * delaySeconds);
    if (delaySample > mMaxDelaySamples) return -1;
    mDelaySample = delaySample;
    return 0;
}

template <typename SampleType>
int RingBuffer<SampleType>::writeSample(SampleType sample, int channel) {
    if (channel > mNumChannels) return -1;
    mRingBuffer.setSample(channel, writeIndex++, sample);
    if (writeIndex >= mMaxDelaySamples) writeIndex = 0;
    return 0;
}

template <typename SampleType>
int RingBuffer<SampleType>::writeChannel(juce::AudioBuffer<SampleType>& buffer, int channel) {
    if (channel > mNumChannels) return -1;
    if (buffer.getNumSamples() > mMaxDelaySamples) return -1;
    for (int n = 0; n < buffer.getNumSamples(); n++) {
        mRingBuffer.setSample(channel, writeIndex, buffer.getSample(channel, n));
        if (++writeIndex >= mMaxDelaySamples) writeIndex = 0;
    }
    return 0;
}

template <typename SampleType>
int RingBuffer<SampleType>::writeBuffer(juce::AudioBuffer<SampleType> &buffer) {
    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();
    if (mNumChannels > numChannels) return -1;
    if (mMaxDelaySamples < numSamples) return -1;
    for (int n = 0; n < numSamples; n++) {
        for (int channel = 0; n < numChannels; n++) {
            mRingBuffer.setSample(channel, writeIndex, buffer.getSample(channel, n));
        }
        if (++writeIndex >= mMaxDelaySamples) writeIndex = 0;
    }
    return 0;
}

template <typename SampleType>
int RingBuffer<SampleType>::readBuffer(juce::AudioBuffer<SampleType> &buffer) {
    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();
    if (mNumChannels > numChannels) return -1;
    if (mMaxDelaySamples < numChannels) return -1;
    readIndex = (writeIndex + mMaxDelaySamples - mDelaySample) & (mMaxDelaySamples - 1);
    for (int n = 0; n < numSamples; n++) {
        for (int channel = 0; n < numChannels; n++) {
            buffer.setSample(channel, n, mRingBuffer.getSample(0, readIndex));
        }
        if (++readIndex > mMaxDelaySamples) readIndex = 0;
    }
    return 0;
}

template class RingBuffer<float>;
template class RingBuffer<double>;
