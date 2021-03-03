#include "engine.h"
#include <common/parser.h>
#include "../main/basic_entity_funcs.h"


void engine::run_tick() {
    point<f32> start_pos = ecs.get_component<sprite>(player_id()).get_dimensions().origin;

    ecs.run_ecs();
    ui.active_interact = ecs.systems.proxinteract.active_interact;
    //renderer.set_texture_data()
    for (auto logic_func : logic.logic) {
        logic_func(*this);
    }

    int h = ui.hover_timer.elapsed<timer::ms>().count();
    if (h >= 1500 && ui.hover_active == false) {
        ui.hover_active = true;
        event a;
        //handle_hover(a, *this);
    }

    renderer().set_texture_data(ecs.systems.health.get_texture(), ecs.systems.health.get_data());

    renderer().set_camera(offset);
    renderer().clear_sprites();
    sprites.clear();
    for (auto& sprite : ecs.components.get_pool(type_tag<sprite>())) {
        for (int i = 0; i < sprite.num_subsprites(); i++) {
            if (sprite.get_subsprite(i).layer == render_layers::null) continue;
            sprites.emplace(sprite.get_subsprite(i));
        }
    }

    point<f32> end_pos = ecs.get_component<sprite>(player_id()).get_dimensions().origin;
    offset += (end_pos - start_pos);
}


void engine::destroy_entity(entity e) {
    ecs.entities.mark_entity(e);
    // Recursively delete child widget entities
    if (ecs.component_exists<c_widget>(e)) {
        c_widget& w = ecs.get_component<c_widget>(e);
        for (auto child : w.children) destroy_entity(child);
    }
}

void engine::render() {
    renderer().clear_screen();
    renderer().render_layer(sprites);
    window.swap_buffers(renderer());
}

void logic_manager::add(logic_func func) {
    logic.push_back(func);
}

void logic_manager::remove(logic_func func) {
    for (auto it : logic) {
        if (it == func) {
            std::swap(it, logic.back());
            logic.pop_back();
            break;
        }
    }
}


settings_manager::settings_manager() {
    bindings['w'] = command::move_up;
    bindings['a'] = command::move_left;
    bindings['s'] = command::move_down;
    bindings['d'] = command::move_right;
    bindings['e'] = command::interact;
    bindings['\t'] = command::toggle_inventory;

    config_parser p("settings.txt");
    auto d = p.parse();

    const config_list* list = dynamic_cast<const config_list*>(d->get("window_size"));
    resolution = size<u16>(list->get<int>(0), list->get<int>(1));

    int fullscreen =  d->get<int>("fullscreen");
    if (fullscreen != 0) flags.set(window_flags::fullscreen);
    if (fullscreen == 1) flags.set(window_flags::bordered_window);

    if (d->get<bool>("vsync")) flags.set(window_flags::vsync);
    if(d->get<bool>("use_software_renderer")) flags.set(window_flags::use_software_render);
}


template <int type>
entity at_cursor(entity e, engine& g, point<f32> cursor) {
    entity next = 65535;
    c_widget& w = g.ecs.get_component<c_widget>(e);
    for (auto it = w.children.begin(); it != w.children.end(); it++) {
        if (!g.ecs.component_exists<event_wrapper<type>>(*it)) continue;
        if (test_collision(g.ecs.get_component<sprite>(*it).get_dimensions(0), cursor)) next = *it;
    }

    if (next == 65535) return e;
    return at_cursor<type>(next, g, cursor);
}

bool handle_button(event& e, engine& g) {
    g.ui.hover_timer.start();
    entity dest = at_cursor<0>(g.ui.root, g, e.pos);
    bool success = false;
    if (dest != 65535 && dest != g.ui.root)
        success = g.ecs.get_component<c_mouseevent>(dest).run_event(e);
    if (success) return true;

    point<f32> h (e.pos.x / 64.0f, e.pos.y / 64.0f);
    if (e.active_state() && g.in_dungeon) {
        c_player& p = g.ecs.get_component<c_player>(g.player_id());
        p.shoot = true;
        p.target = h + g.offset;
        return true;
    }
    return false;
}

bool handle_keypress(event& e, engine& g) {
    auto it = g.settings.bindings.find(e.ID);
    if (it == g.settings.bindings.end()) {
        if (g.ui.focus == 65535) return false;
        return g.ecs.get_component<c_keyevent>(g.ui.focus).run_event(e);
    }

    command command = it->second;
    switch (command) {
        case command::interact: {
            if (!e.active_state())  return false;
            entity active = g.ui.active_interact;
            if (active == 65535)  return false;
            return g.ecs.get_component<c_keyevent>(active).run_event(e);
        }
        case command::toggle_inventory: {
            if (!e.active_state()) return false;
            bool toggle_state = g.command_states.test(command::toggle_inventory) == true;
            if (toggle_state) {
                g.destroy_entity(g.ui.focus);
                g.ui.focus = 65535;
                g.ui.cursor = 65535;
            }
            else
                g.create_entity(inventory_init, g.ui.root, g.ecs.get_component<c_inventory>(g.player_id()), point<u16>(100, 100));
            g.command_states.set(command::toggle_inventory, !toggle_state);
            return false;
        }
        default: {  // Semantically, using the default case for movement is wrong, but I don't want to chain all four movement cases together
            g.command_states.set(command, e.active_state());
            point<f32>& velocity = g.ecs.get_component<c_velocity>(g.player_id()).delta;
            velocity.x = 0.05 * (g.command_states.test(command::move_right) - g.command_states.test(command::move_left));
            velocity.y = 0.05 * (g.command_states.test(command::move_down) - g.command_states.test(command::move_up));
            return true;
        }
    }
}

bool handle_cursor(event& e, engine& g) {
    g.ui.hover_timer.start();
    if (g.ui.cursor != 65535) g.ecs.get_component<c_cursorevent>(g.ui.cursor).run_event(e);

    entity dest = at_cursor<1>(g.ui.root, g, e.pos);
    entity start = at_cursor<1>(g.ui.root, g, g.ui.last_position);

    g.ui.last_position = e.pos;

    if (start == g.ui.root && dest == g.ui.root) return false;
    if (start == dest) return g.ecs.get_component<c_cursorevent>(start).run_event(e);

    if (dest != 65535 && dest != g.ui.root) return g.ecs.get_component<c_cursorevent>(dest).run_event(e);
    e.set_active(false);
    if (start != 65535 && start != g.ui.root) return g.ecs.get_component<c_cursorevent>(start).run_event(e);

    return false;
}

bool handle_hover(event& e, engine& g) {
    entity dest = at_cursor<3>(g.ui.root, g, g.ui.last_position);
    if (dest == 65535 || dest == g.ui.root) return false;
    return g.ecs.get_component<c_hoverevent>(dest).run_event(e);
}



bool process_event(event& e, engine& g) {
    if (e.type() == event_flags::null) return true;

    g.ui.hover_timer.start();
    g.ui.hover_active = false;

    if (e.type() == event_flags::button_press)
        handle_button(e, g);
    else if (e.type() == event_flags::key_press)
        handle_keypress(e, g);
    else if (e.type() == event_flags::cursor_moved)
        handle_cursor(e, g);
    return true;
}




bool engine::process_events() {
    return window.poll_events();
}

engine::engine()  : settings(), window(settings){

    printf("Window Initialized\n");
    window.set_event_callback([this](event& ev) { process_event(ev, *this); });
    window.set_resize_callback([this](size<u16> size) {
        renderer().set_viewport(size);
    });

    if (settings.flags.test(window_flags::use_software_render)) {
        _renderer = std::unique_ptr<renderer_base>(new renderer_software(settings.resolution));
    } else {
        _renderer = std::unique_ptr<renderer_base>(new renderer_gl);
    }
    ecs.systems.shooting.bullet_types.push_back(s_shooting::bullet{size<f32>(0.3, 0.6), point<f32>(8, 8), "bullet"});
    ecs.systems.shooting.shoot = [this] (std::string s, size<f32> d, point<f32> v, point<f32> o, point<f32> t, collision_flags a) {
        create_entity(egen_bullet, s, d, v, o, t, a);
    };

    window.swap_buffers(renderer());
    ecs._map_id = create_entity([&](entity, engine&){});
    ecs._player_id = create_entity([&](entity, engine&){});
    ecs.add_component<c_player>(ecs._player_id);
    ecs.add_component<c_mapdata>(map_id());


    ecs.systems.health.set_texture(renderer().add_texture("healthbar_atlas"));

    auto& inv = ecs.add_component<c_inventory>(player_id());
    ecs.add_component<sprite>(player_id());
    ecs.add_component<c_weapon_pool>(player_id());
    for(int i = 0; i < 36; i++) {
        u32 h = rand() % 3;
        if (h == 2) continue;
        inv.data.add(i, item{h, 0});
    }

    renderer().set_viewport(settings.resolution);
    create_entity([&](entity e, engine& g){
        g.ecs.add_component<c_widget>(e);
        g.ui.root = e;
    });
}
