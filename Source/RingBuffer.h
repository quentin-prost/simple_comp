/* ==============================================================================
    RingBuffer.h
    Created: 15 Jun 2023 12:45:30pm
    Author:  Quentin Prost
  ============================================================================== */

#pragma once

#include "JuceHeader.h"

template<typename SampleType>
class RingBuffer {
public:
    RingBuffer() {};
    RingBuffer(int sampleRate, SampleType maxDelaySeconds, int numChannels);
    ~RingBuffer();
    
    int setDelay(int delaySeconds);
    int writeSample(SampleType sample, int channel);
    // Write block of samples from buffer
    int writeChannel(juce::AudioBuffer<SampleType>& buffer, int channel);
    // Read samples from the ring buffer and store it into buffer
    int writeBuffer(juce::AudioBuffer<SampleType>& buffer);
    int readBuffer(juce::AudioBuffer<SampleType>& buffer);
    
    int getNumChannels();
    int maxDelaySamples();
    void resize(int numSamples, int numChannels);
    void update();
    void reset();
private:
    juce::AudioBuffer<SampleType> mRingBuffer;
    int mMaxDelaySamples, mDelaySample;
    int writeIndex, readIndex;
    int mSampleRate = 44100, mNumChannels = 2;
};
