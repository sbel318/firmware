#include "configuration.h"

#if !MESHTASTIC_EXCLUDE_ENVIRONMENTAL_SENSOR && __has_include(<MS5803.h>)

#include "MS5803Sensor.h"
#include "TelemetrySensor.h"
#include <typeinfo>

// Use EnvironmentMetrics.distance to report water depth in meters.
// Conversion: Approximately 1 m water ~ 9.80665 kPa.
// MS5803 returns pressure in mbar (hPa). 1 kPa = 10 hPa.
// depth_m = max(0, (P_bottom_hPa - P_atm_hPa) / (9.80665 * 10))

MS5803Sensor::MS5803Sensor() : TelemetrySensor(meshtastic_TelemetrySensorType_SENSOR_UNSET, "MS5803") {}

bool MS5803Sensor::initDevice(TwoWire *bus, ScanI2C::FoundDevice *dev)
{
    LOG_INFO("Init sensor: %s", sensorName);

#if WIRE_INTERFACES_COUNT > 1
    if (dev && dev->address.port == ScanI2C::I2CPort::WIRE1) {
        Wire1.begin();
        ms5803.setI2C(&Wire1);
    } else {
        Wire.begin();
        ms5803.setI2C(&Wire);
    }
#else
    Wire.begin();
    ms5803.setI2C(&Wire);
#endif

    if (!ms5803.reset()) {
        LOG_ERROR("MS5803 reset failed");
        return false;
    }
    if (!ms5803.begin()) {
        LOG_ERROR("MS5803 begin failed");
        return false;
    }
    initialized = true;
    initI2CSensor();
    return true;
}

bool MS5803Sensor::getMetrics(meshtastic_Telemetry *measurement)
{
    if (!initialized) return false;

    // Trigger a reading and fetch results
    ms5803.read();
    float pressure_hPa = ms5803.getPressure();     // hPa (mbar)
    float temperature_c = ms5803.getTemperature(); // C

    // Compute depth (meters)
    float diff_hPa = pressure_hPa - atmosphericBaselineHpa;
    if (diff_hPa < 0) diff_hPa = 0; // don't report negative depth
    float depth_m = diff_hPa / (9.80665f * 10.0f);

    // Populate EnvironmentMetrics
    measurement->which_variant = meshtastic_Telemetry_environment_metrics_tag;
    measurement->variant.environment_metrics.has_temperature = true;
    measurement->variant.environment_metrics.temperature = temperature_c;
    measurement->variant.environment_metrics.has_distance = true; // use distance for depth (meters)
    measurement->variant.environment_metrics.distance = depth_m;

    return true;
}

#endif


