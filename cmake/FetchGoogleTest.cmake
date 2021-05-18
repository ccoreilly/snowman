
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        4679637f1c9d5a0728bdc347a531737fad0b1ca3
)
FetchContent_MakeAvailable(googletest)