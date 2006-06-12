#ifndef CONSTRAINT_HPP
#define CONSTRAINT_HPP

#include <set>
#include <cstring>

//-----------------------------------------------------------------------------

class constraint
{
protected:

    float M[16];
    float T[16];

    int   mode;
    int   axis;
    int   grid_a;
    float grid_d;

    float mouse_x;
    float mouse_y;
    float mouse_a;
    float mouse_d;

    void calc_rot(float&, float&, const float[3], const float[3]) const;
    void calc_pos(float&, float&, const float[3], const float[3]) const;

    void draw_rot(int) const;
    void draw_pos(int) const;

    void orient();

public:

    constraint();

    void set_mode(int);
    void set_axis(int);
    void set_grid(int);

    void set_transform(const float[16]);

    bool point(float[16], const float[3], const float[3]);
    void click(           const float[3], const float[3]);

    void draw(int n) const;
};

//-----------------------------------------------------------------------------

#endif
