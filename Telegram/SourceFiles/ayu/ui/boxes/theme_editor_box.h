// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2025
#pragma once

#include "ui/layers/box_content.h"
#include "ui/widgets/fields/input_field.h"
#include "ui/widgets/buttons.h"
#include "ayu/ayu_theme_settings.h"

namespace Ui {
class ColorButton;
class Checkbox;
class RoundButton;
class FlatLabel;
class VerticalLayout;
class SettingsButton;
} // namespace Ui

namespace AyuUi {

class ThemeEditorBox : public Ui::BoxContent {
public:
    ThemeEditorBox(QWidget *parent, bool isDarkMode);
    
protected:
    void prepare() override;
    
private:
    void setupContent();
    
    // Разделы настроек
    void setupColorSettings(not_null<Ui::VerticalLayout*> container);
    void setupFontSettings(not_null<Ui::VerticalLayout*> container);
    void setupBubbleSettings(not_null<Ui::VerticalLayout*> container);
    void setupBackgroundSettings(not_null<Ui::VerticalLayout*> container);
    void setupAnimationSettings(not_null<Ui::VerticalLayout*> container);
    
    // Создает цветной кнопку для выбора цвета
    not_null<Ui::ColorButton*> createColorButton(
        not_null<QWidget*> parent,
        const QColor &color);
    
    // Сохраняет настройки темы
    void saveThemeSettings();
    
    // Сбрасывает настройки темы на значения по умолчанию
    void resetThemeSettings();
    
    // Текущая тема (светлая или темная)
    AyuTheme::ThemeColors _currentTheme;
    
    // Флаг темной темы
    const bool _isDarkMode;
    
    // Цветовые кнопки для основных элементов интерфейса
    Ui::ColorButton *_textColorButton = nullptr;
    Ui::ColorButton *_accentColorButton = nullptr;
    Ui::ColorButton *_backgroundColorButton = nullptr;
    Ui::ColorButton *_secondaryBackgroundColorButton = nullptr;
    Ui::ColorButton *_bubbleBackgroundColorButton = nullptr;
    Ui::ColorButton *_bubbleOutBackgroundColorButton = nullptr;
    Ui::ColorButton *_linkColorButton = nullptr;
    Ui::ColorButton *_headerColorButton = nullptr;
    Ui::ColorButton *_headerBackgroundColorButton = nullptr;
    Ui::ColorButton *_windowBackgroundColorButton = nullptr;
    
    // Чекбоксы
    Ui::Checkbox *_useCustomBubbleRadiusCheckbox = nullptr;
    Ui::Checkbox *_useCustomBackgroundsCheckbox = nullptr;
    Ui::Checkbox *_useCustomFontsCheckbox = nullptr;
    Ui::Checkbox *_useAnimatedEmojiCheckbox = nullptr;
    Ui::Checkbox *_useAnimatedStickersCheckbox = nullptr;
    Ui::Checkbox *_useChatBlurEffectsCheckbox = nullptr;
    
    // Поля ввода
    Ui::InputField *_bubbleRadiusField = nullptr;
    Ui::InputField *_normalFontField = nullptr;
    Ui::InputField *_semiboldFontField = nullptr;
    Ui::InputField *_boldFontField = nullptr;
    Ui::InputField *_fontSizeField = nullptr;
    
    // Кнопки для фонов
    Ui::SettingsButton *_lightBackgroundButton = nullptr;
    Ui::SettingsButton *_darkBackgroundButton = nullptr;
    
    // Пути к выбранным фонам
    QString _selectedLightBackground;
    QString _selectedDarkBackground;
};

} // namespace AyuUi