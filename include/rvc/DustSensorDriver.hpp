#pragma once

namespace rvc {

class DustSensorDriver {
private:
    bool dust;

public:
    void initialize();
    void deactivateDustSensor();
};

}