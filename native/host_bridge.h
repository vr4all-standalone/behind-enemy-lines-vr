// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "shared_runtime.h"
#include "modules/lib/osdlib.h"
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#ifdef __ANDROID__
#include <jni.h>
extern "C" void* SDL_AndroidGetJNIEnv(void);
extern "C" void* SDL_AndroidGetActivity(void);
#endif
namespace bel {
inline bool vr_requested(){static const bool enabled=std::getenv("BEL_VR")!=nullptr;return enabled;}
class HostBridge {
    osd::dynamic_module::ptr module_;void(*stop_)(){};
public:
    HostBridge() {
        if(!vr_requested())return;
        const char* name=std::getenv("BEL_XR_LIBRARY");
#ifdef _WIN32
        module_=osd::dynamic_module::open({name?name:"bel_xr.dll"});
#else
        module_=osd::dynamic_module::open({name?name:"libbel_xr.so"});
#endif
        auto start=module_->bind<bool(*)(SharedRuntime*)>("bel_xr_start");
        stop_=module_->bind<void(*)()>("bel_xr_stop");
#ifdef __ANDROID__
        auto init=module_->bind<void(*)(void*,void*)>("bel_xr_android_init");
        auto env=static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());JavaVM* vm{};
        if(env&&init){env->GetJavaVM(&vm);auto activity=SDL_AndroidGetActivity();init(vm,activity);env->DeleteLocalRef(static_cast<jobject>(activity));}
#endif
        if(!start||!stop_||!start(&shared_runtime())){std::cerr<<"Behind Enemy Lines VR: host could not start; check the library and log.\n";shared_runtime().quit_requested=true;}
    }
    ~HostBridge(){if(stop_)stop_();}
};
inline std::unique_ptr<HostBridge>& host_bridge_slot(){static std::unique_ptr<HostBridge> bridge;return bridge;}
inline void start_host(){auto& slot=host_bridge_slot();if(!slot){shared_runtime().reset();slot=std::make_unique<HostBridge>();}}
inline void stop_host(){host_bridge_slot().reset();shared_runtime().reset();}
template<class Machine> void poll_host(Machine& machine) {
    if(!vr_requested())return;
    start_host();auto& runtime=shared_runtime();
    if(runtime.quit_requested)machine.schedule_exit();
    if(runtime.host_ready){if(runtime.pause_requested&&!machine.paused()){runtime.release_inputs();machine.pause();}else if(!runtime.pause_requested&&machine.paused())machine.resume();}
    using Clock=std::chrono::steady_clock;static auto last=Clock::now();static double native_before=machine.time().as_double();
    auto now=Clock::now();static auto previous_poll=now;static bool previous_paused=machine.paused();static double paused_wall{};
    if(previous_paused)paused_wall+=std::chrono::duration<double>(now-previous_poll).count();
    previous_poll=now;previous_paused=machine.paused();
    double elapsed=std::chrono::duration<double>(now-last).count();
    if(elapsed>=5){double native=machine.time().as_double();auto frame=runtime.scene.load();
        if(const char* path=std::getenv("BEL_XR_LOG")){
            const double active=std::max(0.,elapsed-paused_wall);
            const auto snapshot=runtime.native_snapshot();
            std::ofstream out(std::string(path)+".engine",std::ios::app);
            out<<"Native seconds "<<native<<", speed "<<(native-native_before)/elapsed*100
               <<"%, active speed "<<(active>.001?(native-native_before)/active*100:0)
               <<"%, active wall "<<active<<", captured frame "<<(frame?frame->sequence:0)<<", paused "<<machine.paused()<<", mode "<<snapshot.state.mode<<'/'<<snapshot.state.submode
               <<", players "<<snapshot.state.players[0].state<<'/'<<snapshot.state.players[1].state
               <<", wall_s "<<std::chrono::duration<double>(now.time_since_epoch()).count()<<'\n';
            out<<"Gun setup "<<snapshot.state.automatic_calibration<<", erased "<<snapshot.state.calibration_erased
               <<", saved config "<<snapshot.state.calibration_config_valid<<", service "<<snapshot.state.service
               <<", menu "<<snapshot.state.service_selection<<'/'<<snapshot.state.service_menu<<'/'<<snapshot.state.service_submode
               <<", live aim "<<snapshot.calibration[0].valid<<'/'<<snapshot.calibration[1].valid<<'\n';
        }last=now;native_before=native;paused_wall=0;
    }
}
inline Cabinet cabinet_input(){auto& runtime=shared_runtime();std::lock_guard lock(runtime.mutex);return runtime.engine_input.cabinet;}
inline uint8_t system_input(uint8_t native) {
    if(!vr_requested())return native;
    auto input=cabinet_input();native&=~uint8_t(shared_runtime().operator_latched.load()&0x3c);
    using Clock=std::chrono::steady_clock;static Clock::time_point until{};
    if(shared_runtime().service_requested.exchange(false))until=Clock::now()+std::chrono::milliseconds(200);
    input.test=input.test||Clock::now()<until;
    return in0(native,input);
}
inline uint8_t trigger_input(uint8_t native){return vr_requested()?in1(native,cabinet_input()):native;}
inline uint8_t analog_input(unsigned channel,uint8_t native){return vr_requested()?adc(channel,cabinet_input()):native;}
template<class Read32> void native_tick(Read32 read) {
    if(!vr_requested())return;
    shared_runtime().update_native(read);
}
}
