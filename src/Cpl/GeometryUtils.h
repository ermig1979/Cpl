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
    template <class TD, class TS>
    CPL_INLINE TD Convert(TS src)
    {
        return (TD)src;
    }

    template <> CPL_INLINE ptrdiff_t Convert<ptrdiff_t, double>(double src)
    {
        return Round(src);
    }

    template <> CPL_INLINE ptrdiff_t Convert<ptrdiff_t, float>(float src)
    {
        return Round(src);
    }

    //---------------------------------------------------------------------------------------------

    template <typename T> struct Point
    {
        typedef T Type;

        T x, y;

        CPL_INLINE Point()
            : x(0)
            , y(0)
        {
        }

        template <typename TX, typename TY> CPL_INLINE Point(TX tx, TY ty)
            : x(Convert<T, TX>(tx))
            , y(Convert<T, TY>(ty))
        {
        }

        template <class TP, template<class> class TPoint> CPL_INLINE Point(const TPoint<TP>& p)
            : x(Convert<T, TP>(p.x))
            , y(Convert<T, TP>(p.y))
        {
        }

        template <class TP, template<class> class TPoint> CPL_INLINE operator TPoint<TP>() const
        {
            return TPoint<TP>(Convert<TP, T>(x), Convert<TP, T>(y));
        }

        template <typename TP> CPL_INLINE Point& operator = (const Point<TP>& p)
        {
            x = Convert<T, TP>(p.x);
            y = Convert<T, TP>(p.y);
            return *this;
        }

        template <typename TP> CPL_INLINE Point& operator += (const Point<TP>& p)
        {
            x += Convert<T, TP>(p.x);
            y += Convert<T, TP>(p.y);
            return *this;
        }

        template <typename TP> CPL_INLINE Point& operator -= (const Point<TP>& p)
        {
            x -= Convert<T, TP>(p.x);
            y -= Convert<T, TP>(p.y);
            return *this;
        }

        template <typename TA> CPL_INLINE Point& operator *= (const TA& a)
        {
            x = Convert<T, TA>(x * a);
            y = Convert<T, TA>(y * a);
            return *this;
        }

        CPL_INLINE Point& operator /= (double a)
        {
            x = Convert<T, double>(x / a);
            y = Convert<T, double>(y / a);
            return *this;
        }

        CPL_INLINE Point operator << (ptrdiff_t shift) const
        {
            return Point<T>(x << shift, y << shift);
        }

        CPL_INLINE Point operator >> (ptrdiff_t shift) const
        {
            return Point<T>(x >> shift, y >> shift);
        }
    };

    template <typename T> CPL_INLINE bool operator == (const Point<T>& p1, const Point<T>& p2)
    {
        return p1.x == p2.x && p1.y == p2.y;
    }

    template <typename T> CPL_INLINE bool operator != (const Point<T>& p1, const Point<T>& p2)
    {
        return p1.x != p2.x || p1.y != p2.y;
    }

    template <typename T> CPL_INLINE Point<T> operator + (const Point<T>& p1, const Point<T>& p2)
    {
        return Point<T>(p1.x + p2.x, p1.y + p2.y);
    }

    template <typename T> CPL_INLINE Point<T> operator - (const Point<T>& p1, const Point<T>& p2)
    {
        return Point<T>(p1.x - p2.x, p1.y - p2.y);
    }

    template <typename T> CPL_INLINE Point<T> operator * (const Point<T>& p1, const Point<T>& p2)
    {
        return Point<T>(p1.x * p2.x, p1.y * p2.y);
    }

    template <typename T> CPL_INLINE Point<T> operator / (const Point<T>& p1, const Point<T>& p2)
    {
        return Point<T>(p1.x / p2.x, p1.y / p2.y);
    }

    template <typename T> CPL_INLINE Point<T> operator - (const Point<T>& p)
    {
        return Point<T>(-p.x, -p.y);
    }

    template <typename TP, typename TA> CPL_INLINE Point<TP> operator / (const Point<TP>& p, const TA& a)
    {
        return Point<TP>(p.x / a, p.y / a);
    }

    template <typename TP, typename TA> CPL_INLINE Point<TP> operator * (const Point<TP>& p, const TA& a)
    {
        return Point<TP>(p.x * a, p.y * a);
    }

    template <typename TP, typename TA> CPL_INLINE Point<TP> operator * (const TA& a, const Point<TP>& p)
    {
        return Point<TP>(p.x * a, p.y * a);
    }

    template <typename T> CPL_INLINE T SquaredDistance(const Point<T>& p1, const Point<T>& p2)
    {
        Point<T> dp = p2 - p1;
        return dp.x * dp.x + dp.y * dp.y;
    }

    template <typename T> CPL_INLINE double Distance(const Point<T>& p1, const Point<T>& p2)
    {
        return ::sqrt(double(SquaredDistance(p1, p2)));
    }

    template <typename T> CPL_INLINE T DotProduct(const Point<T>& p1, const Point<T>& p2)
    {
        return (p1.x * p2.x + p1.y * p2.y);
    }

    template <typename T> CPL_INLINE T CrossProduct(const Point<T>& p1, const Point<T>& p2)
    {
        return (p1.x * p2.y - p1.y * p2.x);
    }

    template <typename T> CPL_INLINE Point<T> Max(const Point<T>& p1, const Point<T>& p2)
    {
        return Point<T>(std::max(p1.x, p2.x), std::max(p1.y, p2.y));
    }

    template <typename T> CPL_INLINE Point<T> Min(const Point<T>& p1, const Point<T>& p2)
    {
        return Point<T>(std::min(p1.x, p2.x), std::min(p1.y, p2.y));
    }

    //---------------------------------------------------------------------------------------------

    template <typename T> struct Rectangle
    {
        typedef T Type;

        T x, y, w, h;

        CPL_INLINE Rectangle()
            : x(0)
            , y(0)
            , w(0)
            , h(0)
        {
        }

        template <typename TX, typename TY, typename TW, typename TH> CPL_INLINE Rectangle(TX x_, TY y_, TW w_, TH h_)
            : x(x_)
            , y(y_)
            , w(w_)
            , h(h_)
        {
        }

        template <typename TP, typename TS> CPL_INLINE Rectangle(const Point<TP>& p, const Point<TS>& s)
            : x(p.x)
            , y(p.y)
            , w(s.x)
            , h(s.y)
        {
        }

        template <typename TP, typename TS> CPL_INLINE Rectangle(const Point<TS>& s)
            : x(0)
            , y(0)
            , w(s.x)
            , h(s.y)
        {
        }

        template <class TR, template<class> class TRectangle> CPL_INLINE Rectangle(const TRectangle<TR>& r)
            : x(r.x)
            , y(r.y)
            , w(r.w)
            , h(r.h)
        {
        }

        template <class TR, template<class> class TRectangle> CPL_INLINE operator TRectangle<TR>() const
        {
            return TRectangle<TR>(Convert<TR, T>(x), Convert<TR, T>(y), Convert<TR, T>(w), Convert<TR, T>(h));
        }

        template <typename TR> CPL_INLINE Rectangle<T>& operator = (const Rectangle<TR>& r)
        {
            x = Convert<T, TR>(r.x);
            y = Convert<T, TR>(r.y);
            w = Convert<T, TR>(r.w);
            h = Convert<T, TR>(r.h);
            return *this;
        }

        CPL_INLINE T Area() const
        {
            return w * h;
        }

        CPL_INLINE bool Empty() const
        {
            return Area() == 0;
        }

        CPL_INLINE Point<T> Size() const
        {
            return Point<T>(w, h);
        }

        CPL_INLINE Point<T> Center() const
        {
            return Point<T>(x + w / 2, y + h / 2);
        }

        CPL_INLINE T Right() const
        {
            return x + w;
        }

        CPL_INLINE T Bottom() const
        {
            return y + h;
        }

        template <typename TP> CPL_INLINE bool Contains(const Point<TP>& p) const
        {
            return p.x >= x && p.x < x + w && p.y >= y && p.y < y + h;
        }

        template <typename TX, typename TY> CPL_INLINE bool Contains(TX x_, TY y_) const
        {
            return Contains(Point<T>(x_, y_));
        }

        template <typename TR> CPL_INLINE bool Contains(const Rectangle <TR>& r) const
        {
            return r.x >= x && r.Right() <= Right() && r.y >= y && r.Bottom() <= Bottom();
        }

        template <typename TX, typename TY, typename TW, typename TH> CPL_INLINE bool Contains(TX x_, TY y_, TW w_, TH h_) const
        {
            return Contains(Rectangle<T>(x_, y_, w_, h_));
        }

        template <typename TR> CPL_INLINE Rectangle<T> Intersection(const Rectangle<TR>& rect) const
        {
            Rectangle<T> _r(rect);
            T l = std::max(x, _r.x);
            T t = std::max(y, _r.y);
            T r = std::max(l, std::min(Right(), _r.Right()));
            T b = std::max(b, std::min(Bottom(), _r.Bottom()));
            return Rectangle(l, t, r - l, b - t);
        }

        CPL_INLINE bool Overlaps(const Rectangle<T>& r) const
        {
            bool lr = x < r.Right();
            bool rl = Right() > r.x;
            bool tb = y < r.Bottom();
            bool bt = Bottom() > r.y;
            return (lr == rl) && (tb == bt);
        }

        CPL_INLINE std::vector<Point<T> > Polygon() const
        {
            return std::vector<Point<T> >( { Point<T>(x, y), Point<T>(x, y + h), Point<T>(x + w, y + h), Point<T>(x + w, y) } );
        }
    };

    //---------------------------------------------------------------------------------------------

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

    template <class T, template<class> class TPoint>
    CPL_INLINE T SquareDistanceToLine(const TPoint<T> & a, const TPoint<T> & b, const TPoint<T> & c)
    {
        return SquareDistance(a, ProjectionToLine(a, b, c));
    }

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

    template <class T, template<class> class TPoint>
    CPL_INLINE bool CrossSections(const TPoint<T> & a1, const TPoint<T> & a2, const TPoint<T> & b1, const TPoint<T> & b2)
    {
        return CrossScore(a1, a2, b1, b2) != 0;
    }

    template <class T, template<class> class TPoint> CPL_INLINE Point<T> OutsidePoint(const std::vector<Point<T> >& polygon)
    {
        size_t size = polygon.size();
        Point<T> outside = polygon[0];
        for (size_t i = 1; i < size; ++i)
            outside = Max<T>(outside, polygon[i]);
        return outside + Point<T>(1, 1);
    }

    template <class T, template<class> class TPoint> CPL_INLINE Rectangle<T> BoundingBox(const std::vector<TPoint<T> >& polygon)
    {
        size_t size = polygon.size();
        Point<T> min = polygon[0], max = polygon[0];
        for (size_t i = 1; i < size; ++i)
            min = Min<T>(min, polygon[i]), max = Max<T>(max, polygon[i]);
        return Rectangle<T>(min, max - min);
    }

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
