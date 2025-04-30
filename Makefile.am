##  This file is part of par2cmdline (a PAR 2.0 compatible file verification and
##  repair tool). See http://parchive.sourceforge.net for details of PAR 2.0.
##
##  Copyright (c) 2003 Peter Brian Clements
##  Copyright (c) 2019 Michael D. Nahas
##
##  par2cmdline is free software; you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation; either version 2 of the License, or
##  (at your option) any later version.
##
##  par2cmdline is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##
##  You should have received a copy of the GNU General Public License
##  along with this program; if not, write to the Free Software
##  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

AUTOMAKE_OPTIONS = subdir-objects

.DELETE_ON_ERROR:

AM_CPPFLAGS = -Wall -DNDEBUG -DPARPAR_ENABLE_HASHER_MD5CRC -DPARPAR_INVERT_SUPPORT -DPARPAR_SLIM_GF16
AM_CXXFLAGS = $(PTHREAD_CFLAGS)

noinst_LIBRARIES = libpar2.a \
	libparpar_gf16.a libparpar_gf16_sse2.a libparpar_gf16_ssse3.a libparpar_gf16_avx.a libparpar_gf16_avx2.a libparpar_gf16_avx512.a libparpar_gf16_vbmi.a libparpar_gf16_gfni.a libparpar_gf16_gfni_avx2.a libparpar_gf16_gfni_avx512.a libparpar_gf16_gfni_avx10.a libparpar_gf16_clmul.a libparpar_gf16_avx2_clmul.a libparpar_gf16_vpclmul.a libparpar_gf16_vpclgfni.a libparpar_gf16_neon.a libparpar_gf16_neonsha3.a libparpar_gf16_sve.a libparpar_gf16_sve2.a libparpar_gf16_rvv.a libparpar_gf16_rvv_zvbc.a \
	libparpar_hasher.a libparpar_hasher_sse2.a libparpar_hasher_clmul.a libparpar_hasher_xop.a libparpar_hasher_bmi1.a libparpar_hasher_avx2.a libparpar_hasher_avx512.a libparpar_hasher_avx512vl.a libparpar_hasher_armcrc.a libparpar_hasher_neon.a libparpar_hasher_neoncrc.a libparpar_hasher_sve2.a libparpar_hasher_zbkc.a

libpar2_a_SOURCES = src/crc.cpp src/crc.h \
	src/creatorpacket.cpp src/creatorpacket.h \
	src/criticalpacket.cpp src/criticalpacket.h \
	src/datablock.cpp src/datablock.h \
	src/descriptionpacket.cpp src/descriptionpacket.h \
	src/diskfile.cpp src/diskfile.h \
	src/filechecksummer.cpp src/filechecksummer.h \
	src/galois.cpp src/galois.h \
	src/letype.h \
	src/mainpacket.cpp src/mainpacket.h \
	src/md5.cpp src/md5.h \
	src/par1fileformat.cpp src/par1fileformat.h \
	src/par1repairer.cpp src/par1repairer.h \
	src/par1repairersourcefile.cpp src/par1repairersourcefile.h \
	src/par2creator.cpp src/par2creator.h \
	src/par2creatorsourcefile.cpp src/par2creatorsourcefile.h \
	src/par2fileformat.cpp src/par2fileformat.h \
	src/par2repairer.cpp src/par2repairer.h \
	src/par2repairersourcefile.cpp src/par2repairersourcefile.h \
	src/recoverypacket.cpp src/recoverypacket.h \
	src/reedsolomon.cpp src/reedsolomon.h \
	src/verificationhashtable.cpp src/verificationhashtable.h \
	src/verificationpacket.cpp src/verificationpacket.h \
	src/libpar2.cpp src/libpar2.h src/libpar2internal.h \
	src/utf8.cpp src/utf8.h
libpar2_a_DEPENDENCIES = \
	libparpar_gf16.a libparpar_gf16_sse2.a libparpar_gf16_ssse3.a libparpar_gf16_avx.a libparpar_gf16_avx2.a libparpar_gf16_avx512.a libparpar_gf16_vbmi.a libparpar_gf16_gfni.a libparpar_gf16_gfni_avx2.a libparpar_gf16_gfni_avx512.a libparpar_gf16_gfni_avx10.a libparpar_gf16_clmul.a libparpar_gf16_avx2_clmul.a libparpar_gf16_vpclmul.a libparpar_gf16_vpclgfni.a libparpar_gf16_neon.a libparpar_gf16_neonsha3.a libparpar_gf16_sve.a libparpar_gf16_sve2.a libparpar_gf16_rvv.a libparpar_gf16_rvv_zvbc.a \
	libparpar_hasher.a libparpar_hasher_sse2.a libparpar_hasher_clmul.a libparpar_hasher_xop.a libparpar_hasher_bmi1.a libparpar_hasher_avx2.a libparpar_hasher_avx512.a libparpar_hasher_avx512vl.a libparpar_hasher_armcrc.a libparpar_hasher_neon.a libparpar_hasher_neoncrc.a libparpar_hasher_sve2.a libparpar_hasher_zbkc.a
libpar2_a_LIBADD = parpar/gf16/libparpar_gf16*.o parpar/hasher/libparpar_hasher*.o

libparpar_gf16_a_SOURCES = \
	parpar/gf16/controller.cpp \
	parpar/gf16/controller_cpu.cpp \
	parpar/gf16/gfmat_coeff.c \
	parpar/gf16/gfmat_inv.cpp \
	parpar/gf16/gf16pmul.cpp \
	parpar/gf16/gf16mul.cpp \
	parpar/gf16/gf16_lookup.c \
	parpar/gf16/gf_add_generic.c \
	parpar/gf16/gf16_cksum_generic.c
libparpar_gf16_a_CFLAGS = $(CFLAGS_POSIX_SOURCE) -D_DARWIN_C_SOURCE -D_GNU_SOURCE -D_DEFAULT_SOURCE
libparpar_gf16_a_CPPFLAGS = -Iparpar/gf16 -Iparpar/gf16/opencl-include $(AM_CPPFLAGS)
libparpar_gf16_a_CXXFLAGS = 

libparpar_gf16_sse2_a_SOURCES = \
	parpar/gf16/gf16_xor_sse2.c \
	parpar/gf16/gf16_lookup_sse2.c \
	parpar/gf16/gf_add_sse2.c \
	parpar/gf16/gf16_cksum_sse2.c
libparpar_gf16_sse2_a_CFLAGS = $(CFLAGS_SSE2) $(libparpar_gf16_a_CFLAGS)

libparpar_gf16_ssse3_a_SOURCES = \
	parpar/gf16/gf16_shuffle_ssse3.c
libparpar_gf16_ssse3_a_CFLAGS = $(CFLAGS_SSSE3) $(libparpar_gf16_a_CFLAGS)

libparpar_gf16_avx_a_SOURCES = \
	parpar/gf16/gf16_shuffle_avx.c
libparpar_gf16_avx_a_CFLAGS = $(CFLAGS_AVX) $(libparpar_gf16_a_CFLAGS)

libparpar_gf16_avx2_a_SOURCES = \
	parpar/gf16/gf16_xor_avx2.c \
	parpar/gf16/gf16_shuffle_avx2.c \
	parpar/gf16/gf_add_avx2.c \
	parpar/gf16/gf16_cksum_avx2.c
libparpar_gf16_avx2_a_CFLAGS = $(CFLAGS_AVX2) $(libparpar_gf16_a_CFLAGS)

libparpar_gf16_avx512_a_SOURCES = \
	parpar/gf16/gf16_xor_avx512.c \
	parpar/gf16/gf16_shuffle_avx512.c \
	parpar/gf16/gf_add_avx512.c \
	parpar/gf16/gf16_cksum_avx512.c
libparpar_gf16_avx512_a_CFLAGS = $(CFLAGS_AVX512) $(libparpar_gf16_a_CFLAGS)

libparpar_gf16_vbmi_a_SOURCES = \
	parpar/gf16/gf16_shuffle_vbmi.c
libparpar_gf16_vbmi_a_CFLAGS = $(CFLAGS_VBMI) $(libparpar_gf16_a_CFLAGS)

libparpar_gf16_gfni_a_SOURCES = \
	parpar/gf16/gf16_affine_gfni.c
libparpar_gf16_gfni_a_CFLAGS = $(CFLAGS_GFNI) $(libparpar_gf16_a_CFLAGS)

libparpar_gf16_gfni_avx2_a_SOURCES = \
	parpar/gf16/gf16_affine_avx2.c \
	parpar/src/platform_warnings.c
libparpar_gf16_gfni_avx2_a_CFLAGS = $(CFLAGS_GFNI) $(CFLAGS_AVX2) $(libparpar_gf16_a_CFLAGS)

libparpar_gf16_gfni_avx512_a_SOURCES = \
	parpar/gf16/gf16_affine_avx512.c
libparpar_gf16_gfni_avx512_a_CFLAGS = $(CFLAGS_GFNI) $(CFLAGS_AVX512) $(libparpar_gf16_a_CFLAGS)

libparpar_gf16_gfni_avx10_a_SOURCES = \
	parpar/gf16/gf16_affine_avx10.c \
	parpar/gf16/gf_add_avx10.c
libparpar_gf16_gfni_avx10_a_CFLAGS = $(CFLAGS_GFNI) $(CFLAGS_AVX10) $(libparpar_gf16_a_CFLAGS)

libparpar_gf16_clmul_a_SOURCES = parpar/gf16/gf16pmul_sse.c
libparpar_gf16_clmul_a_CFLAGS = $(CFLAGS_PCLMUL) $(libparpar_gf16_a_CFLAGS)
libparpar_gf16_avx2_clmul_a_SOURCES = parpar/gf16/gf16pmul_avx2.c
libparpar_gf16_avx2_clmul_a_CFLAGS = $(CFLAGS_PCLMUL) $(CFLAGS_AVX2) $(libparpar_gf16_a_CFLAGS)
libparpar_gf16_vpclmul_a_SOURCES = parpar/gf16/gf16pmul_vpclmul.c
libparpar_gf16_vpclmul_a_CFLAGS = $(CFLAGS_VPCLMUL) $(libparpar_gf16_a_CFLAGS)
libparpar_gf16_vpclgfni_a_SOURCES = parpar/gf16/gf16pmul_vpclgfni.c
libparpar_gf16_vpclgfni_a_CFLAGS = $(CFLAGS_VPCLMUL) $(CFLAGS_GFNI) $(libparpar_gf16_a_CFLAGS)

libparpar_gf16_neon_a_SOURCES = \
	parpar/gf16/gf16_shuffle_neon.c \
	parpar/gf16/gf16_clmul_neon.c \
	parpar/gf16/gf_add_neon.c \
	parpar/gf16/gf16_cksum_neon.c \
	parpar/gf16/gf16pmul_neon.c
libparpar_gf16_neon_a_CFLAGS = $(CFLAGS_NEON) $(libparpar_gf16_a_CFLAGS)

libparpar_gf16_neonsha3_a_SOURCES = \
	parpar/gf16/gf16_clmul_sha3.c
libparpar_gf16_neonsha3_a_CFLAGS = $(CFLAGS_NEONSHA3) $(libparpar_gf16_a_CFLAGS)

libparpar_gf16_sve_a_SOURCES = \
	parpar/gf16/gf16_shuffle128_sve.c \
	parpar/gf16/gf_add_sve.c \
	parpar/gf16/gf16_cksum_sve.c
libparpar_gf16_sve_a_CFLAGS = $(CFLAGS_SVE) $(libparpar_gf16_a_CFLAGS)

libparpar_gf16_sve2_a_SOURCES = \
	parpar/gf16/gf16_shuffle128_sve2.c \
	parpar/gf16/gf16_shuffle2x128_sve2.c \
	parpar/gf16/gf16_shuffle512_sve2.c \
	parpar/gf16/gf16_clmul_sve2.c \
	parpar/gf16/gf_add_sve2.c \
	parpar/gf16/gf16pmul_sve2.c
libparpar_gf16_sve2_a_CFLAGS = $(CFLAGS_SVE2) $(libparpar_gf16_a_CFLAGS)

libparpar_gf16_rvv_a_SOURCES = \
	parpar/gf16/gf16_cksum_rvv.c \
	parpar/gf16/gf16_shuffle128_rvv.c \
	parpar/gf16/gf_add_rvv.c
libparpar_gf16_rvv_a_CFLAGS = $(CFLAGS_RV64V) $(CFLAGS_RV32V) $(libparpar_gf16_a_CFLAGS)

libparpar_gf16_rvv_zvbc_a_SOURCES = \
	parpar/gf16/gf16_clmul_rvv.c \
	parpar/gf16/gf16pmul_rvv.c
libparpar_gf16_rvv_zvbc_a_CFLAGS = $(CFLAGS_RV64ZVBC) $(CFLAGS_RV32ZVBC) $(libparpar_gf16_a_CFLAGS)

libparpar_hasher_a_SOURCES = \
	parpar/hasher/crc_zeropad.c \
	parpar/hasher/hasher.cpp \
	parpar/hasher/hasher_scalar.cpp \
	parpar/hasher/hasher_input.cpp \
	parpar/hasher/hasher_md5crc.cpp \
	parpar/hasher/tables.cpp \
	parpar/hasher/md5-final.c
# these two lines are needed for the right object files to be generated (for some reason)
libparpar_hasher_a_CFLAGS = $(CFLAGS_POSIX_SOURCE)
libparpar_hasher_a_CXXFLAGS = 

libparpar_hasher_sse2_a_SOURCES = parpar/hasher/hasher_sse.cpp
libparpar_hasher_sse2_a_CXXFLAGS = $(CFLAGS_SSE2) $(libparpar_hasher_a_CXXFLAGS)

libparpar_hasher_clmul_a_SOURCES = parpar/hasher/hasher_clmul.cpp
libparpar_hasher_clmul_a_CXXFLAGS = $(CFLAGS_PCLMUL) $(libparpar_hasher_a_CXXFLAGS)

libparpar_hasher_xop_a_SOURCES = parpar/hasher/hasher_xop.cpp
libparpar_hasher_xop_a_CXXFLAGS = $(CFLAGS_XOP) $(libparpar_hasher_a_CXXFLAGS)

libparpar_hasher_bmi1_a_SOURCES = parpar/hasher/hasher_bmi1.cpp
libparpar_hasher_bmi1_a_CXXFLAGS = $(CFLAGS_BMI1) $(libparpar_hasher_a_CXXFLAGS)

libparpar_hasher_avx2_a_SOURCES = parpar/hasher/hasher_avx2.cpp
libparpar_hasher_avx2_a_CXXFLAGS = $(CFLAGS_AVX2) $(libparpar_hasher_a_CXXFLAGS)

libparpar_hasher_avx512_a_SOURCES = parpar/hasher/hasher_avx512.cpp
libparpar_hasher_avx512_a_CXXFLAGS = $(CFLAGS_AVX512F) $(libparpar_hasher_a_CXXFLAGS)

libparpar_hasher_avx512vl_a_SOURCES = parpar/hasher/hasher_avx512vl.cpp
libparpar_hasher_avx512vl_a_CXXFLAGS = $(CFLAGS_AVX512_HASHER) $(CFLAGS_AVX10) $(libparpar_hasher_a_CXXFLAGS)

libparpar_hasher_armcrc_a_SOURCES = parpar/hasher/hasher_armcrc.cpp
libparpar_hasher_armcrc_a_CXXFLAGS = $(CFLAGS_ARMCRC) $(CFLAGS_ARMCRC_FP) $(libparpar_hasher_a_CXXFLAGS)

libparpar_hasher_neon_a_SOURCES = parpar/hasher/hasher_neon.cpp
libparpar_hasher_neon_a_CXXFLAGS = $(CFLAGS_NEON) $(libparpar_hasher_a_CXXFLAGS)

libparpar_hasher_neoncrc_a_SOURCES = parpar/hasher/hasher_neoncrc.cpp
libparpar_hasher_neoncrc_a_CXXFLAGS = $(CFLAGS_NEON) $(CFLAGS_ARMCRC) $(libparpar_hasher_a_CXXFLAGS)

libparpar_hasher_sve2_a_SOURCES = parpar/hasher/hasher_sve2.cpp
libparpar_hasher_sve2_a_CXXFLAGS = $(CFLAGS_SVE2) $(libparpar_hasher_a_CXXFLAGS)

libparpar_hasher_zbkc_a_SOURCES = parpar/hasher/hasher_rvzbc.cpp
libparpar_hasher_zbkc_a_CXXFLAGS = $(CFLAGS_RV64ZBKC) $(CFLAGS_RV32ZBKC) $(libparpar_hasher_a_CXXFLAGS)


bin_PROGRAMS = par2
man_MANS = man/par2.1

par2_SOURCES = src/par2cmdline.cpp \
	src/commandline.cpp src/commandline.h
par2_LDADD = libpar2.a -lstdc++ $(PTHREAD_LIBS) $(LDFLAGS_LIBATOMIC)

LDADD = -lstdc++ $(PTHREAD_LIBS) $(LDFLAGS_LIBATOMIC)

EXTRA_DIST = ROADMAP \
			 man/par2.1 \
			 automake.sh \
			 tests/flatdata.tar.gz \
			 tests/flatdata-par1files.tar.gz \
			 tests/flatdata-par2files.tar.gz \
			 tests/subdirdata.tar.gz \
			 tests/subdirdata-par2files-unix.tar.gz \
			 tests/subdirdata-par2files-win.tar.gz \
			 tests/smallsubdirdata.tar.gz \
			 tests/smallsubdirdata-par2files.tar.gz \
			 tests/100blocks.tar.gz \
			 tests/readbeyondeof.tar.gz \
			 tests/par2-0.6.8-crash.tar.gz \
			 tests/bug44.tar.gz \
			 tests/bug128-parfiles.tar.gz \
			 tests/bug190.tar.gz \
			 tests/test1 \
			 tests/test2 \
			 tests/test3 \
			 tests/test4 \
			 tests/test5 \
			 tests/test5rk \
			 tests/test6 \
			 tests/test7 \
			 tests/test8 \
			 tests/test9 \
			 tests/test10 \
			 tests/test11 \
			 tests/test12 \
			 tests/test13 \
			 tests/test14 \
			 tests/test15 \
			 tests/test16 \
			 tests/test17 \
			 tests/test18 \
			 tests/test19 \
			 tests/test20 \
			 tests/test21 \
			 tests/test22 \
			 tests/test23 \
			 tests/test24 \
			 tests/test25 \
			 tests/test26 \
			 tests/test27 \
			 tests/test28 \
			 tests/test29 \
			 tests/test30 \
			 tests/test31 \
			 tests/test32 \
			 tests/utf8_test \
			 tests/unit_tests


# Programs that need to be compiled for the test suite.
# These are the unit tests.
check_PROGRAMS = tests/letype_test tests/crc_test tests/md5_test tests/diskfile_test tests/libpar2_test tests/commandline_test tests/descriptionpacket_test tests/criticalpacket_test tests/reedsolomon_test tests/galois_test tests/utf8_test

tests_letype_test_SOURCES = src/letype_test.cpp src/letype.h

tests_crc_test_SOURCES = src/crc_test.cpp src/crc.cpp src/crc.h
tests_crc_test_LDADD = libpar2.a

tests_md5_test_SOURCES = src/md5_test.cpp src/md5.cpp src/md5.h
tests_md5_test_LDADD = libpar2.a

tests_diskfile_test_SOURCES = src/diskfile_test.cpp src/diskfile.cpp src/diskfile.h
tests_diskfile_test_LDADD = libpar2.a

tests_libpar2_test_SOURCES = src/libpar2_test.cpp src/libpar2.h
tests_libpar2_test_LDADD = libpar2.a

tests_commandline_test_SOURCES = src/commandline_test.cpp src/commandline.cpp src/commandline.h
tests_commandline_test_LDADD = libpar2.a

tests_descriptionpacket_test_SOURCES = src/descriptionpacket_test.cpp src/descriptionpacket.cpp src/descriptionpacket.h
tests_descriptionpacket_test_LDADD = libpar2.a

tests_criticalpacket_test_SOURCES = src/criticalpacket_test.cpp src/criticalpacket.cpp src/criticalpacket.h
tests_criticalpacket_test_LDADD = libpar2.a

tests_reedsolomon_test_SOURCES = src/reedsolomon_test.cpp src/reedsolomon.cpp src/reedsolomon.h

tests_galois_test_SOURCES = src/galois_test.cpp src/galois.cpp src/galois.h

tests_utf8_test_SOURCES = src/utf8_test.cpp src/utf8.cpp src/utf8.h


# List of all tests.
# tests/test* are integration tests that use the binary.
# $(check_PROGRAMS) is the list of compiled unit tests.
TESTS = tests/test1 \
		tests/test2 \
		tests/test3 \
		tests/test4 \
		tests/test5 \
		tests/test5rk \
		tests/test6 \
		tests/test7 \
		tests/test8 \
		tests/test9 \
		tests/test10 \
		tests/test11 \
		tests/test12 \
		tests/test13 \
		tests/test14 \
		tests/test15 \
		tests/test16 \
		tests/test17 \
		tests/test18 \
		tests/test19 \
		tests/test20 \
		tests/test21 \
		tests/test22 \
		tests/test23 \
		tests/test24 \
		tests/test25 \
		tests/test26 \
		tests/test27 \
		tests/test28 \
		tests/test29 \
		tests/test30 \
		tests/test31 \
		tests/test32 \
		tests/utf8_test \
		tests/unit_tests

install-exec-hook :
	cd $(DESTDIR)$(bindir)/ && \
	ln -sf par2$(EXEEXT) par2create$(EXEEXT) && \
	ln -sf par2$(EXEEXT) par2verify$(EXEEXT) && \
	ln -sf par2$(EXEEXT) par2repair$(EXEEXT)

uninstall-hook :
	rm -f $(DESTDIR)$(bindir)/par2create$(EXEEXT)
	rm -f $(DESTDIR)$(bindir)/par2verify$(EXEEXT)
	rm -f $(DESTDIR)$(bindir)/par2repair$(EXEEXT)
