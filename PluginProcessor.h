#pragma once

#include <JuceHeader.h>

class CTBVocalSuiteAudioProcessor : public juce::AudioProcessor
{
public:
    CTBVocalSuiteAudioProcessor();
    ~CTBVocalSuiteAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    using Filter = juce::dsp::IIR::Filter<float>;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;
    using StereoFilter = juce::dsp::ProcessorDuplicator<Filter, Coefficients>;

    juce::AudioProcessorValueTreeState apvts;

    StereoFilter highPass;
    juce::dsp::Compressor<float> compressor;
    juce::dsp::WaveShaper<float> saturator;
    StereoFilter presenceFilter;
    StereoFilter airFilter;
    StereoFilter deEssFilter;
    juce::dsp::Limiter<float> limiter;
    juce::dsp::Gain<float> outputGain;

    juce::SmoothedValue<float> inputGain;
    juce::SmoothedValue<float> mixValue;
    float deEssEnvelope = 0.0f;

    void updateDSPParameters();
    void processDeEsser(juce::AudioBuffer<float>& buffer, float amount);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CTBVocalSuiteAudioProcessor)
};
