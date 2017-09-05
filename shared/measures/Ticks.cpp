#include "Ticks.h"
#include <QDebug>
#include "Game.h"
#include <limits>

using namespace std::chrono;

Ticks::Ticks(const std::string& name, bool autostart, bool enable_loggin) :
	m_name(name),
    m_ticks(0),
    m_autostart(autostart),
    m_enable_loggin(enable_loggin),
    m_correction(0)
{
    if (m_autostart) {
        m_start = duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
        m_paused = false;
    } else {
        m_start = 0;
        m_paused = true;
    }
}

long long Ticks::getTimeNanos() const
{
    if (m_start) {
        return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count() - m_start + m_ticks;
    }  else if (m_paused) {
        return m_ticks;
    } else {
        return 0;
    }
}

std::string Ticks::getMillisecondsStr() const
{
	char str[32];
    sprintf(str, "%.2f ms", getTimeNanos()  / 1000000.0 );
	return str;
}

void Ticks::stop() {
    if(m_enable_loggin && m_start)  {
        qDebug() << m_name.c_str() << ":" << getMillisecondsStr().c_str();
    }
    m_start = 0;
    m_ticks = 0;
}

void Ticks::start() {
    m_paused = false;
    m_start = duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
}

void Ticks::pause() {
    long long ticks = duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();

    if (m_start) {
        long long elapsed_ticks = ticks - m_start;
        if (elapsed_ticks > m_correction) {
            m_ticks = m_ticks + elapsed_ticks - m_correction;
        }
    }
    m_start = 0;
    m_paused = true;
}

long long Ticks::calcCorrection() {
    Ticks ticks("", false);

    long long min_correcton = std::numeric_limits<long long>::max();

    for (int i = 0; i != 10; ++i) {
        ticks.start();
        ticks.pause();

        long long correcton = ticks.m_ticks;

        if (correcton < min_correcton) {
            min_correcton = correcton;
        }

        ticks.stop();
    }
    return min_correcton;
}

Ticks::~Ticks() {
    if (m_autostart) {
        stop();
    }
}
