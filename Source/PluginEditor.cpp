#include "PluginEditor.h"

namespace photosynth
{
    void RadialGauge::paint(juce::Graphics& g)
    {
        auto r = getLocalBounds().toFloat().reduced(4.0f);
        const auto c = r.getCentre();
        const float radius = juce::jmin(r.getWidth(), r.getHeight()) * 0.5f - 8.0f;

        g.setColour(juce::Colour(0xff151b23));
        g.fillEllipse(c.x - radius, c.y - radius, radius * 2.0f, radius * 2.0f);

        juce::Path p;
        const float start = juce::MathConstants<float>::pi * 0.75f;
        const float end = start + juce::MathConstants<float>::pi * 1.5f * value;
        p.addCentredArc(c.x, c.y, radius, radius, 0.0f, start, end, true);

        g.setColour(juce::Colours::white.withAlpha(0.12f));
        g.drawEllipse(c.x - radius, c.y - radius, radius * 2.0f, radius * 2.0f, 3.0f);

        g.setColour(color);
        g.strokePath(p, juce::PathStrokeType(4.0f));

        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(12.0f, juce::Font::bold));
        g.drawFittedText(juce::String((int) std::round(value * 100.0f)) + "%", getLocalBounds().reduced(8), juce::Justification::centred, 1);

        g.setFont(juce::Font(10.0f));
        g.drawFittedText(label, getLocalBounds().removeFromBottom(14), juce::Justification::centred, 1);
    }

    PhotoSynthAudioProcessorEditor::PhotoSynthAudioProcessorEditor(PhotoSynthAudioProcessor& p)
        : AudioProcessorEditor(&p),
          processor(p),
          keyboard(processor.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
    {
        setSize(1360, 860);

        addAndMakeVisible(timbreGauge);
        addAndMakeVisible(brightnessGauge);
        addAndMakeVisible(saturationGauge);
        addAndMakeVisible(complexityGauge);

        addAndMakeVisible(imageStatusLabel);
        imageStatusLabel.setJustificationType(juce::Justification::centred);
        imageStatusLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.82f));

        addAndMakeVisible(changeImageAButton);
        changeImageAButton.onClick = [this] { chooseImageForSlot(true); };

        addAndMakeVisible(changeImageBButton);
        changeImageBButton.onClick = [this] { chooseImageForSlot(false); };

        addAndMakeVisible(savePresetButton);
        savePresetButton.onClick = [this]
        {
            juce::FileChooser chooser("Save PhotoSynth preset", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.photosynthpreset");
            if (chooser.browseForFileToSave(true))
                processor.savePresetToFile(chooser.getResult());
        };

        addAndMakeVisible(loadPresetButton);
        loadPresetButton.onClick = [this]
        {
            juce::FileChooser chooser("Load PhotoSynth preset", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.photosynthpreset");
            if (chooser.browseForFileToOpen())
                processor.loadPresetFromFile(chooser.getResult());
        };

        addAndMakeVisible(deepDiveButton);
        deepDiveButton.onClick = [this]
        {
            deepDiveVisible = !deepDiveVisible;
            setDeepDiveVisible(deepDiveVisible);
            resized();
            repaint();
        };

        addAndMakeVisible(effectsEditableButton);
        effectsEditableButton.setToggleState(false, juce::dontSendNotification);
        effectsEditableButton.onClick = [this]
        {
            const bool editable = effectsEditableButton.getToggleState();
            delayIntensity.setEnabled(editable);
            reverbIntensity.setEnabled(editable);
            chorusIntensity.setEnabled(editable);
            phaserIntensity.setEnabled(editable);
            flangerIntensity.setEnabled(editable);
            distortionIntensity.setEnabled(editable);
        };

        addAndMakeVisible(oscilloscope);
        oscilloscope.setBufferSize(512);
        oscilloscope.setSamplesPerBlock(32);
        oscilloscope.setColours(juce::Colours::black, juce::Colours::cyan);

        addAndMakeVisible(keyboard);
        keyboard.setAvailableRange(24, 108);

        addAndMakeVisible(engineType);
        engineType.addItemList({ "analog_brass", "digital_fm_bells", "hybrid_wavetable", "acoustic_piano_organ", "overdriven_saw_stack", "fm_square_bell" }, 1);

        configureKnob(cutoff, "Cutoff");
        configureKnob(resonance, "Resonance");
        configureKnob(lfoRate, "LFO Rate");
        configureKnob(lfoDepth, "LFO Depth");
        configureKnob(volume, "Master Vol");
        configureKnob(attack, "Attack");
        configureKnob(decay, "Decay");
        configureKnob(sustain, "Sustain");
        configureKnob(release, "Release");
        configureKnob(threshold, "Threshold");

        configureMiniSlider(morphSlider, "Image Fusion");
        morphSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        morphSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 72, 18);
        morphSlider.onValueChange = [this]
        {
            processor.applyMorphFromLoadedImages((float) morphSlider.getValue());
        };

        configureMiniSlider(bpmSlider, "BPM");
        configureMiniSlider(temporalEra, "Temporal Era");
        configureMiniSlider(opticalFocus, "Optical Focus");
        configureMiniSlider(gridSymmetry, "Grid Symmetry");
        configureMiniSlider(chromaticClash, "Chromatic Clash");
        configureMiniSlider(semanticDensity, "Semantic Density");
        configureMiniSlider(bodyDamping, "Body Damping");
        configureMiniSlider(acousticWeight, "Acoustic Weight");
        configureMiniSlider(flutterSpeed, "Flutter Speed");
        configureMiniSlider(flutterDepth, "Flutter Depth");
        configureMiniSlider(warmth, "Analog Warmth");

        configureMiniSlider(delayIntensity, "Delay Intensity");
        configureMiniSlider(reverbIntensity, "Reverb Intensity");
        configureMiniSlider(chorusIntensity, "Chorus Intensity");
        configureMiniSlider(phaserIntensity, "Phaser Intensity");
        configureMiniSlider(flangerIntensity, "Flanger Intensity");
        configureMiniSlider(distortionIntensity, "Distortion Intensity");

        addAndMakeVisible(deepDiveGroup);
        addAndMakeVisible(limiterButton);
        addAndMakeVisible(tempoSyncButton);
        addAndMakeVisible(bpmLabel);
        bpmLabel.setJustificationType(juce::Justification::centredRight);
        bpmLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.85f));

        auto& apvts = processor.apvts;
        cutoffAtt = std::make_unique<SliderAttachment>(apvts, "cutoffOffset", cutoff);
        resonanceAtt = std::make_unique<SliderAttachment>(apvts, "resonance", resonance);
        lfoRateAtt = std::make_unique<SliderAttachment>(apvts, "lfoRate", lfoRate);
        lfoDepthAtt = std::make_unique<SliderAttachment>(apvts, "lfoDepth", lfoDepth);
        volumeAtt = std::make_unique<SliderAttachment>(apvts, "masterVolume", volume);
        attackAtt = std::make_unique<SliderAttachment>(apvts, "attackTime", attack);
        decayAtt = std::make_unique<SliderAttachment>(apvts, "decayTime", decay);
        sustainAtt = std::make_unique<SliderAttachment>(apvts, "sustainLevel", sustain);
        releaseAtt = std::make_unique<SliderAttachment>(apvts, "releaseTime", release);
        thresholdAtt = std::make_unique<SliderAttachment>(apvts, "threshold", threshold);
        morphAtt = std::make_unique<SliderAttachment>(apvts, "morphAmount", morphSlider);
        bpmAtt = std::make_unique<SliderAttachment>(apvts, "bpm", bpmSlider);

        temporalEraAtt = std::make_unique<SliderAttachment>(apvts, "temporalEraVal", temporalEra);
        opticalFocusAtt = std::make_unique<SliderAttachment>(apvts, "opticalFocusDepth", opticalFocus);
        gridSymmetryAtt = std::make_unique<SliderAttachment>(apvts, "gridSymmetryDensity", gridSymmetry);
        chromaticClashAtt = std::make_unique<SliderAttachment>(apvts, "chromaticClash", chromaticClash);
        semanticDensityAtt = std::make_unique<SliderAttachment>(apvts, "semanticDensityWeight", semanticDensity);

        bodyDampingAtt = std::make_unique<SliderAttachment>(apvts, "bodyDamping", bodyDamping);
        acousticWeightAtt = std::make_unique<SliderAttachment>(apvts, "acousticWeight", acousticWeight);
        flutterSpeedAtt = std::make_unique<SliderAttachment>(apvts, "tapeFlutterSpeed", flutterSpeed);
        flutterDepthAtt = std::make_unique<SliderAttachment>(apvts, "tapeFlutterDepth", flutterDepth);
        warmthAtt = std::make_unique<SliderAttachment>(apvts, "analogSaturationWarmth", warmth);

        delayIntensityAtt = std::make_unique<SliderAttachment>(apvts, "delayIntensity", delayIntensity);
        reverbIntensityAtt = std::make_unique<SliderAttachment>(apvts, "reverbIntensity", reverbIntensity);
        chorusIntensityAtt = std::make_unique<SliderAttachment>(apvts, "chorusIntensity", chorusIntensity);
        phaserIntensityAtt = std::make_unique<SliderAttachment>(apvts, "phaserIntensity", phaserIntensity);
        flangerIntensityAtt = std::make_unique<SliderAttachment>(apvts, "flangerIntensity", flangerIntensity);
        distortionIntensityAtt = std::make_unique<SliderAttachment>(apvts, "distortionIntensity", distortionIntensity);

        limiterAtt = std::make_unique<ButtonAttachment>(apvts, "limiterEnabled", limiterButton);
        tempoSyncAtt = std::make_unique<ButtonAttachment>(apvts, "tempoSync", tempoSyncButton);
        engineAtt = std::make_unique<ComboBoxAttachment>(apvts, "engineType", engineType);

        setDeepDiveVisible(false);
        startTimerHz(30);
    }

    void PhotoSynthAudioProcessorEditor::configureKnob(juce::Slider& slider, const juce::String& name)
    {
        slider.setName(name);
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 18);
        slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::cyan.withAlpha(0.8f));
        slider.setColour(juce::Slider::thumbColourId, juce::Colours::white);
        addAndMakeVisible(slider);
    }

    void PhotoSynthAudioProcessorEditor::configureMiniSlider(juce::Slider& slider, const juce::String& name)
    {
        slider.setName(name);
        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 64, 18);
        slider.setColour(juce::Slider::trackColourId, juce::Colours::cyan.withAlpha(0.8f));
        slider.setColour(juce::Slider::thumbColourId, juce::Colours::white);
        addAndMakeVisible(slider);
    }

    void PhotoSynthAudioProcessorEditor::chooseImageForSlot(bool slotA)
    {
        juce::FileChooser chooser(slotA ? "Choose Image A" : "Choose Image B", {}, "*.png;*.jpg;*.jpeg;*.bmp;*.webp");
        if (chooser.browseForFileToOpen())
            processor.loadImageFileToSlot(chooser.getResult(), slotA);
    }

    bool PhotoSynthAudioProcessorEditor::isInterestedInFileDrag(const juce::StringArray& files)
    {
        if (files.isEmpty())
            return false;

        const auto ext = juce::File(files[0]).getFileExtension().toLowerCase();
        return ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".webp" || ext == ".bmp";
    }

    void PhotoSynthAudioProcessorEditor::filesDropped(const juce::StringArray& files, int x, int)
    {
        if (files.isEmpty())
            return;

        const bool slotA = x < dropzone.getCentreX();
        processor.loadImageFileToSlot(juce::File(files[0]), slotA);
    }

    void PhotoSynthAudioProcessorEditor::timerCallback()
    {
        const auto& m = processor.getMetrics();
        timbreGauge.setValue(m.timbreDna);
        brightnessGauge.setValue(m.brightness);
        saturationGauge.setValue(m.saturation);
        complexityGauge.setValue(m.complexity);

        juce::AudioBuffer<float> scope;
        processor.getScopeData(scope);
        if (scope.getNumSamples() > 0)
            oscilloscope.pushBuffer(scope);

        imageStatusLabel.setText(
            processor.hasImageA() && processor.hasImageB() ? "Image A + Image B loaded (Fusion active)"
            : processor.hasImageA() ? "Image A loaded"
            : processor.hasImageB() ? "Image B loaded"
            : "Drop image files or use Change Image A / B",
            juce::dontSendNotification);

        bpmLabel.setText(juce::String((int) std::round(bpmSlider.getValue())) + " BPM", juce::dontSendNotification);

        repaint(dropzone);
    }

    void PhotoSynthAudioProcessorEditor::setDeepDiveVisible(bool visible)
    {
        deepDiveGroup.setVisible(visible);
        temporalEra.setVisible(visible);
        opticalFocus.setVisible(visible);
        gridSymmetry.setVisible(visible);
        chromaticClash.setVisible(visible);
        semanticDensity.setVisible(visible);

        bodyDamping.setVisible(visible);
        acousticWeight.setVisible(visible);
        flutterSpeed.setVisible(visible);
        flutterDepth.setVisible(visible);
        warmth.setVisible(visible);

        delayIntensity.setVisible(visible);
        reverbIntensity.setVisible(visible);
        chorusIntensity.setVisible(visible);
        phaserIntensity.setVisible(visible);
        flangerIntensity.setVisible(visible);
        distortionIntensity.setVisible(visible);
    }

    void PhotoSynthAudioProcessorEditor::paint(juce::Graphics& g)
    {
        g.fillAll(juce::Colour(0xff0b1119));

        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(26.0f, juce::Font::bold));
        g.drawText("PHOTO SYNTH", 20, 12, 320, 36, juce::Justification::left);

        g.setColour(juce::Colours::cyan.withAlpha(0.4f));
        g.drawFittedText("Image-to-Synth Fusion Instrument", 24, 44, 360, 20, juce::Justification::left, 1);

        g.setColour(juce::Colour(0xff141b26));
        g.fillRoundedRectangle(dropzone.toFloat(), 10.0f);
        g.setColour(juce::Colours::cyan.withAlpha(0.45f));
        g.drawRoundedRectangle(dropzone.toFloat(), 10.0f, 2.0f);

        auto imageRect = dropzone.reduced(12);
        auto preview = processor.getPreviewImage(imageRect.getWidth(), imageRect.getHeight());
        if (!preview.isNull())
            g.drawImageWithin(preview, imageRect.getX(), imageRect.getY(), imageRect.getWidth(), imageRect.getHeight(), juce::RectanglePlacement::centred, false);
        else
        {
            g.setColour(juce::Colours::white.withAlpha(0.65f));
            g.setFont(16.0f);
            g.drawFittedText("Drop image A/B here or click Change Image A / Change Image B", imageRect, juce::Justification::centred, 2);
        }

        g.setColour(juce::Colour(0xff11161f));
        g.fillRoundedRectangle(juce::Rectangle<float>(22.0f, 80.0f, 240.0f, 320.0f), 10.0f);

        g.setColour(juce::Colour(0xff11161f));
        g.fillRoundedRectangle(juce::Rectangle<float>((float) getWidth() - 490.0f, 80.0f, 468.0f, 700.0f), 10.0f);

        if (deepDiveVisible)
        {
            g.setColour(juce::Colours::cyan.withAlpha(0.2f));
            g.drawRoundedRectangle(deepDiveGroup.getBounds().toFloat(), 8.0f, 1.5f);
        }
    }

    void PhotoSynthAudioProcessorEditor::resized()
    {
        const int margin = 20;

        auto area = getLocalBounds().reduced(margin);
        area.removeFromTop(60);

        auto leftPanel = area.removeFromLeft(250);
        auto centerPanel = area.removeFromLeft(560);
        auto rightPanel = area;

        auto gArea = leftPanel.reduced(10);
        timbreGauge.setBounds(gArea.removeFromTop(125));
        brightnessGauge.setBounds(gArea.removeFromTop(125));
        saturationGauge.setBounds(gArea.removeFromTop(125));
        complexityGauge.setBounds(gArea.removeFromTop(125));

        dropzone = centerPanel.removeFromTop(370).reduced(10);
        imageStatusLabel.setBounds(centerPanel.removeFromTop(26).reduced(10, 0));

        auto morphRow = centerPanel.removeFromTop(32).reduced(10, 0);
        morphSlider.setBounds(morphRow.removeFromLeft(420));
        bpmLabel.setBounds(morphRow);

        auto controls = centerPanel.removeFromTop(36).reduced(10, 0);
        changeImageAButton.setBounds(controls.removeFromLeft(132));
        controls.removeFromLeft(6);
        changeImageBButton.setBounds(controls.removeFromLeft(132));
        controls.removeFromLeft(6);
        savePresetButton.setBounds(controls.removeFromLeft(120));
        controls.removeFromLeft(6);
        loadPresetButton.setBounds(controls.removeFromLeft(120));

        auto controlRow2 = centerPanel.removeFromTop(36).reduced(10, 0);
        deepDiveButton.setBounds(controlRow2.removeFromLeft(120));
        controlRow2.removeFromLeft(8);
        effectsEditableButton.setBounds(controlRow2.removeFromLeft(140));
        controlRow2.removeFromLeft(8);
        tempoSyncButton.setBounds(controlRow2.removeFromLeft(120));
        controlRow2.removeFromLeft(8);
        bpmSlider.setBounds(controlRow2.removeFromLeft(180));

        oscilloscope.setBounds(centerPanel.removeFromTop(140).reduced(10));
        keyboard.setBounds(centerPanel.removeFromTop(80).reduced(10, 8));

        auto rightTop = rightPanel.removeFromTop(36);
        engineType.setBounds(rightTop.removeFromLeft(260));
        limiterButton.setBounds(rightTop.removeFromLeft(120));

        auto grid = rightPanel.reduced(8);
        const int knobW = 108, knobH = 122, gapX = 8;

        auto row1 = grid.removeFromTop(knobH);
        cutoff.setBounds(row1.removeFromLeft(knobW));
        row1.removeFromLeft(gapX);
        resonance.setBounds(row1.removeFromLeft(knobW));
        row1.removeFromLeft(gapX);
        lfoRate.setBounds(row1.removeFromLeft(knobW));
        row1.removeFromLeft(gapX);
        lfoDepth.setBounds(row1.removeFromLeft(knobW));

        auto row2 = grid.removeFromTop(knobH);
        volume.setBounds(row2.removeFromLeft(knobW));
        row2.removeFromLeft(gapX);
        attack.setBounds(row2.removeFromLeft(knobW));
        row2.removeFromLeft(gapX);
        decay.setBounds(row2.removeFromLeft(knobW));
        row2.removeFromLeft(gapX);
        sustain.setBounds(row2.removeFromLeft(knobW));

        auto row3 = grid.removeFromTop(knobH);
        release.setBounds(row3.removeFromLeft(knobW));
        row3.removeFromLeft(gapX);
        threshold.setBounds(row3.removeFromLeft(knobW));

        if (!deepDiveVisible)
            return;

        auto deepArea = grid.reduced(2);
        deepDiveGroup.setBounds(deepArea);
        deepArea.reduce(10, 28);

        auto placeLine = [&deepArea](juce::Slider& s)
        {
            s.setBounds(deepArea.removeFromTop(22));
            deepArea.removeFromTop(4);
        };

        placeLine(temporalEra);
        placeLine(opticalFocus);
        placeLine(gridSymmetry);
        placeLine(chromaticClash);
        placeLine(semanticDensity);

        deepArea.removeFromTop(8);

        placeLine(bodyDamping);
        placeLine(acousticWeight);
        placeLine(flutterSpeed);
        placeLine(flutterDepth);
        placeLine(warmth);

        deepArea.removeFromTop(8);

        placeLine(delayIntensity);
        placeLine(reverbIntensity);
        placeLine(chorusIntensity);
        placeLine(phaserIntensity);
        placeLine(flangerIntensity);
        placeLine(distortionIntensity);
    }
}
