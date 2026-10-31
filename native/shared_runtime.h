// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "model2_capture.h"
#include "snapshot_slot.h"
#include "input_policy.h"
#include "game_state.h"
#include "aim_policy.h"
#include "gun_setup.h"
#include "native_weapon.h"
#include <atomic>
#include <mutex>
namespace bel {
struct NativeSnapshot {
    GameState state;
    std::array<NativeAimCalibration,2> calibration;
    ShotOwnership ownership;
};
struct SharedRuntime {
    static constexpr uint32_t Abi=0x42454c05;
    uint32_t abi{Abi};
    SnapshotSlot<const NativeFrame> scene;
    std::mutex mutex;
    GameState state;
    ShotTracker shot_tracker;
    ShotOwnership shot_ownership;
    std::array<NativeAimCalibration,2> calibration;
    std::array<std::shared_ptr<const NativeWeapon>,7> weapons;
    CabinetInput input,engine_input;
    CabinetLatch input_latch;OperatorLatch operator_latch;
    NativeAimPolicy native_aim;
    GunSetupPolicy gun_setup;
    uint64_t session{1};
    std::atomic<bool> host_ready{},pause_requested{},quit_requested{},service_requested{},service_active{};
    std::atomic<uint16_t> operator_pending{},operator_latched{};
    NativeSnapshot native_snapshot(){std::lock_guard lock(mutex);return {state,calibration,shot_ownership};}
    template<class Read32> void update_native(Read32 read) {
        std::lock_guard lock(mutex);
        auto current=read_game_state(read);
        const bool had_players=state.players[0].joined()||state.players[1].joined();
        const bool has_players=current.players[0].joined()||current.players[1].joined();
        if((had_players&&!has_players&&!current.service)||(current.mode==11&&state.mode!=11))++session;
        current.session=session;
        shot_tracker.update(current,read);
        // Attribute the preceding accepted frame before accepting new intent.
        shot_ownership.update(current,state,engine_input);
        for(unsigned p=0;p<2;p++)calibration[p]=read_native_calibration(read,p);
        const bool left_service=state.service&&!current.service;
        state=current;
        auto desired=input;
        uint8_t joined{};for(unsigned p=0;p<2;p++)if(current.players[p].joined())joined|=1<<p;
        const auto release=[&] {
            desired.cabinet.trigger={};desired.cabinet.missile={};
            desired.cabinet.coins=desired.cabinet.starts=0;desired.accepted_shots={};
        };
        if(current.service) {
            native_aim.suspend();
            // Only deliberate ServiceGunPolicy intent can press a native
            // calibration trigger. Queued gameplay input cannot cross modes.
            if(!desired.service_guns)release();
            desired.cabinet.missile={};desired.cabinet.coins=desired.cabinet.starts=0;
            desired.accepted_shots={};
        } else if(!current.valid||desired.service_guns||desired.native_session!=current.session||desired.native_joined!=joined) {
            native_aim.suspend();release();
        } else {
            if(left_service)native_aim.suspend();
            // Resolve immediately before native delivery, against this tick's
            // actual previous yaw/calibration and the physical held buttons.
            native_aim.apply(desired,desired.aim_hands,calibration);
            if(!current.ready)desired.cabinet.coins=desired.cabinet.starts=0;
        }
        const auto setup=gun_setup.update(current,host_ready&&!pause_requested);
        state.automatic_calibration=setup.active;
        if(setup.active) {
            native_aim.suspend();desired={};desired.cabinet=setup.cabinet;
            operator_pending=0;operator_latched=0;operator_latch={};
        } else operator_latched=operator_latch.update(operator_pending.exchange(0));
        engine_input=input_latch.update(desired);
    }
    void release_inputs(){std::lock_guard lock(mutex);input={};engine_input={};input_latch={};operator_latch={};native_aim.suspend();gun_setup.release();operator_pending=0;operator_latched=0;}
    void reset(){scene.store({});weapons={};{std::lock_guard lock(mutex);state={};calibration={};gun_setup={};shot_tracker.suspend();shot_ownership={};input={};engine_input={};input_latch={};operator_latch={};native_aim.suspend();++session;}host_ready=false;pause_requested=false;quit_requested=false;service_requested=false;service_active=false;operator_pending=0;operator_latched=0;}
};
inline SharedRuntime& shared_runtime(){static SharedRuntime runtime;return runtime;}
}
