#include <stdexcept>

#include "ogl-frame.hpp"

// CAVEAT: This implementation only allows a stencil buffer to be used in
// the presence of a depth buffer.  The OpenGL implementation must support
// EXT_packed_depth_stencil.

//-----------------------------------------------------------------------------
/*
std::vector<GLuint> ogl::frame::stack;

void ogl::frame::push(GLuint o)
{
    stack.push_back(o);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, stack.back());
}

void ogl::frame::pop()
{
    stack.pop_back();
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, stack.back());
}
*/

std::vector<ogl::frame::state> ogl::frame::stack;

void ogl::frame::push(GLuint o, GLint x, GLint y, GLsizei w, GLsizei h)
{
    stack.push_back(state(o, x, y, w, h));
    stack.back().apply();
}

void ogl::frame::pop()
{
    stack.pop_back();
    stack.back().apply();
}

//-----------------------------------------------------------------------------

ogl::frame::frame(GLsizei w, GLsizei h,
                  GLenum  t, GLenum  f, bool c, bool d, bool s) :
    target(t),
    format(f),
    buffer(0),
    color(0),
    depth(0),
    has_color(c),
    has_depth(d),
    has_stencil(s),
    w(w),
    h(h)
{
    if (stack.empty()) stack.push_back(state(0, 0, 0, 0, 0));

    init();
}

ogl::frame::~frame()
{
    fini();
}

//-----------------------------------------------------------------------------

void ogl::frame::bind_color(GLenum unit) const
{
    ogl::bind_texture(target, unit, color);
}

void ogl::frame::free_color(GLenum unit) const
{
}

void ogl::frame::bind_depth(GLenum unit) const
{
    ogl::bind_texture(target, unit, depth);
}

void ogl::frame::free_depth(GLenum unit) const
{
}

//-----------------------------------------------------------------------------

/*
void ogl::frame::bind(bool proj) const
{
    // Store the current viewport state.

    glPushAttrib(GL_VIEWPORT_BIT);

    // Enable this framebuffer object's state.

    push(buffer);

    glViewport(0, 0, w, h);

    // Set up a one-to-one model-view-projection transformation, if requested.

    if (proj)
    {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, w, 0, h, 0, 1);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
    }
    OGLCK();
}

void ogl::frame::free(bool proj) const
{
    // Restore the previous transformation, if requested.

    if (proj)
    {
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();

        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }

    // Restore the previous viewport and bind the default frame buffer.

    glPopAttrib();

    pop();

    OGLCK();
}
*/

//-----------------------------------------------------------------------------

void ogl::frame::bind(int target) const
{
    push(buffer, 0, 0, w, h);

    // TODO: there's probably a smarter way to handle cube face switching.

    if (target)
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                                  GL_COLOR_ATTACHMENT0_EXT,
                                  target, color, 0);
    OGLCK();
}

void ogl::frame::bind() const
{
    push(buffer, 0, 0, w, h);
    OGLCK();
}

void ogl::frame::free() const
{
    pop();
    OGLCK();
}

//-----------------------------------------------------------------------------

void ogl::frame::draw(int i, int n) const
{
    GLfloat k =  2.0f / n;

    GLfloat l = -1.0f + (i    ) * k;
    GLfloat r = -1.0f + (i + 1) * k;
    GLfloat b = -1.0f;
    GLfloat t = -1.0f + k;

    bind_color(GL_TEXTURE0);
    {
        glMatrixMode(GL_PROJECTION);
        {
            glPushMatrix();
            glLoadIdentity();
        }
        glMatrixMode(GL_MODELVIEW);
        {
            glPushMatrix();
            glLoadIdentity();
        }

        glBegin(GL_QUADS);
        {
            glTexCoord2f(0.0f, 0.0f); glVertex2f(l, b);
            glTexCoord2f(1.0f, 0.0f); glVertex2f(r, b);
            glTexCoord2f(1.0f, 1.0f); glVertex2f(r, t);
            glTexCoord2f(0.0f, 1.0f); glVertex2f(l, t);
        }
        glEnd();

        glMatrixMode(GL_PROJECTION);
        {
            glPopMatrix();
        }
        glMatrixMode(GL_MODELVIEW);
        {
            glPopMatrix();
        }
    }
    free_color(GL_TEXTURE0);
}

void ogl::frame::draw() const
{
    bind_color(GL_TEXTURE0);
    {
        glMatrixMode(GL_PROJECTION);
        {
            glPushMatrix();
            glLoadIdentity();
        }
        glMatrixMode(GL_MODELVIEW);
        {
            glPushMatrix();
            glLoadIdentity();
        }

        glRecti(-1, -1, +1, +1);

        glMatrixMode(GL_PROJECTION);
        {
            glPopMatrix();
        }
        glMatrixMode(GL_MODELVIEW);
        {
            glPopMatrix();
        }
    }
    free_color(GL_TEXTURE0);
}

//-----------------------------------------------------------------------------

void ogl::frame::init_cube()
{
     // Initialize the cube map color render buffer object.

    ogl::bind_texture(target, 0, color);

    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, format,
                 w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, format,
                 w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, format,
                 w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, format,
                 w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, format,
                 w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, format,
                 w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_R,     GL_CLAMP_TO_EDGE);
}

void ogl::frame::init_color()
{
     // Initialize the color render buffer object.

    ogl::bind_texture(target, 0, color);

    glTexImage2D(target, 0, format, w, h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
}

void ogl::frame::init_depth()
{
    // Initialize the depth and stencil render buffer objects.

    ogl::bind_texture(target, 0, depth);

#ifdef GL_DEPTH_STENCIL_EXT
    if (has_stencil && ogl::has_depth_stencil)
        glTexImage2D(target, 0, GL_DEPTH24_STENCIL8_EXT, w, h, 0,
                     GL_DEPTH_STENCIL_EXT, GL_UNSIGNED_INT_24_8_EXT, NULL);
    else
#endif
        glTexImage2D(target, 0, GL_DEPTH_COMPONENT24,    w, h, 0,
                     GL_DEPTH_COMPONENT,   GL_UNSIGNED_BYTE,         NULL);

    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);

    glTexParameteri(target, GL_TEXTURE_COMPARE_MODE_ARB,
                            GL_COMPARE_R_TO_TEXTURE_ARB);
}

void ogl::frame::init_frame()
{
    // Initialize the frame buffer object.

    if (has_stencil)
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                                  GL_STENCIL_ATTACHMENT_EXT,
                                  target, depth, 0);
    if (has_depth)
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                                  GL_DEPTH_ATTACHMENT_EXT,
                                  target, depth, 0);
    if (has_color)
    {
        if (target == GL_TEXTURE_CUBE_MAP)
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                                      GL_COLOR_ATTACHMENT0_EXT,
                                      GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                                      color, 0);
        else
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                                      GL_COLOR_ATTACHMENT0_EXT,
                                      target, color, 0);
    }
    else
    {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    OGLCK();

    // Confirm the frame buffer object status.

    switch (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT))
    {
    case GL_FRAMEBUFFER_COMPLETE_EXT:
        break; 
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
        throw std::runtime_error("Framebuffer incomplete attachment");
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
        throw std::runtime_error("Framebuffer missing attachment");
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
}

void ogl::frame::init()
{
    if (has_color)
    {
        glGenTextures(1, &color);

        if (target == GL_TEXTURE_CUBE_MAP)
            init_cube();
        else
            init_color();
    }
    if (has_depth)
    {
        glGenTextures(1, &depth);
        init_depth();
    }

    glGenFramebuffersEXT(1, &buffer);

    push(buffer, 0, 0, w, h);
    {
        init_frame();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    pop();

    OGLCK();
}

void ogl::frame::fini()
{
    if (color) glDeleteTextures(1, &color);
    if (depth) glDeleteTextures(1, &depth);

    if (buffer) glDeleteFramebuffersEXT(1, &buffer);
}

//-----------------------------------------------------------------------------

