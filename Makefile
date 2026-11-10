# Build and test every problem in problems/.
#
#   make                build all problems into build/bin/
#   make test           build, run every problem, print a pass/fail summary
#   make sanitize       same as test, built with AddressSanitizer + UBSan
#   make run Q=21       build and run one problem by its number
#   make list           list all problems
#   make clean          delete build/
#
# Add STRICT=1 to treat warnings as errors (CI does this).

CC       ?= cc
WARN     := -std=c11 -Wall -Wextra -Wpedantic
ifeq ($(STRICT),1)
WARN     += -Werror
endif
CFLAGS   ?= -O2
SANFLAGS ?= -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer

SRCS     := $(foreach level,easy medium hard,$(sort $(wildcard problems/$(level)/*.c)))
BINS     := $(patsubst problems/%.c,build/bin/%,$(SRCS))
SAN_BINS := $(patsubst problems/%.c,build/san/%,$(SRCS))

.PHONY: all test sanitize run list clean

all: $(BINS)

build/bin/%: problems/%.c
	@mkdir -p $(@D)
	$(CC) $(WARN) $(CFLAGS) $< -o $@

build/san/%: problems/%.c
	@mkdir -p $(@D)
	$(CC) $(WARN) $(SANFLAGS) $< -o $@

test: $(BINS)
	@sh scripts/run_all.sh $(BINS)

sanitize: $(SAN_BINS)
	@sh scripts/run_all.sh $(SAN_BINS)

run:
	@q=$$(printf '%s' "$(Q)" | sed 's/^0*//'); \
	if [ -z "$$q" ]; then echo "usage: make run Q=<number>   e.g. make run Q=21"; exit 2; fi; \
	src=$$(ls problems/*/$$(printf '%02d' "$$q")_*.c 2>/dev/null | head -n 1); \
	if [ -z "$$src" ]; then echo "no problem numbered $(Q) (try: make list)"; exit 2; fi; \
	bin=build/bin/$${src#problems/}; bin=$${bin%.c}; \
	$(MAKE) --no-print-directory "$$bin" && "./$$bin"

list:
	@for f in $(SRCS); do printf '%s\n' "$$f"; done

clean:
	rm -rf build
