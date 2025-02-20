/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
Simple_compAudioProcessor::Simple_compAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       .withInput ("SideChain", juce::AudioChannelSet::stereo(), true)
                       ), apvts(*this, nullptr, "Parameters", createParameters()), comp(), inputBuffer(), outputBuffer(), inputSideChainBuffer(), outputSideChainBuffer()
#endif
{
    // Initialisation of audio parameters pointer
    castParameter(apvts, ParameterID::attackValue, params.attack);
    castParameter(apvts, ParameterID::holdValue, params.hold);
    castParameter(apvts, ParameterID::releaseValue, params.release);
    castParameter(apvts, ParameterID::thresholdValue, params.threshold);
    castParameter(apvts, ParameterID::kneeValue, params.knee);
    castParameter(apvts, ParameterID::ratioValue, params.ratio);
    castParameter(apvts, ParameterID::makeUpGainValue, params.makeUpGain);
    castParameter(apvts, ParameterID::estimationTypeValue, params.estimationType);
    castParameter(apvts, ParameterID::externalSideChain, params.externalSideChain);
    castParameter(apvts, ParameterID::bypassValue, params.bypass);
    castParameter(apvts, ParameterID::eqBandActive1, params.eq.bands[0].active);
    castParameter(apvts, ParameterID::eqBandFreq1, params.eq.bands[0].freq);
    castParameter(apvts, ParameterID::eqBandGain1, params.eq.bands[0].gain);
    castParameter(apvts, ParameterID::eqBandQuality1, params.eq.bands[0].quality);
    castParameter(apvts, ParameterID::eqBandType1, params.eq.bands[0].type);
    castParameter(apvts, ParameterID::eqBandSlope2, params.eq.bands[1].slope);
    castParameter(apvts, ParameterID::eqBandActive2, params.eq.bands[1].active);
    castParameter(apvts, ParameterID::eqBandFreq2, params.eq.bands[1].freq);
    castParameter(apvts, ParameterID::eqBandGain2, params.eq.bands[1].gain);
    castParameter(apvts, ParameterID::eqBandQuality2, params.eq.bands[1].quality);
    castParameter(apvts, ParameterID::eqBandType2, params.eq.bands[1].type);
    castParameter(apvts, ParameterID::eqBandSlope2, params.eq.bands[1].slope);
    castParameter(apvts, ParameterID::eqBandSlope3, params.eq.bands[2].slope);
    castParameter(apvts, ParameterID::eqBandActive3, params.eq.bands[2].active);
    castParameter(apvts, ParameterID::eqBandFreq3, params.eq.bands[2].freq);
    castParameter(apvts, ParameterID::eqBandGain3, params.eq.bands[2].gain);
    castParameter(apvts, ParameterID::eqBandQuality3, params.eq.bands[2].quality);
    castParameter(apvts, ParameterID::eqBandType3, params.eq.bands[2].type);
    castParameter(apvts, ParameterID::eqBandSlope3, params.eq.bands[2].slope);
    
    apvts.addParameterListener(ParameterID::attackValue.getParamID(), this);
    apvts.addParameterListener(ParameterID::holdValue.getParamID(), this);
    apvts.addParameterListener(ParameterID::releaseValue.getParamID(), this);
    apvts.addParameterListener(ParameterID::thresholdValue.getParamID(), this);
    apvts.addParameterListener(ParameterID::kneeValue.getParamID(), this);
    apvts.addParameterListener(ParameterID::ratioValue.getParamID(), this);
    apvts.addParameterListener(ParameterID::makeUpGainValue.getParamID(), this);
    apvts.addParameterListener(ParameterID::estimationTypeValue.getParamID(), this);
    apvts.addParameterListener(ParameterID::externalSideChain.getParamID(), this);
    apvts.addParameterListener(ParameterID::bypassValue.getParamID(), this);
    apvts.addParameterListener(ParameterID::eqBandActive1.getParamID(), this);
    apvts.addParameterListener(ParameterID::eqBandFreq1.getParamID(), this);
    apvts.addParameterListener(ParameterID::eqBandGain1.getParamID(), this);
    apvts.addParameterListener(ParameterID::eqBandQuality1.getParamID(), this);
    apvts.addParameterListener(ParameterID::eqBandType1.getParamID(), this);
    apvts.addParameterListener(ParameterID::eqBandSlope1.getParamID(), this);
    apvts.addParameterListener(ParameterID::eqBandActive2.getParamID(), this);
    apvts.addParameterListener(ParameterID::eqBandFreq2.getParamID(), this);
    apvts.addParameterListener(ParameterID::eqBandGain2.getParamID(), this);
    apvts.addParameterListener(ParameterID::eqBandQuality2.getParamID(), this);
    apvts.addParameterListener(ParameterID::eqBandType2.getParamID(), this);
    apvts.addParameterListener(ParameterID::eqBandSlope2.getParamID(), this);
    apvts.addParameterListener(ParameterID::eqBandActive3.getParamID(), this);
    apvts.addParameterListener(ParameterID::eqBandFreq3.getParamID(), this);
    apvts.addParameterListener(ParameterID::eqBandGain3.getParamID(), this);
    apvts.addParameterListener(ParameterID::eqBandQuality3.getParamID(), this);
    apvts.addParameterListener(ParameterID::eqBandType3.getParamID(), this);
    apvts.addParameterListener(ParameterID::eqBandSlope3.getParamID(), this);
}

Simple_compAudioProcessor::~Simple_compAudioProcessor()
{
    apvts.removeParameterListener(ParameterID::attackValue.getParamID(), this);
    apvts.removeParameterListener(ParameterID::holdValue.getParamID(), this);
    apvts.removeParameterListener(ParameterID::releaseValue.getParamID(), this);
    apvts.removeParameterListener(ParameterID::thresholdValue.getParamID(), this);
    apvts.removeParameterListener(ParameterID::kneeValue.getParamID(), this);
    apvts.removeParameterListener(ParameterID::ratioValue.getParamID(), this);
    apvts.removeParameterListener(ParameterID::makeUpGainValue.getParamID(), this);
    apvts.removeParameterListener(ParameterID::estimationTypeValue.getParamID(), this);
    apvts.removeParameterListener(ParameterID::externalSideChain.getParamID(), this);
    apvts.removeParameterListener(ParameterID::bypassValue.getParamID(), this);
    apvts.removeParameterListener(ParameterID::eqBandActive1.getParamID(), this);
    apvts.removeParameterListener(ParameterID::eqBandFreq1.getParamID(), this);
    apvts.removeParameterListener(ParameterID::eqBandGain1.getParamID(), this);
    apvts.removeParameterListener(ParameterID::eqBandQuality1.getParamID(), this);
    apvts.removeParameterListener(ParameterID::eqBandType1.getParamID(), this);
    apvts.removeParameterListener(ParameterID::eqBandSlope1.getParamID(), this);
    apvts.removeParameterListener(ParameterID::eqBandActive2.getParamID(), this);
    apvts.removeParameterListener(ParameterID::eqBandFreq2.getParamID(), this);
    apvts.removeParameterListener(ParameterID::eqBandGain2.getParamID(), this);
    apvts.removeParameterListener(ParameterID::eqBandQuality2.getParamID(), this);
    apvts.removeParameterListener(ParameterID::eqBandType2.getParamID(), this);
    apvts.removeParameterListener(ParameterID::eqBandSlope2.getParamID(), this);
    apvts.removeParameterListener(ParameterID::eqBandActive3.getParamID(), this);
    apvts.removeParameterListener(ParameterID::eqBandFreq3.getParamID(), this);
    apvts.removeParameterListener(ParameterID::eqBandGain3.getParamID(), this);
    apvts.removeParameterListener(ParameterID::eqBandQuality3.getParamID(), this);
    apvts.removeParameterListener(ParameterID::eqBandType3.getParamID(), this);
    apvts.removeParameterListener(ParameterID::eqBandSlope3.getParamID(), this);
}

//==============================================================================
const juce::String Simple_compAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Simple_compAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Simple_compAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Simple_compAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Simple_compAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Simple_compAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Simple_compAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Simple_compAudioProcessor::setCurrentProgram (int index)
{
    
}

const juce::String Simple_compAudioProcessor::getProgramName (int index)
{
    return {};
}

void Simple_compAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Simple_compAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumOutputChannels();
    comp.prepare(spec);
    outputBuffer.setSize(spec.numChannels, spec.maximumBlockSize);
}

void Simple_compAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Simple_compAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void Simple_compAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    auto blockSize = getBlockSize();
    inputBuffer = this->getBusBuffer(buffer, true, 0);
    inputSideChainBuffer = this->getBusBuffer(buffer, true, 1);
    auto inputBlock = juce::dsp::AudioBlock<float> (inputBuffer);
    auto outputBlock = juce::dsp::AudioBlock<float> (outputBuffer);
    auto inputSideChainBlock = juce::dsp::AudioBlock<float> (inputSideChainBuffer);
    auto outputSideChainBlock = juce::dsp::AudioBlock<float> (outputSideChainBuffer);
    auto processContext = juce::dsp::ProcessContextNonReplacing<float> (inputBlock, outputBlock);
    auto sideChainProcessContext = juce::dsp::ProcessContextNonReplacing<float> (inputSideChainBlock, outputSideChainBlock);
    
    if (!is_bypass) comp.processBlock(processContext, sideChainProcessContext);
    else outputBlock.copyFrom(inputBlock);

    auto output_left = buffer.getWritePointer(0);
    auto output_right = buffer.getWritePointer(1);
    for (int n = 0; n < blockSize; n++) {
        output_left[n] = outputBlock.getSample(0, n);
        output_right[n] = outputBlock.getSample(1, n);
    }

}

//==============================================================================
bool Simple_compAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Simple_compAudioProcessor::createEditor()
{
    auto editor = new juce::GenericAudioProcessorEditor(*this);
    editor->setSize(500, 750);
    return editor;
}

//==============================================================================
void Simple_compAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Simple_compAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Simple_compAudioProcessor();
}

std::vector<std::unique_ptr<juce::RangedAudioParameter>> createEqualiserParams() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> eqParams;
    
    float freqSkewFactor = std::log (0.5f) / std::log (980.0f / 19980.0f);
    float qualitySkewFactor = std::log (0.5f) / std::log (0.9f / 9.9f);
    // Band 1
    eqParams.push_back(std::make_unique<juce::AudioParameterFloat>(ParameterID::eqBandFreq1, "Freq Band 1",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, freqSkewFactor), 100.0f));
    eqParams.push_back(std::make_unique<juce::AudioParameterFloat>(ParameterID::eqBandQuality1, "Quality Factor Band 1",
        juce::NormalisableRange<float>(0.1f, 10.0f, 1.0f, qualitySkewFactor), 1.0f));
    //eqParams.push_back(std::make_unique<juce::AudioParameterFloat>(ParameterID::eqBandGain1, "Gain Band 1",
    //    juce::NormalisableRange<float>(-20.0f, 20.0f, 0.0f, 1.0f), 0.0f));
    //eqParams.push_back(std::make_unique<juce::AudioParameterChoice>(ParameterID::eqBandType1, "Type Filter Band 1", juce::StringArray("Lowpass", "Lowshelf", "Peak", "Notch", "Highpass", "Highshelf"), 0));
    eqParams.push_back(std::make_unique<juce::AudioParameterChoice>(ParameterID::eqBandSlope1, "Slope Filter Band 1",
        juce::StringArray("12 db/oct", "24 db/oct", "36 db/oct", "48 db/oct"), 0));
    eqParams.push_back(std::make_unique<juce::AudioParameterBool>(ParameterID::eqBandActive1, "Active Filter Band 1", false));
    // Band 2
    eqParams.push_back(std::make_unique<juce::AudioParameterFloat>(ParameterID::eqBandFreq2, "Freq Band 2",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, freqSkewFactor), 100.0f));
    eqParams.push_back(std::make_unique<juce::AudioParameterFloat>(ParameterID::eqBandQuality2, "Quality Factor Band 2",
        juce::NormalisableRange<float>(0.1f, 10.0f, 1.0f, qualitySkewFactor), 1.0f));
    eqParams.push_back(std::make_unique<juce::AudioParameterFloat>(ParameterID::eqBandGain2, "Gain Band 2",
        juce::NormalisableRange<float>(-20.0f, 20.0f, 0.0f, 1.0f), 0.0f));
    //eqParams.push_back(std::make_unique<juce::AudioParameterChoice>(ParameterID::eqBandType2, "Type Filter Band 2", juce::StringArray("Lowpass", "Lowshelf", "Peak", "Notch", "Highpass", "Highshelf"), 0));
    //eqParams.push_back(std::make_unique<juce::AudioParameterChoice>(ParameterID::eqBandSlope2, "Slope Filter Band 2",
    //    juce::StringArray("12 db/oct", "24 db/oct", "36 db/oct", "48 db/oct"), 0));
    eqParams.push_back(std::make_unique<juce::AudioParameterBool>(ParameterID::eqBandActive2, "Active Filter Band 2", false));
    // Band 3
    eqParams.push_back(std::make_unique<juce::AudioParameterFloat>(ParameterID::eqBandFreq3, "Freq Band 3",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, freqSkewFactor), 100.0f));
    eqParams.push_back(std::make_unique<juce::AudioParameterFloat>(ParameterID::eqBandQuality3, "Quality Factor Band 3",
        juce::NormalisableRange<float>(0.1f, 10.0f, 1.0f, qualitySkewFactor), 1.0f));
    //eqParams.push_back(std::make_unique<juce::AudioParameterFloat>(ParameterID::eqBandGain3, "Gain Band 3",
    //    juce::NormalisableRange<float>(-20.0f, 20.0f, 0.0f, 1.0f), 0.0f));
    eqParams.push_back(std::make_unique<juce::AudioParameterChoice>(ParameterID::eqBandType3, "Type Filter Band 3", juce::StringArray("Lowpass", "Lowshelf", "Peak", "Notch", "Highpass", "Highshelf"), 0));
    eqParams.push_back(std::make_unique<juce::AudioParameterChoice>(ParameterID::eqBandSlope3, "Slope Filter Band 3",
        juce::StringArray("12 db/oct", "24 db/oct", "36 db/oct", "48 db/oct"), 0));
    eqParams.push_back(std::make_unique<juce::AudioParameterBool>(ParameterID::eqBandActive3, "Active Filter Band 3", false));
    
    return eqParams;
}

juce::AudioProcessorValueTreeState::ParameterLayout Simple_compAudioProcessor::createParameters() {

    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> eqParams = createEqualiserParams();
    juce::AudioProcessorValueTreeState::ParameterLayout paramsLayout;
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ParameterID::attackValue, "Attack Time", juce::NormalisableRange<float>(0.0001f, 0.5f, 0.0001f, 0.8f), 0.010f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ParameterID::holdValue, "Hold Time", juce::NormalisableRange<float>(0.0f, 0.5f, 0.001f, 0.8f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ParameterID::releaseValue, "Release Time", juce::NormalisableRange<float>(0.001f, 1.0f, 0.001f, 0.8f), 0.050f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ParameterID::thresholdValue, "Threshold", juce::NormalisableRange<float>(-80.0f, 0.0f, 0.1f, 1.0f), -6.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ParameterID::ratioValue, "Ratio", juce::NormalisableRange<float>(1.0f, 20.0f, 0.1f, 1.0f), 2.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ParameterID::kneeValue, "Knee", juce::NormalisableRange<float>(0.0f, 12.0f, 0.1f, 1.0f), 6.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ParameterID::makeUpGainValue, "Make Up Gain", juce::NormalisableRange<float>(0.0f, 20.0f, 0.1f, 1.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(ParameterID::estimationTypeValue, "Estimation Type", juce::StringArray("Peak", "RMS"), 1));
    params.push_back(std::make_unique<juce::AudioParameterBool>(ParameterID::externalSideChain, "External Side Chain", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(ParameterID::bypassValue, "Bypass", false));
    params.insert(params.end(), std::make_move_iterator(eqParams.begin()), std::make_move_iterator(eqParams.end()));
    paramsLayout.add(params.begin(), params.end());
    
    return paramsLayout;
}

void Simple_compAudioProcessor::parameterChanged(const juce::String& paramId, float newValue) {
    if (paramId == ParameterID::attackValue.getParamID()) {
        comp.setAttack(static_cast<float>(newValue));
        return;
    }
    
    if (paramId == ParameterID::holdValue.getParamID()) {
        comp.setHold(static_cast<float>(newValue));
        return;
    }
    
    if (paramId == ParameterID::releaseValue.getParamID()) {
        comp.setRelease(static_cast<float>(newValue));
        return;
    }

    if (paramId == ParameterID::thresholdValue.getParamID()) {
        comp.setThreshold(static_cast<float>(newValue));
        return;
    }
    
    if (paramId == ParameterID::kneeValue.getParamID()) {
        comp.setKnee(static_cast<float>(newValue));
        return;
    }
    
    if (paramId == ParameterID::ratioValue.getParamID()) {
        comp.setRatio(static_cast<float>(newValue));
        return;
    }
    
    if (paramId == ParameterID::makeUpGainValue.getParamID()) {
        comp.setMakeUpGain(static_cast<float>(newValue));
        return;
    }
    
    if (paramId == ParameterID::estimationTypeValue.getParamID()) {
        comp.setEstimationType(static_cast<juce::dsp::BallisticsFilterLevelCalculationType>(newValue));
        return;
    }
    
    if (paramId == ParameterID::externalSideChain.getParamID()) {
        bool value = static_cast<bool>(newValue);
        comp.setExternalSideChain(value);
        return;
    }
    
    if (paramId == ParameterID::bypassValue.getParamID()) {
        is_bypass = static_cast<bool>(newValue);
        return;
    }
    
    if (paramId == ParameterID::eqBandActive1.getParamID()) {
        comp.setEqBandBypass(0, static_cast<bool>(newValue));
        return;
    }
    
    if (paramId == ParameterID::eqBandActive2.getParamID()) {
        comp.setEqBandBypass(1, static_cast<bool>(newValue));
        return;
    }
    
    if (paramId == ParameterID::eqBandActive3.getParamID()) {
        comp.setEqBandBypass(2, static_cast<bool>(newValue));
        return;
    }
    
    if (paramId == ParameterID::eqBandFreq1.getParamID()) {
        FilterParams params = comp.eq.getBandParams(0);
        params.freq = static_cast<float>(newValue);
        comp.setEqBandParams(0, params);
        return;
    }
    
    if (paramId == ParameterID::eqBandFreq2.getParamID()) {
        FilterParams params = comp.eq.getBandParams(1);
        params.freq = static_cast<float>(newValue);
        comp.setEqBandParams(1, params);
        return;
    }
    
    if (paramId == ParameterID::eqBandFreq3.getParamID()) {
        FilterParams params = comp.eq.getBandParams(2);
        params.freq = static_cast<float>(newValue);
        comp.setEqBandParams(2, params);
        return;
    }
    
    if (paramId == ParameterID::eqBandQuality1.getParamID()) {
        FilterParams params = comp.eq.getBandParams(0);
        params.quality = static_cast<float>(newValue);
        comp.setEqBandParams(0, params);
        return;
    }
    
    if (paramId == ParameterID::eqBandQuality2.getParamID()) {
        FilterParams params = comp.eq.getBandParams(1);
        params.quality = static_cast<float>(newValue);
        comp.setEqBandParams(1, params);
        return;
    }
    
    if (paramId == ParameterID::eqBandQuality3.getParamID()) {
        FilterParams params = comp.eq.getBandParams(2);
        params.quality = static_cast<float>(newValue);
        comp.setEqBandParams(2, params);
        return;
    }
    
    if (paramId == ParameterID::eqBandGain1.getParamID()) {
        FilterParams params = comp.eq.getBandParams(0);
        params.gainDb = static_cast<float>(newValue);
        comp.setEqBandParams(0, params);
        return;
    }
    
    if (paramId == ParameterID::eqBandGain2.getParamID()) {
        FilterParams params = comp.eq.getBandParams(1);
        params.gainDb = static_cast<float>(newValue);
        comp.setEqBandParams(1, params);
        return;
    }
    
    if (paramId == ParameterID::eqBandGain3.getParamID()) {
        FilterParams params = comp.eq.getBandParams(2);
        params.gainDb = static_cast<float>(newValue);
        comp.setEqBandParams(2, params);
        return;
    }
    
    if (paramId == ParameterID::eqBandSlope1.getParamID()) {
        FilterParams params = comp.eq.getBandParams(0);
        params.slope = static_cast<FilterSlope>(newValue);
        comp.setEqBandParams(0, params);
        return;
    }
    
    if (paramId == ParameterID::eqBandSlope2.getParamID()) {
        FilterParams params = comp.eq.getBandParams(1);
        params.slope = static_cast<FilterSlope>(newValue);
        comp.setEqBandParams(1, params);
        return;
    }
    
    if (paramId == ParameterID::eqBandSlope3.getParamID()) {
        FilterParams params = comp.eq.getBandParams(2);
        params.slope = static_cast<FilterSlope>(newValue);
        comp.setEqBandParams(2, params);
        return;
    }
    
    if (paramId == ParameterID::eqBandType1.getParamID()) {
        FilterParams params = comp.eq.getBandParams(0);
        params.type = static_cast<FilterType>(newValue);
        comp.setEqBandParams(0, params);
        return;
    }
    
    if (paramId == ParameterID::eqBandType2.getParamID()) {
        FilterParams params = comp.eq.getBandParams(1);
        params.type = static_cast<FilterType>(newValue);
        comp.setEqBandParams(1, params);
        return;
    }
    
    if (paramId == ParameterID::eqBandType3.getParamID()) {
        FilterParams params = comp.eq.getBandParams(2);
        params.type = static_cast<FilterType>(newValue);
        comp.setEqBandParams(2, params);
        return;
    }
    
}
