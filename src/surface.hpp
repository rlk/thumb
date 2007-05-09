#ifndef SURFACE_HPP
#define SURFACE_HPP

#include <string>

#include "obj.hpp"

//-----------------------------------------------------------------------------

namespace ogl
{
    class surface
    {
        std::string name;
        obj::obj   *data;

    public:

        const std::string& get_name() const { return name; }

        surface(std::string);
       ~surface();

        void box_bound(float *) const;
        void sph_bound(float *) const;

        GLsizei ecopy(GLsizei, GLsizei);
        GLsizei vcopy(GLsizei);
        GLsizei esize() const;
        GLsizei vsize() const;

        void draw(int=DRAW_OPAQUE) const;
        int  type(               ) const;

        void init();
        void fini();
    };
}

#endif
