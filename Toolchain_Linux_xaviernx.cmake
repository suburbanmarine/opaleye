set(CMAKE_SYSTEM_NAME Linux)

set(CXX_STANDARD 17)
set(C_STANDARD 11)

# or mtune cortex-a75 / cortex-a76
set(FLAGS_ARCH  "-march=armv8.2-a -mtune=generic-armv8.2-a")
set(FLAGS_GEN   "-ffunction-sections -fdata-sections")
set(FLAGS_WARN  "-Wall -Werror=return-type")

set(CMAKE_ASM_FLAGS                "${FLAGS_ARCH} ${FLAGS_GEN} ${FLAGS_WARN} -Wa,--warn -x assembler-with-cpp" CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS_DEBUG           "-Og -g3" CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS_RELEASE         "-O2"     CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS_RELWITHDEBINFO  "-Os -g3" CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS_MINSIZEREL      "-Os"     CACHE STRING "" FORCE)

set(CMAKE_C_FLAGS                   "${FLAGS_ARCH} ${FLAGS_GEN} ${FLAGS_WARN}" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_DEBUG             "-Og -g3" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELEASE           "-O2"     CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELWITHDEBINFO    "-Os -g3" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_MINSIZEREL        "-Os"     CACHE STRING "" FORCE)

set(CMAKE_CXX_FLAGS                 "${FLAGS_ARCH} ${FLAGS_GEN} ${FLAGS_WARN}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG           "-Og -g3" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE         "-O2"     CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "-Os -g3" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_MINSIZEREL      "-Os"     CACHE STRING "" FORCE)
