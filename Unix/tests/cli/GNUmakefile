TOP = ../..
include $(TOP)/config.mak

CXXUNITTEST = test_cli

SOURCES = $(TOP)/ut/omitestcli.cpp $(TOP)/ut/omifaultsimtest.cpp test_cli.cpp \
    ../../cli/cli_c.c

INCLUDES = $(TOP) $(TOP)/common

DEFINES = TEST_BUILD

LIBRARIES = omiclient test_util micxx wql miapi  protocol sock base $(UNITTESTLIBS)  omi_error wsman http pal xmlserializer xml base micodec mofparser

include $(TOP)/mak/rules.mak

tests:
	$(call RUNUNITTEST)
