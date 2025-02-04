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

#include <boost/filesystem/path.hpp>

uint32_t CalcChecksumOfFile(const boost::filesystem::path& path);
uint32_t CalcChecksumOfBuffer(const uint8_t* buffer, size_t size);

inline uint32_t CalcChecksumOfBuffer(const char* buffer, size_t size)
{
    return CalcChecksumOfBuffer(reinterpret_cast<const uint8_t*>(buffer), size);
}

template<typename T>
inline uint32_t CalcChecksumOfBuffer(const T& buffer)
{
    return CalcChecksumOfBuffer(buffer.data(), buffer.size());
}