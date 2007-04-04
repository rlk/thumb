#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>

#include "opengl.hpp"

//-----------------------------------------------------------------------------

namespace ogl
{
    class shader
    {
        GLhandleARB vert;
        GLhandleARB frag;
        GLhandleARB prog;

        void log(GLhandleARB);

    public:

        shader(std::string, std::string);
       ~shader();

        void bind() const;
        void free() const;

        void uniform(std::string, int)                        const;
        void uniform(std::string, float)                      const;
        void uniform(std::string, float, float)               const;
        void uniform(std::string, float, float, float)        const;
        void uniform(std::string, float, float, float, float) const;
    };
}

//-----------------------------------------------------------------------------

#endif
