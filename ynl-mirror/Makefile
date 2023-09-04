LINUX ?= ../linux

ynl:	## Copy kernel YNL files to here
	@echo "Checking for linux tree in $(LINUX)" && [ -d "$(LINUX)" ]
	tar cf - -C "$(LINUX)" Documentation/netlink tools/net/ynl | tar xf -

help:	## This help
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?## / {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)

.PHONY: help
.DEFAULT_GOAL := help
