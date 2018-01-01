/*
 * MeasureStats.cpp
 *
 *  Created on: 17 ��� 2015 �.
 *      Author: User
 */

#include "MeasureStats.h"
#include "Game.h"
#include <cassert>
#include <sstream>
#include <limits>
#include <QDebug>

MeasureStats::MeasureStats(const std::string& name, double print_period)
	:m_name(name),
	 m_print_period(print_period),
     m_last_print_time(0),
     m_correction(0)
{
}

MeasureStats::~MeasureStats(){
}

void MeasureStats::init() {
    calcCorrection();
}

void MeasureStats::addStat(const std::string& name, long long tm)
{
    m_measures[name].push_back(tm);
}

size_t MeasureStats::min_max_avg(const DataVector& data, long long& min_out, long long& max_out, long long& avg_out) const
{
	if(data.empty())
		return 0;
	long long min = std::numeric_limits<long long>::max();
	long long max = 0;
	long long sum = 0;
	for(auto it = data.begin();it != data.end();++it)
	{
		long long val = *it;
		if(val > max)
			max = val;
		if(val < min)
			min = val;
		sum += val;
	}
	min_out = min;
	max_out = max;
	avg_out = sum/data.size();
	return data.size();
}

static std::string nanosec2string(long long nano)
{
    char str[256];
    sprintf(str, "%.2f ms", nano / 1000000.0);
	return str;
}

std::string MeasureStats::toString() const
{
	if(m_measures.empty())
		return "";
	std::stringstream ss;
	ss << m_name << " measures\n";
	long long avg_total = 0;
	//print to string all measure indices with min, max, avg
    std::map<std::string, DataVector> measures_sorted;

    for(auto it = m_measures.begin();it != m_measures.end();++it) {
        measures_sorted.insert(std::make_pair(it->first, it->second));
    }

    for(auto it = measures_sorted.begin();it != measures_sorted.end();++it)
	{
        const std::string& measure_name = it->first;
        const DataVector& data = it->second;
		long long min, max, avg;
		if(min_max_avg(data, min, max, avg) > 0)
		{
			avg_total += avg;
			ss << "[MEASURES] " << measure_name << ": min=" << nanosec2string(min) << ", max=" << nanosec2string(max) << ", avg=" << nanosec2string(avg) << "\n";
		}else{
			ss << "[MEASURES] " << measure_name << ": empty\n";
		}
	}
    ss << "[MEASURES] " << "Avg total:" << nanosec2string(avg_total) << "\n";
	return ss.str();
}

void MeasureStats::clear()
{
	MeasuresType data;
	m_measures.swap(data);
}

void MeasureStats::clear_vectors()
{
	for(auto it = m_measures.begin();it != m_measures.end();++it)
	{
        it->second.clear();
	}
}

void MeasureStats::onGameUpdate(double game_time)
{
	if(game_time > m_last_print_time + m_print_period)
	{
		m_last_print_time = game_time;

        qInfo() << toString().c_str();
		clear_vectors();
	}
}

void MeasureStats::createTimer(unsigned int idx_timer, const std::string& name) {
    if (idx_timer >= m_timers.size()) {
        m_timers.resize(idx_timer + 1);
    }
    m_timers[idx_timer] = std::make_shared<Ticks>(name, false);
    m_timers[idx_timer]->setCorrection(m_correction);

}

void MeasureStats::addStat(unsigned int idx_timer) {
    if (idx_timer < m_timers.size()) {
        Ticks* timer = m_timers[idx_timer].get();
        if (timer) {
            addStat(timer->getName(), timer->getTimeNanos());
            timer->stop();
        }
    }
}

void MeasureStats::calcCorrection() {
#ifdef MEASURES
    createTimer(0, "");
    TicksPtr correction_timer = getTimer(0);

    if (correction_timer) {
        const int n = 10;

        long long sum = 0;

        for (int i = 0; i != n; ++i) {
            Game::instance().getMeasures()->startTimer(0);
            Game::instance().getMeasures()->pauseTimer(0);

            long long correcton = correction_timer->getTimeNanos();
            sum += correcton;
            stopTimer(0);
        }
        m_correction = sum / n;
    }
#endif
}
