#pragma once

#include <dolphin/mtx.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "settings.h"

class camera_process_class;
class view_class;

#ifdef __cplusplus
namespace dusk {
namespace frame_interp {

void ensure_initialized();

void begin_record();
void end_record();
void begin_sim_tick();
uint64_t sim_tick_seq();
void begin_frame(FrameInterpMode mode, bool is_sim_frame, float step);
void interpolate();
float get_interpolation_step();

void request_presentation_sync();
bool presentation_sync_active();

bool is_enabled();

// TODO: These should be phased out as UI is progressively updated to use game_clock
void set_ui_tick_pending(bool value);
bool get_ui_tick_pending();

bool is_sim_frame();

class TickSpin {
public:
    template <typename F>
    float draw_then_advance(F&& deltaFn) {
        if (get_ui_tick_pending()) {
            m_base = wrap(m_base + m_delta);
            m_delta = deltaFn();
        }
        return sample();
    }

    float draw_then_advance(float delta) {
        return draw_then_advance([delta] { return delta; });
    }

    template <typename F>
    float advance_then_draw(F&& deltaFn) {
        if (get_ui_tick_pending()) {
            m_delta = deltaFn();
            m_base = wrap(m_base + m_delta);
        }
        return sample();
    }

    float advance_then_draw(float delta) {
        return advance_then_draw([delta] { return delta; });
    }

private:
    static float wrap(float value) {
        if (value >= 360.0f) {
            value -= 360.0f;
        } else if (value <= -360.0f) {
            value += 360.0f;
        }
        return value;
    }

    float sample() const {
        return m_base + m_delta * get_interpolation_step();
    }

    float m_base = 0.0f;
    float m_delta = 0.0f;
};

void record_camera(::camera_process_class* cam, int camera_id);
void interp_view(::view_class* view);
void record_final_mtx(Mtx m, const void *key);
void record_final_mtx(Mtx m);

bool lookup_replacement(const void* key, Mtx out);
bool lookup_concat_replacement(const void* lhs, const void* rhs, Mtx out);

typedef void (*InterpolationCallBack)(bool isSimFrame, void* pUserWork);
// call on a sim tick, will get called during presentation
void add_interpolation_callback(InterpolationCallBack pCallBack, void* pUserWork);

void begin_presentation_camera();
void end_presentation_camera();

}  // namespace frame_interp
}  // namespace dusk
#endif
