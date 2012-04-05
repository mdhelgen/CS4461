.PHONY: all
all: server client

FLAGS=-g -Wall 
EXEC=server client

server: server.c common.c
	gcc ${FLAGS} -o server server.c common.c -lmxml -lpthread 

client: client.c common.c
	gcc ${FLAGS} -o client client.c common.c -lmxml -lpthread



# Bash is required for "make tar" and "make submit".
SHELL=/bin/bash

# Information for "make submit"
COURSENAME=cs4461
COURSESEC=sec1
# Number of minutes before we warn about stale file to submit:
WARN_STALE_FILE=5


.PHONY: clean clobber tar submit

clean:
	rm -f $(EXEC) *.o
clobber: clean


TAR_FILE_BASENAME=${USER}
TAR_FILE=${TAR_FILE_BASENAME}.tgz

tar:
	@${MAKE} -s clean
	@rm -f ${TAR_FILE}
	@tar -czf ${TAR_FILE} *
	@echo "======================================================="
	@tar --list -vzf ${TAR_FILE}
	@echo "======================================================="
	@echo -n "Size: "
	@du -h ${TAR_FILE} | awk '{ print $$1 }'
	@echo "The files above are stored in: ${TAR_FILE}"
	@echo
	@echo "Note: If you extract the files from ${TAR_FILE}, it will"
	@echo "extract the files into your CURRENT DIRECTORY and overwrite"
	@echo "the existing files in your current directory!"


submit:
	@[[ -r ${TAR_FILE} ]] || echo "ERROR: ${TAR_FILE} missing, run 'make tar' first.";
	@[[ -r ${TAR_FILE} ]] || exit 1;

	@[[ `domainname` == "csl.mtu.edu" ]] || echo "ERROR: You must be on a Rekhi lab machine to submit!";
	@[[ `domainname` == "csl.mtu.edu" ]] || exit 1;

	@echo "Going to submit ${TAR_FILE}. It contains:"
	@echo "======================================================="
	@tar --list -vzf ${TAR_FILE}
	@echo "======================================================="

	@echo -n "Total size of ${TAR_FILE}: "
	@du -h ${TAR_FILE} | awk '{ print $$1 }'

	@echo -n "${TAR_FILE} was last modified "
	@echo -n $$(( $$(( `date +%s` - `date +%s -r ${TAR_FILE}` )) / 60 ));
	@echo " minutes ago."
	@echo "======================================================="

	@if [[ $$(( $$(( `date +%s` - `date +%s -r ${TAR_FILE}` )) / 60 )) -ge ${WARN_STALE_FILE} ]]; then \
	echo ;\
	echo "WARNING WARNING WARNING WARNING WARNING WARNING WARNING";\
	echo "${TAR_FILE} is ${WARN_STALE_FILE} or more minutes old."; \
	echo "Make sure it contains the latest copies of your files!."; \
	echo "If you recently made changes, re-run 'make tar' to"; \
	echo "create a fresh file."; \
	echo "WARNING WARNING WARNING WARNING WARNING WARNING WARNING"; \
	echo ;\
	fi;

	@echo "Press Ctrl+C to cancel, any other key to continue."
	@read

	@echo -n "Enter the name of this assignment: "; \
	read hwname; \
	echo submit ${COURSENAME}.${COURSESEC} $$hwname ${TAR_FILE}; \
	submit ${COURSENAME}.${COURSESEC} $$hwname ${TAR_FILE};
	@echo
	@echo
	@echo "SUBMITTING LATE?  If you submit your assignment"
	@echo "late, you must email the professor---otherwise"
	@echo "your submission might not be noticed!"
