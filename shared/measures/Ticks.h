#pragma once

#include <string>
#include <chrono>
#include <memory>

//Highest resolution timer
class Ticks
{
public:

    Ticks(const std::string& name=std::string(), bool autostart = true, bool enable_loggin = false);
    ~Ticks();

    long long getTimeNanos() const;

    std::string getMillisecondsStr() const;

    void stop();
    void start();
    void pause();

    const std::string& getName() const { return m_name; }
    void setCorrection(long long correction) { m_correction = correction; }
    static long long calcCorrection();
private:

    std::string     m_name;
    long long       m_start;
    long long       m_ticks;

    bool m_autostart;
    bool m_paused;
    bool m_enable_loggin;

    long long       m_correction;
};

typedef std::shared_ptr<Ticks> TicksPtr;
