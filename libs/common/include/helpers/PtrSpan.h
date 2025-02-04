// Copyright (c) 2021 - 2021 Settlers Freaks (sf-team at siedler25.org)
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

#include <boost/config.hpp>
#include <iterator>

// Using BOOST_FORCEINLINE mostly for increased debug performance (doesn't work for MSVC though)
#define RTTR_CONSTEXPR_INLINE constexpr BOOST_FORCEINLINE

namespace helpers {
namespace detail {
    template<class T>
    struct UnwrapSmartPointer
    {
        using type = std::remove_pointer_t<typename T::pointer>;
    };
    template<class T>
    struct UnwrapSmartPointer<T*>
    {
        using type = T;
    };
} // namespace detail

/// View type (access to current underlying container) which provides iterable access to a range of pointer-like
/// elements Those can be raw pointers or smart pointers and are assumed to be non-NULL Iteration yields references to
/// the pointed-to type
template<class TRange>
class NonNullPtrSpan
{
    const TRange& range;

    using RangeIterator = decltype(range.begin());
    class Iterator
    {
        RangeIterator it;

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = typename detail::UnwrapSmartPointer<typename TRange::value_type>::type;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        RTTR_CONSTEXPR_INLINE explicit Iterator(RangeIterator it) : it(std::move(it)) {}
        RTTR_CONSTEXPR_INLINE bool operator==(const Iterator& other) const { return it == other.it; }
        RTTR_CONSTEXPR_INLINE bool operator!=(const Iterator& other) const { return it != other.it; }
        RTTR_CONSTEXPR_INLINE reference operator*() const { return **it; }
        RTTR_CONSTEXPR_INLINE pointer operator->() const { return &(**it); }
        RTTR_CONSTEXPR_INLINE Iterator& operator++()
        {
            ++it;
            return *this;
        }
        RTTR_CONSTEXPR_INLINE Iterator operator++(int) { return Iterator(it++); }
        RTTR_CONSTEXPR_INLINE Iterator& operator--()
        {
            --it;
            return *this;
        }
        RTTR_CONSTEXPR_INLINE Iterator operator--(int) { return Iterator(it--); }
    };

public:
    constexpr NonNullPtrSpan(const TRange& range) : range(range) {}
    NonNullPtrSpan(TRange&&) = delete; // Avoid accidentally using temporary ranges
    RTTR_CONSTEXPR_INLINE Iterator begin() const { return Iterator(range.begin()); }
    RTTR_CONSTEXPR_INLINE Iterator end() const { return Iterator(range.end()); }
    RTTR_CONSTEXPR_INLINE typename Iterator::reference front() const { return **range.begin(); }
    RTTR_CONSTEXPR_INLINE bool empty() const { return range.empty(); }
    RTTR_CONSTEXPR_INLINE auto size() const { return range.size(); }
};

template<class TRange>
constexpr auto nonNullPtrSpan(const TRange& range)
{
    return NonNullPtrSpan<TRange>(range);
}

#undef RTTR_CONSTEXPR_INLINE

} // namespace helpers
