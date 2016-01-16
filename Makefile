SOURCES = test/runtests.cpp \
	  test/fake_serial.cpp \
	  test/mock_arduino.cpp \
	  src/CC784Arduino.cpp

OBJECTS := $(addsuffix .o, $(addprefix .build/, $(basename $(SOURCES))))
DEPFILES := $(subst .o,.dep, $(subst .build/,.deps/, $(OBJECTS)))
TESTCPPFLAGS = -D_TEST_ -Isrc -Itest -Iarduino
CPPDEPFLAGS = -MMD -MP -MF .deps/$(basename $<).dep
RUNTEST := $(if $(COMSPEC), runtest.exe, runtest)


all: runtests

.build/%.o: %.cpp
		mkdir -p .deps/$(dir $<)
			mkdir -p .build/$(dir $<)
				$(COMPILE.cpp) $(TESTCPPFLAGS) $(CPPDEPFLAGS) -o $@ $<

runtests: $(OBJECTS)
		$(CC) $(OBJECTS) -lstdc++ -o $@

clean:
		@rm -rf .deps/ .build/ $(RUNTEST)

-include $(DEPFILES)
