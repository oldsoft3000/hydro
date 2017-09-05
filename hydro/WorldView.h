#pragma once

#include <QLabel>
#include <QSlider>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QOpenGLShaderProgram>
#include <Box2D/Box2D.h>
#include <QElapsedTimer>

class GeometryEngine;
class SceneGeometry;
class QPushButton;
class FluidGeometry;
typedef std::shared_ptr<FluidGeometry> FluidGeometryPtr;

class WorldView : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit WorldView(QWidget *parent = 0);
    ~WorldView();

    void initialize();
    b2Vec2 toWorldPoint( const QPoint& point );
protected:
    virtual void initializeGL();
    virtual void paintGL();
    virtual void resizeGL(int w, int h);

    virtual void mousePressEvent  (QMouseEvent* pe);
    virtual void mouseReleaseEvent(QMouseEvent* pe);
    virtual void mouseMoveEvent   (QMouseEvent* pe);
    virtual void keyPressEvent(QKeyEvent *event);

    void paintCentripetalAcceleration(QPainter& p);
    bool eventFilter(QObject* object, QEvent* event);
protected:
    QPushButton* createButton(const QString& str);
    QSlider* createSlider(const QString& str);
    void paintNative(QPainter& p);
private:
    QTransform              _transform;
    QOpenGLShaderProgram    _program;
    QOpenGLShaderProgram    _program_fluid;

    std::shared_ptr<GeometryEngine>     _geometry_engine;
    std::shared_ptr<SceneGeometry>      _scene_geometry;
    std::vector<FluidGeometryPtr>       _fluid_geometry;

    QLabel*                 _label_fps;
    QElapsedTimer           _timer_fps;
    int                     _count_frame;
    qint64                  _time_fps;

    QMatrix4x4              _view;
    QMatrix4x4              _projection;
    QMatrix4x4              _iso;

    std::vector<QVector2D>  _buffer_points_bezier;
    std::vector<QVector2D>  _buffer_points;

public slots:
    void slotButtonClicked();
    void setSliderValue( int value );
};

