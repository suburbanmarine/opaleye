// REST URL endpoints

// Get sensor data
// /api/v1/sensors/
// /api/v1/sensors/pressure
// /api/v1/sensors/pressure/0
// /api/v1/sensors/temperature
// /api/v1/sensors/temperature/0
// /api/v1/sensors/temperature/1

Sensor list format
<sensors>
	<pressure>
		<sensor name="ext_pressure" index=0 />
	</pressure>
	<temperature>
		<sensor name="ext_temperature" index=0 />
		<sensor name="AO-therm" index=1 />
		<sensor name="AUX-therm" index=2 />
	</temperature>
</sensors>

Sensor data format
<name>External Pressure</name>
<value unit="m">0.00</value>
<type>pressure</type>

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