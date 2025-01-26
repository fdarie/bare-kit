add_library(bare_kit_dotnet SHARED)

set_target_properties(
  bare_kit_dotnet
  PROPERTIES
  C_STANDARD 11
  POSITION_INDEPENDENT_CODE ON
  OUTPUT_NAME bare-kit
)

target_sources(
  bare_kit_dotnet
  PRIVATE
    src/BareKit/Native/IPC.c
    src/BareKit/Native/Worklet.c
)

target_link_libraries(
  bare_kit_dotnet
  PUBLIC
    bare_worklet
)
