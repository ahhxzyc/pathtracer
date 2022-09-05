#include "Types.h"
#include "Utils.h"
#include "Scene.h"
#include "Camera.h"
#include "Integrator/Whitted.h"
#include "Integrator/Path.h"
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
        // Horizontal layout with 3 buttons
        QHBoxLayout *hLayout = new QHBoxLayout;
        QPushButton *b1 = new QPushButton("Save");
        m_ImageLabel = new QLabel;
        hLayout->addWidget(b1);
        hLayout->addWidget(m_ImageLabel);

        QVBoxLayout *mainLayout = new QVBoxLayout;
        mainLayout->addLayout(hLayout);

        setLayout(mainLayout);
        setWindowTitle("Window");

        connect(b1, SIGNAL(clicked()), this, SLOT(Save()));
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
        m_ImageLabel->setPixmap(QPixmap::fromImage(newImage));
    }

    void UpdateImage(const Color3b* buf, int width, int height, int frameIndex)
    {
        std::lock_guard<std::mutex> lock(renderMutex);
        m_FrameIndex = frameIndex;

        QImage img((const uchar*)buf, width, height, QImage::Format_RGB888);
        img = img.mirrored(false, true);
        if (img.isNull())
        {
            LOG_ERROR("bad image");
        }
        auto pixmap = QPixmap::fromImage(img);
        if (pixmap.isNull())
        {
            LOG_ERROR("bad pix map");
        }
        m_ImageLabel->setPixmap(pixmap);
    }

public slots:
    void Save()
    {
        auto pixmap = m_ImageLabel->pixmap();
        pixmap->save(QString("output_") + QString::number(m_FrameIndex) + QString(".png"));
        LOG_INFO("Saved {}", m_FrameIndex);
    }

private:
    int m_FrameIndex;
    QLabel *m_ImageLabel;
};
#include "main.moc"


int main(int argc, char** argv)
{
    Log::Init();
    QApplication app(argc, argv);

    // Window title
    Window window;
    //window.loadFile("E:/vscodedev/ptracer/build/output_78.bmp");
    window.show();

    // Construct scene
    Scene scene;
    scene.addModel("E:/vscodedev/ptracer/res/cornell-box/cornell-box.obj");
    scene.BuildAggregate();

    // Construct integrator
    auto camera = std::make_shared<Camera>(Size2i{ 1024, 1024 });
    auto integrator = std::make_shared<PathIntegrator>(camera);
    
    auto threadFunc = [&]()
    {
        for (int framei = 1; ; framei ++ )
        {
            integrator->Render(scene);
            auto size = camera->film->Size();
            window.UpdateImage(camera->film->GetColorsUchar(), size.x, size.y, framei);
            if (framei % 200 == 0)
                window.Save();
        }
    };
    std::thread renderThread(threadFunc);

    return app.exec();
}