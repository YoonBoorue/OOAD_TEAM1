#pragma once

namespace rvc
{

    class ObstacleSensorDriver
    {
    private:
        bool direction[3];
        bool active;

    public:
        ObstacleSensorDriver();
        void initialize();
        void deactivateObstacleSensor();

        bool isActive() const;
    };

}