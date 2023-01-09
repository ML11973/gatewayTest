#ifndef GRID_H
#define GRID_H

#include "node.h"
#include <vector>
#include <cstdint>
#include <string>

typedef enum{
    NONE,
    REMOTE_CONTROL,
    MIN_GRID_CONSUMPTION,
    MIN_GRID_PRODUCTION,
    CUSTOM
}ePolicy;

typedef struct{
    float totalPowerConsumption; // Total used electrical power
    float totalGridConsumption;  // Real grid consumption as given by counter, negative if electricity is reinjected in the grid
    std::vector<sNodeReport> nodeReports;
}sGridReport;




/**
 * @todo write docs
 */
class Grid
{
public:
    /**
     * Default constructor
     */
    Grid();

    /**
     * Copy constructor
     *
     * @param other TODO
     */
    Grid(const Grid& other);

    /**
     * Destructor
     */
    ~Grid();


    bool optimizeGrid();
    uint8_t * getGridReport();

private:
    Node getLowestPriorityConsumer();
    Node getLowestPriorityProducer();
    bool sortNodesByPriority();

    Node mainCounter;
    std::vector<Node> nodes;
    ePolicy energyPolicy;
};

#endif // GRID_H
