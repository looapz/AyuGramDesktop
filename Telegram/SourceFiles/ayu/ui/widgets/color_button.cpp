#include "ayu/ui/widgets/color_button.h"

#include "styles/style_settings.h"
#include "styles/palette.h"
#include "lang/lang_keys.h"
#include "ui/effects/ripple_animation.h"
#include "ui/painter.h"
#include "ui/text/text_utilities.h"

#include <QColorDialog>

namespace Ayu::Ui {

namespace {

constexpr auto kAnimationDuration = 200;
constexpr auto kColorPreviewSize = 16;
constexpr auto kColorAnimationPeriod = crl::time(500);

} // namespace

ColorButton::ColorButton(
    QWidget *parent,
    rpl::producer<QString> text,
    QColor initialColor)
: Ui::RippleButton(parent, st::settingsButton.ripple)
, _color(initialColor)
, _colorDialogTitle(tr::lng_ayu_color_dialog_title(tr::now))
, _text(st::settingsButton.style, std::move(text)) {
    setCursor(style::cur_pointer);
    
    const auto height = st::settingsButton.height;
    resize(width(), height);
    
    updateButtonStyle();
    
    clicks(
    ) | rpl::start_with_next([=] {
        showColorDialog();
    }, lifetime());
}

ColorButton::~ColorButton() = default;

QColor ColorButton::color() const {
    return _color;
}

void ColorButton::setColor(QColor color) {
    if (_color == color) {
        return;
    }
    
    // Сохраняем предыдущий цвет и устанавливаем новый целевой цвет
    _animationStartColor = _color;
    _animationTargetColor = color;
    _color = color;
    
    // Запускаем анимацию смены цвета
    _lastChangeTime = crl::now();
    if (!_colorChangeAnimation.animating()) {
        _colorChangeAnimation.start([=] {
            update();
        }, 0., 1., kAnimationDuration);
    }
    
    _colorChanges.fire_copy(color);
    updateButtonStyle();
    update();
}

rpl::producer<QColor> ColorButton::colorChanged() const {
    return _colorChanges.events();
}

void ColorButton::setColorDialogTitle(const QString &title) {
    _colorDialogTitle = title;
}

int ColorButton::resizeGetHeight(int newWidth) {
    updateButtonStyle();
    return st::settingsButton.height;
}

void ColorButton::paintEvent(QPaintEvent *e) {
    auto p = QPainter(this);
    
    // Анимация при переходе от одного цвета к другому
    auto progress = _colorChangeAnimation.value(1.);
    
    auto paintColorRect = [&](QRect rect, QColor color) {
        if (color.alpha() == 0) {
            // Для прозрачного цвета рисуем шахматный узор
            static QPixmap transparent(Ui::TransparentThumbCached());
            p.drawPixmap(rect, transparent, QRect(0, 0, rect.width(), rect.height()));
            p.drawRect(rect);
        } else {
            p.fillRect(rect, color);
            p.drawRect(rect);
        }
    };
    
    // Отрисовка фона кнопки
    const auto &st = st::settingsButton;
    const auto height = st.height;
    const auto over = isOver();
    p.fillRect(rect(), over ? st.overBgColor : st.bgColor);
    
    // Позиция цветного индикатора
    const auto textLeft = st.padding.left();
    const auto colorLeft = width() - st.padding.right() - kColorPreviewSize;
    const auto colorTop = (height - kColorPreviewSize) / 2;
    const auto colorRect = QRect(colorLeft, colorTop, kColorPreviewSize, kColorPreviewSize);
    
    // Отрисовка текста
    p.setPen(over ? st.overFg : st.fg);
    const auto textWidth = colorLeft - textLeft - st.padding.right();
    _text.drawLeftElided(p, textLeft, (height - st.style.font->height) / 2, textWidth, width());
    
    // Отрисовка цветного индикатора с анимацией
    if (progress < 1. && _lastChangeTime > 0) {
        // Интерполируем цвет для плавного перехода
        QColor currentColor;
        currentColor.setRedF(
            _animationStartColor.redF() * (1. - progress) + 
            _animationTargetColor.redF() * progress);
        currentColor.setGreenF(
            _animationStartColor.greenF() * (1. - progress) + 
            _animationTargetColor.greenF() * progress);
        currentColor.setBlueF(
            _animationStartColor.blueF() * (1. - progress) + 
            _animationTargetColor.blueF() * progress);
        currentColor.setAlphaF(
            _animationStartColor.alphaF() * (1. - progress) + 
            _animationTargetColor.alphaF() * progress);
        
        paintColorRect(colorRect, currentColor);
    } else {
        paintColorRect(colorRect, _color);
    }
    
    // Отрисовка эффекта нажатия
    paintRipple(p, 0, 0);
}

void ColorButton::showColorDialog() {
    const auto initialColor = _color.isValid() ? _color : QColor(Qt::white);
    const auto color = QColorDialog::getColor(
        initialColor,
        this,
        _colorDialogTitle,
        QColorDialog::ShowAlphaChannel);
    
    if (color.isValid()) {
        setColor(color);
    }
}

void ColorButton::updateButtonStyle() {
    const auto inner = rect().marginsRemoved(st::settingsButton.padding);
    const auto left = inner.x();
    const auto availableWidth = inner.width();
    
    // Устанавливаем область эффекта нажатия
    setRippleArea(QRect(0, 0, width(), st::settingsButton.height));
}

} // namespace Ayu::Ui