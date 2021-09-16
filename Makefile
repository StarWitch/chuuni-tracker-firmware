# Uncomment lines below if you have problems with $PATH
#SHELL := /bin/bash
#PATH := /usr/local/bin:$(PATH)

all:
	pio -f -c vim run

upload:
	pio -f -c vim run --target upload

clean:
	pio -f -c vim run --target clean

program:
	pio -f -c vim run --target program

uploadfs:
	pio -f -c vim run --target uploadfs

update:
	pio -f -c vim update

compiledb:
	pio run -t compiledb

leftleg:
	pio run -e leftleg -t upload
rightleg:
	pio run -e rightleg -t upload
lefthand:
	pio run -e lefthand -t upload
righthand:
	pio run -e righthand -t upload
torso:
	pio run -e torso -t upload
