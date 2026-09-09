# The pure note and pitch helpers, kept clear of wxWidgets and TSE3 so tests/ can
# link them on their own.
add_library(ml_notes STATIC ml_notes.cpp ml_notes.h)
target_include_directories(ml_notes PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(midilearn WIN32 
  MidiLearnMain.cpp MidiLearnApp.cpp Dlg_Search.cpp Dlg_Port.cpp ctl_miditrack.cpp
  MidiLearnMain.h MidiLearnApp.h Dlg_Search.h Dlg_Port.h ctl_miditrack.h
  ../res/resource.rc)

target_link_libraries(midilearn PRIVATE wx::core wx::base tse3 ml_notes)

# Keep the executable where run.bat and the debug-only "../../" data path expect it:
# build/bin/<Config>/midilearn.exe
set_target_properties(midilearn PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

# Stage data/ beside the executable, the same layout the installer ships (data\ next to the
# exe, WorkingDir: {app}). Release builds resolve data/Standard.ins against the current
# directory, and ML_CTL_Control loads it at static-init time, so running the exe from its own
# directory without this crashes before main().
add_custom_command(TARGET midilearn POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${PROJECT_SOURCE_DIR}/data $<TARGET_FILE_DIR:midilearn>/data
  COMMENT "Staging data/ next to midilearn")
