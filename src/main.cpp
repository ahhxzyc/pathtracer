#include "common.h"
#include "scene.h"
#include "camera.h"
#include "integrator.h"
#include "Log.h"

#include <QApplication>
#include <QLabel>
#include <QWidget>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QImageReader>
#include <QMessageBox>
#include <QGroupBox>
#include <QDir>

#include <iostream>
#include <thread>

bool renderComplete = false;
std::mutex renderMutex;

class Window : public QWidget
{
    Q_OBJECT

public:
    Window()
    {
        auto panelBox = new QGroupBox("Panel");
        auto panelLayout = new QVBoxLayout;
        m_pbSave = new QPushButton("Save");
        m_lbFrameCount = new QLabel;
        m_lbImage = new QLabel;
        panelLayout->addWidget(m_pbSave);
        panelLayout->addWidget(m_lbFrameCount);
        panelBox->setLayout(panelLayout);

        auto mainLayout = new QHBoxLayout;
        mainLayout->addWidget(panelBox);
        mainLayout->addWidget(m_lbImage);

        setLayout(mainLayout);
        setWindowTitle("Window");

        connect(m_pbSave, SIGNAL(clicked()), this, SLOT(Save()));
    }

    bool loadFile(const QString &fileName)
    {
        QImageReader reader(fileName);
        reader.setAutoTransform(true);
        const QImage newImage = reader.read();
        if (newImage.isNull())
        {
            QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                tr("Cannot load %1: %2")
                .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
            return false;
        }
        m_lbImage->setPixmap(QPixmap::fromImage(newImage));
    }

    void UpdateImage(const Color3b* buf, int width, int height, int frameIndex)
    {
        std::lock_guard<std::mutex> lock(renderMutex);
        m_FrameIndex = frameIndex;

        m_Image = QImage((const uchar *)buf, width, height, QImage::Format_RGB888)
            .mirrored(false, true);
        if (m_Image.isNull())
        {
            LOG_ERROR("bad image");
        }
        auto pixmap = QPixmap::fromImage(m_Image.scaledToHeight(800));
        if (pixmap.isNull())
        {
            LOG_ERROR("bad pix map");
        }
        m_lbImage->setPixmap(pixmap);
        m_lbFrameCount->setText("Frame Index : " + QString::number(frameIndex));
    }

public slots:
    void Save()
    {
        auto pixmap = QPixmap::fromImage(m_Image);
        pixmap.save(QString("output_") + QString::number(m_FrameIndex) + QString(".png"));
        LOG_INFO("Saved {}", m_FrameIndex);
    }

private:
    int m_FrameIndex;
    QLabel *m_lbImage;
    QLabel *m_lbFrameCount;
    QPushButton *m_pbSave;
    QImage m_Image;
};
#include "main.moc"


int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    Log::Init();

    Window window;
    window.show();

    Scene scene;
    scene.parse("E:/vscodedev/ptracer/res/veach-mis", "veach-mis");
    //scene.parse("E:/vscodedev/ptracer/res/cornell-box", "cornell-box");
    //scene.parse("E:/vscodedev/ptracer/res/cornell-glass", "test");
    //scene.parse("E:/vscodedev/ptracer/res/bedroom", "bedroom");
    scene.build_accel();
    
    auto integrator = std::make_shared<PathIntegrator>();
    //auto integrator = std::make_shared<WhiteFurnaceIntegrator>(Color3f(0.4));

    auto threadFunc = [&]()
    {
        for (int framei = 1; ; framei ++ )
        {
            integrator->render(scene);
            auto size = scene.camera->film->size;
            window.UpdateImage(scene.camera->film->framebuffer_ub(), size.x, size.y, framei);
            if (framei - (framei & -framei) == 0)
                window.Save();
        }
    };
    std::thread renderThread(threadFunc);

    return app.exec();
}