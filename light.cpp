#include "opengl.hpp"
#include "light.hpp"

//-----------------------------------------------------------------------------

ent::light::light(GLenum n) : name(n)
{
    turn_world(-60.0f, 1.0f, 0.0f, 0.0f);
    turn_world( 30.0f, 0.0f, 1.0f, 0.0f);
}

void ent::light::draw_fill() const
{
    GLfloat P[4] = { 0.0, 0.0, 1.0, 0.0 };

    glPushMatrix();
    {
        mult_M();

        glEnable (name);
        glLightfv(name, GL_POSITION, P);
    }
    glPopMatrix();
}

//-----------------------------------------------------------------------------
