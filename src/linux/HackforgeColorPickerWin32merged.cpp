#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QColor>
#include <QScreen>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QCursor>
#include <QPixmap>
#include <QImage>

class HackforgeColorPickerApp : public QWidget {
    Q_OBJECT

public:
    HackforgeColorPickerApp(QWidget *parent = nullptr) : QWidget(parent), capturing(false) {
        setWindowTitle("Hackforge Paint - Linux Color Picker & Adjustment");
        setFixedSize(320, 290);

        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        // Color Preview Box
        lblColorPreview = new QLabel(this);
        lblColorPreview->setFixedHeight(60);
        lblColorPreview->setAutoFillBackground(true);
        currentBaseColor = QColor(41, 128, 185);
        updateDisplayColor(currentBaseColor);
        mainLayout->addWidget(lblColorPreview);

        // Eyedropper Button
        btnEyedropper = new QPushButton("Pick Color from Screen", this);
        mainLayout->addWidget(btnEyedropper);

        // Color Adjustment Sliders (Brightness & Saturation)
        mainLayout->addWidget(new QLabel("Brightness Adjustment:", this));
        sliderBrightness = new QSlider(Qt::Horizontal, this);
        sliderBrightness->setRange(-100, 100);
        sliderBrightness->setValue(0);
        mainLayout->addWidget(sliderBrightness);

        mainLayout->addWidget(new QLabel("Saturation Factor:", this));
        sliderSaturation = new QSlider(Qt::Horizontal, this);
        sliderSaturation->setRange(0, 200); // 0% to 200%
        sliderSaturation->setValue(100);    // 100% normal
        mainLayout->addWidget(sliderSaturation);

        // Hex and RGB readouts
        QHBoxLayout *textLayout = new QHBoxLayout();
        txtHexCode = new QLineEdit(this);
        txtHexCode->setReadOnly(true);
        txtRgbCode = new QLineEdit(this);
        txtRgbCode->setReadOnly(true);
        textLayout->addWidget(txtHexCode);
        textLayout->addWidget(txtRgbCode);
        mainLayout->addLayout(textLayout);

        // Signals & Slots
        connect(btnEyedropper, &QPushButton::clicked, this, &HackforgeColorPickerApp::activateEyedropper);
        connect(sliderBrightness, &QSlider::valueChanged, this, &HackforgeColorPickerApp::applyAdjustments);
        connect(sliderSaturation, &QSlider::valueChanged, this, &HackforgeColorPickerApp::applyAdjustments);
    }

private slots:
    void activateEyedropper() {
        capturing = true;
        setCursor(Qt::CrossCursor);
        setWindowOpacity(0.4); // Fade window slightly so you can see behind it
    }

    void applyAdjustments() {
        int brightnessOffset = sliderBrightness->value();
        qreal satFactor = sliderSaturation->value() / 100.0;

        QColor adjusted = currentBaseColor;
        
        // Adjust Brightness
        int r = qBound(0, adjusted.red() + brightnessOffset, 255);
        int g = qBound(0, adjusted.green() + brightnessOffset, 255);
        int b = qBound(0, adjusted.blue() + brightnessOffset, 255);
        adjusted.setRgb(r, g, b);

        // Adjust Saturation
        qreal h, s, l, a;
        adjusted.getHslF(&h, &s, &l, &a);
        s = qBound(0.0, s * satFactor, 1.0);
        adjusted.setHslF(h, s, l, a);

        updateDisplayColor(adjusted, false);
    }

protected:
    void mousePressEvent(QMouseEvent *event) override {
        if (capturing && event->button() == Qt::LeftButton) {
            QPoint pos = QCursor::pos();
            QScreen *screen = QGuiApplication::screenAt(pos);
            if (!screen) {
                screen = QGuiApplication::primaryScreen();
            }

            if (screen) {
                // Grab 1x1 pixel from global desktop root coordinates
                QPixmap pixmap = screen->grabWindow(0, pos.x(), pos.y(), 1, 1);
                QImage img = pixmap.toImage();
                if (!img.isNull()) {
                    currentBaseColor = img.pixelColor(0, 0);
                    sliderBrightness->setValue(0);
                    sliderSaturation->setValue(100);
                    updateDisplayColor(currentBaseColor);
                }
            }
            capturing = false;
            setCursor(Qt::ArrowCursor);
            setWindowOpacity(1.0);
            event->accept();
        } else {
            QWidget::mousePressEvent(event);
        }
    }

private:
    QColor currentBaseColor;
    QPushButton *btnEyedropper;
    QLabel *lblColorPreview;
    QSlider *sliderBrightness;
    QSlider *sliderSaturation;
    QLineEdit *txtHexCode;
    QLineEdit *txtRgbCode;
    bool capturing;

    void updateDisplayColor(const QColor &color, bool updateBase = true) {
        if (updateBase) {
            currentBaseColor = color;
        }
        QString qss = QString("background-color: %1; border: 1px solid #444; border-radius: 4px;").arg(color.name());
        lblColorPreview->setStyleSheet(qss);
        txtHexCode->setText(color.name().toUpper());
        txtRgbCode->setText(QString("RGB: %1, %2, %3").arg(color.red()).arg(color.green()).arg(color.blue()));
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    HackforgeColorPickerApp pickerWindow;
    pickerWindow.show();
    return app.exec();
}

#include "HackforgeColorPickerApp.moc"
