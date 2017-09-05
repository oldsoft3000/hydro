#include "WorldView.h"
#include <QtOpenGL>

#include <WorldController.h>

#include "Game.h"

#include "FixturePainter.h"
#include "Atom.h"
#include "GeometryEngine.h"
#include "Fluid.h"
#include "FluidGeometry.h"
#include "SceneGeometry.h"

#include "ElementUtils.h"
#include "GlobalParams.h"
#include "CommonUtils.h"
#ifdef MEASURES
#include "Ticks.h"
#endif

WorldView::WorldView(QWidget *parent) : QOpenGLWidget(parent),
                                        _count_frame(0),
                                        _time_fps(0)
{
    QHBoxLayout* layout = new QHBoxLayout;

    layout->addWidget( createButton("1"), 0, Qt::AlignTop );
    layout->addWidget( createButton("2"), 0, Qt::AlignTop );
    layout->addWidget( createButton("3"), 0, Qt::AlignTop );
    layout->addWidget( createButton("4"), 0, Qt::AlignTop );

    QVBoxLayout* layout_slider = new QVBoxLayout;

    layout->addLayout( layout_slider, 0 );

    layout_slider->addWidget( createSlider("slider_x"), 0, Qt::AlignTop );
    layout_slider->addWidget( createSlider("slider_y"), 0, Qt::AlignTop );
    layout_slider->addWidget( createSlider("slider_z"), 0, Qt::AlignTop );
    QSlider* slider_s = createSlider("slider_s");
    slider_s->setMinimum( 1 );
    slider_s->setMaximum( 5 );
    layout_slider->addWidget( slider_s, 0, Qt::AlignTop );
    layout_slider->addStretch();

    setLayout(layout);

    _label_fps = new QLabel("FPS:     ", this);
    _label_fps->move(10, 45);
    _label_fps->show();

    QPalette palette = _label_fps->palette();
    palette.setColor(_label_fps->foregroundRole(), Qt::blue);
    _label_fps->setPalette(palette);

    _timer_fps.start();

    //setAttribute(Qt::WA_NoBackground);
    //setAttribute(Qt::WA_NoSystemBackground);

    //installEventFilter(this);

    _buffer_points_bezier.reserve(255);
    _buffer_points.reserve(255);
}
WorldView::~WorldView() {
}

void WorldView::initializeGL()
{
    initializeOpenGLFunctions();
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    _geometry_engine = std::make_shared<GeometryEngine>();
    _scene_geometry = std::make_shared<SceneGeometry>(50, 50);

    // Compile vertex shader
    if (!_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vshader.glsl")) {
        qDebug() << _program.log();
        close();
    }

    // Compile fragment shader
    if (!_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fshader.glsl")) {
        qDebug() << _program.log();
        close();
    }

    // Link shader pipeline
    if (!_program.link()) {
        qDebug() << _program.log();
        close();
    }

    // Compile vertex shader
    if (!_program_fluid.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vshader_fluid.glsl")) {
        qDebug() << _program_fluid.log();
        close();
    }

    // Compile fragment shader
    if (!_program_fluid.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fshader_fluid.glsl")) {
        qDebug() << _program_fluid.log();
        close();
    }

    // Link shader pipeline
    if (!_program_fluid.link()) {
        qDebug() << _program_fluid.log();
        close();
    }
}

QPushButton* WorldView::createButton(const QString& str)
{
    QPushButton* pcmd = new QPushButton(str);

    pcmd->setMinimumSize(40, 40);
    connect(pcmd, SIGNAL(clicked()), SLOT(slotButtonClicked()));
    return pcmd;
}

QSlider* WorldView::createSlider(const QString& str)
{
    QSlider* slider = new QSlider(Qt::Horizontal);

    slider->setObjectName( str );
    //slider->setAccessibleName( str );
    slider->setMaximumSize( 160, 40 );
    slider->setFocusPolicy( Qt::StrongFocus );
    slider->setTickPosition( QSlider::TicksBothSides );
    slider->setTickInterval( 10 );
    slider->setSingleStep( 1 );
    slider->setMaximum(360);
    connect( slider, SIGNAL(valueChanged(int)),  SLOT(setSliderValue(int)) );
    return slider;
}

void WorldView::resizeGL(int w, int h)
{
    _projection.setToIdentity();
    _view.setToIdentity();
    _iso.setToIdentity();

    _projection.ortho(0, w, h, 0, -1000, 1000);

    QSlider* slider_x = findChild<QSlider*>( "slider_x" );
    QSlider* slider_y = findChild<QSlider*>( "slider_y" );
    QSlider* slider_z = findChild<QSlider*>( "slider_z" );
    QSlider* slider_s = findChild<QSlider*>( "slider_s" );

    float value_slider_x = 0;
    float value_slider_y = 0;
    float value_slider_z = 0;
    float value_slider_s = 1;

    if ( slider_x ) {
        value_slider_x = slider_x->value();
    }
    if ( slider_y ) {
        value_slider_y = slider_y->value();
    }
    if ( slider_z ) {
        value_slider_z = slider_z->value();
    }
    if ( slider_s ) {
        value_slider_s = slider_s->value();
    }

    QVector3D z(0, 0, 1);
    QVector3D x(1, 0, 0);
    QVector3D y(0, 1, 0);

    _iso.rotate( value_slider_x, x );
    _iso.rotate( value_slider_z, z);
    //_iso.translate(-width() * 0.3, height() * 0.7, 0);

    _iso.scale(WORLD_SCALE * value_slider_s);
    _iso.translate(-width() * 0.5 / WORLD_SCALE + width() * 0.5 / ( WORLD_SCALE * value_slider_s ),
                   -height() * 0.5 / WORLD_SCALE + height() * 0.5 / ( WORLD_SCALE * value_slider_s ) , 0);


    _view = _view * _iso;
}

void WorldView::paintGL() {
    _buffer_points.clear();
    _buffer_points_bezier.clear();

    _count_frame++;

    if (_timer_fps.elapsed() - _time_fps > 1000) {
        qint64 fps = _count_frame * 1000.0 / ( _timer_fps.elapsed() - _time_fps );

        _label_fps->setText("FPS: " + QString::number(fps));

        _count_frame = 0;
        _time_fps = _timer_fps.elapsed();
    }

    if (!Game::instance().getWorldController()) {
        return;
    }

    glShadeModel(GL_FLAT);

    QPainter p;
    p.begin(this);

    QPen pen;


    _program.bind();
    _program.setUniformValue("mvp_matrix", _projection * _view);
    _program_fluid.bind();
    _program_fluid.setUniformValue("mvp_matrix", _projection * _view);
    _program_fluid.setUniformValue("u_camera", QVector3D(0, 100, 0));
    _program_fluid.setUniformValue("u_lightPosition", QVector3D(0, 100.0, 20.0));

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Fluids& fluids = Game::instance().getWorldController()->getFluids();
    for (Fluids::iterator ifluid = fluids.begin(); ifluid != fluids.end(); ++ifluid) {
        for (auto fluid_vertex : (*ifluid)->getBoundaryVertecies()) {
            _buffer_points.push_back(fluid_vertex.vertex);
        }
    }

    if (!_buffer_points.empty()) {
        START_TIMER(MEASURE_BEZIER_POINTS);
        ClosedBezierSpline::GetBezierPoints(_buffer_points, _buffer_points_bezier);
        PAUSE_TIMER(MEASURE_BEZIER_POINTS);
    }

    if (_scene_geometry) {
        _scene_geometry->drawScene(&_program);
    }

    Elements& elements = Game::instance().getWorldController()->getElements();
    for ( Elements::iterator i = elements.begin(); i != elements.end(); ++i ) {
        b2Filter filter = (*i)->getBody()->GetFixtureList()->GetFilterData();
        QColor color = Qt::white;
        if (filter.maskBits == MASK_ATOMS) {
            AtomPtr atom = std::static_pointer_cast<Atom>(*i);
            color = atom->getColor();
        } else {
            color = Qt::lightGray;
        }

        color = Qt::darkBlue;

        if (filter.categoryBits & CATEGORY_ATOM) {
           //FixturePainter::paintFixture(&p, (*i)->getBody(), color);
        }
    }

    for (Fluids::iterator ifluid = fluids.begin(); ifluid != fluids.end(); ++ifluid) {
        FluidGeometryPtr fg = (*ifluid)->getGeometry();
        if (!fg) {
            fg = std::make_shared<FluidGeometry>();

            (*ifluid)->setGeometry(fg);

            _fluid_geometry.push_back(fg);
        }

        if (fg) {
            if (fg->isInited() == false) {
                fg->init(_buffer_points_bezier.size());
            }

            b2Vec2 fluid_center = (*ifluid)->getKernel()->getBody()->GetWorldCenter();
            b2Vec2 velocity = (*ifluid)->getKernel()->getBody()->GetLinearVelocity();

            QVector2D qfluid_center;
            QVector2D qvelocity;

            bvec2qvec(fluid_center, qfluid_center);
            bvec2qvec(velocity, qvelocity);

            fg->drawFluid(&_program_fluid,
                         _buffer_points_bezier);
        }
    }
}

void WorldView::initialize() {
    //_transform.scale(1.0f, -1.0f);
    //_transform.translate(0.0f, -height() / WORLD_SCALE);
}

void WorldView::slotButtonClicked()
{
    QString str = ((QPushButton*)sender())->text();

    if (str == "1") {
        /*for ( int i = 0; i != 100; ++i ) {
            int dx = width() / 2 - qrand() % width();
            int dy = qrand() % 20;
            createBall(b2Vec2(width() / 2 + dx, height() - dy), 3.0f);
        }*/
        Game::instance().getWorldController()->createMatter();
    } else if (str == "2") {
        Game::instance().getWorldController()->createFluid();
    } else if (str == "3") {
        bool is_updates_enabled = Game::instance().getWorldController()->isUpdatesEnabled();
        Game::instance().getWorldController()->enableUpdates(!is_updates_enabled);
    } else if (str == "4") {
        //_world_controller->clear();
    }
}

void WorldView::setSliderValue( int value ) {
    QString str = ((QSlider*)sender())->objectName();

    if (str == "slider_x") {

    } else if (str == "slider_y") {

    } else if (str == "slider_z") {

    }

    resizeGL( width(), height() );
}

void WorldView::mousePressEvent(QMouseEvent* pe) {
    b2Vec2 pos = toWorldPoint(pe->pos());
    Game::instance().getWorldController()->mousePress(pos);
}

void WorldView::keyPressEvent(QKeyEvent *event) {
    if (event->key()==Qt::Key_W) {
        bool is_updates_enabled = Game::instance().getWorldController()->isUpdatesEnabled();
        Game::instance().getWorldController()->enableUpdates(!is_updates_enabled);
    }
}

void WorldView::mouseReleaseEvent(QMouseEvent* pe) {
    b2Vec2 pos = toWorldPoint(pe->pos());
    Game::instance().getWorldController()->mouseRelease(pos);
}

void WorldView::mouseMoveEvent(QMouseEvent* pe) {
    b2Vec2 pos = toWorldPoint(pe->pos());
    Game::instance().getWorldController()->mouseMove(pos);
}

b2Vec2 WorldView::toWorldPoint(const QPoint& point) {
    QVector4D vz(0, 0, 1, 0);
    QVector4D qworldPoint;
    b2Vec2 worldPoint;

    vz = _iso * vz;

    float z = (-vz.x() * (point.x() -_iso(0, 3)) - vz.y() * (point.y() - _iso(1, 3)) + vz.z() * _iso(2, 3)) / vz.z();

    qworldPoint.setW(1);
    qworldPoint.setX(point.x());
    qworldPoint.setY(point.y());
    qworldPoint.setZ(z);

    qworldPoint = _iso.inverted() * qworldPoint;

    //qworldPoint.setX( qworldPoint.x() / WORLD_SCALE );
    //qworldPoint.setY( qworldPoint.y() / WORLD_SCALE );

    qvec2bvec(qworldPoint, worldPoint);

    return worldPoint;
}

bool WorldView::eventFilter(QObject* object, QEvent* event) {
  if (event->type() == QEvent::Paint) { return true; }
  return false;
}
