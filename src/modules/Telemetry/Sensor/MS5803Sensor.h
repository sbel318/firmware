#include "configuration.h"

#if !MESHTASTIC_EXCLUDE_ENVIRONMENTAL_SENSOR && __has_include(<MS5803.h>)

#include "../mesh/generated/meshtastic/telemetry.pb.h"
#include "TelemetrySensor.h"

// Prefer a generic MS5803.h if available; otherwise support SparkFun library
#if __has_include(<MS5803.h>)
#include <MS5803.h>
#define USE_MS5803_GENERIC 1
#elif __has_include(<SparkFun_MS5803_14BA.h>)
#include <SparkFun_MS5803_14BA.h>
#define USE_MS5803_SPARKFUN 1
#endif

// MS5803 submersible absolute pressure sensor
// Reports depth via EnvironmentMetrics.distance (meters).
// NOTE: This implementation assumes an independently measured atmospheric pressure baseline.
// For v1, a configurable baseline can be used; future work can auto-couple to BMP280 readings.

class MS5803Sensor : public TelemetrySensor
{
  private:
    // Backing driver instance depending on available library
#if defined(USE_MS5803_GENERIC)
    MS5803 ms5803;
#elif defined(USE_MS5803_SPARKFUN)
    MS5803 ms5803; // SparkFun class is also named MS5803
#endif
    bool initialized = false;

    // Baseline atmospheric pressure in mbar (hPa). Set via config; default 1013.25.
    float atmosphericBaselineHpa = 1013.25f;

  public:
    MS5803Sensor();
    virtual bool getMetrics(meshtastic_Telemetry *measurement) override;
    virtual bool initDevice(TwoWire *bus, ScanI2C::FoundDevice *dev) override;

    // Optional: allow setting baseline from elsewhere (future coupling to BMP280).
    void setAtmosphericBaseline(float hpa) { atmosphericBaselineHpa = hpa; }
};

#endif


