find_package(wxWidgets REQUIRED core base)
if(wxWidgets_FOUND)
  include(${wxWidgets_USE_FILE})
endif()

find_package(TSE3)
include_directories(${TSE3_INCLUDE_DIR})




add_executable(midilearn WIN32 
  MidiLearnMain.cpp MidiLearnApp.cpp Dlg_Search.cpp Dlg_Port.cpp ctl_miditrack.cpp
  MidiLearnMain.h MidiLearnApp.h Dlg_Search.h Dlg_Port.h ctl_miditrack.h)

target_link_libraries(midilearn ${TSE3_LIBRARY})

if(wxWidgets_FOUND)
  target_link_libraries(midilearn ${wxWidgets_LIBRARIES})
endif()
