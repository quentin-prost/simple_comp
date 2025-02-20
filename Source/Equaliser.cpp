/*
  ==============================================================================

    Equaliser.cpp
    Created: 19 Jun 2023 2:02:41pm
    Author:  Student

  ==============================================================================
*/

#include "Equaliser.h"

void updateCoefficients(Coefficients& coefs, const Coefficients& newCoefs) {
    *coefs = *newCoefs;
}

template<int Index, typename ChainType, typename SampleType>
void update(ChainType& chain, const SampleType& coefficients) {
    updateCoefficients(chain.template get<Index>().coefficients, coefficients[Index]);
}

template <typename ChainType, typename SampleType>
void updateCutCoefficients(ChainType& chain, const SampleType& coefficients, const FilterSlope& slope) {
    switch (slope) {
        case SLOPE_12:
            update<0>(chain, coefficients);
            break;
        case SLOPE_24:
            update<1>(chain, coefficients);
            break;
        case SLOPE_36:
            update<2>(chain, coefficients);
            break;
        case SLOPE_48:
            update<3>(chain, coefficients);
            break;
        default:
            break;
    }
}

template <typename T>
void Equaliser<T>::updateLowPassFilter(FilterBand<T>& filter) {
    if (filter.params.type != LOWPASS) jassert(false);
    
    auto coefficients = juce::dsp::FilterDesign<T>::designIIRLowpassHighOrderButterworthMethod(filter.params.freq, static_cast<double>(sampleRate), static_cast<int>(filter.params.slope));
    
    auto& leftLowPass = leftProcessorChain.get<filter.index>();
    auto& rightLowPass = rightProcessorChain.get<filter.index>();
    
    updateCutCoefficients(leftLowPass, coefficients, filter.params.slope);
    updateCutCoefficients(rightLowPass, coefficients, filter.params.slope);
}

template <typename T>
void Equaliser<T>::updateHighPassFilter(FilterBand<T>& filter) {
    if (filter.params.type != HIGHPASS) jassert(false);
    auto coefficients = juce::dsp::FilterDesign<T>::designIIRHighpassHighOrderButterworthMethod(filter.params.freq, static_cast<double>(sampleRate), static_cast<int>(filter.params.slope));
    
    auto& leftHighPass = leftProcessorChain.get<filter.index>();
    auto& rightHighPass = rightProcessorChain.get<filter.index>();
    
    updateCutCoefficients(leftHighPass, coefficients, filter.params.slope);
    updateCutCoefficients(rightHighPass, coefficients, filter.params.slope);
}

template <typename T>
void Equaliser<T>::updatePeakFilter(FilterBand<T>& filter) {
    if (filter.params.type != PEAK) jassert(false);
    
    juce::IIRCoefficients coefs;
    coefs.makePeakFilter(filter.sampleRate, filter.params.freq, filter.params.quality, filter.params.gainDb);
    
    Filter& leftPeakFilter = leftProcessorChain.get<filter.index>();
    Filter& rightPeakFilter = rightProcessorChain.get<filter.index>();
    
    updateCoefficients(leftPeakFilter.coefficients, (Coefficients&)coefs.coefficients[0]);
    updateCoefficients(rightPeakFilter.coefficients, (Coefficients&)coefs.coefficients[0]);
}

template <typename T>
void Equaliser<T>::updateFilter(FilterBand<T>& filter) {
    switch (filter.params.type) {
        case LOWPASS:
            updateLowPassFilter(filter);
            break;
        case HIGHPASS:
            updateHighPassFilter(filter);
            break;
        case PEAK:
            updatePeakFilter(filter);
            break;
        default:
            break;
    }
}

template <typename T>
Equaliser<T>::Equaliser(T sampleRate) {
    FilterParams<T> lowPassParams((T) 20000.0, (T) 0.0, (T) 0.0, SLOPE_12, LOWPASS);
    FilterParams<T> highPassParams((T) 100.0, (T) 0.0, (T) 0.0, SLOPE_12, HIGHPASS);
    FilterParams<T> peakParams((T) 1000.0, (T) 0.0, (T) 0.0, SLOPE_24, PEAK);
    bands[0] = FilterBand("HighPass", highPassParams, 0, true);
    bands[1] = FilterBand("LowPass", lowPassParams, 1, true);
    bands[2] = FilterBand("Peak", peakParams, 2, true);
    updateAll();
}

template <typename T>
void Equaliser<T>::processBlock(juce::dsp::ProcessContextNonReplacing<T>& input) {
    updateAll();
    
    auto& leftBlock = input.getInputBlock().getSingleChannelBlock(0);
    auto& rightBlock = input.getInputBlock().getSingleChannelBlock(1);
    
    leftProcessorChain.process(leftBlock);
    rightProcessorChain.process(rightBlock);
}
