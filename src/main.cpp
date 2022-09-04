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
        QPushButton *b1 = new QPushButton("A");
        QPushButton *b2 = new QPushButton("B");
        QPushButton *b3 = new QPushButton("C");
        m_ImageLabel = new QLabel;
        hLayout->addWidget(b1);
        hLayout->addWidget(b2);
        hLayout->addWidget(b3);
        hLayout->addWidget(m_ImageLabel);

        QVBoxLayout *mainLayout = new QVBoxLayout;
        mainLayout->addLayout(hLayout);

        setLayout(mainLayout);
        setWindowTitle("Window");
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

    void UpdateImage(const Color3b* buf, int width, int height)
    {
        std::lock_guard<std::mutex> lock(renderMutex);

        QImage img((const uchar*)buf, width, height, QImage::Format_RGB888);
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

private:
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
    auto camera = std::make_shared<Camera>(Size2i{ 100, 100 });
    auto integrator = std::make_shared<PathIntegrator>(camera);
    
    auto threadFunc = [&]()
    {
        integrator->Render(scene);
        auto size = camera->film->Size();
        window.UpdateImage(camera->film->GetColorsUchar(), size.x, size.y);
    };
    std::thread renderThread(threadFunc);

    return app.exec();
}