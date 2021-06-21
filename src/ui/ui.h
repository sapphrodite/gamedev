#ifndef UI_HELPER_FUNCS_H
#define UI_HELPER_FUNCS_H

#include <engine/engine.h>




void checkbox_navigation(entity e, engine& g, u32 new_index);
void add_checkbox(entity e, engine& g, point<f32> pos, size<f32> grid_size, u32 index, bool state, std::string label);
void initialize_checkbox_group(entity e, entity parent, u8 z_index, engine& g, u32 num_checkboxes);


void slider_set_value(entity e, engine& g, int value, int index);
void slider_navigation(entity e, engine& g, u32 new_index);
void add_slider(entity e, engine& g, point<f32> pos, size<f32> grid_size, u32 index, int min, int current, int max, int increment, std::string label);
void initialize_slider_group(entity e, entity parent, u8 z_index, engine& g, int num_sliders);



void button_activation(entity e, engine& g);
void button_navigation(entity e, engine& g, u32 new_index);
void add_button(entity e, engine& g, point<f32> pos, size<f32> grid_size, u32 index, ecs::c_widget::activation_action function, std::string label);
void initialize_button_group(entity e, entity parent, u8 z_index, engine& g, int num_buttons);


void dropdown_activation(entity e, engine& g, bool release);
void dropdown_navigation(entity e, engine& g, u32 new_index);
void add_dropdown(entity e, engine& g, point<f32> pos, size<f32> grid_size, u32 index, std::vector<std::string> options, int active);
void initialize_dropdown_group(entity e, entity parent, u8 z_index, engine& g, int num_dropdowns);


void selectiongrid_set_highlight(entity e, engine& g, u32 index, bool enter);
void selectiongrid_navigation(entity e, engine& g,  u32 new_index);

point<u16> get_gridindex(rect<f32> grid, size<u16> num_elements, point<f32> pos);

void inv_transfer_init(entity e, engine& g, ecs::c_inventory&);
rect<f32> calc_size_percentages(rect<f32> parent, rect<f32> sizes );
void make_widget(entity e, engine& g, entity parent);


void inventory_init(entity, engine&, entity, ecs::c_inventory& inv, screen_coords);
void options_menu_init(entity, engine&, entity, screen_coords);



#endif //UI_HELPER_FUNCS_H
