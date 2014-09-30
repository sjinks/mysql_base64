TARGET = udf_gbbase64.so

all: $(TARGET)

$(TARGET): udf_gbbase64.c
	$(CC) -Wall -O2 -shared -fPIC $(CFLAGS) $(LDLAGS) -o "$@" $^

clean:
	rm -f "$(TARGET)"

install: $(TARGET)
	[ -d "/usr/lib/mysql/plugin" ] && install -m 0644 "$(TARGET)" "/usr/lib/mysql/plugin"

uninstall:
	[ -f "/usr/lib/mysql/plugin/$(TARGET)" ] && rm -f "/usr/lib/mysql/plugin/$(TARGET)"
