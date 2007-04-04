#include <stdexcept>
#include <SDL.h>

#include "frame.hpp"

//-----------------------------------------------------------------------------

ogl::frame::frame(GLsizei w, GLsizei h, GLenum t, GLenum cf, GLenum df) :
    w(w), h(h), target(t), color(0), depth(0)
{
    // Initialize the color render buffer object.

    if (cf && (color = new image(t, cf, w, h)))
    {
        glTexParameteri(t, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(t, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        color->free();
    }

    // Initialize the depth render buffer object.

    if (df && (depth = new image(t, df, w, h)))
    {
        // Need to pass GL_DEPTH_COMPONENT external format?

        glTexParameteri(t, GL_TEXTURE_COMPARE_MODE_ARB,
                           GL_COMPARE_R_TO_TEXTURE_ARB);

        glTexParameteri(t, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(t, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        depth->free();
    }

    // Initialize the frame buffer object.

    glGenFramebuffersEXT(1, &buffer);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, buffer);

    if (color) glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                                         GL_COLOR_ATTACHMENT0_EXT,
                                         t, color->get_o(), 0);
    if (depth) glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                                         GL_DEPTH_ATTACHMENT_EXT,
                                         t, depth->get_o(), 0);

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

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    OGLCK();
}

ogl::frame::~frame()
{
    if (buffer) glDeleteFramebuffersEXT(1, &buffer);

    if (depth) delete depth;
    if (color) delete color;

    OGLCK();
}

//-----------------------------------------------------------------------------

void ogl::frame::bind_color(GLenum unit) const
{
    if (color) color->bind(unit);
}

void ogl::frame::free_color(GLenum unit) const
{
    if (color) color->free(unit);
}

//-----------------------------------------------------------------------------

void ogl::frame::bind(bool proj) const
{
//  glFinish();

    // Store the current viewport state.

    glPushAttrib(GL_VIEWPORT_BIT);

    // Enable this framebuffer object's state.

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, buffer);
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

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    OGLCK();
}

void ogl::frame::draw() const
{
    if (color) color->draw();
}

void ogl::frame::null() const
{
    if (color) color->null();
    if (depth) depth->null();
}

//-----------------------------------------------------------------------------

