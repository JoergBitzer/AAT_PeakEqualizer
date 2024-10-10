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
    float paramsampleRate = 1.f/(g_desired_blocksize_ms * 0.001);
    m_smoothedGain.reset(paramsampleRate, 0.05);
    m_smoothedQ.reset(paramsampleRate, 0.05);
    m_smoothedFreq.reset(paramsampleRate, 0.05);
    m_smoothedGain.setCurrentAndTargetValue(m_gain);
    m_smoothedQ.setCurrentAndTargetValue(m_Q);
    m_smoothedFreq.setCurrentAndTargetValue(m_f0);

}

int PeakEqualizerAudio::processSynchronBlock(juce::AudioBuffer<float> & buffer, juce::MidiBuffer &midiMessages)
{
    bool somethingchanged = false;
    bool somethingchangedGain = m_gainParam.updateWithNotification(m_gain);
    if (somethingchangedGain)
    {
        m_smoothedGain.setTargetValue(m_gain);
    }
    somethingchanged != somethingchangedGain;
    bool somethingchangedQ = m_QParam.updateWithNotification(m_Q);
    if (somethingchangedQ)
    {
        m_smoothedQ.setTargetValue(m_Q);
    }
    bool somethingchangedFreq = m_FreqParam.updateWithNotification(m_f0);
    if (somethingchangedFreq)
    {
        m_smoothedFreq.setTargetValue(m_f0);
    }
    float curGain = m_smoothedGain.getNextValue();
    float curQ = m_smoothedQ.getNextValue();
    float curFreq = m_smoothedFreq.getNextValue();
    EqualizerErrorCode error = designPeakEqualizer(m_b, m_a, curFreq, curQ, curGain, m_fs);
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
    paramVector.push_back(std::make_unique<AudioParameterFloat>(g_paramGain.ID,
        g_paramGain.name,
        NormalisableRange<float>(g_paramGain.minValue, g_paramGain.maxValue),
        g_paramGain.defaultValue,
        AudioParameterFloatAttributes().withLabel (g_paramGain.unitName)
                                        .withCategory (juce::AudioProcessorParameter::genericParameter)
                                        // or two additional lines with lambdas to convert data for display
                                        .withStringFromValueFunction (std::move ([](float value, int MaxLen) { value = int((value) * 10) * 0.1f;  return (String(value, MaxLen)); }))
                                        .withValueFromStringFunction (std::move ([](const String& text) {return text.getFloatValue(); }))
                        ));
    // this is just a placeholder (necessary for compiling/testing the template)
    paramVector.push_back(std::make_unique<AudioParameterFloat>(g_paramQ.ID,
        g_paramQ.name,
        NormalisableRange<float>(g_paramQ.minValue, g_paramQ.maxValue),
        g_paramQ.defaultValue,
        AudioParameterFloatAttributes().withLabel (g_paramQ.unitName)
                                        .withCategory (juce::AudioProcessorParameter::genericParameter)
                                        // or two additional lines with lambdas to convert data for display
                                        .withStringFromValueFunction (std::move ([](float value, int MaxLen) { value = int(exp(value) * 100) * 0.01f;  return (String(value, MaxLen)); }))
                                        .withValueFromStringFunction (std::move ([](const String& text) {return text.getFloatValue(); }))
                        ));
    // this is just a placeholder (necessary for compiling/testing the template)
    paramVector.push_back(std::make_unique<AudioParameterFloat>(g_paramFreq.ID,
        g_paramFreq.name,
        NormalisableRange<float>(g_paramFreq.minValue, g_paramFreq.maxValue),
        g_paramFreq.defaultValue,
        AudioParameterFloatAttributes().withLabel (g_paramFreq.unitName)
                                        .withCategory (juce::AudioProcessorParameter::genericParameter)
                                        // or two additional lines with lambdas to convert data for display
                                        .withStringFromValueFunction (std::move ([](float value, int MaxLen) { value = int(exp(value) * 10) * 0.1f;  return (String(value, MaxLen)); }))
                                        .withValueFromStringFunction (std::move ([](const String& text) {return text.getFloatValue(); }))
                        ));

}

void PeakEqualizerAudio::prepareParameter(std::unique_ptr<juce::AudioProcessorValueTreeState> &vts)
{
    m_gainParam.prepareParameter(vts->getRawParameterValue(g_paramGain.ID));
    m_QParam.prepareParameter(vts->getRawParameterValue(g_paramQ.ID));
    m_QParam.changeTransformer(jade::AudioProcessParameter<float>::transformerFunc::exptransform);
    m_FreqParam.prepareParameter(vts->getRawParameterValue(g_paramFreq.ID));
    m_FreqParam.changeTransformer(jade::AudioProcessParameter<float>::transformerFunc::exptransform);
}


PeakEqualizerGUI::PeakEqualizerGUI(juce::AudioProcessorValueTreeState& apvts)
:m_apvts(apvts)
{
    m_GainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    m_GainSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, false, 70, 20);
    m_GainSlider.setRange(g_paramGain.minValue, g_paramGain.maxValue);
    m_GainSlider.setTextValueSuffix(g_paramGain.unitName);
    auto val = m_apvts.getRawParameterValue(g_paramGain.ID);
    m_GainSlider.setValue(*val);
    m_gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, g_paramGain.ID, m_GainSlider);
    addAndMakeVisible(m_GainSlider);

    m_QSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    m_QSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, false, 70, 20);
    m_QSlider.setRange(g_paramQ.minValue, g_paramQ.maxValue);
    m_QSlider.setTextValueSuffix(g_paramQ.unitName);
    val = m_apvts.getRawParameterValue(g_paramQ.ID);
    m_QSlider.setValue(*val);
    m_QAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, g_paramQ.ID, m_QSlider);
    addAndMakeVisible(m_QSlider);

    m_FreqSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    m_FreqSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, false, 70, 20);
    m_FreqSlider.setRange(g_paramFreq.minValue, g_paramFreq.maxValue);
    m_FreqSlider.setTextValueSuffix(g_paramFreq.unitName);
    val = m_apvts.getRawParameterValue(g_paramFreq.ID);
    m_FreqSlider.setValue(*val);
    m_FreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, g_paramFreq.ID, m_FreqSlider);
    addAndMakeVisible(m_FreqSlider);

}

void PeakEqualizerGUI::paint(juce::Graphics &g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId).brighter(0.3f));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    
    juce::String text2display = "PeakEqualizer V " + juce::String(PLUGIN_VERSION_MAJOR) + "." + juce::String(PLUGIN_VERSION_MINOR) + "." + juce::String(PLUGIN_VERSION_PATCH);
    g.drawFittedText (text2display, getLocalBounds(), juce::Justification::bottomRight, 1);

}

void PeakEqualizerGUI::resized()
{
	auto r = getLocalBounds();
    
    // if you have to place several components, use scaleFactor
    //int width = r.getWidth();
	//float scaleFactor = float(width)/g_minGuiSize_x;

    // use the given canvas in r
    int height = r.getHeight();
    m_GainSlider.setBounds(r.removeFromTop(height/4));
    m_QSlider.setBounds(r.removeFromTop(height/4));
    m_FreqSlider.setBounds(r.removeFromTop(height/4));


}
