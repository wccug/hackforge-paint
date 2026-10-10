#ifndef COLORPICKERWINDOW_H
#ifndef COLORPICKERWINDOW_H
#define COLORPICKERWINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QColor>

class ColorPickerWindow : public QWidget {
    Q_OBJECT

public:
    ColorPickerWindow(QWidget *parent = nullptr);

private slots:
    void activateEyedropper();
    void updateColorValues(const QColor &color);

private:
    QColor currentColor;
    QPushButton *btnEyedropper;
    QLabel *lblColorPreview;
    QLineEdit *txtHexCode;
    QLineEdit *txtRgbCode;
    
    bool capturing;
    void pickColorAt(const QPoint &pos);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
};

#endif
#endif
