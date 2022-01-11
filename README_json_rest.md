// REST URL endpoints

// Get sensor data
// /api/v1/sensor_types
// /api/v1/sensor_types/pressure
// /api/v1/sensor_types/pressure/sensors/0
// /api/v1/sensor_types/temperature
// /api/v1/sensor_types/temperature/sensors/0
// /api/v1/sensor_types/temperature/sensors/1
// /api/v1/sensor_types/voltage
// /api/v1/sensor_types/voltage/sensors/0
// /api/v1/sensor_types/current
// /api/v1/sensor_types/current/sensors/0

//list of types
// /api/v1/sensor_types

//list of ???
// /api/v1/sensor_types/foo

//list of sensors
// /api/v1/sensor_types/foo/sensors

//sensor data
// /api/v1/sensor_types/foo/sensors/0

Sensor types list format
<sensor_types>
	<type>pressure</type>
	<type>temperature</type>
</sensor_types>

{
	"sensor_types" : 
	[
		{"type": "pressure"}, {"type": "temperature"}
	]
}

Sensor list format
<sensors>
	<sensor name="ext_temperature" index=0 />
	<sensor name="AO-therm" index=1 />
	<sensor name="AUX-therm" index=2 />
</sensors>
{
	"sensors" : 
	[
		{"name": "ext_temperature", "index": 0},
		{"name": "AO-therm", "index": 1},
		{"name": "AUX-therm", "index": 2}
	]
}

Sensor data format
{
	"name": "ext_temperature",
	"location": "asdfasf",
	"sample": {"value": 0.0, "unit": "degC", "timestamp": "20211224093600.123456"}
	"min" : {"value": 0.0, "unit": "degC"},
	"max" : {"value": 100.0, "unit": "degC",
	"precision": {"value": 1e-3, "unit": "degC"},
	"accuracy": {"value": 500, "unit": "ppm")
}

Data
{
	"value": 0.0,
	"unit": "degC",
	"timestamp": "20211224093600.123456"
}

<value>
	<name>External Pressure</name>
	<value unit="m">0.00</value>
	<type>pressure</type>
</value>

// Get camera info
// /api/v1/cameras/
// /api/v1/cameras/<camera-name>/properties
// /api/v1/cameras/<camera-name>/live/full
// /api/v1/cameras/<camera-name>/live/preview

// Get camera RTP stream
// /api/v1/cameras/
// /api/v1/cameras/<camera-name>/rtp
// /api/v1/cameras/<camera-name>/rtp/clients

// Get pipeline info
// /api/v1/cameras/<camera-name>/pipelines
// /api/v1/cameras/<camera-name>/pipelines/<pipeline-name>/properties
// /api/v1/cameras/<camera-name>/pipelines/<pipeline-name>/graph

// Get config file
// /api/v1/config

// /api/v1/system/
// /api/v1/system/nvpmodel/
// /api/v1/system/fan/

// /api/v1/util/
// /api/v1/util/time

// Other RPC calls
// /api/v1/rpc