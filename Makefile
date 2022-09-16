#
#
#
export CC = /usr/bin/gcc
export MAIN_BIN_NAME = tbconvert
INSTALL_DIR = /usr/local/bin
#
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
install:
	@echo Installing $(MAIN_BIN_NAME) to $(INSTALL_DIR)...
	@cp ./$(MAIN_BIN_NAME) $(INSTALL_DIR)
	@echo Finish installing of $(MAIN_BIN_NAME).

#
#
echo_msg_normal:
	@echo "-----------------------------------";
	@echo "-      Making main tbconvert      -";
	@echo "-----------------------------------";
echo_msg_centos:
	@echo "------------------------------------";
	@echo "- Making main tbconvert for centos -";
	@echo "------------------------------------";
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
