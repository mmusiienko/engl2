#include <iostream>

#include "core/Application.h"
#include "spdlog/spdlog.h"


int main()
{
    try 
    {
        EnGl::Application app{};
        app.Run();
    } 
    catch (const std::exception& e)
    {
        spdlog::error(e.what());
        std::cin.get();
        return -1;
    }

    return 0;
}