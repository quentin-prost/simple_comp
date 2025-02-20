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

struct FilterParams {
    FilterParams(float freqToUse, float qualityToUse, float gainToUse, FilterSlope slopeToUse, FilterType typeToUse) :
        freq(freqToUse),
        quality(qualityToUse),
        gainDb(gainToUse),
        slope(slopeToUse),
        type(typeToUse) {}
    float freq = static_cast<float>(1000.0);
    float quality = static_cast<float>(1.0);
    float gainDb = static_cast<float>(0.0);
    FilterSlope slope = {SLOPE_12};
    FilterType type = {LOWPASS};
};

struct FilterBand {
    FilterBand (const juce::String& nameToUse, FilterParams paramsToUse, size_t index, float sampleRate) :
            name(nameToUse),
            params(paramsToUse)
            {}
    juce::String name;
    FilterParams params;
    size_t index;
    float sampleRate;
    bool bypass = true;
};

using Filter = juce::dsp::IIR::Filter<float>;

using FilterCutChain = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;

using MonoChain = juce::dsp::ProcessorChain<FilterCutChain, Filter, FilterCutChain>;

using Coefficients = Filter::CoefficientsPtr;

template <typename SampleType>
class Equaliser {
public:
    Equaliser();
    Equaliser(float sampleRate);
    ~Equaliser() {};
    
    size_t getNumBands() const {
        return _bands;
    }
    
    juce::String getBandName(size_t index) const {
        return bands[index].name;
    }
    
    void setBandBypass(size_t index, bool bypass) {
        bands[index].bypass = bypass;
        leftProcessorChain.setBypassed<(int) index>(bypass);
        rightProcessorChain.setBypassed<(int) index>(bypass);
    }
    
    void setBandParams(size_t index, FilterParams& params) {
        bands[index].params = params;
        updateFilter(bands[index]);
    }
    
    FilterParams& getBandParams(size_t index) {
        return bands[index].params;
    }
    
    juce::String getFilterBandName(size_t index) {
        return bands[index].name;
    }
    
    void prepare(const juce::dsp::ProcessSpec &specs) {
        leftProcessorChain.prepare(specs);
        rightProcessorChain.prepare(specs);
    }
    
    void updateAll() {
        for (size_t i = 0; i < 2; i++)
            updateFilter(bands[i]);
    }
    
    void processBlock(juce::dsp::ProcessContextNonReplacing<SampleType>& buffer);
    
private:
    void updateFilter(FilterBand& filter);
    void updateLowPassFilter(FilterBand& filter);
    void updateHighPassFilter(FilterBand& filter);
    void updatePeakFilter(FilterBand& filter);
    std::vector<FilterBand> bands;
    MonoChain leftProcessorChain, rightProcessorChain;
    float sampleRate = 44100.0;
    const int _bands = 3;
};
