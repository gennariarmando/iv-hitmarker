#include "plugin.h"
#include "common.h"
#include "Rage.h"
#include "CTimer.h"
#include "T_CB_Generic.h"
#include "CTxdStore.h"
#include "CWeaponInfo.h"
#include "CHud.h"
#include "CHudColours.h"
#include "CTaskSimpleAimGun.h"
#include "CCutsceneMgr.h"

#include "Utility.h"

using namespace plugin;

class HitMarkerIV {
public:
    static inline CSprite2d hitmarkerSprite = {};
    static inline uint32_t targetDisplayTime = 0;
    static inline CPed* targetPed = nullptr;
    static inline bool targetHeadShot = false;

    HitMarkerIV() {
        plugin::Events::initEngineEvent += []() {
            int32_t slot = CTxdStore::AddTxdSlot("hitmarker");
            CTxdStore::LoadTxd(slot, "platform:/textures/hitmarker");
            CTxdStore::AddRef(slot);
            CTxdStore::PushCurrentTxd();
            CTxdStore::SetCurrentTxd(slot);
        
            hitmarkerSprite.SetTexture("hitmarker");
            CTxdStore::PopCurrentTxd();
        };
        
        plugin::Events::shutdownEngineEvent += []() {
            hitmarkerSprite.Delete();

            int32_t slot = CTxdStore::FindTxdSlot("hitmarker");
            CTxdStore::RemoveTxdSlot(slot);
        };

        plugin::Events::drawHudEvent += []() {
            if (CCutsceneMgr::IsRunning())
                return;

            auto base = new T_CB_Generic_NoArgs([]() {
                auto playa = FindPlayerPed(0);
                if (!playa)
                    return;

                bool isDotShowing = (CHud::Components[aHudComponentInfo[HUD_WEAPON_DOT].m_nIndex]->IsDisplaying()) || (CHud::Components[aHudComponentInfo[HUD_WEAPON_SCOPE].m_nIndex]->IsDisplaying());
                if (!isDotShowing)
                    return;

                CTaskSimpleAimGun* taskSimpleAimGun = playa->m_pPedIntelligence->m_TaskMgr.FindActiveTaskByType<CTaskSimpleAimGun>(6);

                if (taskSimpleAimGun) {
                    if (targetPed && CPed::IsPedDead(targetPed)) {
                        targetHeadShot = targetPed->m_bKilledByHeadshot;
                        targetDisplayTime = CTimer::GetTimeInMilliseconds() + 500;
                        targetPed = nullptr;
                    }

                    if (targetDisplayTime > CTimer::GetTimeInMilliseconds()) {
                        rage::Color32 col = CHudColours::Get(HUD_COLOUR_WHITE, 255);

                        if (targetHeadShot)
                            col = CHudColours::Get(HUD_COLOUR_REDDARK, 255);

                        float x = (SCREEN_WIDTH / 2);
                        float y = (SCREEN_HEIGHT / 2);
                        float w = ScaleX(5.0f);
                        float h = ScaleY(5.0f);
                        if (hitmarkerSprite.m_pTexture) {
                            hitmarkerSprite.Push();
                            CSprite2d::Draw(rage::Vector4(x - w, y - h, x + w, y + h), col);
                            CSprite2d::Pop();
                        }
                    }
                    else {
                        CPed* ped = dynamic_cast<CPed*>(taskSimpleAimGun->GetAt(0, 0));
                        if (ped && targetPed != ped && !CPed::IsPedDead(ped))
                            targetPed = ped;
                    }
                }
                else {
                    targetHeadShot = false;
                    targetDisplayTime = 0;
                    targetPed = nullptr;
                }
                });
            base->Append();
        };
    }
} hitMarkerIV;
