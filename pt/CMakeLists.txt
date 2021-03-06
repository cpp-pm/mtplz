set (PT_SOURCE
  access.cc
  create.cc
  format.cc
  query.cc
)
add_library(mtplz_pt ${PT_SOURCE})
target_link_libraries(mtplz_pt kenlm_util)
target_compile_features(mtplz_pt PUBLIC cxx_range_for)

install(
    TARGETS mtplz_pt
    EXPORT "${targets_export_name}"
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    RUNTIME DESTINATION "bin"
    INCLUDES DESTINATION "${include_install_dir}"
)

if(MTPLZ_BUILD_EXE)
  AddExes(EXES binarize_phrase_table LIBRARIES mtplz_pt kenlm_util ${Boost_LIBRARIES} Threads::Threads)
  target_compile_features(binarize_phrase_table PUBLIC cxx_range_for)

  AddTests(TESTS
    access_test
    phrase_table_test
    LIBRARIES mtplz_pt kenlm_util ${Boost_LIBRARIES} Threads::Threads)
  target_compile_features(access_test PUBLIC cxx_range_for)
  target_compile_features(phrase_table_test PUBLIC cxx_range_for)
endif()
