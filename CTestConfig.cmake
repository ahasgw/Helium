## This file should be placed in the root directory of your project.
## Then modify the CMakeLists.txt file in the root directory of your
## project to incorporate the testing dashboard.

set(CTEST_PROJECT_NAME "Helium")
set(CTEST_NIGHTLY_START_TIME "01:00:00 UTC")

set(CTEST_DROP_METHOD "http")
set(CTEST_DROP_SITE "moldb.net")
set(CTEST_DROP_LOCATION "/CDash/submit.php?project=Helium")
set(CTEST_DROP_SITE_CDASH TRUE)
