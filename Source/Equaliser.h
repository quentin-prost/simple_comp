/*  Equaliser.h
    Quentin Prost */

#pragma once

#include <JuceHeader.h>

enum FilterType {
    LOWPASS,
    PEAK,
    HIGHPASS,
};

enum FilterSlope {
    SLOPE_12,
    SLOPE_24,
    SLOPE_36,
    SLOPE_48
};

template <typename T>
struct FilterParams {
    FilterParams(T freqToUse, T qualityToUse, T gainToUse, FilterSlope slopeToUse, FilterType typeToUse) :
        freq(freqToUse),
        quality(qualityToUse),
        gainDb(gainToUse),
        slope(slopeToUse),
        type(typeToUse) {}
    T freq = static_cast<T>(1000.0);
    T quality = static_cast<T>(1.0);
    T gainDb = static_cast<T>(0.0);
    FilterSlope slope = {SLOPE_12};
    FilterType type = {LOWPASS};
};

template <typename T>
struct FilterBand {
    FilterBand (const juce::String& nameToUse, FilterParams<T> paramsToUse, size_t index, T sampleRate) :
            name(nameToUse),
            params(paramsToUse)
            {}
    juce::String name;
    FilterParams<T> params;
    size_t index;
    T sampleRate;
    bool bypass = true;
};

using Filter = juce::dsp::IIR::Filter<float>;

using FilterCutChain = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;

using MonoChain = juce::dsp::ProcessorChain<FilterCutChain, Filter, FilterCutChain>;

using Coefficients = Filter::CoefficientsPtr;

template <typename T>
class Equaliser {
public:
    Equaliser(T sampleRate);
    ~Equaliser() {};
    
    size_t getNumBands() const {
        return _bands;
    }
    
    juce::String getBandName(size_t index) const {
        return bands[index].name;
    }
    
    void bypassBand(size_t index, bool bypass) {
        bands[index].bypass = bypass;
        leftProcessorChain.setBypassed<index>(bypass);
        rightProcessorChain.setBypassed<index>(bypass);
    }
    
    juce::String getFilterBandName(size_t index) {
        return bands[index].name;
    }
    
    void updateFilter(FilterBand<T>& filter);
    void updateLowPassFilter(FilterBand<T>& filter);
    void updateHighPassFilter(FilterBand<T>& filter);
    void updatePeakFilter(FilterBand<T>& filter);
    
    void prepare(const juce::dsp::ProcessSpec &specs) {
        leftProcessorChain.prepare(specs);
        rightProcessorChain.prepare(specs);
    }
    
    void updateAll() {
        for (size_t i = 0; i < 2; i++)
            updateFilter(bands[i]);
    }
    
    void processBlock(juce::dsp::ProcessContextNonReplacing<T>& buffer);
    
private:
    std::vector<FilterBand<T>> bands;
    MonoChain leftProcessorChain, rightProcessorChain;
    T sampleRate = 44100.0;
    const int _bands = 3;
    
};
