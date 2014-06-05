SUBDIRS += AdcSitcpReader
SUBDIRS += Dispatcher
SUBDIRS += AdcSitcpLogger
SUBDIRS += AdcSitcpMonitor

.PHONY: $(SUBDIRS)

all: $(SUBDIRS)
	@set -e; for dir in $(SUBDIRS); do $(MAKE) -C $${dir} $@; done

clean:
	@set -e; for dir in $(SUBDIRS); do $(MAKE) -C $${dir} $@; done
