#include "main.hpp"
#include "image.hpp"

//-----------------------------------------------------------------------------

ogl::image::image()
{
    glGenTextures(1, &texture);
}

ogl::image::~image()
{
    glDeleteTextures(1, &texture);
}

void ogl::image::bind(GLenum T) const
{
    glActiveTextureARB(T);
    {
        glBindTexture(target, texture);
    }
    glActiveTextureARB(GL_TEXTURE0);
}

void ogl::image::draw() const
{
    float s = 1.0f;
    float t = 1.0f;

    glPushAttrib(GL_ENABLE_BIT);
    {
        glEnable(target);

        glDisable(GL_BLEND);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        glActiveTextureARB(GL_TEXTURE0);
        glBindTexture(target, texture);

        if (target == GL_TEXTURE_RECTANGLE_ARB)
        {
            s = float(width);
            t = float(height);
        }

        // Draw the image to a rectangle.

        glBegin(GL_QUADS);
        {
            glColor3f(1.0f, 1.0f, 1.0f);

            glTexCoord2f(0, 0); glVertex2i(0, 0);
            glTexCoord2f(s, 0); glVertex2i(1, 0);
            glTexCoord2f(s, t); glVertex2i(1, 1);
            glTexCoord2f(0, t); glVertex2i(0, 1);
        }
        glEnd();

        glBindTexture(target, 0);
    }
    glPopAttrib();
}

//-----------------------------------------------------------------------------

ogl::image_file::image_file(std::string filename)
{
    void *p;
    int   w;
    int   h;
    int   b;

    target = GL_TEXTURE_2D;
    format = GL_RGB;

    // Load the image data.

    if ((p = data->get_img(filename, w, h, b)))
    {
        // Determine the correct texture target and format.

        if (w & (w - 1)) target = GL_TEXTURE_RECTANGLE_ARB;
        if (h & (h - 1)) target = GL_TEXTURE_RECTANGLE_ARB;

        width  = (GLsizei) w;
        height = (GLsizei) h;

        switch (b)
        {
        case 1: format = GL_LUMINANCE;       break;
        case 2: format = GL_LUMINANCE_ALPHA; break;
        case 3: format = GL_RGB;             break;
        case 4: format = GL_RGBA;            break;
        }

        // Initialize the texture object.

        glBindTexture(target, texture);

        if (target == GL_TEXTURE_2D)
        {
            gluBuild2DMipmaps(target, b, w, h, format, GL_UNSIGNED_BYTE, p);

            glTexParameteri(target, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
            glTexParameteri(target, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER,
                                    GL_LINEAR_MIPMAP_LINEAR);
        }
        else
        {
            glTexImage2D(target, 0, b, w, h, 0, format, GL_UNSIGNED_BYTE, p);

            glTexParameteri(target, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
            glTexParameteri(target, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }

        glBindTexture(target, 0);
    }

    GL_CHECK();
}

//-----------------------------------------------------------------------------

ogl::image_snap::image_snap()
{
    int w = conf->get_i("window_w");
    int h = conf->get_i("window_h");

    GLubyte *p;

    if ((p = new GLubyte[w * h * 3]))
    {
        // Read the current frame.

        glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, p);

        // Prepare a texture using this image.

        width  = (GLsizei) w;
        height = (GLsizei) h;
        target = GL_TEXTURE_RECTANGLE_ARB;
        format = GL_RGB;

        glBindTexture(target, texture);

        glTexImage2D(target, 0, 3, w, h, 0, format, GL_UNSIGNED_BYTE, p);

        glTexParameteri(target, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
        glTexParameteri(target, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        delete p;
    }
}

//-----------------------------------------------------------------------------

