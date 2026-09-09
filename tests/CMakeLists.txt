# Links only ml_notes, so the tests need neither wxWidgets nor TSE3 and build in
# a second. Run them with: ctest --test-dir build -C RelWithDebInfo
add_executable(test_ml_notes test_ml_notes.cpp)
target_link_libraries(test_ml_notes PRIVATE ml_notes)

add_test(NAME ml_notes COMMAND test_ml_notes)
