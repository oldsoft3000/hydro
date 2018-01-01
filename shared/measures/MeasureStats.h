#pragma once

#include <vector>
#include <map>
#include <unordered_map>
#include "Ticks.h"

class MeasureStats
{
    typedef std::vector<long long> DataVector;
    typedef std::unordered_map<std::string, DataVector> MeasuresType;
	MeasuresType 	m_measures;
	std::string 	m_name;
	double 			m_last_print_time;
	double			m_print_period;
public:
	MeasureStats(const std::string& name, double print_period);
	virtual ~MeasureStats();

	void clear();

    void addStat(const std::string& name, long long tm);

	//called from Game::update
	void onGameUpdate(double game_time);

    std::string toString() const;

    void createTimer(unsigned int idx_timer, const std::string& name);

    void startTimer(unsigned int idx_timer) {
        if (idx_timer < m_timers.size() && m_timers[idx_timer]) {
            m_timers[idx_timer]->start();
        }
    }

    void pauseTimer(unsigned int idx_timer) {
        if (idx_timer < m_timers.size() && m_timers[idx_timer]) {
            m_timers[idx_timer]->pause();
        }
     }

    void stopTimer(unsigned int idx_timer) {
        if (idx_timer < m_timers.size() && m_timers[idx_timer]) {
            m_timers[idx_timer]->stop();
        }
    }

    TicksPtr getTimer(unsigned int idx_timer) {
        if (idx_timer < m_timers.size()) {
            return m_timers[idx_timer];
        } else {
            return nullptr;
        }
    }

    void addStat(unsigned int idx_timer);

    void init();
private:
    void calcCorrection();
	void clear_vectors();
	//returns data vector size
	size_t min_max_avg(const DataVector& data, long long& min, long long& max, long long& avg) const;

    std::vector<TicksPtr>  m_timers;
    long long       m_correction;
};
