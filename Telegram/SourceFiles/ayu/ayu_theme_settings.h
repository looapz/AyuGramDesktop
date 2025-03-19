// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2025
#pragma once

#include "ayu/libs/json.hpp"
#include "ayu/libs/json_ext.hpp"
#include "ui/effects/animation_value.h"
#include "ui/cached_round_corners.h"

namespace AyuTheme {

struct ThemeColors {
    QColor textColor;
    QColor accentColor;
    QColor backgroundColor;
    QColor secondaryBackgroundColor;
    QColor bubbleBackgroundColor;
    QColor bubbleOutBackgroundColor;
    QColor linkColor;
    QColor headerColor;
    QColor headerBackgroundColor;
    QColor windowBackgroundColor;
};

class ThemeSettings {
public:
    ThemeSettings();

    bool customThemeEnabled;
    ThemeColors lightTheme;
    ThemeColors darkTheme;
    
    bool useCustomBubbleRadius;
    int bubbleRadius;
    
    bool useCustomChatBackgrounds;
    QString lightChatBackground;
    QString darkChatBackground;
    
    bool useCustomFonts;
    QString normalFont;
    QString semiboldFont;
    QString boldFont;
    double fontSizeMultiplier;
    
    bool useAnimatedEmoji;
    bool useAnimatedStickers;
    
    bool useChatBlurEffects;
    bool useCustomReactions;
    
    std::vector<QString> customEmojiPacks;
    
    void set_customThemeEnabled(bool val);
    void set_lightTheme(ThemeColors val);
    void set_darkTheme(ThemeColors val);
    
    void set_useCustomBubbleRadius(bool val);
    void set_bubbleRadius(int val);
    
    void set_useCustomChatBackgrounds(bool val);
    void set_lightChatBackground(QString val);
    void set_darkChatBackground(QString val);
    
    void set_useCustomFonts(bool val);
    void set_normalFont(QString val);
    void set_semiboldFont(QString val);
    void set_boldFont(QString val);
    void set_fontSizeMultiplier(double val);
    
    void set_useAnimatedEmoji(bool val);
    void set_useAnimatedStickers(bool val);
    
    void set_useChatBlurEffects(bool val);
    void set_useCustomReactions(bool val);
    
    void set_customEmojiPacks(std::vector<QString> val);
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
    ThemeColors,
    textColor,
    accentColor,
    backgroundColor,
    secondaryBackgroundColor,
    bubbleBackgroundColor,
    bubbleOutBackgroundColor,
    linkColor,
    headerColor,
    headerBackgroundColor,
    windowBackgroundColor
);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
    ThemeSettings,
    customThemeEnabled,
    lightTheme,
    darkTheme,
    useCustomBubbleRadius,
    bubbleRadius,
    useCustomChatBackgrounds,
    lightChatBackground,
    darkChatBackground,
    useCustomFonts,
    normalFont,
    semiboldFont,
    boldFont,
    fontSizeMultiplier,
    useAnimatedEmoji,
    useAnimatedStickers,
    useChatBlurEffects,
    useCustomReactions,
    customEmojiPacks
);

ThemeSettings &getInstance();

void load();
void save();

// Получить текущую цветовую тему в зависимости от активного светлого/темного режима
ThemeColors getCurrentTheme();

// Включить применение темы
void applyTheme();

QColor parseColor(const QString &colorString);
QString colorToString(const QColor &color);

// Utilities
QByteArray generateCorners(int radius);
void updateRoundCorners(int radius);

// Reactive update signals
rpl::producer<> get_themeUpdateReactive();
void triggerThemeUpdate();

}