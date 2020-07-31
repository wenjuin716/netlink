ROOT_DIR=$(shell pwd)
LIB_NETLINK_VER=libnl-3.2.25
LIB_INSTALL_DIR=$(ROOT_DIR)/install

all:
	if [ ! -d $(ROOT_DIR)/$(LIB_NETLINK_VER) ]; then \
		tar -zxf $(LIB_NETLINK_VER).tar.gz; \
	fi; \
	if [ ! -f $(ROOT_DIR)/$(LIB_NETLINK_VER)/Makefile ]; then \
		cd $(LIB_NETLINK_VER);./configure --disable-static --disable-cli --prefix=$(LIB_INSTALL_DIR);cd -; \
	fi; \
	cd $(LIB_NETLINK_VER);make;make install;cd -;

clean:
	rm -rf $(LIB_INSTALL_DIR);
	make clean -C $(LIB_NETLINK_VER);

distclean:
	rm -rf $(LIB_INSTALL_DIR) $(LIB_NETLINK_VER)

