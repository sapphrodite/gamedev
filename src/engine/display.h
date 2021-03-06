#ifndef DISPLAY_H
#define DISPLAY_H

#include <common/basic_types.h>
#include <common/graphical_types.h>
#include "input_event.h"
#include <functional>
#include <unordered_map>
#include <string>
#include <memory>


namespace display {
class texture_manager {
private:
    [[no_unique_address]] no_copy disable_copy;
    [[no_unique_address]] no_move disable_move;
public:
    texture* add(std::string name);
    texture* get(std::string name);
    void load_textures();
    virtual ~texture_manager() = default;
    virtual void update(texture*) = 0;
protected:
    virtual u32 get_new_id() = 0;
private:
    std::array<texture, 2048> textures;
    std::unordered_map<std::string, u32> texture_map;
};

class renderer {
private:
    [[no_unique_address]] no_copy disable_copy;
    [[no_unique_address]] no_move disable_move;
public:
    virtual ~renderer() = default;
    virtual void set_viewport(screen_coords) = 0;
    virtual void set_camera(vec2d<f32>) = 0;
    virtual void clear_screen() = 0;

    void clear_sprites();
    void mark_sprites_dirty();
    void add_sprite(sprite_data&);
    void render_layer(texture_manager&);
protected:
    virtual void render_batch(texture*, render_layers, texture_manager&) = 0;

    std::vector<sprite_data> batching_pool;
    vertex* vertex_buffer = nullptr;
    u8* zindex_buffer = nullptr;
    size_t quads_batched = 0;
    bool sprites_dirty = false;
};

struct window_impl {
private:
    [[no_unique_address]] no_copy disable_copy;
    [[no_unique_address]] no_move disable_move;
public:
    virtual ~window_impl() {}
    virtual std::vector<std::unique_ptr<input_event>> poll_events() = 0;
    virtual void swap_buffers(renderer&) = 0;
    virtual void set_vsync(bool) = 0;

    virtual void set_resolution(screen_coords) = 0;
    virtual void set_fullscreen(bool) = 0;

    screen_coords resolution() { return _resolution; }
    bool fullscreen() { return _fullscreen; }
    bool renderer_busy() { return _renderer_busy; }
    std::function<void(screen_coords)> resize_callback;
protected:
    bool update_resolution = true; // Update resolution internally on the next tick
    bool _renderer_busy = false;
    bool _fullscreen = false;
    screen_coords _resolution;
};

class display_manager : no_copy, no_move {
public:
    enum display_types {
        opengl,
        software
    };
    void initialize(display_types, screen_coords);
    void render();
    std::vector<std::unique_ptr<input_event>> poll_events() { return get_window().poll_events(); }

    renderer& get_renderer() { return *_renderer.get(); }
    texture_manager& textures() { return *_textures.get(); }
private:
    window_impl& get_window() { return *_window.get(); }
    std::unique_ptr<renderer> _renderer;
    std::unique_ptr<window_impl> _window;
    std::unique_ptr<texture_manager> _textures;
};

}
#endif //DISPLAY_H
