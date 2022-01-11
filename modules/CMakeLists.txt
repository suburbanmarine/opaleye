cmake_minimum_required(VERSION 3.10)

project(opaleye-modules
	VERSION 0.2.0
	DESCRIPTION "Opaleye smart maritime node drivers"
	LANGUAGES C
)

INSTALL(
	PROGRAMS
		opaleye_gpio/opaleye_gpio.ko
	DESTINATION /lib/modules/
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

INSTALL(
	PROGRAMS
		config/etc/sysconfig/modules/opaleye_gpio.modules
	DESTINATION /etc/sysconfig/modules
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
