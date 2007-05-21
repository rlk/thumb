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

        surface(std::string);
       ~surface();

        // Bound calculators

        void box_bound(GLfloat *) const;
        void sph_bound(GLfloat *) const;

        // Batch data accessors

        GLsizei      count()                                   const;
        GLsizei      esize(GLsizei)                            const;
        GLsizei      vsize(GLsizei)                            const;
        void         ecopy(GLsizei, GLvoid *, GLuint)          const;
        void         vcopy(GLsizei, GLvoid *, const GLfloat *) const;
        std::string& state(GLsizei)                            const;
    };
}

#endif
