#ifndef GRAPHICAL_TYPES_H
#define GRAPHICAL_TYPES_H

#include "basic_types.h"
#include <vector>

struct color {
    u8 r, g, b, a;
    color() = default;
    color(u8 r_in, u8 g_in, u8 b_in, u8 a_in) : r(r_in), g(g_in), b(b_in), a(a_in) {}
};

template <typename T>
struct image_wrapper {
public:
    image_wrapper() = default;
    image_wrapper(T data_in, size<u16> size_in) : _data(data_in), _size(size_in)
    {

    }
    color get(size_t i) {
        if (i * 4 >= _data.size()) throw "bruh";
        return color{_data[i * 4], _data[i * 4 + 1], _data[i * 4 + 2], _data[i * 4 + 3]};
    }
    void write(size_t i, color c) {
        if (i * 4 >= _data.size()) throw "bruh";
        _data[i * 4] = c.r;
        _data[i * 4 + 1] = c.g;
        _data[i * 4 + 2] = c.b;
        _data[i * 4 + 3] = c.a;
    }
    T& data() { return _data; }
    ::size<u16> size() { return _size; }
private:
    T _data;
    ::size <u16> _size;
};

using image = image_wrapper<std::vector<u8>>;
using framebuffer = image_wrapper<u8*>;

struct vertex {
    sprite_coords pos;
    point<f32> uv;
};

struct texture {
    u32 id;
    image image_data;
    size<u16> regions;
};


class texture_generator {
public:
    void set_texture(texture *in) {
        tex = in;
        regenerate = true;
    }

    texture* get_texture() { return tex; }
protected:
    size_t last_atlassize = 0;
    texture* tex;
    bool regenerate = true;
};

// You can set render layer to "null" to prevent display of a sprite.
enum class render_layers : u8 {
    null,
    sprites,
    text,
    ui
};


static constexpr unsigned vertices_per_quad = 4;

struct sprite_data {
    texture* tex = nullptr;
    u8 z_index = 1;
    render_layers layer;

    sprite_data(size_t num_quads, texture * tex_in, int z_index_in, render_layers layer_in) {
        _vertices = std::vector<vertex>(num_quads * vertices_per_quad);
        tex = tex_in;
        z_index = z_index_in;
        layer = layer_in;
    }

    const std::vector<vertex>& vertices() const { return _vertices; };
    int num_quads() { return _vertices.size() / 4; };
    rect<f32> get_dimensions(u8 quad_index = 255);

    void set_pos(sprite_coords, sprite_coords, size_t);
    void set_uv(point<f32>, size<f32>, size_t);
    void set_tex_region(size_t, size_t);
    void rotate(f32 theta);
    void move_by(sprite_coords);
    void move_to(sprite_coords);

    inline bool operator < (const sprite_data& rhs ) const {
        if (layer < rhs.layer) return true;
        if (z_index < rhs.z_index) return true;
        if (tex->id < rhs.tex->id) return true;
        return false;
    }
private:
    std::vector<vertex> _vertices {};
};



#endif //GRAPHICAL_TYPES_H