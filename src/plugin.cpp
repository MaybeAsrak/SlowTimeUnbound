

#include "include\PerkEntryPointExtenderAPI.h"

struct Hooks {
    struct SetMagicTimeSlowdownStart {
        static void thunk(RE::VATS* a_vats, float a_worldMagnitude, float a_playerMagnitude) {
            auto magicTimeSlowdown = a_vats->magicTimeSlowdown;
            auto playermagicTimeSlowdown = a_vats->playerMagicTimeSlowdown;
            if (magicTimeSlowdown == 0) {
                magicTimeSlowdown = 1.0f;
            }
            if (playermagicTimeSlowdown == 0) {
                playermagicTimeSlowdown = 1.0f;
            }
            if (a_worldMagnitude < 0) {
                    func(a_vats, magicTimeSlowdown, a_worldMagnitude * -1 * playermagicTimeSlowdown);
            }
            else {
                    func(a_vats, magicTimeSlowdown * a_worldMagnitude, playermagicTimeSlowdown);   
            }
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

        struct GetActiveEffectMagnitude {
        static float thunk(RE::ActiveEffect* a_AE) {
            float timescale;
            const auto magicTime = RE::VATS::GetSingleton();
            float playertimescale = magicTime->playerMagicTimeSlowdown;
            if (playertimescale == 0.0f) {
                playertimescale = 1.0f;
            }
           if (a_AE->effect->baseEffect->IsHostile()) {
           timescale = a_AE->GetMagnitude()*-1;          
           a_AE->duration = a_AE->duration * timescale * -1 * playertimescale;
           }
           else {
           timescale = a_AE->GetMagnitude();
           a_AE->duration = a_AE->duration * playertimescale;
           }
           return timescale;
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct SlowTimeStop {
        static void thunk(RE::SlowTimeEffect* a_SlowTime) {
           const auto magicTime = RE::VATS::GetSingleton();
           if (magicTime) {
           
           auto magicTimeSlowdown = magicTime->magicTimeSlowdown;
           auto playermagicTimeSlowdown = magicTime->playerMagicTimeSlowdown;
           float timescale;

           float newWorldTimeScale;
           float newPlayerTimeScale;

           if (a_SlowTime->effect->baseEffect->IsHostile()) {
               timescale = a_SlowTime->GetMagnitude();

               newWorldTimeScale = magicTimeSlowdown;
               newPlayerTimeScale = playermagicTimeSlowdown / timescale;
           } else {
               timescale = a_SlowTime->GetMagnitude();

               newWorldTimeScale = magicTimeSlowdown / timescale;
               newPlayerTimeScale = playermagicTimeSlowdown;
           }

           if (
               abs(newWorldTimeScale - 1.0f) < 0.001
               && abs(newPlayerTimeScale - 1.0f) < 0.001
           ) {
               // If there is no slow time effect, world time scale must be 0,
               // otherwise horses will break.
               newWorldTimeScale = 0.0f;
               newPlayerTimeScale = 1.0f;
           }

           magicTime->SetMagicTimeSlowdown(newWorldTimeScale, newPlayerTimeScale);

           func(a_SlowTime);
        }
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct NullMagicTimeSlowdownStop {
        static void thunk(RE::VATS* a_vats, float a_worldMagnitude, float a_playerMagnitude) {
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct NullGetCurrentMultiplier {
        static float thunk(RE::VATS* a_vats) {
           float test = func(a_vats);
           return 1.0f;
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    // struct PlayerCharacter_Update {
    //     static void thunk(RE::PlayerCharacter* a_player, float a_delta) {
    //        func(a_player, a_delta);
    //    }
    //    static inline REL::Relocation<decltype(thunk)> func;
    //};

    static void Install() {
        // stl::write_vfunc<RE::PlayerCharacter, 0xAD, PlayerCharacter_Update>();

        REL::Relocation<std::uintptr_t> checkslowtime{RELOCATION_ID(529316, 34972), REL::Relocate(0x0, 0x0)};
        REL::safe_fill(checkslowtime.address(), REL::NOP, 0x13);

        stl::write_vfunc<RE::SlowTimeEffect, 0x13, SlowTimeStop>();


        REL::Relocation<std::uintptr_t> FunctionNullGetCurrentMultiplier{RELOCATION_ID(34175, 34968),
                                                                          REL::Relocate(0x6E, 0x6E)};
        stl::write_thunk_call<NullGetCurrentMultiplier>(FunctionNullGetCurrentMultiplier.address());
        

        REL::Relocation<std::uintptr_t> functionNullMagicTimeSlowdownStop{RELOCATION_ID(34177, 34970),
                                                                          REL::Relocate(0x35, 0x35)};
        stl::write_thunk_call<NullMagicTimeSlowdownStop>(functionNullMagicTimeSlowdownStop.address());


         REL::Relocation<std::uintptr_t> functionSetMagicTimeSlowdownStart{RELOCATION_ID(34175, 34968), REL::Relocate(0x5D, 0x5D)};
         stl::write_thunk_call<SetMagicTimeSlowdownStart>(functionSetMagicTimeSlowdownStart.address());

         REL::Relocation<std::uintptr_t> functionGetActiveEffectMagnitude{RELOCATION_ID(34175, 34968), REL::Relocate(0x4C, 0x4C)};
         stl::write_thunk_call<GetActiveEffectMagnitude>(functionGetActiveEffectMagnitude.address());


    }
};

SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
    Hooks::Install();
    return true;
}