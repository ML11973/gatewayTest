#include "grid.h"

Grid::Grid()
{

}

Grid::Grid(const Grid& other)
{

}

Grid::~Grid()
{

}



bool optimizeGrid(){
    // Get consumers

    // Get producers

    // Get current grid consumption

    // Optimize per policy

    switch(energyPolicy){
        case NONE:
            // No optimization
            break;
        case REMOTE_CONTROL:
            // Get set consumption for each node from external packet
            // Default setting is no optimization
            break;
        case MIN_GRID_CONSUMPTION:
            // Minimize consumption
            break;
        case MIN_GRID_PRODUCTION:
            // Maxiize consumption while staying close to zero grid output
            break;
        case CUSTOM:
            // Aim for target consumption
            break;
        default:
            break;
    }
}
    uint8_t * getGridReport();

    Node getLowestPriorityConsumer();
    Node getLowestPriorityProducer();
    bool sortNodesByPriority();
