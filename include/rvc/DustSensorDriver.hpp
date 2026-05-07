#pragma once

namespace rvc
{

    class DustSensorDriver
    {
    private:
        bool dust;
        bool active;

    public:
        DustSensorDriver();
        void initialize();
        void deactivateDustSensor();

        bool isActive() const;
        bool isDustDetected() const;
    };

}