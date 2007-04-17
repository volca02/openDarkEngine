# Some global configuration checks

INCLUDE(CheckIncludeFiles)

CHECK_INCLUDE_FILES(inttypes.h HAVE_INTTYPES_H)
MARK_AS_ADVANCED(HAVE_INTTYPES_H)

# endianness check
INCLUDE(TestBigEndian)
TEST_BIG_ENDIAN(BIG_ENDIAN)
MARK_AS_ADVANCED(BIG_ENDIAN)
