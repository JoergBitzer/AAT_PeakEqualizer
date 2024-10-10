#pragma once

#include <vector>
#include <juce_audio_processors/juce_audio_processors.h>
#include "tools/AudioProcessParameter.h"
#include "tools/SynchronBlockProcessor.h"
#include "PluginSettings.h"


// This is how we define our parameter as globals to use it in the audio processor as well as in the editor
const struct
{
	const std::string ID = "GainID";
	const std::string name = "Gain";
	const std::string unitName = " dB";
	const float minValue = -24.f;
	const float maxValue = 24.f;
	const float defaultValue = 0.f;
}g_paramGain;
const struct
{
	const std::string ID = "QID";
	const std::string name = "Q";
	const std::string unitName = "";
	const float minValue = logf(0.1f);
	const float maxValue = logf(10.f);
	const float defaultValue = logf(1.0f);
}g_paramQ;
const struct
{
	const std::string ID = "FreqID";
	const std::string name = "Freq";
	const std::string unitName = " Hz";
	const float minValue = logf(50.f);
	const float maxValue = logf(15000.f);
	const float defaultValue = logf(1000.f);
}g_paramFreq;


class PeakEqualizerAudio : public SynchronBlockProcessor
{
public:
    PeakEqualizerAudio();
    void prepareToPlay(double sampleRate, int max_samplesPerBlock, int max_channels);
    virtual int processSynchronBlock(juce::AudioBuffer<float>&, juce::MidiBuffer& midiMessages);

    // parameter handling
  	void addParameter(std::vector < std::unique_ptr<juce::RangedAudioParameter>>& paramVector);
    void prepareParameter(std::unique_ptr<juce::AudioProcessorValueTreeState>&  vts);
    
    // some necessary info for the host
    int getLatency(){return m_Latency;};

private:
    int m_Latency = 0;
	float m_fs = 44100.f;
	float m_f0 = 1000.f;
	float m_Q = 1.f;
	float m_gain = 0.f;
	std::vector<double> m_b;
	std::vector<double> m_a;
	std::vector<float> m_state_b1;
	std::vector<float> m_state_b2;
	std::vector<float> m_state_a1;
	std::vector<float> m_state_a2;

	jade::AudioProcessParameter<float> m_gainParam;
	jade::AudioProcessParameter<float> m_QParam;
	jade::AudioProcessParameter<float> m_FreqParam;
	juce::SmoothedValue<float> m_smoothedGain;
	juce::SmoothedValue<float> m_smoothedQ;
	juce::SmoothedValue<float> m_smoothedFreq;
	
};
class PeakEqualizerGUI : public juce::Component
{
public:
	PeakEqualizerGUI(juce::AudioProcessorValueTreeState& apvts);

	void paint(juce::Graphics& g) override;
	void resized() override;
private:
    juce::AudioProcessorValueTreeState& m_apvts;
	juce::Slider m_GainSlider;
	juce::Slider m_QSlider;
	juce::Slider m_FreqSlider;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_gainAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_QAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_FreqAttachment; 

};
