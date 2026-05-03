#pragma once

namespace rvc {

class ObstacleSensorDriver {
private:
    bool direction[3];

public:
    void initialize();
    void deactivateObstacleSensor();
};

}