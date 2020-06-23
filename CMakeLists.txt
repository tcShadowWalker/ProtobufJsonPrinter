project(ProtobufJsonPrinter)
cmake_minimum_required(VERSION 3.16)

### Find protobuf

find_package(Protobuf REQUIRED)
find_program(PROTOC NAMES protoc)
find_library(PROTOBUF_LIBRARIES NAMES protobuf)

set(GeneratedDir ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GeneratedDir})
include_directories( ${GeneratedDir} )

function(generate_protobuf filename)
	add_custom_command(OUTPUT ${GeneratedDir}/${filename}.pb.cc 
		COMMAND ${PROTOC} ${filename}.proto -I${CMAKE_CURRENT_SOURCE_DIR} --cpp_out=${GeneratedDir} 
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${filename}.proto )
	set(PROTO_SRCS ${PROTO_SRCS} ${GeneratedDir}/${filename}.pb.cc PARENT_SCOPE)
endfunction()

generate_protobuf(examples/example1)

#

include_directories(.)

add_executable(example1 examples/example1.cpp ${PROTO_SRCS})
target_link_libraries(example1 ${PROTOBUF_LIBRARIES})
