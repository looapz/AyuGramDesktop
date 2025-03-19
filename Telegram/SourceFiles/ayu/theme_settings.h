#pragma once

#include <QColor>
#include <QString>

namespace Ayu {

// Структура для хранения настроек темы
struct ThemeSettings {
    // Основные цвета
    QColor accentColor = QColor(0, 136, 204);  // Цвет акцента по умолчанию (синий)
    
    // Настройки шрифта
    bool useCustomFont = false;
    int fontSize = 13;  // Значение от 8 до 28
    QString fontFamily = "Open Sans";
    
    // Настройки пузырей сообщений
    bool useRoundedBubbles = true;
    int bubbleRadius = 10;  // Значение от 5 до 25
    QColor inBubbleColor = QColor(255, 255, 255);  // Белый цвет для входящих сообщений
    QColor outBubbleColor = QColor(231, 249, 231);  // Светло-зеленый для исходящих
    
    // Настройки фона
    bool useCustomBackground = false;
    QColor backgroundColor = QColor(240, 240, 240);  // Светло-серый фон по умолчанию
    
    // Оператор сравнения для проверки изменений настроек
    bool operator==(const ThemeSettings &other) const {
        return 
            accentColor == other.accentColor &&
            useCustomFont == other.useCustomFont &&
            fontSize == other.fontSize &&
            fontFamily == other.fontFamily &&
            useRoundedBubbles == other.useRoundedBubbles &&
            bubbleRadius == other.bubbleRadius &&
            inBubbleColor == other.inBubbleColor &&
            outBubbleColor == other.outBubbleColor &&
            useCustomBackground == other.useCustomBackground &&
            backgroundColor == other.backgroundColor;
    }
    
    bool operator!=(const ThemeSettings &other) const {
        return !(*this == other);
    }
};

} // namespace Ayu