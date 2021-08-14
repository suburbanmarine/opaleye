function(build_grpc_cpp IDL SRC HDR)
	
    get_filename_component(ABS_IDL ${IDL} ABSOLUTE)
    get_filename_component(WE_IDL ${IDL}  NAME_WE)


	set(LSRC "${CMAKE_CURRENT_BINARY_DIR}/${WE_IDL}.grpc.pb.cc")
	set(LHDR "${CMAKE_CURRENT_BINARY_DIR}/${WE_IDL}.grpc.pb.h")
	set(${SRC} "${LSRC}" PARENT_SCOPE)
	set(${HDR} "${LHDR}"  PARENT_SCOPE)
	#set_source_files_properties(${SRC} ${HDR} PROPERTIES GENERATED TRUE)

    message("IDL is ${WE_IDL} at ${ABS_IDL}")
    message("SRC is ${LSRC}")
    message("HDR is ${LHDR}")

	find_program(PROTOC_LOC protoc)
	find_program(GRPC_CPP_LOC grpc_cpp_plugin)

	add_custom_command(
		OUTPUT
			"${LSRC}"
			"${LHDR}"
		COMMAND
			${PROTOC_LOC}
		ARGS
			--proto_path=${CMAKE_CURRENT_SOURCE_DIR}
			--plugin=protoc-gen-grpc=${GRPC_CPP_LOC}
			--grpc_out=${CMAKE_CURRENT_BINARY_DIR}
			${ABS_IDL}
		DEPENDS
			"${ABS_IDL}"
		COMMENT 
			"Running gRPC C++ protocol buffer compiler on ${WE_IDL}"
		WORKING_DIRECTORY
			${CMAKE_CURRENT_BINARY_DIR}
		VERBATIM
	)

endfunction()

function(build_grpc_js IDL SRC)
	
    get_filename_component(ABS_IDL ${IDL} ABSOLUTE)
    get_filename_component(WE_IDL ${IDL}  NAME_WE)

	set(LSRC "${CMAKE_CURRENT_BINARY_DIR}/${WE_IDL}_grpc_web_pb.js")
	set(${SRC} "${LSRC}" PARENT_SCOPE)
	#set_source_files_properties(${SRC} ${HDR} PROPERTIES GENERATED TRUE)

    message("IDL is ${WE_IDL} at ${ABS_IDL}")
    message("SRC is ${LSRC}")

	find_program(PROTOC_LOC protoc)
#	find_program(GRPC_WEB_LOC 
#		NAMES
#			protoc-gen-grpc-web 
#		PATHS
#			${PROJECT_SOURCE_DIR}/external/grpc-web/javascript/net/grpc/web/
#		)

	set(GRPC_WEB_LOC ${PROJECT_SOURCE_DIR}/external/grpc-web/javascript/net/grpc/web/protoc-gen-grpc-web)

	add_custom_command(
		OUTPUT
			"${LSRC}"
		COMMAND
			${PROTOC_LOC}
		ARGS
			--proto_path=${CMAKE_CURRENT_SOURCE_DIR}
			--plugin=${GRPC_WEB_LOC}
			--grpc-web_out=import_style=commonjs,mode=grpcwebtext:${CMAKE_CURRENT_BINARY_DIR}
			${ABS_IDL}
		DEPENDS
			"${ABS_IDL}"
			protoc-gen-grpc-web-target
		COMMENT 
			"Running gRPC js protocol buffer compiler on ${WE_IDL}"
		WORKING_DIRECTORY
			${CMAKE_CURRENT_BINARY_DIR}
		VERBATIM
	)

endfunction()

function(build_grpc_node IDL SRC HDR)
	
    get_filename_component(ABS_IDL ${IDL} ABSOLUTE)
    get_filename_component(WE_IDL ${IDL}  NAME_WE)


	set(LSRC "${CMAKE_CURRENT_BINARY_DIR}/${WE_IDL}.grpc.pb.cc")
	set(LHDR "${CMAKE_CURRENT_BINARY_DIR}/${WE_IDL}.grpc.pb.h")
	set(${SRC} "${LSRC}" PARENT_SCOPE)
	set(${HDR} "${LHDR}"  PARENT_SCOPE)
	#set_source_files_properties(${SRC} ${HDR} PROPERTIES GENERATED TRUE)

    message("IDL is ${WE_IDL} at ${ABS_IDL}")
    message("SRC is ${LSRC}")
    message("HDR is ${LHDR}")

	find_program(PROTOC_LOC protoc)
	find_program(GRPC_LOC grpc_node_plugin)

	add_custom_command(
		OUTPUT
			"${LSRC}"
			"${LHDR}"
		COMMAND
			${PROTOC_LOC}
		ARGS
			--proto_path=${CMAKE_CURRENT_SOURCE_DIR}
			--plugin=protoc-gen-grpc=${GRPC_LOC}
			--grpc_out=${CMAKE_CURRENT_BINARY_DIR}
			${ABS_IDL}
		DEPENDS
			"${ABS_IDL}"
		COMMENT 
			"Running gRPC C++ protocol buffer compiler on ${WE_IDL}"
		WORKING_DIRECTORY
			${CMAKE_CURRENT_BINARY_DIR}
		VERBATIM
	)

endfunction()

function(build_proto_cpp IDL SRC HDR)
	
    get_filename_component(ABS_IDL ${IDL} ABSOLUTE)
    get_filename_component(WE_IDL ${IDL}  NAME_WE)

	set(LSRC "${CMAKE_CURRENT_BINARY_DIR}/${WE_IDL}.pb.cc")
	set(LHDR "${CMAKE_CURRENT_BINARY_DIR}/${WE_IDL}.pb.h")
	set(${SRC} "${LSRC}" PARENT_SCOPE)
	set(${HDR} "${LHDR}"  PARENT_SCOPE)
	#set_source_files_properties(${SRC} ${HDR} PROPERTIES GENERATED TRUE)

    message("IDL is ${WE_IDL} at ${ABS_IDL}")
    message("SRC is ${LSRC}")
    message("HDR is ${LHDR}")

	find_program(PROTOC_LOC protoc)

	add_custom_command(
		OUTPUT
			"${LSRC}"
			"${LHDR}"
		COMMAND
			${PROTOC_LOC}
		ARGS
			--proto_path=${CMAKE_CURRENT_SOURCE_DIR}
			--cpp_out=${CMAKE_CURRENT_BINARY_DIR}
			${ABS_IDL}
		DEPENDS
			"${ABS_IDL}"
		COMMENT 
			"Running C++ protocol buffer compiler on ${WE_IDL}"
		WORKING_DIRECTORY
			${CMAKE_CURRENT_BINARY_DIR}
		VERBATIM
	)

endfunction()

function(build_proto_js IDL SRC)
	
    get_filename_component(ABS_IDL ${IDL} ABSOLUTE)
    get_filename_component(WE_IDL ${IDL}  NAME_WE)

	set(LSRC "${CMAKE_CURRENT_BINARY_DIR}/${WE_IDL}_pb.js")
	set(${SRC} "${LSRC}" PARENT_SCOPE)
	#set_source_files_properties(${SRC} ${HDR} PROPERTIES GENERATED TRUE)

    message("IDL is ${WE_IDL} at ${ABS_IDL}")
    message("SRC is ${LSRC}")

	find_program(PROTOC_LOC protoc)

	add_custom_command(
		OUTPUT
			"${LSRC}"
		COMMAND
			${PROTOC_LOC}
		ARGS
			--proto_path=${CMAKE_CURRENT_SOURCE_DIR}
			--js_out=import_style=commonjs:${CMAKE_CURRENT_BINARY_DIR}
			${ABS_IDL}
		DEPENDS
			"${ABS_IDL}"
		COMMENT 
			"Running js protocol buffer compiler on ${WE_IDL}"
		WORKING_DIRECTORY
			${CMAKE_CURRENT_BINARY_DIR}
		VERBATIM
	)

endfunction()

#build_grpc_node(opaleye_service.proto opaleye_service_grpc_src opaleye_service_grpc_hdr)

build_grpc_cpp(opaleye_service.proto opaleye_service_grpc_src opaleye_service_grpc_hdr)
build_proto_cpp(opaleye_service.proto opaleye_service_src opaleye_service_hdr)
build_proto_cpp(opaleye_msg.proto opaleye_msg_src opaleye_msg_hdr)

#
build_grpc_js(opaleye_service.proto opaleye_service_js_src)
build_proto_js(opaleye_msg.proto opaleye_msg_js_src)

# protoc --plugin=protoc-gen-grpc=/home/rounin/suburbanmarine/grpc-web/javascript/net/grpc/web/protoc-gen-grpc-web opaleye_service.proto --grpc_out=import_style=commonjs,mode=grpcwebtext:. --js_out=import_style=commonjs,binary:.


add_library(opaleye_rpc
	${opaleye_service_grpc_src}
	${opaleye_service_src}
	${opaleye_msg_src}
	${opaleye_service_js_src}
	${opaleye_msg_js_src}
)
