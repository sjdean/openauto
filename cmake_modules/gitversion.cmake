# cmake_modules/gitversion.cmake
cmake_minimum_required(VERSION 3.16)
message(STATUS "Resolving git version")

set(_build_version   "unknown")
set(_build_branch    "unknown")
set(_git_commit_count "0")
set(_build_changes   "")

find_package(Git QUIET)
if(Git_FOUND)
  execute_process(
          COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
          WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
          OUTPUT_VARIABLE   _build_version
          ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  execute_process(
          COMMAND "${GIT_EXECUTABLE}" rev-parse --abbrev-ref HEAD
          WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
          OUTPUT_VARIABLE   _build_branch
          ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  execute_process(
          COMMAND "${GIT_EXECUTABLE}" rev-list --count HEAD
          WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
          OUTPUT_VARIABLE   _git_commit_count
          ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  execute_process(
          COMMAND "${GIT_EXECUTABLE}" diff --no-ext-diff --quiet
          WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
          RESULT_VARIABLE   _git_dirty
          ERROR_QUIET
  )
  if(_git_dirty EQUAL 1)
    set(_build_changes "*")
  endif()
endif()

# Safe fallbacks if git unavailable (Yocto work dir, CI tarballs)
if(NOT _build_version   OR _build_version   STREQUAL "") set(_build_version   "unknown") endif()
if(NOT _build_branch    OR _build_branch    STREQUAL "") set(_build_branch    "unknown") endif()
if(NOT _git_commit_count OR _git_commit_count STREQUAL "") set(_git_commit_count "0")   endif()

message(STATUS "Git hash: ${_build_version}  branch: ${_build_branch}  commits: ${_git_commit_count}${_build_changes}")