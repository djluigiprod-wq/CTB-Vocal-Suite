#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class CTBVocalSuiteAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit CTBVocalSuiteAudioProcessorEditor(CTBVocalSuiteAudioProcessor&);
    ~CTBVocalSuiteAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    CTBVocalSuiteAudioProcessor& processor;

    juce::Label title;
    juce::Label subtitle;
    juce::TextButton resetButton { "RESET" };

    struct Knob
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    Knob input, hpf, presence, air, threshold, ratio, attack, release, deess, saturation, output, mix;

    void setupKnob(Knob&, const juce::String& parameterId, const juce::String& text);
    void resetParameters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CTBVocalSuiteAudioProcessorEditor)
};
