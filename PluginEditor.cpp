#include "PluginEditor.h"

CTBVocalSuiteAudioProcessorEditor::CTBVocalSuiteAudioProcessorEditor(CTBVocalSuiteAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(1000, 610);

    title.setText("CTB VOCAL SUITE", juce::dontSendNotification);
    title.setFont(juce::Font(30.0f, juce::Font::bold));
    title.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(title);

    subtitle.setText("CAPTURE THE BEATS  •  VOCAL PROCESSOR", juce::dontSendNotification);
    subtitle.setFont(juce::Font(13.0f));
    subtitle.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(subtitle);

    setupKnob(input, "INPUT", "INPUT");
    setupKnob(hpf, "HPF", "HPF");
    setupKnob(presence, "PRESENCE", "PRESENCE");
    setupKnob(air, "AIR", "AIR");
    setupKnob(threshold, "COMP THRESH", "COMP THRESH");
    setupKnob(ratio, "COMP RATIO", "COMP RATIO");
    setupKnob(attack, "COMP ATTACK", "ATTACK");
    setupKnob(release, "COMP RELEASE", "RELEASE");
    setupKnob(deess, "DEESS", "DE-ESSER");
    setupKnob(saturation, "SATURATION", "SATURATION");
    setupKnob(output, "OUTPUT", "OUTPUT");
    setupKnob(mix, "MIX", "MIX");

    resetButton.onClick = [this] { resetParameters(); };
    addAndMakeVisible(resetButton);
}

void CTBVocalSuiteAudioProcessorEditor::setupKnob(Knob& k, const juce::String& id, const juce::String& text)
{
    k.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    k.slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 85, 22);
    k.slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::white);
    k.slider.setColour(juce::Slider::thumbColourId, juce::Colours::white);
    k.slider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    k.slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff202020));
    k.slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff444444));
    addAndMakeVisible(k.slider);

    k.label.setText(text, juce::dontSendNotification);
    k.label.setJustificationType(juce::Justification::centred);
    k.label.setFont(juce::Font(11.0f, juce::Font::bold));
    k.label.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(k.label);

    k.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getAPVTS(), id, k.slider);
}

void CTBVocalSuiteAudioProcessorEditor::resetParameters()
{
    auto& state = processor.getAPVTS();
    state.getParameter("INPUT")->setValueNotifyingHost(state.getParameterRange("INPUT").convertTo0to1(0.0f));
    state.getParameter("HPF")->setValueNotifyingHost(state.getParameterRange("HPF").convertTo0to1(80.0f));
    state.getParameter("PRESENCE")->setValueNotifyingHost(state.getParameterRange("PRESENCE").convertTo0to1(0.0f));
    state.getParameter("AIR")->setValueNotifyingHost(state.getParameterRange("AIR").convertTo0to1(0.0f));
    state.getParameter("COMP_THRESHOLD")->setValueNotifyingHost(state.getParameterRange("COMP_THRESHOLD").convertTo0to1(-18.0f));
    state.getParameter("COMP_RATIO")->setValueNotifyingHost(state.getParameterRange("COMP_RATIO").convertTo0to1(3.0f));
    state.getParameter("COMP_ATTACK")->setValueNotifyingHost(state.getParameterRange("COMP_ATTACK").convertTo0to1(10.0f));
    state.getParameter("COMP_RELEASE")->setValueNotifyingHost(state.getParameterRange("COMP_RELEASE").convertTo0to1(100.0f));
    state.getParameter("DEESS")->setValueNotifyingHost(state.getParameterRange("DEESS").convertTo0to1(25.0f));
    state.getParameter("SATURATION")->setValueNotifyingHost(state.getParameterRange("SATURATION").convertTo0to1(10.0f));
    state.getParameter("OUTPUT")->setValueNotifyingHost(state.getParameterRange("OUTPUT").convertTo0to1(0.0f));
    state.getParameter("MIX")->setValueNotifyingHost(state.getParameterRange("MIX").convertTo0to1(100.0f));
}

void CTBVocalSuiteAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff0d0d0f));
    g.setColour(juce::Colour(0xff18181c));
    g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(18.0f), 18.0f);

    g.setColour(juce::Colour(0xff303038));
    g.drawLine(25.0f, 92.0f, getWidth() - 25.0f, 92.0f, 1.0f);
    g.drawLine(25.0f, 335.0f, getWidth() - 25.0f, 335.0f, 1.0f);

    g.setColour(juce::Colours::grey);
    g.setFont(11.0f);
    g.drawText("TONE", 30, 345, 80, 20, juce::Justification::left);
    g.drawText("DYNAMICS", 265, 345, 100, 20, juce::Justification::left);
    g.drawText("COLOR", 520, 345, 80, 20, juce::Justification::left);
    g.drawText("OUTPUT", 750, 345, 80, 20, juce::Justification::left);
}

void CTBVocalSuiteAudioProcessorEditor::resized()
{
    title.setBounds(30, 22, 400, 38);
    subtitle.setBounds(32, 59, 400, 22);
    resetButton.setBounds(getWidth() - 105, 30, 75, 30);

    const int y1 = 115;
    const int y2 = 385;
    const int cellW = 155;
    const int knobW = 130;
    const int knobH = 150;

    std::array<Knob*, 4> row1 { &input, &hpf, &presence, &air };
    for (size_t i = 0; i < row1.size(); ++i)
    {
        row1[i]->slider.setBounds(25 + static_cast<int>(i) * cellW + 12, y1, knobW, knobH);
        row1[i]->label.setBounds(25 + static_cast<int>(i) * cellW + 12, y1 - 5, knobW, 22);
    }

    std::array<Knob*, 8> row2 { &threshold, &ratio, &attack, &release, &deess, &saturation, &output, &mix };
    const int w2 = (getWidth() - 50) / 8;
    for (size_t i = 0; i < row2.size(); ++i)
    {
        row2[i]->slider.setBounds(25 + static_cast<int>(i) * w2 + 2, y2, w2 - 4, 155);
        row2[i]->label.setBounds(25 + static_cast<int>(i) * w2 + 2, y2 - 5, w2 - 4, 22);
    }
}
