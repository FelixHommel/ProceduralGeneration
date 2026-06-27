#include "Application.hpp"

#include <exception>
#include <iostream>
#include <memory>
#include <utility>

int main()
{
    using namespace pen::app;

    std::unique_ptr<Application> app;

    try
    {
        app = std::move(std::make_unique<Application>());
    }
    catch(const std::exception& e)
    {
        std::cout << "Failed to create the Application: " << e.what();
        return 0;
    }

    app->start();

    return 0;
}
