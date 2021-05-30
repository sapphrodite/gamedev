#include <engine/engine.h>
#include <world/basic_entity_funcs.h>
#include "ui_helper_funcs.h"

void transfer_item(entity , engine& g, ecs::c_inventory&);


// Given a gridsquare, this function returns the top left sprite coordinate corresponding to it
sprite_coords snap_to_grid(ecs::c_display& spr, point<u16> in) {
    sprite_coords tl = spr.sprites(0).get_dimensions().origin;
	return sprite_coords(in.x * 64 + tl.x, in.y * 64 + tl.y);
}

void set_square_highlight(entity e, engine& g, int square, bool highlight) {
    g.ecs.get<ecs::c_display>(e).sprites(0).set_tex_region(highlight ? 1 : 0, square);
}

// Set the inventory to be the active cursor widget, and put the selection box in place of the clicked-on element.
void pickup_item(entity e, sprite_coords box_snap_point, engine& g) {
    ecs::c_selection& select = g.ecs.get<ecs::c_selection>(e);
    ecs::c_display& spr = g.ecs.get<ecs::c_display>(e);
    g.ui.cursor = e;
}

void write_item_to_slot(size_t index, sprite_coords pos, entity e, ecs::c_inventory::item item_in, engine& g) {
    ecs::c_display& spr = g.ecs.get<ecs::c_display>(e);
    ecs::c_selection &select = g.ecs.get<ecs::c_selection>(e);

    size<f32> item_size = spr.sprites(0).get_dimensions(0).size;
    spr.sprites(1).set_pos(pos, item_size, index);
    spr.sprites(1).set_tex_region(item_in.ID, index);

    auto& text = g.ecs.get<ecs::c_text>(e);
    spr.sprites(text.sprite_index).set_pos(pos, item_size, index);
    text.text_entries[index].quad_index = index;
    text.text_entries[index].text = std::to_string(item_in.quantity);
}

void empty_slot(size_t index, sprite_coords pos, entity e, engine& g) {
    ecs::c_display& spr = g.ecs.get<ecs::c_display>(e);
    spr.sprites(1).set_pos(pos, size(0.0f, 0.0f), index);

    auto& text = g.ecs.get<ecs::c_text>(e);
    spr.sprites(text.sprite_index).set_pos(point<f32>(0, 0), size<f32>(0, 0), index);
    text.text_entries[index].text = "";
}

void place_item(entity e, engine& g, ecs::c_selection& select, ecs::c_inventory& inv, ecs::c_display& spr) {
    g.ui.cursor = 65535;
    // box_index represents the position the currently held item was taken from
    point<u16> box_point = g.ecs.get<ecs::c_selection>(e).secondary_active;
    size_t box_index = box_point.x + (box_point.y * select.grid_size.x);

    ecs::c_inventory::item held = inv.data.get(box_index);
    empty_slot(box_index, snap_to_grid(spr, box_point), e, g);
    inv.data.remove(box_index);


    if (inv.data.exists(select.active_index())) {
        ecs::c_inventory::item highlight = inv.data.get(select.active_index());
        inv.data.remove(select.active_index());
        inv.data.add(box_index, highlight);
        write_item_to_slot(box_index, snap_to_grid(spr, select.secondary_active), e, highlight, g);
        pickup_item(e, snap_to_grid(spr, box_point), g);
    } else {
        set_square_highlight(e, g, project_to_1D(select.secondary_active, select.grid_size.x), false);
        select.secondary_active = point<u16>(0, 0);
    }

    inv.data.add(select.active_index(), held);
    write_item_to_slot(select.active_index(), snap_to_grid(spr, select.active), e, held, g);
}

void inventory_activation(entity e, engine& g) {
    ecs::c_inventory& inv = g.ecs.get<ecs::c_inventory>(g.player_id());
    ecs::c_selection& select = g.ecs.get<ecs::c_selection>(e);
    ecs::c_display& spr = g.ecs.get<ecs::c_display>(e);

	// There's no item "in hand", try and pick one up
	if (g.ui.cursor == 65535) {
        if (inv.data.exists(select.active_index())) {
            pickup_item(e, snap_to_grid(spr, select.active), g);
            select.secondary_active = select.active;
        }
	} else if (g.ui.cursor != e) { // There's another inventory widget active
        entity parent = g.ecs.get<ecs::c_widget>(e).parent;
        transfer_item(parent, g, inv);
	} else {  // You already have an item, try to place or swap it.
        place_item(e, g, select, inv, spr);
	}
}

void inventory_navigation(entity e, engine& g, int old_target, int new_target) {
    ecs::c_selection& select = g.ecs.get<ecs::c_selection>(e);
    if (g.ui.cursor == e) {
        auto& spr = g.ecs.get<ecs::c_display>(e);
        size<f32> item_size = spr.sprites(0).get_dimensions(0).size;
        sprite_coords p (g.ui.last_position.to<f32>() - (item_size / 2));
        size_t box_index = project_to_1D(select.secondary_active, select.grid_size.x);
        spr.sprites(1).set_pos(p, item_size, box_index);
        spr.sprites(2).set_pos(p, item_size, box_index);
    }
    if (old_target != project_to_1D(select.secondary_active, select.grid_size.x)) {
        set_square_highlight(e, g, old_target, false);
    }
    if (abs(new_target) >= (select.grid_size.x * select.grid_size.y)) return;
    set_square_highlight(e, g, new_target, true);
    select.active = project_to_2D<u16>(new_target, select.grid_size.x);
}

void inventory_init(entity e, engine& g, entity parent, ecs::c_inventory& inv, screen_coords origin) {
	g.ui.focus = e;
	make_widget(e, g, parent);
    ecs::c_selection& select = g.ecs.add<ecs::c_selection>(e);

    auto& w = g.ecs.get<ecs::c_widget>(e);
    w.on_activate = inventory_activation;
    w.on_navigate = inventory_navigation;

	select.grid_size = size<u16>(9, 4);
	sprite_coords element_size(64, 64);
	rect<f32> dim(origin.to<f32>(), sprite_coords(select.grid_size.x * element_size.x, select.grid_size.y * element_size.y));

	int num_grid_elements = select.grid_size.x * select.grid_size.y;
    ecs::c_display& spr = g.ecs.add<ecs::c_display>(e);
	spr.add_sprite(num_grid_elements, render_layers::ui);
	spr.sprites(0).tex = g.textures().get("highlight");

	int item_spr = spr.add_sprite(num_grid_elements, render_layers::ui);
	spr.sprites(item_spr).tex = g.textures().get("items");

    ecs::c_text& text = g.ecs.add<ecs::c_text>(e);
    text.sprite_index = spr.add_sprite(num_grid_elements, render_layers::text);
    text.text_entries = std::vector<ecs::c_text::text_entry>(num_grid_elements);

	int index = 0;
	sprite_coords pos = origin.to<f32>();
	for (u16 y = 0; y < select.grid_size.y; y++) {
		for (u16 x = 0; x < select.grid_size.x; x++) {
			spr.sprites(0).set_pos(pos, element_size, index);
			spr.sprites(0).set_tex_region(0, index);

			if (inv.data.exists(index)) {
                write_item_to_slot(index, pos, e, inv.data.get(index), g);
			}

			pos.x += element_size.x;
			index++;
		}
		pos.y += element_size.y;
		pos.x = 100;
	}
}

// transfer from A to B
void transfer_item(entity e, engine& g, ecs::c_inventory& b_inv) {
    ecs::c_widget& widget = g.ecs.get<ecs::c_widget>(e);
    entity a = g.ui.cursor;


    ecs::c_inventory& player_inv = g.ecs.get<ecs::c_inventory>(g.player_id());
    ecs::c_inventory& storage = g.ecs.get<ecs::c_inventory>(g.ui.active_interact);
    ecs::c_inventory& a_inv = &b_inv == &player_inv ? storage : player_inv;
    ecs::c_selection& a_select = g.ecs.get<ecs::c_selection>(a);
    ecs::c_display& a_spr = g.ecs.get<ecs::c_display>(a);
    size_t held_index = project_to_1D(a_select.secondary_active, a_select.grid_size.x);

    ecs::c_inventory::item held = a_inv.data.get(held_index);
    empty_slot(held_index, snap_to_grid(a_spr, a_select.secondary_active), a, g);
    a_inv.data.remove(held_index);




    entity b = 65535;
    for (auto& child : widget.children)
        if (child != a) {
            b = child;
            break;
        }
    ecs::c_selection& b_select = g.ecs.get<ecs::c_selection>(b);
    ecs::c_display& b_spr = g.ecs.get<ecs::c_display>(b);

    g.ui.cursor = 65535;

    if (b_inv.data.exists(b_select.active_index())) {
        ecs::c_inventory::item highlight = b_inv.data.get(b_select.active_index());
        b_inv.data.remove(b_select.active_index());
        a_inv.data.add(held_index, highlight);
        write_item_to_slot(held_index, snap_to_grid(a_spr, a_select.active), a, highlight, g);
        pickup_item(a, snap_to_grid(a_spr, a_select.secondary_active), g);

    }

    b_inv.data.add(b_select.active_index(), held);
    write_item_to_slot(b_select.active_index(), snap_to_grid(b_spr, b_select.active), b, held, g);
}

void inv_transfer_init(entity e, engine& g, ecs::c_inventory& storage_inv) {
    g.ecs.add<ecs::c_widget>(e);
    g.command_states.set(command::toggle_inventory, true);
    g.create_entity(inventory_init, e, g.ecs.get<ecs::c_inventory>(g.player_id()), screen_coords(100, 100));
    g.create_entity(inventory_init, e, storage_inv, screen_coords(100, 400));
    g.ui.focus = e;
}