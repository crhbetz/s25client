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

#include "nofPassiveSoldier.h"
#include "EventManager.h"
#include "GamePlayer.h"
#include "SerializedGameData.h"
#include "buildings/nobMilitary.h"
#include "helpers/containerUtils.h"
#include "helpers/pointerContainerUtils.h"
#include "random/Random.h"
#include "world/GameWorld.h"
#include "gameData/MilitaryConsts.h"

nofPassiveSoldier::nofPassiveSoldier(const nofSoldier& soldier) : nofSoldier(soldier), healing_event(nullptr)
{
    // Soldat von einer Mission nach Hause gekommen --> ggf heilen!
    Heal();
    // Laufevent nullen, laufen ja nicht mehr
    current_ev = nullptr;
}

nofPassiveSoldier::nofPassiveSoldier(const MapPoint pos, const unsigned char player, nobBaseMilitary* const goal,
                                     nobMilitary* const home, const unsigned char rank)
    : nofSoldier(pos, player, goal, home, rank), healing_event(nullptr)
{}

nofPassiveSoldier::~nofPassiveSoldier() = default;

void nofPassiveSoldier::Destroy()
{
    GetEvMgr().RemoveEvent(healing_event);
    nofSoldier::Destroy();
}

void nofPassiveSoldier::Serialize(SerializedGameData& sgd) const
{
    nofSoldier::Serialize(sgd);

    sgd.PushEvent(healing_event);
}

nofPassiveSoldier::nofPassiveSoldier(SerializedGameData& sgd, const unsigned obj_id)
    : nofSoldier(sgd, obj_id), healing_event(sgd.PopEvent())
{}

void nofPassiveSoldier::Draw(DrawPoint drawPt)
{
    // Soldat normal laufend zeichnen
    DrawWalkingBobJobs(drawPt, job_);
}

void nofPassiveSoldier::HandleDerivedEvent(const unsigned id)
{
    switch(id)
    {
        // "Heilungs-Event"
        case 1:
        {
            healing_event = nullptr;

            // Sind wir noch im Haus?
            if(fs == FigureState::Job)
            {
                // Dann uns heilen, wenn wir nicht schon gesund sind
                if(hitpoints < HITPOINTS[GetRank()])
                {
                    ++hitpoints;

                    // Sind wir immer noch nicht gesund? Dann neues Event anmelden!
                    if(hitpoints < HITPOINTS[GetRank()])
                        healing_event =
                          GetEvMgr().AddEvent(this, CONVALESCE_TIME + RANDOM_RAND(CONVALESCE_TIME_RANDOM), 1);
                }
            }
        }
        break;
    }
}

void nofPassiveSoldier::Heal()
{
    // Schon ein Event angemeldet?
    // Dann muss dieses entfernt werden, wahrscheinlich war er zuvor draußen gewesen
    if(healing_event)
    {
        GetEvMgr().RemoveEvent(healing_event);
        healing_event = nullptr;
    }

    // Ist er verletzt?
    // Dann muss er geheilt werden
    if(hitpoints < HITPOINTS[GetRank()])
        healing_event = GetEvMgr().AddEvent(this, CONVALESCE_TIME + RANDOM_RAND(CONVALESCE_TIME_RANDOM), 1);
}

void nofPassiveSoldier::GoalReached()
{
    static_cast<nobMilitary*>(building)->AddPassiveSoldier(world->RemoveFigure(pos, *this));
}

void nofPassiveSoldier::LeaveBuilding()
{
    // Nach Hause in ein Lagerhaus gehen
    rs_dir = true;
    rs_pos = 1;
    cur_rs = building->GetRoute(Direction::SouthEast);
    GoHome();

    building = nullptr;
}

void nofPassiveSoldier::Upgrade()
{
    // We must not be in the buildings list while upgrading. This would destroy the ordered list
    RTTR_Assert(!building || !static_cast<nobMilitary*>(building)->IsInTroops(*this));
    // Einen Rang höher
    job_ = Job(unsigned(job_) + 1);

    // wieder heilen bzw. Hitpoints anpasen
    GamePlayer& owner = world->GetPlayer(player);
    hitpoints = HITPOINTS[GetRank()];

    // Inventur entsprechend erhöhen und verringern
    owner.IncreaseInventoryJob(job_, 1);
    owner.DecreaseInventoryJob(Job(unsigned(job_) - 1), 1);
}

void nofPassiveSoldier::Walked()
{
    throw std::logic_error("Passive soldiers shall not walk");
}

void nofPassiveSoldier::NotNeeded()
{
    building = nullptr;
    GoHome();
}

nobMilitary* nofPassiveSoldier::getHome() const
{
    return checkedCast<nobMilitary*>(building);
}
