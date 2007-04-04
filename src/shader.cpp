#include <iostream>
#include <fstream>

#include "shader.hpp"

//-----------------------------------------------------------------------------

void ogl::shader::log(GLhandleARB handle)
{
    char *log;
    GLint len;

    // Dump the contents of the log, if any.

    glGetObjectParameterivARB(handle, GL_OBJECT_INFO_LOG_LENGTH_ARB, &len);

    if ((len > 1) && (log = new char[len + 1]))
    {
        glGetInfoLogARB(handle, len, NULL, log);

        std::cerr << log << std::endl;

        delete [] log;
    }
}

//-----------------------------------------------------------------------------

ogl::shader::shader(std::string vert_txt,
                    std::string frag_txt)
{
    prog = glCreateProgramObjectARB();
    vert = 0;
    frag = 0;

    // Compile the vertex shader.

    if (vert_txt.length() > 0)
    {
        const GLcharARB *p = (const GLcharARB *) vert_txt.c_str();

        vert = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);

        glShaderSourceARB (vert, 1, &p, 0);
        glCompileShaderARB(vert);
        
        log(vert);
    }

    // Compile the frag shader.

    if (frag_txt.length() > 0)
    {
        const GLcharARB *p = (const GLcharARB *) frag_txt.c_str();

        frag = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

        glShaderSourceARB (frag, 1, &p, 0);
        glCompileShaderARB(frag);
        
        log(frag);
    }

    // Link these shader objects to a program object.

    if (vert) glAttachObjectARB(prog, vert);
    if (frag) glAttachObjectARB(prog, frag);

    glLinkProgramARB(prog);

    log(prog);

    OGLCK();
}

ogl::shader::~shader()
{
    if (prog) glDeleteObjectARB(prog);
    if (vert) glDeleteObjectARB(vert);
    if (frag) glDeleteObjectARB(frag);

    OGLCK();
}

void ogl::shader::bind() const
{
    glUseProgramObjectARB(prog);
    OGLCK();
}

void ogl::shader::free() const
{
    glUseProgramObjectARB(0);
    OGLCK();
}

//-----------------------------------------------------------------------------

void ogl::shader::uniform(std::string name, int d) const
{
    int loc;

    if ((loc = glGetUniformLocationARB(prog, name.c_str())) >= 0)
        glUniform1iARB(loc, d);

    OGLCK();
}

void ogl::shader::uniform(std::string name, float a) const
{
    int loc;

    if ((loc = glGetUniformLocationARB(prog, name.c_str())) >= 0)
        glUniform1fARB(loc, a);

    OGLCK();
}

void ogl::shader::uniform(std::string name, float a, float b) const
{
    int loc;

    if ((loc = glGetUniformLocationARB(prog, name.c_str())) >= 0)
        glUniform2fARB(loc, a, b);

    OGLCK();
}

void ogl::shader::uniform(std::string name, float a, float b, float c) const
{
    int loc;

    if ((loc = glGetUniformLocationARB(prog, name.c_str())) >= 0)
        glUniform3fARB(loc, a, b, c);

    OGLCK();
}

void ogl::shader::uniform(std::string name, float a, float b,
                                            float c, float d) const
{
    int loc;

    if ((loc = glGetUniformLocationARB(prog, name.c_str())) >= 0)
        glUniform4fARB(loc, a, b, c, d);

    OGLCK();
}

//-----------------------------------------------------------------------------
