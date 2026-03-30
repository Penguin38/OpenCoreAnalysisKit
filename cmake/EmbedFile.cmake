#   cmake -DINPUT=index.html -DOUTPUT=index_html.h -DVAR_NAME=IndexHtml -P EmbedFile.cmake
#
#   #pragma once
#   #include <cstdint>
#   const std::uint8_t IndexHtml[] = { 0x3C, 0x21, 0x44, ... };
#   const std::size_t IndexHtmlSize = sizeof(IndexHtml) - 1;

if(NOT DEFINED INPUT)
    message(FATAL_ERROR "EmbedFile.cmake: INPUT variable is not defined.")
endif()
if(NOT DEFINED OUTPUT)
    message(FATAL_ERROR "EmbedFile.cmake: OUTPUT variable is not defined.")
endif()
if(NOT DEFINED VAR_NAME)
    message(FATAL_ERROR "EmbedFile.cmake: VAR_NAME variable is not defined.")
endif()

file(READ "${INPUT}" BYTES HEX)
string(REGEX REPLACE "(..)" "0x\\1, " FORMATTED_BYTES "${BYTES}")
set(HEADER_CONTENT "#pragma once\n\n\nconst char ${VAR_NAME}[] = { ${FORMATTED_BYTES} };\nconst size_t ${VAR_NAME}Size = sizeof(${VAR_NAME}) - 1;\n")
file(WRITE "${OUTPUT}" "${HEADER_CONTENT}")

message(STATUS "Successfully embedded ${INPUT} into ${OUTPUT} as '${VAR_NAME}'")
