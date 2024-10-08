#include <math.h>
#include "PeakEqualizer.h"

#include "EqualizerDesign.h"
#include "hermite-cubic-curve.h"

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
    bool somethingchanged = false;
    somethingchanged |= m_gainParam.updateWithNotification(m_gain);
    somethingchanged |= m_QParam.updateWithNotification(m_Q);
    somethingchanged |= m_FreqParam.updateWithNotification(m_f0);
    if (somethingchanged)
    {
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
    m_GainSlider.onValueChange = [this] { repaint(); };

    m_QSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    m_QSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, false, 70, 20);
    m_QSlider.setRange(g_paramQ.minValue, g_paramQ.maxValue);
    m_QSlider.setTextValueSuffix(g_paramQ.unitName);
    val = m_apvts.getRawParameterValue(g_paramQ.ID);
    m_QSlider.setValue(*val);
    m_QAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, g_paramQ.ID, m_QSlider);
    addAndMakeVisible(m_QSlider);
    m_QSlider.onValueChange = [this] { repaint(); };

    m_FreqSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    m_FreqSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, false, 70, 20);
    m_FreqSlider.setRange(g_paramFreq.minValue, g_paramFreq.maxValue);
    m_FreqSlider.setTextValueSuffix(g_paramFreq.unitName);
    val = m_apvts.getRawParameterValue(g_paramFreq.ID);
    m_FreqSlider.setValue(*val);
    m_FreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(m_apvts, g_paramFreq.ID, m_FreqSlider);
    addAndMakeVisible(m_FreqSlider);
    m_FreqSlider.onValueChange = [this] { repaint(); };

}

void PeakEqualizerGUI::paint(juce::Graphics &g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId).brighter(0.3f));
    float gain = m_GainSlider.getValue();
    float Q = (m_QSlider.getValue());
    float Freq = (m_FreqSlider.getValue());

    float relGain = gain/g_paramGain.maxValue;
    float relQ = (1.f-(Q-g_paramQ.minValue)/(g_paramQ.maxValue-g_paramQ.minValue))*0.8;
    if (relQ < 0.f)
    {
        relQ = 0.f;
    }
    if (relQ > 1.f)
    {
        relQ = 1.f;
    }
    float relFreq = (Freq-g_paramFreq.minValue)/(g_paramFreq.maxValue-g_paramFreq.minValue);
    float leftQ = relFreq - relQ/2 -0.05f;
    float rightQ = relFreq + relQ/2 + 0.05f;
/*    if (leftQ < 0.f)
    {
        leftQ = 0.f;
    }
    if (rightQ > 1.f)
    {
        rightQ = 1.f;
    } //*/
    // spline for the EQ curve
    HermiteCubicCurve <float> curve;
    curve.clear();
    if (leftQ <  0.f)
        curve.add(leftQ, 0.0f);
    else
        curve.add(0.f, 0.0f);

    curve.add(leftQ, 0.5f*relGain);
    curve.add(relFreq, relGain);
    curve.add(rightQ, 0.5f*relGain);
    curve.add(1.0f, 0.0f);
    curve.finish();
    // draw the curve
    g.setColour (juce::Colours::white);
    int height = getHeight();
    int width = getWidth();
    int margin = 10;
    g.fillRect(0,height*3/4 + margin ,width,height/4 - 2*margin);
 
    g.setColour (juce::Colours::red);
 
    int startheight = +7.f/8.f*height;
    
    int drawheight = height/8-margin;
    float x1 = 0.f;
    float y1 = curve.at(x1);
    for (int i = 1; i < width; i++)
    {
        float x2 = float(i)/width;
        float y2 = curve.at(x2);
        //g.drawEllipse(x*getWidth(), -y*drawheight + startheight, 2, 2, 2);
        if (x2>=0)
            g.drawLine(x1*width,-y1*drawheight + startheight,x2*width,-y2*drawheight + startheight,2);
        x1 = x2;
        y1 = y2;
    }

    // draw the version number
    g.setColour (juce::Colours::white);
    g.setFont (margin);
    
    juce::String text2display = "PeakEqualizer V " + juce::String(PLUGIN_VERSION_MAJOR) + "." + juce::String(PLUGIN_VERSION_MINOR) + "." + juce::String(PLUGIN_VERSION_PATCH);
    g.drawFittedText (text2display, getLocalBounds(), juce::Justification::bottomLeft, 1);

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
