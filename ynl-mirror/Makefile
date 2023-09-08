LINUX ?= ../../linux

ynl:	## Copy kernel YNL files to here
	@echo "Checking for linux tree in $(LINUX)" && [ -d "$(LINUX)" ]
	tar cf - -C "$(LINUX)" Documentation/netlink tools/net/ynl | tar xf -

start:	## Run the server and agent
	screen -S ekho-hw-agent -dm ssh -t ekho-dpu sudo ./ynl-mirror/l3-hw-agent.py
	screen -S ekho-server -dm ssh -t ekho-kernel sudo ./ynl-mirror/l3-server.py

stop:	## Stop the server and agent
	-screen -X -S ekho-server stuff "^c"
	-screen -X -S ekho-hw-agent stuff "^c"

help:	## This help
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?## / {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)

.PHONY: help
.DEFAULT_GOAL := help
