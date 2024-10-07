#pragma once

#include <vector>
#include <juce_audio_processors/juce_audio_processors.h>

#include "tools/SynchronBlockProcessor.h"
#include "PluginSettings.h"


// This is how we define our parameter as globals to use it in the audio processor as well as in the editor
const struct
{
	const std::string ID = "ExampleID";
	const std::string name = "Example";
	const std::string unitName = "xyz";
	const float minValue = 1.f;
	const float maxValue = 2.f;
	const float defaultValue = 1.2f;
}g_paramExample;


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
};

class PeakEqualizerGUI : public juce::Component
{
public:
	PeakEqualizerGUI(juce::AudioProcessorValueTreeState& apvts);

	void paint(juce::Graphics& g) override;
	void resized() override;
private:
    juce::AudioProcessorValueTreeState& m_apvts; 

};
