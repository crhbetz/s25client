// Copyright (c) 2005 - 2020 Settlers Freaks (sf-team at siedler25.org)
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

#include "nofArmorer.h"
#include "EventManager.h"
#include "GamePlayer.h"
#include "GlobalGameSettings.h"
#include "Loader.h"
#include "SerializedGameData.h"
#include "SoundManager.h"
#include "addons/const_addons.h"
#include "buildings/nobUsual.h"
#include "network/GameClient.h"
#include "ogl/glArchivItem_Bitmap_Player.h"
#include "world/GameWorld.h"
#include "gameData/JobConsts.h"
#include "gameData/ShieldConsts.h"

nofArmorer::nofArmorer(const MapPoint pos, const unsigned char player, nobUsual* workplace)
    : nofWorkman(Job::Armorer, pos, player, workplace), sword_shield(false)
{}

void nofArmorer::Serialize(SerializedGameData& sgd) const
{
    nofWorkman::Serialize(sgd);

    sgd.PushBool(sword_shield);
}

nofArmorer::nofArmorer(SerializedGameData& sgd, const unsigned obj_id)
    : nofWorkman(sgd, obj_id), sword_shield(sgd.PopBool())
{}

void nofArmorer::DrawWorking(DrawPoint drawPt)
{
    constexpr helpers::EnumArray<DrawPoint, Nation> offsets = {{{-10, 15}, {-11, 9}, {-14, 16}, {-19, 1}, {-11, 9}}};

    constexpr unsigned max_id = 280;
    const unsigned now_id = GAMECLIENT.Interpolate(max_id, current_ev);
    if(now_id < 200)
    {
        const Nation wpNation = workplace->GetNation();
        const unsigned plColor = world->GetPlayer(player).color;

        LOADER.GetPlayerImage("rom_bobs", 16 + (now_id % 8))
          ->DrawFull(drawPt + offsets[wpNation], COLOR_WHITE, plColor);

        if((now_id % 8) == 5)
        {
            world->GetSoundMgr().playNOSound(52, *this, now_id / 8);
            was_sounding = true;
        }
    }
}

unsigned short nofArmorer::GetCarryID() const
{
    if(sword_shield)
        return 56;
    else
    {
        // Je nach Nation einen bestimmtem Schild fertigen
        switch(world->GetPlayer(player).nation)
        {
            case Nation::Africans: return 60; //-V525
            case Nation::Japanese: return 58;
            case Nation::Romans: return 57;
            case Nation::Vikings: return 59;
            case Nation::Babylonians: return 58; // babylonians use japanese shield carry-animation
        }
        RTTR_Assert(false);
        return 0;
    }
}

void nofArmorer::HandleDerivedEvent(const unsigned /*id*/)
{
    switch(state)
    {
        case State::Waiting1:
        {
            if(!world->GetGGS().isEnabled(AddonId::HALF_COST_MIL_EQUIP) || !sword_shield)
            {
                // LOG.write(("armorer handlewait1 - consume wares %i \n",player);
                nofWorkman::HandleStateWaiting1();
            } else
            {
                // Nach 1. Warten wird gearbeitet
                current_ev = GetEvMgr().AddEvent(this, JOB_CONSTS[job_].work_length, 1);
                state = State::Work;
                workplace->is_working = true;
                // LOG.write(("armorer handlewait1 - no consume wares %i \n",player);
            }
        }
        break;
        case State::Work:
        {
            HandleStateWork();
        }
        break;
        case State::Waiting2:
        {
            HandleStateWaiting2();
        }
        break;
        default: break;
    }
}

bool nofArmorer::AreWaresAvailable() const
{
    return workplace->WaresAvailable() || (world->GetGGS().isEnabled(AddonId::HALF_COST_MIL_EQUIP) && sword_shield);
}

helpers::OptionalEnum<GoodType> nofArmorer::ProduceWare()
{
    sword_shield = !sword_shield;

    if(sword_shield)
        return GoodType::Sword;
    else
        return SHIELD_TYPES[world->GetPlayer(player).nation];
}
