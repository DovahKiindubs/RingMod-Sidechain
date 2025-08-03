/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RMSCAudioProcessorEditor::RMSCAudioProcessorEditor (RMSCAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
	mixSlider.setSliderStyle(juce::Slider::LinearBar);
	mixSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 60, 25);
    mixSlider.setColour(juce::Slider::trackColourId,
        juce::Colour(0Xff4e445f));
	mixSlider.setRange(0.0f, 1.0f, 0.001f);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "mix", mixSlider);
    mixSlider.textFromValueFunction = [](double value) {
        return juce::String(value * 100.0, 0.1) + " %"; 
        };
    mixSlider.updateText();
    addAndMakeVisible(mixSlider);

	bypasslabel.setText("RingMod Sidechain", juce::dontSendNotification);
	bypasslabel.setFont(juce::Font(20.0f));
	bypasslabel.setJustificationType(juce::Justification::centredLeft);
	bypasslabel.setColour(juce::Label::textColourId, juce::Colours::whitesmoke);
	addAndMakeVisible(bypasslabel);

    bypassButton.setClickingTogglesState(true);
    bypassButton.setColour(juce::TextButton::buttonOnColourId,
        juce::Colour(0xff73d3d1));
    bypassButton.setColour(juce::TextButton::buttonColourId,juce::Colour(0XFF342d40));
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "bypass", bypassButton);
    bypassButton.onClick = [this] {
        audioProcessor.sidechainvisual.setVisible(bypassButton.getToggleState());
        bypassButton.repaint();
        };
    addAndMakeVisible(bypassButton);


    audioProcessor.mainvisual.setColours(juce::Colour(0xff1b1722), juce::Colour(0xffe4627c));
    addAndMakeVisible(audioProcessor.mainvisual);
    audioProcessor.sidechainvisual.setColours(juce::Colours::transparentBlack, juce::Colours::whitesmoke);
    audioProcessor.sidechainvisual.setOpaque(false);
	addAndMakeVisible(audioProcessor.sidechainvisual);

    setSize (300, 200);
}

RMSCAudioProcessorEditor::~RMSCAudioProcessorEditor()
{
}

//==============================================================================
void RMSCAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (juce::Colour(0XFF34373d));
    g.fillAll(juce::Colour(0xff342d40));
    g.setColour (juce::Colours::whitesmoke);
    g.setFont (juce::FontOptions (20.0f));
    g.setColour(juce::Colour(0XFFe2617b)); 
    g.drawLine(42, 37, 190, 37, 2); 

}

void RMSCAudioProcessorEditor::resized()
{
	mixSlider.setBounds(215, 10, 70, 25);
	bypassButton.setBounds(10, 10, 25, 25);
    bypasslabel.setBounds(40, 7, 150, 25);

    int visualiserHeight = getHeight() - 60;
	audioProcessor.mainvisual.setBounds(10, 50, getWidth() - 20, visualiserHeight);
	audioProcessor.sidechainvisual.setBounds(10, 50, getWidth() - 20, visualiserHeight);
}
