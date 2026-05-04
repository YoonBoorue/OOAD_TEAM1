#pragma once

namespace rvc {

class DustSensorDriver {
private:
    bool dust;

public:
    DustSensorDriver();
    void initialize();
    void deactivateDustSensor();
};

}