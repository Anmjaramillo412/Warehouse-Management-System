#include "WarehouseSystem.h"
#include "WebServer.h"

#include <iostream>

using namespace std;


int main()
{
    cout << "Starting Warehouse Management System..."
        << endl;


    WarehouseSystem system;


    WebServer server(
        &system);


    server.run();


    return 0;
}