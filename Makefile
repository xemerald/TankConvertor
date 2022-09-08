#
#
#
export BYTE_ORDER = _INTEL
export CC = /usr/bin/gcc

usage:
	@echo "Usage: make normal or make centos"

# Compile rule for Earthworm version under 7.9
#
normal: libs echo_msg_normal
	@(cd ./src; make -f makefile.unix;);

# Compile rule for Earthworm version over 7.10
#
centos: libs echo_msg_centos
	@(cd ./src; make -f makefile.unix centos;);

#
#
libs: echo_msg_libraries
	@(cd ./src/libsrc; make -f makefile.unix;);

#
#
echo_msg_normal:
	@echo "--------------------------------";
	@echo "-      Making main tb2sac      -";
	@echo "--------------------------------";
echo_msg_centos:
	@echo "---------------------------------";
	@echo "- Making main tb2sac for centos -";
	@echo "---------------------------------";
echo_msg_libraries:
	@echo "----------------------------------";
	@echo "-        Making libraries        -";
	@echo "----------------------------------";

# Clean-up rules
clean:
	@(cd ./src; make -f makefile.unix clean;);
	@(cd ./src/libsrc; make -f makefile.unix clean; make -f makefile.unix clean_lib;);

clean_bin:
	@(cd ./src; make -f makefile.unix clean_bin;);
