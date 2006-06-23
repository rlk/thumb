//  Copyright (C) 2005 Robert Kooima
//
//  THUMB is free software; you can redistribute it and/or modify it under
//  the terms of  the GNU General Public License as  published by the Free
//  Software  Foundation;  either version 2  of the  License,  or (at your
//  option) any later version.
//
//  This program  is distributed in the  hope that it will  be useful, but
//  WITHOUT   ANY  WARRANTY;   without  even   the  implied   warranty  of
//  MERCHANTABILITY  or FITNESS  FOR A  PARTICULAR PURPOSE.   See  the GNU
//  General Public License for more details.

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <string>

#include "util.hpp"
#include "opengl.hpp"

//-----------------------------------------------------------------------------

#ifndef __APPLE__
PFNGLACTIVETEXTUREARBPROC              glActiveTextureARB;

PFNGLGETOBJECTPARAMETERIVARBPROC       glGetObjectParameterivARB;
PFNGLBINDATTRIBLOCATIONARBPROC         glBindAttribLocationARB;
PFNGLUSEPROGRAMOBJECTARBPROC           glUseProgramObjectARB;
PFNGLCREATESHADEROBJECTARBPROC         glCreateShaderObjectARB;
PFNGLCREATEPROGRAMOBJECTARBPROC        glCreateProgramObjectARB;
PFNGLVALIDATEPROGRAMARBPROC            glValidateProgramARB;
PFNGLSHADERSOURCEARBPROC               glShaderSourceARB;
PFNGLCOMPILESHADERARBPROC              glCompileShaderARB;
PFNGLATTACHOBJECTARBPROC               glAttachObjectARB;
PFNGLLINKPROGRAMARBPROC                glLinkProgramARB;
PFNGLGETINFOLOGARBPROC                 glGetInfoLogARB;
PFNGLDELETEOBJECTARBPROC               glDeleteObjectARB;

PFNGLGETUNIFORMLOCATIONARBPROC         glGetUniformLocationARB;
PFNGLUNIFORM1IARBPROC                  glUniform1iARB;
PFNGLUNIFORM1FARBPROC                  glUniform1fARB;
PFNGLUNIFORM2FARBPROC                  glUniform2fARB;
PFNGLUNIFORM3FARBPROC                  glUniform3fARB;
PFNGLUNIFORM4FARBPROC                  glUniform4fARB;

PFNGLGENFRAMEBUFFERSEXTPROC            glGenFramebuffersEXT;
PFNGLBINDFRAMEBUFFEREXTPROC            glBindFramebufferEXT;
PFNGLDELETEFRAMEBUFFERSEXTPROC         glDeleteFramebuffersEXT;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC       glFramebufferTexture2DEXT;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC     glCheckFramebufferStatusEXT;

PFNGLGENBUFFERSARBPROC                 glGenBuffersARB;
PFNGLBINDBUFFERARBPROC                 glBindBufferARB;
PFNGLMAPBUFFERARBPROC                  glMapBufferARB;
PFNGLUNMAPBUFFERARBPROC                glUnmapBufferARB;
PFNGLBUFFERDATAARBPROC                 glBufferDataARB;
PFNGLDELETEBUFFERSARBPROC              glDeleteBuffersARB;
#endif

//-----------------------------------------------------------------------------

#define init_proc(t, n) if (!(n = (t) glGetProcAddress(#n))) \
                            std::runtime_error(#n)

static void init_ext(const char *needle)
{
    const GLubyte *haystack, *c;

    // Search for the given string in the OpenGL extension strings.

    for (haystack = glGetString(GL_EXTENSIONS); *haystack; haystack++)
    {
        for (c = (const GLubyte *) needle; *c && *haystack; c++, haystack++)
            if (*c != *haystack)
                break;

        if ((*c == 0) && (*haystack == ' ' || *haystack == '\0'))
            return;
    }

    throw std::runtime_error(needle);
}

void init_ogl()
{
    init_ext("ARB_vertex_shader");
    init_ext("ARB_shader_objects");
    init_ext("ARB_fragment_shader");
    init_ext("ARB_vertex_buffer_object");
//  init_ext("ARB_pixel_buffer_object");
    init_ext("EXT_framebuffer_object");

#ifndef __APPLE__
    init_proc(PFNGLACTIVETEXTUREARBPROC,          glActiveTextureARB);

    init_proc(PFNGLGETOBJECTPARAMETERIVARBPROC,   glGetObjectParameterivARB);
    init_proc(PFNGLBINDATTRIBLOCATIONARBPROC,     glBindAttribLocationARB);
    init_proc(PFNGLUSEPROGRAMOBJECTARBPROC,       glUseProgramObjectARB);
    init_proc(PFNGLCREATESHADEROBJECTARBPROC,     glCreateShaderObjectARB);
    init_proc(PFNGLCREATEPROGRAMOBJECTARBPROC,    glCreateProgramObjectARB);
    init_proc(PFNGLVALIDATEPROGRAMARBPROC,        glValidateProgramARB);
    init_proc(PFNGLSHADERSOURCEARBPROC,           glShaderSourceARB);
    init_proc(PFNGLCOMPILESHADERARBPROC,          glCompileShaderARB);
    init_proc(PFNGLATTACHOBJECTARBPROC,           glAttachObjectARB);
    init_proc(PFNGLLINKPROGRAMARBPROC,            glLinkProgramARB);
    init_proc(PFNGLGETINFOLOGARBPROC,             glGetInfoLogARB);
    init_proc(PFNGLDELETEOBJECTARBPROC,           glDeleteObjectARB);

    init_proc(PFNGLGETUNIFORMLOCATIONARBPROC,     glGetUniformLocationARB);
    init_proc(PFNGLUNIFORM1IARBPROC,              glUniform1iARB);
    init_proc(PFNGLUNIFORM1FARBPROC,              glUniform1fARB);
    init_proc(PFNGLUNIFORM2FARBPROC,              glUniform2fARB);
    init_proc(PFNGLUNIFORM3FARBPROC,              glUniform3fARB);
    init_proc(PFNGLUNIFORM4FARBPROC,              glUniform4fARB);

    init_proc(PFNGLGENFRAMEBUFFERSEXTPROC,        glGenFramebuffersEXT);
    init_proc(PFNGLBINDFRAMEBUFFEREXTPROC,        glBindFramebufferEXT);
    init_proc(PFNGLDELETEFRAMEBUFFERSEXTPROC,     glDeleteFramebuffersEXT);
    init_proc(PFNGLFRAMEBUFFERTEXTURE2DEXTPROC,   glFramebufferTexture2DEXT);
    init_proc(PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC, glCheckFramebufferStatusEXT);
    init_proc(PFNGLGENBUFFERSARBPROC,             glGenBuffersARB);
    init_proc(PFNGLBINDBUFFERARBPROC,             glBindBufferARB);
    init_proc(PFNGLMAPBUFFERARBPROC,              glMapBufferARB);
    init_proc(PFNGLUNMAPBUFFERARBPROC,            glUnmapBufferARB);
    init_proc(PFNGLBUFFERDATAARBPROC,             glBufferDataARB);
    init_proc(PFNGLDELETEBUFFERSARBPROC,          glDeleteBuffersARB);
#endif
}

//-----------------------------------------------------------------------------

static const char *error_string(GLenum err)
{
    switch (err)
    {
    case GL_INVALID_ENUM:      return "Invalid enumerant";
    case GL_INVALID_VALUE:     return "Invalid value";
    case GL_INVALID_OPERATION: return "Invalid operation";
    case GL_STACK_OVERFLOW:    return "Stack overflow";
    case GL_OUT_OF_MEMORY:     return "Out of memory";
    case GL_TABLE_TOO_LARGE:   return "Table too large";
    }
    return "Unknown";
}

void check_ogl(const char *file, int line)
{
    GLenum err = glGetError();

    if (err != GL_NO_ERROR)
        std::cerr << "OpenGL error: "
                  << file << ":"
                  << line << " "
                  << error_string(err)
                  << std::endl;
}

//-----------------------------------------------------------------------------

ogl::fbo::fbo(GLint color_format,
              GLint depth_format, GLsizei w, GLsizei h) : _w(w), _h(h)
{
    GLint  o[1], v[4];
    GLenum T;

    if (((w & (w - 1)) == 0) && ((h & (w - 1)) == 0))
        T = GL_TEXTURE_2D;
    else
        T = GL_TEXTURE_RECTANGLE_ARB;

    get_framebuffer(o, v);

    frame = 0;
    color = 0;
    depth = 0;

    // Initialize the color render buffer object.

    if (color_format)
    {
        glGenTextures(1, &color);

        glBindTexture(T, color);
        glTexImage2D (T, 0, color_format, w, h, 0,
                      GL_RGBA, GL_INT, NULL);

        glTexParameteri(T, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(T, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(T, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
        glTexParameteri(T, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
    }

    // Initialize the depth render buffer object.

    if (depth_format)
    {
        glGenTextures(1, &depth);

        glBindTexture(T, depth);
        glTexImage2D (T, 0, depth_format, w, h, 0,
                      GL_DEPTH_COMPONENT, GL_INT, NULL);

        glTexParameteri(T, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(T, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(T, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
        glTexParameteri(T, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);

        glTexParameteri(T, GL_TEXTURE_COMPARE_MODE_ARB,
                           GL_COMPARE_R_TO_TEXTURE_ARB);
    }

    // Initialize the frame buffer object.

    glGenFramebuffersEXT(1, &frame);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frame);

    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                              GL_COLOR_ATTACHMENT0_EXT, T, color, 0);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                              GL_DEPTH_ATTACHMENT_EXT,  T, depth, 0);

    // Confirm the frame buffer object status.

    switch (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT))
    {
    case GL_FRAMEBUFFER_COMPLETE_EXT:
        break; 
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
        throw std::runtime_error("Framebuffer incomplete attachment");
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
        throw std::runtime_error("Framebuffer missing attachment");
    case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT:
        throw std::runtime_error("Framebuffer duplicate attachment");
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
        throw std::runtime_error("Framebuffer dimensions");
    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
        throw std::runtime_error("Framebuffer formats");
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
        throw std::runtime_error("Framebuffer draw buffer");
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
        throw std::runtime_error("Framebuffer read buffer");
    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
        throw std::runtime_error("Framebuffer unsupported");
    default:
        throw std::runtime_error("Framebuffer error");
    }

    // Zero the buffer.

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    set_framebuffer(o, v);

    GL_CHECK();
}

ogl::fbo::~fbo()
{
    glDeleteFramebuffersEXT(1, &frame);
    glDeleteTextures       (1, &color);

    GL_CHECK();
}

void ogl::fbo::bind_frame() const
{
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frame);
    glViewport(0, 0, _w, _h);

    GL_CHECK();
}

void ogl::fbo::bind_color(GLenum T) const
{
    glActiveTextureARB(T);
    {
        glBindTexture(GL_TEXTURE_2D, color);
    }
    glActiveTextureARB(GL_TEXTURE0);

    GL_CHECK();
}

//-----------------------------------------------------------------------------

ogl::vbo::vbo(GLenum target, GLenum usage, GLsizei size)
{
    glGenBuffersARB(1, &buffer);

    glBindBufferARB(target, buffer);
    glBufferDataARB(target, size, NULL, usage);

    glBindBufferARB(target, 0);

    GL_CHECK();
}

ogl::vbo::~vbo()
{
    glDeleteBuffersARB(1, &buffer);
    GL_CHECK();
}

void ogl::vbo::bind(GLenum target) const
{
    glBindBufferARB(target, buffer);
    GL_CHECK();
}

//-----------------------------------------------------------------------------

void ogl::shader::check_log(GLhandleARB handle)
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

ogl::shader::shader(std::string vert_str, std::string frag_str)
{
    prog = glCreateProgramObjectARB();
    vert = 0;
    frag = 0;

    // Compile the vertex shader.

    if (vert_str.length() > 0)
    {
        const GLcharARB *p = (const GLcharARB *) vert_str.c_str();

        vert = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);

        glShaderSourceARB (vert, 1, &p, 0);
        glCompileShaderARB(vert);
        
        check_log(vert);
    }

    // Compile the frag shader.

    if (frag_str.length() > 0)
    {
        const GLcharARB *p = (const GLcharARB *) frag_str.c_str();

        frag = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

        glShaderSourceARB (frag, 1, &p, 0);
        glCompileShaderARB(frag);
        
        check_log(frag);
    }

    // Link these shader objects to a program object.

    glAttachObjectARB(prog, vert);
    glAttachObjectARB(prog, frag);

    glLinkProgramARB(prog);   check_log(prog);

    GL_CHECK();
}

ogl::shader::~shader()
{
    glDeleteObjectARB(prog);
    glDeleteObjectARB(vert);
    glDeleteObjectARB(frag);
    GL_CHECK();
}

void ogl::shader::bind() const
{
    glUseProgramObjectARB(prog);
    GL_CHECK();
}

//-----------------------------------------------------------------------------

void ogl::shader::uniform(std::string name, int d) const
{
    int loc;

    if ((loc = glGetUniformLocationARB(prog, name.c_str())) >= 0)
        glUniform1iARB(loc, d);
}

void ogl::shader::uniform(std::string name, float a) const
{
    int loc;

    if ((loc = glGetUniformLocationARB(prog, name.c_str())) >= 0)
        glUniform1fARB(loc, a);
}

void ogl::shader::uniform(std::string name, float a, float b) const
{
    int loc;

    if ((loc = glGetUniformLocationARB(prog, name.c_str())) >= 0)
        glUniform2fARB(loc, a, b);
}

void ogl::shader::uniform(std::string name, float a, float b, float c) const
{
    int loc;

    if ((loc = glGetUniformLocationARB(prog, name.c_str())) >= 0)
        glUniform3fARB(loc, a, b, c);
}

void ogl::shader::uniform(std::string name, float a, float b,
                                            float c, float d) const
{
    int loc;

    if ((loc = glGetUniformLocationARB(prog, name.c_str())) >= 0)
        glUniform4fARB(loc, a, b, c, d);
}

//-----------------------------------------------------------------------------

void ogl::get_framebuffer(GLint o[1], GLint v[4])
{
    glGetIntegerv(GL_VIEWPORT,                v);
    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, o);
}

void ogl::set_framebuffer(GLint o[1], GLint v[4])
{
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, o[0]);
    glViewport(v[0], v[1], v[2], v[3]);
}

//-----------------------------------------------------------------------------

void ogl::draw_disc(int a)
{
    static float point[360][2];
    static bool  state = false;

    // If this is the first call, initialize the vertex array.

    if (!state)
    {
        for (int i = 0; i < 360; ++i)
        {
            point[i][0] = float(cos(i * 6.28318528f / 360.0f));
            point[i][1] = float(sin(i * 6.28318528f / 360.0f));
        }
        state = true;
    }

    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        glEnableClientState(GL_VERTEX_ARRAY);

        glVertexPointer(2, GL_FLOAT, 2 * a * sizeof (float), point);
        glDrawArrays(GL_POLYGON, 0, 360 / a);
    }
    glPopClientAttrib();
}

void ogl::draw_cube()
{
    static const GLfloat point[8][3] = {
        { -1.0f, -1.0f, -1.0f },
        { +1.0f, -1.0f, -1.0f },
        { -1.0f, +1.0f, -1.0f },
        { +1.0f, +1.0f, -1.0f },
        { -1.0f, -1.0f, +1.0f },
        { +1.0f, -1.0f, +1.0f },
        { -1.0f, +1.0f, +1.0f },
        { +1.0f, +1.0f, +1.0f },
    };

    static const GLuint index[6][4] = {
        { 0, 4, 6, 2 },
        { 5, 1, 3, 7 },
        { 0, 1, 5, 4 },
        { 6, 7, 3, 2 },
        { 1, 0, 2, 3 },
        { 4, 5, 7, 6 },
    };

    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    {
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        glVertexPointer(3, GL_FLOAT, 0, point);
        glEnableClientState(GL_VERTEX_ARRAY);

        glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, index);
    }
    glPopClientAttrib();
}

void ogl::draw_axes()
{
    glBegin(GL_LINES);
    {
        float k = 16.0f;

        glColor4f(1.0f, 0.0f, 0.0f, 0.5f);
        glVertex3f(-k, 0.0f, 0.0f);
        glVertex3f(+k, 0.0f, 0.0f);

        glColor4f(0.0f, 1.0f, 0.0f, 0.5f);
        glVertex3f(0.0f, -k, 0.0f);
        glVertex3f(0.0f, +k, 0.0f);

        glColor4f(0.0f, 0.0f, 1.0f, 0.5f);
        glVertex3f(0.0f, 0.0f, -k);
        glVertex3f(0.0f, 0.0f, +k);
    }
    glEnd();
}

//-----------------------------------------------------------------------------
