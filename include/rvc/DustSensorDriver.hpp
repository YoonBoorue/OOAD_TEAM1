#pragma once

namespace rvc {

class DustSensorDriver {
private:
    bool dust;
    bool readDustFromSensor() const { return dust; }

public:
    DustSensorDriver();
    void initialize();
    void deactivateDustSensor();
    bool hasDust() const { return readDustFromSensor(); }
};

}