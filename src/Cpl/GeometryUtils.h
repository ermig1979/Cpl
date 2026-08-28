/*
* Common Purpose Library (http://github.com/ermig1979/Cpl).
*
* Copyright (c) 2021-2022 Yermalayeu Ihar,
*               2021-2022 Andrey Drogolyub.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#pragma once

#include "Cpl/Defs.h"
#include "Cpl/Utils.h"

#include <math.h>

namespace Cpl
{
    /*! @ingroup cpl_geometry
    * \brief Converts a value from type TS to type TD.
    * \tparam TD - Destination type.
    * \tparam TS - Source type.
    * \param [in] src - Value to convert.
    * \return src cast to TD.
    */
    template <class TD, class TS>
    CPL_INLINE TD Convert(TS src)
    {
        return (TD)src;
    }

    /*! @ingroup cpl_geometry
    * \brief Converts a double to ptrdiff_t by rounding to the nearest integer.
    * \param [in] src - Value to convert.
    * \return Nearest ptrdiff_t. Ties (fractional part 0.5) are rounded away from zero.
    */
    template <> CPL_INLINE ptrdiff_t Convert<ptrdiff_t, double>(double src)
    {
        return Round(src);
    }

    /*! @ingroup cpl_geometry
    * \brief Converts a float to ptrdiff_t by rounding to the nearest integer.
    * \param [in] src - Value to convert.
    * \return Nearest ptrdiff_t. Ties (fractional part 0.5) are rounded away from zero.
    */
    template <> CPL_INLINE ptrdiff_t Convert<ptrdiff_t, float>(float src)
    {
        return Round(src);
    }

    //---------------------------------------------------------------------------------------------

    /*! @ingroup cpl_geometry
    * \struct Point
    * \brief Two-dimensional point with coordinates of type T.
    * \tparam T - Coordinate type.
    * \note Coordinates can be converted from other types with Convert. Arithmetic operators
    *       act component-wise. Comparison requires both coordinates to match.
    */
    template <typename T> struct Point
    {
        typedef T Type; //!< Coordinate type of the point.

        T x; //!< Horizontal coordinate.
        T y; //!< Vertical coordinate.

        /*!
        * \fn Point()
        * \brief Constructs a point at the origin (0, 0).
        */
        CPL_INLINE Point()
            : x(0)
            , y(0)
        {
        }

        /*!
        * \fn Point(TX tx, TY ty)
        * \brief Constructs a point from the given coordinates, converting each to T.
        * \tparam TX - Type of the x coordinate.
        * \tparam TY - Type of the y coordinate.
        * \param [in] tx - X coordinate.
        * \param [in] ty - Y coordinate.
        */
        template <typename TX, typename TY> CPL_INLINE Point(TX tx, TY ty)
            : x(Convert<T, TX>(tx))
            , y(Convert<T, TY>(ty))
        {
        }

        /*!
        * \fn Point(const TPoint<TP>& p)
        * \brief Constructs a point by converting another point-like object.
        * \tparam TP - Coordinate type of the source point.
        * \tparam TPoint - Point template of the source. Must provide x and y members.
        * \param [in] p - Source point.
        */
        template <class TP, template<class> class TPoint> CPL_INLINE Point(const TPoint<TP>& p)
            : x(Convert<T, TP>(p.x))
            , y(Convert<T, TP>(p.y))
        {
        }

        /*!
        * \fn operator TPoint<TP>() const
        * \brief Converts this point to another point-like type.
        * \tparam TP - Coordinate type of the destination point.
        * \tparam TPoint - Point template of the destination.
        * \return A TPoint whose coordinates are this point converted to TP.
        */
        template <class TP, template<class> class TPoint> CPL_INLINE operator TPoint<TP>() const
        {
            return TPoint<TP>(Convert<TP, T>(x), Convert<TP, T>(y));
        }

        /*!
        * \fn Point& operator = (const Point<TP>& p)
        * \brief Assigns another point, converting its coordinates to T.
        * \tparam TP - Coordinate type of the source point.
        * \param [in] p - Source point.
        * \return Reference to this point.
        */
        template <typename TP> CPL_INLINE Point& operator = (const Point<TP>& p)
        {
            x = Convert<T, TP>(p.x);
            y = Convert<T, TP>(p.y);
            return *this;
        }

        /*!
        * \fn Point& operator += (const Point<TP>& p)
        * \brief Adds another point to this one, converting its coordinates to T.
        * \tparam TP - Coordinate type of the source point.
        * \param [in] p - Point to add.
        * \return Reference to this point.
        */
        template <typename TP> CPL_INLINE Point& operator += (const Point<TP>& p)
        {
            x += Convert<T, TP>(p.x);
            y += Convert<T, TP>(p.y);
            return *this;
        }

        /*!
        * \fn Point& operator -= (const Point<TP>& p)
        * \brief Subtracts another point from this one, converting its coordinates to T.
        * \tparam TP - Coordinate type of the source point.
        * \param [in] p - Point to subtract.
        * \return Reference to this point.
        */
        template <typename TP> CPL_INLINE Point& operator -= (const Point<TP>& p)
        {
            x -= Convert<T, TP>(p.x);
            y -= Convert<T, TP>(p.y);
            return *this;
        }

        /*!
        * \fn Point& operator *= (const TA& a)
        * \brief Scales this point by a scalar, converting each product back to T.
        * \tparam TA - Type of the scale factor.
        * \param [in] a - Scale factor applied to both coordinates.
        * \return Reference to this point.
        */
        template <typename TA> CPL_INLINE Point& operator *= (const TA& a)
        {
            x = Convert<T, TA>(x * a);
            y = Convert<T, TA>(y * a);
            return *this;
        }

        /*!
        * \fn Point& operator /= (double a)
        * \brief Divides this point by a scalar, converting each quotient back to T.
        * \param [in] a - Divisor applied to both coordinates.
        * \return Reference to this point.
        */
        CPL_INLINE Point& operator /= (double a)
        {
            x = Convert<T, double>(x / a);
            y = Convert<T, double>(y / a);
            return *this;
        }

        /*!
        * \fn Point operator << (ptrdiff_t shift) const
        * \brief Returns a point whose coordinates are shifted left.
        * \param [in] shift - Number of bits to shift each coordinate.
        * \return Point (x << shift, y << shift). T must support bitwise left shift.
        */
        CPL_INLINE Point operator << (ptrdiff_t shift) const
        {
            return Point<T>(x << shift, y << shift);
        }

        /*!
        * \fn Point operator >> (ptrdiff_t shift) const
        * \brief Returns a point whose coordinates are shifted right.
        * \param [in] shift - Number of bits to shift each coordinate.
        * \return Point (x >> shift, y >> shift). T must support bitwise right shift.
        */
        CPL_INLINE Point operator >> (ptrdiff_t shift) const
        {
            return Point<T>(x >> shift, y >> shift);
        }
    };

    /*! @ingroup cpl_geometry
    * \brief Tests whether two points have equal coordinates.
    * \tparam T - Coordinate type.
    * \param [in] p1 - First point.
    * \param [in] p2 - Second point.
    * \return true if both x and y are equal.
    */
    template <typename T> CPL_INLINE bool operator == (const Point<T>& p1, const Point<T>& p2)
    {
        return p1.x == p2.x && p1.y == p2.y;
    }

    /*! @ingroup cpl_geometry
    * \brief Tests whether two points differ in at least one coordinate.
    * \tparam T - Coordinate type.
    * \param [in] p1 - First point.
    * \param [in] p2 - Second point.
    * \return true if x or y differs.
    */
    template <typename T> CPL_INLINE bool operator != (const Point<T>& p1, const Point<T>& p2)
    {
        return p1.x != p2.x || p1.y != p2.y;
    }

    /*! @ingroup cpl_geometry
    * \brief Adds two points component-wise.
    * \tparam T - Coordinate type.
    * \param [in] p1 - First point.
    * \param [in] p2 - Second point.
    * \return Point (p1.x + p2.x, p1.y + p2.y).
    */
    template <typename T> CPL_INLINE Point<T> operator + (const Point<T>& p1, const Point<T>& p2)
    {
        return Point<T>(p1.x + p2.x, p1.y + p2.y);
    }

    /*! @ingroup cpl_geometry
    * \brief Subtracts two points component-wise.
    * \tparam T - Coordinate type.
    * \param [in] p1 - First point.
    * \param [in] p2 - Second point.
    * \return Point (p1.x - p2.x, p1.y - p2.y).
    */
    template <typename T> CPL_INLINE Point<T> operator - (const Point<T>& p1, const Point<T>& p2)
    {
        return Point<T>(p1.x - p2.x, p1.y - p2.y);
    }

    /*! @ingroup cpl_geometry
    * \brief Multiplies two points component-wise.
    * \tparam T - Coordinate type.
    * \param [in] p1 - First point.
    * \param [in] p2 - Second point.
    * \return Point (p1.x * p2.x, p1.y * p2.y).
    */
    template <typename T> CPL_INLINE Point<T> operator * (const Point<T>& p1, const Point<T>& p2)
    {
        return Point<T>(p1.x * p2.x, p1.y * p2.y);
    }

    /*! @ingroup cpl_geometry
    * \brief Divides two points component-wise.
    * \tparam T - Coordinate type.
    * \param [in] p1 - Dividend point.
    * \param [in] p2 - Divisor point.
    * \return Point (p1.x / p2.x, p1.y / p2.y).
    */
    template <typename T> CPL_INLINE Point<T> operator / (const Point<T>& p1, const Point<T>& p2)
    {
        return Point<T>(p1.x / p2.x, p1.y / p2.y);
    }

    /*! @ingroup cpl_geometry
    * \brief Negates both coordinates of a point.
    * \tparam T - Coordinate type.
    * \param [in] p - Point to negate.
    * \return Point (-p.x, -p.y).
    */
    template <typename T> CPL_INLINE Point<T> operator - (const Point<T>& p)
    {
        return Point<T>(-p.x, -p.y);
    }

    /*! @ingroup cpl_geometry
    * \brief Divides a point by a scalar.
    * \tparam TP - Coordinate type of the point.
    * \tparam TA - Type of the divisor.
    * \param [in] p - Point to divide.
    * \param [in] a - Divisor applied to both coordinates.
    * \return Point (p.x / a, p.y / a).
    */
    template <typename TP, typename TA> CPL_INLINE Point<TP> operator / (const Point<TP>& p, const TA& a)
    {
        return Point<TP>(p.x / a, p.y / a);
    }

    /*! @ingroup cpl_geometry
    * \brief Multiplies a point by a scalar.
    * \tparam TP - Coordinate type of the point.
    * \tparam TA - Type of the scale factor.
    * \param [in] p - Point to scale.
    * \param [in] a - Scale factor applied to both coordinates.
    * \return Point (p.x * a, p.y * a).
    */
    template <typename TP, typename TA> CPL_INLINE Point<TP> operator * (const Point<TP>& p, const TA& a)
    {
        return Point<TP>(p.x * a, p.y * a);
    }

    /*! @ingroup cpl_geometry
    * \brief Multiplies a scalar by a point.
    * \tparam TP - Coordinate type of the point.
    * \tparam TA - Type of the scale factor.
    * \param [in] a - Scale factor applied to both coordinates.
    * \param [in] p - Point to scale.
    * \return Point (p.x * a, p.y * a).
    */
    template <typename TP, typename TA> CPL_INLINE Point<TP> operator * (const TA& a, const Point<TP>& p)
    {
        return Point<TP>(p.x * a, p.y * a);
    }

    /*! @ingroup cpl_geometry
    * \brief Returns the squared Euclidean distance between two points.
    * \tparam T - Coordinate type.
    * \param [in] p1 - First point.
    * \param [in] p2 - Second point.
    * \return (p2.x - p1.x)^2 + (p2.y - p1.y)^2.
    */
    template <typename T> CPL_INLINE T SquaredDistance(const Point<T>& p1, const Point<T>& p2)
    {
        Point<T> dp = p2 - p1;
        return dp.x * dp.x + dp.y * dp.y;
    }

    /*! @ingroup cpl_geometry
    * \brief Returns the Euclidean distance between two points.
    * \tparam T - Coordinate type.
    * \param [in] p1 - First point.
    * \param [in] p2 - Second point.
    * \return Square root of SquaredDistance(p1, p2), computed as a double.
    */
    template <typename T> CPL_INLINE double Distance(const Point<T>& p1, const Point<T>& p2)
    {
        return ::sqrt(double(SquaredDistance(p1, p2)));
    }

    /*! @ingroup cpl_geometry
    * \brief Returns the 2D dot product of two points treated as vectors.
    * \tparam T - Coordinate type.
    * \param [in] p1 - First vector.
    * \param [in] p2 - Second vector.
    * \return p1.x * p2.x + p1.y * p2.y.
    */
    template <typename T> CPL_INLINE T DotProduct(const Point<T>& p1, const Point<T>& p2)
    {
        return (p1.x * p2.x + p1.y * p2.y);
    }

    /*! @ingroup cpl_geometry
    * \brief Returns the 2D cross product of two points treated as vectors.
    * \tparam T - Coordinate type.
    * \param [in] p1 - First vector.
    * \param [in] p2 - Second vector.
    * \return p1.x * p2.y - p1.y * p2.x. Positive when p2 is counterclockwise from p1.
    */
    template <typename T> CPL_INLINE T CrossProduct(const Point<T>& p1, const Point<T>& p2)
    {
        return (p1.x * p2.y - p1.y * p2.x);
    }

    /*! @ingroup cpl_geometry
    * \brief Returns the component-wise maximum of two points.
    * \tparam T - Coordinate type.
    * \param [in] p1 - First point.
    * \param [in] p2 - Second point.
    * \return Point (max(p1.x, p2.x), max(p1.y, p2.y)).
    */
    template <typename T> CPL_INLINE Point<T> Max(const Point<T>& p1, const Point<T>& p2)
    {
        return Point<T>(std::max(p1.x, p2.x), std::max(p1.y, p2.y));
    }

    /*! @ingroup cpl_geometry
    * \brief Returns the component-wise minimum of two points.
    * \tparam T - Coordinate type.
    * \param [in] p1 - First point.
    * \param [in] p2 - Second point.
    * \return Point (min(p1.x, p2.x), min(p1.y, p2.y)).
    */
    template <typename T> CPL_INLINE Point<T> Min(const Point<T>& p1, const Point<T>& p2)
    {
        return Point<T>(std::min(p1.x, p2.x), std::min(p1.y, p2.y));
    }

    //---------------------------------------------------------------------------------------------

    /*! @ingroup cpl_geometry
    * \struct Rectangle
    * \brief Axis-aligned rectangle with origin (x, y) and size (w, h) of type T.
    * \tparam T - Coordinate and size type.
    * \note The origin is the top-left corner when y increases downward. Right() is x + w
    *       and Bottom() is y + h. Point containment is half-open: [x, x + w) x [y, y + h).
    */
    template <typename T> struct Rectangle
    {
        typedef T Type; //!< Coordinate and size type of the rectangle.

        T x; //!< Left edge (horizontal origin).
        T y; //!< Top edge (vertical origin).
        T w; //!< Width.
        T h; //!< Height.

        /*!
        * \fn Rectangle()
        * \brief Constructs an empty rectangle at the origin (0, 0, 0, 0).
        */
        CPL_INLINE Rectangle()
            : x(0)
            , y(0)
            , w(0)
            , h(0)
        {
        }

        /*!
        * \fn Rectangle(TX x_, TY y_, TW w_, TH h_)
        * \brief Constructs a rectangle from origin and size.
        * \tparam TX - Type of the left edge.
        * \tparam TY - Type of the top edge.
        * \tparam TW - Type of the width.
        * \tparam TH - Type of the height.
        * \param [in] x_ - Left edge.
        * \param [in] y_ - Top edge.
        * \param [in] w_ - Width.
        * \param [in] h_ - Height.
        */
        template <typename TX, typename TY, typename TW, typename TH> CPL_INLINE Rectangle(TX x_, TY y_, TW w_, TH h_)
            : x(x_)
            , y(y_)
            , w(w_)
            , h(h_)
        {
        }

        /*!
        * \fn Rectangle(const Point<TP>& p, const Point<TS>& s)
        * \brief Constructs a rectangle from an origin point and a size point.
        * \tparam TP - Coordinate type of the origin.
        * \tparam TS - Coordinate type of the size.
        * \param [in] p - Origin (x, y).
        * \param [in] s - Size; s.x is the width and s.y is the height.
        */
        template <typename TP, typename TS> CPL_INLINE Rectangle(const Point<TP>& p, const Point<TS>& s)
            : x(p.x)
            , y(p.y)
            , w(s.x)
            , h(s.y)
        {
        }

        /*!
        * \fn Rectangle(const Point<TS>& s)
        * \brief Constructs a rectangle at the origin with the given size.
        * \tparam TS - Coordinate type of the size.
        * \param [in] s - Size; s.x is the width and s.y is the height. Origin is (0, 0).
        */
        template <typename TP, typename TS> CPL_INLINE Rectangle(const Point<TS>& s)
            : x(0)
            , y(0)
            , w(s.x)
            , h(s.y)
        {
        }

        /*!
        * \fn Rectangle(const TRectangle<TR>& r)
        * \brief Constructs a rectangle by copying another rectangle-like object.
        * \tparam TR - Coordinate type of the source rectangle.
        * \tparam TRectangle - Rectangle template of the source. Must provide x, y, w and h.
        * \param [in] r - Source rectangle.
        */
        template <class TR, template<class> class TRectangle> CPL_INLINE Rectangle(const TRectangle<TR>& r)
            : x(r.x)
            , y(r.y)
            , w(r.w)
            , h(r.h)
        {
        }

        /*!
        * \fn operator TRectangle<TR>() const
        * \brief Converts this rectangle to another rectangle-like type.
        * \tparam TR - Coordinate type of the destination rectangle.
        * \tparam TRectangle - Rectangle template of the destination.
        * \return A TRectangle whose origin and size are this rectangle converted to TR.
        */
        template <class TR, template<class> class TRectangle> CPL_INLINE operator TRectangle<TR>() const
        {
            return TRectangle<TR>(Convert<TR, T>(x), Convert<TR, T>(y), Convert<TR, T>(w), Convert<TR, T>(h));
        }

        /*!
        * \fn Rectangle<T>& operator = (const Rectangle<TR>& r)
        * \brief Assigns another rectangle, converting its origin and size to T.
        * \tparam TR - Coordinate type of the source rectangle.
        * \param [in] r - Source rectangle.
        * \return Reference to this rectangle.
        */
        template <typename TR> CPL_INLINE Rectangle<T>& operator = (const Rectangle<TR>& r)
        {
            x = Convert<T, TR>(r.x);
            y = Convert<T, TR>(r.y);
            w = Convert<T, TR>(r.w);
            h = Convert<T, TR>(r.h);
            return *this;
        }

        /*!
        * \fn T Area() const
        * \brief Returns the area of the rectangle.
        * \return w * h.
        */
        CPL_INLINE T Area() const
        {
            return w * h;
        }

        /*!
        * \fn bool Empty() const
        * \brief Checks whether the rectangle has zero area.
        * \return true if Area() is 0.
        */
        CPL_INLINE bool Empty() const
        {
            return Area() == 0;
        }

        /*!
        * \fn Point<T> Size() const
        * \brief Returns the size of the rectangle as a point.
        * \return Point (w, h).
        */
        CPL_INLINE Point<T> Size() const
        {
            return Point<T>(w, h);
        }

        /*!
        * \fn Point<T> Center() const
        * \brief Returns the center of the rectangle.
        * \return Point (x + w / 2, y + h / 2). Integer T uses truncating division.
        */
        CPL_INLINE Point<T> Center() const
        {
            return Point<T>(x + w / 2, y + h / 2);
        }

        /*!
        * \fn T Right() const
        * \brief Returns the right edge of the rectangle.
        * \return x + w.
        */
        CPL_INLINE T Right() const
        {
            return x + w;
        }

        /*!
        * \fn T Bottom() const
        * \brief Returns the bottom edge of the rectangle.
        * \return y + h.
        */
        CPL_INLINE T Bottom() const
        {
            return y + h;
        }

        /*!
        * \fn bool Contains(const Point<TP>& p) const
        * \brief Tests whether a point lies inside the half-open rectangle.
        * \tparam TP - Coordinate type of the point.
        * \param [in] p - Point to test.
        * \return true if p.x is in [x, x + w) and p.y is in [y, y + h).
        */
        template <typename TP> CPL_INLINE bool Contains(const Point<TP>& p) const
        {
            return p.x >= x && p.x < x + w && p.y >= y && p.y < y + h;
        }

        /*!
        * \fn bool Contains(TX x_, TY y_) const
        * \brief Tests whether the point (x_, y_) lies inside the half-open rectangle.
        * \tparam TX - Type of the x coordinate.
        * \tparam TY - Type of the y coordinate.
        * \param [in] x_ - X coordinate to test.
        * \param [in] y_ - Y coordinate to test.
        * \return true if Contains(Point(x_, y_)) is true.
        */
        template <typename TX, typename TY> CPL_INLINE bool Contains(TX x_, TY y_) const
        {
            return Contains(Point<T>(x_, y_));
        }

        /*!
        * \fn bool Contains(const Rectangle<TR>& r) const
        * \brief Tests whether another rectangle lies completely inside this one.
        * \tparam TR - Coordinate type of the other rectangle.
        * \param [in] r - Rectangle to test.
        * \return true if r's origin is at or inside this origin and r's right and bottom
        *         edges are at or inside this rectangle's right and bottom edges.
        */
        template <typename TR> CPL_INLINE bool Contains(const Rectangle <TR>& r) const
        {
            return r.x >= x && r.Right() <= Right() && r.y >= y && r.Bottom() <= Bottom();
        }

        /*!
        * \fn bool Contains(TX x_, TY y_, TW w_, TH h_) const
        * \brief Tests whether the rectangle (x_, y_, w_, h_) lies completely inside this one.
        * \tparam TX - Type of the left edge.
        * \tparam TY - Type of the top edge.
        * \tparam TW - Type of the width.
        * \tparam TH - Type of the height.
        * \param [in] x_ - Left edge of the rectangle to test.
        * \param [in] y_ - Top edge of the rectangle to test.
        * \param [in] w_ - Width of the rectangle to test.
        * \param [in] h_ - Height of the rectangle to test.
        * \return true if Contains(Rectangle(x_, y_, w_, h_)) is true.
        */
        template <typename TX, typename TY, typename TW, typename TH> CPL_INLINE bool Contains(TX x_, TY y_, TW w_, TH h_) const
        {
            return Contains(Rectangle<T>(x_, y_, w_, h_));
        }

        /*!
        * \fn Rectangle<T> Intersection(const Rectangle<TR>& rect) const
        * \brief Returns the overlapping region of this rectangle and another.
        * \tparam TR - Coordinate type of the other rectangle.
        * \param [in] rect - Rectangle to intersect with.
        * \return The overlapping rectangle. Width and/or height are zero when the
        *         rectangles do not overlap.
        */
        template <typename TR> CPL_INLINE Rectangle<T> Intersection(const Rectangle<TR>& rect) const
        {
            Rectangle<T> _r(rect);
            T l = std::max(x, _r.x);
            T t = std::max(y, _r.y);
            T r = std::max(l, std::min(Right(), _r.Right()));
            T b = std::max(b, std::min(Bottom(), _r.Bottom()));
            return Rectangle(l, t, r - l, b - t);
        }

        /*!
        * \fn bool Overlaps(const Rectangle<T>& r) const
        * \brief Tests whether this rectangle and another have overlapping interiors.
        * \param [in] r - Rectangle to test.
        * \return true if the interiors overlap. Rectangles that only touch at an edge
        *         do not overlap.
        */
        CPL_INLINE bool Overlaps(const Rectangle<T>& r) const
        {
            bool lr = x < r.Right();
            bool rl = Right() > r.x;
            bool tb = y < r.Bottom();
            bool bt = Bottom() > r.y;
            return (lr == rl) && (tb == bt);
        }

        /*!
        * \fn std::vector<Point<T> > Polygon() const
        * \brief Returns the four corners of the rectangle as a polygon.
        * \return Vertices in order: (x, y), (x, y + h), (x + w, y + h), (x + w, y).
        */
        CPL_INLINE std::vector<Point<T> > Polygon() const
        {
            return std::vector<Point<T> >( { Point<T>(x, y), Point<T>(x, y + h), Point<T>(x + w, y + h), Point<T>(x + w, y) } );
        }
    };

    //---------------------------------------------------------------------------------------------

    /*! @ingroup cpl_geometry
    * \brief Projects point a onto the infinite line through b and c.
    * \tparam T - Coordinate type.
    * \tparam TPoint - Point template. Must provide x and y and support subtraction.
    * \param [in] a - Point to project.
    * \param [in] b - First point on the line.
    * \param [in] c - Second point on the line.
    * \return The closest point on line bc to a. If the squared distance between b and c
    *         is less than 1, returns b.
    */
    template <class T, template<class> class TPoint> 
    CPL_INLINE TPoint<T> ProjectionToLine(const TPoint<T> & a, const TPoint<T> & b, const TPoint<T> & c)
    {
        if (SquareDistance(b, c) < 1)
        {
            return b;
        }
        else
        {
            TPoint<T> d = c - b;
            T e = a.x * d.x + a.y * d.y;
            T f = b.x * d.y - b.y * d.x;
            T d2 = d.x * d.x + d.y * d.y;
            T x = (e * d.x + f * d.y) / d2;
            T y = (e * d.y - f * d.x) / d2;
            return TPoint<T>(x, y);
        }
    }

    /*! @ingroup cpl_geometry
    * \brief Returns the squared distance from point a to the line through b and c.
    * \tparam T - Coordinate type.
    * \tparam TPoint - Point template. Must provide x and y.
    * \param [in] a - Point to measure from.
    * \param [in] b - First point on the line.
    * \param [in] c - Second point on the line.
    * \return Squared distance between a and ProjectionToLine(a, b, c).
    */
    template <class T, template<class> class TPoint>
    CPL_INLINE T SquareDistanceToLine(const TPoint<T> & a, const TPoint<T> & b, const TPoint<T> & c)
    {
        return SquareDistance(a, ProjectionToLine(a, b, c));
    }

    /*! @ingroup cpl_geometry
    * \brief Classifies the intersection of segments a1-a2 and b1-b2.
    * \tparam T - Coordinate type.
    * \tparam TPoint - Point template. Must provide x and y.
    * \param [in] a1 - Start of the first segment.
    * \param [in] a2 - End of the first segment.
    * \param [in] b1 - Start of the second segment.
    * \param [in] b2 - End of the second segment.
    * \return 0 if the segments are parallel or do not intersect. Otherwise the magnitude is
    *         1 if they meet at an endpoint and 2 if they cross in the interior. The sign is
    *         positive when the direction from a1-a2 to b1-b2 is a left turn (positive 2D
    *         cross product), negative otherwise.
    */
    template <class T, template<class> class TPoint>
    CPL_INLINE int CrossScore(const TPoint<T> & a1, const TPoint<T> & a2, const TPoint<T> & b1, const TPoint<T> & b2)
    {
        T Aa = a1.y - a2.y; 
        T Ba = a2.x - a1.x; 
        T Ca = a1.x*a2.y - a2.x*a1.y;

        T Ab = b1.y - b2.y; 
        T Bb = b2.x - b1.x; 
        T Cb = b1.x*b2.y - b2.x*b1.y;

        T D = Aa*Bb - Ab*Ba;

        if(D == 0)
            return 0;

        T x = Ba*Cb - Bb*Ca;
        T y = Ab*Ca - Aa*Cb;

        T a1x = a1.x*D;
        T a1y = a1.y*D;
        T a2x = a2.x*D;
        T a2y = a2.y*D;
        T b1x = b1.x*D;
        T b1y = b1.y*D;
        T b2x = b2.x*D;
        T b2y = b2.y*D;

        if((x < a1x && x < a2x) || (x > a2x && x > a1x) || (y < a1y && y < a2y) || (y > a2y && y > a1y) ||
           (x < b1x && x < b2x) || (x > b2x && x > b1x) || (y < b1y && y < b2y) || (y > b2y && y > b1y))
            return 0;

        return ((x == b1x && y == b1y) || (x == b2x && y == b2y) || 
                (x == a1x && y == a1y) || (x == a2x && y == a2y) ? 1 : 2)*
                (Ba*Ab - Bb*Aa < 0 ? 1 : -1);
    }

    /*! @ingroup cpl_geometry
    * \brief Checks whether segments a1-a2 and b1-b2 intersect, including at endpoints.
    * \tparam T - Coordinate type.
    * \tparam TPoint - Point template. Must provide x and y.
    * \param [in] a1 - Start of the first segment.
    * \param [in] a2 - End of the first segment.
    * \param [in] b1 - Start of the second segment.
    * \param [in] b2 - End of the second segment.
    * \return true if CrossScore is not 0.
    */
    template <class T, template<class> class TPoint>
    CPL_INLINE bool CrossSections(const TPoint<T> & a1, const TPoint<T> & a2, const TPoint<T> & b1, const TPoint<T> & b2)
    {
        return CrossScore(a1, a2, b1, b2) != 0;
    }

    /*! @ingroup cpl_geometry
    * \brief Returns a point guaranteed to lie outside the given polygon.
    * \tparam T - Coordinate type.
    * \tparam TPoint - Point template. The polygon is a vector of Point.
    * \param [in] polygon - Vertices of the polygon. Must not be empty.
    * \return Component-wise maximum of the vertices plus (1, 1).
    */
    template <class T, template<class> class TPoint> CPL_INLINE Point<T> OutsidePoint(const std::vector<Point<T> >& polygon)
    {
        size_t size = polygon.size();
        Point<T> outside = polygon[0];
        for (size_t i = 1; i < size; ++i)
            outside = Max<T>(outside, polygon[i]);
        return outside + Point<T>(1, 1);
    }

    /*! @ingroup cpl_geometry
    * \brief Returns the axis-aligned bounding box of a polygon.
    * \tparam T - Coordinate type.
    * \tparam TPoint - Point template of the vertices. Must provide x and y.
    * \param [in] polygon - Vertices of the polygon. Must not be empty.
    * \return Rectangle whose origin is the component-wise minimum of the vertices and whose
    *         size is the component-wise maximum minus that minimum.
    */
    template <class T, template<class> class TPoint> CPL_INLINE Rectangle<T> BoundingBox(const std::vector<TPoint<T> >& polygon)
    {
        size_t size = polygon.size();
        Point<T> min = polygon[0], max = polygon[0];
        for (size_t i = 1; i < size; ++i)
            min = Min<T>(min, polygon[i]), max = Max<T>(max, polygon[i]);
        return Rectangle<T>(min, max - min);
    }

    /*! @ingroup cpl_geometry
    * \brief Tests whether a point lies inside a polygon or on its boundary.
    * \tparam T - Coordinate type.
    * \tparam TPoint - Point template of the vertices. Must provide x and y.
    * \param [in] polygon - Vertices of the polygon in order. Must not be empty.
    * \param [in] point - Point to test.
    * \param [in] outside - Far point used to cast a ray from point. If this is (0, 0), a
    *                       point outside the polygon is chosen automatically with OutsidePoint.
    * \return true if point coincides with a vertex or the ray from point to outside crosses
    *         the polygon boundary (the summed CrossScore is not 0).
    */
    template <class T, template<class> class TPoint>
    CPL_INLINE bool PolygonHasPoint(const std::vector<TPoint<T> >& polygon, const TPoint<T> & point, TPoint<T> outside = TPoint<T>())
    {
        if (outside == Point<T>())
            outside = OutsidePoint<T, TPoint>(polygon);
        size_t size = polygon.size();
        int crossScore = CrossScore(point, outside, polygon[size - 1], polygon[0]);
        for (size_t i = 1; i < size; ++i)
        {
            if (point == polygon[i])
                return true;
            crossScore += CrossScore(point, outside, polygon[i - 1], polygon[i]);
        }
        return crossScore != 0 || point == polygon[0];
    }

    /*! @ingroup cpl_geometry
    * \brief Tests whether a polygon and an axis-aligned rectangle have a non-empty intersection.
    * \tparam T - Coordinate type.
    * \tparam TPoint - Point template of the vertices. Must provide x and y.
    * \param [in] polygon - Vertices of the polygon in order. Must not be empty.
    * \param [in] rect - Axis-aligned rectangle.
    * \return true if the rectangle contains the polygon, a rectangle corner lies inside the
    *         polygon, a polygon vertex lies inside the rectangle, or an edge of the polygon
    *         intersects an edge of the rectangle. false if their bounding boxes do not overlap.
    */
    template <class T, template<class> class TPoint>
    CPL_INLINE bool PolygonOverlapsRectangle(const std::vector<TPoint<T> >& polygon, const Rectangle<T>& rect)
    {
        Rectangle<T> bbox = BoundingBox(polygon);
        Point<T> outside = Point<T>(bbox.x - 1, bbox.y - 1);
        if (!bbox.Overlaps(rect))
            return false;
        if (rect.Contains(bbox))
            return true;
        std::vector<Point<T> > points = rect.Polygon();
        for(int i = 0; i < 4; ++i)
            if (bbox.Contains(points[i]) && PolygonHasPoint(polygon, points[i], outside))
                return true;
        size_t size = polygon.size();
        for (size_t c = 0; c < size; ++c)
        {
            size_t p = c ? c - 1 : size - 1;
            if (rect.Contains(polygon[c]))
                return true;
            for (int i = 0; i < 4; ++i)
                if (CrossScore(polygon[c], polygon[p], points[i], points[(i + 1) & 3]))
                    return true;
        }
        return false;
    }
}
