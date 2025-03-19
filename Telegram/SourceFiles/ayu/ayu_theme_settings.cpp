// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2025
#include "ayu_theme_settings.h"

#include "ayu/ayu_constants.h"
#include "ayu/libs/json.hpp"
#include "ayu/libs/json_ext.hpp"
#include "app.h"
#include "core/file_utilities.h"
#include "platform/platform_specific.h"
#include "styles/style_overview.h"
#include "styles/style_window.h"
#include "ui/effects/animation_value.h"
#include "ui/platform/ui_platform_utility.h"
#include "ui/cached_round_corners.h"
#include "window/themes/window_theme.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QCoreApplication>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtGui/QPainter>

namespace AyuTheme {

namespace {
// Статическая инстанция настроек
std::unique_ptr<ThemeSettings> Instance;

// Реактивный поток для обновления темы
rpl::event_stream<> ThemeUpdateStream;
}

ThemeSettings::ThemeSettings() 
    : customThemeEnabled(false)
    , useCustomBubbleRadius(false)
    , bubbleRadius(16)
    , useCustomChatBackgrounds(false)
    , useCustomFonts(false)
    , fontSizeMultiplier(1.0)
    , useAnimatedEmoji(true)
    , useAnimatedStickers(true)
    , useChatBlurEffects(true)
    , useCustomReactions(false) {
    
    // Инициализация светлой темы по умолчанию
    lightTheme.textColor = parseColor("#000000");
    lightTheme.accentColor = parseColor("#2196F3");
    lightTheme.backgroundColor = parseColor("#FFFFFF");
    lightTheme.secondaryBackgroundColor = parseColor("#F5F5F5");
    lightTheme.bubbleBackgroundColor = parseColor("#F5F5F5");
    lightTheme.bubbleOutBackgroundColor = parseColor("#E3F2FD");
    lightTheme.linkColor = parseColor("#2196F3");
    lightTheme.headerColor = parseColor("#FFFFFF");
    lightTheme.headerBackgroundColor = parseColor("#2196F3");
    lightTheme.windowBackgroundColor = parseColor("#FFFFFF");
    
    // Инициализация темной темы по умолчанию
    darkTheme.textColor = parseColor("#FFFFFF");
    darkTheme.accentColor = parseColor("#42A5F5");
    darkTheme.backgroundColor = parseColor("#121212");
    darkTheme.secondaryBackgroundColor = parseColor("#1E1E1E");
    darkTheme.bubbleBackgroundColor = parseColor("#1E1E1E");
    darkTheme.bubbleOutBackgroundColor = parseColor("#0D47A1");
    darkTheme.linkColor = parseColor("#42A5F5");
    darkTheme.headerColor = parseColor("#FFFFFF");
    darkTheme.headerBackgroundColor = parseColor("#0D47A1");
    darkTheme.windowBackgroundColor = parseColor("#121212");
}

void load() {
    Instance = std::make_unique<ThemeSettings>();
    
    const auto settingsFile = cWorkingDir() + "tdata/ayu/theme_settings.json";
    QFile file(settingsFile);
    
    if (!file.exists()) {
        save(); // Создаем файл с дефолтными настройками
        return;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    
    auto data = file.readAll();
    file.close();
    
    if (data.isEmpty()) {
        return;
    }
    
    try {
        const auto json = nlohmann::json::parse(data.constData());
        *Instance = json.get<ThemeSettings>();
    } catch (...) {
        // В случае ошибки парсинга используем дефолтные настройки
    }
}

void save() {
    const auto directory = cWorkingDir() + "tdata/ayu";
    if (!QDir(directory).exists()) {
        QDir().mkpath(directory);
    }
    
    const auto settingsFile = directory + "/theme_settings.json";
    
    auto json = nlohmann::json(*Instance);
    const auto data = QString::fromStdString(json.dump(4)).toUtf8();
    
    QFile file(settingsFile);
    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }
    
    file.write(data);
    file.close();
}

ThemeSettings &getInstance() {
    if (!Instance) {
        load();
    }
    
    return *Instance;
}

void ThemeSettings::set_customThemeEnabled(bool val) {
    customThemeEnabled = val;
    
    if (val) {
        applyTheme();
    } else {
        // Возвращаем стандартную тему
        Window::Theme::Background()->revert();
    }
}

void ThemeSettings::set_lightTheme(ThemeColors val) {
    lightTheme = val;
    if (customThemeEnabled && !Window::Theme::IsNightMode()) {
        triggerThemeUpdate();
    }
}

void ThemeSettings::set_darkTheme(ThemeColors val) {
    darkTheme = val;
    if (customThemeEnabled && Window::Theme::IsNightMode()) {
        triggerThemeUpdate();
    }
}

void ThemeSettings::set_useCustomBubbleRadius(bool val) {
    useCustomBubbleRadius = val;
    if (val) {
        updateRoundCorners(bubbleRadius);
    } else {
        updateRoundCorners(Ui::CachedRoundCorners::kLarge);
    }
}

void ThemeSettings::set_bubbleRadius(int val) {
    bubbleRadius = val;
    if (useCustomBubbleRadius) {
        updateRoundCorners(val);
    }
}

void ThemeSettings::set_useCustomChatBackgrounds(bool val) {
    useCustomChatBackgrounds = val;
    // Обновление фонов чатов
    if (val) {
        const auto &background = Window::Theme::IsNightMode() ? 
            darkChatBackground : lightChatBackground;
        if (!background.isEmpty()) {
            Window::Theme::Background()->set(background, false);
        }
    } else {
        Window::Theme::Background()->revert();
    }
}

void ThemeSettings::set_lightChatBackground(QString val) {
    lightChatBackground = val;
    if (useCustomChatBackgrounds && !Window::Theme::IsNightMode()) {
        Window::Theme::Background()->set(val, false);
    }
}

void ThemeSettings::set_darkChatBackground(QString val) {
    darkChatBackground = val;
    if (useCustomChatBackgrounds && Window::Theme::IsNightMode()) {
        Window::Theme::Background()->set(val, false);
    }
}

void ThemeSettings::set_useCustomFonts(bool val) {
    useCustomFonts = val;
    triggerThemeUpdate();
}

void ThemeSettings::set_normalFont(QString val) {
    normalFont = val;
    if (useCustomFonts) {
        triggerThemeUpdate();
    }
}

void ThemeSettings::set_semiboldFont(QString val) {
    semiboldFont = val;
    if (useCustomFonts) {
        triggerThemeUpdate();
    }
}

void ThemeSettings::set_boldFont(QString val) {
    boldFont = val;
    if (useCustomFonts) {
        triggerThemeUpdate();
    }
}

void ThemeSettings::set_fontSizeMultiplier(double val) {
    fontSizeMultiplier = val;
    triggerThemeUpdate();
}

void ThemeSettings::set_useAnimatedEmoji(bool val) {
    useAnimatedEmoji = val;
    triggerThemeUpdate();
}

void ThemeSettings::set_useAnimatedStickers(bool val) {
    useAnimatedStickers = val;
    triggerThemeUpdate();
}

void ThemeSettings::set_useChatBlurEffects(bool val) {
    useChatBlurEffects = val;
    triggerThemeUpdate();
}

void ThemeSettings::set_useCustomReactions(bool val) {
    useCustomReactions = val;
    triggerThemeUpdate();
}

void ThemeSettings::set_customEmojiPacks(std::vector<QString> val) {
    customEmojiPacks = val;
    if (useCustomReactions) {
        triggerThemeUpdate();
    }
}

ThemeColors getCurrentTheme() {
    auto &instance = getInstance();
    return Window::Theme::IsNightMode() ? instance.darkTheme : instance.lightTheme;
}

void applyTheme() {
    auto &instance = getInstance();
    if (!instance.customThemeEnabled) {
        return;
    }
    
    const auto &theme = getCurrentTheme();
    
    // Применить цвета к текущей теме
    auto palette = Window::Theme::DefaultPalette();
    
    // Применяем основные цвета
    palette.msgServiceFg = theme.textColor;
    palette.historyTextFg = theme.textColor;
    palette.historyTextInFg = theme.textColor;
    palette.historyTextOutFg = theme.textColor;
    
    // Акцентные цвета
    palette.historyFileInIconFg = theme.accentColor;
    palette.historyFileOutIconFg = theme.accentColor;
    palette.windowActiveTextFg = theme.accentColor;
    palette.activeButtonFg = theme.accentColor;
    palette.historyLinkInFg = theme.linkColor;
    palette.historyLinkOutFg = theme.linkColor;
    
    // Фоны
    palette.windowBg = theme.windowBackgroundColor;
    palette.windowBgOver = theme.backgroundColor;
    palette.windowBgRipple = theme.backgroundColor.lighter(110);
    palette.historyPeer1NameFg = theme.accentColor;
    palette.historyPeer2NameFg = theme.accentColor;
    palette.historyPeer3NameFg = theme.accentColor;
    palette.historyPeer4NameFg = theme.accentColor;
    palette.historyPeer5NameFg = theme.accentColor;
    palette.historyPeer6NameFg = theme.accentColor;
    palette.historyPeer7NameFg = theme.accentColor;
    palette.historyPeer8NameFg = theme.accentColor;
    
    // Пузыри сообщений
    palette.msgInBg = theme.bubbleBackgroundColor;
    palette.msgInBgSelected = theme.bubbleBackgroundColor.darker(110);
    palette.msgOutBg = theme.bubbleOutBackgroundColor;
    palette.msgOutBgSelected = theme.bubbleOutBackgroundColor.darker(110);
    
    // Хедер
    palette.topBarBg = theme.headerBackgroundColor;
    palette.titleBg = theme.headerBackgroundColor;
    palette.titleShadow = QColor(0, 0, 0, 0);
    palette.titleButtonFg = theme.headerColor;
    
    // Установить новую цветовую палитру
    Window::Theme::SetPalette(palette);
    
    // Применить кастомные радиусы для пузырей
    if (instance.useCustomBubbleRadius) {
        updateRoundCorners(instance.bubbleRadius);
    }
    
    // Применить кастомный фон чата
    if (instance.useCustomChatBackgrounds) {
        const auto &background = Window::Theme::IsNightMode() ? 
            instance.darkChatBackground : instance.lightChatBackground;
        if (!background.isEmpty()) {
            Window::Theme::Background()->set(background, false);
        }
    }
    
    // Оповестить об изменении темы
    triggerThemeUpdate();
}

QColor parseColor(const QString &colorString) {
    if (colorString.isEmpty()) {
        return QColor();
    }
    
    if (colorString.startsWith('#')) {
        return QColor(colorString);
    }
    
    // Поддержка rgb/rgba форматов
    if (colorString.startsWith("rgba(") || colorString.startsWith("rgb(")) {
        const auto parts = colorString
            .mid(colorString.indexOf('(') + 1, colorString.length() - colorString.indexOf('(') - 2)
            .split(',');
        
        if (parts.size() >= 3) {
            const auto r = parts[0].trimmed().toInt();
            const auto g = parts[1].trimmed().toInt();
            const auto b = parts[2].trimmed().toInt();
            const auto a = (parts.size() == 4) ? parts[3].trimmed().toInt() : 255;
            
            return QColor(r, g, b, a);
        }
    }
    
    return QColor();
}

QString colorToString(const QColor &color) {
    if (!color.isValid()) {
        return QString();
    }
    
    return QString("#%1%2%3%4")
        .arg(color.red(), 2, 16, QChar('0'))
        .arg(color.green(), 2, 16, QChar('0'))
        .arg(color.blue(), 2, 16, QChar('0'))
        .arg(color.alpha() < 255 ? QString("%1").arg(color.alpha(), 2, 16, QChar('0')) : QString());
}

QByteArray generateCorners(int radius) {
    auto flag = Ui::CachedRoundCorners::None;
    auto size = 0;
    
    switch (radius) {
    case 0: return QByteArray();
    case 2: flag = Ui::CachedRoundCorners::kSmall; size = Ui::RoundRadiusSmall(); break;
    case 6: flag = Ui::CachedRoundCorners::kLargeBottom; size = Ui::RoundRadiusLarge(); break;
    case 12: flag = Ui::CachedRoundCorners::kLarge; size = Ui::RoundRadiusLarge(); break;
    default: 
        // Генерация кастомных уголков для радиуса
        auto result = QByteArray(4 * radius * radius, Qt::transparent);
        auto bits = result.bits();
        
        auto transparent = QColor(0, 0, 0, 0);
        auto black = QColor(0, 0, 0, 255);
        
        for (auto y = 0; y != radius; ++y) {
            for (auto x = 0; x != radius; ++x) {
                auto real = (x + 0.5 - radius) * (x + 0.5 - radius) + (y + 0.5 - radius) * (y + 0.5 - radius);
                auto opacity = (real < radius * radius) ? 255 : 0;
                
                auto index = (y * radius + x) * 4;
                bits[index + 0] = opacity ? black.blue() : transparent.blue();
                bits[index + 1] = opacity ? black.green() : transparent.green();
                bits[index + 2] = opacity ? black.red() : transparent.red();
                bits[index + 3] = opacity;
            }
        }
        
        return result;
    }
    
    return QByteArray();
}

void updateRoundCorners(int radius) {
    // Можно использовать стандартные корнеры, если радиус соответствует
    if (radius == Ui::RoundRadiusLarge()) {
        // Используем стандартные корнеры
        return;
    }
    
    // Генерируем кастомные корнеры для сообщений
    const auto corners = generateCorners(radius);
    if (!corners.isEmpty()) {
        // Здесь должно быть обновление кэшированных корнеров Telegram
        // в реальной реализации нужно обновить кэш корнеров в Ui::CachedRoundCorners
    }
}

rpl::producer<> get_themeUpdateReactive() {
    return ThemeUpdateStream.events();
}

void triggerThemeUpdate() {
    ThemeUpdateStream.fire({});
}

}