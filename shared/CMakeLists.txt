cmake_minimum_required(VERSION 3.31)

# 1) Generate the bundle, if needed
add_bare_bundle(
  bare_worklet_bundle
  ENTRY shared/worklet.js
  OUT shared/worklet.bundle.h
  BUILTINS shared/builtins.json
  WORKING_DIRECTORY ..
)

# 2) Create the OBJECT library
add_library(bare_worklet_objs OBJECT)

set_target_properties(
  bare_worklet_objs
  PROPERTIES
    C_STANDARD 99
    POSITION_INDEPENDENT_CODE ON
)

target_sources(
  bare_worklet_objs
  PRIVATE
    ipc.h
    ipc.c
    worklet.c
    worklet.bundle.h
    worklet.h
)

target_link_libraries(
  bare_worklet_objs
  PUBLIC
    $<LINK_LIBRARY:WHOLE_ARCHIVE,bare_static>  # Adjust as needed
  PUBLIC
    log_static
)

# If link_bare_modules introduces any .def files, it might cause trouble in an OBJECT library.
# Make sure it doesn't embed .def files directly into this OBJECT library.
link_bare_modules(bare_worklet_objs WORKING_DIRECTORY ..)

# 3) Wrap the OBJECT library into a final STATIC library (same on all platforms)
add_library(bare_worklet STATIC
  $<TARGET_OBJECTS:bare_worklet_objs>
)

# Optionally set any properties or link additional libraries here
# set_target_properties(bare_worklet PROPERTIES OUTPUT_NAME "bare_worklet_static")
