// Copyright (c) 2005 - 2015 Settlers Freaks (sf-team at siedler25.org)
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

#include "mapGenerator/MapGenerator.h"
#include "mapGenerator/MapUtility.h"
#include "mapGenerator/RandomIslandsGenerator.h"
#include "mapGenerator/RandomGreenlandGenerator.h"
#include "mapGenerator/RandomRiverlandGenerator.h"
#include "mapGenerator/RandomMigrationGenerator.h"
#include "mapGenerator/RandomContinentGenerator.h"
#include "mapGenerator/RandomRinglandGenerator.h"
#include "mapGenerator/RandomMapGenerator.h"

#include "libsiedler2/src/libsiedler2.h"
#include "helpers/Deleter.h"

#include <boost/interprocess/smart_ptr/unique_ptr.hpp>
#include <stdexcept>

typedef boost::interprocess::unique_ptr<RandomMapGenerator, Deleter<RandomMapGenerator> > GeneratorPtr;

void MapGenerator::Create(const std::string& filePath, const MapSettings& settings)
{
    GeneratorPtr generator;

    // create a random map generator based on the map style
    switch (settings.style)
    {
        case MS_Greenland:
            generator = GeneratorPtr(new RandomGreenlandGenerator);
            break;
        case MS_Riverland:
            generator = GeneratorPtr(new RandomRiverlandGenerator);
            break;
        case MS_Islands:
            generator = GeneratorPtr(new RandomIslandsGenerator);
            break;
        case MS_Continent:
            generator = GeneratorPtr(new RandomContinentGenerator);
            break;
        case MS_Migration:
            generator = GeneratorPtr(new RandomMigrationGenerator);
            break;
        case MS_Ringland:
            generator = GeneratorPtr(new RandomRinglandGenerator);
            break;
        case MS_Random:
            generator = GeneratorPtr(new RandomMapGenerator);
            break;
    }
    
    if (generator.get() == NULL)
    {
        throw new std::invalid_argument("Style not supported");
    }

    // generate the random map
    Map* map = generator->Create(settings);
    libsiedler2::ArchivInfo* archiv = map->CreateArchiv();
    libsiedler2::Write(filePath, *archiv);
    
    // cleanup map and writer
    delete map;
    delete archiv;
}
