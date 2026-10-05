#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    using APVTS = juce::AudioProcessorValueTreeState;

    APVTS::ParameterLayout makeLayout()
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

        p.push_back(std::make_unique<juce::AudioParameterFloat>("INPUT", "Input", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.01f), 0.0f, " dB"));
        p.push_back(std::make_unique<juce::AudioParameterFloat>("HPF", "High Pass", juce::NormalisableRange<float>(20.0f, 500.0f, 1.0f, 0.35f), 80.0f, " Hz"));
        p.push_back(std::make_unique<juce::AudioParameterFloat>("PRESENCE", "Presence", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.01f), 0.0f, " dB"));
        p.push_back(std::make_unique<juce::AudioParameterFloat>("AIR", "Air", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.01f), 0.0f, " dB"));
        p.push_back(std::make_unique<juce::AudioParameterFloat>("COMP_THRESHOLD", "Comp Threshold", juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f), -18.0f, " dB"));
        p.push_back(std::make_unique<juce::AudioParameterFloat>("COMP_RATIO", "Comp Ratio", juce::NormalisableRange<float>(1.0f, 20.0f, 0.01f), 3.0f, ":1"));
        p.push_back(std::make_unique<juce::AudioParameterFloat>("COMP_ATTACK", "Comp Attack", juce::NormalisableRange<float>(0.1f, 100.0f, 0.1f, 0.4f), 10.0f, " ms"));
        p.push_back(std::make_unique<juce::AudioParameterFloat>("COMP_RELEASE", "Comp Release", juce::NormalisableRange<float>(10.0f, 1000.0f, 1.0f, 0.4f), 100.0f, " ms"));
        p.push_back(std::make_unique<juce::AudioParameterFloat>("DEESS", "De-Esser", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 25.0f, "%"));
        p.push_back(std::make_unique<juce::AudioParameterFloat>("SATURATION", "Saturation", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 10.0f, "%"));
        p.push_back(std::make_unique<juce::AudioParameterFloat>("OUTPUT", "Output", juce::NormalisableRange<float>(-24.0f, 12.0f, 0.01f), 0.0f, " dB"));
        p.push_back(std::make_unique<juce::AudioParameterFloat>("MIX", "Mix", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 100.0f, "%"));

        return { p.begin(), p.end() };
    }
}

CTBVocalSuiteAudioProcessor::CTBVocalSuiteAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout CTBVocalSuiteAudioProcessor::createParameterLayout()
{
    return makeLayout();
}

bool CTBVocalSuiteAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& input = layouts.getChannelSet(true, 0);
    const auto& output = layouts.getChannelSet(false, 0);

    if (output != juce::AudioChannelSet::mono() && output != juce::AudioChannelSet::stereo())
        return false;

    return input == output;
}

void CTBVocalSuiteAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    const juce::dsp::ProcessSpec spec
    {
        sampleRate,
        static_cast<juce::uint32>(samplesPerBlock),
        static_cast<juce::uint32>(getTotalNumOutputChannels())
    };

    highPass.prepare(spec);
    compressor.prepare(spec);
    saturator.prepare(spec);
    presenceFilter.prepare(spec);
    airFilter.prepare(spec);
    deEssFilter.prepare(spec);
    limiter.prepare(spec);
    outputGain.prepare(spec);

    inputGain.reset(sampleRate, 0.02);
    mixValue.reset(sampleRate, 0.02);
    inputGain.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(apvts.getRawParameterValue("INPUT")->load()));
    mixValue.setCurrentAndTargetValue(apvts.getRawParameterValue("MIX")->load() * 0.01f);
    deEssEnvelope = 0.0f;

    updateDSPParameters();

    highPass.reset();
    compressor.reset();
    saturator.reset();
    presenceFilter.reset();
    airFilter.reset();
    deEssFilter.reset();
    limiter.reset();
    outputGain.reset();
}

void CTBVocalSuiteAudioProcessor::releaseResources()
{
    highPass.reset();
    compressor.reset();
    saturator.reset();
    presenceFilter.reset();
    airFilter.reset();
    deEssFilter.reset();
    limiter.reset();
    outputGain.reset();
}

void CTBVocalSuiteAudioProcessor::updateDSPParameters()
{
    const auto sampleRate = getSampleRate();
    if (sampleRate <= 0.0)
        return;

    const auto hpf = apvts.getRawParameterValue("HPF")->load();
    *highPass.state = *Coefficients::makeHighPass(sampleRate, hpf, 0.7071f);

    const auto presence = apvts.getRawParameterValue("PRESENCE")->load();
    *presenceFilter.state = *Coefficients::makePeakFilter(
        sampleRate, 3500.0, 0.9, juce::Decibels::decibelsToGain(presence));

    const auto air = apvts.getRawParameterValue("AIR")->load();
    *airFilter.state = *Coefficients::makeHighShelf(
        sampleRate, 10000.0, 0.7071, juce::Decibels::decibelsToGain(air));

    compressor.setThreshold(apvts.getRawParameterValue("COMP_THRESHOLD")->load());
    compressor.setRatio(apvts.getRawParameterValue("COMP_RATIO")->load());
    compressor.setAttack(apvts.getRawParameterValue("COMP_ATTACK")->load());
    compressor.setRelease(apvts.getRawParameterValue("COMP_RELEASE")->load());

    const auto saturation = apvts.getRawParameterValue("SATURATION")->load() * 0.01f;
    saturator.functionToUse = [saturation](float x)
    {
        const float drive = 1.0f + saturation * 7.0f;
        const float makeup = 1.0f / std::tanh(drive);
        return std::tanh(x * drive) * makeup;
    };

    *deEssFilter.state = *Coefficients::makeHighPass(sampleRate, 5500.0, 0.7071f);

    limiter.setThreshold(-1.0f);
    limiter.setRelease(80.0f);
    outputGain.setGainDecibels(apvts.getRawParameterValue("OUTPUT")->load());
}

void CTBVocalSuiteAudioProcessor::processDeEsser(juce::AudioBuffer<float>& buffer, float amount)
{
    if (amount <= 0.0001f)
        return;

    juce::AudioBuffer<float> highBand(buffer.getNumChannels(), buffer.getNumSamples());
    highBand.makeCopyOf(buffer);

    juce::dsp::AudioBlock<float> block(highBand);
    juce::dsp::ProcessContextReplacing<float> context(block);
    deEssFilter.process(context);

    const auto sr = static_cast<float>(getSampleRate());
    const float attackCoeff = std::exp(-1.0f / (0.0015f * sr));
    const float releaseCoeff = std::exp(-1.0f / (0.045f * sr));
    const float threshold = juce::Decibels::decibelsToGain(-28.0f);
    const float maxReductionDb = 18.0f * amount;

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float detector = 0.0f;
        for (int channel = 0; channel < highBand.getNumChannels(); ++channel)
            detector = juce::jmax(detector, std::abs(highBand.getSample(channel, sample)));

        if (detector > deEssEnvelope)
            deEssEnvelope = attackCoeff * deEssEnvelope + (1.0f - attackCoeff) * detector;
        else
            deEssEnvelope = releaseCoeff * deEssEnvelope + (1.0f - releaseCoeff) * detector;

        const float over = juce::jmax(0.0f, deEssEnvelope - threshold);
        const float normalised = juce::jlimit(0.0f, 1.0f, over / (1.0f - threshold));
        const float reductionDb = normalised * maxReductionDb;
        const float reduction = juce::Decibels::decibelsToGain(-reductionDb);

        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            const float original = buffer.getSample(channel, sample);
            const float sibilant = highBand.getSample(channel, sample);
            buffer.setSample(channel, sample, original + sibilant * (reduction - 1.0f));
        }
    }
}

void CTBVocalSuiteAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ignoreUnused(midi);
    juce::ScopedNoDenormals noDenormals;

    for (int channel = getTotalNumInputChannels(); channel < getTotalNumOutputChannels(); ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());

    juce::AudioBuffer<float> dry(buffer.getNumChannels(), buffer.getNumSamples());
    dry.makeCopyOf(buffer);

    const float inputDb = apvts.getRawParameterValue("INPUT")->load();
    const float mix = apvts.getRawParameterValue("MIX")->load() * 0.01f;
    const float deEss = apvts.getRawParameterValue("DEESS")->load() * 0.01f;

    inputGain.setTargetValue(juce::Decibels::decibelsToGain(inputDb));
    mixValue.setTargetValue(mix);
    updateDSPParameters();

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

    // Input gain.
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            buffer.setSample(channel, sample, buffer.getSample(channel, sample) * inputGain.getNextValue());

    highPass.process(context);
    compressor.process(context);
    saturator.process(context);
    presenceFilter.process(context);
    airFilter.process(context);
    processDeEsser(buffer, deEss);
    limiter.process(context);
    outputGain.process(context);

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            const float wet = buffer.getSample(channel, sample);
            const float d = dry.getSample(channel, sample);
            const float mixNow = mixValue.getNextValue();
            buffer.setSample(channel, sample, d + (wet - d) * mixNow);
        }
}

void CTBVocalSuiteAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void CTBVocalSuiteAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CTBVocalSuiteAudioProcessor();
}
