#include "ayu/ui/boxes/theme_editor_box.h"

#include "styles/style_boxes.h"
#include "styles/style_chat.h"
#include "styles/style_layers.h"
#include "styles/style_menu_icons.h"
#include "ui/boxes/confirm_box.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/checkbox.h"
#include "ui/widgets/input_fields.h"
#include "ui/widgets/labels.h"
#include "ui/wrap/vertical_layout.h"
#include "ui/wrap/slide_wrap.h"
#include "ui/toast/toast.h"
#include "ui/text/text_utilities.h"
#include "lang/lang_keys.h"
#include "ayu/config.h"
#include "window/window_session_controller.h"
#include "main/main_session.h"
#include "base/platform/base_platform_info.h"
#include "core/application.h"
#include "core/core_settings.h"

#include <QColorDialog>

namespace Ayu {

// Вспомогательный класс для создания кнопки выбора цвета
class ColorButton : public Ui::RippleButton {
public:
    ColorButton(QWidget *parent, const QString &title, const QColor &initialColor)
        : Ui::RippleButton(parent, st::defaultRippleAnimation)
        , _title(title)
        , _color(initialColor) {
        resize(st::defaultColorButton.width, st::defaultColorButton.height);
        setClickedCallback([this] { showColorDialog(); });
    }

    void setColor(const QColor &color) {
        if (_color != color) {
            _color = color;
            update();
            if (_callback) {
                _callback(_color);
            }
        }
    }

    QColor color() const {
        return _color;
    }

    void setColorChangeCallback(Fn<void(QColor)> callback) {
        _callback = std::move(callback);
    }

protected:
    void paintEvent(QPaintEvent *e) override {
        Painter p(this);
        
        // Рисуем фон кнопки
        p.fillRect(rect(), st::boxBg);
        
        // Рисуем образец цвета
        const auto colorRect = QRect(
            st::defaultColorButton.padding.left(),
            st::defaultColorButton.padding.top(),
            st::defaultColorButton.sampleSize,
            st::defaultColorButton.sampleSize);
        
        p.fillRect(colorRect, _color);
        p.setPen(st::defaultColorButton.border);
        p.drawRect(colorRect.adjusted(0, 0, -1, -1));
        
        // Рисуем текст
        p.setPen(st::defaultColorButton.textFg);
        p.setFont(st::defaultColorButton.font);
        p.drawText(
            st::defaultColorButton.padding.left() + st::defaultColorButton.sampleSize + st::defaultColorButton.textLeft,
            st::defaultColorButton.padding.top() + st::defaultColorButton.textTop + st::defaultColorButton.font->ascent,
            _title);
        
        // Рисуем ripple-эффект
        if (hasRipple()) {
            paintRipple(p, 0, 0);
        }
    }

private:
    void showColorDialog() {
        QColorDialog dialog(parentWidget());
        dialog.setCurrentColor(_color);
        if (dialog.exec() == QDialog::Accepted) {
            setColor(dialog.currentColor());
        }
    }

    QString _title;
    QColor _color;
    Fn<void(QColor)> _callback;
};

// Реализация класса ThemeEditorBox
ThemeEditorBox::ThemeEditorBox(QWidget *parent, not_null<Window::SessionController*> controller)
    : Ui::BoxContent(parent)
    , _controller(controller) {
    // Загрузка текущих настроек темы из конфигурации
    loadSettings();
}

void ThemeEditorBox::loadSettings() {
    // Загружаем настройки из конфигурации или устанавливаем значения по умолчанию
    auto &settings = Core::App().settings();
    
    // Цвета чата
    _messageInBg = st::msgInBg->c;
    _messageOutBg = st::msgOutBg->c;
    _messageBgSelected = st::msgSelectBg->c;
    _messageBgSelectedOverlay = st::msgSelectOverlay->c;
    
    // Настройки шрифтов
    _fontSizeMultiplier = settings.chatFontSizeMultiplier();
    _useSystemFont = settings.useSystemFont();
    
    // Настройки пузырей
    _useBubbles = settings.chatBubble();
    _useWideForOutboundMessages = settings.chatWide();
    _useCornersForOutboundMessages = settings.chatRounded();
    
    // Настройки фона
    _useCustomChatBackground = !Ayu::config().disableCustomBackgrounds();
    _customChatBackground = settings.chatBackgroundPath();
}

void ThemeEditorBox::prepare() {
    setTitle(tr::lng_ayu_theme_editor_title());
    
    addButton(tr::lng_settings_save(), [=] { save(); });
    addButton(tr::lng_cancel(), [=] { closeBox(); });
    
    setupContent();
}

void ThemeEditorBox::setupContent() {
    const auto content = verticalLayout();
    
    // Добавляем группы настроек
    setupColorSettings(content);
    setupFontSettings(content);
    setupBubbleSettings(content);
    setupBackgroundSettings(content);
    
    // Добавляем кнопку сброса настроек
    const auto resetButton = content->add(
        object_ptr<Ui::LinkButton>(
            content,
            tr::lng_ayu_theme_editor_reset(tr::now)),
        st::boxLinkButton);
    
    resetButton->setClickedCallback([=] {
        showResetConfirmation();
    });
}

void ThemeEditorBox::setupColorSettings(not_null<Ui::VerticalLayout*> container) {
    container->add(
        object_ptr<Ui::GenericBox::Section>(
            container,
            tr::lng_ayu_theme_editor_colors(tr::now),
            st::boxTitle),
        style::margins(0, 0, 0, 10));

    // Кнопки выбора цвета для сообщений
    auto createColorButton = [&](const QString &title, const QColor &initialColor, auto callback) {
        auto button = object_ptr<ColorButton>(container, title, initialColor);
        button->setColorChangeCallback(std::move(callback));
        container->add(std::move(button), st::boxRowPadding);
    };

    // Входящие сообщения
    createColorButton(
        tr::lng_ayu_theme_editor_message_in_bg(tr::now),
        _messageInBg,
        [this](QColor color) { _messageInBg = color; }
    );

    // Исходящие сообщения
    createColorButton(
        tr::lng_ayu_theme_editor_message_out_bg(tr::now),
        _messageOutBg,
        [this](QColor color) { _messageOutBg = color; }
    );

    // Выделенные сообщения
    createColorButton(
        tr::lng_ayu_theme_editor_message_selected_bg(tr::now),
        _messageBgSelected,
        [this](QColor color) { _messageBgSelected = color; }
    );

    // Оверлей для выделенных сообщений
    createColorButton(
        tr::lng_ayu_theme_editor_message_selected_overlay(tr::now),
        _messageBgSelectedOverlay,
        [this](QColor color) { _messageBgSelectedOverlay = color; }
    );
}

void ThemeEditorBox::setupFontSettings(not_null<Ui::VerticalLayout*> container) {
    container->add(
        object_ptr<Ui::GenericBox::Section>(
            container,
            tr::lng_ayu_theme_editor_fonts(tr::now),
            st::boxTitle),
        style::margins(0, 10, 0, 10));

    // Использовать системный шрифт
    _systemFontCheckbox = container->add(
        object_ptr<Ui::Checkbox>(
            container,
            tr::lng_ayu_theme_editor_use_system_font(tr::now),
            _useSystemFont,
            st::defaultCheckbox),
        st::boxRowPadding);

    // Размер шрифта
    container->add(
        object_ptr<Ui::FlatLabel>(
            container,
            tr::lng_ayu_theme_editor_font_size(tr::now),
            st::boxLabel),
        st::boxRowPadding);

    const auto fontSizes = std::vector<QString>{
        "85%", "100%", "115%", "125%"
    };

    const auto fontSizeGroup = std::make_shared<Ui::RadiobuttonGroup>(_fontSizeMultiplier - 1);
    
    for (auto i = 0; i < fontSizes.size(); i++) {
        container->add(
            object_ptr<Ui::Radiobutton>(
                container,
                fontSizeGroup,
                i,
                fontSizes[i],
                st::defaultRadiobutton),
            st::boxRowPadding);
    }

    fontSizeGroup->setChangedCallback([this](int value) {
        _fontSizeMultiplier = value + 1;
    });
}

void ThemeEditorBox::setupBubbleSettings(not_null<Ui::VerticalLayout*> container) {
    container->add(
        object_ptr<Ui::GenericBox::Section>(
            container,
            tr::lng_ayu_theme_editor_bubbles(tr::now),
            st::boxTitle),
        style::margins(0, 10, 0, 10));

    // Использовать пузыри для сообщений
    _useBubblesCheckbox = container->add(
        object_ptr<Ui::Checkbox>(
            container,
            tr::lng_ayu_theme_editor_use_bubbles(tr::now),
            _useBubbles,
            st::defaultCheckbox),
        st::boxRowPadding);

    // Дополнительные опции для пузырей
    auto createBubbleOptionsWrap = [&] {
        return container->add(
            object_ptr<Ui::SlideWrap<Ui::VerticalLayout>>(
                container,
                object_ptr<Ui::VerticalLayout>(container)),
            st::boxRowPadding);
    };

    _bubbleOptionsWrap = createBubbleOptionsWrap();
    const auto inner = _bubbleOptionsWrap->entity();

    // Широкие пузыри для исходящих сообщений
    _wideOutboundCheckbox = inner->add(
        object_ptr<Ui::Checkbox>(
            inner,
            tr::lng_ayu_theme_editor_use_wide_outbound(tr::now),
            _useWideForOutboundMessages,
            st::defaultCheckbox),
        st::boxRowPadding);

    // Закругленные углы для исходящих сообщений
    _roundedOutboundCheckbox = inner->add(
        object_ptr<Ui::Checkbox>(
            inner,
            tr::lng_ayu_theme_editor_use_corners_outbound(tr::now),
            _useCornersForOutboundMessages,
            st::defaultCheckbox),
        st::boxRowPadding);

    // Обновляем видимость опций пузырей
    _bubbleOptionsWrap->toggle(_useBubbles, anim::type::normal);
    
    // Добавляем обработчик для чекбокса пузырей
    _useBubblesCheckbox->checkedChanges(
    ) | rpl::start_with_next([=](bool checked) {
        _useBubbles = checked;
        _bubbleOptionsWrap->toggle(checked, anim::type::normal);
    }, lifetime());
}

void ThemeEditorBox::setupBackgroundSettings(not_null<Ui::VerticalLayout*> container) {
    container->add(
        object_ptr<Ui::GenericBox::Section>(
            container,
            tr::lng_ayu_theme_editor_background(tr::now),
            st::boxTitle),
        style::margins(0, 10, 0, 10));

    // Использовать пользовательский фон чата
    _useCustomBackgroundCheckbox = container->add(
        object_ptr<Ui::Checkbox>(
            container,
            tr::lng_ayu_theme_editor_use_custom_background(tr::now),
            _useCustomChatBackground,
            st::defaultCheckbox),
        st::boxRowPadding);

    // Выбор фона чата
    auto createBackgroundSelectorWrap = [&] {
        return container->add(
            object_ptr<Ui::SlideWrap<Ui::VerticalLayout>>(
                container,
                object_ptr<Ui::VerticalLayout>(container)),
            st::boxRowPadding);
    };

    _backgroundSelectorWrap = createBackgroundSelectorWrap();
    const auto inner = _backgroundSelectorWrap->entity();

    // Текущий фон
    inner->add(
        object_ptr<Ui::FlatLabel>(
            inner,
            _customChatBackground.isEmpty() 
                ? tr::lng_ayu_theme_editor_no_background(tr::now)
                : QFileInfo(_customChatBackground).fileName(),
            st::boxLabel),
        st::boxRowPadding);

    // Кнопка выбора фона
    inner->add(
        object_ptr<Ui::LinkButton>(
            inner,
            tr::lng_ayu_theme_editor_select_background(tr::now)),
        st::boxRowPadding)->setClickedCallback([=] {
            chooseBackground();
        });

    // Обновляем видимость селектора фона
    _backgroundSelectorWrap->toggle(_useCustomChatBackground, anim::type::normal);
    
    // Добавляем обработчик для чекбокса пользовательского фона
    _useCustomBackgroundCheckbox->checkedChanges(
    ) | rpl::start_with_next([=](bool checked) {
        _useCustomChatBackground = checked;
        _backgroundSelectorWrap->toggle(checked, anim::type::normal);
    }, lifetime());
}

void ThemeEditorBox::chooseBackground() {
    // Реализация выбора файла фона чата
    // В реальном приложении здесь должен быть диалог выбора файла
    Ui::Toast::Show(box(), "Выбор фона будет реализован позже");
}

void ThemeEditorBox::showResetConfirmation() {
    const auto weak = Ui::MakeWeak(this);
    Ui::show(Ui::MakeConfirmBox({
        .text = tr::lng_ayu_theme_editor_reset_confirm(),
        .confirmed = [=] {
            if (weak) {
                resetSettings();
            }
        },
        .confirmText = tr::lng_ayu_theme_editor_reset_yes(),
        .cancelText = tr::lng_cancel(),
    }));
}

void ThemeEditorBox::resetSettings() {
    // Сбрасываем настройки на значения по умолчанию
    loadSettings();
    
    // Перезагружаем интерфейс
    closeBox();
    _controller->show(Box<ThemeEditorBox>(_controller));
    
    Ui::Toast::Show(tr::lng_ayu_theme_editor_reset_done(tr::now));
}

void ThemeEditorBox::save() {
    auto &settings = Core::App().settings();
    
    // Сохраняем настройки цветов
    // Здесь должна быть реализация сохранения цветов в настройки темы
    
    // Сохраняем настройки шрифтов
    settings.setChatFontSizeMultiplier(_fontSizeMultiplier);
    settings.setUseSystemFont(_systemFontCheckbox->checked());
    
    // Сохраняем настройки пузырей
    settings.setChatBubble(_useBubblesCheckbox->checked());
    if (_useBubblesCheckbox->checked()) {
        settings.setChatWide(_wideOutboundCheckbox->checked());
        settings.setChatRounded(_roundedOutboundCheckbox->checked());
    }
    
    // Сохраняем настройки фона
    Ayu::config().setDisableCustomBackgrounds(!_useCustomBackgroundCheckbox->checked());
    
    // Применяем настройки
    Core::App().saveSettingsDelayed();
    Ayu::config().save();
    
    // Показываем уведомление
    Ui::Toast::Show(tr::lng_ayu_theme_editor_saved(tr::now));
    
    // Закрываем окно
    closeBox();
}

} // namespace Ayu