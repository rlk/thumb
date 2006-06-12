#include "main.hpp"
#include "opengl.hpp"
#include "sky.hpp"

//-----------------------------------------------------------------------------

ent::sky::sky(float k) :

    free(data->get_obj("sky.obj")),

    dist(k),

    glow("sky_glow.png"),
    fill("sky_fill.png"),

    prog(data->get_txt("sky.vert"),
         data->get_txt("sky.frag"))
{
}

void ent::sky::draw_fill() const
{
    glPushAttrib(GL_ENABLE_BIT);
    glPushMatrix();
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glScalef(dist, dist, dist);

        fill.bind(GL_TEXTURE0);
        glow.bind(GL_TEXTURE1);

        prog.bind();
        prog.uniform("fill", 0);
        prog.uniform("glow", 1);

        obj_draw_file(file);

        glUseProgramObjectARB(0);
    }
    glPopMatrix();
    glPopAttrib();
}

//-----------------------------------------------------------------------------
