#pragma once

#include "ui/widgets/buttons.h"
#include "ui/effects/animations.h"
#include "base/object_ptr.h"

namespace Ayu::Ui {

// Кнопка для выбора цвета с возможностью отображения предпросмотра
class ColorButton : public Ui::RippleButton {
public:
    ColorButton(
        QWidget *parent,
        rpl::producer<QString> text,
        QColor initialColor = Qt::transparent);
    
    ~ColorButton();

    [[nodiscard]] QColor color() const;
    void setColor(QColor color);

    void setColorDialogTitle(const QString &title);

    [[nodiscard]] rpl::producer<QColor> colorChanged() const;

protected:
    int resizeGetHeight(int newWidth) override;
    void paintEvent(QPaintEvent *e) override;

private:
    void showColorDialog();
    void updateButtonStyle();

    QColor _color;
    QString _colorDialogTitle;
    Ui::Text::String _text;
    
    rpl::event_stream<QColor> _colorChanges;
    Ui::Animations::Simple _colorChangeAnimation;
    
    QColor _animationStartColor;
    QColor _animationTargetColor;
    crl::time _lastChangeTime = 0;
};

} // namespace Ayu::Ui