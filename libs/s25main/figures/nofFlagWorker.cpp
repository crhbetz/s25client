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

#include "nofFlagWorker.h"
#include "FindWhConditions.h"
#include "GamePlayer.h"
#include "SerializedGameData.h"
#include "buildings/nobBaseWarehouse.h"
#include "world/GameWorld.h"
#include "nodeObjs/noFlag.h"

nofFlagWorker::nofFlagWorker(const Job job, const MapPoint pos, const unsigned char player, noRoadNode* goal)
    : noFigure(job, pos, player, goal), flag(nullptr), state(State::FigureWork)
{
    // Flagge als Ziel, dann arbeiten wir auch, ansonsten kanns aber auch nur ein Lagerhaus oder Null sein, wenn ein
    // Lagerhaus abgerissen wurde oder ausgelagert wurde etc., dann auch den nicht als Flag-Worker registrieren
    if(goal)
    {
        if(goal->GetGOT() == GO_Type::Flag)
        {
            this->flag = static_cast<noFlag*>(goal);
            world->GetPlayer(player).RegisterFlagWorker(this);
        } else
            this->flag = nullptr;
    } else
        this->flag = nullptr;
}

nofFlagWorker::nofFlagWorker(SerializedGameData& sgd, const unsigned obj_id)
    : noFigure(sgd, obj_id), flag(sgd.PopObject<noFlag>(GO_Type::Flag)), state(sgd.Pop<State>())
{}

void nofFlagWorker::Serialize(SerializedGameData& sgd) const
{
    noFigure::Serialize(sgd);

    sgd.PushObject(flag, true);
    sgd.PushEnum<uint8_t>(state);
}

void nofFlagWorker::Destroy()
{
    RTTR_Assert(!flag);
    RTTR_Assert(!world->GetPlayer(player).IsFlagWorker(this));
    noFigure::Destroy();
}

void nofFlagWorker::AbrogateWorkplace()
{
    if(flag)
    {
        /// uns entfernen, da wir wieder umdrehen müssen
        world->GetPlayer(player).RemoveFlagWorker(this);
        flag = nullptr;
    } else
        RTTR_Assert(!world->GetPlayer(player).IsFlagWorker(this));
}

/// Geht wieder zurück zur Flagge und dann nach Hause
void nofFlagWorker::GoToFlag()
{
    // Zur Flagge zurücklaufen

    // Bin ich an der Fahne?
    if(pos == flag->GetPos())
    {
        // nach Hause gehen
        nobBaseWarehouse* wh = world->GetPlayer(player).FindWarehouse(*flag, FW::AcceptsFigure(job_), true, false);
        if(wh)
        {
            GoHome(wh);
            // Vorgaukeln, dass wir ein Stück Straße bereits geschafft haben
            // damit wir mit WalkToGoal weiter bis zum Ziel laufen können
            cur_rs = &emulated_wanderroad;
            rs_pos = 0;
            WalkToGoal();
        } else
        {
            // Weg führt nicht mehr zum Lagerhaus, dann rumirren
            StartWandering();
            Wander();
        }

        // Da wir quasi "freiwillig" nach Hause gegangen sind ohne das Abreißen der Flagge, auch manuell wieder
        // "abmelden"
        world->GetPlayer(player).RemoveFlagWorker(this);
        state = State::FigureWork;
        flag = nullptr;
    } else
    {
        // Weg suchen
        const auto dir = world->FindHumanPath(pos, flag->GetPos(), 40);

        // Wenns keinen gibt, rumirren, ansonsten hinlaufen
        if(dir)
            StartWalking(*dir);
        else
        {
            Abrogate();
            StartWandering();
            Wander();
            state = State::FigureWork;

            flag = nullptr;
        }
    }
}
