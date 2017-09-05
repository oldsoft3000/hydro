#ifndef MATHUTILS_H
#define MATHUTILS_H

#include <vector>
#include <deque>
#include <stdexcept>
#include <QVector2D>
#include <QDebug>
#include <QElapsedTimer>
#include <cmath>



class Tridiagonal;

/// <summary>
/// Solves the cyclic set of linear equations.
/// </summary>
/// <remarks>
/// The cyclic set of equations have the form
/// ---------------------------
/// b0 c0  0 · · · · · · ß
///	a1 b1 c1 · · · · · · ·
///  · · · · · · · · · · ·
///  · · · a[n-2] b[n-2] c[n-2]
/// a  · · · · 0  a[n-1] b[n-1]
/// ---------------------------
/// This is a tridiagonal system, except for the matrix elements
/// a and ß in the corners.
/// </remarks>
///

/// Tridiagonal system solution.
class Tridiagonal {
public:

    static void Solve(const std::vector<double>& a,
                               const std::vector<double>& b,
                               const std::vector<double>& c,
                               const std::vector<double>& rhs,
                               std::vector<double>& result)
    {
        result.clear();
        // a, b, c and rhs vectors must have the same size.
        if (a.size() != b.size() ||
            c.size() != b.size() ||
            rhs.size() != b.size())
            throw std::runtime_error("Diagonal and rhs vectors must have the same size.");

        if (b[0] == 0.0)
            throw std::runtime_error("Singular matrix.");
            // If this happens then you should rewrite your equations as a set of
            // order N - 1, with u2 trivially eliminated.

        std::vector<double> gam;

        ulong n = rhs.size();

        result.resize(n);
        gam.resize(n);

        // One vector of workspace, gam is needed.

        double bet = b[0];
        result[0] = rhs[0] / bet;

        for (ulong j = 1; j < n; j++) { // Decomposition and forward substitution.
            gam[j] = c[j-1] / bet;
            bet = b[j] - a[j] * gam[j];
            if (bet == 0.0)
                throw std::runtime_error("Singular matrix.");
                // Algorithm fails.

            result[j] = (rhs[j] - a[j] * result[j - 1]) / bet;
        }

        for (ulong j = 1;j < n;j++)
            result[n - j - 1] -= gam[n - j] * result[n - j]; // Backsubstitution.
    }
};




class Cyclic {
    /// <summary>
    /// Solves the cyclic set of linear equations.
    /// </summary>
    /// <remarks>
    /// All vectors have size of n although some elements are not used.
    /// The input is not modified.
    /// </remarks>
    /// <param name="a">Lower diagonal vector of size n; a[0] not used.</param>
    /// <param name="b">Main diagonal vector of size n.</param>
    /// <param name="c">Upper diagonal vector of size n; c[n-1] not used.</param>
    /// <param name="alpha">Bottom-left corner value.</param>
    /// <param name="beta">Top-right corner value.</param>
    /// <param name="rhs">Right hand side vector.</param>
    /// <returns>The solution vector of size n.</returns>
public:

    static void Solve(const std::vector<double>& a,
                        const std::vector<double>& b,
                        const std::vector<double>& c,
                        const std::vector<double>& rhs,
                        double alpha,
                        double beta,
                      std::vector<double>& result) {
        result.clear();
        // a, b, c and rhs vectors must have the same size.
        if (a.size() != b.size() ||
            c.size() != b.size() ||
            rhs.size() != b.size())
            throw std::runtime_error("Diagonal and rhs vectors must have the same size.");

        int n = b.size();

        if (n <= 2)
            throw std::runtime_error("n too small in Cyclic; must be greater than 2.");

        double gamma = -b[0]; // Avoid subtraction error in forming bb[0].
        // Set up the diagonal of the modified tridiagonal system.
        std::vector<double> bb(n);
        bb[0] = b[0] - gamma;
        bb[n-1] = b[n - 1] - alpha * beta / gamma;
        for (int i = 1; i < n - 1; ++i)
            bb[i] = b[i];
        // Solve A · x = rhs.
        std::vector<double> solution;

        Tridiagonal::Solve(a, bb, c, rhs, solution);

        result.resize(n);

        for (int k = 0; k < n; ++k)
            result[k] = solution[k];

        // Set up the vector u.
        std::vector<double> u(n);
        u[0] = gamma;
        u[n-1] = alpha;
        for (int i = 1; i < n - 1; ++i)
            u[i] = 0.0;
        // Solve A · z = u.
        Tridiagonal::Solve(a, bb, c, u, solution);
        std::vector<double> z(n);
        for (int k = 0; k < n; ++k)
            z[k] = solution[k];

        // Form v · x/(1 + v · z).
        double fact = (result[0] + beta * result[n - 1] / gamma)
            / (1.0 + z[0] + beta * z[n - 1] / gamma);

        // Now get the solution vector x.
        for (int i = 0; i < n; ++i)
            result[i] -= fact * z[i];
    }
};


/// <summary>
/// Closed Bezier Spline Control Points calculation.
/// </summary>
class ClosedBezierSpline
{
public:
    /// <summary>
    /// Get Closed Bezier Spline Control Points.
    /// </summary>
    /// <param name="knots">Input Knot Bezier spline points.</param>
    /// <param name="firstControlPoints">
    /// Output First Control points array of the same
    /// length as the <paramref name="knots"> array.</param>
    /// <param name="secondControlPoints">
    /// Output Second Control points array of the same
    /// length as the <paramref name="knots"> array.</param>
    static void GetCurveControlPoints(const std::vector<QVector2D>& knots,
                                      std::vector<QVector2D>& firstControlPoints,
                                      std::vector<QVector2D>& secondControlPoints)
    {
        firstControlPoints.clear();
        secondControlPoints.clear();

        int n = knots.size();
        if (n <= 2)
        { // There should be at least 3 knots to draw closed curve.
            firstControlPoints.resize(1);
            secondControlPoints.resize(1);
            return;
        }

        // Calculate first Bezier control points

        // The matrix.
        std::vector<double> a(n);
        std::vector<double> b(n);
        std::vector<double> c(n);

        for (int i = 0; i < n; ++i)
        {
            a[i] = 1;
            b[i] = 4;
            c[i] = 1;
        }

        // Right hand side vector for points X coordinates.
        std::vector<double> rhs(n);
        for (int i = 0; i < n; ++i)
        {
            int j = (i == n - 1) ? 0 : i + 1;
            rhs[i] = 4 * knots[i].x() + 2 * knots[j].x();
        }
        // Solve the system for X.
        std::vector<double> x;
        Cyclic::Solve(a, b, c, rhs, 1, 1, x);

        // Right hand side vector for points Y coordinates.
        for (int i = 0; i < n; ++i)
        {
            int j = (i == n - 1) ? 0 : i + 1;
            rhs[i] = 4 * knots[i].y() + 2 * knots[j].y();
        }
        // Solve the system for Y.
        std::vector<double> y;
        Cyclic::Solve(a, b, c, rhs, 1, 1, y);

        // Fill output arrays.
        firstControlPoints.resize(n);
        secondControlPoints.resize(n);

        for (int i = 0; i < n; ++i) {
            // First control point.
            firstControlPoints[i] = QVector2D(x[i], y[i]);
            // Second control point.
            secondControlPoints[i] = QVector2D(2 * knots[i].x() - x[i], 2 * knots[i].y() - y[i]);
        }
    }

    static void appendBezierSegment(const QVector2D& knot_1,
                                const QVector2D& knot_2,
                                const QVector2D& control_1,
                                const QVector2D& control_2,
                                std::vector<QVector2D>& bezier_points) {
        const float reso = 0.2;


        for (float t = 0; t <= 1; t += reso) {
            QVector2D bezier = std::pow(1 - t, 3) * knot_1 + 3 * std::pow(1 - t, 2) * t * control_1 + 3 * (1 - t) * std::pow(t, 2) * control_2 + std::pow(t, 3) * knot_2;
            if (bezier_points.empty() || (bezier != bezier_points.back() && bezier != bezier_points.front())) {
                bezier_points.push_back(bezier);
            }
        }
    }

    static void GetBezierPoints(const std::vector<QVector2D>& knots,
                                std::vector<QVector2D>& bezierPoints) {

        bezierPoints.clear();

        std::vector<QVector2D> firstControlPoints;
        std::vector<QVector2D> secondControlPoints;

        GetCurveControlPoints(knots, firstControlPoints, secondControlPoints);

        for (unsigned int i = 1; i != knots.size(); ++i) {
            appendBezierSegment(knots[i - 1], knots[i], firstControlPoints[i - 1], secondControlPoints[i], bezierPoints);
        }
        appendBezierSegment(knots[knots.size() - 1], knots[0], firstControlPoints[knots.size() - 1], secondControlPoints[0], bezierPoints);
    }
};



class CentripetalAcceleration {
    std::deque<QVector2D>   _trace;
    QVector2D               _center;
    QVector2D               _last_center;
    QElapsedTimer           _timer;

private:

    bool getCircleByPoints(const QVector2D& point_1,
                                const QVector2D& point_2,
                                const QVector2D& point_3,
                                QVector2D& center);
public:
    CentripetalAcceleration() : _center(qQNaN(), qQNaN()),
                                _last_center(qQNaN(), qQNaN()) {
        _timer.start();
    }

    static bool isValidVector(const QVector2D& vec) {
        if ( qIsNaN(vec.x()) || qIsNaN(vec.y()) ||
             qIsInf(vec.x()) || qIsInf(vec.y()) ) {
            return false;
        } else {
            return true;
        }
    }

    bool update(const QVector2D& point);
    bool getCenter(QVector2D& center) const;
    bool getLastCenter(QVector2D& center) const;
    static double calcCentripetalAcceleration(double speed, const QVector2D& point_center, const QVector2D& point_body);
};

#endif // MATHUTILS_H
