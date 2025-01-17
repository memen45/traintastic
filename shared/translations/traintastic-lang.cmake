find_package(Python3)

file(GLOB TRANSLATION_FILES "${CMAKE_CURRENT_LIST_DIR}/*.json")
list(TRANSFORM TRANSLATION_FILES REPLACE "[.]json$" ".lang")

add_custom_command(
  OUTPUT ${TRANSLATION_FILES}
  COMMAND Python3::Interpreter json2lang.py
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  DEPENDS ${TRANSLATION_SRC_FILES}
)
