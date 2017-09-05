#ifndef COMMONUTILS_H
#define COMMONUTILS_H

#include <functional>
#include <QVector2D>
#include <Box2D/Box2D.h>
#include <QVector4D>

void bvec2qvec(const b2Vec2& vec1, QVector2D& vec2);
void qvec2bvec(const QVector2D& vec1, b2Vec2& vec2);
void qvec2bvec(const QVector4D& vec1, b2Vec2& vec2);

template<typename Iterator,
         typename Function> void for_each_cycled(Iterator ibegin,
                                                 Iterator iend,
                                                 Iterator first,
                                                 int offset,
                                                 Function fn) {
    Iterator i = first;

    while (offset != 0) {

        fn(*i, offset);

        if (offset > 0) {
            if (++i == iend) {
                i = ibegin;
            }
            offset--;
        } else if (offset < 0) {
            if (--i == ibegin) {
                i = iend - 1;
            }
            offset++;
        }
    }

}

template<typename Iterator> Iterator advance_cycled(Iterator ibegin,
                                                    Iterator iend,
                                                    Iterator ielement,
                                                    int offset) {

    unsigned int container_size = std::distance( ibegin, iend );

    if ((unsigned)std::abs(offset) > container_size) {
        offset = offset % container_size;
    }

    Iterator iresult = ielement + offset;
    if (offset > 0 && iresult >= iend) {
        iresult = ibegin + (offset - (container_size - (ielement - ibegin)));
    } else if (offset < 0 && iresult < ibegin) {
        iresult = iend + (offset + (ielement - ibegin));
    }

    return iresult;
}

template<typename Iterator> Iterator inc_cycled(Iterator ibegin,
                                                    Iterator iend,
                                                    Iterator ielement) {
    if (ielement == --iend) {
        return ibegin;
    } else {
        return ++ielement;
    }
}

template<typename Iterator> Iterator dec_cycled(Iterator ibegin,
                                                    Iterator iend,
                                                    Iterator ielement) {
    if (ielement == ibegin) {
        return --iend;
    } else {
        return --ielement;
    }
}

template<typename Iterator> unsigned int distance_cycled(Iterator ibegin,
                                                         Iterator iend,
                                                         Iterator ielement_0,
                                                         Iterator ielement_1,
                                                         bool cw = true) {
    unsigned int result = 0;

    if (ielement_1 < ielement_0) {
        result = std::distance( ielement_0, iend );
        result = result + std::distance( ibegin, ielement_1 );
    } else {
        result = std::distance( ielement_0, ielement_1 );
    }

    if (cw) {
        return result;
    } else {
        return std::distance( ibegin, iend ) - result;
    }
}

template<typename Iterator> bool in_bounds_cycled(Iterator ibegin,
                                                  Iterator iend,
                                                  Iterator ielement,
                                                  Iterator ielement_0,
                                                  Iterator ielement_1) {
    if (ielement_0 > ielement_1) {
        if ((ielement >= ielement_0 && ielement < iend) ||
            (ielement >= ibegin && ielement <= ielement_1)) {
            return true;
        }
    } else {
        if (ielement >= ielement_0 && ielement <= ielement_1) {
            return true;
        }
    }

    return false;
}

#endif // COMMONUTILS_H
