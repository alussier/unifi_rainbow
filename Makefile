# macOS:
# brew tap messense/macos-cross-toolchains
# brew install aarch64-unknown-linux-gnu

# Compiler settings
CC = aarch64-linux-gnu-gcc
SRC = rainbow.c
TARGET = rainbow
CFLAGS = -lm -static

# Remote host and installation directory
REMOTE = root@unifi
REMOTE_INSTALL_DIR = /usr/local/bin
SYSTEMD_SERVICE_DIR = /lib/systemd/system

all: install

$(TARGET): $(SRC)
	$(CC) $(SRC) $(CFLAGS) -o $(TARGET)

# The install target copies the binary to the remote host and makes it executable.
install: $(TARGET)
	-ssh $(REMOTE) "systemctl stop rainbow"
	scp $(TARGET) $(REMOTE):$(REMOTE_INSTALL_DIR)/$(TARGET)
	scp $(TARGET).service $(REMOTE):$(SYSTEMD_SERVICE_DIR)/$(TARGET).service
	ssh $(REMOTE) "systemctl daemon-reload; systemctl enable rainbow; systemctl start rainbow"

clean:
	rm -f $(TARGET)
