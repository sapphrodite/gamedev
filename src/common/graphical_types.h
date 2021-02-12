#ifndef GRAPHICAL_TYPES_H
#define GRAPHICAL_TYPES_H

#include "basic_types.h"
#include <vector>

using entity = u32;
using texture = u32;
constexpr static entity null_entity = 65535;
constexpr static texture null_texture = 65535;


struct color {
    u8 r, g, b, a;
    color() = default;
    color(u8 r_in, u8 g_in, u8 b_in, u8 a_in) : r(r_in), g(g_in), b(b_in), a(a_in) {}
};

struct image {
public:
    image() = default;
    image(std::vector<u8> data_in, size<u16> size_in) : _data(data_in), _size(size_in) {}
    color get(size_t i) { return color{_data[i * 4], _data[i * 4 + 1], _data[i * 4 + 2], _data[i * 4 + 3]}; }
    void write(size_t i, color c) {
        _data[i * 4] = c.r;
        _data[i * 4 + 1] = c.g;
        _data[i * 4 + 2] = c.b;
        _data[i * 4 + 3] = c.a;
    }
    const u8* data() { return _data.data(); }
    ::size<u16> size() { return _size; }
private:
    std::vector <u8> _data;
    ::size <u16> _size;
};


struct vertex {
    point<f32> pos;
    point<f32> uv;
};

struct texture_data {
    f32 z_index;
    image image_data;
    size<u16> subsprites;
    bool is_greyscale = false;
};

// You can set render layer to "null" to prevent display of a sprite.
enum class render_layers : u8 {
    null,
    sprites,
    text,
    ui
};

#endif //GRAPHICAL_TYPES_H