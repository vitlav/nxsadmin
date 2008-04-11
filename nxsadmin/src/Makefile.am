sbin_PROGRAMS = nxsadmin



AM_CXXFLAGS = @CXXFLAGS@ @GTKMM_CFLAGS@ -DPROGRAMNAME_LOCALEDIR=\"${PROGRAMNAME_LOCALEDIR}\"

nxsadmin_LDADD = @LIBS@ @GTKMM_LIBS@ 
nxsadmin_SOURCES = main.cc sa_main_window.cc sa_process_window.cc \
	sa_sys_utils.cc
noinst_HEADERS = sa_main_window.h sa_process_window.h sa_sys_utils.h
