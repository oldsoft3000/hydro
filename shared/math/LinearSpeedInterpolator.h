#ifndef LINEARSPEEDINTERPOLATOR_H
#define LINEARSPEEDINTERPOLATOR_H

#include <functional>
#include <vector>
#include <iostream>

template<class ValueType> class LinearSpeedInterpolator
{
public:
    typedef std::function<double(ValueType)> ConvFuncT;
private:
    ConvFuncT       _conv;
    std::vector<ValueType>       _values_last;
    std::vector<long long>       _times_last;
private:

public:
    LinearSpeedInterpolator() : _times_last(0)
    {

    }

    bool calcValue(unsigned int idx,
                   long long time,
                   double speed,
                   const ValueType& value0,
                   const ValueType& value1,
                   ValueType& value) {

        if (_conv == nullptr) {
            return false;
        }

        if (idx >= _values_last.size()) {
            _values_last.resize(idx + 1);
            _times_last.resize(idx + 1);
        }

        if (_times_last[idx] == 0) {
            value = _values_last[idx] = value0;
            _times_last[idx] = time;
            return true;
        }

        double time_elapsed = (time - _times_last[idx]) / 1000.0;
        _times_last[idx] = time;

        ValueType value_delta_total = value1 - _values_last[idx];
        double value_delta_total_conv;

        value_delta_total_conv = _conv(value_delta_total);

        if (value_delta_total_conv == 0) {
            value = _values_last[idx] = value1;
            return true;
        }

        double value_delta = speed * time_elapsed;

        double k = value_delta / value_delta_total_conv;

        if (k < 0) {
            k *= -1.0;
        }
        if (k > 1) {
            k = 1;
        }
        if (k == 0) {
            value = _values_last[idx];
            return true;
        }

        value = _values_last[idx] = value_delta_total * k + _values_last[idx];

        return true;
    }

    void setConvFunc(ConvFuncT conv) {
        _conv = conv;
    }

private:

};


#endif // LINEARSPEEDINTERPOLATOR_H
