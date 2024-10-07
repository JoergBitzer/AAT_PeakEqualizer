#include <math.h>
#include "PeakEqualizer.h"

#include "EqualizerDesign.h"

PeakEqualizerAudio::PeakEqualizerAudio()
:SynchronBlockProcessor()
{
}

void PeakEqualizerAudio::prepareToPlay(double sampleRate, int max_samplesPerBlock, int max_channels)
{
    juce::ignoreUnused(max_samplesPerBlock,max_channels);
    int synchronblocksize;
    synchronblocksize = static_cast<int>(round(g_desired_blocksize_ms * sampleRate * 0.001)); // 0.001 to transform ms to seconds
    if (g_forcePowerOf2)
    {
        int nextpowerof2 = int(log2(synchronblocksize))+1;
        synchronblocksize = int(pow(2,nextpowerof2));
    }
    prepareSynchronProcessing(max_channels,synchronblocksize);
    m_Latency += synchronblocksize;
    // here your code
    m_fs = sampleRate;
    m_f0 = 4000.f;
    m_Q = 9.f;
    m_gain = 20.f;
    EqualizerErrorCode error = designPeakEqualizer(m_b, m_a, m_f0, m_Q, m_gain, m_fs);
    if (error != NO_ERROR)
    {
        // handle error
        m_b.resize(3);
        m_a.resize(3);
        m_b[0] = 1.0;
        m_b[1] = 0.0;
        m_b[2] = 0.0;
        m_a[0] = 1.0;
        m_a[1] = 0.0;
        m_a[2] = 0.0;
    }
    m_state_b1.resize(max_channels);
    m_state_b2.resize(max_channels);
    m_state_a1.resize(max_channels);
    m_state_a2.resize(max_channels);
    for (int i = 0; i < max_channels; i++)
    {
        m_state_b1[i] = 0.f;
        m_state_b2[i] = 0.f;
        m_state_a1[i] = 0.f;
        m_state_a2[i] = 0.f;
    }

}

int PeakEqualizerAudio::processSynchronBlock(juce::AudioBuffer<float> & buffer, juce::MidiBuffer &midiMessages)
{
    juce::ignoreUnused(midiMessages);
    int numChannels = buffer.getNumChannels();
    int numSamples = buffer.getNumSamples();
    for (int channel = 0; channel < numChannels; channel++)
    {
        float* channelData = buffer.getWritePointer(channel);
        for (int sample = 0; sample < numSamples; sample++)
        {   
            float In = channelData[sample];
            float Out = m_b[0] * In + m_b[1] * m_state_b1[channel] + m_b[2] * m_state_b2[channel] 
                    - m_a[1] * m_state_a1[channel] - m_a[2] * m_state_a2[channel];
        
            m_state_b2[channel] = m_state_b1[channel];
            m_state_b1[channel] = In;
            m_state_a2[channel] = m_state_a1[channel];
            m_state_a1[channel] = Out;
            channelData[sample] = Out;
        }
    }
    return 0;
}

void PeakEqualizerAudio::addParameter(std::vector<std::unique_ptr<juce::RangedAudioParameter>> &paramVector)
{
    // this is just a placeholder (necessary for compiling/testing the template)
    paramVector.push_back(std::make_unique<AudioParameterFloat>(g_paramExample.ID,
        g_paramExample.name,
        NormalisableRange<float>(g_paramExample.minValue, g_paramExample.maxValue),
        g_paramExample.defaultValue,
        g_paramExample.unitName,
        AudioProcessorParameter::genericParameter));
        // these are two additional lines with lambdas to convert data (to use delete )); after 
        // AudioProcessorParameter::genericParameter and uncomment to activate)
        // [](float value, int MaxLen) { value = int(exp(value) * 10) * 0.1;  return (String(value, MaxLen) + " Hz"); },
        // [](const String& text) {return text.getFloatValue(); }));

}

void PeakEqualizerAudio::prepareParameter(std::unique_ptr<juce::AudioProcessorValueTreeState> &vts)
{
    juce::ignoreUnused(vts);
}


PeakEqualizerGUI::PeakEqualizerGUI(juce::AudioProcessorValueTreeState& apvts)
:m_apvts(apvts)
{
    
}

void PeakEqualizerGUI::paint(juce::Graphics &g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId).brighter(0.3f));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    
    juce::String text2display = "PeakEqualizer V " + juce::String(PLUGIN_VERSION_MAJOR) + "." + juce::String(PLUGIN_VERSION_MINOR) + "." + juce::String(PLUGIN_VERSION_PATCH);
    g.drawFittedText (text2display, getLocalBounds(), juce::Justification::centred, 1);

}

void PeakEqualizerGUI::resized()
{
	auto r = getLocalBounds();
    
    // if you have to place several components, use scaleFactor
    //int width = r.getWidth();
	//float scaleFactor = float(width)/g_minGuiSize_x;

    // use the given canvas in r
    juce::ignoreUnused(r);


}
