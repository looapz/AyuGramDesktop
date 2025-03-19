#include "ayu/ui/boxes/theme_editor_box.h"

#include "lang/lang_keys.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/checkbox.h"
#include "ui/widgets/shadow.h"
#include "ui/widgets/continuous_sliders.h"
#include "ui/wrap/vertical_layout.h"
#include "ui/wrap/slide_wrap.h"
#include "ui/wrap/padding_wrap.h"
#include "ui/layers/layer_manager.h"
#include "ui/text/text_utilities.h"
#include "styles/style_boxes.h"
#include "styles/style_layers.h"
#include "styles/style_settings.h"
#include "ayu/ayu_settings.h"
#include "ayu/ui/widgets/color_button.h"

namespace Ayu::Ui {

ThemeEditorBox::ThemeEditorBox(QWidget *parent, not_null<Window::SessionController*> controller)
	: Ui::BoxContent(parent)
	, _controller(controller)
	, _initialSettings(Ayu::AyuSettings::GetInstance()->GetThemeSettings()) {
	prepare();
}

ThemeEditorBox::~ThemeEditorBox() {
}

void ThemeEditorBox::prepare() {
	setTitle(tr::lng_ayu_theme_editor_title());
	
	const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
	
	// General category
	content->add(
		object_ptr<Ui::HeaderedDivider>(content, tr::lng_ayu_theme_general_category()));
	
	// Main accent color
	auto colorLayout = content->add(
		object_ptr<Ui::FixedHeightWidget>(
			content,
			st::settingsColorButton.height));
	
	_accentColorButton = Ui::CreateChild<Ayu::Ui::ColorButton>(
		colorLayout,
		tr::lng_ayu_theme_accent_color(),
		_initialSettings.accentColor);
	
	_accentColorButton->setGeometry(0, 0, colorLayout->width(), st::settingsColorButton.height);
	
	_accentColorButton->colorChanged(
	) | rpl::start_with_next([=](const QColor &color) {
		_modifiedSettings.accentColor = color;
		settingsChanged();
	}, _accentColorButton->lifetime());
	
	// Custom font options
	_customFontCheck = content->add(
		object_ptr<Ui::Checkbox>(
			content,
			tr::lng_ayu_theme_custom_font(),
			_initialSettings.useCustomFont,
			st::settingsCheckbox),
		st::settingsSectionSkip);
		
	_customFontCheck->checkedChanges(
	) | rpl::start_with_next([=](bool checked) {
		_modifiedSettings.useCustomFont = checked;
		_fontSlideWrap->toggleAnimated(checked);
		settingsChanged();
	}, _customFontCheck->lifetime());
	
	_fontSlideWrap = content->add(
		object_ptr<Ui::SlideWrap<Ui::VerticalLayout>>(
			content,
			object_ptr<Ui::VerticalLayout>(content)));
	
	const auto fontSettings = _fontSlideWrap->entity();
	
	// Font size slider
	fontSettings->add(
		object_ptr<Ui::FixedHeightWidget>(
			fontSettings,
			st::settingsSectionSkip));
	
	const auto fontSizeLabel = fontSettings->add(
		object_ptr<Ui::LabelWithNumbers>(
			fontSettings,
			st::settingsSliderLabel),
		style::margins(
			st::settingsSliderLabelMargin,
			0,
			st::settingsSliderLabelMargin,
			0));
	
	fontSizeLabel->setText(tr::lng_ayu_theme_font_size());
	
	_fontSizeSlider = fontSettings->add(
		object_ptr<Ui::MediaSlider>(
			fontSettings,
			st::settingsSlider),
		st::settingsSliderPadding);
	
	_fontSizeSlider->resize(st::settingsSlider.seekSize);
	_fontSizeSlider->setPseudoDiscrete(
		20,                             // steps
		[](int val) { return val + 8; }, // converter
		_initialSettings.fontSize,
		[=](int value) {                // changed callback
			_modifiedSettings.fontSize = value;
			fontSizeLabel->setNumber(value);
			settingsChanged();
		});
	
	// Font family selection
	// TODO: Add font family selection
	
	// Chat bubble settings
	content->add(
		object_ptr<Ui::HeaderedDivider>(content, tr::lng_ayu_theme_bubble_category()));
	
	// Rounded bubbles
	_roundedBubblesCheck = content->add(
		object_ptr<Ui::Checkbox>(
			content,
			tr::lng_ayu_theme_rounded_bubbles(),
			_initialSettings.useRoundedBubbles,
			st::settingsCheckbox),
		st::settingsSectionSkip);
		
	_roundedBubblesCheck->checkedChanges(
	) | rpl::start_with_next([=](bool checked) {
		_modifiedSettings.useRoundedBubbles = checked;
		_bubbleRadiusWrap->toggleAnimated(checked);
		settingsChanged();
	}, _roundedBubblesCheck->lifetime());
	
	// Bubble radius slider
	_bubbleRadiusWrap = content->add(
		object_ptr<Ui::SlideWrap<Ui::VerticalLayout>>(
			content,
			object_ptr<Ui::VerticalLayout>(content)));
	
	const auto bubbleSettings = _bubbleRadiusWrap->entity();
	
	bubbleSettings->add(
		object_ptr<Ui::FixedHeightWidget>(
			bubbleSettings,
			st::settingsSectionSkip));
	
	const auto bubbleRadiusLabel = bubbleSettings->add(
		object_ptr<Ui::LabelWithNumbers>(
			bubbleSettings,
			st::settingsSliderLabel),
		style::margins(
			st::settingsSliderLabelMargin,
			0,
			st::settingsSliderLabelMargin,
			0));
	
	bubbleRadiusLabel->setText(tr::lng_ayu_theme_bubble_radius());
	
	_bubbleRadiusSlider = bubbleSettings->add(
		object_ptr<Ui::MediaSlider>(
			bubbleSettings,
			st::settingsSlider),
		st::settingsSliderPadding);
	
	_bubbleRadiusSlider->resize(st::settingsSlider.seekSize);
	_bubbleRadiusSlider->setPseudoDiscrete(
		20,                                      // steps
		[](int val) { return val + 5; },        // converter
		_initialSettings.bubbleRadius,
		[=](int value) {                        // changed callback
			_modifiedSettings.bubbleRadius = value;
			bubbleRadiusLabel->setNumber(value);
			settingsChanged();
		});
	
	// Bubble colors
	auto inBubbleColorLayout = content->add(
		object_ptr<Ui::FixedHeightWidget>(
			content,
			st::settingsColorButton.height));
	
	_inBubbleColorButton = Ui::CreateChild<Ayu::Ui::ColorButton>(
		inBubbleColorLayout,
		tr::lng_ayu_theme_in_bubble_color(),
		_initialSettings.inBubbleColor);
	
	_inBubbleColorButton->setGeometry(0, 0, inBubbleColorLayout->width(), st::settingsColorButton.height);
	
	_inBubbleColorButton->colorChanged(
	) | rpl::start_with_next([=](const QColor &color) {
		_modifiedSettings.inBubbleColor = color;
		settingsChanged();
	}, _inBubbleColorButton->lifetime());
	
	auto outBubbleColorLayout = content->add(
		object_ptr<Ui::FixedHeightWidget>(
			content,
			st::settingsColorButton.height));
	
	_outBubbleColorButton = Ui::CreateChild<Ayu::Ui::ColorButton>(
		outBubbleColorLayout,
		tr::lng_ayu_theme_out_bubble_color(),
		_initialSettings.outBubbleColor);
	
	_outBubbleColorButton->setGeometry(0, 0, outBubbleColorLayout->width(), st::settingsColorButton.height);
	
	_outBubbleColorButton->colorChanged(
	) | rpl::start_with_next([=](const QColor &color) {
		_modifiedSettings.outBubbleColor = color;
		settingsChanged();
	}, _outBubbleColorButton->lifetime());
	
	// Background settings
	content->add(
		object_ptr<Ui::HeaderedDivider>(content, tr::lng_ayu_theme_background_category()));
	
	// Custom background color
	_customBgCheck = content->add(
		object_ptr<Ui::Checkbox>(
			content,
			tr::lng_ayu_theme_custom_bg(),
			_initialSettings.useCustomBackground,
			st::settingsCheckbox),
		st::settingsSectionSkip);
		
	_customBgCheck->checkedChanges(
	) | rpl::start_with_next([=](bool checked) {
		_modifiedSettings.useCustomBackground = checked;
		_bgColorWrap->toggleAnimated(checked);
		settingsChanged();
	}, _customBgCheck->lifetime());
	
	// Background color
	_bgColorWrap = content->add(
		object_ptr<Ui::SlideWrap<Ui::FixedHeightWidget>>(
			content,
			object_ptr<Ui::FixedHeightWidget>(
				content,
				st::settingsColorButton.height)));
	
	_bgColorButton = Ui::CreateChild<Ayu::Ui::ColorButton>(
		_bgColorWrap->entity(),
		tr::lng_ayu_theme_bg_color(),
		_initialSettings.backgroundColor);
	
	_bgColorButton->setGeometry(0, 0, _bgColorWrap->width(), st::settingsColorButton.height);
	
	_bgColorButton->colorChanged(
	) | rpl::start_with_next([=](const QColor &color) {
		_modifiedSettings.backgroundColor = color;
		settingsChanged();
	}, _bgColorButton->lifetime());
	
	// Bottom buttons
	addButton(tr::lng_settings_save(), [=] { save(); });
	addButton(tr::lng_cancel(), [=] { closeBox(); });
	
	_resetButton = addLeftButton(tr::lng_ayu_theme_reset(), [=] { reset(); });
	_resetButton->setVisible(false);
	
	// Set initial visibility states
	_fontSlideWrap->toggleFast(_initialSettings.useCustomFont);
	_bubbleRadiusWrap->toggleFast(_initialSettings.useRoundedBubbles);
	_bgColorWrap->toggleFast(_initialSettings.useCustomBackground);
	
	// Set content widget
	setDimensionsToContent(st::boxWideWidth, content);
	Ui::BoxContent::setInner(std::move(content));
}

void ThemeEditorBox::settingsChanged() {
	const bool changed = 
		_initialSettings.accentColor != _modifiedSettings.accentColor ||
		_initialSettings.useCustomFont != _modifiedSettings.useCustomFont ||
		_initialSettings.fontSize != _modifiedSettings.fontSize ||
		_initialSettings.useRoundedBubbles != _modifiedSettings.useRoundedBubbles ||
		_initialSettings.bubbleRadius != _modifiedSettings.bubbleRadius ||
		_initialSettings.inBubbleColor != _modifiedSettings.inBubbleColor ||
		_initialSettings.outBubbleColor != _modifiedSettings.outBubbleColor ||
		_initialSettings.useCustomBackground != _modifiedSettings.useCustomBackground ||
		_initialSettings.backgroundColor != _modifiedSettings.backgroundColor;
	
	_resetButton->setVisible(changed);
}

void ThemeEditorBox::save() {
	// Save settings to Ayu::AyuSettings
	auto settings = Ayu::AyuSettings::GetInstance();
	settings->SetThemeSettings(_modifiedSettings);
	
	// Apply settings visually
	applySettings();
	
	// Close dialog
	closeBox();
}

void ThemeEditorBox::reset() {
	// Reset sliders and checkboxes to initial values
	_accentColorButton->setColor(_initialSettings.accentColor);
	_customFontCheck->setChecked(_initialSettings.useCustomFont);
	_fontSizeSlider->setValue(_initialSettings.fontSize);
	_roundedBubblesCheck->setChecked(_initialSettings.useRoundedBubbles);
	_bubbleRadiusSlider->setValue(_initialSettings.bubbleRadius);
	_inBubbleColorButton->setColor(_initialSettings.inBubbleColor);
	_outBubbleColorButton->setColor(_initialSettings.outBubbleColor);
	_customBgCheck->setChecked(_initialSettings.useCustomBackground);
	_bgColorButton->setColor(_initialSettings.backgroundColor);
	
	// Reset modified settings to initial values
	_modifiedSettings = _initialSettings;
	
	// Update UI state
	settingsChanged();
}

void ThemeEditorBox::applySettings() {
	// Apply the theme settings to the application
	// This would typically involve updating the application's palette and styles
	
	// Emit signal that theme has changed
	// This would be connected to handlers that update the UI components
	Ayu::AyuSettings::GetInstance()->notifyThemeChanged();
}

} // namespace Ayu::Ui