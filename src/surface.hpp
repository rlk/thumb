#ifndef SURFACE_HPP
#define SURFACE_HPP

#include <string>
#include <memory>

#include "obj.hpp"

//-----------------------------------------------------------------------------

namespace ogl
{
    class surface
    {
        std::string             name;
        std::auto_ptr<obj::obj> data;

    public:

        const std::string& get_name() const { return name; }

        surface(std::string name) : name(name), data(new obj::obj(name)) { }

        // Mesh accessors

        GLsizei     max_mesh()          const { return data->max_mesh();  }
        const mesh *get_mesh(GLsizei i) const { return data->get_mesh(i); }

        // Bound calculators

        void box_bound(GLfloat *b) const { return data->box_bound(b); }
        void sph_bound(GLfloat *b) const { return data->sph_bound(b); }
    };
}

//-----------------------------------------------------------------------------

#endif
