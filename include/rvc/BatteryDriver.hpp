#pragma once

namespace rvc {

class BatteryDriver {
private:
    int LV;
    bool status;

public:
    BatteryDriver();
    void initialize();
    void turnOffBattery();
    void declineLV();
    void inclineLV();
    void startCharging();
    void stopCharging();
};

}