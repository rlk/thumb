#ifndef PROG_HPP
#define PROG_HPP

//-----------------------------------------------------------------------------

namespace app
{
    class prog
    {
    protected:

        void snap(std::string, int, int) const;

    public:

        prog() { }

        virtual void timer(float)     { }
        virtual void point(int, int)  { }
        virtual void click(int, bool) { }
        virtual void keybd(int, bool, int);
        
        virtual void draw() const { }

        virtual ~prog() { }
    };
}

//-----------------------------------------------------------------------------

#endif
