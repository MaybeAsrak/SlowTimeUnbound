

#include "include\PerkEntryPointExtenderAPI.h"
class Serialization {
public:
    Serialization() = default;
    Serialization(const Serialization&) = delete;
    Serialization(Serialization&&) = delete;

    ~Serialization() = default;

    Serialization& operator=(const Serialization&) = delete;
    Serialization& operator=(Serialization&&) = delete;

    mutable std::mutex lock;

    std::unordered_map<int, float> collection;
    std::vector<RE::ActiveEffect*> LastCastEffect;

    static enum : std::uint32_t { kModID = 'STUB', kVersion = 1, kCollection = 'COLL' };
    static auto GetSingleton() -> Serialization* {
        static Serialization singleton;
        return std::addressof(singleton);
    }

    static void OnGameLoaded(SKSE::SerializationInterface* a_interface) {
        std::uint32_t type;
        std::uint32_t version;
        std::uint32_t length;

        while (a_interface->GetNextRecordInfo(type, version, length)) {
            if (version != kVersion) {
                bool failedversion = 1;
                continue;
            }

            switch (type) {
                case kCollection:
                    std::size_t count;
                    a_interface->ReadRecordData(&count, sizeof(count));

                    for (std::size_t i = 0; i < count; i++) {
                        int value;

                        float global;

                        a_interface->ReadRecordData(&value, sizeof(value));
                        a_interface->ReadRecordData(&global, sizeof(global));

                        Serialization::GetSingleton()->collection[value] = global;
                    }
                    break;
            }
        }
    }

    static void OnGameSaved(SKSE::SerializationInterface* a_interface) {
        if (!a_interface->OpenRecord(kCollection, kVersion)) {
            return;
        }

        std::unique_lock lock(Serialization::GetSingleton()->lock);

        const size_t size = Serialization::GetSingleton()->collection.size();

        if (!a_interface->WriteRecordData(&size, sizeof(size))) {
            return;
        }

        for (const auto& element : Serialization::GetSingleton()->collection) {
            const auto value = element.first;
            const auto global = element.second;

            if (!a_interface->WriteRecordData(&value, sizeof(value))) {
                return;
            }

            if (!a_interface->WriteRecordData(&global, sizeof(global))) {
                return;
            }
        }
    }

    static void OnRevert(SKSE::SerializationInterface*) {
        std::unique_lock lock(Serialization::GetSingleton()->lock);
        Serialization::GetSingleton()->collection[1] = 0.0f;
        Serialization::GetSingleton()->collection[2] = 1.0f;
        Serialization::GetSingleton()->collection[3] = 1.0f;
        Serialization::GetSingleton()->collection[4] = 1.0f;
        Serialization::GetSingleton()->collection[5] = 1.0f;
        Serialization::GetSingleton()->collection[6] = 0.0f;
        Serialization::GetSingleton()->collection[7] = 1.0f;
        Serialization::GetSingleton()->collection[8] = 1.0f;
        Serialization::GetSingleton()->collection[9] = 1.0f;
        Serialization::GetSingleton()->collection[10] = 1.0f;
    }

    static void CreateGlobalVariables() {
        if (Serialization::GetSingleton()->collection.empty()) {
            Serialization::GetSingleton()->collection.try_emplace(1, 0.0f);
            Serialization::GetSingleton()->collection.try_emplace(2, 1.0f);
            Serialization::GetSingleton()->collection.try_emplace(3, 1.0f);
            Serialization::GetSingleton()->collection.try_emplace(4, 1.0f);
            Serialization::GetSingleton()->collection.try_emplace(5, 1.0f);
            Serialization::GetSingleton()->collection.try_emplace(6, 0.0f);
            Serialization::GetSingleton()->collection.try_emplace(7, 1.0f);
            Serialization::GetSingleton()->collection.try_emplace(8, 1.0f);
            Serialization::GetSingleton()->collection.try_emplace(9, 1.0f);
            Serialization::GetSingleton()->collection.try_emplace(10, 1.0f);
        }
    }

    static auto GetGlobalValue(int a_value) -> float { return Serialization::GetSingleton()->collection[a_value]; }

    static void SetGlobalValue(int a_value, float a_amount) {
        Serialization::GetSingleton()->collection[a_value] = (a_amount);
    }

    static void MultGlobalValue(int a_value, float a_amount) {
        Serialization::GetSingleton()->collection[a_value] =
            Serialization::GetSingleton()->collection[a_value] * (a_amount);
    }

    static void DivideGlobalValue(int a_value, float a_amount) {
        Serialization::GetSingleton()->collection[a_value] =
            Serialization::GetSingleton()->collection[a_value] / (a_amount);
    }

    static void AddGlobalValue(int a_value, float a_amount) {
        Serialization::GetSingleton()->collection[a_value] =
            (Serialization::GetSingleton()->collection[a_value] + (a_amount));
    }
};

struct Hooks {
    struct SetMagicTimeSlowdownStart {
        static void thunk(RE::VATS* a_vats, float a_worldMagnitude, float a_playerMagnitude) {
            if (a_worldMagnitude < 0) {
                Serialization::MultGlobalValue(2, -1 * a_worldMagnitude);
            } else {
                Serialization::MultGlobalValue(3, a_worldMagnitude);
            }
            Serialization::AddGlobalValue(1, 1.0f);
            Serialization::SetGlobalValue(4, a_playerMagnitude);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct GetActiveEffectMagnitude {
        static float thunk(RE::ActiveEffect* a_AE) {
            float timescale;
            Serialization::GetSingleton()->LastCastEffect.push_back(a_AE);
            if (a_AE->effect->baseEffect->data.flags.any(RE::EffectSetting::EffectSettingData::Flag::kPainless) ==
                true) {
                timescale = a_AE->GetMagnitude() * -1;
            } else {
                timescale = a_AE->GetMagnitude();
            }
            return timescale;
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct SlowTimeStop {
        static void thunk(RE::SlowTimeEffect* a_SlowTime) {
            float timescale = a_SlowTime->GetMagnitude();
            if (a_SlowTime->effect->baseEffect->data.flags.any(RE::EffectSetting::EffectSettingData::Flag::kPainless)) {
                Serialization::DivideGlobalValue(2, timescale);
            } else {
                Serialization::DivideGlobalValue(3, timescale);
            }
            Serialization::AddGlobalValue(1, -1.0f);
            func(a_SlowTime);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct NullMagicTimeSlowdownStop {
        static void thunk(RE::VATS* a_vats, float a_worldMagnitude, float a_playerMagnitude) {}
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct NullGetCurrentMultiplier {
        static float thunk(RE::VATS* a_vats) {
            float test = func(a_vats);
            return 1.0f;
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct PlayerCharacter_Update {
        static void thunk(RE::PlayerCharacter* a_player, float a_delta) {
            auto vatscontroller = RE::VATS::GetSingleton();
            if (vatscontroller) {
                float globaltimemult = Serialization::GetGlobalValue(3);
                float playertimemult = Serialization::GetGlobalValue(2);
                float activetimespells = Serialization::GetGlobalValue(1);
                float activebowzoom = Serialization::GetGlobalValue(6);
                float playertimemultBase = Serialization::GetGlobalValue(4);
                float globaltimemultBow = Serialization::GetGlobalValue(5);
                float playertimemultBow = Serialization::GetGlobalValue(7);
                float previousGlobal = Serialization::GetGlobalValue(8);
                float previousPlayer = Serialization::GetGlobalValue(9);
                float finalGlobal = 1.0f;
                float finalPlayer = 1.0f;

                if (activetimespells > 0.1f) {
                    finalGlobal *= globaltimemult;
                    finalPlayer *= (playertimemult * playertimemultBase);
                }
                if (activebowzoom > 0.1f) {
                    finalGlobal *= globaltimemultBow;
                    RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_player, &playertimemultBow,
                                         "SlowTimeUnbound", 2, {});
                    finalPlayer *= playertimemultBow;
                }
                RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_player, &finalGlobal, "SlowTimeUnbound",
                                     3, {});
                RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_player, &finalPlayer, "SlowTimeUnbound",
                                     4, {});

                if (previousGlobal != finalGlobal || previousPlayer != finalPlayer) {
                    Serialization::SetGlobalValue(8, finalGlobal);
                    Serialization::SetGlobalValue(9, finalPlayer);
                    //char buffer[150];
                    //sprintf_s(buffer, "New Time Mult: Global %.00f; Player %.00f", finalGlobal*100, finalPlayer*100);
                    //RE::ConsoleLog::GetSingleton()->Print(buffer);

                    if (finalGlobal == 1.0f && finalPlayer == 1.0f) {
                        vatscontroller->SetMagicTimeSlowdown(0.0f, 0.0f);

                    } else {
                        vatscontroller->SetMagicTimeSlowdown(finalGlobal, finalPlayer);
                    }
                }
                if (Serialization::GetSingleton()->LastCastEffect.size() > 0) {
                    for (auto& element : Serialization::GetSingleton()->LastCastEffect) {
                        if (element) {
                            element->duration *= finalPlayer;
                        }
                    }
                    Serialization::GetSingleton()->LastCastEffect.clear();
                }
            }
            func(a_player, a_delta);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
    struct BowZoomStartSGTM {
        static void thunk(RE::BSTimer* a_timer, float a_multiplier, bool a_arg2) {
            float timefactorbow =
                RE::PlayerCharacter::GetSingleton()->AsActorValueOwner()->GetActorValue(RE::ActorValue::kBowSpeedBonus);
            Serialization::MultGlobalValue(5, timefactorbow);
            Serialization::MultGlobalValue(7, timefactorbow);
            Serialization::AddGlobalValue(6, 1.0f);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
    struct BowZoomStopSGTM {
        static void thunk(RE::BSTimer* a_timer, float a_multiplier, bool a_arg2) {
            float timefactorbow =
                RE::PlayerCharacter::GetSingleton()->AsActorValueOwner()->GetActorValue(RE::ActorValue::kBowSpeedBonus);

            if (Serialization::GetGlobalValue(6) > 0.1f) {
                Serialization::DivideGlobalValue(5, timefactorbow);
                Serialization::DivideGlobalValue(7, timefactorbow);
                Serialization::AddGlobalValue(6, -1.0f);
            }
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    static void Install() {
        stl::write_vfunc<RE::PlayerCharacter, 0xAD, PlayerCharacter_Update>();

        REL::Relocation<std::uintptr_t> checkslowtime{RELOCATION_ID(529316, 34972), REL::Relocate(0x0, 0x0)};
        REL::safe_fill(checkslowtime.address(), REL::NOP, 0x13);

        REL::Relocation<std::uintptr_t> BowZoomStartAddressCheck{RELOCATION_ID(41779, 42860),
                                                                 REL::Relocate(0x30, 0x30)};
        REL::safe_fill(BowZoomStartAddressCheck.address(), REL::NOP, 0x06);

        REL::Relocation<std::uintptr_t> BowZoomStopAddressCheck{RELOCATION_ID(41780, 42861), REL::Relocate(0xAC, 0xB4)};
        REL::safe_fill(BowZoomStopAddressCheck.address(), REL::NOP, 0x02);

        stl::write_vfunc<RE::SlowTimeEffect, 0x13, SlowTimeStop>();

        REL::Relocation<std::uintptr_t> FunctionNullGetCurrentMultiplier{RELOCATION_ID(34175, 34968),
                                                                         REL::Relocate(0x6E, 0x6E)};
        stl::write_thunk_call<NullGetCurrentMultiplier>(FunctionNullGetCurrentMultiplier.address());

        REL::Relocation<std::uintptr_t> functionNullMagicTimeSlowdownStop{RELOCATION_ID(34177, 34970),
                                                                          REL::Relocate(0x35, 0x35)};
        stl::write_thunk_call<NullMagicTimeSlowdownStop>(functionNullMagicTimeSlowdownStop.address());

        REL::Relocation<std::uintptr_t> functionSetMagicTimeSlowdownStart{RELOCATION_ID(34175, 34968),
                                                                          REL::Relocate(0x5D, 0x5D)};
        stl::write_thunk_call<SetMagicTimeSlowdownStart>(functionSetMagicTimeSlowdownStart.address());

        REL::Relocation<std::uintptr_t> functionGetActiveEffectMagnitude{RELOCATION_ID(34175, 34968),
                                                                         REL::Relocate(0x4C, 0x4C)};
        stl::write_thunk_call<GetActiveEffectMagnitude>(functionGetActiveEffectMagnitude.address());

        REL::Relocation<std::uintptr_t> FunctionBowZoomStartSGTM{RELOCATION_ID(41779, 42860),
                                                                 REL::Relocate(0x5A, 0x5A)};
        stl::write_thunk_call<BowZoomStartSGTM>(FunctionBowZoomStartSGTM.address());

        REL::Relocation<std::uintptr_t> FunctionBowZoomStopSGTM{RELOCATION_ID(41780, 42861), REL::Relocate(0xBD, 0xC5)};
        stl::write_thunk_call<BowZoomStopSGTM>(FunctionBowZoomStopSGTM.address());
    }
};

void InitSerialization() {
    auto* interfaceA = SKSE::GetSerializationInterface();
    interfaceA->SetUniqueID(Serialization::kModID);
    interfaceA->SetSaveCallback(Serialization::OnGameSaved);
    interfaceA->SetRevertCallback(Serialization::OnRevert);
    interfaceA->SetLoadCallback(Serialization::OnGameLoaded);
    Serialization::CreateGlobalVariables();
}

SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
    InitSerialization();
    Hooks::Install();
    return true;
}