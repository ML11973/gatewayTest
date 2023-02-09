# Radio driver test

This small program is a basic client test for RM1S3 module.

It was used to develop and troubleshoot ISM3 driver port to Linux on a Pi CM4 platform.

It allows testing of communication with RM1S3 module and connection to a gateway.
Python script test_choice.py can be used to get basic gateway functionality from RM1S3 module in standalone mode, connected to a PC via USB. Pay attention to jumper configuration in this case.

If the program doesn't work, start by looking at the UART signals and work your way up in the ISM3 driver. Probable cause of failure is misconfiguration of Linux serial driver in buffered_uart.c.

Marc Leemann, February 2023
