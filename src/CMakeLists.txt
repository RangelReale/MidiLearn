add_executable(midilearn WIN32 
  MidiLearnMain.cpp MidiLearnApp.cpp Dlg_Search.cpp Dlg_Port.cpp ctl_miditrack.cpp
  MidiLearnMain.h MidiLearnApp.h Dlg_Search.h Dlg_Port.h ctl_miditrack.h
  ../res/resource.rc)

target_link_libraries(midilearn PRIVATE wx::core wx::base tse3)

# Keep the executable where run.bat and the debug-only "../../" data path expect it:
# build/bin/<Config>/midilearn.exe
set_target_properties(midilearn PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
