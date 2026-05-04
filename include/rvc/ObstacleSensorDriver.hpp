#pragma once

namespace rvc {

class ObstacleSensorDriver {
private:
    bool direction[3];

public:
    ObstacleSensorDriver();
    void initialize();
    void deactivateObstacleSensor();
};

}