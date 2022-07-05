#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    addAndMakeVisible(frequencySlider);
    frequencySlider.setRange(50, 5000);
    frequencySlider.setSkewFactorFromMidPoint(500);
    frequencySlider.setValue(currentFrequency, juce::dontSendNotification);
    frequencySlider.onValueChange = [this]
    {
        targetFrequency = frequencySlider.getValue();
    };

    addAndMakeVisible(levelSlider);
    levelSlider.setRange(0.f, 0.25f);
    levelSlider.onValueChange = [this]
    {
        level = levelSlider.getValue();
    };

    setSize (600, 200);

    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        setAudioChannels (0, 2);
    }
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    currentSampleRate = sampleRate;
    updateAngleDelta();
}

void MainComponent::updateAngleDelta()
{
    auto cyclesPerSample = frequencySlider.getValue() / currentSampleRate;
    angleDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    auto* leftBuffer = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
    auto* rightBuffer = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);

    auto localTargetFrequency = targetFrequency;

    if (localTargetFrequency != currentFrequency)                                                   
    {
        auto frequencyIncrement = (localTargetFrequency - currentFrequency) / bufferToFill.numSamples;   

        for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
        {
            auto currentSample = (float)std::sin(currentAngle);
            currentFrequency += frequencyIncrement;                                                     
            updateAngleDelta();                                                                        
            currentAngle += angleDelta;
            leftBuffer[sample] = currentSample * level;
            rightBuffer[sample] = currentSample * level;
        }

        currentFrequency = localTargetFrequency;
    }
    else                                                                                          
    {
        for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
        {
            auto currentSample = (float)std::sin(currentAngle);
            currentAngle += angleDelta;
            leftBuffer[sample] = currentSample * level;
            rightBuffer[sample] = currentSample * level;
        }
    }
}

void MainComponent::releaseResources()
{

}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    frequencySlider.setBounds(10, 10, getWidth() - 20, 20);
    levelSlider.setBounds(10, 40, getWidth() - 20, 20);
}
