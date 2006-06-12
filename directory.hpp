#ifndef DIR_HPP
#define DIR_HPP

#include <string>
#include <vector>

//-----------------------------------------------------------------------------

typedef std::vector<std::string> strvec;

class directory
{
    strvec path;

public:

    directory(std::string="/");

    std::string cwd();
    void        set(std::string);
    void        get(strvec&, strvec&, std::string&);
};

//-----------------------------------------------------------------------------

#endif
