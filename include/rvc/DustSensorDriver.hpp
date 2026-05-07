#pragma once

namespace rvc
{
class DustSensorDriver {
private:
    bool dust;
    bool readDustFromSensor() const { return dust; }

    //test
    bool active;

public:
    DustSensorDriver();
    void initialize();
    void deactivateDustSensor();
    bool hasDust() const { return readDustFromSensor(); }

    //test
    bool isActive() const;
    bool isDustDetected() const;
};

}