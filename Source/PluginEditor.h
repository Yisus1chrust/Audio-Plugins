#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

namespace photosynth
{
    class RadialGauge : public juce::Component
    {
    public:
        RadialGauge(juce::String title, juce::Colour accent) : label(std::move(title)), color(accent) {}
        void setValue(float v) { value = juce::jlimit(0.0f, 1.0f, v); repaint(); }
        void paint(juce::Graphics& g) override;

    private:
        juce::String label;
        juce::Colour color;
        float value = 0.0f;
    };

    class PhotoSynthAudioProcessorEditor : public juce::AudioProcessorEditor,
                                           public juce::FileDragAndDropTarget,
                                           private juce::Timer
    {
    public:
        explicit PhotoSynthAudioProcessorEditor(PhotoSynthAudioProcessor&);
        ~PhotoSynthAudioProcessorEditor() override = default;

        void paint(juce::Graphics&) override;
        void resized() override;

        bool isInterestedInFileDrag(const juce::StringArray& files) override;
        void filesDropped(const juce::StringArray& files, int x, int y) override;

    private:
        using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
        using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
        using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

        PhotoSynthAudioProcessor& processor;

        RadialGauge timbreGauge { "TIMBRE DNA", juce::Colours::cyan };
        RadialGauge brightnessGauge { "BRIGHTNESS", juce::Colours::orange };
        RadialGauge saturationGauge { "SATURATION", juce::Colours::deeppink };
        RadialGauge complexityGauge { "COMPLEXITY", juce::Colours::limegreen };

        juce::TextButton changeImageAButton { "Change Image A" };
        juce::TextButton changeImageBButton { "Change Image B" };
        juce::TextButton savePresetButton { "Save Preset" };
        juce::TextButton loadPresetButton { "Load Preset" };
        juce::TextButton deepDiveButton { "Deep Dive" };
        juce::ToggleButton effectsEditableButton { "Effects Editable" };

        juce::Label imageStatusLabel;
        juce::Label bpmLabel;

        juce::AudioVisualiserComponent oscilloscope { 2 };
        juce::MidiKeyboardComponent keyboard;

        juce::ComboBox engineType;
        juce::ToggleButton limiterButton { "Limiter" };
        juce::ToggleButton tempoSyncButton { "Tempo Sync" };

        juce::Slider cutoff, resonance, lfoRate, lfoDepth;
        juce::Slider volume, attack, decay, sustain, release, threshold;
        juce::Slider morphSlider, bpmSlider;

        // Deep dive controls
        juce::GroupComponent deepDiveGroup { {}, "Temporal + Hidden Parameters" };
        juce::Slider temporalEra, opticalFocus, gridSymmetry, chromaticClash, semanticDensity;
        juce::Slider bodyDamping, acousticWeight, flutterSpeed, flutterDepth, warmth;
        juce::Slider delayIntensity, reverbIntensity, chorusIntensity, phaserIntensity, flangerIntensity, distortionIntensity;

        std::unique_ptr<SliderAttachment> cutoffAtt, resonanceAtt, lfoRateAtt, lfoDepthAtt;
        std::unique_ptr<SliderAttachment> volumeAtt, attackAtt, decayAtt, sustainAtt, releaseAtt, thresholdAtt;
        std::unique_ptr<SliderAttachment> morphAtt, bpmAtt;

        std::unique_ptr<SliderAttachment> temporalEraAtt, opticalFocusAtt, gridSymmetryAtt, chromaticClashAtt, semanticDensityAtt;
        std::unique_ptr<SliderAttachment> bodyDampingAtt, acousticWeightAtt, flutterSpeedAtt, flutterDepthAtt, warmthAtt;
        std::unique_ptr<SliderAttachment> delayIntensityAtt, reverbIntensityAtt, chorusIntensityAtt, phaserIntensityAtt, flangerIntensityAtt, distortionIntensityAtt;

        std::unique_ptr<ButtonAttachment> limiterAtt;
        std::unique_ptr<ButtonAttachment> tempoSyncAtt;
        std::unique_ptr<ComboBoxAttachment> engineAtt;

        juce::Rectangle<int> dropzone;
        bool deepDiveVisible = false;

        void timerCallback() override;
        void configureKnob(juce::Slider& slider, const juce::String& name);
        void configureMiniSlider(juce::Slider& slider, const juce::String& name);
        void chooseImageForSlot(bool slotA);
        void setDeepDiveVisible(bool visible);

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhotoSynthAudioProcessorEditor)
    };
}
