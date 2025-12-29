#ifndef IMAGETRANSFORM_H
#define IMAGETRANSFORM_H

#include <QMainWindow>
#include <QMenuBar>
#include <QLabel>
#include <QGroupBox>
#include <QCheckBox>
#include <QPushButton>
#include <QDial>
#include <QSpacerItem>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QImage>
#include <QScrollArea>
#include <QMouseEvent>
#include <QDialog>
#include <QMenu>
#include <QToolBar>
#include <QAction>
#include <QStatusBar>

class ImageLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ImageLabel(QWidget *parent = nullptr) : QLabel(parent)
    {
        setMouseTracking(true);
    }

signals:
    void mouseMoved(const QPoint &pos);

protected:
    void mouseMoveEvent(QMouseEvent *event) override
    {
        emit mouseMoved(event->pos());
        QLabel::mouseMoveEvent(event);
    }
};

class ImageTransform : public QMainWindow
{
    Q_OBJECT

public:
    ImageTransform(QWidget *parent = nullptr);
    ~ImageTransform();

private slots:
    void openImage();
    void saveImage();

    void mirroredImage();
    void rotatedImage();

    void zoomIn();
    void zoomOut();

    void onMouseMoved(const QPoint &pos);

private:
    void updateView(const QImage &img);
    void clearView();
    void showZoomWindow(double factor);
    void updateUiEnabled();

private:
    QWidget *central = nullptr;

    QHBoxLayout *mainLayout = nullptr;
    QVBoxLayout *leftLayout = nullptr;

    QGroupBox *mirrorGroup = nullptr;
    QVBoxLayout *groupLayout = nullptr;
    QCheckBox *hCheckBox = nullptr;
    QCheckBox *vCheckBox = nullptr;
    QPushButton *mirrorButton = nullptr;

    QDial *rotateDial = nullptr;
    QSpacerItem *vSpacer = nullptr;

    QScrollArea *scrollArea = nullptr;
    ImageLabel *inWin = nullptr;

    QMenu *fileMenu = nullptr;
    QMenu *toolMenu = nullptr;

    QAction *openAction = nullptr;
    QAction *saveAction = nullptr;
    QAction *zoomOutAction = nullptr;
    QAction *zoomInAction = nullptr;

    QImage srcImg;
    QImage dstImg;
    QImage viewImg;
};

#endif
