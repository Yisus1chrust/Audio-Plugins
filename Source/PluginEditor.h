#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

namespace photosynth
{
    class PhotoSynthAudioProcessorEditor : public juce::AudioProcessorEditor,
                                           private juce::Timer
    {
    public:
        explicit PhotoSynthAudioProcessorEditor(PhotoSynthAudioProcessor&);
        ~PhotoSynthAudioProcessorEditor() override;

        void paint(juce::Graphics&) override;
        void resized() override;

    private:
        class WebUiServer;

        PhotoSynthAudioProcessor& processor;
        std::unique_ptr<WebUiServer> server;
        std::unique_ptr<juce::WebBrowserComponent> browser;

        juce::Label statusLabel;

        void timerCallback() override;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhotoSynthAudioProcessorEditor)
    };
}
