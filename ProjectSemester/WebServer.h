#pragma once

#include "WarehouseSystem.h"

class WebServer
{
private:

    WarehouseSystem* system;

public:

    WebServer(
        WarehouseSystem* system);

    void run();
};