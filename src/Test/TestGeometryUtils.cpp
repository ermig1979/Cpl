/*
* Tests for Common Purpose Library (http://github.com/ermig1979/Cpl).
*
* Copyright (c) 2021-2022 Yermalayeu Ihar.
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

#include "Test/Test.h"

#include "Cpl/GeometryUtils.h"

namespace Test
{
    typedef Cpl::Point<int> Point;
    typedef std::vector<Point> Polygon;
    typedef Cpl::Rectangle<int> Rect;

    typedef Cpl::Point<float> PointF;
    typedef std::vector<PointF> PolygonF;
    typedef Cpl::Rectangle<float> RectF;

    Polygon CreatePolygon()
    {
        Polygon polygon;
        polygon.push_back(Point(2, 2));
        polygon.push_back(Point(4, -2));
        polygon.push_back(Point(2, -6));
        polygon.push_back(Point(9, -1));
        polygon.push_back(Point(9, 4));
        polygon.push_back(Point(-1, 6));
        polygon.push_back(Point(-1, 2));
        return polygon;
    }

    bool PolygonHasPointTest()
    {
        Polygon polygon = CreatePolygon();

        if (Cpl::PolygonHasPoint(polygon, Point(0, 0)) == true)
            return false;

        if (Cpl::PolygonHasPoint(polygon, Point(7, -7)) == true)
            return false;

        if (Cpl::PolygonHasPoint(polygon, Point(5, 5)) == true)
            return false;

        if (Cpl::PolygonHasPoint(polygon, Point(3, 3)) == false)
            return false;

        if (Cpl::PolygonHasPoint(polygon, Point(9, 0)) == false)
            return false;

        if (Cpl::PolygonHasPoint(polygon, Point(2, 2)) == false)
            return false;

        return true;
    }

    bool PolygonOverlapsRectangleTest()
    {
        Polygon polygon = CreatePolygon();

        if (Cpl::PolygonOverlapsRectangle(polygon, Rect(-1, -1, 2, 2)) == true)
            return false;

        if (Cpl::PolygonOverlapsRectangle(polygon, Rect(6, 6, 9, 9)) == true)
            return false;

        if (Cpl::PolygonOverlapsRectangle(polygon, Rect(4, 0, 3, 3)) == false)
            return false;

        if (Cpl::PolygonOverlapsRectangle(polygon, Rect(8, 3, 3, 3)) == false)
            return false;

        if (Cpl::PolygonOverlapsRectangle(polygon, Rect(1, -8, 5, 3)) == false)
            return false;

        if (Cpl::PolygonOverlapsRectangle(polygon, Rect(-10, -10, 20, 20)) == false)
            return false;

        return true;
    }

    bool PolygonOverlapsRectangleFloatTest()
    {
        PolygonF polygon = {
            {0, 0},
            {1, 0.5f},
            {2, 0},
            {2, 2},
            {1, 1.5f},
            {0, 2}
        };

        if (Cpl::PolygonOverlapsRectangle(polygon, RectF(0.75f, 0.f, 0.5f, 0.25f)) == true)
            return false;

        if (Cpl::PolygonOverlapsRectangle(polygon, RectF(0.75f, 0.f, 0.5f, 0.5f)) == false)
            return false;

        if (Cpl::PolygonOverlapsRectangle(polygon, RectF(0.f, 0.f, 0.5f, 0.5f)) == false)
            return false;

        if (Cpl::PolygonOverlapsRectangle(polygon, RectF(0.f, 0.5f, 0.5f, 0.5f)) == false)
            return false;

        return true;
    }
}
