#include "ColorPickerWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScreen>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QCursor>
#include <QPixmap>
#include <QPainter>

ColorPickerWindow::ColorPickerWindow(QWidget *parent)
    : QWidget(parent), capturing(false) {
    
    setWindowTitle("Hackforge Paint - Linux Color Picker (clandrews Port)");
    setFixedSize(300, 160);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Color Preview Box
    lblColorPreview = new QLabel(this);
    lblColorPreview->setFixedHeight(50);
    lblColorPreview->setAutoFillBackground(true);
    updateColorValues(QColor(41, 128, 185)); // Default initial color
    mainLayout->addWidget(lblColorPreview);

    // Eyedropper Button
    btnEyedropper = new QPushButton("Pick Color from Screen", this);
    mainLayout->addWidget(btnEyedropper);

    // Text details layout (Hex / RGB)
    QHBoxLayout *textLayout = new QHBoxLayout();
    txtHexCode = new QLineEdit(this);
    txtHexCode->setReadOnly(true);
    txtRgbCode = new QLineEdit(this);
    txtRgbCode->setReadOnly(true);
    
    textLayout->addWidget(txtHexCode);
    textLayout->addWidget(txtRgbCode);
    mainLayout->addLayout(textLayout);

    // Connect signals
    connect(btnEyedropper, &QPushButton::clicked, this, &ColorPickerWindow::activateEyedropper);
}

void ColorPickerWindow::activateEyedropper() {
    capturing = true;
    setCursor(Qt::CrossCursor);
    // Hide main UI temporarily to sample screen underneath cleanly if desired, 
    // or keep operational depending on user choice.
    this->setWindowOpacity(0.3); 
}

void ColorPickerWindow::mousePressEvent(QMouseEvent *event) {
    if (capturing && event->button() == Qt::LeftButton) {
        pickColorAt(QCursor::pos());
        capturing = false;
        setCursor(Qt::ArrowCursor);
        this->setWindowOpacity(1.0);
        event->accept();
    } else {
        QWidget::mousePressEvent(event);
    }
}

void ColorPickerWindow::mouseMoveEvent(QMouseEvent *event) {
    if (capturing) {
        // Optional: Live preview tracking while dragging
        event->accept();
    } else {
        QWidget::mouseMoveEvent(event);
    }
}

void ColorPickerWindow::mouseReleaseEvent(QMouseEvent *event) {
    QWidget::mouseReleaseEvent(event);
}

void ColorPickerWindow::pickColorAt(const QPoint &pos) {
    // Grab the screen region around the cursor safely across Linux display backends
    QScreen *screen = QGuiApplication::screenAt(pos);
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    
    if (screen) {
        // Capture 1x1 pixel mapping from desktop root space
        QPixmap pixmap = screen->grabWindow(0, pos.x(), pos.y(), 1, 1);
        QImage image = pixmap.toImage();
        if (!image.isNull()) {
            QColor sampledColor = image.pixelColor(0, 0);
            updateColorValues(sampledColor);
        }
    }
}

void ColorPickerWindow::updateColorValues(const QColor &color) {
    currentColor = color;
    
    // Update preview background stylesheet
    QString qss = QString("background-color: %1; border: 1px solid #555; border-radius: 4px;").arg(color.name());
    lblColorPreview->setStyleSheet(qss);

    // Update text fields
    txtHexCode->setText(color.name().toUpper());
    txtRgbCode->setText(QString("RGB: %1, %2, %3")
                        .arg(color.red())
                        .arg(color.green())
                        .arg(color.blue()));
}
