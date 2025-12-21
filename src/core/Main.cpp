#include "Application.h"
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
        return -1;
    }
    

    return 0;
}