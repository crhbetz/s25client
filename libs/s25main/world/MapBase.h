// Copyright (c) 2005 - 2017 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include "RTTR_Assert.h"
#include "helpers/EnumArray.h"
#include "helpers/EnumRange.h"
#include "gameTypes/Direction.h"
#include "gameTypes/MapCoordinates.h"
#include "gameTypes/ShipDirection.h"
#include <array>
#include <vector>

struct AlwaysTrue
{
    template<typename T>
    constexpr bool operator()(T&&) const noexcept
    {
        return true;
    }
};
struct ReturnMapPoint
{
    constexpr MapPoint operator()(MapPoint pt, unsigned /*radius*/) const noexcept { return pt; }
};
namespace detail {
template<typename T_TransformPt>
using GetPointsResult_t = std::vector<decltype(std::declval<T_TransformPt>()(MapPoint{}, unsigned{}))>;
}

/// Base class for a map. A map has a size and functions for getting from one point to another in that map
class MapBase
{
    /// Size of the map in nodes
    MapExtent size_;

public:
    static unsigned CreateGUIID(MapPoint pt);

    MapBase();

    virtual void Resize(const MapExtent& newSize);

    /// Return the size of the world
    unsigned short GetWidth() const { return GetSize().x; }
    unsigned short GetHeight() const { return GetSize().y; }
    MapExtent GetSize() const { return size_; }
    /// Returns a MapPoint from a point. This ensures, the coords are actually in the map [0, mapSize)
    MapPoint MakeMapPoint(Position pt) const;
    /// Returns the linear index for a map point
    unsigned GetIdx(MapPoint pt) const;

    /// Get coordinates of neighbor in the given direction
    MapPoint GetNeighbour(MapPoint pt, Direction dir) const;
    /// Return neighboring point (2nd layer: dir 0-11)
    MapPoint GetNeighbour2(MapPoint, unsigned dir) const;
    // Convenience functions for the above function
    MapCoord GetXA(MapPoint pt, Direction dir) const;
    // Gets all neighbors (in all directions) for given position
    helpers::EnumArray<MapPoint, Direction> GetNeighbours(MapPoint pt) const;

    /// Return all points in a radius around pt (excluding pt) that satisfy a given condition.
    /// Points can be transformed (e.g. to flags at those points) by the functor taking a map point and a radius
    /// Number of results is constrained to maxResults (if > 0)
    template<int T_maxResults = -1, class T_TransformPt = ReturnMapPoint, class T_IsValidPt = AlwaysTrue>
    detail::GetPointsResult_t<T_TransformPt>
    GetPointsInRadius(MapPoint pt, unsigned radius, T_TransformPt&& transformPt = T_TransformPt{},
                      T_IsValidPt&& isValid = T_IsValidPt{}, bool includePt = false) const;
    /// Return all points in the given radius that match the condition
    template<int T_maxResults = -1, class T_IsValidPt>
    std::vector<MapPoint> GetMatchingPointsInRadius(MapPoint pt, unsigned radius, T_IsValidPt&& isValid,
                                                    bool includePt = false) const
    {
        return GetPointsInRadius<T_maxResults>(pt, radius, ReturnMapPoint{}, std::forward<T_IsValidPt>(isValid),
                                               includePt);
    }
    /// Convenience method to return all points in the given radious including the center point
    std::vector<MapPoint> GetPointsInRadiusWithCenter(const MapPoint pt, unsigned radius) const
    {
        return GetPointsInRadius(pt, radius, ReturnMapPoint{}, AlwaysTrue{}, true);
    }
    /// Returns true, if the IsValid functor returns true for any point in the given radius
    /// If includePt is true, then the point itself is also checked
    template<class T_IsValidPt>
    bool CheckPointsInRadius(MapPoint pt, unsigned radius, T_IsValidPt&& isValid, bool includePt) const;

    /// Return the distance between 2 points on the map (includes wrapping around map borders)
    unsigned CalcDistance(const Position& p1, const Position& p2) const;
    unsigned CalcDistance(const MapPoint p1, const MapPoint p2) const
    {
        return CalcDistance(Position(p1), Position(p2));
    }
    unsigned CalcMaxDistance() const;
    /// Return the direction for ships for going from one point to another
    ShipDirection GetShipDir(MapPoint fromPt, MapPoint toPt) const;
};

//////////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////////

inline MapCoord MapBase::GetXA(const MapPoint pt, Direction dir) const
{
    return GetNeighbour(pt, dir).x;
}

inline unsigned MapBase::GetIdx(const MapPoint pt) const
{
    RTTR_Assert(pt.x < size_.x && pt.y < size_.y);
    return static_cast<unsigned>(pt.y) * size_.x + pt.x;
}

template<int T_maxResults, class T_TransformPt, class T_IsValidPt>
detail::GetPointsResult_t<T_TransformPt> MapBase::GetPointsInRadius(const MapPoint pt, unsigned radius,
                                                                    T_TransformPt&& transformPt, T_IsValidPt&& isValid,
                                                                    bool includePt) const
{
    ::detail::GetPointsResult_t<T_TransformPt> result;
    if(T_maxResults > 0)
        result.reserve(T_maxResults);
    else if(std::is_same<T_IsValidPt, AlwaysTrue>::value)
    {
        // For every additional radius we get 6 * curRadius more points. Hence we have 6 * sum(1..radius) points + the
        // center point if requested This can be reduced via the gauss formula to the following:
        result.reserve((radius * radius + radius) * 3u + (includePt ? 1u : 0u));
    }
    if(includePt)
    {
        const auto el = transformPt(pt, 0);
        if(isValid(el))
        {
            result.push_back(el);
            if(T_maxResults == 1)
                return result;
        }
    }
    MapPoint curStartPt = pt;
    for(unsigned r = 1; r <= radius; ++r)
    {
        // Go one level/hull to the left
        curStartPt = GetNeighbour(curStartPt, Direction::West);
        // Now iterate over the "circle" of radius r by going r steps in one direction, turn right and repeat
        MapPoint curPt = curStartPt;
        for(const auto dir : helpers::enumRange(Direction::NorthEast))
        {
            for(unsigned step = 0; step < r; ++step)
            {
                const auto el = transformPt(curPt, r);
                if(isValid(el))
                {
                    result.push_back(el);
                    if(T_maxResults > 0 && static_cast<int>(result.size()) >= T_maxResults)
                        return result;
                }
                curPt = GetNeighbour(curPt, dir);
            }
        }
    }
    return result;
}

template<class T_IsValidPt>
inline bool MapBase::CheckPointsInRadius(const MapPoint pt, unsigned radius, T_IsValidPt&& isValid,
                                         bool includePt) const
{
    if(includePt && isValid(pt, 0))
        return true;
    MapPoint curStartPt = pt;
    for(unsigned r = 1; r <= radius; ++r)
    {
        // Go one level/hull to the left
        curStartPt = GetNeighbour(curStartPt, Direction::West);
        // Now iterate over the "circle" of radius r by going r steps in one direction, turn right and repeat
        MapPoint curPt = curStartPt;
        for(const auto dir : helpers::enumRange(Direction::NorthEast))
        {
            for(unsigned step = 0; step < r; ++step)
            {
                if(isValid(curPt, r))
                    return true;
                curPt = GetNeighbour(curPt, dir);
            }
        }
    }
    return false;
}
