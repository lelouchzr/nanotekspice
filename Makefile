BUILD_DIR ?= build
BONUS ?= ON
JOBS ?= 4
CMAKE_FLAGS ?= -DCMAKE_BUILD_TYPE=Release
.PHONY: all configure clean fclean re test
all: configure
	cmake --build $(BUILD_DIR) --parallel $(JOBS)
configure:
	cmake -S . -B $(BUILD_DIR) -DNTS_BONUS=$(BONUS) $(CMAKE_FLAGS)
clean:
	@if [ -d "$(BUILD_DIR)" ]; then cmake --build $(BUILD_DIR) --target clean; fi
fclean:
	rm -rf $(BUILD_DIR)
	rm -f nanotekspice
re: fclean
	$(MAKE) all
test: all
	ctest --test-dir $(BUILD_DIR) --output-on-failure
