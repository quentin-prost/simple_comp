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
                       ), apvts(*this, nullptr, "Parameters", createParameters()), comp(), outputBuffer()
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
//    castParameter(apvts, ParameterID::estimationTypeValue, estimationType);
    castParameter(apvts, ParameterID::bypassValue, params.bypass);
    
    apvts.addParameterListener(ParameterID::attackValue.getParamID(), this);
    apvts.addParameterListener(ParameterID::holdValue.getParamID(), this);
    apvts.addParameterListener(ParameterID::releaseValue.getParamID(), this);
    apvts.addParameterListener(ParameterID::thresholdValue.getParamID(), this);
    apvts.addParameterListener(ParameterID::kneeValue.getParamID(), this);
    apvts.addParameterListener(ParameterID::ratioValue.getParamID(), this);
    apvts.addParameterListener(ParameterID::makeUpGainValue.getParamID(), this);
//    apvts.addParameterListener(ParameterID::estimationTypeValue.getParamID(), this);
    apvts.addParameterListener(ParameterID::bypassValue.getParamID(), this);
    
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
//    apvts.removeParameterListener(ParameterID::estimationTypeValue.getParamID(), this);
    apvts.removeParameterListener(ParameterID::bypassValue.getParamID(), this);
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
    auto inputBlock = juce::dsp::AudioBlock<float> (buffer);
    auto outputBlock = juce::dsp::AudioBlock<float> (outputBuffer);
    auto processContext = juce::dsp::ProcessContextNonReplacing<float> (inputBlock, outputBlock);
    auto output_left = buffer.getWritePointer(0);
    auto output_right = buffer.getWritePointer(1);
    
    comp.processBlock(processContext);
    
    for (int n = 0; n < blockSize; n++) {
        output_left[n] = outputBlock.getSample(0, n);
        output_right[n] = outputBlock.getSample(1, n);
    }
//    limitOutput(buffer.getWritePointer(0), buffer.getNumSamples());
//    limitOutput(buffer.getWritePointer(1), buffer.getNumSamples());
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

juce::AudioProcessorValueTreeState::ParameterLayout Simple_compAudioProcessor::createParameters() {

    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    juce::AudioProcessorValueTreeState::ParameterLayout paramsLayout;
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ParameterID::attackValue, "Attack Time", juce::NormalisableRange<float>(0.0001f, 0.5f, 0.0001f, 0.8f), 0.010f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ParameterID::holdValue, "Hold Time", juce::NormalisableRange<float>(0.0f, 0.5f, 0.001f, 0.8f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ParameterID::releaseValue, "Release Time", juce::NormalisableRange<float>(0.001f, 1.0f, 0.001f, 0.8f), 0.050f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ParameterID::thresholdValue, "Threshold", juce::NormalisableRange<float>(-80.0f, 0.0f, 0.1f, 1.0f), -6.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ParameterID::ratioValue, "Ratio", juce::NormalisableRange<float>(1.0f, 20.0f, 0.1f, 1.0f), 2.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ParameterID::kneeValue, "Knee", juce::NormalisableRange<float>(0.0f, 12.0f, 0.1f, 1.0f), 6.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ParameterID::makeUpGainValue, "Make Up Gain", juce::NormalisableRange<float>(0.0f, 20.0f, 0.1f, 1.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(ParameterID::bypassValue, "Bypass", false));
    
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
    
    if (paramId == ParameterID::bypassValue.getParamID()) {
        comp.setBypass(static_cast<bool>(newValue));
        return;
    }
}
