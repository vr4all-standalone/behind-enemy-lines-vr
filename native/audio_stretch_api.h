// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <cstdint>

// Plain C ABI between the SDL engine and the separately owned audio worker.
// Native emulation/synthesis and the original WAV recorder remain upstream.
inline constexpr uint32_t BelAudioAbi = 0x42454101;
enum BelAudioState : uint32_t {
    BelAudioWarmup = 0, BelAudioRunning = 1,
    BelAudioPaused = 2, BelAudioFailed = 3
};
struct BelAudioStats {
    uint32_t size{},abi{},state{},sample_rate{},channels{};
    uint32_t input_queue_frames{},output_queue_frames{},reserved{};
    uint64_t native_frames{},output_frames{},requested_frames{},delivered_frames{};
    uint64_t missing_frames{},warmup_frames{},paused_frames{},paused_input_frames{};
    uint64_t input_dropped_frames{},output_dropped_frames{};
    uint64_t callbacks{},underrun_callbacks{},input_blocks{},output_blocks{};
    double tempo{1},input_ratio{1},max_input_gap_ms{},max_output_gap_ms{};
};

// Push/read timestamps use the same caller-supplied monotonic seconds.
// Push consumes/copies input without DSP work. Intentional paused input is
// counted separately from overflow. Read always fills the destination, returns
// real delivered frames, and zero-fills any remainder. It performs no locking,
// allocation or I/O. Missing includes warmup but excludes intentional pause:
// requested = delivered + missing + paused; warmup is a subset of missing.
// Pause only stores control state; DSP reset/drain occurs on worker/consumer.
// Stats are cumulative atomic observations, not a globally latched snapshot.
#if defined(_WIN32) && defined(BEL_AUDIO_BUILD)
#define BEL_AUDIO_EXPORT extern "C" __declspec(dllexport)
#elif defined(BEL_AUDIO_BUILD)
#define BEL_AUDIO_EXPORT extern "C" __attribute__((visibility("default")))
#else
#define BEL_AUDIO_EXPORT extern "C"
#endif
BEL_AUDIO_EXPORT void* bel_audio_create(uint32_t rate,uint32_t channels);
BEL_AUDIO_EXPORT uint32_t bel_audio_push(void*,const int16_t*,uint32_t frames,double now);
BEL_AUDIO_EXPORT uint32_t bel_audio_read(void*,int16_t*,uint32_t frames,double now);
BEL_AUDIO_EXPORT void bel_audio_pause(void*,uint32_t paused);
BEL_AUDIO_EXPORT uint32_t bel_audio_stats(void*,BelAudioStats*,uint32_t bytes);
BEL_AUDIO_EXPORT void bel_audio_destroy(void*);
