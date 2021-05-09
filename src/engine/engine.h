#ifndef ENGINE_H
#define ENGINE_H

#include <ecs/ecs.h>
#include "renderer.h"
#include <unordered_map>
#include <memory>

class engine;

class ui_manager {
public:
	entity root = 65535;
	entity focus = 65535;
	entity cursor = 65535;
	entity active_interact = 65535;

	screen_coords window_size;
	screen_coords last_position;

	timer hover_timer;
	bool hover_active = false;
};

enum window_flags {
	fullscreen,
	use_software_render,
	bordered_window,
	vsync,
};

class settings_manager {
public:
	settings_manager();
	screen_coords resolution;
	std::bitset<8> flags;
	std::unordered_map<u8, command> bindings;
	int framerate_multiplier = 2;
};

struct window_impl {
    std::function<void(event&)> event_callback;
    std::function<void(screen_coords)> resize_callback;
    bool fullscreen = false;
	bool update_resolution = true; // Update resolution internally on the next tick
    screen_coords resolution;

    virtual bool poll_events() = 0;
    virtual void swap_buffers(renderer_base&) = 0;
    virtual screen_coords get_drawable_resolution() = 0;
    virtual ~window_impl() {}
	bool renderer_busy() { return _renderer_busy; }
protected:
	bool _renderer_busy = false;
};

void set_window_impl(std::unique_ptr<window_impl>&, settings_manager&);


typedef void (*logic_func)(engine&);
class logic_manager {
public:
    void add(logic_func);
    void remove(logic_func);
private:
    friend class engine;
    std::vector<logic_func> logic;
};

class engine {
public:
    settings_manager settings;
	ecs_engine ecs;
	ui_manager ui;
    logic_manager logic;
	renderer_base& renderer() { return *_renderer.get(); }
	window_impl& window() { return *_window.get(); }

	engine();
	bool process_events();
	void render();
	void run_tick();

    std::multiset<sprite_data> sprites;

	template <typename f, typename... Args>
	entity create_entity(f func, Args&&... args) {
		entity e = ecs.entities.add_entity();
		std::apply(func, std::tuple_cat(std::forward_as_tuple(e, *this), std::forward_as_tuple(args...)));
		return e;
	};

	void destroy_entity(entity e);
	entity player_id() { return ecs._player_id; }
	entity map_id() { return ecs._map_id; }

	bool in_dungeon = false;
	world_coords offset = world_coords(0, 0);
    std::bitset<8> command_states;
private:
    std::unique_ptr<renderer_base> _renderer;
    std::unique_ptr<window_impl> _window;
};



void inventory_init(entity, engine&, entity, c_inventory& inv, screen_coords);

#endif //ENGINE_H
