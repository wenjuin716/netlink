sub_dir=userspace kernel

export CFLAGS
all: $(sub_dir)
	for i in $(sub_dir); do \
		cd $$i;make;cd -; \
	done

clean: $(sub_dir)
	for i in $(sub_dir); do \
		cd $$i;make clean;cd -; \
	done
